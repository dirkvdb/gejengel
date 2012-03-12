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

#ifndef GEJENGEL_CORE_H
#define GEJENGEL_CORE_H

#include "config.h"
#include "gejengeltypes.h"
#include "utils/types.h"
#include "utils/subscriber.h"

namespace Gejengel
{

class Track;
class Album;
class PlayQueue;
class Settings;
class PluginManager;
class IAlbumArtProvider;
class IStatusReporter;
class MusicLibrary;
class UPnPServerSettings;
class LibraryAccess;

class IGejengelCore
{
public:
    virtual void play() = 0;
    virtual void pause() = 0;
    virtual void resume() = 0;
    virtual void prev() = 0;
    virtual void next() = 0;
    virtual void stop() = 0;
    virtual PlaybackState getPlaybackState() = 0;

    virtual void seek(double seconds) = 0;
    virtual double getTrackPosition() = 0;

    virtual void setVolume(int32_t volume) = 0;
    virtual int32_t getVolume() = 0;

    virtual void getCurrentTrack(Track& track) = 0;

    virtual PlayQueue& getPlayQueue() = 0;
    virtual Settings& getSettings() = 0;
#ifdef HAVE_LIBUPNP
    virtual UPnPServerSettings& getUPnPServerSettings() = 0;
#endif
    virtual PluginManager& getPluginManager() = 0;
    virtual IAlbumArtProvider& getAlbumArtProvider() = 0;
    virtual IStatusReporter& getStatusReporter() = 0;
    virtual LibraryAccess& getLibraryAccess() = 0;

    virtual void quitApplication() = 0;
    virtual void showHideWindow() = 0;
};

}

#endif
