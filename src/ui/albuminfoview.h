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

#ifndef ALBUM_INFO_VIEW_H
#define ALBUM_INFO_VIEW_H

#include <gtkmm.h>

#include "signals.h"
#include "trackview.h"
#include "dispatchedsubscriber.h"
#include "MusicLibrary/album.h"
#include "MusicLibrary/albumart.h"

namespace Gejengel
{

class Track;
class TrackModel;
class Settings;
class MusicLibrary;
class IGejengelCore;
class IAlbumArtProvider;

class AlbumInfoView : public IDispatchedSubscriber<Album>
					, public IDispatchedSubscriber<AlbumArt>
{
public:
    AlbumInfoView(IGejengelCore& core, TrackModel& trackModel);
    void setAlbum(const std::string& albumId);

    void onDispatchedItem(const Album& album, void* pData = nullptr);
    void onDispatchedItem(const AlbumArt& albumArt, void* pData = nullptr);

    Gtk::Widget& getWidget();
    SignalBackButtonPressed signalBackButtonPressed;
    SignalAlbumQueued signalAlbumQueued;
    SignalTrackQueued signalTrackQueued;

private:
    void saveSettings();
    void loadSettings();
    void queueAlbum();
    void dispatchAlbumArt();
    
    void iconThemeChanged();

    Gtk::Frame              	m_Frame;
    Gtk::HBox               	m_MainLayout;
    Gtk::VBox               	m_AlbumDetailsLayout;
    Gtk::Label              	m_Artist;
    Gtk::Label              	m_Album;
    Gtk::Label              	m_Year;
    Gtk::Label              	m_Genre;
    Gtk::Image              	m_CoverImage;
    Gtk::Image              	m_QueueImage;
    Gtk::Button             	m_QueueButton;
    Gtk::Button             	m_BackButton;
    TrackView               	m_TrackView;
    Glib::RefPtr<Gdk::Pixbuf> 	m_DefaultCover;
    IGejengelCore&        		m_Core;
    IAlbumArtProvider&			m_ArtProvider;
    std::string					m_AlbumId;
    std::vector<std::string> 	m_AlbumArtFilenames;
};

}

#endif
