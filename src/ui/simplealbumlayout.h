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

#ifndef SIMPLE_ALBUM_LAYOUT_H
#define SIMPLE_ALBUM_LAYOUT_H

#include <gtkmm.h>

#include "uilayout.h"
#include "albumview.h"
#include "albuminfoview.h"
#include "playqueueview.h"
#include "nowplayingview.h"
#include "detailedalbumview.h"


namespace Gejengel
{

class TrackModel;
class AlbumModel;
class PlayQueueModel;
class Settings;
class IGejengelCore;


class SimpleAlbumLayout : public UILayout
{
public:
    SimpleAlbumLayout(IGejengelCore& core, PlayQueueModel& playQueueModel, TrackModel& trackModel, AlbumModel& albumModel);
    virtual ~SimpleAlbumLayout();

    Gtk::Widget& getWidget();
    void getPlugins(std::vector<GejengelPlugin*>& plugins);

private:
    void loadSettings();
    void saveSettings();
    void showAlbumInfo(const std::string& albumId);
    void showAlbums();

    Settings&               m_Settings;
    DetailedAlbumView       m_AlbumView;
    PlayQueueView           m_PlayQueueView;
    NowPlayingView          m_NowPlaying;
    AlbumInfoView           m_AlbumInfoView;

    Gtk::VBox               m_Layout;
    Gtk::HPaned             m_AlbumQueuePane;
};

}

#endif
