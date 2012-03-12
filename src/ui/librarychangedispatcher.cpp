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

#include "librarychangedispatcher.h"

#include "MusicLibrary/track.h"
#include "MusicLibrary/album.h"

namespace Gejengel
{

ScanDispatcher::ScanDispatcher()
: m_NumTracks(0)
, m_ScannedTracks(0)
{
    sendScanStart.connect(sigc::mem_fun(this, &ScanDispatcher::dispatchScanStart));
    sendScanUpdate.connect(sigc::mem_fun(this, &ScanDispatcher::dispatchScanUpdate));
    sendScanFinish.connect(sigc::mem_fun(this, &ScanDispatcher::dispatchScanFinish));
    sendScanFailed.connect(sigc::mem_fun(this, &ScanDispatcher::dispatchScanFailed));
}

void ScanDispatcher::addSubscriber(IScanSubscriber& subscriber)
{
    std::lock_guard<std::mutex> lock(m_VectorMutex);
    m_Subscribers.push_back(&subscriber);
}

void ScanDispatcher::scanStart(uint32_t numTracks)
{
    m_NumTracks = numTracks;
    sendScanStart();
}

void ScanDispatcher::scanUpdate(uint32_t scannedTracks)
{
    m_ScannedTracks = scannedTracks;
    sendScanUpdate();
}

void ScanDispatcher::scanFinish()
{
    sendScanFinish();
}

void ScanDispatcher::scanFailed()
{
    sendScanFailed();
}

void ScanDispatcher::dispatchScanStart()
{
    std::lock_guard<std::mutex> lock(m_VectorMutex);
    for (size_t j = 0; j < m_Subscribers.size(); ++j)
    {
        m_Subscribers[j]->scanStart(m_NumTracks);
    }
}

void ScanDispatcher::dispatchScanUpdate()
{
    std::lock_guard<std::mutex> lock(m_VectorMutex);
    for (size_t j = 0; j < m_Subscribers.size(); ++j)
    {
        m_Subscribers[j]->scanUpdate(m_ScannedTracks);
    }
}

void ScanDispatcher::dispatchScanFinish()
{
    std::lock_guard<std::mutex> lock(m_VectorMutex);
    for (size_t j = 0; j < m_Subscribers.size(); ++j)
    {
        m_Subscribers[j]->scanFinish();
    }
}

void ScanDispatcher::dispatchScanFailed()
{
    std::lock_guard<std::mutex> lock(m_VectorMutex);
    for (size_t j = 0; j < m_Subscribers.size(); ++j)
    {
        m_Subscribers[j]->scanFailed();
    }
}

LibraryChangeDispatcher::LibraryChangeDispatcher()
{
    sendTrackAdded.connect(sigc::mem_fun(this, &LibraryChangeDispatcher::dispatchNewTrack));
    sendTrackDeleted.connect(sigc::mem_fun(this, &LibraryChangeDispatcher::dispatchDeletedTrack));
    sendTrackUpdated.connect(sigc::mem_fun(this, &LibraryChangeDispatcher::dispatchUpdatedTrack));
    sendAlbumAdded.connect(sigc::mem_fun(this, &LibraryChangeDispatcher::dispatchNewAlbum));
    sendAlbumDeleted.connect(sigc::mem_fun(this, &LibraryChangeDispatcher::dispatchDeletedAlbum));
    sendAlbumUpdated.connect(sigc::mem_fun(this, &LibraryChangeDispatcher::dispatchUpdatedAlbum));
    sendLibraryCleared.connect(sigc::mem_fun(this, &LibraryChangeDispatcher::dispatchLibraryCleared));
}

void LibraryChangeDispatcher::addSubscriber(ILibrarySubscriber& subscriber)
{
    m_Subscribers.push_back(&subscriber);
}

void LibraryChangeDispatcher::newTrack(const Track& track)
{
    std::lock_guard<std::mutex> lock(m_VectorMutex);
    m_NewTracks.push_back(track);
    sendTrackAdded();
}

void LibraryChangeDispatcher::newAlbum(const Album& album)
{
    std::lock_guard<std::mutex> lock(m_VectorMutex);
    m_NewAlbums.push_back(album);
    sendAlbumAdded();
}

void LibraryChangeDispatcher::deletedTrack(const std::string& trackId)
{
    std::lock_guard<std::mutex> lock(m_VectorMutex);
    m_DeletedTracks.push_back(trackId);
    sendTrackDeleted();
}

void LibraryChangeDispatcher::deletedAlbum(const std::string& albumId)
{
    std::lock_guard<std::mutex> lock(m_VectorMutex);
    m_DeletedAlbums.push_back(albumId);
    sendAlbumDeleted();
}

void LibraryChangeDispatcher::updatedTrack(const Track& track)
{
    std::lock_guard<std::mutex> lock(m_VectorMutex);
    m_UpdatedTracks.push_back(track);
    sendTrackUpdated();
}

void LibraryChangeDispatcher::updatedAlbum(const Album& album)
{
    std::lock_guard<std::mutex> lock(m_VectorMutex);
    m_UpdatedAlbums.push_back(album);
    sendAlbumUpdated();
}

void LibraryChangeDispatcher::libraryCleared()
{
    sendLibraryCleared();
}

void LibraryChangeDispatcher::dispatchNewTrack()
{
    std::lock_guard<std::mutex> lock(m_VectorMutex);
    for (size_t i = 0; i < m_NewTracks.size(); ++i)
    {
        for (size_t j = 0; j < m_Subscribers.size(); ++j)
        {
            m_Subscribers[j]->newTrack(m_NewTracks[i]);
        }
    }
    m_NewTracks.clear();
}

void LibraryChangeDispatcher::dispatchNewAlbum()
{
    std::lock_guard<std::mutex> lock(m_VectorMutex);
    for (size_t i = 0; i < m_NewAlbums.size(); ++i)
    {
        for (size_t j = 0; j < m_Subscribers.size(); ++j)
        {
            m_Subscribers[j]->newAlbum(m_NewAlbums[i]);
        }
    }
    m_NewAlbums.clear();
}

void LibraryChangeDispatcher::dispatchDeletedTrack()
{
    std::lock_guard<std::mutex> lock(m_VectorMutex);
    for (size_t i = 0; i < m_DeletedTracks.size(); ++i)
    {
        for (size_t j = 0; j < m_Subscribers.size(); ++j)
        {
            m_Subscribers[j]->deletedTrack(m_DeletedTracks[i]);
        }
    }
    m_DeletedTracks.clear();
}

void LibraryChangeDispatcher::dispatchDeletedAlbum()
{
    std::lock_guard<std::mutex> lock(m_VectorMutex);
    for (size_t i = 0; i < m_DeletedAlbums.size(); ++i)
    {
        for (size_t j = 0; j < m_Subscribers.size(); ++j)
        {
            m_Subscribers[j]->deletedAlbum(m_DeletedAlbums[i]);
        }
    }
    m_DeletedAlbums.clear();
}

void LibraryChangeDispatcher::dispatchUpdatedTrack()
{
    std::lock_guard<std::mutex> lock(m_VectorMutex);
    for (size_t i = 0; i < m_UpdatedTracks.size(); ++i)
    {
        for (size_t j = 0; j < m_Subscribers.size(); ++j)
        {
            m_Subscribers[j]->updatedTrack(m_UpdatedTracks[i]);
        }
    }
    m_UpdatedTracks.clear();
}

void LibraryChangeDispatcher::dispatchUpdatedAlbum()
{
    std::lock_guard<std::mutex> lock(m_VectorMutex);
    for (size_t i = 0; i < m_UpdatedAlbums.size(); ++i)
    {
        for (size_t j = 0; j < m_Subscribers.size(); ++j)
        {
            m_Subscribers[j]->updatedAlbum(m_UpdatedAlbums[i]);
        }
    }
    m_UpdatedAlbums.clear();
}

void LibraryChangeDispatcher::dispatchLibraryCleared()
{
    for (size_t j = 0; j < m_Subscribers.size(); ++j)
    {
        m_Subscribers[j]->libraryCleared();
    }
}

PlayQueueDispatcher::PlayQueueDispatcher(PlayQueueSubscriber& subscriber)
: m_Subscriber(subscriber)
{
    sendTrackQueued.connect(sigc::mem_fun(this, &PlayQueueDispatcher::dispatchTrackQueued));
    sendTrackRemoved.connect(sigc::mem_fun(this, &PlayQueueDispatcher::dispatchTrackRemoved));
    sendTrackMoved.connect(sigc::mem_fun(this, &PlayQueueDispatcher::dispatchTrackMoved));
    sendQueueCleared.connect(sigc::mem_fun(this, &PlayQueueDispatcher::dispatchQueueCleared));
}

void PlayQueueDispatcher::onTrackQueued(uint32_t index, const Track& track)
{
    m_QueuedIndexes.push_back(index);
    m_QueuedTracks.push_back(track);
    sendTrackQueued();
}

void PlayQueueDispatcher::onTrackRemoved(uint32_t index)
{
    m_RemovedIndexes.push_back(index);
    sendTrackRemoved();
}

void PlayQueueDispatcher::onTrackMoved(uint32_t sourceIndex, uint32_t destIndex)
{
    m_MovedIndexes.push_back(MoveInfo(sourceIndex, destIndex));
    sendTrackMoved();
}

void PlayQueueDispatcher::onQueueCleared()
{
    sendQueueCleared();
}

void PlayQueueDispatcher::dispatchTrackQueued()
{
    std::lock_guard<std::mutex> lock(m_VectorMutex);
    for (size_t i = 0; i < m_QueuedIndexes.size(); ++i)
    {
        m_Subscriber.onTrackQueued(m_QueuedIndexes[i], m_QueuedTracks[i]);
    }

    m_QueuedIndexes.clear();
    m_QueuedTracks.clear();
}

void PlayQueueDispatcher::dispatchTrackRemoved()
{
    std::lock_guard<std::mutex> lock(m_VectorMutex);
    for (size_t i = 0; i < m_RemovedIndexes.size(); ++i)
    {
        m_Subscriber.onTrackRemoved(m_RemovedIndexes[i]);
    }

    m_RemovedIndexes.clear();
}

void PlayQueueDispatcher::dispatchTrackMoved()
{
    std::lock_guard<std::mutex> lock(m_VectorMutex);
    for (size_t i = 0; i < m_MovedIndexes.size(); ++i)
    {
        m_Subscriber.onTrackMoved(m_MovedIndexes[i].sourceIndex, m_MovedIndexes[i].destIndex);
    }

    m_MovedIndexes.clear();
}

void PlayQueueDispatcher::dispatchQueueCleared()
{
    m_Subscriber.onQueueCleared();
}

}
