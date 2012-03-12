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

#ifndef TRACK_MODEL_H
#define TRACK_MODEL_H

#include <gtkmm.h>

#include "utils/types.h"
#include "utils/subscriber.h"
#include "MusicLibrary/track.h"

namespace Gejengel
{
class Track;

class TrackModelColumns : public Gtk::TreeModel::ColumnRecord
{
public:
    TrackModelColumns()
    {
        add(trackNr); add(artist); add(title); add(album);
        add(duration); add(genre); add(discNr); add(bitrate); add(filepath);
        add(albumArtist); add(composer);
        
        // Id MUST be added last, it is used to count the columns
        add(id);
    }

    Gtk::TreeModelColumn<Glib::ustring> title;
    Gtk::TreeModelColumn<Glib::ustring> artist;
    Gtk::TreeModelColumn<Glib::ustring> album;
    Gtk::TreeModelColumn<Glib::ustring> duration;
    Gtk::TreeModelColumn<Glib::ustring> genre;
    Gtk::TreeModelColumn<Glib::ustring> filepath;
    Gtk::TreeModelColumn<Glib::ustring> albumArtist;
    Gtk::TreeModelColumn<Glib::ustring> composer;
    Gtk::TreeModelColumn<uint32_t>      trackNr;
    Gtk::TreeModelColumn<uint32_t>      discNr;
    Gtk::TreeModelColumn<uint32_t>      bitrate;
    Gtk::TreeModelColumn<std::string>   id;
};

class TrackModel : public utils::ISubscriber<Track>
{
public:
    TrackModel();
    virtual ~TrackModel();

    void addTrack(const Track& track);
    void deleteTrack(const std::string& id);
    const TrackModelColumns& columns();
    void clear();

    void setSelectedAlbumId(const std::string& albumId);
    void clearSelectedAlbumId();

    void setSortColumn(int32_t id, Gtk::SortType& order);
    void getSortColumn(int32_t& id, Gtk::SortType& order) const;
    int onCompareTrack(const Gtk::TreeModel::iterator& a, const Gtk::TreeModel::iterator& b);

    /* ITrackSubscriber interface */
    void onItem(const Track& track, void* pData = nullptr);

    Glib::RefPtr<Gtk::ListStore> getStore() { return m_ListStore; }

private:
    Glib::RefPtr<Gtk::ListStore>    m_ListStore;
    TrackModelColumns               m_Columns;
    std::string                     m_CurrentAlbumId;
};

}

#endif
