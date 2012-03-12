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

#ifndef DISPATCHED_SUBSCRIBER_H
#define DISPATCHED_SUBSCRIBER_H

#include <glibmm.h>
#include <mutex>

#include "utils/subscriber.h"
#include <deque>

namespace Gejengel
{

template <typename T>
class IDispatchedSubscriber : public utils::ISubscriber<T>
{
public:
	IDispatchedSubscriber()
	{
		m_AlbumArtDispatcher.connect(sigc::mem_fun(this, &IDispatchedSubscriber::dispatchItem));
	}

	virtual ~IDispatchedSubscriber() {}

	void onItem(const T& item, void* pData = nullptr)
	{
		{
			std::lock_guard<std::mutex> lock(m_Mutex);
			m_ItemQueue.push_back(item);
			m_DataQueue.push_back(pData);
		}
		m_AlbumArtDispatcher();
	}

	virtual void onDispatchedItem(const T& item, void* pData = nullptr) = 0;

private:
	void dispatchItem()
	{
		std::lock_guard<std::mutex> lock(m_Mutex);
		while (!m_ItemQueue.empty())
		{
			onDispatchedItem(m_ItemQueue.front(), m_DataQueue.front());
			m_ItemQueue.pop_front();
			m_DataQueue.pop_front();
		}
	}

	std::deque<T> 		m_ItemQueue;
	std::deque<void*> 	m_DataQueue;
	std::mutex		    m_Mutex;
	Glib::Dispatcher    m_AlbumArtDispatcher;
};

}

#endif
