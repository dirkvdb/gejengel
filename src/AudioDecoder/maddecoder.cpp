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

#include "maddecoder.h"

#include <cmath>
#include <cassert>
#include <cstring>
#include <stdexcept>

#include "readerfactory.h"
#include "AudioRenderer/audioframe.h"
#include "AudioRenderer/audioformat.h"
#include "utils/log.h"
#include "utils/fileoperations.h"
#include "utils/numericoperations.h"

using namespace std;
using namespace utils;

namespace Gejengel
{

static const size_t     INPUT_BUFFER_SIZE = 65536; //64kb buffer

MadDecoder::MadDecoder(const std::string& uri)
: AudioDecoder(uri)
, m_FileSize(0)
, m_TrackPos(mad_timer_zero)
, m_EncoderDelay(0)
, m_ZeroPadding(0)
, m_Bitrate(0)
, m_Duration(0)
, m_Id3Size(0)
, m_InputBufSize(0)
, m_FirstFrame(true)
, m_InputBuffer(INPUT_BUFFER_SIZE + MAD_BUFFER_GUARD)
, m_RandomValueL(0)
, m_RandomValueR(0)
, m_pReader(ReaderFactory::create(uri))
{
    m_FileSize = static_cast<uint32_t>(m_pReader->getContentLength());

    if (!readHeaders())
    {
        throw logic_error("File is not an mp3 file: " + m_pReader->url());
    }

    log::debug("Mad Audio format: bits (16) rate (", m_MpegHeader.sampleRate, ") numChannels (", m_MpegHeader.numChannels, ") bitrate (", m_MpegHeader.bitRate, ")");
    log::debug("Encoderdelay: ", m_LameHeader.encoderDelay, " Zeropadding:", m_LameHeader.zeroPadding);

    m_pReader->seekAbsolute(m_Id3Size);

    mad_stream_init(&m_MadStream);
    mad_frame_init(&m_MadFrame);
    mad_synth_init(&m_MadSynth);
}

MadDecoder::~MadDecoder()
{
    mad_stream_finish(&m_MadStream);
    mad_frame_finish(&m_MadFrame);
    mad_synth_finish(&m_MadSynth);
}

AudioFormat MadDecoder::getAudioFormat()
{
    AudioFormat format;
    format.bits = 16;
    format.rate = m_MpegHeader.sampleRate;
    format.numChannels = m_MpegHeader.numChannels;

    return format;
}

double MadDecoder::getProgress()
{
    return 0.0;
}

double MadDecoder::getAudioClock()
{
    return mad_timer_count(m_TrackPos, MAD_UNITS_MILLISECONDS) / 1000.0;
}

double MadDecoder::getDuration()
{
    return m_Duration;
}

int32_t MadDecoder::getFrameSize()
{
    return m_MpegHeader.samplesPerFrame * m_MpegHeader.numChannels * 2 /* 16 bits / 2 */; ;
}

void MadDecoder::seekAbsolute(double time)
{
    uint32_t byteOffset;
    
    float percentage = static_cast<float>(time / getDuration());
    numericops::clip(percentage, 0.f, 100.f);
    
    if (m_XingHeader.hasToc)
    {
        int32_t tocIndex = static_cast<int32_t>(percentage * 100);
        numericops::clip(tocIndex, 0, 99);

        uint32_t value1 = m_XingHeader.toc[tocIndex];
        uint32_t value2 = percentage < 99 ? m_XingHeader.toc[tocIndex+1] : 256;

        float filePercentage = value1 + ((value2 - value1) * ((percentage * 100) - static_cast<float>(tocIndex)));
        byteOffset = m_Id3Size + static_cast<uint32_t>((filePercentage * m_FileSize) / 256.0f);
    }
    else
    {
        byteOffset = m_Id3Size + static_cast<uint32_t>((m_FileSize * percentage));
    }
    
    m_pReader->seekAbsolute(byteOffset);

    mad_stream_finish(&m_MadStream);
    mad_stream_init(&m_MadStream);

    mad_frame_mute (&m_MadFrame);
    mad_synth_mute (&m_MadSynth);

    uint32_t seconds = static_cast<uint32_t>(time);
    mad_timer_set(&m_TrackPos, seconds, 0, 0);
    
    if (!readDataIfNecessary())
    {
        return; //eof
    }
    
    synchronize();

    //already decode one frame to avoid clicks
    AudioFrame frame;
    decodeAudioFrame(frame, false);
    decodeAudioFrame(frame, false);
}

void MadDecoder::seekRelative(double offset)
{
    seekAbsolute(getAudioClock() + offset);
}

bool MadDecoder::readDataIfNecessary()
{
    if (m_MadStream.buffer == nullptr || m_MadStream.error == MAD_ERROR_BUFLEN)
    {
        size_t bytesToRead = INPUT_BUFFER_SIZE;
        size_t choppedFrameSize = 0;

        if (m_MadStream.next_frame != nullptr)
        {
            //partial frame in the end of the framebuffer. moved it to the front
            choppedFrameSize = m_MadStream.bufend - m_MadStream.next_frame;
            memmove(&m_InputBuffer[0], m_MadStream.next_frame, choppedFrameSize);
            bytesToRead -= choppedFrameSize;
        }

        uint64_t readBytes = m_pReader->read(&m_InputBuffer[choppedFrameSize], bytesToRead);
        if (m_pReader->eof())
        {
            if (readBytes == 0)
            {
                return false;
            }
            memset(&m_InputBuffer[choppedFrameSize + static_cast<size_t>(readBytes)], 0, MAD_BUFFER_GUARD);
            mad_stream_buffer(&m_MadStream, &m_InputBuffer[0], static_cast<size_t>(readBytes) + choppedFrameSize + MAD_BUFFER_GUARD);
            m_InputBufSize = static_cast<size_t>(readBytes) + choppedFrameSize;
        }
        //else if (m_FileStream.fail())
        //{
        //    log::critical("Mad: Failed to read from file:", m_Filepath);
        //    m_MadStream.error = MAD_ERROR_BUFPTR;
        //}
        else
        {
            mad_stream_buffer(&m_MadStream, &m_InputBuffer[0], INPUT_BUFFER_SIZE);
            m_InputBufSize = INPUT_BUFFER_SIZE;
        }

        m_MadStream.error = MAD_ERROR_NONE;
    }

    return !m_pReader->eof();
}

bool MadDecoder::synchronize()
{
    //try to synchronize in the data stream
    for (int32_t i = 0; i < 32; ++i)
    {
        if (mad_stream_sync(&m_MadStream) == 0)
        {
            return true;
        }
        
        if (!readDataIfNecessary())
        {
            return false;
        }
    }

    return false;
}

static inline unsigned long prng(unsigned long state)
{
  return (state * 0x0019660dL + 0x3c6ef35fL) & 0xffffffffL;
}

//based on madplay dither code
inline int32_t dither(mad_fixed_t sample, mad_fixed_t ditherError[3], mad_fixed_t random)
{
    /* noise shape */
    sample += ditherError[0] - ditherError[1] + ditherError[2];

    ditherError[2] = ditherError[1];
    ditherError[1] = ditherError[0] / 2;

    /* bias */
    mad_fixed_t output = sample + (1L << (MAD_F_FRACBITS + 1 - 16 - 1));

    uint32_t scalebits = MAD_F_FRACBITS + 1 - 16;
    mad_fixed_t mask = (1L << scalebits) - 1;

    /* dither */
    mad_fixed_t rand = prng(random);
    output += (rand & mask) - (random & mask);

    random = rand;

    /* clip */
    if (output >= MAD_F_ONE)
    {
        output = MAD_F_ONE - 1;
    }
    else if (sample < -MAD_F_ONE)
    {
        output = -MAD_F_ONE;
    }

    /* quantize */
    output &= ~mask;

    /* error feedback */
    ditherError[0] = sample - output;

    /* scale */
    return output >> scalebits;
}

bool MadDecoder::decodeAudioFrame(AudioFrame& audioFrame)
{
    return decodeAudioFrame(audioFrame, true);
}

bool MadDecoder::decodeAudioFrame(AudioFrame& frame, bool processSamples)
{
    readDataIfNecessary();

    while (mad_frame_decode(&m_MadFrame, &m_MadStream))
    {
        if (MAD_RECOVERABLE(m_MadStream.error))
        {
            if (processSamples)
                log::warn("Decode error, but recoverable:", mad_stream_errorstr(&m_MadStream));
        }
        else
        {        
            if (m_MadStream.error == MAD_ERROR_BUFLEN)
            {
                if (m_pReader->eof())
                {
                    return false;
                }
                readDataIfNecessary();
                continue;
            }
            else
            {
                log::error("Decoder error, unrecoverable:", mad_stream_errorstr(&m_MadStream));
                return false;
            }
        }
    }

    mad_timer_add(&m_TrackPos, m_MadFrame.header.duration);

    m_MadFrame.options |= MAD_OPTION_IGNORECRC;
    mad_synth_frame(&m_MadSynth, &m_MadFrame);

    if (!processSamples)
    {
        //we are decoding frames after a seek, no need to process them
        return false;
    }

    m_OutputBuffer.resize(m_MadSynth.pcm.length * m_MadSynth.pcm.channels * 2);

    uint8_t* pFrameData = reinterpret_cast<uint8_t*>(&m_OutputBuffer[0]);
    mad_fixed_t const* leftChannel  = m_MadSynth.pcm.samples[0];
    mad_fixed_t const* rightChannel = m_MadSynth.pcm.samples[1];

    for (int32_t i = 0; i < m_MadSynth.pcm.length; ++i)
    {
        int32_t sample = dither(*leftChannel++, m_DitherErrorL, m_RandomValueL);
        *pFrameData++ = sample & 0xFF;
        *pFrameData++ = (sample >> 8) & 0xFF;

        if (m_MadSynth.pcm.channels == 2)
        {
            sample = dither(*rightChannel++, m_DitherErrorR, m_RandomValueR);
            *pFrameData++ = sample & 0xFF;
            *pFrameData++ = (sample >> 8) & 0xFF;
        }
    }


    frame.setPts(getAudioClock());
    
    if (m_FirstFrame)
    {
        if (m_LameHeader.encoderDelay >= m_MadSynth.pcm.length)
        {
            m_LameHeader.encoderDelay -= m_MadSynth.pcm.length;
            //delay is bigger then first frame, decode next frame
            return decodeAudioFrame(frame, processSamples);
        }

        uint32_t delay = m_LameHeader.encoderDelay * m_MpegHeader.numChannels * 2 /*16 / 8*/;
        frame.setFrameData(&m_OutputBuffer[delay]);
        frame.setDataSize(m_OutputBuffer.size() - delay);
        m_FirstFrame = false;
    }
    else if (m_pReader->eof() && ((&m_InputBuffer[m_InputBufSize] - m_MadStream.next_frame) == 1173) && (m_LameHeader.zeroPadding >= m_MadSynth.pcm.length))
    {
        //1 to last frame
        uint32_t paddingBytes = (m_LameHeader.zeroPadding - m_MadSynth.pcm.length) * m_MpegHeader.numChannels * 2 /*16 / 8*/;
        m_LameHeader.zeroPadding -= m_MadSynth.pcm.length;
        frame.setFrameData(&m_OutputBuffer[0]);
        frame.setDataSize(m_OutputBuffer.size() - paddingBytes);
    }
    else if (m_pReader->eof() && ((&m_InputBuffer[m_InputBufSize] - m_MadStream.next_frame) <= 128))
    {
        //final frame of file has just been proccessed
        uint32_t paddingBytes = m_LameHeader.zeroPadding * m_MpegHeader.numChannels * 2 /*16 / 8*/;
        if (paddingBytes >= m_OutputBuffer.size())
        {
            return false;
        }

        frame.setFrameData(&m_OutputBuffer[0]);
        frame.setDataSize(m_OutputBuffer.size() - paddingBytes);
    }
    else
    {
        frame.setFrameData(&m_OutputBuffer[0]);
        frame.setDataSize(m_OutputBuffer.size());
    }    

    return true;
}

bool MadDecoder::readHeaders()
{
    m_Id3Size = MpegUtils::skipId3Tag(*m_pReader);
    
    uint32_t xingPos;
    if (MpegUtils::readMpegHeader(*m_pReader, m_MpegHeader, xingPos) == 0)
    {
        log::debug("No mpeg header found, offset =", m_Id3Size);
        //try a bruteforce scan in the first meg to be really sure
        if (MpegUtils::searchMpegHeader(*m_pReader, m_MpegHeader, m_Id3Size, xingPos) == 0)
        {
            return false;
        }
    }

    if (m_MpegHeader.bitRate == 0)
    {
        log::debug("Freeformat file");
        return true;
    }

    m_pReader->seekAbsolute(m_Id3Size + xingPos);
    if (MpegUtils::readXingHeader(*m_pReader, m_XingHeader) == 0)
    {
        log::debug("No xing header found");
        m_Duration = (m_FileSize - m_Id3Size) / (m_MpegHeader.bitRate * 125); //m_MpegHeader.bitRate / 8) * 1000
        return true;
    }

    if (MpegUtils::readLameHeader(*m_pReader, m_LameHeader) == 0)
    {
        log::debug("No lame header found");
    }

    if (m_LameHeader.encoderDelay > 0)
    {
        m_LameHeader.encoderDelay += 528 + m_MpegHeader.samplesPerFrame; //add decoderdelay
    }

    assert(m_MpegHeader.samplesPerFrame);
    assert(m_MpegHeader.sampleRate);
    assert(m_MpegHeader.bitRate);
    
    if (m_XingHeader.numFrames > 0)
    {
        m_Duration = (m_XingHeader.numFrames * m_MpegHeader.samplesPerFrame) / m_MpegHeader.sampleRate;
    }
    else
    {
        m_Duration = (m_FileSize - m_Id3Size) / (m_MpegHeader.bitRate * 125); //m_MpegHeader.bitRate / 8) * 1000
    }

    return true;
}

}
