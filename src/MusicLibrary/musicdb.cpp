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

#include "musicdb.h"

#include <stdexcept>
#include <sstream>
#include <cassert>
#include <cstring>
#include <sqlite3.h>
#include <set>
#include <map>

#include "track.h"
#include "album.h"
#include "albumart.h"
#include "subscribers.h"
#include "utils/scopedlock.h"
#include "utils/fileoperations.h"
#include "utils/stringoperations.h"
#include "utils/numericoperations.h"
#include "utils/log.h"
#include "utils/trace.h"

using namespace std;
using namespace utils;

#define BUSY_RETRIES 50

namespace Gejengel
{

MusicDb::MusicDb(const string& dbFilepath)
: m_pDb(nullptr)
, m_pSubscriber(nullptr)
{
    utils::trace("Create Music database");

    if (sqlite3_open(dbFilepath.c_str(), &m_pDb) != SQLITE_OK)
    {
        throw logic_error("Failed to open database: " + dbFilepath);
    }

    if (sqlite3_busy_handler(m_pDb, MusicDb::busyCb, nullptr) != SQLITE_OK)
    {
        throw logic_error("Failed to set busy handler");
    }

    createInitialDatabase();

    utils::trace("Music database loaded");
}

MusicDb::~MusicDb()
{
    if (sqlite3_close(m_pDb) != SQLITE_OK)
    {
        log::error("Failed to close database");
    }
}

void MusicDb::setSubscriber(ILibrarySubscriber& subscriber)
{
    m_pSubscriber = &subscriber;
}

uint32_t MusicDb::getTrackCount()
{
    std::lock_guard<std::recursive_mutex> lock(m_DbMutex);
    uint32_t count;
    performQuery(createStatement("SELECT COUNT(Id) FROM tracks;"), countCb, &count);

    return count;
}

uint32_t MusicDb::getAlbumCount()
{
    std::lock_guard<std::recursive_mutex> lock(m_DbMutex);
    uint32_t count;
    performQuery(createStatement("SELECT COUNT(Id) FROM albums;"), countCb, &count);

    return count;
}

void MusicDb::addTrack(const Track& track)
{
    {
        std::lock_guard<std::recursive_mutex> lock(m_DbMutex);
        sqlite3_stmt* pStmt = createStatement(
            "INSERT INTO tracks "
            "(Id, AlbumId, ArtistId, GenreId, Title, Filepath, Composer, Year, TrackNr, DiscNr, AlbumOrder, Duration, BitRate, SampleRate, Channels, FileSize, ModifiedTime) "
            "VALUES (NULL, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?);");

        uint32_t albumId = getIdFromTable("albums", track.album);
        assert(albumId != 0);

        string artistId, genreId;
        addArtistIfNotExists(track.artist, artistId);
        addGenreIfNotExists(track.genre, genreId);

        bindValue(pStmt, albumId, 1);
        bindValue(pStmt, artistId, 2);
        bindValue(pStmt, genreId, 3);
        bindValue(pStmt, track.title, 4);
        bindValue(pStmt, track.filepath, 5);
        bindValue(pStmt, track.composer, 6);
        bindValue(pStmt, track.year, 7);
        bindValue(pStmt, track.trackNr, 8);
        bindValue(pStmt, track.discNr, 9);
        bindValue(pStmt, track.discNr * 1000 + track.trackNr, 10);
        bindValue(pStmt, track.durationInSec, 11);
        bindValue(pStmt, track.bitrate, 12);
        bindValue(pStmt, track.sampleRate, 13);
        bindValue(pStmt, track.channels, 14);
        bindValue(pStmt, static_cast<uint32_t>(track.fileSize), 15);
        bindValue(pStmt, track.modifiedTime, 16);
        performQuery(pStmt);
    }

    if (m_pSubscriber)
    {
        m_pSubscriber->newTrack(track);
    }
}

void MusicDb::updateTrack(const Track& track)
{
    {
        std::lock_guard<std::recursive_mutex> lock(m_DbMutex);
        sqlite3_stmt* pStmt = createStatement(
            "UPDATE tracks "
            "SET AlbumId=?, ArtistId=?, GenreId=?, Title=?, FilePath=?, Composer=?, Year=?, TrackNr=?, DiscNr=?, AlbumOrder=?, Duration=?, BitRate=?, SampleRate=?, Channels=?, FileSize=?, ModifiedTime=? "
            "WHERE FilePath=?;"
        );

        string albumId, artistId, genreId;

        getIdFromTable("albums", track.album, albumId);
        assert(!albumId.empty());
        addArtistIfNotExists(track.artist, artistId);
        addGenreIfNotExists(track.genre, genreId);

        bindValue(pStmt, albumId, 1);
        bindValue(pStmt, artistId, 2);
        bindValue(pStmt, genreId, 3);
        bindValue(pStmt, track.title, 4);
        bindValue(pStmt, track.filepath, 5);
        bindValue(pStmt, track.composer, 6);
        bindValue(pStmt, track.year, 7);
        bindValue(pStmt, track.trackNr, 8);
        bindValue(pStmt, track.discNr, 9);
        bindValue(pStmt, track.discNr * 1000 + track.trackNr, 10);
        bindValue(pStmt, track.durationInSec, 11);
        bindValue(pStmt, track.bitrate, 12);
        bindValue(pStmt, track.sampleRate, 13);
        bindValue(pStmt, track.channels, 14);
        bindValue(pStmt, track.fileSize, 15);
        bindValue(pStmt, track.modifiedTime, 16);
        bindValue(pStmt, track.filepath, 17);

        performQuery(pStmt);
    }

    if (m_pSubscriber)
    {
        m_pSubscriber->updatedTrack(track);
    }
}

void MusicDb::addAlbum(Album& album, AlbumArt& art)
{
    {
        std::lock_guard<std::recursive_mutex> lock(m_DbMutex);
        if (album.title.empty()) return;

		sqlite3_stmt* pStmt = createStatement(
			"INSERT INTO albums "
			"(Id, Name, AlbumArtist, Year, Duration, DiscCount, DateAdded, CoverImage, GenreId) "
			"VALUES (NULL, ?, ?, ?, ?, ?, ?, ?, ?);");

		string genreId;
		addGenreIfNotExists(album.genre, genreId);

		bindValue(pStmt, album.title, 1);
		bindValue(pStmt, album.artist, 2);
		bindValue(pStmt, album.year, 3);
		bindValue(pStmt, album.durationInSec, 4);
		bindValue(pStmt, 0, 5);
		bindValue(pStmt, album.dateAdded, 6);
		art.getData().empty() ? bindValue(pStmt, "NULL", 7) : bindValue(pStmt, &(art.getData()[0]), art.getData().size(), 7);
		bindValue(pStmt, genreId, 8);

		performQuery(pStmt);

        getIdFromTable("albums", album.title, album.id);
    }

    if (m_pSubscriber)
    {
        m_pSubscriber->newAlbum(album);
    }
}

void MusicDb::updateAlbum(const Album& album)
{
    std::lock_guard<std::recursive_mutex> lock(m_DbMutex);
    sqlite3_stmt* pStmt = createStatement(
        "UPDATE albums "
        "SET Name=?, AlbumArtist=?, Year=?, Duration=?, GenreId=?"
        "WHERE Id=?;"
    );

    string genreId;
    addGenreIfNotExists(album.genre, genreId);

    bindValue(pStmt, album.title, 1);
    bindValue(pStmt, album.artist, 2);
    bindValue(pStmt, album.year, 3);
    bindValue(pStmt, album.durationInSec, 4);
    bindValue(pStmt, genreId, 5);
    bindValue(pStmt, album.id, 6);

    performQuery(pStmt);

    if (m_pSubscriber)
    {
        m_pSubscriber->updatedAlbum(album);
    }
}

void MusicDb::addArtistIfNotExists(const string& name, string& id)
{
    std::lock_guard<std::recursive_mutex> lock(m_DbMutex);
    if (name.empty()) return;

    getIdFromTable("artists", name, id);

    if (id.empty())
    {
        sqlite3_stmt* pStmt = createStatement("INSERT INTO artists (Id, Name) VALUES (NULL, ?);");
        bindValue(pStmt, name, 1);
        performQuery(pStmt);

        getIdFromTable("artists", name, id);
        assert(!id.empty());
    }
}

void MusicDb::addGenreIfNotExists(const string& name, string& id)
{
    std::lock_guard<std::recursive_mutex> lock(m_DbMutex);
    if (name.empty()) return;

    getIdFromTable("genres", name, id);

    if (id.empty())
    {
        sqlite3_stmt* pStmt = createStatement("INSERT INTO genres (Id, Name) VALUES (NULL, ?);");
        bindValue(pStmt, name, 1);
        performQuery(pStmt);

        getIdFromTable("genres", name, id);
        assert(!id.empty());
    }
}


bool MusicDb::trackExists(const string& filepath)
{
    std::lock_guard<std::recursive_mutex> lock(m_DbMutex);
    sqlite3_stmt* pStmt = createStatement("SELECT Id FROM tracks WHERE tracks.Filepath = ?;");
    if (sqlite3_bind_text(pStmt, 1, filepath.c_str(), filepath.size(), SQLITE_STATIC) != SQLITE_OK )
    {
        throw logic_error(string("Failed to bind value: ") + sqlite3_errmsg(m_pDb));
    }

    return performQuery(pStmt) == 1;
}

MusicDb::TrackStatus MusicDb::getTrackStatus(const std::string& filepath, uint32_t modifiedTime)
{
    std::lock_guard<std::recursive_mutex> lock(m_DbMutex);
    sqlite3_stmt* pStmt = createStatement("SELECT ModifiedTime FROM tracks WHERE tracks.Filepath = ?;");
    if (sqlite3_bind_text(pStmt, 1, filepath.c_str(), filepath.size(), SQLITE_STATIC) != SQLITE_OK )
    {
        throw logic_error(string("Failed to bind value: ") + sqlite3_errmsg(m_pDb));
    }

    uint32_t dbModifiedTime;
    int32_t numTracks = performQuery(pStmt, getTrackModificationTimeCb, &dbModifiedTime);
    assert (numTracks <= 1);

    if (numTracks == 0)
    {
        return DoesntExist;
    }
    else if (dbModifiedTime < modifiedTime)
    {
        return NeedsUpdate;
    }
    else
    {
        return UpToDate;
    }
}

void MusicDb::albumExists(const string& name, std::string& id)
{
    std::lock_guard<std::recursive_mutex> lock(m_DbMutex);
    if (name.empty()) return;

    getIdFromTable("albums", name, id);
}

bool MusicDb::getTrack(const std::string& id, Track& track)
{
    std::lock_guard<std::recursive_mutex> lock(m_DbMutex);
    sqlite3_stmt* pStmt = createStatement(
        "SELECT tracks.Id, tracks.albumId, tracks.Title, tracks.Composer, tracks.Filepath, tracks.Year, tracks.TrackNr, tracks.DiscNr, tracks.Duration, tracks.BitRate, tracks.SampleRate, tracks.Channels, tracks.FileSize, tracks.ModifiedTime, artists.Name, albums.Name, albums.AlbumArtist, genres.Name "
        "FROM tracks "
        "LEFT OUTER JOIN albums ON tracks.AlbumId = albums.Id "
        "LEFT OUTER JOIN artists ON tracks.ArtistId = artists.Id "
        "LEFT OUTER JOIN genres ON tracks.GenreId = genres.Id "
        "WHERE tracks.Id = ? ;");

    bindValue(pStmt, id, 1);
    performQuery(pStmt, getTrackInfoCb, &track);

    return !track.id.empty();
}

bool MusicDb::getTrackWithPath(const string& filepath, Track& track)
{
    std::lock_guard<std::recursive_mutex> lock(m_DbMutex);
    sqlite3_stmt* pStmt = createStatement(
        "SELECT tracks.Id, tracks.albumId, tracks.Title, tracks.Composer, tracks.Filepath, tracks.Year, tracks.TrackNr, tracks.DiscNr, tracks.Duration, tracks.BitRate, tracks.SampleRate, tracks.Channels, tracks.FileSize, tracks.ModifiedTime, artists.Name, albums.Name, albums.AlbumArtist, genres.Name "
        "FROM tracks "
        "LEFT OUTER JOIN albums ON tracks.AlbumId = albums.Id "
        "LEFT OUTER JOIN artists ON tracks.ArtistId = artists.Id "
        "LEFT OUTER JOIN genres ON tracks.GenreId = genres.Id "
        "WHERE tracks.Filepath = ? ;");

    bindValue(pStmt, filepath, 1);
    performQuery(pStmt, getTrackInfoCb, &track);

    return !track.id.empty();
}

bool MusicDb::getAlbum(const std::string& albumId, Album& album)
{
    std::lock_guard<std::recursive_mutex> lock(m_DbMutex);
    assert(!albumId.empty());
    sqlite3_stmt* pStmt = createStatement(
        "SELECT albums.Id, albums.Name, albums.AlbumArtist, albums.Year, albums.Duration, albums.DateAdded, genres.Name "
        "FROM albums "
        "LEFT OUTER JOIN genres ON albums.GenreId = genres.Id "
        "WHERE albums.Id = ?;");

    bindValue(pStmt, albumId, 1);
    performQuery(pStmt, getAlbumCb, &album);
    return !album.id.empty();
}

void MusicDb::getRandomTracks(uint32_t trackCount, utils::ISubscriber<Track>& subscriber)
{
    std::lock_guard<std::recursive_mutex> lock(m_DbMutex);

    sqlite3_stmt* pStmt = createStatement(
        "SELECT tracks.Id, tracks.AlbumId, tracks.Title, tracks.Composer, tracks.Filepath, tracks.Year, tracks.TrackNr, tracks.DiscNr, tracks.Duration, tracks.BitRate, tracks.SampleRate, tracks.Channels, tracks.FileSize, tracks.ModifiedTime, artists.Name, albums.Name, albums.AlbumArtist, genres.Name "
        "FROM tracks "
        "LEFT OUTER JOIN albums ON tracks.AlbumId = albums.Id "
        "LEFT OUTER JOIN artists ON tracks.ArtistId = artists.Id "
        "LEFT OUTER JOIN genres ON tracks.GenreId = genres.Id "
        "ORDER BY RANDOM() LIMIT ?;");

    bindValue(pStmt, trackCount, 1);
    performQuery(pStmt, getTracksCb, &subscriber);
}

void MusicDb::getRandomAlbum(utils::ISubscriber<Track>& subscriber)
{
    std::lock_guard<std::recursive_mutex> lock(m_DbMutex);

    sqlite3_stmt* pStmt = createStatement(
        "SELECT Id "
        "FROM albums "
        "ORDER BY RANDOM() LIMIT 1;");

    std::string id;
    performQuery(pStmt, getIdCb, &id);

    getTracksFromAlbum(id, subscriber);
}

void MusicDb::getFirstTrackFromAlbum(const std::string& albumId, utils::ISubscriber<Track>& subscriber)
{
    std::lock_guard<std::recursive_mutex> lock(m_DbMutex);
    sqlite3_stmt* pStmt = createStatement(
        "SELECT tracks.Id, tracks.AlbumId, tracks.Title, tracks.Composer, tracks.Filepath, tracks.Year, tracks.TrackNr, tracks.DiscNr, tracks.Duration, tracks.BitRate, tracks.SampleRate, tracks.Channels, tracks.FileSize, tracks.ModifiedTime, artists.Name, albums.Name, albums.AlbumArtist, genres.Name "
        "FROM tracks "
        "LEFT OUTER JOIN albums ON tracks.AlbumId = albums.Id "
        "LEFT OUTER JOIN artists ON tracks.ArtistId = artists.Id "
        "LEFT OUTER JOIN genres ON tracks.GenreId = genres.Id "
        "WHERE albums.Id = ? "
        "ORDER BY tracks.AlbumOrder LIMIT 1;");

    bindValue(pStmt, albumId, 1);
    performQuery(pStmt, getTracksCb, &subscriber);
}

void MusicDb::getTracksFromAlbum(const std::string& albumId, utils::ISubscriber<Track>& subscriber)
{
    std::lock_guard<std::recursive_mutex> lock(m_DbMutex);
    sqlite3_stmt* pStmt = createStatement(
        "SELECT tracks.Id, tracks.AlbumId, tracks.Title, tracks.Composer, tracks.Filepath, tracks.Year, tracks.TrackNr, tracks.DiscNr, tracks.Duration, tracks.BitRate, tracks.SampleRate, tracks.Channels, tracks.FileSize, tracks.ModifiedTime, artists.Name, albums.Name, albums.AlbumArtist, genres.Name "
        "FROM tracks "
        "LEFT OUTER JOIN albums ON tracks.AlbumId = albums.Id "
        "LEFT OUTER JOIN artists ON tracks.ArtistId = artists.Id "
        "LEFT OUTER JOIN genres ON tracks.GenreId = genres.Id "
        "WHERE albums.Id = ? "
        "ORDER BY tracks.AlbumOrder;");

    bindValue(pStmt, albumId, 1);

    performQuery(pStmt, getTracksCb, &subscriber);
}

void MusicDb::getAlbums(utils::ISubscriber<Album>& subscriber)
{
    std::lock_guard<std::recursive_mutex> lock(m_DbMutex);
    performQuery(createStatement("SELECT albums.Id, albums.Name, albums.AlbumArtist, albums.Year, albums.Duration, albums.DateAdded, genres.Name "
                                 "FROM albums "
                                 "LEFT OUTER JOIN genres ON albums.GenreId = genres.Id;"), getAlbumsCb, &subscriber);
    subscriber.finalItemReceived();
}

bool MusicDb::getAlbumArt(const Album& album, AlbumArt& art)
{
    std::lock_guard<std::recursive_mutex> lock(m_DbMutex);
    sqlite3_stmt* pStmt = createStatement("SELECT CoverImage FROM albums WHERE Id = ?;");
    bindValue(pStmt, album.id, 1);
    performQuery(pStmt, getAlbumArtCb, &art.getData());

    return !art.getData().empty();
}

void MusicDb::setAlbumArt(const std::string& albumId, const std::vector<uint8_t>& data)
{
    std::lock_guard<std::recursive_mutex> lock(m_DbMutex);
    sqlite3_stmt* pStmt = createStatement("UPDATE albums SET CoverImage=? WHERE Id=?;");
    bindValue(pStmt, &data.front(), data.size(), 1);
    bindValue(pStmt, albumId, 2);
    performQuery(pStmt);
}

void MusicDb::removeTrack(const std::string& id)
{
    {
        std::lock_guard<std::recursive_mutex> lock(m_DbMutex);
        sqlite3_stmt* pStmt = createStatement("DELETE from tracks WHERE Id = ?");
        bindValue(pStmt, id, 1);
        performQuery(pStmt);
    }
    
    if (m_pSubscriber)
    {
        m_pSubscriber->deletedTrack(id);
    }
}

void MusicDb::removeAlbum(const std::string& id)
{
    {
        std::lock_guard<std::recursive_mutex> lock(m_DbMutex);
        sqlite3_stmt* pStmt = createStatement("DELETE from albums WHERE Id = ?");
        bindValue(pStmt, id, 1);
        performQuery(pStmt);
    }
    
    if (m_pSubscriber)
    {
        m_pSubscriber->deletedAlbum(id);
    }
}

void MusicDb::removeNonExistingFiles()
{
    std::lock_guard<std::recursive_mutex> lock(m_DbMutex);
    vector<uint32_t> files;
    performQuery(createStatement("SELECT Id, Filepath from tracks;"), removeNonExistingFilesCb, &files);

    for (size_t i = 0; i < files.size(); ++i)
    {
        log::debug("Removed deleted file from database:", files[i]);
        removeTrack(numericops::toString(files[i]));
    }
}

void MusicDb::removeNonExistingAlbums()
{
    std::lock_guard<std::recursive_mutex> lock(m_DbMutex);
    set<uint32_t> albums;
    performQuery(createStatement("SELECT Id from albums;"), getAllAlbumIdsCb, &albums);

    sqlite3_stmt* pStmt = createStatement(
        "SELECT COUNT(Id) "
        "FROM tracks "
        "WHERE tracks.AlbumId = ?;");

    vector<uint32_t> albumsToBeRemoved;
    for (set<uint32_t>::iterator iter = albums.begin(); iter != albums.end(); ++iter)
    {
        uint32_t count;
        bindValue(pStmt, *iter, 1);
        performQuery(pStmt, countCb, &count, false);

        if (count == 0)
        {
            log::debug("Removed album without tracks from database:",  *iter);
            albumsToBeRemoved.push_back(*iter);
        }

        if (sqlite3_reset(pStmt) != SQLITE_OK)
        {
            sqlite3_finalize(pStmt);
            throw logic_error(string("Failed to reset statement: ") + sqlite3_errmsg(m_pDb));
        }
        if (sqlite3_clear_bindings(pStmt) != SQLITE_OK)
        {
            sqlite3_finalize(pStmt);
            throw logic_error(string("Failed to clear bindings: ") + sqlite3_errmsg(m_pDb));
        }
    }
    sqlite3_finalize(pStmt);

    for (size_t i = 0; i < albumsToBeRemoved.size(); ++i)
    {
        removeAlbum(numericops::toString(albumsToBeRemoved[i]));
    }
}

void MusicDb::updateAlbumMetaData()
{
    vector<uint32_t> albums;
    performQuery(createStatement("SELECT Id from albums;"), getAllAlbumIdsCb, &albums);

    sqlite3_stmt* pStmt = createStatement(
        "SELECT Duration DiscNr "
        "FROM tracks "
        "WHERE tracks.AlbumId = ?;");

    map<uint32_t, uint32_t> albumDurations;
    for (size_t i = 0; i < albums.size(); ++i)
    {
        uint32_t totalDuration = 0;
        bindValue(pStmt, albums[i], 1);
        performQuery(pStmt, addResultCb, &totalDuration, false);

        albumDurations[albums[i]] = totalDuration;

        if (sqlite3_reset(pStmt) != SQLITE_OK)
        {
            sqlite3_finalize(pStmt);
            throw logic_error(string("Failed to reset statement: ") + sqlite3_errmsg(m_pDb));
        }
        if (sqlite3_clear_bindings(pStmt) != SQLITE_OK)
        {
            sqlite3_finalize(pStmt);
            throw logic_error(string("Failed to clear bindings: ") + sqlite3_errmsg(m_pDb));
        }
    }

    sqlite3_finalize(pStmt);

    pStmt = createStatement(
        "UPDATE albums "
        "SET Duration=? "
        "WHERE albums.Id = ?;");

    for (map<uint32_t, uint32_t>::iterator iter = albumDurations.begin(); iter != albumDurations.end(); ++iter)
    {
        bindValue(pStmt, iter->second, 1);
        bindValue(pStmt, iter->first, 2);
        performQuery(pStmt, nullptr, nullptr, false);

        if (sqlite3_reset(pStmt) != SQLITE_OK)
        {
            sqlite3_finalize(pStmt);
            throw logic_error(string("Failed to reset statement: ") + sqlite3_errmsg(m_pDb));
        }
        if (sqlite3_clear_bindings(pStmt) != SQLITE_OK)
        {
            sqlite3_finalize(pStmt);
            throw logic_error(string("Failed to clear bindings: ") + sqlite3_errmsg(m_pDb));
        }
    }

    sqlite3_finalize(pStmt);
}

class SearchTrackData
{
public:
    SearchTrackData(utils::ISubscriber<Track>& trackSubscriber, set<uint32_t>& foundIds)
    : subscriber(trackSubscriber)
    , ids(foundIds)
    {
    }

