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

#include "albumartgrabber.h"

#include "Core/gejengelcore.h"
#include "Core/settings.h"
#include "Core/libraryaccess.h"
#include "MusicLibrary/album.h"
#include "MusicLibrary/track.h"
#include "MusicLibrary/metadata.h"
#include "utils/log.h"
#include "utils/simplesubscriber.h"

using namespace utils;

namespace Gejengel
{

AlbumArtGrabber::AlbumArtGrabber(IGejengelCore& core)
: m_Core(core)
, m_Destroy(false)
{
    core.getSettings().getAsVector("AlbumArtFilenames", m_AlbumArtFilenames);

	m_FetchAlbumArtThread = std::thread(&AlbumArtGrabber::fetchLoop, this);
}

AlbumArtGrabber::~AlbumArtGrabber()
{
    {
        std::lock_guard<std::mutex> lock(m_Mutex);
        m_Destroy = true;
    }

    m_Condition.notify_all();

	if (m_FetchAlbumArtThread.joinable())
	{
	    log::debug("Waiting for album art thread");
	    m_FetchAlbumArtThread.join();
	    log::debug("Album art thread finished");
	}
}

void AlbumArtGrabber::getAlbumArt(const Album& album, utils::ISubscriber<AlbumArt>& subscriber)
{
    {
        std::lock_guard<std::mutex> lock(m_Mutex);
        m_AlbumQueue.push_back(WorkDescription<Album>(album, subscriber));
    }
	m_Condition.notify_all();
}

void AlbumArtGrabber::getAlbumArt(const Track& track, utils::ISubscriber<AlbumArt>& subscriber)
{
    {
        std::lock_guard<std::mutex> lock(m_Mutex);
        Album album(track.album);
        m_AlbumQueue.push_back(WorkDescription<Album>(Album(track.albumId), subscriber));
    }

    m_Condition.notify_all();
}

void AlbumArtGrabber::getAlbumArtFromSource(const Album& album, uint32_t size, utils::ISubscriber<AlbumArt>& subscriber)
{
    {
        std::lock_guard<std::mutex> lock(m_Mutex);
        m_AlbumFromSourceQueue.push_back(WorkDescription<Album>(album, subscriber, size));
    }

    m_Condition.notify_all();
}

void AlbumArtGrabber::getAlbumArtFromSource(const Track& track, uint32_t size, utils::ISubscriber<AlbumArt>& subscriber)
{
    {
        std::lock_guard<std::mutex> lock(m_Mutex);
        m_TrackFromSourceQueue.push_back(WorkDescription<Track>(track, subscriber, size));
    }

	m_Condition.notify_all();
}

template <typename T>
bool AlbumArtGrabber::getQueuedItem(std::deque<WorkDescription<T> >& queue, WorkDescription<T>& description)
{
    std::lock_guard<std::mutex> lock(m_Mutex);
	if (!queue.empty())
	{
		description = queue.front();
		queue.pop_front();
		return true;
	}

	return false;
}

bool AlbumArtGrabber::fetchAlbumFromSource(const Album& album, AlbumArt& art, uint32_t size)
{
	if (!album.artUrl.empty())
	{
		return m_Core.getLibraryAccess().getAlbumArt(album, art);
	}

	utils::SimpleSubscriber<Track> subscriber;
	m_Core.getLibraryAccess().getFirstTrackFromAlbum(album.id, subscriber);

	Track& track = subscriber.getItem();
	if (!track.id.empty())
	{
		return fetchTrackFromSource(track, art, size);
	}

	return false;
}

bool AlbumArtGrabber::fetchTrackFromSource(const Track& track, AlbumArt& art, uint32_t size)
{
	if (!track.filepath.empty())
	{
		Metadata md(track.filepath);
		md.getAlbumArt(art.getData(), m_AlbumArtFilenames, size);
	}

	/* Failed to get art from file, try to get it from the library */
	if (art.getData().empty())
	{
		m_Core.getLibraryAccess().getAlbumArt(Album(track.albumId), art);
	}

	return !art.getData().empty();
}

void AlbumArtGrabber::fetchLoop()
{
	while (!m_Destroy)
	{
		{
			std::unique_lock<std::mutex> lock(m_Mutex);
			m_Condition.wait(lock);
		}

		WorkDescription<Album> albumDescription;
		while (!m_Destroy && getQueuedItem<Album>(m_AlbumQueue, albumDescription))
		{
			Album& album = albumDescription.item;

			try
			{
				AlbumArt art(album.id);
				if (m_Core.getLibraryAccess().getAlbumArt(album, art))
				{
					//log::debug("Album art fetched for album:", album.id, art.getDataSize());
					albumDescription.pSubscriber->onItem(art);
				}
			}
			catch (std::exception& e)
			{
				log::warn("Failed to fetch album art for album", album.id, ":", e.what());
			}
		}

		while (!m_Destroy && getQueuedItem<Album>(m_AlbumFromSourceQueue, albumDescription))
		{
			Album& album = albumDescription.item;

			try
			{
				AlbumArt art(album.id);
				if (fetchAlbumFromSource(album, art, albumDescription.size))
				{
					albumDescription.pSubscriber->onItem(art);
				}
			}
			catch (std::exception& e)
			{
				log::warn("Failed to fetch album art from source for album", album.id, ":", e.what());
			}
		}

		WorkDescription<Track> trackDescription;
		while (!m_Destroy && getQueuedItem<Track>(m_TrackFromSourceQueue, trackDescription))
		{
			Track& track = trackDescription.item;

			try
			{
				AlbumArt art(track.album);
				if (fetchTrackFromSource(track, art, trackDescription.size))
				{
					trackDescription.pSubscriber->onItem(art);
				}
			}
			catch (std::exception& e)
			{
				log::warn("Failed to fetch album art for track", track.id, ":", e.what());
			}
		}
	}
}

}
