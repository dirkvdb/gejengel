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

#ifndef ALSA_RENDERER_H
#define ALSA_RENDERER_H

#include "audiobuffer.h"
#include "audioformat.h"
#include "audiorenderer.h"
#include <alsa/asoundlib.h>
#include <string>
#include <deque>

namespace Gejengel
{

class AlsaRenderer : public AudioRenderer
{
public:
    AlsaRenderer();
    ~AlsaRenderer();

    void setFormat(const AudioFormat& format);

    void play();
    void pause();
    void resume();
    void stop(bool drain);
    void setVolume(int volume);
    int getVolume();

    bool hasBufferSpace(uint32_t dataSize);
    void flushBuffers();
    void queueFrame(const AudioFrame& frame);

    double getCurrentPts();

    bool isPlaying();

private:
    void throwOnError(int err, const std::string& message);
    snd_pcm_state_t getDeviceStatus();
    std::string getDeviceStatusString(snd_pcm_state_t status);
    void setHardwareParams(snd_pcm_format_t format, uint32_t channels, uint32_t rate);
    void setSoftwareParams();
    void applyVolume(const AudioFrame& frame);

    snd_pcm_t*              m_pAudioDevice;
    snd_pcm_uframes_t       m_BufferSize;
    snd_pcm_uframes_t       m_PeriodSize;
    uint32_t                m_BufferTime;
    uint32_t                m_PeriodTime;

    AudioFormat             m_Format;
    int                     m_Volume;

    int                     m_FrameSize;
    double                  m_LastPts;

    bool                    m_SupportPause;

    AudioBuffer             m_Buffer;
};

}

#endif //ALSA_RENDERER_H
