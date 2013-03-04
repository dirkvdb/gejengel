//    Copyright (C) 2009 Dirk Vanden Boer <dirk.vdb@gmail.com>
//
//    This program is free software; you can redistribute it and/or modify
//    it under the terms of the GNU General Public License as published by
//    the Free Software Foundation; either version 2 of the License, or
//    (at your option) any later version.
//
//    This program is distributed in the hope that it will be useful,
//    but WITHOUT ANY WARRANTY; without even the implied warranty of
//    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//    GNU General Public License for more details.
//
//    You should have received a copy of the GNU General Public License
//    along with this program; if not, write to the Free Software
//    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

#include "albummodel.h"

#include <sstream>
#include <iomanip>
#include <cassert>

#include <glibmm/i18n.h>
#include <gdkmm/pixbufloader.h>

#include "sharedfunctions.h"
#include "utils/log.h"
#include "utils/trace.h"
#include "MusicLibrary/album.h"
#include "MusicLibrary/albumart.h"
#include "Core/albumartprovider.h"
#include "Core/commonstrings.h"

using namespace std;
using namespace Gtk;
using namespace utils;

namespace Gejengel
{


AlbumModel::AlbumModel(IAlbumArtProvider& artProvider)
: m_AlbumArtProvider(artProvider)
, m_AlbumArtSize(0)
{
    utils::trace("Create AlbumModel");
    m_ListStore = Gtk::ListStore::create(m_Columns);
    m_AlbumArtDispatcher.connect(sigc::mem_fun(this, &AlbumModel::dispatchAlbumArt));
}


AlbumModel::~AlbumModel()
{
}

const AlbumModel::Columns& AlbumModel::columns()
{
    return m_Columns;
}

void AlbumModel::setSortColumn(int32_t id, Gtk::SortType type)
{
    m_ListStore->set_sort_column(id, type);
}

void AlbumModel::getSortColumn(int32_t& id, Gtk::SortType& type)
{
    m_ListStore->get_sort_column_id(id, type);
}

void AlbumModel::clear()
{
    Gtk::TreeModel::Children rows = m_ListStore->children();
    while (rows.begin() != rows.end())
    {
        m_ListStore->erase(rows.begin());
    }

    //clear seams to screw up the entire model
    //m_ListStore.clear();
}

void AlbumModel::addAlbum(const Album& album)
{
    Glib::ustring duration;
    if (album.durationInSec != 0)
    {
        Shared::durationToString(album.durationInSec, duration);
    }
    
    Gtk::TreeModel::Row row 	= *(m_ListStore->append());
    row[m_Columns.id]       	= album.id;
    row[m_Columns.title]    	= album.title == UNKNOWN_ALBUM ? _(UNKNOWN_ALBUM) : album.title;
    row[m_Columns.year]     	= album.year;
    row[m_Columns.duration] 	= duration;
    row[m_Columns.genre]    	= album.genre;
    row[m_Columns.dateAdded]	= static_cast<uint32_t>(album.dateAdded);
    row[m_Columns.albumArtUrl]  = album.artUrl;

    if (album.artist == UNKNOWN_ARTIST)
    {
        row[m_Columns.artist] = _(UNKNOWN_ARTIST);
    }
    else if (album.artist == VARIOUS_ARTISTS)
    {
        row[m_Columns.artist] = _(VARIOUS_ARTISTS);
    }
    else
    {
        row[m_Columns.artist] = album.artist;
    }

    fetchAlbumArt(album.id);
}

void AlbumModel::deleteAlbum(const std::string& id)
{
    Gtk::TreeModel::Children rows = m_ListStore->children();

    for (Gtk::TreeModel::iterator iter = rows.begin(); iter != rows.end(); ++iter)
    {
        const std::string& modelId = (*iter)[m_Columns.id];
        if (modelId == id)
        {
            m_ListStore->erase(iter);
            break;
        }
    }
}

void AlbumModel::updateAlbum(const Album& album)
{
    Gtk::TreeModel::Children rows = m_ListStore->children();

    for (Gtk::TreeModel::iterator iter = rows.begin(); iter != rows.end(); ++iter)
    {
        const std::string& modelId = (*iter)[m_Columns.id];
        if (modelId == album.id)
        {
            Glib::ustring duration;
            Shared::durationToString(album.durationInSec, duration);

            (*iter)[m_Columns.title]    = album.title == UNKNOWN_ALBUM ? _(UNKNOWN_ALBUM) : album.title;
            (*iter)[m_Columns.year]     = album.year;
            (*iter)[m_Columns.duration] = duration;
            (*iter)[m_Columns.genre]    = album.genre;
            (*iter)[m_Columns.dateAdded]= static_cast<uint32_t>(album.dateAdded);

            if (album.artist == UNKNOWN_ARTIST)
            {
                (*iter)[m_Columns.artist] = _(UNKNOWN_ARTIST);
            }
            else if (album.artist == VARIOUS_ARTISTS)
            {
                (*iter)[m_Columns.artist] = _(VARIOUS_ARTISTS);
            }
            else
            {
                (*iter)[m_Columns.artist] = album.artist;
            }

            break;
        }
    }
}

void AlbumModel::fetchAllAlbumArt()
{
    Gtk::TreeModel::Children rows = m_ListStore->children();

    for (Gtk::TreeModel::iterator iter = rows.begin(); iter != rows.end(); ++iter)
    {
        const std::string& albumId = (*iter)[m_Columns.id];
        m_AlbumArtProvider.getAlbumArt(albumId, *this);
    }
}

void AlbumModel::dispatchAlbumArt()
{
    std::lock_guard<std::mutex> lock(m_Mutex);
    for (size_t i = 0; i < m_FetchedAlbumArt.size(); ++i)
    {
    	AlbumArt& albumArt = m_FetchedAlbumArt[i];
    	if (albumArt.getData().empty())
    	{
    		continue;
    	}

        Gtk::TreeModel::Children rows = m_ListStore->children();
        for (Gtk::TreeModel::iterator iter = rows.begin(); iter != rows.end(); ++iter)
        {
            const std::string& modelId = (*iter)[m_Columns.id];
            if (modelId == albumArt.getAlbumId())
            {
                try
                {
                    (*iter)[m_Columns.albumArt] = Shared::createCoverPixBuf(albumArt, m_AlbumArtSize);
                }
                catch (Gdk::PixbufError& e)
                {
                    log::error("Failed to load album art for %s (%s)", albumArt.getAlbumId(), e.what());
                }
                break;
            }
        }
    }

    m_FetchedAlbumArt.clear();
}

void AlbumModel::fetchAlbumArt(const std::string& albumId)
{
    Gtk::TreeModel::Children rows = m_ListStore->children();

    for (Gtk::TreeModel::iterator iter = rows.begin(); iter != rows.end(); ++iter)
    {
        const std::string& modelId = (*iter)[m_Columns.id];
        if (modelId == albumId)
        {
            Album album(albumId);
            album.artUrl = (*iter)[m_Columns.albumArtUrl];
            
            m_AlbumArtProvider.getAlbumArt(album, *this);
            return;
        }
    }
}

void AlbumModel::setAlbumArtSize(uint32_t size)
{
	assert(size != 0);

	m_AlbumArtSize = size;
	log::debug("Refetch albums");
	clearAlbumArtCache();
}

void AlbumModel::onItem(const Album& album, void* pData)
{
    addAlbum(album);
}

void AlbumModel::onItem(const AlbumArt& albumArt, void* pData)
{
	{
		std::lock_guard<std::mutex> lock(m_Mutex);
		m_FetchedAlbumArt.push_back(albumArt);
	}
	m_AlbumArtDispatcher();
}

void AlbumModel::finalItemReceived()
{
}

void AlbumModel::clearAlbumArtCache()
{
    Gtk::TreeModel::Children rows = m_ListStore->children();
    for (Gtk::TreeModel::iterator iter = rows.begin(); iter != rows.end(); ++iter)
    {
        (*iter)[m_Columns.albumArt] = Glib::RefPtr<Gdk::Pixbuf>();
    }
}

}
