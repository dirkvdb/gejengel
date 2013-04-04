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

#ifndef PLAYQUEUE_H
#define PLAYQUEUE_H

#include "utils/subscriber.h"
#include "utils/types.h"
#include "audio/audioplaylistinterface.h"
#include "MusicLibrary/track.h"


#include <list>
#include <vector>
#include <map>
#include <string>
#include <mutex>

namespace Gejengel
{

class IGejengelCore;

class PlayQueueSubscriber
{
public:
    virtual ~PlayQueueSubscriber() {};

    virtual void onTrackQueued(uint32_t index, const Track& track) {}
    virtual void onTrackRemoved(uint32_t index) {}
    virtual void onTrackMoved(uint32_t sourceIndex, uint32_t destIndex) {}
    virtual void onQueueCleared() {}
};

class PlayQueue : public utils::ISubscriber<const Track&>
                , public audio::IPlaylist
{
public:
    PlayQueue(IGejengelCore& core);
    virtual ~PlayQueue();

    void queueTrack(const Track& track, int32_t index = -1);
    void queueTrack(const std::string& id, int32_t index = -1);
    void queueAlbum(const std::string& id, int32_t index = -1);

    void queueRandomTracks(uint32_t count);
    void queueRandomAlbum();

    // IPLaylist
    bool dequeueNextTrack(std::string& track);
    size_t getNumberOfTracks() const;

    Track getCurrentTrack();
    void removeTrack(uint32_t index);
    void removeTracks(std::vector<uint32_t> indexes);
    void moveTrack(uint32_t sourceIndex, uint32_t destIndex);
    bool getTrackInfo(uint32_t index, Track& track);
    void clear();
    const std::list<Track>& getTracks();

    void subscribe(PlayQueueSubscriber& subscriber);
    void unsubscribe(PlayQueueSubscriber& subscriber);
    
    void loadQueue();
    void saveQueue();

    void onItem(const Track& track, void* pData = nullptr);

private:
    void notifyTrackQueued(uint32_t index, const Track& track);
    void notifyTrackRemoved(uint32_t index);
    void notifyTrackMoved(uint32_t sourceIndex, uint32_t destIndex);
    void notifyQueueCleared();
    void loadQueueFromFile();

    static std::string determinePlayQueuePath();

    IGejengelCore&                      m_Core;
    std::list<Track>                    m_Tracks;
    std::vector<PlayQueueSubscriber*>   m_Subscribers;
    int32_t                             m_QueueIndex;
    std::map<std::string, int32_t>		m_IndexMap;
    
    Track                               m_CurrentTrack; //the last popped track

    std::mutex                          m_TracksMutex;
    std::mutex                          m_SubscribersMutex;

    bool                                m_Destroy;
};

}

#endif
