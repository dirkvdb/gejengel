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

#ifndef UPNP_VIEW_H
#define UPNP_VIEW_H

#include <gtkmm.h>
#include "signals.h"
#include "upnp/upnpdevice.h"

namespace Gejengel
{

class Settings;
class LibraryAccess;
class UPnPServerSettings;
class AddUPnPServerDlg;
class IGejengelCore;

class UPnPView : public Gtk::Frame
{
public:
    UPnPView(IGejengelCore& core);
    ~UPnPView();

private:
    class ModelColumns : public Gtk::TreeModel::ColumnRecord
    {
    public:
        ModelColumns()
        { add(serverName); add(serverIcon); add(server); }

        Gtk::TreeModelColumn<Glib::ustring>                 serverName;
        Gtk::TreeModelColumn<Glib::RefPtr<Gdk::Pixbuf> >    serverIcon;
        Gtk::TreeModelColumn<upnp::Device>                  server;
    };
    
    void loadServers();
    void saveServers();
    void onSelectionChanged();
    void onAddServer();
    void onDeleteServer();
    void onAddDialogClosed(int response);

    UPnPServerSettings&             m_ServerSettings;
    LibraryAccess&                  m_Library;
    ModelColumns                    m_Columns;
    Gtk::TreeView                   m_TreeView;
    Glib::RefPtr<Gtk::ListStore>    m_TreeModel;
    Gtk::Frame                      m_TreeViewFrame;
    Gtk::HBox                       m_Layout;
    Gtk::VBox                       m_ButtonsBox;
    Gtk::Button                     m_AddButton;
    Gtk::Button                     m_DeleteButton;
    AddUPnPServerDlg*               m_pDialog;
};

}

#endif
