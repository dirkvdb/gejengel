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

#ifndef DETAILED_ALBUM_LAYOUT_H
#define DETAILED_ALBUM_LAYOUT_H

#include <gtkmm.h>

#include "uilayout.h"
#include "trackview.h"
#include "playqueueview.h"
#include "nowplayingview.h"
#include "detailedalbumview.h"


namespace Gejengel
{

class TrackModel;
class AlbumModel;
class PlayQueueModel;
class Settings;


class DetailedAlbumLayout : public UILayout
{
public:
    DetailedAlbumLayout(IGejengelCore& core, PlayQueueModel& playQueueModel, TrackModel& trackModel, AlbumModel& albumModel);
    virtual ~DetailedAlbumLayout();

    Gtk::Widget& getWidget();
    void getPlugins(std::vector<GejengelPlugin*>& plugins);

private:
    void loadSettings();
    void saveSettings();

    Settings&               m_Settings;
    TrackView               m_TrackView;
    
    DetailedAlbumView       m_AlbumView;
    PlayQueueView           m_PlayQueueView;
    NowPlayingView          m_NowPlaying;

    Gtk::VBox               m_LeftBox;
    Gtk::HPaned             m_AlbumTrackPane;
    Gtk::HPaned             m_Layout;
};

}

#endif
