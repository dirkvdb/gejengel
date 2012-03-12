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

#ifndef FILESYSTEM_MUSIC_LIBRARY_H
#define FILESYSTEM_MUSIC_LIBRARY_H

#include <string>
#include <vector>
#include <thread>
#include <mutex>

#include "utils/types.h"
#include "musicdb.h"
#include "musiclibrary.h"

namespace Gejengel
{

class Track;
class Album;
class Settings;
class Scanner;
class IScanSubscriber;
class LibrarySource;

class FilesystemMusicLibrary : public MusicLibrary
{
public:
    FilesystemMusicLibrary(const Settings& settings);
    ~FilesystemMusicLibrary();
    
    uint32_t getTrackCount();
    uint32_t getAlbumCount();

    void getTrack(const std::string& id, Track& track);
    void getTrackAsync(const std::string& id, utils::ISubscriber<Track>& subscriber);

    void getTracksFromAlbum(const std::string& albumId, utils::ISubscriber<Track>& subscriber);
    void getTracksFromAlbumAsync(const std::string& albumId, utils::ISubscriber<Track>& subscriber);

    void getFirstTrackFromAlbum(const std::string& albumId, utils::ISubscriber<Track>& subscriber);
    void getFirstTrackFromAlbumAsync(const std::string& albumId, utils::ISubscriber<Track>& subscriber);

    void getAlbum(const std::string& albumId, Album& album);
    void getAlbumAsync(const std::string& albumId, utils::ISubscriber<Album>& subscriber);

    void getAlbums(utils::ISubscriber<Album>& subscriber);
    void getAlbumsAsync(utils::ISubscriber<Album>& subscriber);

    void getRandomTracks(uint32_t trackCount, utils::ISubscriber<Track>& subscriber);
    void getRandomTracksAsync(uint32_t trackCount, utils::ISubscriber<Track>& subscriber);

    void getRandomAlbum(utils::ISubscriber<Track>& subscriber);
    void getRandomAlbumAsync(utils::ISubscriber<Track>& subscriber);

    bool getAlbumArt(const Album& album, AlbumArt& art);

    void scan(bool startFresh, IScanSubscriber& subscriber);
    void search(const std::string& search, utils::ISubscriber<Track>& trackSubscriber, utils::ISubscriber<Album>& albumSubscriber);

    void setSource(const LibrarySource& source);

private:
    void scannerThread(IScanSubscriber& subscriber);

    MusicDb                         m_Db;
    std::string                     m_LibraryPath;
    std::thread                     m_ScannerThread;
    std::mutex						m_ScanMutex;
    std::unique_ptr<Scanner>	    m_Scanner;
    bool							m_Destroy;
};

}

#endif
