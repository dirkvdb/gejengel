#ifndef TEST_FUNCTIONS_H
#define TEST_FUNCTIONS_H

#include "Utils/fileoperations.h"
#include "MusicLibrary/track.h"

inline Gejengel::Track getTrackForSong1()
{
    Gejengel::Track item;
    item.id             = "2";
    item.albumId        = "1";
    item.filepath       = FileOperations::combinePath(getenv("srcdir"), "testdata/audio/song1.mp3");
    item.artist         = "anArtist";
    item.title          = "aTitle";
    item.album          = "anAlbum";
    item.albumArtist    = "anAlbumArtist";
    item.genre          = "aGenre";
    item.composer       = "aComposer";
    item.year           = 2000;
    item.trackNr        = 1;
    item.discNr         = 1;
    item.durationInSec  = 0;
    item.bitrate        = 128;
    item.sampleRate     = 44100;
    item.channels       = 2;
    item.fileSize       = 1987;
    item.modifiedTime   = 10000;

    return item;
}

#endif
