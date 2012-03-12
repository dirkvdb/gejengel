#ifndef TEST_CLASSES_H
#define TEST_CLASSES_H

#include "MusicLibrary/subscribers.h"
#include "MusicLibrary/track.h"
#include "MusicLibrary/album.h"

class LibrarySubscriberMock : public Gejengel::ILibrarySubscriber
{
public:
    void newTrack(const Gejengel::Track& track) { newTracks.push_back(track); }
    void deletedTrack(const std::string& id) { deletedTracks.push_back(id); }
    void updatedTrack(const Gejengel::Track& track) { updatedTracks.push_back(track); }
    void newAlbum(const Gejengel::Album& album) { newAlbums.push_back(album); }
    void deletedAlbum(const std::string& id) { deletedAlbums.push_back(id); }
    void updatedAlbum(const Gejengel::Album& album) { updatedAlbums.push_back(album); }

    void clearLists()
    {
        newTracks.clear();
        deletedTracks.clear();
        updatedTracks.clear();
        newAlbums.clear();
        deletedAlbums.clear();
        updatedAlbums.clear();
    }

    std::vector<Gejengel::Track> newTracks;
    std::vector<std::string> deletedTracks;
    std::vector<Gejengel::Track> updatedTracks;
    std::vector<Gejengel::Album> newAlbums;
    std::vector<std::string> deletedAlbums;
    std::vector<Gejengel::Album> updatedAlbums;
};

class ScanSubscriberMock : public Gejengel::IScanSubscriber
{
    void scanStart(uint32_t numTracks) {}
    void scanUpdate(uint32_t scannedTracks) {}
    void scanFinish() {}
    void scanFailed() {}
};

class TrackSubscriberMock : public Gejengel::ITrackSubscriber
{
public:
    void onTrack(const Gejengel::Track& track)
    {
        tracks.push_back(track);
    }

    std::vector<Gejengel::Track> tracks;
};

class AlbumSubscriberMock : public Gejengel::IAlbumSubscriber
{
public:
    void onAlbum(const Gejengel::Album& album)
    {
        albums.push_back(album);
    }

    std::vector<Gejengel::Album> albums;
};

#endif
