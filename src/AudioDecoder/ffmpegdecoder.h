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

#ifndef FFMPEG_DECODER_H
#define FFMPEG_DECODER_H

#include <string>

struct AVFormatContext;
struct AVCodecContext;
struct AVCodec;
struct AVPacket;
struct AVStream;

#include "audiodecoder.h"
#include "utils/types.h"

extern "C"
{
    #include <libavformat/avformat.h>
}

namespace audio
{

class Frame;
struct Format;

class FFmpegDecoder : public IDecoder
{
public:
    FFmpegDecoder(const std::string& filename);
    ~FFmpegDecoder();

    bool decodeAudioFrame(Frame& audioFrame);
    void seekAbsolute(double time);
    void seekRelative(double offset);

    Format getAudioFormat();

    double  getProgress();
    double  getDuration();
    int32_t getFrameSize();

private:
    bool readPacket(AVPacket& packet);
    void seek(int64_t timestamp);
    void initialize();
    void destroy();
    void initializeAudio();

    int32_t                 m_AudioStream;
    AVFormatContext*        m_pFormatContext;
    AVCodecContext*         m_pAudioCodecContext;
    AVCodec*                m_pAudioCodec;
    AVStream*               m_pAudioStream;
    AVPacket                m_CurrentPacket;
    AVPacket                m_CurrentPacketOffsets;
    uint8_t*                m_pAudioBuffer;
    uint32_t                m_BytesPerFrame;
};

}

#endif
