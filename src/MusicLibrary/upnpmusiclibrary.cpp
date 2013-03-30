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

#include "upnpmusiclibrary.h"

#include <stdexcept>
#include <cassert>
#include <cstdlib>

#include "album.h"
#include "albumart.h"
#include "track.h"
#include "upnpalbumfetcher.h"
#include "upnplibrarysource.h"
#include "Core/settings.h"
#include "utils/log.h"
#include "utils/trace.h"
#include "utils/simplesubscriber.h"
#include "utils/stringoperations.h"
#include "upnp/upnphttpclient.h"

using namespace std;
using namespace utils;

namespace Gejengel
{

UPnPMusicLibrary::UPnPMusicLibrary(Settings& settings)
: MusicLibrary(settings)
, m_Server(m_Client)
, m_TrackFetcher(m_Server)
, m_AlbumFetcher(m_Server)
{
    utils::trace("Create UPnPMusicLibrary");
    m_Client.initialize();
}

UPnPMusicLibrary::~UPnPMusicLibrary()
{
}

uint32_t UPnPMusicLibrary::getTrackCount()
{
    return 0;
}

uint32_t UPnPMusicLibrary::getAlbumCount()
{
    auto container = std::make_shared<upnp::Item>(m_CurrentServer->m_ContainerId);
    m_Server.getMetaData(container);

    return container->getChildCount();
}

void UPnPMusicLibrary::getTrack(const std::string& id, Track& track)
{
    track = m_TrackFetcher.fetchTrack(id);
}

void UPnPMusicLibrary::getTrackAsync(const std::string& id, utils::ISubscriber<const Track&>& subscriber)
{
    m_TrackFetcher.fetchTrackAsync(id, subscriber);
}

void UPnPMusicLibrary::getTracksFromAlbum(const std::string& albumId, utils::ISubscriber<const Track&>& subscriber)
{
    m_TrackFetcher.fetchTracks(albumId, subscriber);
}

void UPnPMusicLibrary::getTracksFromAlbumAsync(const std::string& albumId, utils::ISubscriber<const Track&>& subscriber)
{
    m_TrackFetcher.fetchTracksAsync(albumId, subscriber);
}

void UPnPMusicLibrary::getFirstTrackFromAlbum(const std::string& albumId, utils::ISubscriber<const Track&>& subscriber)
{
	m_TrackFetcher.fetchFirstTrack(albumId, subscriber);
}

void UPnPMusicLibrary::getFirstTrackFromAlbumAsync(const std::string& albumId, utils::ISubscriber<const Track&>& subscriber)
{
	m_TrackFetcher.fetchFirstTrackAsync(albumId, subscriber);
}

void UPnPMusicLibrary::getAlbum(const std::string& albumId, Album& album)
{
    album = m_AlbumFetcher.fetchAlbum(albumId);
}

void UPnPMusicLibrary::getAlbumAsync(const std::string& albumId, utils::ISubscriber<const Album&>& subscriber)
{
	m_AlbumFetcher.fetchAlbumAsync(albumId, subscriber);
}

void UPnPMusicLibrary::getRandomTracks(uint32_t trackCount, utils::ISubscriber<const Track&>& subscriber)
{
	log::warn("UPnPMusicLibrary::getRandomTracks TODO: implement me");
}

void UPnPMusicLibrary::getRandomTracksAsync(uint32_t trackCount, utils::ISubscriber<const Track&>& subscriber)
{
	log::warn("UPnPMusicLibrary::getRandomTracks async TODO: implement me");
}

void UPnPMusicLibrary::getRandomAlbum(utils::ISubscriber<const Track&>& subscriber)
{
    uint32_t albumCount = getAlbumCount();
    if (albumCount == 0)
    {
        log::error("Failed to get albumCount: not queueing anything");
        return;
    }

    uint32_t randomAlbum = rand() % albumCount;

    auto container = std::make_shared<upnp::Item>(m_CurrentServer->m_ContainerId);
    m_Server.getContainersInContainer(container, [&] (const upnp::ItemPtr& container) {
        getTracksFromAlbumAsync(container->getObjectId(), subscriber);
    }, randomAlbum, 1);
}

void UPnPMusicLibrary::getRandomAlbumAsync(utils::ISubscriber<const Track&>& subscriber)
{
	log::debug("TODO: UPnPMusicLibrary::getRandomAlbumAsync: implement me");
}

void UPnPMusicLibrary::getAlbums(utils::ISubscriber<const Album&>& subscriber)
{
    if (!m_CurrentServer->m_ContainerId.empty())
    {
        m_AlbumFetcher.fetchAlbums(m_CurrentServer->m_ContainerId, subscriber);
    }
}

void UPnPMusicLibrary::getAlbumsAsync(utils::ISubscriber<const Album&>& subscriber)
{
    if (!m_CurrentServer->m_ContainerId.empty())
    {
        m_AlbumFetcher.fetchAlbumsAsync(m_CurrentServer->m_ContainerId, subscriber);
    }
}

bool UPnPMusicLibrary::getAlbumArt(const Album& album, AlbumArt& art)
{
	Album localAlbum = album;
	string url = localAlbum.artUrl;

    if (url.empty())
    {
        getAlbum(localAlbum.id, localAlbum);
        url = localAlbum.artUrl;
        if (url.empty())
        {
        	utils::SimpleSubscriber<Track> subscriber;
        	getFirstTrackFromAlbum(album.id, subscriber);
        	url = subscriber.getItem().artUrl;
        	if (url.empty())
        	{
        		return false;
        	}
        }
    }
    
    try
    {
		int32_t timeout = 10;
		upnp::HttpClient client(timeout);
		art.getData() = client.getData(url);
    }
    catch (std::exception& e)
    {
    	log::error(e.what());
    	return false;
    }

    return true;
}

void UPnPMusicLibrary::scan(bool startFresh, IScanSubscriber& subscriber)
{
    subscriber.scanFinish();
}

void UPnPMusicLibrary::search(const std::string& searchString, utils::ISubscriber<const Track&>& trackSubscriber, utils::ISubscriber<const Album&>& albumSubscriber)
{
}

void UPnPMusicLibrary::setSource(const LibrarySource& source)
{
	const UPnPLibrarySource& upnpSource = dynamic_cast<const UPnPLibrarySource&>(source);

    m_CurrentServer = upnpSource.getDevice();
    
    try
    {
        m_Server.setDevice(m_CurrentServer);
    }
    catch (std::exception& e)
    {
        log::error("Failed to set UPnP device: %s", e.what());
    }
}

upnp::Client& UPnPMusicLibrary::getClient()
{
    return m_Client;
}

}
