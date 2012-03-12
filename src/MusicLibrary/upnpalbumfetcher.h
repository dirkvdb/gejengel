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

#ifndef UPNP_ALBUM_FETCHER_H
#define UPNP_ALBUM_FETCHER_H

#include <string>
#include <vector>
#include <memory>

#include "Core/gejengeltypes.h"
#include "utils/subscriber.h"
#include "upnp/upnpitem.h"
#include "MusicLibrary/subscribers.h"

namespace upnp
{
    class Browser;
}

namespace Gejengel
{

class Track;
class Album;

class UPnPTrackFetcher	: public utils::ISubscriber<upnp::Item>
                        , public utils::ISubscriber<std::shared_ptr<upnp::Item>>
{
public:
    UPnPTrackFetcher(upnp::Browser& browser);

    void onItem(const upnp::Item& item, void* pData = nullptr);
    void onItem(const std::shared_ptr<upnp::Item>& item, void* pData = nullptr);

    void fetchTrack(const std::string& TrackId, Track& track);
    void fetchTrackAsync(const std::string& trackId, utils::ISubscriber<Track>& subscriber);

    void fetchTracks(const std::string& containerId, utils::ISubscriber<Track>& subscriber);
    void fetchTracksAsync(const std::string& containerId, utils::ISubscriber<Track>& subscriber);

    void fetchFirstTrack(const std::string& containerId, utils::ISubscriber<Track>& subscriber);
    void fetchFirstTrackAsync(const std::string& containerId, utils::ISubscriber<Track>& subscriber);

private:
    void resetTrackCount();
    void itemToTrack(upnp::Item& item, Track& track);
    
    upnp::Browser&        		m_Browser;
    uint32_t					m_CurrentTrackNr;
    uint32_t					m_CurrentDiscNr;
};

class UPnPAlbumFetcher 	: public utils::ISubscriber<upnp::Item>
                        , public utils::ISubscriber<std::shared_ptr<upnp::Item>>
{
public:
    UPnPAlbumFetcher(upnp::Browser& browser);

    void onItem(const upnp::Item& container, void* pData = nullptr);
    void onItem(const std::shared_ptr<upnp::Item>& item, void* pData = nullptr);

    void fetchAlbum(const std::string& albumId, Album& album);
    void fetchAlbumAsync(const std::string& albumId, utils::ISubscriber<Album>& subscriber);

    void fetchAlbums(const std::string& containerId, utils::ISubscriber<Album>& subscriber);
    void fetchAlbumsAsync(const std::string& containerId, utils::ISubscriber<Album>& subscriber);

    void finalItemReceived(void* pData = nullptr);

private:
    void containerToAlbum(const upnp::Item& container, Album& album);

    upnp::Browser&        		m_Browser;
};

}

#endif

