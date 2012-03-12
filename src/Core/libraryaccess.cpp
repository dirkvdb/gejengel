//    Copyright (C) 2011 Dirk Vanden Boer <dirk.vdb@gmail.com>
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

#include "libraryaccess.h"

#include "utils/log.h"
#include "MusicLibrary/musiclibrary.h"

#include <vector>

using namespace utils;

namespace Gejengel
{

class ILibrarySubscriber;

LibraryAccess::LibraryAccess(Settings& settings)
: m_Settings(settings)
, m_LibraryType(None)
{
}

LibraryAccess::~LibraryAccess()
{
	cleanup();
}

LibraryType LibraryAccess::getLibraryType()
{
	return m_LibraryType;
}

void LibraryAccess::setLibraryType(LibraryType type)
{
	if (type == m_LibraryType)
    {
        return;
    }

	std::lock_guard<std::mutex> lock(m_Mutex);
    m_LibraryType = type;
    std::vector<ILibrarySubscriber*> subscribers;

    if (m_Library.get())
    {
    	subscribers = m_Library->getSubscribers();
    }

    try
    {
    	m_Library.reset(MusicLibraryFactory::create(m_LibraryType, m_Settings));
    }
    catch (std::exception& e)
	{
		log::error("LibraryAccess::setLibraryType Failed to load library, falling back to local library:", e.what());
		m_LibraryType = Local;
		m_Library.reset(MusicLibraryFactory::create(m_LibraryType, m_Settings));
	}

    for (size_t i = 0; i < subscribers.size(); ++i)
    {
        m_Library->addLibrarySubscriber(*(subscribers[i]));
    }
}

void LibraryAccess::cleanup()
{
    std::lock_guard<std::mutex> lock(m_Mutex);
	if (m_Library.get())
	{
		m_Library->clearSubscribers();
	}

	m_Library.reset();
}

uint32_t LibraryAccess::getTrackCount()
{
    std::lock_guard<std::mutex> lock(m_Mutex);
    if (m_Library.get())
    {
        m_Library->getTrackCount();
	}

	return 0;
}

uint32_t LibraryAccess::getAlbumCount()
{
    std::lock_guard<std::mutex> lock(m_Mutex);
	if (m_Library.get())
	{
		return m_Library->getAlbumCount();
	}

	return 0;
}

void LibraryAccess::getTrackAsync(const std::string& id, utils::ISubscriber<Track>& subscriber)
{
    std::lock_guard<std::mutex> lock(m_Mutex);
	if (m_Library.get())
	{
		m_Library->getTrackAsync(id, subscriber);
	}
}

void LibraryAccess::getTracksFromAlbumAsync(const std::string& albumId, utils::ISubscriber<Track>& subscriber)
{
    std::lock_guard<std::mutex> lock(m_Mutex);
	if (m_Library.get())
	{
		m_Library->getTracksFromAlbumAsync(albumId, subscriber);
	}
}

void LibraryAccess::getFirstTrackFromAlbum(const std::string& albumId, utils::ISubscriber<Track>& subscriber)
{
    std::lock_guard<std::mutex> lock(m_Mutex);
	if (m_Library.get())
	{
		m_Library->getFirstTrackFromAlbum(albumId, subscriber);
	}
}

void LibraryAccess::getFirstTrackFromAlbumAsync(const std::string& albumId, utils::ISubscriber<Track>& subscriber)
{
    std::lock_guard<std::mutex> lock(m_Mutex);
	if (m_Library.get())
	{
		m_Library->getFirstTrackFromAlbumAsync(albumId, subscriber);
	}
}

void LibraryAccess::getAlbumAsync(const std::string& albumId, utils::ISubscriber<Album>& subscriber)
{
    std::lock_guard<std::mutex> lock(m_Mutex);
	if (m_Library.get())
	{
		m_Library->getAlbumAsync(albumId, subscriber);
	}
}

void LibraryAccess::getAlbumsAsync(utils::ISubscriber<Album>& subscriber)
{
    std::lock_guard<std::mutex> lock(m_Mutex);
	if (m_Library.get())
	{
		m_Library->getAlbumsAsync(subscriber);
	}
}

void LibraryAccess::getRandomTracksAsync(uint32_t trackCount, utils::ISubscriber<Track>& subscriber)
{
    std::lock_guard<std::mutex> lock(m_Mutex);
	if (m_Library.get())
	{
		m_Library->getRandomTracksAsync(trackCount, subscriber);
	}
}

void LibraryAccess::getRandomAlbumAsync(utils::ISubscriber<Track>& subscriber)
{
    std::lock_guard<std::mutex> lock(m_Mutex);
	if (m_Library.get())
	{
		m_Library->getRandomAlbumAsync(subscriber);
	}
}

bool LibraryAccess::getAlbumArt(const Album& album, AlbumArt& art)
{
    std::lock_guard<std::mutex> lock(m_Mutex);
	if (m_Library.get())
	{
		return m_Library->getAlbumArt(album, art);
	}

	return false;
}

void LibraryAccess::scan(bool startFresh, IScanSubscriber& subscriber)
{
    std::lock_guard<std::mutex> lock(m_Mutex);
	if (m_Library.get())
	{
		m_Library->scan(startFresh, subscriber);
	}
}

void LibraryAccess::search(const std::string& search, utils::ISubscriber<Track>& trackSubscriber, utils::ISubscriber<Album>& albumSubscriber)
{
    std::lock_guard<std::mutex> lock(m_Mutex);
	if (m_Library.get())
	{
		m_Library->search(search, trackSubscriber, albumSubscriber);
	}
}

void LibraryAccess::setSource(const LibrarySource& source)
{
    std::lock_guard<std::mutex> lock(m_Mutex);
	if (m_Library.get())
	{
		m_Library->setSource(source);
	}
}

MusicLibrary& LibraryAccess::getLibrary()
{
    std::lock_guard<std::mutex> lock(m_Mutex);
	return *m_Library;
}

void LibraryAccess::addLibrarySubscriber(ILibrarySubscriber& subscriber)
{
    std::lock_guard<std::mutex> lock(m_Mutex);
	if (m_Library.get())
	{
		m_Library->addLibrarySubscriber(subscriber);
	}
}

}
