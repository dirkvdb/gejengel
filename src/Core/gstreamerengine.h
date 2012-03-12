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

#ifndef GSTREAMER_ENGINE_H
#define GSTREAMER_ENGINE_H

#include <list>

#include "utils/types.h"
#include "playbackengine.h"
#include "MusicLibrary/track.h"

#include <gstreamermm/playbin2.h>

namespace Gejengel
{

class GejengelCore;
class AudioRenderer;

class GStreamerEngine : public PlaybackEngine
{
public:
    GStreamerEngine(GejengelCore& core);
    virtual ~GStreamerEngine();

    void play();
    void pause();
    void resume();
    void stop();
    void prev();
    void next();
    bool isPaused();
    bool isPlaying();

    void seek(double seconds);
    double getCurrentTime();
    double getDuration();

    void setVolume(int32_t volume);
    int32_t getVolume() const;

    const Track& getTrack() const;

private:
    bool startNewTrack();
    bool onBusMessage(const Glib::RefPtr<Gst::Bus>& bus, const Glib::RefPtr<Gst::Message>& message);
    bool onTimeout();
    void onAboutTofinish();
    Gst::State getState();
    bool setState(Gst::State state);
    static std::string getStateString(Gst::State state);

    Track                       m_CurrentTrack;
    Glib::RefPtr<Gst::PlayBin2> m_PlayBin;
    guint                       m_WatchId;
    sigc::connection            m_TimeoutConnection;
};

}

#endif
