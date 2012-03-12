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

#ifndef GEJENGEL_HTTP_READER_H
#define GEJENGEL_HTTP_READER_H

#include <string>
#include <fstream>
#include <inttypes.h>

#include "reader.h"

namespace Gejengel
{

class HttpReader : public IReader
{
public:
    HttpReader();
    HttpReader(const std::string& url);
    ~HttpReader();
    
    void open(const std::string& url);

    uint64_t getContentLength();
    uint64_t currentPosition();
    void seekAbsolute(uint64_t position);
    void seekRelative(uint64_t offset);
    bool eof();
    uint64_t read(uint8_t* pData, uint64_t size);
    
private:
    uint64_t    m_ContentLength;
    uint64_t    m_CurrentPosition;
};

}

#endif
