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

#ifndef MUSIC_DB_H
#define MUSIC_DB_H

#include <string>
#include <vector>
#include <mutex>

#include "utils/types.h"
#include "utils/subscriber.h"


struct sqlite3;
struct sqlite3_stmt;


namespace Gejengel
{

class Track;
class Album;
class AlbumArt;
class ILibrarySubscriber;


class MusicDb
{
public:
    enum TrackStatus
    {
        DoesntExist,
        NeedsUpdate,
        UpToDate
    };

    MusicDb(const std::string& dbFilepath);
    ~MusicDb();
    
    void setSubscriber(ILibrarySubscriber& subscriber);

    uint32_t getTrackCount();
    uint32_t getAlbumCount();

    void addTrack(const Track& track);
    void updateTrack(const Track& track);
    void addAlbum(Album& album, AlbumArt& art);
    void updateAlbum(const Album& album);

    bool trackExists(const std::string& filepath);
    TrackStatus getTrackStatus(const std::string& filepath, uint32_t modifiedTime);
    void albumExists(const std::string& name, std::string& id);

    bool getTrack(const std::string& id, Track& track);
    bool getTrackWithPath(const std::string& filepath, Track& track);
    bool getAlbum(const std::string& id, Album& album);
    bool getAlbumArt(const Album& album, AlbumArt& art);
    void setAlbumArt(const std::string& albumId, const std::vector<uint8_t>& data);

    void getRandomTracks(uint32_t trackCount, utils::ISubscriber<const Track&>& subscriber);
    void getRandomAlbum(utils::ISubscriber<const Track&>& subscriber);
    void getFirstTrackFromAlbum(const std::string& albumId, utils::ISubscriber<const Track&>& subscriber);
    void getTracksFromAlbum(const std::string& albumId, utils::ISubscriber<const Track&>& subscriber);
    void getAlbums(utils::ISubscriber<const Album&>& subscriber);
    void removeTrack(const std::string& id);
    void removeAlbum(const std::string& id);

    void removeNonExistingFiles();
    void removeNonExistingAlbums();
    void updateAlbumMetaData();

    void searchLibrary(const std::string& search, utils::ISubscriber<const Track&>& trackSubscriber, utils::ISubscriber<const Album&>& albumSubscriber);
    void clearDatabase();

private:
    typedef void (*QueryCallback)(sqlite3_stmt*, void*);
    static void getIdCb(sqlite3_stmt* pStmt, void* pData);
    static void getIdIntCb(sqlite3_stmt* pStmt, void* pData);
    static void getTrackInfoCb(sqlite3_stmt* pStmt, void* pData);
    static void getTrackModificationTimeCb(sqlite3_stmt* pStmt, void* pData);
    static void getTracksCb(sqlite3_stmt* pStmt, void* pData);
    static void getAlbumCb(sqlite3_stmt* pStmt, void* pData);
    static void getAlbumArtCb(sqlite3_stmt* pStmt, void* pData);
    static void getAlbumsCb(sqlite3_stmt* pStmt, void* pData);
    static void getAlbumListCb(sqlite3_stmt* pStmt, void* pData);
    static void getAllAlbumIdsCb(sqlite3_stmt* pStmt, void* pData);
    static void removeNonExistingFilesCb(sqlite3_stmt* pStmt, void* pData);
    static void countCb(sqlite3_stmt* pStmt, void* pData);
    static void addResultCb(sqlite3_stmt* pStmt, void* pData);
    static void searchTracksCb(sqlite3_stmt* pStmt, void* pData);

    static int32_t busyCb(void* pData, int32_t retries);

    void addArtistIfNotExists(const std::string& name, std::string& id);
    void addGenreIfNotExists(const std::string& name, std::string& id);

    void getIdFromTable(const std::string& table, const std::string& name, std::string& id);
    uint32_t getIdFromTable(const std::string& table, const std::string& name);
    void createInitialDatabase();
    uint32_t performQuery(sqlite3_stmt* pStmt, QueryCallback cb = nullptr, void* pData = nullptr, bool finalize = true);
    sqlite3_stmt* createStatement(const char* query);
    void bindValue(sqlite3_stmt* pStmt, const std::string& value, int32_t index);
    void bindValue(sqlite3_stmt* pStmt, uint32_t value, int32_t index);
    void bindValue(sqlite3_stmt* pStmt, const void* pData, uint32_t dataSize, int32_t index);
    
    sqlite3*                m_pDb;
    ILibrarySubscriber*     m_pSubscriber;
    std::recursive_mutex    m_DbMutex;
};

}

#endif
