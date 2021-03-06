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

#include "scanner.h"

#include <cassert>
#include <ctime>

#include "config.h"
#include "track.h"
#include "album.h"
#include "albumart.h"
#include "musicdb.h"
#include "utils/stringoperations.h"
#include "utils/log.h"
#include "subscribers.h"
#include "Core/commonstrings.h"

#include "audio/audiometadata.h"
#include "image/imagefactory.h"
#include "image/imageloadstoreinterface.h"

using namespace std;
using namespace utils;
using namespace fileops;

namespace Gejengel
{
    
static constexpr int32_t ALBUM_ART_DB_SIZE = 96;

Scanner::Scanner(MusicDb& db, IScanSubscriber& subscriber, const std::vector<std::string>& albumArtFilenames)
: m_LibraryDb(db)
, m_ScanSubscriber(subscriber)
, m_ScannedFiles(0)
, m_AlbumArtFilenames(albumArtFilenames)
, m_InitialScan(false)
, m_Stop(false)
{
}

Scanner::~Scanner()
{
	cancel();
}

void Scanner::performScan(const string& libraryPath)
{
	m_Stop = false;
	
#ifdef ENABLE_DEBUG
    time_t startTime = time(nullptr);
    log::debug("Starting library scan in: %s", libraryPath);
#endif

    m_InitialScan = m_LibraryDb.getTrackCount() == 0;
    m_ScannedFiles = 0;

    m_ScanSubscriber.scanStart(countFilesInDirectory(libraryPath));
    scan(libraryPath);
    m_ScanSubscriber.scanFinish();

#ifdef ENABLE_DEBUG
	if (m_Stop)
	{
		log::debug("Scan aborted");
	}
    log::debug("Library scan took %d seconds. Scanned %d files.", time(nullptr) - startTime, m_ScannedFiles);
#endif
}

void Scanner::scan(const std::string& dir)
{
    for (auto& entry : Directory(dir))
    {
        if (m_Stop)
        {
            break;
        }
        
        if (entry.type() == FileSystemEntryType::Directory)
        {
            scan(entry.path());
        }
        else if (entry.type() == FileSystemEntryType::File)
        {
            try
            {
                onFile(entry.path());
            }
            catch (std::exception& e)
            {
                log::debug("Ignored file: %s", e.what());
            }
        }
    }
}

void Scanner::cancel()
{
	m_Stop = true;
}

void Scanner::onFile(const string& filepath)
{
    auto info = getFileInfo(filepath);
    
    Track track;
    track.filepath      = filepath;
    track.fileSize      = info.sizeInBytes;
    track.modifiedTime  = info.modifyTime;

    m_ScanSubscriber.scanUpdate(++m_ScannedFiles);

    MusicDb::TrackStatus status = m_LibraryDb.getTrackStatus(filepath, track.modifiedTime);
    if (status == MusicDb::UpToDate)
    {
        return;
    }

    audio::Metadata md(track.filepath, audio::Metadata::ReadAudioProperties::Yes);
    track.artist        = md.getArtist();
    track.albumArtist   = md.getAlbumArtist();
    track.title         = md.getTitle();
    track.album         = md.getAlbum();
    track.genre         = md.getGenre();
    track.composer      = md.getComposer();
    track.year          = md.getYear();
    track.trackNr       = md.getTrackNr();
    track.discNr        = md.getDiscNr();
    track.bitrate       = md.getBitRate();
    track.sampleRate    = md.getSampleRate();
    track.channels      = md.getChannels();
    track.durationInSec = md.getDuration();
    
    if (track.album.empty())    track.album = UNKNOWN_ALBUM;
    if (track.artist.empty())   track.artist = UNKNOWN_ARTIST;
    if (track.title.empty())    track.title = UNKNOWN_TITLE;

    Album album;
    std::string albumId;
    m_LibraryDb.albumExists(track.album, albumId);
    if (!albumId.empty())
    {
        if (!m_LibraryDb.getAlbum(albumId, album))
        {
            assert(false && "The corresponding album could not be found in the database");
        }
    }

    if (albumId.empty() || ((!track.albumArtist.empty()) && track.albumArtist != album.artist))
    {
        album.title         = track.album;
        album.artist        = track.albumArtist.empty() ? track.artist : track.albumArtist;
        album.year          = track.year;
        album.genre         = track.genre;
        album.durationInSec = track.durationInSec;
        album.dateAdded     = m_InitialScan ? track.modifiedTime : time(nullptr);

        AlbumArt art(albumId);
        art.setAlbumArt(md.getAlbumArt());
        processAlbumArt(filepath, art);

        m_LibraryDb.addAlbum(album, art);
    }
    else
    {
        if (!track.albumArtist.empty())
        {
            //if a track has an album artist set, we trust it is the right one
            if (track.albumArtist != album.artist)
            {
                album.artist = track.albumArtist;
            }
        }
        else
        {
            //if no album artist set and we detect different artists for an
            //album, we set the album artist to VARIOUS_ARTISTS
            if (album.artist != VARIOUS_ARTISTS && album.artist != track.artist)
            {
                album.artist = VARIOUS_ARTISTS;
            }
        }

        assert(album.id == albumId);

        AlbumArt art(albumId);

        if (!m_LibraryDb.getAlbumArt(album, art) || status == MusicDb::NeedsUpdate)
        {
            art.setAlbumArt(md.getAlbumArt());
            processAlbumArt(filepath, art);

            if (art.getDataSize() > 0)
            {
                m_LibraryDb.setAlbumArt(albumId, art.getData());
            }
        }

        if (status == MusicDb::DoesntExist)
        {
            album.durationInSec += track.durationInSec;
        }

        if (album.genre.empty())
        {
            album.genre = track.genre;
        }

        m_LibraryDb.updateAlbum(album);
    }

    track.albumId = album.id;
    if (status == MusicDb::DoesntExist)
    {
        log::debug("New track: %s %s", filepath, track.albumId);
        m_LibraryDb.addTrack(track);
    }
    else if (status == MusicDb::NeedsUpdate)
    {
        log::debug("Needs update: %s", filepath);
        m_LibraryDb.updateTrack(track);
    }
}

void Scanner::processAlbumArt(const std::string& filepath, AlbumArt& art)
{
    if (art.getData().empty())
    {
        //no embedded album art found, see if we can find a cover.jpg, ... file
        string dir = fileops::getPathFromFilepath(filepath);

        for (auto& filename : m_AlbumArtFilenames)
        {
            try
            {
                string possibleAlbumArt = fileops::combinePath(dir, filename);
                audio::Metadata::AlbumArt artData;
                artData.data = fileops::readFile(possibleAlbumArt);
                log::debug("Art found in: %s", possibleAlbumArt);
                
                art.setAlbumArt(std::move(artData));
            }
            catch (std::exception&) {}
        }
    }
    
    
    // resize the album art if it is present
    if (!art.getData().empty())
    {
        try
        {
            auto image = image::Factory::createFromData(art.getData());
            image->resize(ALBUM_ART_DB_SIZE, ALBUM_ART_DB_SIZE, image::ResizeAlgorithm::Bilinear);

            auto pngStore = image::Factory::createLoadStore(image::Type::Png);
            art.getData() = pngStore->storeToMemory(*image);
        }
        catch (std::exception& e)
        {
            log::error("Failed to scale image: %s", e.what());
        }
    }
}

}