    utils::ISubscriber<Track>& subscriber;
    set<uint32_t>&      ids;
};

void MusicDb::searchLibrary(const std::string& search, utils::ISubscriber<Track>& trackSubscriber, utils::ISubscriber<Album>& albumSubscriber)
{
    std::lock_guard<std::recursive_mutex> lock(m_DbMutex);

    set<uint32_t> albumIds;
    sqlite3_stmt* pStmt = createStatement(
        "SELECT Id "
        "FROM albums "
        "WHERE albums.AlbumArtist LIKE (SELECT '%' || ?1 || '%') "
        "OR albums.Name LIKE (SELECT '%' || ?1 || '%');");
    bindValue(pStmt, search, 1);
    performQuery(pStmt, getAllAlbumIdsCb, &albumIds);

    SearchTrackData data(trackSubscriber, albumIds);
    pStmt = createStatement(
        "SELECT tracks.Id, tracks.AlbumId, tracks.Title, tracks.Composer, tracks.Filepath, tracks.Year, tracks.TrackNr, tracks.DiscNr, tracks.Duration, tracks.BitRate, tracks.SampleRate, tracks.Channels, tracks.FileSize, tracks.ModifiedTime, artists.Name, albums.Name, albums.AlbumArtist, genres.Name "
        "FROM tracks "
        "LEFT OUTER JOIN albums ON tracks.AlbumId = albums.Id "
        "LEFT OUTER JOIN artists ON tracks.ArtistId = artists.Id "
        "LEFT OUTER JOIN genres ON tracks.GenreId = genres.Id "
        "WHERE artists.Name LIKE (SELECT '%' || ?1 || '%')"
        "OR tracks.title LIKE (SELECT '%' || ?1 || '%');");


    bindValue(pStmt, search, 1);
    performQuery(pStmt, searchTracksCb, &data);

    vector<Album> albums;
    pStmt = createStatement(
        "SELECT albums.Id, albums.Name, albums.AlbumArtist, albums.Year, albums.Duration, albums.DateAdded, genres.Name "
        "FROM albums "
    	"LEFT OUTER JOIN genres ON albums.GenreId = genres.Id "
        "WHERE albums.Id=?;");

    for (set<uint32_t>::iterator iter = data.ids.begin(); iter != data.ids.end(); ++iter)
    {
        bindValue(pStmt, *iter, 1);
        performQuery(pStmt, getAlbumListCb, &albums, false);

        if (sqlite3_reset(pStmt) != SQLITE_OK)
        {
            sqlite3_finalize(pStmt);
            throw logic_error(string("Failed to reset statement: ") + sqlite3_errmsg(m_pDb));
        }
        if (sqlite3_clear_bindings(pStmt) != SQLITE_OK)
        {
            sqlite3_finalize(pStmt);
            throw logic_error(string("Failed to clear bindings: ") + sqlite3_errmsg(m_pDb));
        }
    }
    sqlite3_finalize(pStmt);

    for (size_t i = 0; i < albums.size(); ++i)
    {
        albumSubscriber.onItem(albums[i]);
    }
}

void MusicDb::clearDatabase()
{
    {
        std::lock_guard<std::recursive_mutex> lock(m_DbMutex);
        performQuery(createStatement("DROP INDEX IF EXISTS tracks.pathIndex;"));
        performQuery(createStatement("DROP TABLE IF EXISTS albums;"));
        performQuery(createStatement("DROP TABLE IF EXISTS artists;"));
        performQuery(createStatement("DROP TABLE IF EXISTS genres;"));
        performQuery(createStatement("DROP TABLE IF EXISTS tracks;"));

        createInitialDatabase();
    }
    
    if (m_pSubscriber)
    {
        m_pSubscriber->libraryCleared();
    }
}

void MusicDb::createInitialDatabase()
{
	log::debug("Create initial database");
    std::lock_guard<std::recursive_mutex> lock(m_DbMutex);
    log::debug("Lock acquired");
    performQuery(createStatement("CREATE TABLE IF NOT EXISTS albums(Id INTEGER PRIMARY KEY, GenreId INTEGER, Name TEXT, AlbumArtist TEXT, Year INTEGER, Duration Integer, DiscCount INTEGER, DateAdded INTEGER, CoverImage BLOB, FOREIGN KEY (GenreId) REFERENCES genres(Id));"));
    performQuery(createStatement("CREATE TABLE IF NOT EXISTS artists(Id INTEGER PRIMARY KEY, Name TEXT UNIQUE);"));
    performQuery(createStatement("CREATE TABLE IF NOT EXISTS genres(Id INTEGER PRIMARY KEY, Name TEXT UNIQUE);"));
    performQuery(createStatement("CREATE TABLE IF NOT EXISTS tracks(Id INTEGER PRIMARY KEY, AlbumId INTEGER, ArtistId INTEGER, GenreId INTEGER, Title TEXT, Filepath TEXT UNIQUE, Composer TEXT, Year INTEGER, TrackNr INTEGER, DiscNr INTEGER, AlbumOrder INTEGER, Duration INTEGER, BitRate INTEGER, SampleRate INTEGER, Channels INTEGER, FileSize INTEGER, ModifiedTime INTEGER, FOREIGN KEY (AlbumId) REFERENCES albums(Id), FOREIGN KEY (ArtistId) REFERENCES artists(Id), FOREIGN KEY (GenreId) REFERENCES genres(Id));"));

    performQuery(createStatement("CREATE INDEX IF NOT EXISTS pathIndex ON tracks (Filepath);"));

    log::debug("database created");
}

uint32_t MusicDb::performQuery(sqlite3_stmt* pStmt, QueryCallback cb, void* pData, bool finalize)
{
    uint32_t rowCount = 0;

    int32_t rc;
    while ((rc = sqlite3_step(pStmt)) != SQLITE_DONE)
    {
        switch(rc)
        {
        case SQLITE_BUSY:
            sqlite3_finalize(pStmt);
            throw logic_error("Failed to execute statement: SQL is busy");
            break;
        case SQLITE_ERROR:
            sqlite3_finalize(pStmt);
            throw logic_error(string("Failed to execute statement: ") + sqlite3_errmsg(m_pDb));
            break;
        case SQLITE_ROW:
            if (cb != nullptr) cb(pStmt, pData);
            ++rowCount;
            break;
        default:
            sqlite3_finalize(pStmt);
            throw logic_error("FIXME: unhandled return value of sql statement: " + numericops::toString(rc));
        }
    }

    if (finalize)
    {
        sqlite3_finalize(pStmt);
    }
    return rowCount;
}

sqlite3_stmt* MusicDb::createStatement(const char* query)
{
    sqlite3_stmt* pStmt;

    if (sqlite3_prepare_v2(m_pDb, query, -1, &pStmt, 0) != SQLITE_OK)
    {
        throw logic_error(string("Failed to prepare sql statement (") + sqlite3_errmsg(m_pDb) + "): " + query);
    }

    return pStmt;
}

void MusicDb::bindValue(sqlite3_stmt* pStmt, const string& value, int32_t index)
{
    if (value.empty())
    {
        if (sqlite3_bind_null(pStmt, index) != SQLITE_OK)
        {
            throw logic_error(string("Failed to bind string value as NULL: ") + sqlite3_errmsg(m_pDb));
        }
    }
    else if (sqlite3_bind_text(pStmt, index, value.c_str(), value.size(), SQLITE_STATIC) != SQLITE_OK)
    {
        throw logic_error(string("Failed to bind string value: ") + sqlite3_errmsg(m_pDb));
    }
}

void MusicDb::bindValue(sqlite3_stmt* pStmt, uint32_t value, int32_t index)
{
    if (sqlite3_bind_int(pStmt, index, value) != SQLITE_OK )
    {
        throw logic_error(string("Failed to bind int value: ") + sqlite3_errmsg(m_pDb));
    }
}

void MusicDb::bindValue(sqlite3_stmt* pStmt, const void* pData, uint32_t dataSize, int32_t index)
{
    if (pData == nullptr)
    {
        if (sqlite3_bind_null(pStmt, index) != SQLITE_OK)
        {
            throw logic_error(string("Failed to bind blob value as NULL: ") + sqlite3_errmsg(m_pDb));
        }
    }
    else if (sqlite3_bind_blob(pStmt, index, pData, dataSize, SQLITE_TRANSIENT) != SQLITE_OK)
    {
        throw logic_error(string("Failed to bind int value: ") + sqlite3_errmsg(m_pDb));
    }
}

void MusicDb::getIdFromTable(const string& table, const string& name, string& id)
{
    stringstream query;
    query << "SELECT Id FROM " << table << " WHERE Name = ?;";
    sqlite3_stmt* pStmt = createStatement(("SELECT Id FROM " + table + " WHERE Name = ?;").c_str());
    bindValue(pStmt, name, 1);

    id.clear();
    performQuery(pStmt, getIdCb, &id);
}

uint32_t MusicDb::getIdFromTable(const string& table, const string& name)
{
    stringstream query;
    query << "SELECT Id FROM " << table << " WHERE Name = ?;";
    sqlite3_stmt* pStmt = createStatement(("SELECT Id FROM " + table + " WHERE Name = ?;").c_str());
    bindValue(pStmt, name, 1);

    uint32_t id = 0;
    performQuery(pStmt, getIdIntCb, &id);
    return id;
}

void MusicDb::getIdCb(sqlite3_stmt* pStmt, void* pData)
{
    assert(sqlite3_column_count(pStmt) == 1);
    assert(sqlite3_column_text(pStmt, 0));

    string* pId = reinterpret_cast<string*>(pData);
    *pId = reinterpret_cast<const char*>(sqlite3_column_text(pStmt, 0));
}

void MusicDb::getIdIntCb(sqlite3_stmt* pStmt, void* pData)
{
    assert(sqlite3_column_count(pStmt) == 1);
    assert(sqlite3_column_text(pStmt, 0));

    uint32_t* pId = reinterpret_cast<uint32_t*>(pData);
    *pId = sqlite3_column_int(pStmt, 0);
}

static void getStringFromColumn(sqlite3_stmt* pStmt, int column, string& aString)
{
    const char* pString = reinterpret_cast<const char*>(sqlite3_column_text(pStmt, column));
    if (pString != nullptr)
    {
        aString = pString;
    }
}

void MusicDb::getTrackInfoCb(sqlite3_stmt* pStmt, void* pData)
{
    assert(sqlite3_column_count(pStmt) == 18);

    Track* pTrack = reinterpret_cast<Track*>(pData);

    getStringFromColumn(pStmt, 0, pTrack->id);
    getStringFromColumn(pStmt, 1, pTrack->albumId);
    getStringFromColumn(pStmt, 2, pTrack->title);
    getStringFromColumn(pStmt, 3, pTrack->composer);
    getStringFromColumn(pStmt, 4, pTrack->filepath);

    pTrack->year             = sqlite3_column_int(pStmt, 5);
    pTrack->trackNr          = sqlite3_column_int(pStmt, 6);
    pTrack->discNr           = sqlite3_column_int(pStmt, 7);
    pTrack->durationInSec    = sqlite3_column_int(pStmt, 8);
    pTrack->bitrate          = sqlite3_column_int(pStmt, 9);
    pTrack->sampleRate       = sqlite3_column_int(pStmt, 10);
    pTrack->channels         = sqlite3_column_int(pStmt, 11);
    pTrack->fileSize         = sqlite3_column_int(pStmt, 12);
    pTrack->modifiedTime     = sqlite3_column_int(pStmt, 13);

    getStringFromColumn(pStmt, 14, pTrack->artist);
    getStringFromColumn(pStmt, 15, pTrack->album);
    getStringFromColumn(pStmt, 16, pTrack->albumArtist);
    getStringFromColumn(pStmt, 17, pTrack->genre);
}

void MusicDb::getTrackModificationTimeCb(sqlite3_stmt* pStmt, void* pData)
{
    assert(sqlite3_column_count(pStmt) == 1);

    uint32_t* pModifiedTime = reinterpret_cast<uint32_t*>(pData);
    *pModifiedTime = sqlite3_column_int(pStmt, 0);
}

void MusicDb::getTracksCb(sqlite3_stmt* pStmt, void* pData)
{
	utils::ISubscriber<Track>* pSubscriber = reinterpret_cast<utils::ISubscriber<Track>*>(pData);

    Track track;
    getTrackInfoCb(pStmt, &track);
    pSubscriber->onItem(track);
}

void MusicDb::searchTracksCb(sqlite3_stmt* pStmt, void* pData)
{
    SearchTrackData* pSearchData = reinterpret_cast<SearchTrackData*>(pData);

    Track track;
    getTrackInfoCb(pStmt, &track);
    pSearchData->subscriber.onItem(track);
    pSearchData->ids.insert(stringops::toNumeric<uint32_t>(track.albumId));
}

static void getDataFromColumn(sqlite3_stmt* pStmt, int column, vector<uint8_t>& data)
{
	int type = sqlite3_column_type(pStmt, column);
	if (type == SQLITE_BLOB)
	{
		const void* dataPtr = sqlite3_column_blob(pStmt, column);
		int size = sqlite3_column_bytes(pStmt, column);
		if (size == 0)
		{
			return;
		}

		data.resize(size);
		memcpy(&data[0], dataPtr, size);
	}
}

void MusicDb::getAlbumCb(sqlite3_stmt* pStmt, void* pData)
{
    assert(sqlite3_column_count(pStmt) == 7);

    Album* pAlbum = reinterpret_cast<Album*>(pData);

    getStringFromColumn(pStmt, 0, pAlbum->id);
    getStringFromColumn(pStmt, 1, pAlbum->title);
    getStringFromColumn(pStmt, 2, pAlbum->artist);
    pAlbum->year = sqlite3_column_int(pStmt, 3);
    pAlbum->durationInSec = sqlite3_column_int(pStmt, 4);
    pAlbum->dateAdded = sqlite3_column_int(pStmt, 5);
    getStringFromColumn(pStmt, 6, pAlbum->genre);
}

void MusicDb::getAlbumArtCb(sqlite3_stmt* pStmt, void* pData)
{
    assert(sqlite3_column_count(pStmt) == 1);
    vector<uint8_t>* pAlbumArtData = reinterpret_cast<vector<uint8_t>*>(pData);

    getDataFromColumn(pStmt, 0, *pAlbumArtData);
}

void MusicDb::getAlbumsCb(sqlite3_stmt* pStmt, void* pData)
{
    assert(sqlite3_column_count(pStmt) == 7);

    utils::ISubscriber<Album>* pSubscriber = reinterpret_cast<utils::ISubscriber<Album>*>(pData);

    Album album;
    getAlbumCb(pStmt, &album);
    pSubscriber->onItem(album);
}

void MusicDb::getAlbumListCb(sqlite3_stmt* pStmt, void* pData)
{
    assert(sqlite3_column_count(pStmt) == 7);

    vector<Album>* pAlbums = reinterpret_cast<vector<Album>*>(pData);

    Album album;
    pAlbums->push_back(album);
    getAlbumCb(pStmt, &pAlbums->back());
}

void MusicDb::removeNonExistingFilesCb(sqlite3_stmt* pStmt, void* pData)
{
    assert(sqlite3_column_count(pStmt) == 2);

    vector<uint32_t>* pFiles = reinterpret_cast<vector<uint32_t>*>(pData);

    string path;
    getStringFromColumn(pStmt, 1, path);

    if (!fileops::pathExists(path))
    {
        pFiles->push_back(sqlite3_column_int(pStmt, 0));
    }
}

void MusicDb::getAllAlbumIdsCb(sqlite3_stmt* pStmt, void* pData)
{
    assert(sqlite3_column_count(pStmt) == 1);

    set<uint32_t>* pAlbums = reinterpret_cast<set<uint32_t>*>(pData);
    pAlbums->insert(sqlite3_column_int(pStmt, 0));
}

void MusicDb::countCb(sqlite3_stmt* pStmt, void* pData)
{
    assert(sqlite3_column_count(pStmt) == 1);

    uint32_t* pCount = reinterpret_cast<uint32_t*>(pData);
    *pCount = sqlite3_column_int(pStmt, 0);
}

void MusicDb::addResultCb(sqlite3_stmt* pStmt, void* pData)
{
    assert(sqlite3_column_count(pStmt) == 1);

    uint32_t* pSum = reinterpret_cast<uint32_t*>(pData);
    *pSum += sqlite3_column_int(pStmt, 0);
}

int32_t MusicDb::busyCb(void* pData, int32_t retries)
{
    log::debug("DB busy: attempt", retries);
    if (retries > BUSY_RETRIES)
    {
        return 0;
    }

    return 1;
}

}
