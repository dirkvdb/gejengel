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

#include "gejengel.h"

#include "ui/mainwindow.h"
#include "utils/log.h"
#include "utils/timeoperations.h"
#include "utils/numericoperations.h"
#include "MusicLibrary/musiclibrary.h"
#include "MusicLibrary/subscribers.h"
#include "playqueue.h"
#include "playbackengine.h"
#include "playbackenginefactory.h"

#include <glibmm/i18n.h>

using namespace std;
using namespace utils;

namespace Gejengel
{

GejengelCore::GejengelCore()
: m_LibraryAccess(m_Settings)
, m_AlbumArtProvider(*this)
, m_pPlayQueue(new PlayQueue(*this))
, m_PluginMgr(*this)
, m_CurrentProgress(0)
, m_FetchStatus(FetchStarted)
{
    loadSettings();
    m_pMainWindow.reset(new MainWindow(*this));

    dispatchProgress.connect(sigc::mem_fun(this, &GejengelCore::sendProgress));
    dispatchVolumeChanged.connect(sigc::mem_fun(this, &GejengelCore::sendVolumeChange));
    dispatchStop.connect(sigc::mem_fun(&m_PluginMgr, &PluginManager::sendStop));
    dispatchNewTrackStarted.connect(sigc::mem_fun(this, &GejengelCore::newTrackStarted));
    dispatchGetNextTrack.connect(sigc::mem_fun(this, &GejengelCore::sendGetNextTrack));

    m_pPlayQueue->loadQueue();
}

GejengelCore::~GejengelCore()
{
    saveSettings();
    m_LibraryAccess.cleanup();
    
    m_PluginMgr.destroy();
    
    // deletion order matters!
    m_pMainWindow.reset();
    m_pPlayQueue.reset();
    m_pPlayback.reset();
}

void GejengelCore::loadSettings()
{
    m_pPlayback.reset(PlaybackEngineFactory::create(m_Settings.get("PlaybackEngine"), *this));
    m_pPlayback->setVolume(m_Settings.getAsInt("PlaybackVolume", 100));
    
    LibraryType libraryType = static_cast<LibraryType>(m_Settings.getAsInt("LibraryType", Local));
    m_LibraryAccess.setLibraryType(libraryType);
}

void GejengelCore::saveSettings()
{
    m_Settings.set("PlaybackVolume", m_pPlayback->getVolume());
    m_Settings.set("LibraryType", m_LibraryAccess.getLibraryType());

    if (m_Settings.getAsBool("SaveQueueOnExit", false))
    {
        m_pPlayQueue->saveQueue();
    }
}

void GejengelCore::run()
{
    m_pMainWindow->run();
}

void GejengelCore::play()
{
    if (!m_pPlayback->isPlaying())
    {
        try
        {
            m_pPlayback->play();
        }
        catch(exception& e)
        {
            log::error("GejengelCore::play Failed to start playback: %s", e.what());
        }
    }
}

void GejengelCore::pause()
{
    if (m_pPlayback->isPlaying())
    {
        try
        {
            m_pPlayback->pause();
            m_PluginMgr.sendPause();
        }
        catch(exception& e)
        {
            log::error("GejengelCore::pause Failed to pause playback: %s", e.what());
        }
    }
}

void GejengelCore::resume()
{
    if (!m_pPlayback->isPlaying())
    {
        try
        {
            m_pPlayback->resume();
            m_PluginMgr.sendResume();
        }
        catch(exception& e)
        {
            log::error("GejengelCore::resume Failed to resume playback: %s", e.what());
        }
    }
}

void GejengelCore::prev()
{
    m_pPlayback->prev();
}

void GejengelCore::next()
{
    m_pPlayback->next();
}

void GejengelCore::stop()
{
    try
    {
        m_CurrentTrack = Track();
        m_pPlayback->stop();
        m_PluginMgr.sendStop();
    }
    catch(exception& e)
    {
        log::error(string("GejengelCore::stop Failed to stop playback: ") + e.what());
    }
}

PlaybackState GejengelCore::getPlaybackState()
{
    return m_pPlayback ? m_pPlayback->getState() : Stopped;
}

void GejengelCore::seek(double seconds)
{
    try
    {
        m_pPlayback->seek(seconds);
    }
    catch(exception& e)
    {
        log::error("GejengelCore::seek Failed to seek: %s", e.what());
    }
}

double GejengelCore::getTrackPosition()
{
    return m_pPlayback->getCurrentTime();
}

void GejengelCore::getCurrentTrack(Track& track)
{
    if (m_pPlayback)
    { 
        track = m_pPlayback->getTrack();
    }
}

void GejengelCore::setVolume(int32_t volume)
{
    if (m_pPlayback)
    {
        m_pPlayback->setVolume(volume);
    }
}

int32_t GejengelCore::getVolume()
{
    return m_pPlayback ? m_pPlayback->getVolume() : 0;
}

Settings& GejengelCore::getSettings()
{
    return m_Settings;
}

#ifdef HAVE_LIBUPNP
UPnPServerSettings& GejengelCore::getUPnPServerSettings()
{
    return m_ServerSettings;
}
#endif

PluginManager& GejengelCore::getPluginManager()
{
    return m_PluginMgr;
}

PlayQueue& GejengelCore::getPlayQueue()
{
    return *m_pPlayQueue;
}

IStatusReporter& GejengelCore::getStatusReporter()
{
    return *m_pMainWindow;
}

LibraryAccess& GejengelCore::getLibraryAccess()
{
	return m_LibraryAccess;
}

IAlbumArtProvider& GejengelCore::getAlbumArtProvider()
{
	return m_AlbumArtProvider;
}

void GejengelCore::quitApplication()
{
    m_pMainWindow->quit();
}

void GejengelCore::showHideWindow()
{
    m_pMainWindow->showHideWindow();
}

bool GejengelCore::getNextTrack(Track& track)
{
    m_FetchStatus = FetchStarted;
    dispatchGetNextTrack();

    while (m_FetchStatus == FetchStarted)
    {
        timeops::sleepMs(10);
    }

    if (m_FetchStatus == FetchSuccess)
    {
        track = m_CurrentTrack;
        return true;
    }

    dispatchStop();
    return false;
}

void GejengelCore::newTrackStarted()
{

    m_CurrentTrack = m_pPlayback->getTrack();
    m_PluginMgr.sendPlay(m_CurrentTrack);
}

void GejengelCore::sendGetNextTrack()
{
    m_FetchStatus = m_pPlayQueue->getNextTrack(m_CurrentTrack) ? FetchSuccess : FetchFailed;
}

void GejengelCore::sendProgress()
{
    m_PluginMgr.sendProgress(static_cast<int32_t>(m_pPlayback->getCurrentTime()));
}

void GejengelCore::sendVolumeChange()
{
    m_PluginMgr.sendVolumeChanged(m_pPlayback->getVolume());
}

}
