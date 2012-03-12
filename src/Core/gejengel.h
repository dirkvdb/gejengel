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

#ifndef GEJENGEL_H
#define GEJENGEL_H

#include "config.h"
#include "albumartgrabber.h"
#include "gejengelcore.h"
#include "settings.h"
#include "pluginmanager.h"
#include "libraryaccess.h"
#include "upnpserversettings.h"
#include "utils/types.h"

#include <glibmm.h>

namespace Gejengel
{

class PlaybackEngine;
class TrackSubscriber;
class MainWindow;
class PlayQueue;
class IAlbumArtProvider;

class GejengelCore : public IGejengelCore
{
public:
    GejengelCore();
    ~GejengelCore();

    void run();

    void play();
    void pause();
    void resume();
    void prev();
    void next();
    void stop();
    PlaybackState getPlaybackState();

    void seek(double seconds);
    double getTrackPosition();

    void setVolume(int32_t volume);
    int32_t getVolume();

    void getCurrentTrack(Track& track);

    void quitApplication();
    void showHideWindow();

    bool getNextTrack(Track& track);

    Settings& getSettings();
#ifdef HAVE_LIBUPNP
    UPnPServerSettings& getUPnPServerSettings();
#endif
    PluginManager& getPluginManager();
    PlayQueue& getPlayQueue();
    IAlbumArtProvider& getAlbumArtProvider();
    IStatusReporter& getStatusReporter();
    LibraryAccess& getLibraryAccess();

    Glib::Dispatcher dispatchProgress;
    Glib::Dispatcher dispatchVolumeChanged;
    Glib::Dispatcher dispatchNewTrackStarted;

private:
    void saveSettings();
    void loadSettings();

    enum FetchStatus
    {
        FetchStarted,
        FetchSuccess,
        FetchFailed
    };

    Glib::Dispatcher dispatchGetNextTrack;
    Glib::Dispatcher dispatchStop;
    void sendProgress();
    void sendVolumeChange();
    void newTrackStarted();
    void sendGetNextTrack();

    void addPlugin(GejengelPlugin& plugin);

    Settings                            m_Settings;
#ifdef HAVE_LIBUPNP
    UPnPServerSettings                  m_ServerSettings;
#endif
    LibraryAccess						m_LibraryAccess;
    AlbumArtGrabber						m_AlbumArtProvider;
    std::unique_ptr<PlayQueue>          m_pPlayQueue;
    std::unique_ptr<PlaybackEngine>     m_pPlayback;
    PluginManager                       m_PluginMgr;
    std::unique_ptr<MainWindow>         m_pMainWindow;
    Track                               m_CurrentTrack;
    uint32_t                            m_CurrentProgress;
    FetchStatus                         m_FetchStatus;
};

}

#endif
