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

#ifndef THREAD_H
#define THREAD_H

#ifndef WIN32
#include <pthread.h>
#else
#include <Windows.h>
#endif

namespace utils
{
typedef void* (*ThreadFunction)(void* pInstance);

class Thread
{
public:
    Thread(ThreadFunction pfnThreadFunction, void* pInstance);
    ~Thread();

    void start();
    void join();
    void cancel();
    bool isRunning();

private:
    struct InstancePointers
    {
        Thread* pThreadInstance;
        void*   pRunInstance;
    };

#ifndef WIN32
    static void* onThreadStart(void* data);
    static void onThreadExit(void* data);
#else
    static DWORD WINAPI winThreadFunc(LPVOID pData);
#endif

#ifndef WIN32
    pthread_t           m_Thread;
    pthread_key_t       m_Key;
#else
    HANDLE              m_Thread;
    DWORD               m_ThreadId;
#endif
    
    ThreadFunction      m_pfnThreadFunction;
    InstancePointers    m_InstancePtrs;
};
}

#endif
