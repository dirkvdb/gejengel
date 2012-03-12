#include "audiobuffer.h"

#include <algorithm>
#include <stdexcept>
#include <cstring>

namespace Gejengel
{

AudioBuffer::AudioBuffer(uint32_t size)
: m_Size(size)
, m_Fill(0)
, m_pAudioBuffer(new uint8_t[size])
, m_pReadPtr(m_pAudioBuffer)
, m_pWritePtr(m_pAudioBuffer)
, m_pEnd(m_pAudioBuffer + size)
{
}

AudioBuffer::~AudioBuffer()
{
    delete[] m_pAudioBuffer;
}

uint8_t* AudioBuffer::getData(uint32_t& size)
{
    uint8_t* pPtr = m_pReadPtr;
    size = std::min(bytesUsed(), size);
    if (m_pReadPtr + size >= m_pEnd)
    {
        size = m_pEnd - m_pReadPtr;
        m_pReadPtr = m_pAudioBuffer;
    }
    else
    {
        m_pReadPtr += size;
    }

    m_Fill -= size;
    return pPtr;
}

void AudioBuffer::writeData(const uint8_t* pData, uint32_t size)
{
    if (size > bytesFree())
    {
        throw std::logic_error("Not enough room in audio buffer to write data");
    }

    if (m_pWritePtr + size >= m_pEnd)
    {
        uint32_t firstSize  = static_cast<uint32_t>(m_pEnd - m_pWritePtr);
        uint32_t secondSize = size - firstSize;

        memcpy(m_pWritePtr, pData, firstSize);
        memcpy(m_pAudioBuffer, pData + firstSize, secondSize);
        m_pWritePtr = m_pAudioBuffer + secondSize;
    }
    else
    {
        memcpy(m_pWritePtr, pData, size);
        m_pWritePtr += size;
    }

    m_Fill += size;
}

uint32_t AudioBuffer::bytesFree()
{
    return m_Size - m_Fill;
}

uint32_t AudioBuffer::bytesUsed()
{
    return m_Fill;
}

void AudioBuffer::clear()
{
    m_pReadPtr = m_pWritePtr = m_pAudioBuffer;
    m_Fill = 0;
}

}
