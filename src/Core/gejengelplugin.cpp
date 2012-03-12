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

#include "gejengelplugin.h"

#include <gtkmm/icontheme.h>

#include "settings.h"

namespace Gejengel
{

GejengelPlugin::GejengelPlugin()
: m_HasSettingsDialog(false)
{
}

GejengelPlugin::~GejengelPlugin()
{
}

std::string GejengelPlugin::getDescription() const
{
    return "override getDescription() !";
}

Glib::RefPtr<Gdk::Pixbuf> GejengelPlugin::getIcon() const
{
    Glib::RefPtr<Gtk::IconTheme> theme = Gtk::IconTheme::get_default();
    return theme->load_icon("emblem-package", 32, (Gtk::IconLookupFlags) 0);
}

bool GejengelPlugin::hasSettingsDialog() const
{
    return m_HasSettingsDialog;
}

void GejengelPlugin::showSettingsDialog(Settings&)
{
}

}

