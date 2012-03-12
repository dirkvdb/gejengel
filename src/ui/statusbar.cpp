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

#include "statusbar.h"

#include <sstream>
#include <glibmm/i18n.h>
#include <glibmm/ustring.h>

#include "utils/log.h"


namespace Gejengel
{

StatusBar::StatusBar()
: Gtk::Statusbar()
{
    m_StatusBox.pack_start(m_AlbumCountLabel, Gtk::PACK_SHRINK);
    m_StatusBox.pack_start(m_TrackCountLabel, Gtk::PACK_SHRINK);
    
    pack_start(m_StatusBox); 
}

void StatusBar::setLibraryInfo(uint32_t albumCount, uint32_t trackCount)
{
    std::stringstream albums;
    albums << _("Albums: ") << albumCount;

    std::stringstream tracks;
    tracks << _("Tracks: ") << trackCount;
    
    m_AlbumCountLabel.set_text(albums.str());
    m_TrackCountLabel.set_text(tracks.str());
}

}
