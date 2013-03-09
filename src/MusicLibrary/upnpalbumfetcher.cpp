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
#include "upnp/upnpmediaserver.h"

using namespace utils;

namespace Gejengel
{

uint32_t durationFromString(const std::string& durationString);

UPnPTrackFetcher::UPnPTrackFetcher(upnp::MediaServer& server)
: m_Server(server)
, m_CurrentTrackNr(0)
, m_CurrentDiscNr(1)
{
}

void UPnPTrackFetcher::resetTrackCount()
{
    m_CurrentTrackNr = 0;
    m_CurrentDiscNr = 1;
}

Track UPnPTrackFetcher::fetchTrack(const std::string& trackId)
{
    auto item = std::make_shared<upnp::Item>(trackId);
    m_Server.getMetaData(item);
    return itemToTrack(*item);
}

void UPnPTrackFetcher::fetchFirstTrack(const std::string& containerId, utils::ISubscriber<const Track&>& subscriber)
{
    auto container = std::make_shared<upnp::Item>(containerId);
    m_Server.getItemsInContainer(container, [&] (const upnp::ItemPtr& item) {
        subscriber.onItem(itemToTrack(*item));
    }, 0, 1, upnp::Property::TrackNumber);
}

void UPnPTrackFetcher::fetchFirstTrackAsync(const std::string& containerId, utils::ISubscriber<const Track&>& subscriber)
{
    auto container = std::make_shared<upnp::Item>(containerId);
    m_Server.getItemsInContainerAsync(container, [&] (const upnp::ItemPtr& item) {
        subscriber.onItem(itemToTrack(*item));
    }, 0, 1, upnp::Property::TrackNumber);
}

void UPnPTrackFetcher::fetchTrackAsync(const std::string& trackId, utils::ISubscriber<const Track&>& subscriber)
{
    auto item = std::make_shared<upnp::Item>(trackId);
    m_Server.getMetaDataAsync(item, [&] (const upnp::ItemPtr& item) {
        subscriber.onItem(itemToTrack(*item));
    });
}

void UPnPTrackFetcher::fetchTracks(const std::string& containerId, utils::ISubscriber<const Track&>& subscriber)
{
    resetTrackCount();
    auto container = std::make_shared<upnp::Item>(containerId);
    m_Server.getItemsInContainer(container, [&] (const upnp::ItemPtr& item) {
        subscriber.onItem(itemToTrack(*item));
    }, 0, 0, upnp::Property::TrackNumber);
}

void UPnPTrackFetcher::fetchTracksAsync(const std::string& containerId, utils::ISubscriber<const Track&>& subscriber)
{
    resetTrackCount();
    auto container = std::make_shared<upnp::Item>(containerId);
    m_Server.getItemsInContainerAsync(container, [&] (const upnp::ItemPtr& item) {
        subscriber.onItem(itemToTrack(*item));
    }, 0, 0, upnp::Property::TrackNumber);
}

Track UPnPTrackFetcher::itemToTrack(upnp::Item& item)
{
    Track track;
    track.id        = item.getObjectId();
    track.albumId   = item.getMetaData(upnp::Property::ParentId);
    track.title     = item.getMetaData(upnp::Property::Title);
    track.artist    = item.getMetaData(upnp::Property::Artist);
    track.album     = item.getMetaData(upnp::Property::Album);
    track.genre     = item.getMetaData(upnp::Property::Genre);

    if (!item.getResources().empty())
    {
        track.filepath = item.getResources().front().getUrl();
        track.fileSize = stringops::toNumeric<uint64_t>(item.getResources().front().getMetaData("size"));
    }

    std::string trackNrStr = item.getMetaData(upnp::Property::TrackNumber);
    if (!trackNrStr.empty()) {
        track.trackNr = stringops::toNumeric<uint32_t>(trackNrStr);
    }

    if (track.trackNr < m_CurrentTrackNr) {
        ++m_CurrentDiscNr;
    }

    m_CurrentTrackNr = track.trackNr;
    track.discNr = m_CurrentDiscNr;

    std::string date = item.getMetaData(upnp::Property::Date);
    if (!(date.length() > 4))
    {
        track.year = stringops::toNumeric<uint32_t>(date.substr(0, 4));
    }

    auto& resources = item.getResources();
    if (!resources.empty())
    {
        auto& resource = resources.front();
        std::string duration = resource.getMetaData("duration");
        if (!duration.empty())
        {
            track.durationInSec = durationFromString(duration);
        }

        std::string bitrate = resource.getMetaData("bitrate");
        if (!bitrate.empty())
        {
            track.bitrate = stringops::toNumeric<uint32_t>(bitrate);
            track.bitrate /= 1000;
        }

        std::string sampleFrequency = resource.getMetaData("sampleFrequency");
        if (!sampleFrequency.empty())
        {
            track.sampleRate = stringops::toNumeric<uint32_t>(sampleFrequency);
        }

        std::string channels = resource.getMetaData("nrAudioChannels");
        if (!channels.empty())
        {
            track.channels = stringops::toNumeric<uint32_t>(channels);
        }
    }

    track.artUrl = item.getMetaData(upnp::Property::AlbumArt);
    
    return track;
}

UPnPAlbumFetcher::UPnPAlbumFetcher(upnp::MediaServer& server)
: m_Server(server)
{
}

void UPnPAlbumFetcher::processItem(const upnp::ItemPtr& item, utils::ISubscriber<const Album&>& subscriber)
{
    m_Server.getMetaData(item);
    subscriber.onItem(containerToAlbum(*item));
}

/*
void UPnPAlbumFetcher::finalItemReceived(void* pData)
{
    utils::ISubscriber<const Album&>* pSubscriber = reinterpret_cast<utils::ISubscriber<const Album&>*> (pData);
    assert(pSubscriber);
    pSubscriber->finalItemReceived();
}*/

Album UPnPAlbumFetcher::fetchAlbum(const std::string& albumId)
{
    auto container = std::make_shared<upnp::Item>(albumId);
    m_Server.getMetaData(container);

    return containerToAlbum(*container);
}

void UPnPAlbumFetcher::fetchAlbumAsync(const std::string& albumId, utils::ISubscriber<const Album&>& subscriber)
{
    auto container = std::make_shared<upnp::Item>(albumId);
    m_Server.getMetaDataAsync(container, [&] (const upnp::ItemPtr& item) {
        processItem(item, subscriber);
    });
}

void UPnPAlbumFetcher::fetchAlbums(const std::string& containerId, utils::ISubscriber<const Album&>& subscriber)
{
    auto container = std::make_shared<upnp::Item>(containerId);
    m_Server.getContainersInContainer(container, [&] (const upnp::ItemPtr& item) {
        processItem(item, subscriber);
    }, 0, 0);
}

void UPnPAlbumFetcher::fetchAlbumsAsync(const std::string& containerId, utils::ISubscriber<const Album&>& subscriber)
{
    m_Server.setCompletedCallback([&] () {
        subscriber.finalItemReceived();
    });
    
    auto container = std::make_shared<upnp::Item>(containerId);
    m_Server.getContainersInContainerAsync(container, [&] (const upnp::ItemPtr& item) {
        processItem(item, subscriber);
    }, 0, 0);
}

Album UPnPAlbumFetcher::containerToAlbum(const upnp::Item& container)
{
    Album album;
    album.id            = container.getObjectId();
    album.title         = container.getTitle();
    album.trackCount    = container.getChildCount();
    album.artist        = container.getMetaData(upnp::Property::Artist);
    album.artUrl        = container.getMetaData(upnp::Property::AlbumArt);
    album.genre         = container.getMetaData(upnp::Property::Genre);
    
    return album;
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
