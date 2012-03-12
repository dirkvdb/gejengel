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

#ifndef PLAY_QUEUE_MODEL_H
#define PLAY_QUEUE_MODEL_H

#include <gtkmm.h>

#include "utils/types.h"
#include "Core/playqueue.h"
#include "MusicLibrary/albumart.h"

#include "dispatchedsubscriber.h"
#include "signals.h"

namespace Gejengel
{

class IAlbumArtProvider;
class PlayQueue;

class PlayQueueModel 	: public PlayQueueSubscriber
						, public IDispatchedSubscriber<AlbumArt>
{
public:
    class Columns : public Gtk::TreeModel::ColumnRecord
    {
    public:
        Columns()
        {
            add(track); add(title); add(artist); add(albumArt); add(duration);
        }

        Gtk::TreeModelColumn<Track>                         track;
        Gtk::TreeModelColumn<Glib::ustring>                 artist;
        Gtk::TreeModelColumn<Glib::ustring>                 title;
        Gtk::TreeModelColumn<Glib::ustring>                 duration;
        Gtk::TreeModelColumn<Glib::RefPtr<Gdk::Pixbuf> >    albumArt;
    };

    PlayQueueModel(PlayQueue& playqueue, IAlbumArtProvider& artProvider);

    const Columns& columns();

    /* PlayQueueSubscriber interface */
    void onTrackQueued(uint32_t index, const Track& track);
    void onTrackRemoved(uint32_t index);
    void onTrackMoved(uint32_t sourceIndex, uint32_t destIndex);
    void onQueueCleared();

    /* IDispatchedSubscriber interface*/
    void onDispatchedItem(const AlbumArt& art, void* pData);

    void getSelectedRows(std::vector<uint32_t> indexes);

    PlayQueue& getQueue();
    uint32_t getQueueLength();

    Glib::RefPtr<Gtk::ListStore> getStore() { return m_ListStore; }

    SignalModelUpdated signalModelUpdated;

private:
    void addTrack(const Track& track, int32_t index);

    PlayQueue&                      m_PlayQueue;
    IAlbumArtProvider&              m_AlbumArtProvider;
    Glib::RefPtr<Gtk::ListStore>    m_ListStore;
    Columns                         m_Columns;
};

}

#endif
