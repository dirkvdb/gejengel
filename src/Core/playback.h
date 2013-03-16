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

#ifndef PLAYBACK_H
#define PLAYBACK_H

#include <vector>
#include <mutex>
#include <thread>
#include <condition_variable>
#include <sigc++/connection.h>

#include "playbackengine.h"
#include "audio/audioframe.h"
#include "MusicLibrary/track.h"
#include "utils/types.h"


//#define DUMP_TO_WAVE
//#include <fstream>

namespace audio
{
    class IRenderer;
    class IDecoder;
}

namespace Gejengel
{

class GejengelCore;

class Playback : public PlaybackEngine
{
public:
    Playback(GejengelCore& core);
    virtual ~Playback();

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
    PlaybackState getState();

    void setVolume(int32_t volume);
    int32_t getVolume() const;
    const Track& getTrack() const;

private:
    void stopPlayback(bool drain);
    bool startNewTrack();
    void sendProgressIfNeeded();
    void playback();
    void playbackLoop();
    bool rendererHasSpace(uint32_t dataSize);

    std::unique_ptr<audio::IDecoder>        m_pAudioDecoder;
    std::unique_ptr<audio::IRenderer>       m_pAudioRenderer;

    std::thread             m_PlaybackThread;
    std::condition_variable m_PlaybackCondition;
    std::mutex              m_PlaybackMutex;
    std::recursive_mutex    m_DecodeMutex;

    Track                   m_CurrentTrack;
    bool                    m_Destroy;
    bool                    m_Stop;
    bool                    m_NewTrackStarted;
    PlaybackState           m_State;
    bool                    m_SkipTrack;
    bool                    m_SeekOccured;
    double                  m_CurrentPts;
    audio::Frame            m_AudioFrame;

#ifdef DUMP_TO_WAVE
    std::ofstream           m_WaveFile;
    uint32_t                m_WaveBytes;

    void writeWaveHeader();
    void updateWaveHeaderSize();
    void dumpToWav(AudioFrame& frame);
#endif
};

}

#endif
