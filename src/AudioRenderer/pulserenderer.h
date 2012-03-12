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

#ifndef PULSE_RENDERER_H
#define PULSE_RENDERER_H

#include <deque>
#include <vector>
#include <pulse/pulseaudio.h>

#include "audiobuffer.h"
#include "audioformat.h"
#include "audiorenderer.h"


namespace Gejengel
{

class PlaybackEngine;
class AudioFrame;
struct AudioFormat;


class PulseRenderer : public AudioRenderer
{
public:
    PulseRenderer(PlaybackEngine& engine);
    virtual ~PulseRenderer();

    void setFormat(const AudioFormat& format);
    
    double getCurrentPts();
    void play();
    void pause();
    void resume();
    void stop(bool drain);
    void setVolume(int volume);
    int getVolume();

    bool isPlaying();

    bool hasBufferSpace(uint32_t dataSize);
    void flushBuffers();
    void queueFrame(const AudioFrame& frame);

private:
    static void contextStateCb(pa_context* pContext, void* pData);
    static void contextSubscriptionCb(pa_context* pContext, pa_subscription_event_type_t type, uint32_t index, void* pData);
    static void streamStateCb(pa_stream* pStream, void* pData);
    static void streamUnderflowCb(pa_stream* pStream, void* pData);
    static void sinkInputInfoCb(pa_context* pContext, const pa_sink_input_info* pInfo, int eol, void* pData);
    static void streamSuccessCb(pa_stream* pStream, int success, void* pData);
    static void streamUpdateTimingCb(pa_stream* pStream, int success, void* pData);

    void writeSilentData();

    bool pulseIsReady();

    PlaybackEngine&             m_Engine;
    pa_context*                 m_pPulseContext;
    pa_threaded_mainloop*       m_pPulseLoop;
    pa_mainloop_api*            m_pMainloopApi;
    pa_stream*                  m_pStream;
    pa_channel_map              m_ChannelMap;
    pa_sample_spec              m_SampleFormat;
    AudioFormat                 m_AudioFormat;
    pa_cvolume                  m_Volume;
    int                         m_VolumeInt;
    bool                        m_IsPlaying;

    double                      m_LastPts;
    pa_usec_t                   m_Latency;
    uint32_t                    m_FrameSize;
    
    AudioBuffer                 m_Buffer;
};

}

#endif
