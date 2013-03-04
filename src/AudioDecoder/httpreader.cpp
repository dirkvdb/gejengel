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

#include "httpreader.h"

#include <stdexcept>

#include "utils/log.h"
#include "upnp/upnphttpget.h"

namespace Gejengel
{

HttpReader::HttpReader()
: m_ContentLength(0)
, m_CurrentPosition(0)
{
}

HttpReader::HttpReader(const std::string& url)
: IReader(url)
, m_ContentLength(0)
, m_CurrentPosition(0)
{
}

HttpReader::~HttpReader() 
{
}

void HttpReader::open(const std::string& url)
{
    m_Url = url;
}

uint64_t HttpReader::getContentLength()
{
	int32_t timeout = 5;
	upnp::HttpGet httpGet(m_Url.c_str(), timeout);
	m_ContentLength = httpGet.getContentLength();

    return m_ContentLength;
}

uint64_t HttpReader::currentPosition()
{
    return m_CurrentPosition;
}

void HttpReader::seekAbsolute(uint64_t position)
{
    m_CurrentPosition = position;
}

void HttpReader::seekRelative(uint64_t offset)
{
    m_CurrentPosition += offset;
}

bool HttpReader::eof()
{
    if (m_ContentLength == 0)
    {
        m_ContentLength = getContentLength();
    }
    
    return m_CurrentPosition >= m_ContentLength;
}

uint64_t HttpReader::read(uint8_t* pData, uint64_t size)
{
    uint64_t upperLimit = m_CurrentPosition + size - 1;
    if (upperLimit >= m_ContentLength)
    {
        upperLimit = m_ContentLength;
    }
    
    int32_t timeout = 5;
    upnp::HttpGet httpGet(m_Url.c_str(), m_CurrentPosition, upperLimit - m_CurrentPosition, timeout);
    httpGet.get(pData);
    utils::log::debug("read from: %d req: %d actual: %d", m_CurrentPosition, upperLimit - m_CurrentPosition, httpGet.getContentLength());

    m_CurrentPosition += httpGet.getContentLength();

    return httpGet.getContentLength();
}

}
