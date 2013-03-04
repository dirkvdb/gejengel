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

#include "filesystemmusiclibrary.h"

#include <stdexcept>
#include <cassert>

#include "Core/settings.h"
#include "track.h"
#include "album.h"
#include "scanner.h"
#include "utils/log.h"
#include "utils/trace.h"

using namespace std;

namespace Gejengel
{

FilesystemMusicLibrary::FilesystemMusicLibrary(const Settings& settings)
: MusicLibrary(settings)
, m_Db(settings.get("DBFile"))
, m_Destroy(false)
{
    utils::trace("Create FilesystemMusicLibrary");
    m_Db.setSubscriber(*this);
}

FilesystemMusicLibrary::~FilesystemMusicLibrary()
{
	m_Destroy = true;
	cancelScanThread();
}

void FilesystemMusicLibrary::cancelScanThread()
{
    if (m_ScannerThread.joinable())
	{
		{
			std::lock_guard<std::mutex> lock(m_ScanMutex);
			if (m_Scanner.get())
			{
				m_Scanner->cancel();
			}
		}
		
		m_ScannerThread.join();
	}
}

uint32_t FilesystemMusicLibrary::getTrackCount()
{
    return m_Db.getTrackCount();
}

uint32_t FilesystemMusicLibrary::getAlbumCount()
{
    return m_Db.getAlbumCount();
}

void FilesystemMusicLibrary::getTrack(const std::string& id, Track& track)
{
    if (!m_Db.getTrack(id, track))
    {
        throw std::logic_error("Failed to get track from db (id:" + id + ")");
    }
}

void FilesystemMusicLibrary::getTrackAsync(const std::string& id, utils::ISubscriber<const Track&>& subscriber)
{
    Track track;
    getTrack(id, track);
    subscriber.onItem(track);
}

void FilesystemMusicLibrary::getTracksFromAlbum(const std::string& albumId, utils::ISubscriber<const Track&>& subscriber)
{
    m_Db.getTracksFromAlbum(albumId, subscriber);
}

void FilesystemMusicLibrary::getTracksFromAlbumAsync(const std::string& albumId, utils::ISubscriber<const Track&>& subscriber)
{
    getTracksFromAlbum(albumId, subscriber);
}

void FilesystemMusicLibrary::getFirstTrackFromAlbum(const std::string& albumId, utils::ISubscriber<const Track&>& subscriber)
{
    m_Db.getFirstTrackFromAlbum(albumId, subscriber);
}

void FilesystemMusicLibrary::getFirstTrackFromAlbumAsync(const std::string& albumId, utils::ISubscriber<const Track&>& subscriber)
{
	getFirstTrackFromAlbum(albumId, subscriber);
}

void FilesystemMusicLibrary::getAlbum(const std::string& albumId, Album& album)
{
    m_Db.getAlbum(albumId, album);
}

void FilesystemMusicLibrary::getAlbumAsync(const std::string& albumId, utils::ISubscriber<const Album&>& subscriber)
{
	Album album;
    m_Db.getAlbum(albumId, album);
    subscriber.onItem(album);
}

void FilesystemMusicLibrary::getRandomTracks(uint32_t trackCount, utils::ISubscriber<const Track&>& subscriber)
{
    m_Db.getRandomTracks(trackCount, subscriber);
}

void FilesystemMusicLibrary::getRandomTracksAsync(uint32_t trackCount, utils::ISubscriber<const Track&>& subscriber)
{
	getRandomTracks(trackCount, subscriber);
}

void FilesystemMusicLibrary::getRandomAlbum(utils::ISubscriber<const Track&>& subscriber)
{
    m_Db.getRandomAlbum(subscriber);
}

void FilesystemMusicLibrary::getRandomAlbumAsync(utils::ISubscriber<const Track&>& subscriber)
{
	getRandomAlbum(subscriber);
}
void FilesystemMusicLibrary::getAlbums(utils::ISubscriber<const Album&>& subscriber)
{
    m_Db.getAlbums(subscriber);
}

void FilesystemMusicLibrary::getAlbumsAsync(utils::ISubscriber<const Album&>& subscriber)
{
    getAlbums(subscriber);
}

bool FilesystemMusicLibrary::getAlbumArt(const Album& album, AlbumArt& art)
{
    return m_Db.getAlbumArt(album, art);
}

void FilesystemMusicLibrary::scan(bool startFresh, IScanSubscriber& subscriber)
{
    string libraryPath = m_Settings.get("MusicLibrary");

    if (startFresh || (m_LibraryPath != libraryPath && !m_LibraryPath.empty()))
    {
        m_Db.clearDatabase();
    }

    m_LibraryPath = libraryPath;
    
    cancelScanThread();
    m_ScannerThread = std::thread(&FilesystemMusicLibrary::scannerThread, this, std::ref(subscriber));
}

void FilesystemMusicLibrary::search(const std::string& searchString, utils::ISubscriber<const Track&>& trackSubscriber, utils::ISubscriber<const Album&>& albumSubscriber)
{
    m_Db.searchLibrary(searchString, trackSubscriber, albumSubscriber);
}

void FilesystemMusicLibrary::setSource(const LibrarySource& source)
{
	assert(!"FilesystemMusicLibrary::setSource SHOULD NOT HAPPEN");
}

void FilesystemMusicLibrary::scannerThread(IScanSubscriber& subscriber)
{
    try
    {
		std::vector<std::string> filenames;
        m_Settings.getAsVector("AlbumArtFilenames", filenames);
        
        {
			std::lock_guard<std::mutex> lock(m_ScanMutex);
			m_Scanner.reset(new Scanner(m_Db, subscriber, filenames));
		}
        m_Scanner->performScan(m_LibraryPath);
        
        if (!m_Destroy)
        {
			m_Db.removeNonExistingFiles();
		}
		
		if (!m_Destroy)
        {
			m_Db.removeNonExistingAlbums();
		}
    }
    catch (std::exception& e)
    {
        log::error("Failed to scan library: %s", e.what());
        subscriber.scanFailed();
    }
}

}
