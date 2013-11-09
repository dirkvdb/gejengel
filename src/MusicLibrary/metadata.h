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

#ifndef METADATA_H
#define METADATA_H

#include <string>
#include <vector>
#include <taglib.h>
#include <fileref.h>

#include "utils/types.h"

namespace Gejengel
{

class Metadata
{
public:
    Metadata(const std::string& filepath);

    bool isValid();

    void getArtist(std::string& artist);
    void getTitle(std::string& title);
    void getAlbum(std::string& album);
    void getAlbumArtist(std::string& albumArtist);
    void getGenre(std::string& genre);
    void getComposer(std::string& composer);
    uint32_t getDiscNr();
    uint32_t getYear();
    uint32_t getTrackNr();
    uint32_t getBitRate();
    uint32_t getSampleRate();
    uint32_t getChannels();
    uint32_t getDuration();
    bool getAlbumArt(std::vector<uint8_t>& data, const std::vector<std::string>& albumArtFileList, int32_t albumArtSize = -1);

private:
    static bool resizeAlbumArt(const uint8_t* pData, uint64_t dataSize, int32_t albumArtSize, std::vector<uint8_t>& resizedAlbumArtData);
    static uint32_t parseDisc(const std::string& disc);

    TagLib::FileRef     m_TagFile;
    TagLib::Tag*        m_pTagPtr;
    const std::string   m_FilePath;
};

}

#endif
