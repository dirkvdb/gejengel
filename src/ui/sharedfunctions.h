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

#ifndef SHARED_FUNCTIONS_H
#define SHARED_FUNCTIONS_H

#include <sstream>
#include <iomanip>
#include <gdkmm.h>
#include <glibmm/ustring.h>

#include "utils/types.h"

namespace Gejengel
{

class Album;
class AlbumArt;

namespace Shared
{
    void durationToString(uint32_t duration, Glib::ustring& durationString);
    uint32_t durationFromString(const Glib::ustring& durationString);
    Cairo::RefPtr<Cairo::ImageSurface> pixBufToSurface(Glib::RefPtr<Gdk::Pixbuf> pixBuf);
    Glib::RefPtr<Gdk::Pixbuf> createCoverPixBuf(const AlbumArt& albumArt, int32_t size);
    Glib::RefPtr<Gdk::Pixbuf> createCoverPixBufWithOverlay(const AlbumArt& albumArt, int32_t size);
}

}

#endif
