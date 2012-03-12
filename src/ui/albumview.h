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

#ifndef ALBUM_VIEW_H
#define ALBUM_VIEW_H

#include <gtkmm.h>

#include "signals.h"

namespace Gejengel
{

class Settings;
class AlbumModel;

class AlbumView : public Gtk::ScrolledWindow
{
public:
    AlbumView(Settings& settings, AlbumModel& albumModel);
    ~AlbumView();

    SignalAlbumChanged signalAlbumChanged;
    SignalAlbumQueued signalAlbumQueued;

private:
    void loadSettings();
    void saveSettings();
    
    void onSelectionChanged();
    void onRowActivated(const Gtk::TreeModel::Path& path, Gtk::TreeViewColumn* column);

    Settings&       m_Settings;
    AlbumModel&     m_AlbumModel;
    Gtk::TreeView   m_TreeView;
};

}

#endif
