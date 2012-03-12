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

#include "musiclibrary.h"

#include <stdexcept>
#include <cassert>

#include "Core/settings.h"
#include "scanner.h"
#include "utils/log.h"
#include "utils/trace.h"

using namespace std;

namespace Gejengel
{

MusicLibrary::MusicLibrary(const Settings& settings)
: m_Settings(settings)
{
    utils::trace("Create MusicLibrary");
}

MusicLibrary::~MusicLibrary()
{
}

void MusicLibrary::addLibrarySubscriber(ILibrarySubscriber& subscriber)
{
    std::lock_guard<std::mutex> lock(m_SubscribersMutex);
    m_Subscribers.push_back(&subscriber);
}

std::vector<ILibrarySubscriber*> MusicLibrary::getSubscribers()
{
    std::lock_guard<std::mutex> lock(m_SubscribersMutex);
    return m_Subscribers;
}

void MusicLibrary::clearSubscribers()
{
	std::lock_guard<std::mutex> lock(m_SubscribersMutex);
	m_Subscribers.clear();
}

void MusicLibrary::newTrack(const Track& track)
{
    std::lock_guard<std::mutex> lock(m_SubscribersMutex);
    for (SubscriberIter iter = m_Subscribers.begin(); iter != m_Subscribers.end(); ++iter)
    {
        (*iter)->newTrack(track);
    }
}

void MusicLibrary::deletedTrack(const std::string& id)
{
    std::lock_guard<std::mutex> lock(m_SubscribersMutex);
    for (SubscriberIter iter = m_Subscribers.begin(); iter != m_Subscribers.end(); ++iter)
    {
        (*iter)->deletedTrack(id);
    }
}

void MusicLibrary::updatedTrack(const Track& track)
{
    std::lock_guard<std::mutex> lock(m_SubscribersMutex);
    for (SubscriberIter iter = m_Subscribers.begin(); iter != m_Subscribers.end(); ++iter)
    {
        (*iter)->updatedTrack(track);
    }
}

void MusicLibrary::newAlbum(const Album& album)
{   
    std::lock_guard<std::mutex> lock(m_SubscribersMutex);
    for (SubscriberIter iter = m_Subscribers.begin(); iter != m_Subscribers.end(); ++iter)
    {
        (*iter)->newAlbum(album);
    }
}

void MusicLibrary::deletedAlbum(const std::string& id)
{
    std::lock_guard<std::mutex> lock(m_SubscribersMutex);
    for (SubscriberIter iter = m_Subscribers.begin(); iter != m_Subscribers.end(); ++iter)
    {
        (*iter)->deletedAlbum(id);
    }
}

void MusicLibrary::updatedAlbum(const Album& album)
{   
    std::lock_guard<std::mutex> lock(m_SubscribersMutex);
    for (SubscriberIter iter = m_Subscribers.begin(); iter != m_Subscribers.end(); ++iter)
    {
        (*iter)->updatedAlbum(album);
    }
}

void MusicLibrary::libraryCleared()
{   
    std::lock_guard<std::mutex> lock(m_SubscribersMutex);
    for (SubscriberIter iter = m_Subscribers.begin(); iter != m_Subscribers.end(); ++iter)
    {
        (*iter)->libraryCleared();
    }
}

}

