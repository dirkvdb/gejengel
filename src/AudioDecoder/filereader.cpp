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

#include "filereader.h"

#include <stdexcept>

namespace Gejengel
{
    
FileReader::FileReader()
{
}


FileReader::FileReader(const std::string& url)
: IReader(url)
, m_File(url.c_str(), std::ios::binary)
{
    if (!m_File.is_open())
    {
        throw std::logic_error("Failed to open file for reading: " + url);
    }
}

FileReader::~FileReader() 
{
}

void FileReader::open(const std::string& url)
{
    m_Url = url;
    m_File.open(url.c_str(), std::ios::binary);
    
    if (!m_File.is_open())
    {
        throw std::logic_error("Failed to open file for reading: " + url);
    }
}

uint64_t FileReader::getContentLength()
{
    uint64_t curPos = m_File.tellg();
    
    m_File.seekg(0, std::ios::end);
    uint64_t length = m_File.tellg();
  
    m_File.seekg (curPos);
    
    return length;
}

uint64_t FileReader::currentPosition()
{
    return m_File.tellg();
}

void FileReader::seekAbsolute(uint64_t position)
{
    m_File.seekg(position);
}

void FileReader::seekRelative(uint64_t offset)
{
    m_File.seekg(offset, std::ios::cur);
}

bool FileReader::eof()
{
    return m_File.eof();
}

uint64_t FileReader::read(uint8_t* pData, uint64_t size)
{
    m_File.read(reinterpret_cast<char*>(pData), size);
    return m_File.gcount();
}

}
