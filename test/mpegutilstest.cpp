#include <gtest/gtest.h>
#include <fstream>

#include "AudioDecoder/mpegutils.h"
#include "AudioDecoder/filereader.h"
#include "Utils/fileoperations.h"

using namespace std;
using namespace FileOperations;
using namespace Gejengel;

class MpegUtilsTest : public testing::Test
{
protected:
    virtual void SetUp()
    {
        const char* env = getenv("srcdir");
        ASSERT_FALSE(env == nullptr);

        mp3.open(combinePath(env, "testdata/delaytest.mp3"));
        mp3Id3.open(combinePath(env, "testdata/delaytestwithid3.mp3"));
    }

    FileReader mp3;
    FileReader mp3Id3;
};

TEST_F(MpegUtilsTest, skipId3Tag)
{
    ASSERT_EQ(0, MpegUtils::skipId3Tag(mp3));
    ASSERT_EQ(1024, MpegUtils::skipId3Tag(mp3Id3));
}

TEST_F(MpegUtilsTest, MpegHeader)
{
    uint32_t xingPos;
    MpegUtils::MpegHeader header;
    ASSERT_GT(MpegUtils::readMpegHeader(mp3, header, xingPos), 0);
    ASSERT_EQ(MpegUtils::MpegHeader::Mpeg1, header.version);
    ASSERT_EQ(MpegUtils::MpegHeader::Layer3, header.layer);
    ASSERT_EQ(MpegUtils::MpegHeader::JointStereo, header.channelMode);
    ASSERT_EQ(2, header.numChannels);
    ASSERT_EQ(44100, header.sampleRate);
    ASSERT_EQ(36, xingPos);
}

TEST_F(MpegUtilsTest, XingHeader)
{
    mp3.seekAbsolute(36);
    
    MpegUtils::XingHeader header;
    ASSERT_EQ(120, MpegUtils::readXingHeader(mp3, header));
    ASSERT_EQ(41, header.numFrames);
    ASSERT_EQ(20770, header.numBytes);

    ASSERT_EQ(36 + 120, mp3.currentPosition());
}

TEST_F(MpegUtilsTest, LameHeader)
{
    mp3.seekAbsolute(36);
    
    MpegUtils::XingHeader header;
    MpegUtils::readXingHeader(mp3, header);

    MpegUtils::LameHeader lameHeader;
    ASSERT_EQ(36, MpegUtils::readLameHeader(mp3, lameHeader));
    ASSERT_EQ(576, lameHeader.encoderDelay);
    ASSERT_EQ(1728, lameHeader.zeroPadding);
}
