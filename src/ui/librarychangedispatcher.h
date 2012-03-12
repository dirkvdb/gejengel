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

#ifndef LIBRARY_CHANGE_DISPATCHER
#define LIBRARY_CHANGE_DISPATCHER

#include <glibmm/dispatcher.h>
#include <mutex>
#include <type_traits>

#include "utils/types.h"
#include "MusicLibrary/subscribers.h"
#include "MusicLibrary/track.h"
#include "MusicLibrary/album.h"
#include "Core/playqueue.h"
#include "utils/signal.h"

namespace Gejengel
{
    
class ScanDispatcher : public IScanSubscriber
{
public:
    ScanDispatcher();

    void addSubscriber(IScanSubscriber& subscriber);

    void scanStart(uint32_t numTracks);
    void scanUpdate(uint32_t scannedTracks);
    void scanFinish();
    void scanFailed();

private:
    Glib::Dispatcher sendScanStart;
    Glib::Dispatcher sendScanUpdate;
    Glib::Dispatcher sendScanFinish;
    Glib::Dispatcher sendScanFailed;

    void dispatchScanStart();
    void dispatchScanUpdate();
    void dispatchScanFinish();
    void dispatchScanFailed();

    std::vector<IScanSubscriber*>       m_Subscribers;
    uint32_t                            m_NumTracks;
    uint32_t                            m_ScannedTracks;
    std::mutex                          m_VectorMutex;
};

class LibraryChangeDispatcher : public ILibrarySubscriber
{
public:
    LibraryChangeDispatcher();

    void addSubscriber(ILibrarySubscriber& subscriber);

    void newTrack(const Track& track);
    void newAlbum(const Album& album);
    void deletedTrack(const std::string& trackId);
    void deletedAlbum(const std::string& albumId);
    void updatedTrack(const Track& track);
    void updatedAlbum(const Album& album);
    void libraryCleared();

private:
    Glib::Dispatcher sendTrackAdded;
    Glib::Dispatcher sendTrackDeleted;
    Glib::Dispatcher sendTrackUpdated;
    Glib::Dispatcher sendAlbumAdded;
    Glib::Dispatcher sendAlbumDeleted;
    Glib::Dispatcher sendAlbumUpdated;
    Glib::Dispatcher sendLibraryCleared;

    void dispatchNewTrack();
    void dispatchNewAlbum();
    void dispatchDeletedTrack();
    void dispatchDeletedAlbum();
    void dispatchUpdatedTrack();
    void dispatchUpdatedAlbum();
    void dispatchLibraryCleared();

    std::vector<ILibrarySubscriber*>    m_Subscribers;
    std::vector<Track>                  m_NewTracks;
    std::vector<Album>                  m_NewAlbums;
    std::vector<std::string>            m_DeletedTracks;
    std::vector<std::string>            m_DeletedAlbums;
    std::vector<Track>                  m_UpdatedTracks;
    std::vector<Album>                  m_UpdatedAlbums;
    std::mutex                          m_VectorMutex;
};

class PlayQueueDispatcher : public PlayQueueSubscriber
{
public:
    PlayQueueDispatcher(PlayQueueSubscriber& subscriber);
        
    void onTrackQueued(uint32_t index, const Track& track);
    void onTrackRemoved(uint32_t index);
    void onTrackMoved(uint32_t sourceIndex, uint32_t destIndex);
    void onQueueCleared();

private:
    Glib::Dispatcher sendTrackQueued;
    Glib::Dispatcher sendTrackRemoved;
    Glib::Dispatcher sendTrackMoved;
    Glib::Dispatcher sendQueueCleared;

    void dispatchTrackQueued();
    void dispatchTrackRemoved();
    void dispatchTrackMoved();
    void dispatchQueueCleared();

    struct MoveInfo
    {
        MoveInfo(uint32_t sourceIndex, uint32_t destIndex)
        : sourceIndex(sourceIndex), destIndex(destIndex)
        {}

        uint32_t sourceIndex;
        uint32_t destIndex;
    };

    PlayQueueSubscriber&                m_Subscriber;
    std::mutex                          m_VectorMutex;
    std::vector<Track>                  m_QueuedTracks;
    std::vector<uint32_t>               m_QueuedIndexes;
    std::vector<uint32_t>               m_RemovedIndexes;
    std::vector<MoveInfo>               m_MovedIndexes;
};


template<typename T>
class UIDispatcher : public utils::ISubscriber<T>
{
public:
    UIDispatcher(utils::ISubscriber<T>& subscriber)
    : m_Subscriber(subscriber)
    {
        sendItem.connect(sigc::mem_fun(this, &UIDispatcher<T>::dispatchItem));
        sendFinalItem.connect(sigc::mem_fun(this, &UIDispatcher<T>::dispatchFinalItem));
    }

    void onItem(const T& item, void* pData)
    {
        std::lock_guard<std::mutex> lock(m_VectorMutex);
        m_Items.push_back(item);
        m_Datas.push_back(pData);
        sendItem();
    }

    void finalItemReceived()
    {
        sendFinalItem();
    }

private:
    Glib::Dispatcher sendItem;
    Glib::Dispatcher sendFinalItem;

    void dispatchItem()
    {
        std::lock_guard<std::mutex> lock(m_VectorMutex);
        for (size_t i = 0; i < m_Items.size(); ++i)
        {
            m_Subscriber.onItem(m_Items[i], m_Datas[i]);
        }
        m_Items.clear();
        m_Datas.clear();
    }

    void dispatchFinalItem()
    {
        m_Subscriber.finalItemReceived();
    }

    utils::ISubscriber<T>&  m_Subscriber;
    std::vector<T>      	m_Items;
    std::vector<void*>  	m_Datas;
    std::mutex        	    m_VectorMutex;
};

template <typename T,
          typename TStore = typename std::remove_const<typename std::remove_reference<T>::type>::type>
class SignalUIDispatcher
{
public:
    SignalUIDispatcher()
    {
        sendItem.connect(sigc::mem_fun(this, &SignalUIDispatcher<T>::dispatchItem));
    }

    void onItem(T item)
    {
        std::lock_guard<std::mutex> lock(m_VectorMutex);
        m_Items.push_back(item);
        sendItem();
    }

    utils::Signal<void(T)> DispatchedItemEvent;

private:
    void dispatchItem()
    {
        std::lock_guard<std::mutex> lock(m_VectorMutex);
        for (auto item : m_Items)
        {
            DispatchedItemEvent(item);
        }

        m_Items.clear();
    }

    Glib::Dispatcher sendItem;

    std::vector<TStore>    m_Items;
    std::mutex             m_VectorMutex;
};

}

#endif
