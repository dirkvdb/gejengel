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

#include "playqueue.h"

#include "utils/log.h"
#include "utils/fileoperations.h"
#include "utils/stringoperations.h"
#include "Core/gejengel.h"
#include "MusicLibrary/musiclibrary.h"
#include "MusicLibrary/track.h"

#include <fstream>
#include <cassert>

using namespace std;
using namespace utils;

#ifndef WIN32
#define CONFIG_DIR     ".config/gejengel"
#else
#define CONFIG_DIR     "gejengel"
#endif
#define QUEUE_FILE     "gejengel.q"

namespace Gejengel
{
    
PlayQueueItem::PlayQueueItem(const Track& track)
: m_Track(track)
{
}

std::string PlayQueueItem::getUri() const
{
    return m_Track.filepath;
}

Track PlayQueueItem::getTrack() const
{
    return m_Track;
}

PlayQueue::PlayQueue(IGejengelCore& core)
: m_Core(core)
, m_QueueIndex(-1)   
, m_Destroy(false)
{
}

PlayQueue::~PlayQueue()
{
    m_Destroy = true;
}

void PlayQueue::loadQueueFromFile()
{
    try
    {
        string queuePath = determinePlayQueuePath();
        
        std::string playQueue = fileops::readTextFile(queuePath);

        {
            std::lock_guard<std::mutex> lock(m_TracksMutex);
            vector<std::string> ids = stringops::tokenize(playQueue, "\n");
            for (size_t i = 0; i < ids.size() && !m_Destroy; ++i)
            {
                if (!ids[i].empty())
                {
                    queueTrack(ids[i]);
                }
            }
        }

        fileops::deleteFile(queuePath);
    }
    catch (std::exception&) {}
}


void PlayQueue::loadQueue()
{
    loadQueueFromFile();
}

void PlayQueue::saveQueue()
{
    if (m_Tracks.empty())
    {
        return;
    }

    ofstream file(determinePlayQueuePath().c_str());
    if (!file.is_open())
    {
        return;
    }

    std::lock_guard<std::mutex> lock(m_TracksMutex);
    for (auto& track : m_Tracks)
    {
        file << track->getTrack().id << endl;
    }
}

void PlayQueue::queueTrack(const Track& track, int32_t index)
{
    try
    {
        int32_t listIndex = index;
        
        auto queueItem = std::make_shared<PlayQueueItem>(track);

        if (listIndex == -1)
        {
            std::lock_guard<std::mutex> lock(m_TracksMutex);
            listIndex = m_Tracks.size();
            m_Tracks.push_back(queueItem);
        }
        else
        {
            std::lock_guard<std::mutex> lock(m_TracksMutex);
            auto iter = m_Tracks.begin();
            for (int32_t i = 0; i < index && iter != m_Tracks.end(); ++i)
            {
                ++iter;
            }

            if (iter == m_Tracks.end())
            {
                index = m_Tracks.size();
            }

            m_Tracks.insert(iter, queueItem);
        }
    
        log::debug("Track queued: %s", track.id);
        notifyTrackQueued(listIndex, track);

        if (index == -1)
        {  
            m_Core.play();
        }
    }
    catch (std::exception& e)
    {
        log::error("Failed to queue track (%s: %s)", track.id, e.what());
    }
}

void PlayQueue::queueTrack(const std::string& id, int32_t index)
{
    try
    {
    	{
    	    std::lock_guard<std::mutex> lock(m_TracksMutex);
    		m_IndexMap[id] = index;
    	}
        m_Core.getLibraryAccess().getTrackAsync(id, *this);
    }
    catch (logic_error& e)
    {
        log::error("Failed to queue track (%s: %s)", id, e.what());
    }
}

void PlayQueue::queueAlbum(const std::string& id, int32_t index)
{
    try
    {
        m_QueueIndex = index;
        m_Core.getLibraryAccess().getTracksFromAlbumAsync(id, *this);
    }
    catch (logic_error& e)
    {
        log::error("Failed to queue album: %s", e.what());
    }
}

void PlayQueue::queueRandomTracks(uint32_t count)
{
    try
    {
        m_QueueIndex = -1;
        m_Core.getLibraryAccess().getRandomTracksAsync(count, *this);
    }
    catch (logic_error& e)
    {
        log::error("Failed to queue random tracks: %s", e.what());
    }
}

void PlayQueue::queueRandomAlbum()
{
    try
    {
        m_QueueIndex = -1;
        m_Core.getLibraryAccess().getRandomAlbumAsync(*this);
    }
    catch (logic_error& e)
    {
        log::error("Failed to queue random album: %s", e.what());
    }
}

size_t PlayQueue::getNumberOfTracks() const
{
    return m_Tracks.size();
}

std::shared_ptr<audio::ITrack> PlayQueue::dequeueNextTrack()
{
    std::lock_guard<std::mutex> lock(m_TracksMutex);
    if (m_Tracks.empty())
    {
        return false;
    }

    m_CurrentTrack = m_Tracks.front();
    m_Tracks.pop_front();
    notifyTrackRemoved(0);
    
    return m_CurrentTrack;
}

Track PlayQueue::getCurrentTrack()
{
    if (m_CurrentTrack)
    {
        return m_CurrentTrack->getTrack();
    }
    
    return Track();
}

void PlayQueue::removeTrack(uint32_t index)
{
    std::lock_guard<std::mutex> lock(m_TracksMutex);
    if (m_Tracks.empty())
    {
        return;
    }

    auto iter = m_Tracks.begin();
    for (uint32_t i = 0; i < index && iter != m_Tracks.end(); ++i)
    {
        ++iter;
    }

    if (iter == m_Tracks.end())
    {
        log::error("PlayQueue::removeTrack invalid index: %d", index);
        return;
    }

    m_Tracks.erase(iter);
    notifyTrackRemoved(index);
}

void PlayQueue::removeTracks(std::vector<uint32_t> indexes)
{
    sort(indexes.begin(), indexes.end());

    for (size_t i = 0; i < indexes.size(); ++i)
    {
        removeTrack(indexes[i] - i);
    }
}

void PlayQueue::moveTrack(uint32_t sourceIndex, uint32_t destIndex)
{
    if (sourceIndex >= m_Tracks.size())
    {
        log::error("PlayQueue::moveTrack: invalid fromIndex");
        return;
    }

    std::lock_guard<std::mutex> lock(m_TracksMutex);
    auto sourceIter = m_Tracks.begin();
    for (uint32_t i = 0; i < sourceIndex; ++i)
    {
        ++sourceIter;
    }

    auto track = *sourceIter;
    m_Tracks.erase(sourceIter);

    auto destIter = m_Tracks.begin();
    for (uint32_t i = 0; i < destIndex && destIter != m_Tracks.end(); ++i)
    {
        ++destIter;
    }

    if (destIter == m_Tracks.end())
    {
        destIndex = m_Tracks.size();
    }

    m_Tracks.insert(destIter, track);

    notifyTrackMoved(sourceIndex, destIndex);
}

bool PlayQueue::getTrackInfo(uint32_t index, Track& track)
{
    std::lock_guard<std::mutex> lock(m_TracksMutex);
    if (index >= m_Tracks.size())
    {
        return false;
    }

    auto iter = m_Tracks.begin();
    for (uint32_t i = 0; i < index; ++i)
    {
        ++iter;
    }

    track = (*iter)->getTrack();
    return true;
}

void PlayQueue::clear()
{
    std::lock_guard<std::mutex> lock(m_TracksMutex);
    m_Tracks.clear();
    notifyQueueCleared();
}

std::list<Track> PlayQueue::getTracks()
{
    std::list<Track> tracks;
    for (auto& item : m_Tracks)
    {
        tracks.push_back(item->getTrack());
    }
    
    return tracks;
}

void PlayQueue::subscribe(PlayQueueSubscriber& subscriber)
{
    std::lock_guard<std::mutex> lock(m_SubscribersMutex);
    m_Subscribers.push_back(&subscriber);
}

void PlayQueue::unsubscribe(PlayQueueSubscriber& subscriber)
{
    std::lock_guard<std::mutex> lock(m_SubscribersMutex);
    for (vector<PlayQueueSubscriber*>::iterator iter = m_Subscribers.begin(); iter != m_Subscribers.end(); ++iter)
    {
        if (*iter == &subscriber)
        {
            iter = m_Subscribers.erase(iter);
            break;
        }
    }
}

void PlayQueue::notifyTrackQueued(uint32_t index, const Track& track)
{
    std::lock_guard<std::mutex> lock(m_SubscribersMutex);
    for (size_t j = 0; j < m_Subscribers.size(); ++j)
    {
        m_Subscribers[j]->onTrackQueued(index, track);
    }
}

void PlayQueue::notifyTrackRemoved(uint32_t index)
{
    std::lock_guard<std::mutex> lock(m_SubscribersMutex);
    for (size_t j = 0; j < m_Subscribers.size(); ++j)
    {
        m_Subscribers[j]->onTrackRemoved(index);
    }
}

void PlayQueue::notifyTrackMoved(uint32_t sourceIndex, uint32_t destIndex)
{
    std::lock_guard<std::mutex> lock(m_SubscribersMutex);
    for (size_t j = 0; j < m_Subscribers.size(); ++j)
    {
        m_Subscribers[j]->onTrackMoved(sourceIndex, destIndex);
    }
}

void PlayQueue::notifyQueueCleared()
{
    std::lock_guard<std::mutex> lock(m_SubscribersMutex);
    for (size_t j = 0; j < m_Subscribers.size(); ++j)
    {
        m_Subscribers[j]->onQueueCleared();
    }
}

string PlayQueue::determinePlayQueuePath()
{
    string queuePath;
    string home = fileops::getHomeDirectory();
    string configDir = fileops::combinePath(home, CONFIG_DIR);

    if (!fileops::pathExists(configDir))
    {
        fileops::createDirectory(configDir);
    }

    return fileops::combinePath(configDir, QUEUE_FILE);
}

void PlayQueue::onItem(const Track& track, void* pData)
{
    if (m_QueueIndex > 0)
    {
		queueTrack(track, m_QueueIndex);

		if (m_QueueIndex != -1)
		{
			++m_QueueIndex;
		}
    }
    else
    {
        int32_t index = -1;

        {
            std::lock_guard<std::mutex> lock(m_TracksMutex);
            auto iter = m_IndexMap.find(track.id);
            if (iter != m_IndexMap.end())
            {
                index = iter->second;
                m_IndexMap.erase(iter);
            }
        }

    	queueTrack(track, index);
    }
}

}
