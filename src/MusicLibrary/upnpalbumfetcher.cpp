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

#include "upnpalbumfetcher.h"

#include <cassert>

#include "utils/log.h"
#include "utils/stringoperations.h"
#include "track.h"
#include "album.h"
#include "subscribers.h"
#include "upnp/upnpbrowser.h"

using namespace utils;

namespace Gejengel
{

uint32_t durationFromString(const std::string& durationString);

UPnPTrackFetcher::UPnPTrackFetcher(upnp::Browser& browser)
: m_Browser(browser)
, m_CurrentTrackNr(0)
, m_CurrentDiscNr(1)
{
}

void UPnPTrackFetcher::resetTrackCount()
{
    m_CurrentTrackNr = 0;
    m_CurrentDiscNr = 1;
}

void UPnPTrackFetcher::onItem(const upnp::Item& item, void* pData)
{
    utils::ISubscriber<Track>* pSubscriber = reinterpret_cast<utils::ISubscriber<Track>*> (pData);
    assert(pSubscriber);

    Track track;
    fetchTrack(item.getObjectId(), track);

    pSubscriber->onItem(track);
}

void UPnPTrackFetcher::onItem(const std::shared_ptr<upnp::Item>& item, void* pData)
{
    onItem(*item, pData);
}

void UPnPTrackFetcher::fetchTrack(const std::string& trackId, Track& track)
{
    upnp::Item item(trackId);
    m_Browser.getMetaData(item, "*");
    itemToTrack(item, track);
}

void UPnPTrackFetcher::fetchFirstTrack(const std::string& containerId, utils::ISubscriber<Track>& subscriber)
{
    upnp::Item container(containerId);
    m_Browser.getItems(*this, container, 1, 0, "upnp:originalTrackNumber", &subscriber);
}

void UPnPTrackFetcher::fetchFirstTrackAsync(const std::string& containerId, utils::ISubscriber<Track>& subscriber)
{
    m_Browser.getItemsAsync(*this, upnp::Item(containerId), 1, 0, "upnp:originalTrackNumber", &subscriber);
}

void UPnPTrackFetcher::fetchTrackAsync(const std::string& trackId, utils::ISubscriber<Track>& subscriber)
{
    std::shared_ptr<upnp::Item> item = std::make_shared<upnp::Item>(trackId);
    m_Browser.getMetaDataAsync(*this, item, "*", &subscriber);
}

void UPnPTrackFetcher::fetchTracks(const std::string& containerId, utils::ISubscriber<Track>& subscriber)
{
    resetTrackCount();
    upnp::Item container(containerId);
    m_Browser.getItems(*this, container, 0, 0, "upnp:originalTrackNumber", &subscriber);
}

void UPnPTrackFetcher::fetchTracksAsync(const std::string& containerId, utils::ISubscriber<Track>& subscriber)
{
    resetTrackCount();
    m_Browser.getItemsAsync(*this, upnp::Item(containerId), 0, 0, "upnp:originalTrackNumber", &subscriber);
}

void UPnPTrackFetcher::itemToTrack(upnp::Item& item, Track& track)
{
    if (track.id.empty())
    {
        track.id = item.getObjectId();
    }

    track.albumId = item.getMetaData("parentID");
    track.title = item.getMetaData("dc:title");
    track.artist = item.getMetaData("upnp:artist");
    track.album = item.getMetaData("upnp:album");
    track.genre = item.getMetaData("upnp:genre");

    if (!item.getResources().empty())
    {
        track.filepath = item.getResources().front().getUrl();
        track.fileSize = stringops::toNumeric<uint64_t>(item.getResources().front().getMetaData(upnp::Resource::Size));
    }

    std::string trackNrStr = item.getMetaData("upnp:originalTrackNumber");
    if (!trackNrStr.empty()) {
        track.trackNr = stringops::toNumeric<uint32_t>(trackNrStr);
    }

    if (track.trackNr < m_CurrentTrackNr) {
        ++m_CurrentDiscNr;
    }

    m_CurrentTrackNr = track.trackNr;
    track.discNr = m_CurrentDiscNr;

    std::string date = item.getMetaData("dc:date");
    if (!date.length() > 4)
    {
        track.year = stringops::toNumeric<uint32_t>(date.substr(0, 4));
    }

    std::string duration = item.getMetaData("duration");
    if (!duration.empty())
    {
        track.durationInSec = durationFromString(duration);
    }

    std::string bitrate = item.getMetaData("bitrate");
    if (!bitrate.empty())
    {
        track.bitrate = stringops::toNumeric<uint32_t>(bitrate);
        track.bitrate /= 1000;
    }

    std::string sampleFrequency = item.getMetaData("sampleFrequency");
    if (!sampleFrequency.empty())
    {
        track.sampleRate = stringops::toNumeric<uint32_t>(sampleFrequency);
    }

    std::string channels = item.getMetaData("nrAudioChannels");
    if (!channels.empty())
    {
        track.channels = stringops::toNumeric<uint32_t>(channels);
    }

    track.artUrl = item.getMetaData("upnp:albumArtURI");
}

UPnPAlbumFetcher::UPnPAlbumFetcher(upnp::Browser& browser)
: m_Browser(browser)
{
}

void UPnPAlbumFetcher::onItem(const upnp::Item& container, void* pData)
{
    utils::ISubscriber<Album>* pSubscriber = reinterpret_cast<utils::ISubscriber<Album>*> (pData);
    assert(pSubscriber);

    upnp::Item containerWithMetadata = container;
    m_Browser.getMetaData(containerWithMetadata, "@childCount,dc:title,upnp:artist,upnp:genre,upnp:albumArtURI");

    Album album;
    containerToAlbum(containerWithMetadata, album);

    pSubscriber->onItem(album);
}

void UPnPAlbumFetcher::onItem(const std::shared_ptr<upnp::Item>& item, void* pData)
{
    onItem(*item, pData);
}

void UPnPAlbumFetcher::finalItemReceived(void* pData)
{
    utils::ISubscriber<Album>* pSubscriber = reinterpret_cast<utils::ISubscriber<Album>*> (pData);
    assert(pSubscriber);
    pSubscriber->finalItemReceived();
}

void UPnPAlbumFetcher::fetchAlbum(const std::string& albumId, Album& album)
{
    upnp::Item container(albumId);
    m_Browser.getMetaData(container, "@childCount,dc:title,upnp:artist,upnp:genre,upnp:albumArtURI");

    containerToAlbum(container, album);
}

void UPnPAlbumFetcher::fetchAlbumAsync(const std::string& albumId, utils::ISubscriber<Album>& subscriber)
{
    std::shared_ptr<upnp::Item> container = std::make_shared<upnp::Item>(albumId);
    m_Browser.getMetaDataAsync(*this, container, "@childCount,dc:title,upnp:artist,upnp:genre,upnp:albumArtURI", &subscriber);
}

void UPnPAlbumFetcher::fetchAlbums(const std::string& containerId, utils::ISubscriber<Album>& subscriber)
{
    upnp::Item container(containerId);
    m_Browser.getContainers(*this, container, 0, 0, &subscriber);
}

void UPnPAlbumFetcher::fetchAlbumsAsync(const std::string& containerId, utils::ISubscriber<Album>& subscriber)
{
    m_Browser.getContainersAsync(*this, upnp::Item(containerId), 0, 0, &subscriber);
}

void UPnPAlbumFetcher::containerToAlbum(const upnp::Item& container, Album& album)
{
    if (album.id.empty())
    {
        album.id = container.getObjectId();
    }

    album.title = container.getTitle();
    album.artist = container.getMetaData("upnp:artist");
    album.artUrl = container.getMetaData("upnp:albumArtURI");
    album.genre = container.getMetaData("upnp:genre");

    album.trackCount = stringops::toNumeric<uint32_t>(container.getMetaData("childCount"));
}

uint32_t durationFromString(const std::string& durationString)
{
    uint32_t duration = 0;
    std::vector<std::string> times = stringops::tokenize(durationString, ":");

    if (times.size() == 3)
    {
        uint32_t hours = stringops::toNumeric<uint32_t>(times[0]);
        uint32_t minutes = stringops::toNumeric<uint32_t>(times[1]);
        uint32_t seconds = stringops::toNumeric<uint32_t>(times[2]);

        duration += hours * 3600;
        duration += minutes * 60;
        duration += seconds;
    }

    return duration;
}

}
