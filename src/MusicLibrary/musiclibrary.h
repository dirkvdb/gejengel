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

#ifndef MUSIC_LIBRARY_H
#define MUSIC_LIBRARY_H

#include <string>
#include <vector>
#include <mutex>

#include "utils/types.h"
#include "MusicLibrary/musicdb.h"
#include "MusicLibrary/subscribers.h"

namespace Gejengel
{

class Track;
class Album;
class Settings;
class LibrarySource;

class MusicLibrary : public ILibrarySubscriber
{
public:
    MusicLibrary(const Settings& settings);
    virtual ~MusicLibrary();

    virtual uint32_t getTrackCount() = 0;
    virtual uint32_t getAlbumCount() = 0;

    virtual void getTrack(const std::string& id, Track& track) = 0;
    virtual void getTrackAsync(const std::string& id, utils::ISubscriber<Track>& subscriber) = 0;

    virtual void getTracksFromAlbum(const std::string& albumId, utils::ISubscriber<Track>& subscriber) = 0;
    virtual void getTracksFromAlbumAsync(const std::string& albumId, utils::ISubscriber<Track>& subscriber) = 0;

    virtual void getFirstTrackFromAlbum(const std::string& albumId, utils::ISubscriber<Track>& subscriber) = 0;
    virtual void getFirstTrackFromAlbumAsync(const std::string& albumId, utils::ISubscriber<Track>& subscriber) = 0;

    virtual void getAlbum(const std::string& albumId, Album& album) = 0;
    virtual void getAlbumAsync(const std::string& albumId, utils::ISubscriber<Album>& subscriber) = 0;

    virtual void getAlbums(utils::ISubscriber<Album>& subscriber) = 0;
    virtual void getAlbumsAsync(utils::ISubscriber<Album>& subscriber) = 0;

    virtual void getRandomTracks(uint32_t trackCount, utils::ISubscriber<Track>& subscriber) = 0;
    virtual void getRandomTracksAsync(uint32_t trackCount, utils::ISubscriber<Track>& subscriber) = 0;

    virtual void getRandomAlbum(utils::ISubscriber<Track>& subscriber) = 0;
    virtual void getRandomAlbumAsync(utils::ISubscriber<Track>& subscriber) = 0;

    virtual bool getAlbumArt(const Album& album, AlbumArt& art) = 0;

    virtual void scan(bool startFresh, IScanSubscriber& subscriber) = 0;
    virtual void search(const std::string& search, utils::ISubscriber<Track>& trackSubscriber, utils::ISubscriber<Album>& albumSubscriber) = 0;

    virtual void setSource(const LibrarySource& source) = 0;

    void addLibrarySubscriber(ILibrarySubscriber& subscriber);
    std::vector<ILibrarySubscriber*> getSubscribers();
    void clearSubscribers();

    void newTrack(const Track& track);
    void deletedTrack(const std::string& id);
    void updatedTrack(const Track& track);
    void newAlbum(const Album& album);
    void deletedAlbum(const std::string& id);
    void updatedAlbum(const Album& album);
    void libraryCleared();


protected:
    typedef std::vector<ILibrarySubscriber*>::iterator SubscriberIter;

    const Settings&                     m_Settings;
    std::vector<ILibrarySubscriber*>    m_Subscribers;
    std::mutex                          m_SubscribersMutex;
};

}

#endif
