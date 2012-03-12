#include <gtest/gtest.h>
#include <fstream>

#include "AudioRenderer/audiobuffer.h"

using namespace std;
using namespace Gejengel;

TEST(AudioBufferTest, Creation)
{
    AudioBuffer buffer(128);
    ASSERT_EQ(0, buffer.bytesUsed());
    ASSERT_EQ(128, buffer.bytesFree());
}

TEST(AudioBufferTest, ReadWrite)
{
    AudioBuffer buffer(128);

    uint8_t data1[64];
    uint8_t data2[14];
    uint8_t data3[23];
    memset(data1, 14, 64);
    memset(data2, 15, 14);
    memset(data3, 16, 23);

    buffer.writeData(data1, 64);
    ASSERT_EQ(64, buffer.bytesUsed());
    ASSERT_EQ(64, buffer.bytesFree());
    buffer.writeData(data2, 14);
    ASSERT_EQ(78, buffer.bytesUsed());
    ASSERT_EQ(50, buffer.bytesFree());
    buffer.writeData(data3, 23);
    ASSERT_EQ(101, buffer.bytesUsed());
    ASSERT_EQ(27, buffer.bytesFree());

    uint32_t size = 101;
    uint8_t* pData = buffer.getData(size);
    ASSERT_EQ(101, size);
    ASSERT_EQ(0, buffer.bytesUsed());
    ASSERT_EQ(128, buffer.bytesFree());

    ASSERT_EQ(0, memcmp(pData, data1, 64));
    pData += 64;
    ASSERT_EQ(0, memcmp(pData, data2, 14));
    pData += 14;
    ASSERT_EQ(0, memcmp(pData, data3, 23));
}

TEST(AudioBufferTest, ReadWriteOverBoundary)
{
    AudioBuffer buffer(128);

    uint8_t data1[96];
    uint8_t data2[64];
    memset(data1, 14, 96);
    memset(data2, 15, 32);
    memset(data2 + 32, 16, 32);

    uint32_t size = 96;
    buffer.writeData(data1, 96);
    uint8_t* pData = buffer.getData(size);
    ASSERT_EQ(96, size);

    buffer.writeData(data2, 64);
    ASSERT_EQ(64, buffer.bytesUsed());
    ASSERT_EQ(64, buffer.bytesFree());

    size = 64;
    pData = buffer.getData(size);
    ASSERT_EQ(32, size);
    ASSERT_EQ(0, memcmp(pData, data2, 32));
    ASSERT_EQ(32, buffer.bytesUsed());
    ASSERT_EQ(96, buffer.bytesFree());

    size = 64;
    pData = buffer.getData(size);
    ASSERT_EQ(32, size);
    ASSERT_EQ(0, buffer.bytesUsed());
    ASSERT_EQ(128, buffer.bytesFree());

    ASSERT_EQ(0, memcmp(pData, data2 + 32, 32));
}

TEST(AudioBufferTest, ReadFromFullBuffer)
{
    AudioBuffer buffer(128);

    uint8_t data1[128];

    buffer.writeData(data1, 128);
    ASSERT_EQ(128, buffer.bytesUsed());
    ASSERT_EQ(0, buffer.bytesFree());
    
    uint32_t size = 128;
    uint8_t* pData = buffer.getData(size);
    ASSERT_EQ(128, size);
    ASSERT_EQ(0, memcmp(pData, data1, 128));
}

