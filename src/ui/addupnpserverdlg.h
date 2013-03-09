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

#ifndef ADD_UPNPSERVER_DLG_H
#define ADD_UPNPSERVER_DLG_H

#include <gtkmm.h>
#include <vector>
#include <map>
#include <thread>

#include "pluginview.h"
#include "librarychangedispatcher.h"
#include "upnp/upnpdevice.h"
#include "upnp/upnpitem.h"
#include "upnp/upnpmediaserver.h"
#include "upnp/upnpdevicescanner.h"
#include "MusicLibrary/subscribers.h"

namespace upnp
{
    class Client;
}

namespace Gejengel
{

class MusicLibrary;
class UPnPMusicLibrary;

class AddUPnPServerDlg  : public Gtk::Dialog
                        , public utils::ISubscriber<const upnp::Item&>
{
public:
    AddUPnPServerDlg(MusicLibrary& library);
    ~AddUPnPServerDlg();

    std::shared_ptr<upnp::Device> getSelectedServerContainer();
    void onUPnPItem(const upnp::Item& container);
    void onItem(const upnp::Item& container, void* pData = nullptr);
    void onUPnPDeviceDiscovered(const std::shared_ptr<upnp::Device>& device);
    void onUPnPDeviceDissapeared(const std::shared_ptr<upnp::Device>& device);

private:
    class ModelColumns : public Gtk::TreeModel::ColumnRecord
    {
    public:
        ModelColumns()
        { add(name); add(icon); add(id); add(deviceId); add(childCount); }

        Gtk::TreeModelColumn<Glib::ustring>                 name;
        Gtk::TreeModelColumn<Glib::RefPtr<Gdk::Pixbuf> >    icon;
        Gtk::TreeModelColumn<std::string>                   id;
        Gtk::TreeModelColumn<std::string>                   deviceId;
        Gtk::TreeModelColumn<uint32_t>                      childCount;
    };

    void init();
    void onContainerExpanded(const Gtk::TreeModel::iterator& iter, const Gtk::TreeModel::Path& path);
    void onCheckAddSensitivity();
    bool onCustomServerName(GdkEventKey* pEvent);
    bool addItem(Gtk::TreeModel::iterator& currentLevel, const upnp::Item& container, const std::string& deviceId);
    void fetchDeviceTreeThread(std::shared_ptr<upnp::MediaServer> server);

    ModelColumns                            m_Columns;
    Gtk::ScrolledWindow                     m_ScrolledWindow;
    Gtk::Frame                              m_TreeViewFrame;
    Gtk::TreeView                           m_TreeView;
    Glib::RefPtr<Gtk::TreeStore>            m_TreeModel;
    Gtk::Label                              m_SelectContainerLabel;
    Gtk::HBox                               m_NameLayout;
    Gtk::Label                              m_EnterNameLabel;
    Gtk::Entry                              m_ServerNameEntry;
    bool                                    m_CustomServerName;
    bool                                    m_Destroy;
    
    UPnPMusicLibrary*                       m_pLibrary;
    upnp::Client*                           m_pClient;
    upnp::DeviceScanner                     m_DeviceScanner;
    bool                                    m_IOwnControlPoint;
    
    UIDispatcher<upnp::Item>                m_ContainerDispatcher;
    std::vector<std::thread>                m_BrowseThreads;
    
    SignalUIDispatcher<std::shared_ptr<upnp::Device>> m_DeviceAddedDispatcher;
    SignalUIDispatcher<std::shared_ptr<upnp::Device>> m_DeviceRemovedDispatcher;
    
    std::map<std::string, std::shared_ptr<upnp::MediaServer>>   m_ServerMap;
};

}

#endif
