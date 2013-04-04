//    Copyright (C) 2009 Dirk Vanden Boer <dirk.vdb@gmail.com>
//
//    This program is free software; you can redistribute it and/or modifys
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

#include "metadata.h"

#include "config.h"
#include "utils/log.h"
#include "utils/stringoperations.h"
#include "utils/fileoperations.h"

#include <cmath>
#include <cassert>
#include <cstring>
#include <fstream>
#include <sstream>
#include <tag.h>
#include <id3v2tag.h>
#include <mpegfile.h>
#include <vorbisfile.h>
#include <flacfile.h>
#include <attachedpictureframe.h>
#include <tlist.h>
#include <Magick++.h>

static const int32_t ALBUM_ART_DB_SIZE = 96;

using namespace std;
using namespace utils;

namespace Gejengel
{

Metadata::Metadata(const std::string& filepath)
: m_TagFile(filepath.c_str())
, m_pTagPtr(nullptr)
, m_FilePath(filepath)
{
    if (isValid())
    {
        m_pTagPtr = m_TagFile.tag();
    }
}

bool Metadata::isValid()
{
    return !(m_TagFile.isNull() || !m_TagFile.tag());
}

void Metadata::getArtist(string& artist)
{
    artist = stringops::trim(m_pTagPtr->artist().to8Bit(true));
}

void Metadata::getTitle(string& title)
{
    title = stringops::trim(m_pTagPtr->title().to8Bit(true));
}

void Metadata::getAlbum(string& album)
{
    album = stringops::trim(m_pTagPtr->album().to8Bit(true));
}

void Metadata::getAlbumArtist(string& albumArtist)
{
    if (TagLib::MPEG::File* pMpegFile = dynamic_cast<TagLib::MPEG::File*>(m_TagFile.file()))
    {
        if (pMpegFile->ID3v2Tag())
        {
            const TagLib::ID3v2::FrameList& tpe2List = pMpegFile->ID3v2Tag()->frameList("TPE2");
            if (!tpe2List.isEmpty())
            {
                albumArtist = stringops::trim(tpe2List.front()->toString().to8Bit(true));
            }
        }
    }
}

void Metadata::getGenre(std::string& genre)
{
    genre = stringops::trim(m_pTagPtr->genre().to8Bit(true));
}

void Metadata::getComposer(string& composer)
{
    if (TagLib::MPEG::File* pMpegFile = dynamic_cast<TagLib::MPEG::File*>(m_TagFile.file()))
    {
        if (pMpegFile->ID3v2Tag())
        {
            const TagLib::ID3v2::FrameList& tcomList = pMpegFile->ID3v2Tag()->frameList("TCOM");
            if (!tcomList.isEmpty())
            {
                composer = stringops::trim(tcomList.front()->toString().to8Bit(true));
            }
        }
    }
    else if (TagLib::Ogg::Vorbis::File* pVorbisFile = dynamic_cast<TagLib::Ogg::Vorbis::File*>(m_TagFile.file()))
    {
        if (pVorbisFile->tag())
        {
            const TagLib::Ogg::FieldListMap& listMap = pVorbisFile->tag()->fieldListMap();
            if (!listMap["COMPOSER"].isEmpty())
            {
                composer = stringops::trim(listMap["COMPOSER"].front().to8Bit(true));
            }
        }
    }
    else if (TagLib::FLAC::File* pFlacFile = dynamic_cast<TagLib::FLAC::File*>(m_TagFile.file()))
    {
        if (pFlacFile->xiphComment())
        {
            const TagLib::Ogg::FieldListMap& listMap = pFlacFile->xiphComment()->fieldListMap();
            if (!listMap["COMPOSER"].isEmpty())
            {
                composer = stringops::trim(listMap["COMPOSER"].front().to8Bit(true));
            }
        }
    }
}

uint32_t Metadata::getDiscNr()
{
    if (TagLib::MPEG::File* pMpegFile = dynamic_cast<TagLib::MPEG::File*>(m_TagFile.file()))
    {
        if (pMpegFile->ID3v2Tag())
        {
            const TagLib::ID3v2::FrameList& tposList = pMpegFile->ID3v2Tag()->frameList("TPOS");
            if (!tposList.isEmpty())
            {
                return parseDisc(stringops::trim(tposList.front()->toString().to8Bit(true)));
            }
        }
    }
    else if (TagLib::Ogg::Vorbis::File* pVorbisFile = dynamic_cast<TagLib::Ogg::Vorbis::File*>(m_TagFile.file()))
    {
        if (pVorbisFile->tag())
        {
            const TagLib::Ogg::FieldListMap& listMap = pVorbisFile->tag()->fieldListMap();
            if (!listMap["DISCNUMBER"].isEmpty())
            {
                return parseDisc(stringops::trim(listMap["DISCNUMBER"].front().to8Bit(true)));
            }
        }
    }
    else if (TagLib::FLAC::File* pFlacFile = dynamic_cast<TagLib::FLAC::File*>(m_TagFile.file()))
    {
        if (pFlacFile->xiphComment())
        {
            const TagLib::Ogg::FieldListMap& listMap = pFlacFile->xiphComment()->fieldListMap();
            if (!listMap["DISCNUMBER"].isEmpty())
            {
                return parseDisc(stringops::trim(listMap["DISCNUMBER"].front().to8Bit(true)));
            }
        }
    }

    return 0;
}

uint32_t Metadata::getYear()
{
    return m_pTagPtr->year();
}

uint32_t Metadata::getTrackNr()
{
    return m_pTagPtr->track();
}

uint32_t Metadata::getBitRate()
{
    return m_TagFile.audioProperties() ? m_TagFile.audioProperties()->bitrate() : 0;
}

uint32_t Metadata::getSampleRate()
{
    return m_TagFile.audioProperties() ? m_TagFile.audioProperties()->sampleRate() : 0;
}

uint32_t Metadata::getChannels()
{
    return m_TagFile.audioProperties() ? m_TagFile.audioProperties()->channels() : 0;
}

uint32_t Metadata::getDuration()
{
    return m_TagFile.audioProperties() ? m_TagFile.audioProperties()->length() : 0;
}

bool Metadata::getAlbumArt(vector<uint8_t>& data, const vector<string>& albumArtFileList, int32_t albumArtSize)
{
	int32_t size = (albumArtSize == -1) ? ALBUM_ART_DB_SIZE : albumArtSize;

    if (TagLib::MPEG::File* pMpegFile = dynamic_cast<TagLib::MPEG::File*>(m_TagFile.file()))
    {
        if (pMpegFile->ID3v2Tag())
        {
            TagLib::ID3v2::AttachedPictureFrame* pAlbumArt = nullptr;

            TagLib::ID3v2::FrameList list = pMpegFile->ID3v2Tag()->frameList("APIC");
            for (TagLib::ID3v2::FrameList::Iterator iter = list.begin(); iter != list.end(); ++iter)
            {
                TagLib::ID3v2::AttachedPictureFrame* pCurPicture = static_cast<TagLib::ID3v2::AttachedPictureFrame*>(*iter);

                if (pCurPicture->type() == TagLib::ID3v2::AttachedPictureFrame::FrontCover)
                {
                    pAlbumArt = pCurPicture;
                    break;
                }
                else if (pAlbumArt == nullptr)
                {
                    pAlbumArt = pCurPicture;
                }
            }

            if (pAlbumArt != nullptr)
            {
                resizeAlbumArt(reinterpret_cast<uint8_t*>(pAlbumArt->picture().data()), pAlbumArt->picture().size(), size, data);
                return true;
            }
        }
    }

    //no embedded album art found, see if we can find a cover.jpg, ... file
    string dir = fileops::getPathFromFilepath(m_FilePath);

    vector<uint8_t> albumArtData;
    for (size_t i = 0; i < albumArtFileList.size(); ++i)
    {
        string possibleAlbumArt = fileops::combinePath(dir, albumArtFileList[i]);
        if (fileops::pathExists(possibleAlbumArt) && fileops::readFile(albumArtData, possibleAlbumArt))
        {
            log::debug("Art found in: %s", possibleAlbumArt);
            if (resizeAlbumArt(&albumArtData.front(), albumArtData.size(), size, data))
            {
                return true;
            }
        }
    }

    data.clear();
    return false;
}

bool Metadata::resizeAlbumArt(const uint8_t* albumArtData, int32_t dataSize, int32_t albumArtSize, vector<uint8_t>& resizedAlbumArtData)
{
    try
    {
        Magick::Blob resizedBlob;
        Magick::Blob albumArtBlob(albumArtData, dataSize);
        Magick::Image albumArt(albumArtBlob);

        std::stringstream sizeString;
        sizeString << albumArtSize << "x" << albumArtSize;

        albumArt.resize(sizeString.str());
        albumArt.write(&resizedBlob, "PNG");
        albumArt.magick("PNG");
        resizedAlbumArtData.resize(resizedBlob.length());
        memcpy(&resizedAlbumArtData[0], resizedBlob.data(), resizedAlbumArtData.size());
    }
    catch (Magick::Exception& e)
    {
        log::error("Failed to scale image: %s", e.what());
        return false;
    }

    return true;
}

uint32_t Metadata::parseDisc(const std::string& disc)
{
    size_t pos = disc.find('/');
    return stringops::toNumeric<uint32_t>(pos == string::npos ? disc : disc.substr(0, pos));
}

}
