#include <iostream>
#include <fstream>

#include "Utils/log.h"
#include "Utils/fileoperations.h"
#include "AudioDecoder/mpegutils.h"
#include "AudioDecoder/filereader.h"

using namespace std;
using namespace Gejengel;

int main(int argc, char** argv)
{
    if (argc != 2)
    {
        Log::error("Usage:", argv[0], "filename");
        return -1;
    }


    FileReader reader(argv[1]);
    
    uint64_t filesize = reader.getContentLength();

    MpegUtils::MpegHeader mpegHeader;
    
    uint32_t id3Size = MpegUtils::skipId3Tag(reader);
    Log::info("Id3 size:", id3Size);

    uint32_t xingPos;
    if (MpegUtils::readMpegHeader(reader, mpegHeader, xingPos) == 0)
    {
        //try a bruteforce scan in the first meg to be really sure
        if (MpegUtils::searchMpegHeader(reader, mpegHeader, id3Size, xingPos) == 0)
        {
            Log::info("No mpeg header found");
            return 0;
        }
    }

    Log::info("Mpeg Header");
    Log::info("-----------");

    switch (mpegHeader.version)
    {
    case MpegUtils::MpegHeader::Mpeg1:
        Log::info("Mpeg Version 1");
        break;
    case MpegUtils::MpegHeader::Mpeg2:
        Log::info("Mpeg Version 2");
        break;
    case MpegUtils::MpegHeader::Mpeg2_5:
        Log::info("Mpeg Version 2.5");
        break;
    default:
        Log::info("Mpeg Version unknown");
    }

    Log::info("Layer ", mpegHeader.layer);

    switch (mpegHeader.channelMode)
    {
    case MpegUtils::MpegHeader::Mono:
        Log::info("Mono");
        break;
    case MpegUtils::MpegHeader::Stereo:
        Log::info("Stereo");
        break;
    case MpegUtils::MpegHeader::DualChannel:
        Log::info("Dual channel");
        break;
    case MpegUtils::MpegHeader::JointStereo:
        Log::info("Joint stereo");
        break;
    default:
        break;
    };

    
    MpegUtils::XingHeader xingHeader;
    
    reader.seekAbsolute(id3Size + xingPos);
    if (MpegUtils::readXingHeader(reader, xingHeader) == 0)
    {
        Log::info("Bitrate:", mpegHeader.bitRate);
        Log::info("Samplerate:", mpegHeader.sampleRate);
        Log::info("Samples per frame:", mpegHeader.samplesPerFrame);
        Log::info("Duration:", (filesize - id3Size) / (mpegHeader.bitRate * 125));

        Log::info("No xing header found");
        return 0;
    }

    Log::info("Bitrate:", mpegHeader.bitRate, xingHeader.vbr ? "VBR" : "CBR");
    Log::info("Samplerate:", mpegHeader.sampleRate);
    Log::info("Samples per frame:", mpegHeader.samplesPerFrame);

    if (xingHeader.numFrames > 0)
    {
        Log::info("Duration:", (xingHeader.numFrames * mpegHeader.samplesPerFrame) / mpegHeader.sampleRate);
    }
    else
    {
        Log::info("Duration:", (filesize - id3Size) / (mpegHeader.bitRate * 125));
    }
    
    MpegUtils::LameHeader lameHeader;
    if (MpegUtils::readLameHeader(reader, lameHeader) == 0)
    {
        Log::info("No lame header found");
        return 0;
    }

    Log::info("Lame header");
    Log::info("-----------");
    Log::info("Encoderdelay:", lameHeader.encoderDelay);
    Log::info("Zeropadding:", lameHeader.zeroPadding);
}
