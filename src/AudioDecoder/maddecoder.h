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

#ifndef MAD_DECODER_H
#define MAD_DECODER_H

#include <string>
#include <vector>
#include <fstream>
#include <memory>
#include <mad.h>

#include "mpegutils.h"
#include "utils/types.h"
#include "AudioDecoder/audiodecoder.h"

namespace Gejengel
{

class AudioFrame;
class IReader;

class MadDecoder : public AudioDecoder
{
public:
    MadDecoder(const std::string& uri);
    ~MadDecoder();

    bool decodeAudioFrame(AudioFrame& audioFrame);
    void seekAbsolute(double time);
    void seekRelative(double offset);

    AudioFormat getAudioFormat();

    double  getProgress();
    double  getAudioClock();
    double  getDuration();
    int32_t getFrameSize();

private:
    bool readDataIfNecessary();
    bool synchronize();
    bool readHeaders();
    bool decodeAudioFrame(AudioFrame& audioFrame, bool processSamples);
    
    uint32_t                    m_FileSize;
    mad_stream                  m_MadStream;
    mad_frame                   m_MadFrame;
    mad_synth                   m_MadSynth;
    mad_timer_t                 m_TrackPos;

    uint32_t                    m_EncoderDelay;
    uint32_t                    m_ZeroPadding;
    uint32_t                    m_Bitrate;
    uint32_t                    m_Duration;
    uint32_t                    m_Id3Size;
    uint32_t                    m_InputBufSize;

    bool                        m_FirstFrame;
    
    MpegUtils::MpegHeader       m_MpegHeader;
    MpegUtils::XingHeader       m_XingHeader;
    MpegUtils::LameHeader       m_LameHeader;
    
    std::vector<uint8_t>        m_InputBuffer;
    std::vector<uint8_t>        m_OutputBuffer;

    mad_fixed_t                 m_DitherErrorL[3];
    mad_fixed_t                 m_DitherErrorR[3];
    
    mad_fixed_t                 m_RandomValueL;
    mad_fixed_t                 m_RandomValueR;

    std::unique_ptr<IReader>    m_pReader;
};

}

#endif
