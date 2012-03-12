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

#ifndef NOW_PLAYING_VIEW_H
#define NOW_PLAYING_VIEW_H

#include <gtkmm.h>
#include <gtkmm/volumebutton.h>

#include "signals.h"
#include "dispatchedsubscriber.h"
#include "utils/types.h"
#include "utils/subscriber.h"
#include "Core/gejengelplugin.h"
#include "MusicLibrary/albumart.h"

namespace Gejengel
{

class Track;
class Playback;
class GejengelCore;

class NowPlayingView 	: public GejengelPlugin
						, public IDispatchedSubscriber<AlbumArt>
{
public:
    NowPlayingView();

    Gtk::Widget& getWidget();

    bool initialize(IGejengelCore& core);
    void destroy();

    std::string getName() const;
    std::string getDescription() const;

    void onPlay(const Track& track);
    void onPause();
    void onResume();
    void onStop();
    void onProgress(int32_t elapsedSeconds);
    void onVolumeChanged(int32_t volume);
    void onDispatchedItem(const AlbumArt& albumArt, void* pData = nullptr);

private:
    void saveSettings();
    void loadSettings();
    void onPlayPausePress();
    void onStopPress();
    void onNextPress();
    void onVolumeChange(double volume);
    bool onMoveProgressSlider(Gtk::ScrollType type, double value);
    bool onProgressButtonPress(GdkEventButton* pEvent);
    bool onProgressButtonRelease(GdkEventButton* pEvent);
    void updateProgressText(int32_t elapsedSeconds);
    Glib::ustring formatProgress(int32_t elapsedSeconds);
    void iconThemeChanged();
    void updateTrackInfo(const Track& track);
    void updatePlaybackState();
    void dispatchAlbumArt();

    Gtk::Frame              m_Frame;
    Gtk::HBox               m_MainLayout;
    Gtk::HBox               m_TrackDiscLayout;
    Gtk::Table              m_ButtonLayout;
    Gtk::Table              m_TextInfoLayout;
    Gtk::Label              m_Artist;
    Gtk::Label              m_Title;
    Gtk::Label              m_Album;
    Gtk::Label              m_Year;
    Gtk::Label              m_Track;
    Gtk::Label              m_Disc;
    Gtk::Label*             m_pDiscLabel;
    Gtk::Label              m_ProgressLabel;
    Gtk::Button             m_PlayPauseButton;
    Gtk::Button             m_StopButton;
    Gtk::Button             m_NextButton;
    Gtk::HScale             m_PlayingProgress;
    Gtk::Image              m_CoverImage;
    Gtk::Image              m_PlayImage;
    Gtk::Image              m_PauseImage;
    Gtk::Image              m_StopImage;
    Gtk::Image              m_NextImage;
    Gtk::VolumeButton       m_VolumeButton;
    Gtk::Adjustment         m_VolumeAdjustment;
    Glib::RefPtr<Gdk::Pixbuf> m_DefaultCover;

    IGejengelCore*          m_pCore;
    bool                    m_Paused;
    bool                    m_Seeking;
    int32_t                 m_TrackDuration;
    Glib::ustring           m_TrackDurationString;
};

}

#endif
