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

#ifndef ALBUM_ART_GRABBER_H
#define ALBUM_ART_GRABBER_H

#include "albumartprovider.h"
#include "utils/subscriber.h"
#include "MusicLibrary/libraryitem.h"
#include "MusicLibrary/album.h"
#include "MusicLibrary/albumart.h"
#include "MusicLibrary/track.h"

#include <deque>
#include <thread>
#include <mutex>
#include <condition_variable>

namespace Gejengel
{

class IGejengelCore;

class AlbumArtGrabber: public Gejengel::IAlbumArtProvider
{
public:
	AlbumArtGrabber(IGejengelCore& core);
	virtual ~AlbumArtGrabber();

	void getAlbumArt(const Album& album, utils::ISubscriber<AlbumArt>& subscriber);
	void getAlbumArt(const Track& track, utils::ISubscriber<AlbumArt>& subscriber);
	void getAlbumArtFromSource(const Album& album, uint32_t size, utils::ISubscriber<AlbumArt>& subscriber);
	void getAlbumArtFromSource(const Track& track, uint32_t size, utils::ISubscriber<AlbumArt>& subscriber);

private:
	template <typename T>
	class WorkDescription
	{
	public:
		WorkDescription() {}
		WorkDescription(const T& item, utils::ISubscriber<AlbumArt>& subscriber, uint32_t size = -1)
		: item(item), pSubscriber(&subscriber), size(size) {}

		T 								item;
		utils::ISubscriber<AlbumArt>* 	pSubscriber;
		uint32_t 						size;
	};

	template <typename T>
	bool getQueuedItem(std::deque<WorkDescription<T> >& queue, WorkDescription<T>& description);

	bool fetchAlbumFromSource(const Album& track, AlbumArt& art, uint32_t size);
	bool fetchTrackFromSource(const Track& track, AlbumArt& art, uint32_t size);
	void fetchLoop();
	static void* fetchThread(void* pInstance, void* pExtraData);

	IGejengelCore&						m_Core;
	std::thread                   	    m_FetchAlbumArtThread;
	std::mutex                    	    m_Mutex;
	std::condition_variable    			m_Condition;

	std::deque<WorkDescription<Album> >	m_AlbumQueue;
	std::deque<WorkDescription<Album> >	m_AlbumFromSourceQueue;
	std::deque<WorkDescription<Track> >	m_TrackFromSourceQueue;
	std::vector<std::string> 			m_AlbumArtFilenames;

	bool								m_Destroy;
};

}

#endif
