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

#include "flacdecoder.h"

#include <stdexcept>
#include <cassert>

#include "readerfactory.h"
#include "AudioRenderer/audioframe.h"
#include "utils/scopedlock.h"
#include "utils/log.h"

using namespace std;
using namespace utils;

namespace Gejengel
{

FlacDecoder::FlacDecoder(const std::string& uri)
: AudioDecoder(uri)
, m_BytesPerFrame(0)
, m_NumSamples(0)
, m_pReader(ReaderFactory::create(uri))
{
    set_md5_checking(true);

    FLAC__StreamDecoderInitStatus initStatus = init();
    if (initStatus != FLAC__STREAM_DECODER_INIT_STATUS_OK)
    {
        throw logic_error(FLAC__StreamDecoderInitStatusString[initStatus]);
    }

    if (!process_until_end_of_metadata())
    {
        throw logic_error("Failed to parse flac metadata");
    }
}

FlacDecoder::~FlacDecoder()
{
    m_BytesPerFrame = 0;
    finish();
}

AudioFormat FlacDecoder::getAudioFormat()
{
    return m_Format;
}

double FlacDecoder::getProgress()
{
    double duration = getDuration();
    return duration == 0.0 ? 0.0 : m_AudioClock / duration;
}

double FlacDecoder::getDuration()
{
    if (m_Format.rate == 0)
    {
        return 0.0;
    }

    return static_cast<double>(m_NumSamples / m_Format.rate);
}

int32_t FlacDecoder::getFrameSize()
{
    return m_BytesPerFrame;
}

void FlacDecoder::seekAbsolute(double time)
{
    double duration = getDuration();
    if (duration == 0.0 || m_NumSamples == 0)
    {
        return;
    }
    
    FLAC__uint64 sampleNr = static_cast<FLAC__uint64>((time / duration) * m_NumSamples);
    if (!seek_absolute(sampleNr))
    {
        log::warn("Seek failed");
    }
}

void FlacDecoder::seekRelative(double offset)
{
    seekAbsolute(m_AudioClock + offset);
}

bool FlacDecoder::decodeAudioFrame(AudioFrame& frame)
{
    if (!process_single())
    {
        log::error("Flac decode error");
        return false;
    }

    if (get_state() == FLAC__STREAM_DECODER_END_OF_STREAM)
    {
        log::debug("End of stream");
        return false;
    }

    if (m_BytesPerFrame == 0)
    {
        m_BytesPerFrame = m_AudioBuffer.size();
    }

    frame.setFrameData(&m_AudioBuffer[0]);
    frame.setDataSize(m_AudioBuffer.size());
    frame.setPts(m_AudioClock);
    return true;
}

FLAC__StreamDecoderReadStatus FlacDecoder::read_callback(FLAC__byte buffer[], size_t* pBytes)
{
    if (m_pReader->eof())
    {
        return FLAC__STREAM_DECODER_READ_STATUS_END_OF_STREAM;
    }

    try
    {
        *pBytes = static_cast<size_t>(m_pReader->read(buffer, *pBytes));
    }
    catch (std::exception& e)
    {
        log::error("Flac decoder read failed:", e.what());
        return FLAC__STREAM_DECODER_READ_STATUS_ABORT;
    }
    
    return FLAC__STREAM_DECODER_READ_STATUS_CONTINUE;
}

FLAC__StreamDecoderSeekStatus FlacDecoder::seek_callback(FLAC__uint64 position)
{
    m_pReader->seekAbsolute(position);
    return FLAC__STREAM_DECODER_SEEK_STATUS_OK;
}

FLAC__StreamDecoderTellStatus FlacDecoder::tell_callback(FLAC__uint64* pCurrentPosition)
{
    *pCurrentPosition = m_pReader->currentPosition();
    return FLAC__STREAM_DECODER_TELL_STATUS_OK;
}

FLAC__StreamDecoderLengthStatus FlacDecoder::length_callback(FLAC__uint64* pStreamLength)
{
    try
    {
        *pStreamLength = m_pReader->getContentLength();
    }
    catch (std::exception& e)
    {
        log::error("Flac decoder get stream length failed:", e.what());
        return FLAC__STREAM_DECODER_LENGTH_STATUS_ERROR;
    }
    
    return FLAC__STREAM_DECODER_LENGTH_STATUS_OK;
}

bool FlacDecoder::eof_callback()
{
    return m_pReader->eof();
}

FLAC__StreamDecoderWriteStatus FlacDecoder::write_callback(const FLAC__Frame* pFrame, const FLAC__int32* const pBuffer[])
{
    assert(pFrame);
    assert(pFrame->header.bits_per_sample == 16);
    
    uint32_t frameSize = pFrame->header.blocksize * pFrame->header.channels * (pFrame->header.bits_per_sample / 8);
    m_AudioBuffer.resize(frameSize);

    uint16_t* pFrameData = reinterpret_cast<uint16_t*>(&m_AudioBuffer[0]);

    for (size_t i = 0; i < pFrame->header.blocksize; ++i)
    {
        *pFrameData++ = pBuffer[0][i]; //left channel
        *pFrameData++ = pBuffer[1][i]; //right channel
    }

    if (pFrame->header.number_type == FLAC__FRAME_NUMBER_TYPE_FRAME_NUMBER)
    {
        log::debug("TODO");
    }
    else
    {
        assert(pFrame->header.number_type == FLAC__FRAME_NUMBER_TYPE_SAMPLE_NUMBER);
        m_AudioClock = pFrame->header.number.frame_number / m_Format.rate;
    }

    return FLAC__STREAM_DECODER_WRITE_STATUS_CONTINUE;
}

void FlacDecoder::metadata_callback(const FLAC__StreamMetadata* pMetadata)
{
    assert(pMetadata);
    if (pMetadata->type == FLAC__METADATA_TYPE_STREAMINFO)
    {
        m_Format.bits           = pMetadata->data.stream_info.bits_per_sample;
        m_Format.rate           = pMetadata->data.stream_info.sample_rate;
        m_Format.numChannels    = pMetadata->data.stream_info.channels;
        m_NumSamples            = pMetadata->data.stream_info.total_samples;

        log::debug("Flac Audio format: bits (", m_Format.bits, ") rate (", m_Format.rate, ") numChannels (", m_Format.numChannels, pMetadata->data.stream_info.max_framesize);
    }
}

void FlacDecoder::error_callback(FLAC__StreamDecoderErrorStatus status)
{
    log::error("FlacDecoder:",  FLAC__StreamDecoderErrorStatusString[status]);
}

}
