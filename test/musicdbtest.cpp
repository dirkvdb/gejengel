#include <gtest/gtest.h>

#include <sqlite3.h>

#include "testclasses.h"
#include "MusicLibrary/musicdb.h"
#include "MusicLibrary/track.h"

#include "Utils/stringoperations.h"
#include "Utils/numericoperations.h"

#define TEST_DB "test.db"

using namespace std;
using namespace Gejengel;

class MusicDbTest : public testing::Test
{
    protected:

    virtual void SetUp()
    {
        track.id             = "1";
        track.filepath       = "aPath with' quote";
        track.artist         = "anArtist";
        track.title          = "aTitle";
        track.album          = "anAlbum";
        track.albumArtist    = "anAlbumArtist";
        track.genre          = "aGenre";
        track.composer       = "composer";
        track.year           = 2000;
        track.trackNr        = 1;
        track.discNr         = 1;
        track.durationInSec  = 42;
        track.bitrate        = 320;
        track.sampleRate     = 44100;
        track.channels       = 2;
        track.fileSize       = 123456;
        track.modifiedTime   = 10000;

        album.artist    = track.albumArtist;
        album.title     = track.album;

        album.coverData = std::vector<uint8_t>(16, 5);

        pDb = new MusicDb(TEST_DB);
        pDb->setSubscriber(subscriber);
        pDb->addAlbum(album);

        track.albumId = album.id;
    }

    virtual void TearDown()
    {
        subscriber.clearLists();
        delete pDb;
        remove(TEST_DB);
    }

    MusicDb* pDb;
    Track track;
    Album album;
    LibrarySubscriberMock subscriber;
};

TEST_F(MusicDbTest, CreateInitialDbTest)
{
    sqlite3* pSqlite;

    sqlite3_open(TEST_DB, &pSqlite);
    EXPECT_EQ(SQLITE_OK, sqlite3_exec(pSqlite, "SELECT * FROM albums;", nullptr, nullptr, nullptr));
    EXPECT_EQ(SQLITE_OK, sqlite3_exec(pSqlite, "SELECT * FROM artists;", nullptr, nullptr, nullptr));
    EXPECT_EQ(SQLITE_OK, sqlite3_exec(pSqlite, "SELECT * FROM genres;", nullptr, nullptr, nullptr));
    EXPECT_EQ(SQLITE_OK, sqlite3_exec(pSqlite, "SELECT * FROM tracks;", nullptr, nullptr, nullptr));
    EXPECT_EQ(SQLITE_ERROR, sqlite3_exec(pSqlite, "SELECT * FROM dummydb;", nullptr, nullptr, nullptr)); //just make sure it can also fail
    sqlite3_close(pSqlite);
}

TEST_F(MusicDbTest, TrackExists)
{
    pDb->addTrack(track);

    EXPECT_TRUE(pDb->trackExists(track.filepath));
    EXPECT_FALSE(pDb->trackExists("/some/path/track.mp3"));
}

TEST_F(MusicDbTest, GetTrackStatus)
{
    pDb->addTrack(track);

    EXPECT_EQ(MusicDb::DoesntExist, pDb->getTrackStatus("/new/path", 1234));
    EXPECT_EQ(MusicDb::NeedsUpdate, pDb->getTrackStatus(track.filepath, 10001));
    EXPECT_EQ(MusicDb::UpToDate, pDb->getTrackStatus(track.filepath, 10000));
    EXPECT_EQ(MusicDb::UpToDate, pDb->getTrackStatus(track.filepath, 9999));
}


TEST_F(MusicDbTest, AddTrack)
{
    Track returnedTrack;
    pDb->addTrack(track);
    ASSERT_TRUE(pDb->getTrackWithPath(track.filepath, returnedTrack));
    EXPECT_EQ(track, returnedTrack);
}

TEST_F(MusicDbTest, AddTwoTracks)
{
    Track returnedTrack;

    pDb->addTrack(track);
    track.filepath = "anotherPath";
    track.title = "anotherTitle";
    pDb->addTrack(track);
    ASSERT_TRUE(pDb->getTrackWithPath(track.filepath, returnedTrack));
    track.id = "2";
    EXPECT_EQ(track, returnedTrack);
}

