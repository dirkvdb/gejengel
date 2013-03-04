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

#include "pulserenderer.h"

#include "audioframe.h"
#include "audioformat.h"
#include "utils/numericoperations.h"
#include "utils/log.h"
#include "Core/playbackengine.h"

#include "config.h"

#include <cassert>
#include <stdexcept>
#include <string>
#include <cstring>


using namespace std;
using namespace utils;

namespace Gejengel
{

PulseRenderer::PulseRenderer(PlaybackEngine& engine)
: AudioRenderer()
, m_Engine(engine)
, m_pPulseContext(nullptr)
, m_pPulseLoop(nullptr)
, m_pMainloopApi(nullptr)
, m_pStream(nullptr)
, m_VolumeInt(0)
, m_IsPlaying(false)
, m_LastPts(0.0)
, m_Latency(0)
, m_FrameSize(0)
, m_Buffer(256 * 1024)
{
    memset(&m_SampleFormat, 0, sizeof(pa_sample_spec));
    m_pPulseLoop = pa_threaded_mainloop_new();

    if (!m_pPulseLoop)
    {
        throw logic_error("PulseRenderer: Failed to create mainloop");
    }
    m_pMainloopApi = pa_threaded_mainloop_get_api(m_pPulseLoop);
    assert(m_pMainloopApi);

    if (!(m_pPulseContext = pa_context_new(m_pMainloopApi, PACKAGE_NAME)))
    {
        throw logic_error("PulseRenderer: Failed to create new context");
    }

    pa_channel_map_init_auto (&m_ChannelMap, 2, PA_CHANNEL_MAP_DEFAULT);
    pa_context_set_state_callback(m_pPulseContext, PulseRenderer::contextStateCb, this);

    if (pa_context_connect(m_pPulseContext, nullptr, static_cast<pa_context_flags>(0), nullptr) < 0)
    {
        throw logic_error(string("PulseRenderer: failed to connect to the server(") + pa_strerror(pa_context_errno(m_pPulseContext)) + ")");
    }

    pa_threaded_mainloop_start(m_pPulseLoop);
}

PulseRenderer::~PulseRenderer()
{
    log::debug("Destroy Pulse renderer");
    stop(false);
    log::debug("Destroy stopped");
    pa_threaded_mainloop_stop(m_pPulseLoop);
    pa_context_disconnect(m_pPulseContext);
    pa_threaded_mainloop_free(m_pPulseLoop);
    log::debug("Destroy Pulse renderer finished");
}

void PulseRenderer::setFormat(const AudioFormat& format)
{
    assert(m_pPulseContext);

    if (m_AudioFormat == format)
    {
        return;
    }

    stop(true);
    log::debug("Format has changed");

    m_SampleFormat.rate = format.rate;
    m_SampleFormat.channels = format.numChannels;

    switch (format.bits)
    {
    case 8:
        m_SampleFormat.format = PA_SAMPLE_U8;
        m_FrameSize = format.numChannels;
        break;
    case 16:
        m_SampleFormat.format = PA_SAMPLE_S16NE;
        m_FrameSize = format.numChannels * 2;
        break;
    case 32:
        m_SampleFormat.format = PA_SAMPLE_FLOAT32NE;
        m_FrameSize = format.numChannels * 4;
        break;
    default:
        throw logic_error("PulseRenderer: unsupported format");
    }

    if (!pa_sample_spec_valid(&m_SampleFormat))
    {
        throw logic_error("PulseRenderer: Invalid sample specification");
    }

    m_AudioFormat = format;
}

bool PulseRenderer::hasBufferSpace(uint32_t dataSize)
{
    if (!m_pStream || !m_IsPlaying)
    {
        return false;
    }

    return dataSize <= m_Buffer.bytesFree();
}

bool PulseRenderer::pulseIsReady()
{
    pa_threaded_mainloop_lock(m_pPulseLoop);
    bool ready = pa_context_get_state(m_pPulseContext) == PA_CONTEXT_READY;
    pa_threaded_mainloop_unlock(m_pPulseLoop);
    return ready;
}

void PulseRenderer::queueFrame(const AudioFrame& frame)
{
    m_Buffer.writeData(frame.getFrameData(), frame.getDataSize());
    m_LastPts = frame.getPts();
}

void PulseRenderer::flushBuffers()
{
    if (m_IsPlaying && m_pStream)
    {
        pa_threaded_mainloop_lock(m_pPulseLoop);
        size_t available = pa_stream_writable_size(m_pStream);
        if (available > 0 && m_Buffer.bytesUsed() > 0)
        {
            uint32_t size = available;
            uint8_t* pData = m_Buffer.getData(size);
            pa_stream_write(m_pStream, reinterpret_cast<const void*>(pData), size, nullptr, 0, PA_SEEK_RELATIVE);
        }
        pa_threaded_mainloop_unlock(m_pPulseLoop);
    }
}

bool PulseRenderer::isPlaying()
{
    return m_IsPlaying;
}

void PulseRenderer::play()
{
    if (m_pStream == nullptr && pulseIsReady() && !isPlaying() && (m_SampleFormat.rate != 0))
    {
        pa_threaded_mainloop_lock(m_pPulseLoop);
        m_pStream = pa_stream_new(m_pPulseContext, "Music playback", &m_SampleFormat, nullptr);
        assert(m_pStream);

        pa_stream_set_state_callback(m_pStream, PulseRenderer::streamStateCb, this);
        pa_stream_set_underflow_callback(m_pStream, PulseRenderer::streamUnderflowCb, this);
        if (pa_stream_connect_playback(m_pStream, nullptr, nullptr, static_cast<pa_stream_flags_t>(0), pa_cvolume_set(&m_Volume, m_SampleFormat.channels, m_VolumeInt * PA_VOLUME_NORM / 100), nullptr))
        {
            throw logic_error("Failed to start pulseaudio playback");
        }

        for (;;)
        {
            pa_threaded_mainloop_wait(m_pPulseLoop);
            pa_stream_state_t state = pa_stream_get_state(m_pStream);
            if (state == PA_STREAM_READY)
            {
                writeSilentData();
                m_IsPlaying = true;
                break;
            }
            else if (state == PA_STREAM_FAILED || state == PA_STREAM_TERMINATED)
            {
                m_IsPlaying = false;
                pa_threaded_mainloop_unlock(m_pPulseLoop);
                throw logic_error("Failed to start pulseaudio playback");
            }
        }

        pa_threaded_mainloop_unlock(m_pPulseLoop);
    }
}

void PulseRenderer::pause()
{
    pa_threaded_mainloop_lock(m_pPulseLoop);
    pa_operation* pOp = pa_stream_cork(m_pStream, 1, nullptr, nullptr);
    if (!pOp)
    {
        pa_threaded_mainloop_unlock(m_pPulseLoop);
        throw logic_error("Pulseaudio: Error pausing playback");
    }
    pa_operation_unref(pOp);

    m_IsPlaying = false;
    pa_threaded_mainloop_unlock(m_pPulseLoop);
}

void PulseRenderer::resume()
{
    pa_threaded_mainloop_lock(m_pPulseLoop);
    pa_operation* pOp = pa_stream_cork(m_pStream, 0, PulseRenderer::streamSuccessCb, this);
    if (!pOp)
    {
        pa_threaded_mainloop_unlock(m_pPulseLoop);
        throw logic_error("Pulseaudio: Error pausing playback");
    }

    pa_threaded_mainloop_wait(m_pPulseLoop);
    pa_operation_unref(pOp);

    pOp = pa_stream_trigger(m_pStream, nullptr, nullptr);
    if (!pOp)
    {
        pa_threaded_mainloop_unlock(m_pPulseLoop);
        throw logic_error("Pulseaudio: Error pausing playback");
    }

    pa_operation_unref(pOp);

    m_IsPlaying = true;
    pa_threaded_mainloop_unlock(m_pPulseLoop);
}

void PulseRenderer::stop(bool drain)
{
    if (m_pStream && m_IsPlaying)
    {
        pa_threaded_mainloop_lock(m_pPulseLoop);
        pa_stream_disconnect(m_pStream);
        pa_stream_unref(m_pStream);
        m_pStream = nullptr;
        pa_threaded_mainloop_unlock(m_pPulseLoop);
        m_IsPlaying = false;
    }

    m_Buffer.clear();
}

void PulseRenderer::setVolume(int volume)
{
    numericops::clip(volume, 0, 100);

    if (volume == m_VolumeInt)
    {
        return;
    }

    m_VolumeInt = volume;
    if (m_pStream)
    {
        pa_cvolume_set(&m_Volume, m_SampleFormat.channels == 0 ? 2 : m_SampleFormat.channels, volume * PA_VOLUME_NORM / 100);
        pa_operation* pOp = pa_context_set_sink_input_volume(m_pPulseContext, pa_stream_get_index(m_pStream), &m_Volume, nullptr, nullptr);
        if (!pOp)
        {
            log::warn("Pulseaudio: Failed to set sink info");
            return;
        }
        pa_operation_unref(pOp);
    }
}

void PulseRenderer::contextSubscriptionCb(pa_context* pContext, pa_subscription_event_type_t type, uint32_t index, void* pData)
{
    if ((type & PA_SUBSCRIPTION_EVENT_CHANGE) && (type & PA_SUBSCRIPTION_EVENT_SINK_INPUT))
    {
        pa_operation* pOp = pa_context_get_sink_input_info(pContext, index, PulseRenderer::sinkInputInfoCb, pData);
        if (!pOp)
        {
            log::warn("Pulseaudio: Failed to get sink info");
            return;
        }
        pa_operation_unref(pOp);
    }
}

void PulseRenderer::sinkInputInfoCb(pa_context* pContext, const pa_sink_input_info* pInfo, int eol, void* pData)
{
    if (eol < 0)
    {
        if (pa_context_errno(pContext) == PA_ERR_NOENTITY)
        {
            return;
        }
        log::error("Pulseaudio: Sink callback failure");
        return;
    }

    PulseRenderer* pRenderer = reinterpret_cast<PulseRenderer*>(pData);
    if (pInfo && (pInfo->index == pa_stream_get_index(pRenderer->m_pStream)))
    {
        if (!pa_cvolume_equal(&pInfo->volume, &pRenderer->m_Volume) && pInfo->mute == 0)
        {
            pRenderer->m_Engine.setVolume((pa_cvolume_avg(&pInfo->volume) * 100) / PA_VOLUME_NORM);
        }
        else if (pInfo->mute == 1 && pRenderer->m_VolumeInt != 0)
        {
            pRenderer->m_Engine.setVolume(0);
        }
        else if (pInfo->mute == 0 && pRenderer->m_VolumeInt == 0)
        {
            pRenderer->m_Engine.setVolume((pa_cvolume_avg(&pInfo->volume) * 100) / PA_VOLUME_NORM);
        }
    }
}

int PulseRenderer::getVolume()
{
    return m_VolumeInt;
}

void PulseRenderer::streamUpdateTimingCb(pa_stream* pStream, int success, void* pData)
{
    PulseRenderer* pRenderer = reinterpret_cast<PulseRenderer*>(pData);

    int negative;
    if (pa_stream_get_latency(pStream, &pRenderer->m_Latency, &negative) < 0)
    {
        log::debug("Failed to get latency: %s", pa_strerror(pa_context_errno(pRenderer->m_pPulseContext)));
        pRenderer->m_Latency = 0;
    }
}

double PulseRenderer::getCurrentPts()
{
    pa_threaded_mainloop_lock(m_pPulseLoop);

    assert(m_pStream);
    pa_operation* pOp = pa_stream_update_timing_info(m_pStream, streamUpdateTimingCb, this);
    if (pOp)
    {
        pa_operation_unref(pOp);
    }

    pa_threaded_mainloop_unlock(m_pPulseLoop);
    double bufferDelay = m_Buffer.bytesUsed() / static_cast<double>(m_FrameSize * m_AudioFormat.rate);
    return std::max(0.0, m_LastPts - (m_Latency / 1000000.0) - bufferDelay);
}

void PulseRenderer::contextStateCb(pa_context* pContext, void* pData)
{
    switch (pa_context_get_state(pContext))
    {
    case PA_CONTEXT_CONNECTING:
    case PA_CONTEXT_AUTHORIZING:
    case PA_CONTEXT_SETTING_NAME:
        break;
    case PA_CONTEXT_READY:
    {
        log::debug("PulseRenderer: Ready for action");

        PulseRenderer* pRenderer = reinterpret_cast<PulseRenderer*>(pData);
        pa_context_set_subscribe_callback(pRenderer->m_pPulseContext, PulseRenderer::contextSubscriptionCb, pRenderer);

        pa_operation* op = pa_context_subscribe(pRenderer->m_pPulseContext, PA_SUBSCRIPTION_MASK_SINK_INPUT, nullptr, nullptr);
        if (!op)
        {
            log::warn("Pulseaudio: context subscription failed");
            return;
        }
        pa_operation_unref(op);
        break;
    }
    case PA_CONTEXT_TERMINATED:
        log::debug("PulseRenderer: Terminated");
        break;
    case PA_CONTEXT_FAILED:
    default:
        log::debug("PulseRenderer: Failed to connect to server");
    }
}

void PulseRenderer::streamStateCb(pa_stream* pStream, void* pData)
{
    switch (pa_stream_get_state(pStream))
    {
    case PA_STREAM_TERMINATED:
    case PA_STREAM_READY:
    case PA_STREAM_FAILED:
    {
        PulseRenderer* pRenderer = reinterpret_cast<PulseRenderer*>(pData);
        pa_threaded_mainloop_signal(pRenderer->m_pPulseLoop, 0);
        break;
    }
    default:
        break;
    }
}

void PulseRenderer::streamSuccessCb(pa_stream* pStream, int success, void* pData)
{
    PulseRenderer* pRenderer = reinterpret_cast<PulseRenderer*>(pData);
    pa_threaded_mainloop_signal(pRenderer->m_pPulseLoop, 0);
}

void PulseRenderer::writeSilentData()
{
    pa_threaded_mainloop_lock(m_pPulseLoop);
    size_t available = pa_stream_writable_size(m_pStream);
    vector<uint8_t> buf(available, 0);
    pa_stream_write(m_pStream, reinterpret_cast<const void*>(&buf.front()), available, nullptr, 0, PA_SEEK_RELATIVE);
    pa_threaded_mainloop_unlock(m_pPulseLoop);
}

void PulseRenderer::streamUnderflowCb(pa_stream* pStream, void* pData)
{
    assert(pStream);
    PulseRenderer* pRenderer = reinterpret_cast<PulseRenderer*>(pData);
    log::debug("PulseRenderer: XRUN %d %d", pa_stream_writable_size(pStream), pRenderer->m_Buffer.bytesUsed());
}

}
