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

#include "audioframe.h"

#include <cassert>

#include "utils/log.h"

namespace Gejengel
{

AudioFrame::AudioFrame()
: m_pFrameData(nullptr)
, m_DataSize(0)
, m_Pts(0.0)
{
}

AudioFrame::~AudioFrame()
{

}

uint8_t* AudioFrame::getFrameData() const
{
    return m_pFrameData;
}

uint32_t AudioFrame::getDataSize() const
{
    return m_DataSize;
}

double AudioFrame::getPts() const
{
    return m_Pts;
}

void AudioFrame::setFrameData(uint8_t* data)
{
    m_pFrameData = data;
}

void AudioFrame::setDataSize(uint32_t size)
{
    m_DataSize = size;
}

void AudioFrame::setPts(double pts)
{
    m_Pts = pts;
}

void AudioFrame::clear()
{
    m_pFrameData = nullptr;
    m_DataSize = 0;
    m_Pts = 0.0;
}

void AudioFrame::offsetDataPtr(uint32_t offset)
{
    if (offset >= m_DataSize)
    {
        m_pFrameData = nullptr;
        m_DataSize = 0;
    }
    else
    {
        m_pFrameData += offset;
        m_DataSize -= offset;
    }
}

void AudioFrame::allocateData(uint32_t size)
{
    m_pFrameData = new uint8_t[size];
    m_DataSize = size;
}

void AudioFrame::freeData()
{
    delete[] m_pFrameData;
    m_DataSize = 0;
} 

}
