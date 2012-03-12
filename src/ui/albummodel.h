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

#ifndef ALBUM_MODEL_H
#define ALBUM_MODEL_H

#include <gtkmm.h>
#include <mutex>

#include "utils/types.h"
#include "utils/subscriber.h"
#include "MusicLibrary/subscribers.h"

namespace Gejengel
{

class Album;
class AlbumArt;
class IAlbumArtProvider;

class AlbumModel 	: public utils::ISubscriber<Album>
					, public utils::ISubscriber<AlbumArt>
{
public:
    class Columns : public Gtk::TreeModel::ColumnRecord
    {
    public:
        Columns()
        {
            add(albumArt); add(title); add(artist); add(duration);
            add(genre); add(year); add(id); add(dateAdded);
            add(albumArtUrl);
        }

		Gtk::TreeModelColumn<std::string>                   	id;
        Gtk::TreeModelColumn<Glib::RefPtr<Gdk::Pixbuf> >    	albumArt;
        Gtk::TreeModelColumn<std::string>                   	albumArtUrl;
        Gtk::TreeModelColumn<Glib::ustring>                     title;
        Gtk::TreeModelColumn<Glib::ustring>                     artist;
        Gtk::TreeModelColumn<Glib::ustring>                     duration;
        Gtk::TreeModelColumn<Glib::ustring>                     genre;
        Gtk::TreeModelColumn<uint32_t>                          year;
        Gtk::TreeModelColumn<uint32_t>                          dateAdded;
    };

    AlbumModel(IAlbumArtProvider& artProvider);
    virtual ~AlbumModel();

    void addAlbum(const Album& album);
    void deleteAlbum(const std::string& id);
    void updateAlbum(const Album& album);
    void setAlbumArtSize(uint32_t size);
    const Columns& columns();
    void setSortColumn(int32_t id, Gtk::SortType type);
    void getSortColumn(int32_t& id, Gtk::SortType& type);
    void clear();

    void onItem(const Album& album, void* pData = nullptr);
    void onItem(const AlbumArt& albumArt, void* pData = nullptr);
    void finalItemReceived();

    Glib::RefPtr<Gtk::ListStore> getStore() { return m_ListStore; }

private:
    void fetchAlbumArt(const std::string& albumId);
    void fetchAllAlbumArt();
    void dispatchAlbumArt();
    void clearAlbumArtCache();
    
    IAlbumArtProvider&              m_AlbumArtProvider;
    Glib::RefPtr<Gtk::ListStore>    m_ListStore;
    Columns                         m_Columns;
    std::mutex					    m_Mutex;

    Glib::Dispatcher                m_AlbumArtDispatcher;
    std::vector<AlbumArt>           m_FetchedAlbumArt;
    uint32_t                        m_AlbumArtSize;
};

}

#endif /* ALBUM_MODEL_H */
