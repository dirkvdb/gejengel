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

#ifndef SIGNALS_H
#define SIGNALS_H

#include <glibmm/ustring.h>
#include "MusicLibrary/track.h"

#include "utils/types.h"

namespace Gejengel
{
    
typedef sigc::signal<void, const std::string&> SignalAlbumChanged;
typedef sigc::signal<void, const std::string&, int32_t> SignalAlbumQueued;
typedef sigc::signal<void, const std::string&, int32_t> SignalTrackQueued;
typedef sigc::signal<void, const Glib::ustring&, const Glib::ustring&> SignalCellButtonClicked;
typedef sigc::signal<void, const std::string&> SignalAlbumInfoRequested;
typedef sigc::signal<void> SignalBackButtonPressed;
typedef sigc::signal<void> SignalClose;
typedef sigc::signal<void> SignalModelUpdated;
typedef sigc::signal<void, const Glib::ustring&> SignalSearchChanged;

}

#endif
