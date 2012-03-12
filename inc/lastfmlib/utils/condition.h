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

#ifndef CONDITION_H
#define CONDITION_H

#ifndef WIN32
#include <pthread.h>
#else
#include <Windows.h>
#endif

#include "types.h"

namespace utils
{

class Mutex;

class Condition
{
public:
    Condition();
    ~Condition();

    void wait(Mutex& mutex);
    bool wait(Mutex& mutex, int32_t timeoutInMs);
    void signal();
    void broadcast();

private:
#ifndef WIN32
    pthread_cond_t  m_Condition;
#else
    HANDLE m_Condition;
#endif
};

}

#endif
