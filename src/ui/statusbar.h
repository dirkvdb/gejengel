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
 
#ifndef STATUS_BAR_H
#define STATUS_BAR_H

#include <gtkmm.h>

#include "utils/types.h"

namespace Gejengel
{

class StatusBar: public Gtk::Statusbar
{
public:
    StatusBar();

    void setLibraryInfo(uint32_t albumCount, uint32_t songCount);

private:
    Gtk::HBox       m_StatusBox;
    Gtk::Label      m_AlbumCountLabel;
    Gtk::Label      m_TrackCountLabel;
};

}

#endif
