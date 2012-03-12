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

#ifndef UPNP_ALBUM_VIEW_H
#define UPNP_ALBUM_VIEW_H

#include <gtkmm.h>
#include <list>

#include "signals.h"
#include "detailedalbumview.h"

namespace Gejengel
{

class Settings;
class AlbumModel;
class MusicLibrary;
class UPnPMusicLibrary;

class UPnPAlbumView : public Gtk::VBox
{
public:
    UPnPAlbumView(Settings& settings, AlbumModel& albumModel, MusicLibrary& library);
    ~UPnPAlbumView();

    SignalAlbumChanged signalAlbumChanged;
    SignalAlbumQueued signalAlbumQueued;

private:
    //Tree model columns:
    class ModelColumns : public Gtk::TreeModel::ColumnRecord
    {
    public:

        ModelColumns()
        { add(m_FriendlyName); add(m_ServerIcon); add(m_UDN); }

        Gtk::TreeModelColumn<Glib::ustring>                 m_FriendlyName;
        Gtk::TreeModelColumn<Glib::RefPtr<Gdk::Pixbuf> >    m_ServerIcon;
        Gtk::TreeModelColumn<Glib::ustring>                 m_UDN;
    };
  
    void loadSettings();
    void saveSettings();

    void loadServers();
    
    Settings&                       m_Settings;
    Gtk::ComboBoxText               m_ServerCombo;
    DetailedAlbumView               m_AlbumView;
    UPnPMusicLibrary&               m_Library;
    ModelColumns                    m_Columns;
    Glib::RefPtr<Gtk::ListStore>    m_TreeModel;
};

}

#endif
