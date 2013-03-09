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
    class MediaServer;
}

namespace Gejengel
{

class Track;
class Album;

class UPnPTrackFetcher
{
public:
    UPnPTrackFetcher(upnp::MediaServer& server);

    Track fetchTrack(const std::string& TrackId);
    void fetchTrackAsync(const std::string& trackId, utils::ISubscriber<const Track&>& subscriber);

    void fetchTracks(const std::string& containerId, utils::ISubscriber<const Track&>& subscriber);
    void fetchTracksAsync(const std::string& containerId, utils::ISubscriber<const Track&>& subscriber);

    void fetchFirstTrack(const std::string& containerId, utils::ISubscriber<const Track&>& subscriber);
    void fetchFirstTrackAsync(const std::string& containerId, utils::ISubscriber<const Track&>& subscriber);

private:
    void resetTrackCount();
    Track itemToTrack(upnp::Item& item);
    
    upnp::MediaServer&          m_Server;
    uint32_t                    m_CurrentTrackNr;
    uint32_t                    m_CurrentDiscNr;
};

class UPnPAlbumFetcher
{
public:
    UPnPAlbumFetcher(upnp::MediaServer& browser);

    Album fetchAlbum(const std::string& albumId);
    void fetchAlbumAsync(const std::string& albumId, utils::ISubscriber<const Album&>& subscriber);

    void fetchAlbums(const std::string& containerId, utils::ISubscriber<const Album&>& subscriber);
    void fetchAlbumsAsync(const std::string& containerId, utils::ISubscriber<const Album&>& subscriber);

    void finalItemReceived(void* pData = nullptr);

private:
    void processItem(const upnp::ItemPtr& item, utils::ISubscriber<const Album&>& subscriber);

    Album containerToAlbum(const upnp::Item& container);

    upnp::MediaServer&              m_Server;
};

}

#endif

