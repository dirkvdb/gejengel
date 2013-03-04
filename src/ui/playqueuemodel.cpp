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

#include "playqueuemodel.h"

#include <cassert>
#include <sstream>
#include <iomanip>
#include <glibmm/i18n.h>

#include "MusicLibrary/track.h"
#include "MusicLibrary/album.h"
#include "MusicLibrary/metadata.h"
#include "utils/log.h"
#include "utils/numericoperations.h"
#include "Core/albumartprovider.h"
#include "Core/gejengel.h"
#include "sharedfunctions.h"

using namespace std;
using namespace Gtk;
using namespace utils;

#define ALBUM_ART_SIZE 24

namespace Gejengel
{

PlayQueueModel::PlayQueueModel(PlayQueue& playqueue, IAlbumArtProvider& artProvider)
: m_PlayQueue(playqueue)
, m_AlbumArtProvider(artProvider)
{
    m_ListStore = ListStore::create(m_Columns);
}

const PlayQueueModel::Columns& PlayQueueModel::columns()
{
    return m_Columns;
}

void PlayQueueModel::addTrack(const Track& track, int32_t index)
{
    Glib::ustring durationString;
    Shared::durationToString(track.durationInSec, durationString);

    TreeModel::Row row;
    if (index == -1 || index >= static_cast<int>(m_ListStore->children().size()))
    {
        row = *(m_ListStore->append());
    }
    else
    {
        TreeIter iter = m_ListStore->get_iter(Gtk::TreePath(1, static_cast<uint32_t>(index)));
        row = *(m_ListStore->insert(iter));
    }

    row[m_Columns.track]    = track;
    row[m_Columns.artist]   = track.artist;
    row[m_Columns.title]    = track.title;
    row[m_Columns.duration] = durationString;

    m_AlbumArtProvider.getAlbumArt(track, *this);
    signalModelUpdated.emit();
}

void PlayQueueModel::onDispatchedItem(const AlbumArt& art, void* pData)
{
	Glib::RefPtr<Gdk::Pixbuf> artPixBuf;

	TreeModel::Children rows = m_ListStore->children();
	for (Gtk::TreeModel::const_iterator iter = rows.begin(); iter != rows.end(); ++iter)
	{
		Track track = (*iter)[m_Columns.track];
		if (track.albumId == art.getAlbumId())
		{
			Glib::RefPtr<Gdk::Pixbuf> curPixbuf = (*iter)[m_Columns.albumArt];
			if (!curPixbuf)
			{
				if (!artPixBuf)
				{
					try
					{
						artPixBuf = Shared::createCoverPixBuf(art, ALBUM_ART_SIZE);
					}
					catch (Gdk::PixbufError& e)
					{
						log::error("Failed to load album art for %s (%s)", track.albumId, e.what());
					}
				}

				if (artPixBuf)
				{
					(*iter)[m_Columns.albumArt] = artPixBuf;
				}
			}
		}
	}
}

void PlayQueueModel::onTrackQueued(uint32_t index, const Track& track)
{
    addTrack(track, index);
}

void PlayQueueModel::onTrackRemoved(uint32_t index)
{
    m_ListStore->erase(m_ListStore->get_iter(Gtk::TreePath(1, index)));
    signalModelUpdated.emit();
}

void PlayQueueModel::onTrackMoved(uint32_t sourceIndex, uint32_t destIndex)
{
    if (sourceIndex < destIndex)
    {
        ++destIndex;
    }

    if (destIndex >= m_ListStore->children().size())
    {
        //gtkmm should fix this, ther is no move move after
        TreeIter source = m_ListStore->get_iter(Gtk::TreePath(1, sourceIndex));
        TreeIter dest = --m_ListStore->children().end();
        gtk_list_store_move_after(m_ListStore->gobj(), const_cast<GtkTreeIter*>(source.get_gobject_if_not_end()), const_cast<GtkTreeIter*>(dest.get_gobject_if_not_end()));
    }
    else
    {
        m_ListStore->move(m_ListStore->get_iter(Gtk::TreePath(1, sourceIndex)), m_ListStore->get_iter(Gtk::TreePath(1, destIndex)));
    }

    signalModelUpdated.emit();
}

void PlayQueueModel::onQueueCleared()
{
    m_ListStore->clear();
    signalModelUpdated.emit();
}

PlayQueue& PlayQueueModel::getQueue()
{
    return m_PlayQueue;
}

uint32_t PlayQueueModel::getQueueLength()
{
    uint32_t length = 0;

    TreeModel::Children rows = m_ListStore->children();
    for (Gtk::TreeModel::const_iterator iter = rows.begin(); iter != rows.end(); ++iter)
    {
        Track track = (*iter)[m_Columns.track];
        length += track.durationInSec;
    }

    return length;
}

}
