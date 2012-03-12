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

#include "trackmodel.h"

#include <sstream>
#include <iomanip>
#include <cassert>
#include <glibmm/i18n.h>

#include "utils/trace.h"
#include "sharedfunctions.h"
#include "Core/commonstrings.h"
#include "MusicLibrary/track.h"

using namespace std;
using namespace Gtk;

namespace Gejengel
{

TrackModel::TrackModel()
{
    utils::trace("Create Track model");
    m_ListStore = ListStore::create(m_Columns);
    m_ListStore->set_sort_func(0, sigc::mem_fun(*this, &TrackModel::onCompareTrack));
}

TrackModel::~TrackModel()
{

}

const TrackModelColumns& TrackModel::columns()
{
    return m_Columns;
}

void TrackModel::addTrack(const Track& track)
{
    Glib::ustring duration;
    Shared::durationToString(track.durationInSec, duration);

    TreeModel::Row row = *(m_ListStore->append());
    row[m_Columns.artist]       = track.artist == UNKNOWN_ARTIST ? _(UNKNOWN_ARTIST) : track.artist;
    row[m_Columns.title]        = track.title == UNKNOWN_TITLE ? _(UNKNOWN_TITLE) : track.title;
    row[m_Columns.album]        = track.album == UNKNOWN_ALBUM ? _(UNKNOWN_ALBUM) : track.album;
    row[m_Columns.duration]     = duration;
    row[m_Columns.genre]        = track.genre;
    row[m_Columns.trackNr]      = track.trackNr;
    row[m_Columns.discNr]       = track.discNr;
    row[m_Columns.bitrate]      = track.bitrate;
    row[m_Columns.filepath]     = track.filepath;
    row[m_Columns.albumArtist]  = track.albumArtist;
    row[m_Columns.composer]     = track.composer;
    row[m_Columns.id]           = track.id;
}

void TrackModel::deleteTrack(const std::string& id)
{
    TreeModel::Children rows = m_ListStore->children();

    for (TreeModel::iterator iter = rows.begin(); iter < rows.end(); ++iter)
    {
        const std::string& modelId = (*iter)[m_Columns.id];
        if (modelId == id)
        {
            m_ListStore->erase(iter);
            break;
        }
    }
}

void TrackModel::clear()
{
    m_ListStore->clear();
}

void TrackModel::setSelectedAlbumId(const std::string& albumId)
{
    m_CurrentAlbumId = albumId;
}

void TrackModel::clearSelectedAlbumId()
{
    m_CurrentAlbumId.clear();
}

void TrackModel::setSortColumn(int32_t id, Gtk::SortType& order)
{
    m_ListStore->set_sort_column(id, order);
}

void TrackModel::getSortColumn(int32_t& id, Gtk::SortType& order) const
{
    m_ListStore->get_sort_column_id(id, order);
}

template <typename T>
int compare(const T& a, const T&b)
{
    if (a < b) return -1;
    else if (a > b) return 1;
    else if (a == b) return 0;
    else throw logic_error("Bad comparison implementation");
}

int TrackModel::onCompareTrack(const TreeModel::iterator& a, const TreeModel::iterator& b)
{
    const Gtk::TreeModel::Row& rowA = *a;
    const Gtk::TreeModel::Row& rowB = *b;

    Glib::ustring albumA, albumB;
    rowA.get_value(3, albumA);
    rowB.get_value(3, albumB);
    if (albumA != albumB)
    {
        return compare(albumA, albumB);
    }

    uint32_t trackA, trackB, discA, discB;
    rowA.get_value(0, trackA);
    rowB.get_value(0, trackB);
    rowA.get_value(6, discA);
    rowB.get_value(6, discB);

    if (discA == discB || discA == 0 || discB == 0)
    {
        return compare(trackA, trackB);
    }
    else if (discA < discB)
    {
        return -1;
    }
    else
    {
        return 1;
    }
}

void TrackModel::onItem(const Track& track, void* pData)
{
    if (m_CurrentAlbumId.empty() || track.albumId == m_CurrentAlbumId)
    {
        addTrack(track);    
    }
}

}
