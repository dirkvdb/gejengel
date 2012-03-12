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

#include "ffmpegdecoder.h"
#include "AudioRenderer/audioframe.h"
#include "AudioRenderer/audioformat.h"
#include "utils/scopedlock.h"
#include "utils/log.h"

#include <cassert>
#include <stdexcept>
#include <algorithm>

#ifdef WIN32
#include "winconfig.h"
#endif

extern "C"
{
    #include <libavcodec/avcodec.h>
}

using namespace std;
using namespace utils;

namespace Gejengel
{

FFmpegDecoder::FFmpegDecoder(const std::string& filepath)
: AudioDecoder(filepath)
, m_AudioStream(-1)
, m_pFormatContext(nullptr)
, m_pAudioCodecContext(nullptr)
, m_pAudioCodec(nullptr)
, m_pAudioStream(nullptr)
, m_pAudioBuffer(nullptr)
, m_BytesPerFrame(0)
{
    log::debug("AVCodec Major:", LIBAVCODEC_VERSION_MAJOR);

    initialize();
}

FFmpegDecoder::~FFmpegDecoder()
{
    destroy();
}

void FFmpegDecoder::destroy()
{
    m_BytesPerFrame = 0;

    if (m_CurrentPacket.data)
    {
        av_free_packet(&m_CurrentPacket);
    }

    if (m_pAudioCodecContext)
    {
        avcodec_close(m_pAudioCodecContext);
        m_pAudioCodecContext = nullptr;
    }

    if (m_pFormatContext)
    {
        av_close_input_file(m_pFormatContext);
        m_pFormatContext = nullptr;
    }

    av_free(m_pAudioBuffer);
}

void FFmpegDecoder::initialize()
{
    avcodec_register_all();
    av_register_all();


#if LIBAVCODEC_VERSION_MAJOR < 53
    if (av_open_input_file(&m_pFormatContext, m_Filepath.c_str(), nullptr, 0, nullptr) != 0)
#else
    if (avformat_open_input(&m_pFormatContext, m_Filepath.c_str(), nullptr, nullptr) != 0)
#endif
    {
        throw logic_error("Could not open input file: " + m_Filepath);
    }

    if (av_find_stream_info(m_pFormatContext) < 0)
    {
        throw logic_error("Could not find stream information in " + m_Filepath);
    }

#ifdef ENABLE_DEBUG
#if LIBAVCODEC_VERSION_MAJOR < 53
    dump_format(m_pFormatContext, 0, m_Filepath.c_str(), false);
#else
    av_dump_format(m_pFormatContext, 0, m_Filepath.c_str(), false);
#endif
#endif

    initializeAudio();
}

void FFmpegDecoder::initializeAudio()
{
    for(uint32_t i = 0; i < m_pFormatContext->nb_streams; ++i)
    {
#if LIBAVCODEC_VERSION_MAJOR < 53
        if (m_pFormatContext->streams[i]->codec->codec_type == CODEC_TYPE_AUDIO)
#else
        if (m_pFormatContext->streams[i]->codec->codec_type == AVMEDIA_TYPE_AUDIO)
#endif
        {
            m_pAudioStream = m_pFormatContext->streams[i];
            m_AudioStream = i;
            break;
        }
    }

    if (m_AudioStream == -1)
    {
        throw logic_error("No audiostream found in " + m_Filepath);
    }

    m_pAudioCodecContext = m_pFormatContext->streams[m_AudioStream]->codec;
    m_pAudioCodec = avcodec_find_decoder(m_pAudioCodecContext->codec_id);

    if (m_pAudioCodec == nullptr)
    {
        // set to nullptr, otherwise avcodec_close(m_pAudioCodecContext) crashes
        m_pAudioCodecContext = nullptr;
        throw logic_error("Audio Codec not found for " + m_Filepath);
    }

    // currently we disable ac3 surround
    m_pAudioCodecContext->channels = 2;
    m_pAudioCodecContext->workaround_bugs = 1;

    m_pFormatContext->flags |= AVFMT_FLAG_GENPTS;
    m_pFormatContext->streams[m_AudioStream]->discard = AVDISCARD_DEFAULT;

#if LIBAVCODEC_VERSION_MAJOR < 53
    if (avcodec_open(m_pAudioCodecContext, m_pAudioCodec) < 0)
#else
    if (avcodec_open2(m_pAudioCodecContext, m_pAudioCodec, nullptr) < 0)
#endif
    {
        throw logic_error("Could not open audio codec for " + m_Filepath);
    }

    AudioFormat format = getAudioFormat();
    m_BytesPerFrame = format.framesPerPacket * (format.bits / 8) * format.numChannels;

    m_CurrentPacket.data = nullptr;
    m_CurrentPacket.size = 0;

    m_CurrentPacketOffsets.data = nullptr;
    m_CurrentPacketOffsets.size = 0;

    m_pAudioBuffer = reinterpret_cast<uint8_t*>(av_malloc((AVCODEC_MAX_AUDIO_FRAME_SIZE * 3) / 2));
}

double FFmpegDecoder::getProgress()
{
    if (m_pFormatContext)
    {
        return static_cast<double>(m_AudioClock / (m_pFormatContext->duration / AV_TIME_BASE));
    }

    return 0.0;
}

double FFmpegDecoder::getDuration()
{
    assert(m_pFormatContext);
    return static_cast<double>(m_pFormatContext->duration / AV_TIME_BASE);
}

int32_t FFmpegDecoder::getFrameSize()
{
    return m_BytesPerFrame;
}

void FFmpegDecoder::seekAbsolute(double time)
{
    int64_t timestamp = static_cast<int64_t>(time * AV_TIME_BASE);
    seek(timestamp);
}

void FFmpegDecoder::seekRelative(double offset)
{
    seekAbsolute(m_AudioClock + offset);
}

void FFmpegDecoder::seek(::int64_t timestamp)
{
    int32_t flags = timestamp < m_AudioClock ? AVSEEK_FLAG_BACKWARD : 0;

    if (timestamp < 0)
    {
        timestamp = 0;
    }

    //log::debug("av_gettime:", timestamp, "timebase", AV_TIME_BASE, flags);
    int32_t ret = av_seek_frame(m_pFormatContext, -1, timestamp, flags);

    if (ret >= 0)
    {
        m_AudioClock = static_cast<double>(timestamp) / AV_TIME_BASE;
        avcodec_flush_buffers(m_pFormatContext->streams[m_AudioStream]->codec);
        m_CurrentPacket.data = nullptr;
        m_CurrentPacket.size = 0;
        m_CurrentPacketOffsets.data = nullptr;
        m_CurrentPacketOffsets.size = 0;
    }
    else
    {
        log::error("Error seeking to position:", timestamp);
    }
}


bool FFmpegDecoder::decodeAudioFrame(AudioFrame& frame)
{
    for (;;)
    {
        while (m_CurrentPacketOffsets.size > 0)
        {
            int32_t dataSize = (AVCODEC_MAX_AUDIO_FRAME_SIZE * 3) / 2;

#if LIBAVCODEC_VERSION_MAJOR < 53
            int32_t bytesDecoded = avcodec_decode_audio2(m_pAudioStream->codec, reinterpret_cast<int16_t*>(m_pAudioBuffer), &dataSize, m_CurrentPacketOffsets.data, m_CurrentPacketOffsets.size);
#else
            int32_t bytesDecoded = avcodec_decode_audio3(m_pAudioStream->codec, reinterpret_cast<int16_t*>(m_pAudioBuffer), &dataSize, &m_CurrentPacketOffsets);
#endif
            if (bytesDecoded < 0)
            {
                log::error("Error decoding audio frame");
                m_CurrentPacketOffsets.size = 0;
                continue;
            }

            m_CurrentPacketOffsets.data += bytesDecoded;
            m_CurrentPacketOffsets.size -= bytesDecoded;

            if (dataSize <= 0)
            {
                continue;
            }

            if (m_CurrentPacket.pts != static_cast<int64_t>(0x8000000000000000LL)) //AV_NOPTS_VALUE doesn't compile
            {
                m_AudioClock = av_q2d(m_pAudioStream->time_base) * m_CurrentPacket.pts;
            }
            else
            {
                m_AudioClock += static_cast<double>(dataSize) / (2 * m_pAudioStream->codec->channels * m_pAudioStream->codec->sample_rate);
            }

            frame.setDataSize(dataSize);
            frame.setFrameData(m_pAudioBuffer);
            frame.setPts(m_AudioClock);

            m_BytesPerFrame = max(m_BytesPerFrame, static_cast<uint32_t>(dataSize));

            return true;
        }

        if (m_CurrentPacket.data)
        {
            av_free_packet(&m_CurrentPacket);
        }

        if (!readPacket(m_CurrentPacket))
        {
            return false;
        }

        m_CurrentPacketOffsets.data = m_CurrentPacket.data;
        m_CurrentPacketOffsets.size = m_CurrentPacket.size;
    }

    return false;
}

bool FFmpegDecoder::readPacket(AVPacket& packet)
{
    bool frameRead = false;

    while(!frameRead)
    {
        av_init_packet(&packet);
        if (av_read_frame(m_pFormatContext, &packet) >= 0)
        {
            if (packet.stream_index == m_AudioStream)
            {
                frameRead = true;
            }
            else
            {
                av_free_packet(&packet);
            }
        }
        else
        {
            //no more frames in the stream
            break;
        }
    }

    return frameRead;
}

AudioFormat FFmpegDecoder::getAudioFormat()
{
    AudioFormat format;

    switch(m_pAudioCodecContext->sample_fmt)
    {
    case SAMPLE_FMT_U8:
        format.bits = 8;
        break;
    case SAMPLE_FMT_S16:
        format.bits = 16;
        break;
    case SAMPLE_FMT_S32:
        format.bits = 32;
        break;
    default:
        format.bits = 0;
        break;
    }

    format.rate             = m_pAudioCodecContext->sample_rate;
    format.numChannels      = m_pAudioCodecContext->channels;
    format.framesPerPacket  = m_pAudioCodecContext->frame_size;

    log::debug("Audio format: rate (", format.rate, ") numChannels (", format.numChannels, ")");

    return format;
}

}