TEST_F(MusicDbTest, AddIncompleteLibraryTrack)
{
    Track returnedTrack;

    track.album = "Unknown Album";
    track.artist = "Unknown Artist";
    track.albumArtist = "";
    track.genre = "";

    Album anAlbum;
    anAlbum.title = track.album;
    anAlbum.artist = track.albumArtist;
    pDb->addAlbum(anAlbum);
    pDb->addTrack(track);
    ASSERT_TRUE(pDb->getTrackWithPath(track.filepath, returnedTrack));
    track.albumId = anAlbum.id;
    EXPECT_EQ(track, returnedTrack);
}

TEST_F(MusicDbTest, TrackCount)
{
    EXPECT_EQ(0, pDb->getTrackCount());

    pDb->addTrack(track);
    EXPECT_EQ(1, pDb->getTrackCount());

    track.filepath = "otherpath";
    pDb->addTrack(track);
    EXPECT_EQ(2, pDb->getTrackCount());
}

TEST_F(MusicDbTest, RemoveNonExistingTracks)
{
    pDb->addTrack(track); //non existing path
    track.filepath = TEST_DB;
    track.id = "otherid";
    pDb->addTrack(track); //existing path
    EXPECT_EQ(2, pDb->getTrackCount());

    pDb->removeNonExistingFiles();
    EXPECT_EQ(1, pDb->getTrackCount());
    EXPECT_EQ(1, subscriber.deletedTracks.size());
    EXPECT_EQ("1", subscriber.deletedTracks[0]);
}

TEST_F(MusicDbTest, GetAlbums)
{
    AlbumSubscriberMock albumSubscriber;
    Album anAlbum;

    pDb->addTrack(track);
    track.album = "anotherAlbum";
    track.filepath = "anotherPath";
    anAlbum.title = track.album;
    anAlbum.artist = track.albumArtist;
    pDb->addAlbum(anAlbum);
    pDb->addTrack(track);

    pDb->getAlbums(albumSubscriber);
    ASSERT_EQ(2, albumSubscriber.albums.size());

    Album album;
    album.artist = "anAlbumArtist";
    album.title = "anAlbum";
    EXPECT_EQ(album, albumSubscriber.albums[0]);

    album.title = "anotherAlbum";
    EXPECT_EQ(album, albumSubscriber.albums[1]);
}

TEST_F(MusicDbTest, GetFirstSongFromAlbum)
{
    TrackSubscriberMock trackSubscriber;

    pDb->addTrack(track);

    Track otherTrack = track;
    otherTrack.filepath = "anotherPath";
    otherTrack.title = "anotherTitle";
    otherTrack.id++;
    pDb->addTrack(otherTrack);

    pDb->getFirstTrackFromAlbum(track.albumId, trackSubscriber);
    ASSERT_EQ(1, trackSubscriber.tracks.size());
    EXPECT_EQ(track, trackSubscriber.tracks[0]);
}

TEST_F(MusicDbTest, GetSongsFromAlbum)
{
    TrackSubscriberMock trackSubscriber;

    pDb->addTrack(track);

    Track otherTrack = track;
    otherTrack.filepath = "anotherPath";
    otherTrack.title = "anotherTitle";
    
    uint32_t id;
    StringOperations::toNumeric(track.id, id);
    NumericOperations::toString(++id, otherTrack.id);
    pDb->addTrack(otherTrack);    

    pDb->getTracksFromAlbum(track.albumId, trackSubscriber);
    ASSERT_EQ(2, trackSubscriber.tracks.size());
    EXPECT_EQ(track, trackSubscriber.tracks[0]);
    EXPECT_EQ(otherTrack, trackSubscriber.tracks[1]);
}

TEST_F(MusicDbTest, setAlbumArt)
{
    std::vector<uint8_t> data(8, 6);
    
    pDb->setAlbumArt(album.id, data);

    pDb->getAlbumArt(album);

    ASSERT_EQ(data.size(), album.coverData.size());
    EXPECT_EQ(0, memcmp(&data.front(), &album.coverData.front(), 8));
}
