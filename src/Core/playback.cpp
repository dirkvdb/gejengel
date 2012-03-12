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

#include "playback.h"

#include <iomanip>
#include <cmath>
#include <cassert>
#include <cstring>

#include "AudioDecoder/audiodecoder.h"
#include "AudioDecoder/audiodecoderfactory.h"
#include "AudioRenderer/audiorenderer.h"
#include "AudioRenderer/audioformat.h"
#include "AudioRenderer/audioframe.h"
#include "AudioRenderer/audiorendererfactory.h"
#include "MusicLibrary/track.h"
#include "MusicLibrary/album.h"
#include "utils/timeoperations.h"
#include "utils/numericoperations.h"
#include "utils/log.h"
#include "utils/trace.h"
#include "gejengel.h"
#include "gejengelplugin.h"
#include "settings.h"

using namespace std;
using namespace utils;

namespace Gejengel
{

Playback::Playback(GejengelCore& core)
: PlaybackEngine(core)
, m_Destroy(false)
, m_Stop(false)
, m_NewTrackStarted(false)
, m_State(Stopped)
, m_SkipTrack(false)
, m_SeekOccured(false)
, m_CurrentPts(0)
{
    try
    {
        m_pAudioRenderer.reset(AudioRendererFactory::create(core.getSettings().get("AudioBackend"), *this));
    }
    catch (std::exception&)
    {
        log::error("Failed to create audio renderer, sound is disabled");
    }
    
    utils::trace("Create Playback");
    m_PlaybackThread = std::thread(&Playback::playbackLoop, this);
        

#ifdef DUMP_TO_WAVE
    log::critical("Dumping to wave is enabled, we are debugging right?");
    writeWaveHeader();
#endif
}


Playback::~Playback()
{
    stop();
    m_Destroy = true;

#ifdef DUMP_TO_WAVE
    updateWaveHeaderSize();
#endif

    m_PlaybackCondition.notify_all();

    if (m_PlaybackThread.joinable())
    {
        log::debug("Waiting for playback thread");
        m_PlaybackThread.join();
        log::debug("Done waiting");
    }
}

bool Playback::startNewTrack()
{
    if (!m_Core.getNextTrack(m_CurrentTrack))
    {
        stopPlayback(true);
        return false;
    }

    m_CurrentPts = 0.0;

    {
        std::lock_guard<std::recursive_mutex> lock(m_DecodeMutex);
        try
        {
            log::info("Play track:", m_CurrentTrack.filepath);
            m_pAudioDecoder.reset(AudioDecoderFactory::create(m_CurrentTrack.filepath));
        }
        catch (logic_error& e)
        {
            log::error("Failed to play audio file:", e.what());
            m_pAudioDecoder.reset();
            return startNewTrack();
        }
    }

    uint32_t duration = static_cast<uint32_t>(m_pAudioDecoder->getDuration());
    if (duration != 0)
    {
        m_CurrentTrack.durationInSec = duration;
    }
    m_Core.dispatchNewTrackStarted();
    m_NewTrackStarted = true;

    AudioFormat format = m_pAudioDecoder->getAudioFormat();
    if (m_pAudioRenderer)
    {
        m_pAudioRenderer->setFormat(format);
    }
    else
    {
        return false;
    }

    return true;
}

void Playback::playback()
{
    if (!startNewTrack())
    {
        return;
    }

    m_pAudioRenderer->play();
    
    bool frameDecoded = false;
    bool frameConsumed = true;

    while (!m_Stop)
    {
        
        if (m_pAudioDecoder && frameConsumed)
        {
            std::lock_guard<std::recursive_mutex> lock(m_DecodeMutex);
            frameDecoded = m_pAudioDecoder->decodeAudioFrame(m_AudioFrame);
            frameConsumed = !frameDecoded;
        }
        
        while (frameDecoded && !m_Stop && !isPaused() && rendererHasSpace(m_AudioFrame.getDataSize()))
        {
            {
                std::lock_guard<std::mutex> lock(m_PlaybackMutex);
                if (m_SkipTrack)
                {
                    if (!startNewTrack())
                    {
                        return;
                    }
                    m_pAudioRenderer->play();
                    m_SkipTrack = false;
                }

                if (isPaused())
                {
                    break;
                }

                m_pAudioRenderer->queueFrame(m_AudioFrame);
                frameConsumed = true;
                
                #ifdef DUMP_TO_WAVE
                dumpToWav(m_AudioFrame);
                #endif
            }

            sendProgressIfNeeded();
            
            {
                std::lock_guard<std::recursive_mutex> lock(m_DecodeMutex);
                frameDecoded = m_pAudioDecoder->decodeAudioFrame(m_AudioFrame);
                frameConsumed = !frameDecoded;
            }
        }
        
        if (!frameDecoded && !startNewTrack())
        {
            log::debug("Stop it", frameDecoded, frameConsumed);
            return;
        }

        m_pAudioRenderer->flushBuffers();

        if (m_State == Playing && !m_pAudioRenderer->isPlaying())
        {
            log::debug("Kick renderer");
            std::lock_guard<std::mutex> lock(m_PlaybackMutex);
            m_pAudioRenderer->play();
        }

        timeops::sleepMs(50);
    }
}

bool Playback::rendererHasSpace(uint32_t dataSize)
{
    if (m_pAudioRenderer)
    {
        std::lock_guard<std::mutex> lock(m_PlaybackMutex);
        return m_pAudioRenderer->hasBufferSpace(dataSize);
    }

    return false;
}

void Playback::sendProgressIfNeeded()
{
    double pts = m_pAudioRenderer->getCurrentPts();

    if (pts <= 2.0 && m_NewTrackStarted)
    {
        m_NewTrackStarted = false;
    }

    if (static_cast<int>(pts) == 0 || m_NewTrackStarted)
    {
        //avoid sending false progress after a seek, or track that has just been started
        return;
    }

    if ((static_cast<int>(pts) != static_cast<int>(m_CurrentPts)))
    {
        m_CurrentPts = pts;
        m_Core.dispatchProgress();
    }
}

void Playback::play()
{
    if (!m_pAudioRenderer || m_State == Playing || m_State == Paused)
    {
        return;
    }

    std::lock_guard<std::mutex> lock(m_PlaybackMutex);
    m_PlaybackCondition.notify_all();
    m_State = Playing;
}

void Playback::pause()
{
    if (m_pAudioRenderer)
    {
        std::lock_guard<std::mutex> lock(m_PlaybackMutex);
        m_pAudioRenderer->pause();
        m_State = Paused;
    }
}

void Playback::resume()
{
    if (!m_pAudioRenderer)
    {
        return;
    }

    if (isPaused())
    {
        std::lock_guard<std::mutex> lock(m_PlaybackMutex);
        m_SeekOccured ? m_pAudioRenderer->play() : m_pAudioRenderer->resume();
        m_State = Playing;
    }
    else
    {
        log::debug("Not in paused state, ignore resume");
    }
}

void Playback::stop()
{
    stopPlayback(false);
}

void Playback::stopPlayback(bool drain)
{
    if (m_pAudioRenderer)
    {
        m_CurrentTrack = Track();
        m_CurrentPts = 0.0;
        m_Stop = true;

        std::lock_guard<std::mutex> lock(m_PlaybackMutex);
        m_pAudioRenderer->stop(drain);
        m_State = Stopped;
        m_SeekOccured = false;
        m_NewTrackStarted = false;
    }
}

void Playback::next()
{
    if (!m_pAudioRenderer || m_SkipTrack || m_State == Stopped)
    {
        //previous skip not processed yet or we are stopped
        return;
    }

    std::lock_guard<std::mutex> lock(m_PlaybackMutex);

    m_SkipTrack = true;
    m_pAudioRenderer->stop(false);
    m_pAudioRenderer->flushBuffers();
    m_State = Playing;

    m_CurrentPts = 0.0;
    m_Core.dispatchProgress();
}

void Playback::prev()
{
}

bool Playback::isPaused()
{
    return m_State == Paused;
}

bool Playback::isPlaying()
{
    return m_pAudioRenderer ? m_pAudioRenderer->isPlaying() : false;
}

void Playback::seek(double seconds)
{
    if (!m_pAudioDecoder || !m_pAudioRenderer)
    {
        return;
    }

    std::lock_guard<std::mutex> playbackLock(m_PlaybackMutex);
    m_pAudioRenderer->stop(false);
    m_pAudioRenderer->flushBuffers();

    {
        std::lock_guard<std::recursive_mutex> decodeLock(m_DecodeMutex);
        m_pAudioDecoder->seekAbsolute(static_cast<double>(seconds));
    }

    if (!isPaused())
    {
        m_pAudioRenderer->play();
    }
    else
    {
        m_SeekOccured = true;
    }

    m_NewTrackStarted = false;
}

double Playback::getCurrentTime()
{
    return m_CurrentPts;
}

double Playback::getDuration()
{
    return m_pAudioDecoder ? static_cast<double>(m_pAudioDecoder->getDuration()) : 0.0;
}

PlaybackState Playback::getState()
{
    return m_State;
}

void Playback::setVolume(int32_t volume)
{
    if (m_pAudioRenderer)
    {
        std::lock_guard<std::mutex> lock(m_PlaybackMutex);
        m_pAudioRenderer->setVolume(volume);
        m_Core.dispatchVolumeChanged();
    }
}

int32_t Playback::getVolume() const
{
    return m_pAudioRenderer ? m_pAudioRenderer->getVolume() : 100;
}

const Track& Playback::getTrack() const
{
    return m_CurrentTrack;
}

void Playback::playbackLoop()
{
    while (!m_Destroy)
    {
        {
            log::debug("Wait", m_Destroy);
            std::unique_lock<std::mutex> lock(m_PlaybackMutex);
            m_PlaybackCondition.wait(lock);
            log::debug("Condition signaled playback= " + numericops::toString(m_Stop));
        }

        if (m_Destroy)
        {
            break;
        }

        try
        {
            m_Stop = m_SkipTrack = false;
            playback();
        }
        catch (exception& e)
        {
            log::error(string("Playback error: ") + e.what());
        }
    }
}

#ifdef DUMP_TO_WAVE

void Playback::writeWaveHeader()
{
    m_WaveFile.open("/tmp/dump.wav", ios_base::binary);
    if (!m_WaveFile.is_open())
    {
        return;
    }

    uint16_t bps = 16;
    uint32_t subchunk1Size = 16; //16 for PCM
    uint16_t audioFormat = 1; //PCM
    uint16_t channels = 2;
    uint32_t sampleRate = 44100;
    uint32_t byteRate = 44100 * 2 * (bps / 8);
    uint16_t blockAlign = 2 * (bps / 8);

    //don't know yet
    m_WaveBytes = 0;

    m_WaveFile.write("RIFF", 4);
    m_WaveFile.write((char*) &m_WaveBytes, 4);
    m_WaveFile.write("WAVEfmt ", 8);
    m_WaveFile.write((char*) &subchunk1Size, 4);
    m_WaveFile.write((char*) &audioFormat, 2);
    m_WaveFile.write((char*) &channels, 2);
    m_WaveFile.write((char*) &sampleRate, 4);
    m_WaveFile.write((char*) &byteRate, 4);
    m_WaveFile.write((char*) &blockAlign, 2);
    m_WaveFile.write((char*) &bps, 2);
    m_WaveFile.write("data", 4);
    m_WaveFile.write((char*) &m_WaveBytes, 4);
}

void Playback::updateWaveHeaderSize()
{
    m_WaveFile.seekp(4, ios_base::beg);
    m_WaveFile.write((char*) &m_WaveBytes + 36, 4);

    m_WaveFile.seekp(40, ios_base::beg);
    m_WaveFile.write((char*) &m_WaveBytes, 4);

    m_WaveFile.close();
}

void Playback::dumpToWav(AudioFrame& frame)
{
    assert(m_WaveFile.is_open());
    m_WaveFile.write(reinterpret_cast<char*>(frame.getFrameData()), frame.getDataSize());
    m_WaveBytes += frame.getDataSize();
}

#endif
}
