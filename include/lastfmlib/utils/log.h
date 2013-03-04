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

#ifndef UTILS_LOG_H
#define UTILS_LOG_H

namespace Log
{
    template<typename T1>
    void info(const T1& t1);
    template<typename T1, typename T2>
    void info(const T1& t1, const T2& t2);
    template<typename T1, typename T2, typename T3>
    void info(const T1& t1, const T2& t2, const T3& t3);
    template<typename T1, typename T2, typename T3, typename T4>
    void info(const T1& t1, const T2& t2, const T3& t3, const T4& t4);
    template<typename T1, typename T2, typename T3, typename T4, typename T5>
    void info(const T1& t1, const T2& t2, const T3& t3, const T4& t4, const T5& t5);
    template<typename T1, typename T2, typename T3, typename T4, typename T5, typename T6>
    void info(const T1& t1, const T2& t2, const T3& t3, const T4& t4, const T5& t5, const T6& t6);

    template<typename T1>
    void warn(const T1& t1);
    template<typename T1, typename T2>
    void warn(const T1& t1, const T2& t2);
    template<typename T1, typename T2, typename T3>
    void warn(const T1& t1, const T2& t2, const T3& t3);
    template<typename T1, typename T2, typename T3, typename T4>
    void warn(const T1& t1, const T2& t2, const T3& t3, const T4& t4);
    template<typename T1, typename T2, typename T3, typename T4, typename T5>
    void warn(const T1& t1, const T2& t2, const T3& t3, const T4& t4, const T5& t5);
    template<typename T1, typename T2, typename T3, typename T4, typename T5, typename T6>
    void warn(const T1& t1, const T2& t2, const T3& t3, const T4& t4, const T5& t5, const T6& t6);

    template<typename T1>
    void critical(const T1& t1);
    template<typename T1, typename T2>
    void critical(const T1& t1, const T2& t2);
    template<typename T1, typename T2, typename T3>
    void critical(const T1& t1, const T2& t2, const T3& t3);
    template<typename T1, typename T2, typename T3, typename T4>
    void critical(const T1& t1, const T2& t2, const T3& t3, const T4& t4);
    template<typename T1, typename T2, typename T3, typename T4, typename T5>
    void critical(const T1& t1, const T2& t2, const T3& t3, const T4& t4, const T5& t5);
    template<typename T1, typename T2, typename T3, typename T4, typename T5, typename T6>
    void critical(const T1& t1, const T2& t2, const T3& t3, const T4& t4, const T5& t5, const T6& t6);

    template<typename T1>
    void error(const T1& t1);
    template<typename T1, typename T2>
    void error(const T1& t1, const T2& t2);
    template<typename T1, typename T2, typename T3>
    void error(const T1& t1, const T2& t2, const T3& t3);
    template<typename T1, typename T2, typename T3, typename T4>
    void error(const T1& t1, const T2& t2, const T3& t3, const T4& t4);
    template<typename T1, typename T2, typename T3, typename T4, typename T5>
    void error(const T1& t1, const T2& t2, const T3& t3, const T4& t4, const T5& t5);
    template<typename T1, typename T2, typename T3, typename T4, typename T5, typename T6>
    void error(const T1& t1, const T2& t2, const T3& t3, const T4& t4, const T5& t5, const T6& t6);

    template<typename T1>
    void debug(const T1& t1);
    template<typename T1, typename T2>
    void debug(const T1& t1, const T2& t2);
    template<typename T1, typename T2, typename T3>
    void debug(const T1& t1, const T2& t2, const T3& t3);
    template<typename T1, typename T2, typename T3, typename T4>
    void debug(const T1& t1, const T2& t2, const T3& t3, const T4& t4);
    template<typename T1, typename T2, typename T3, typename T4, typename T5>
    void debug(const T1& t1, const T2& t2, const T3& t3, const T4& t4, const T5& t5);
    template<typename T1, typename T2, typename T3, typename T4, typename T5, typename T6>
    void debug(const T1& t1, const T2& t2, const T3& t3, const T4& t4, const T5& t5, const T6& t6);
    template<typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7>
    void debug(const T1& t1, const T2& t2, const T3& t3, const T4& t4, const T5& t5, const T6& t6, const T7& t7);
}

#include "log.cpp"

#endif
