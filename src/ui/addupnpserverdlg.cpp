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

#include "addupnpserverdlg.h"

#include <cassert>
#include <sstream>
#include <glibmm/i18n.h>

#include "utils/log.h"
#include "upnp/upnpcontrolpoint.h"
#include "MusicLibrary/upnpmusiclibrary.h"

static const long MAX_BROWSE_LEVEL = 3;

using namespace utils;
using namespace std::placeholders;

namespace Gejengel
{

AddUPnPServerDlg::AddUPnPServerDlg(MusicLibrary& library)
: Gtk::Dialog(_("Add UPnP server"), true)
, m_SelectContainerLabel(_("Select a upnp container containing albums"), Gtk::ALIGN_LEFT)
, m_EnterNameLabel(_("Server name: "), Gtk::ALIGN_LEFT)
, m_CustomServerName(false)
, m_Destroy(false)
, m_pLibrary(dynamic_cast<UPnPMusicLibrary*>(&library))
, m_pCtrlPoint(m_pLibrary ? (&m_pLibrary->getControlPoint()) : new upnp::ControlPoint())
, m_DeviceScanner(*m_pCtrlPoint, upnp::DeviceScanner::Servers)
, m_IOwnControlPoint(m_pLibrary == nullptr)
, m_ContainerDispatcher(*this)
{
    set_title(_("Add UPnP server"));
    set_size_request(400, 400);
    set_resizable(true);
    set_border_width(5);

    m_TreeModel = Gtk::TreeStore::create(m_Columns);
    m_TreeView.set_model(m_TreeModel);

    Gtk::TreeView::Column* pColumn = Gtk::manage(new Gtk::TreeView::Column("Server"));
    pColumn->pack_start(m_Columns.icon, false); //false = don't expand.
    pColumn->pack_start(m_Columns.name);

    m_TreeView.append_column(*pColumn);
    m_TreeView.set_headers_visible(false);

    m_ScrolledWindow.add(m_TreeView);
    m_ScrolledWindow.set_shadow_type(Gtk::SHADOW_IN);
    m_ScrolledWindow.set_policy(Gtk::POLICY_AUTOMATIC, Gtk::POLICY_AUTOMATIC);

    m_NameLayout.set_spacing(5);
    m_NameLayout.pack_start(m_EnterNameLabel, Gtk::PACK_SHRINK);
    m_NameLayout.pack_start(m_ServerNameEntry, Gtk::PACK_EXPAND_WIDGET);

    Gtk::VBox* pVBox = get_vbox();
    pVBox->set_spacing(5);
    pVBox->pack_start(m_SelectContainerLabel, Gtk::PACK_SHRINK);
    pVBox->pack_start(m_ScrolledWindow, Gtk::PACK_EXPAND_WIDGET);
    pVBox->pack_start(m_NameLayout, Gtk::PACK_SHRINK);

    add_button(Gtk::Stock::CANCEL, Gtk::RESPONSE_CANCEL);
    add_button(Gtk::Stock::ADD, Gtk::RESPONSE_OK);
    set_response_sensitive (Gtk::RESPONSE_OK, false);

    m_TreeView.get_selection()->signal_changed().connect(sigc::mem_fun(*this, &AddUPnPServerDlg::onCheckAddSensitivity));
    m_ServerNameEntry.signal_changed().connect(sigc::mem_fun(*this, &AddUPnPServerDlg::onCheckAddSensitivity));
    m_ServerNameEntry.signal_key_release_event().connect(sigc::mem_fun(*this, &AddUPnPServerDlg::onCustomServerName));

    show_all_children();

    m_DeviceAddedDispatcher.DispatchedItemEvent.connect(std::bind(&AddUPnPServerDlg::onUPnPDeviceDiscovered, this, _1), this);
    m_DeviceRemovedDispatcher.DispatchedItemEvent.connect(std::bind(&AddUPnPServerDlg::onUPnPDeviceDissapeared, this, _1), this);
    m_DeviceScanner.DeviceDiscoveredEvent.connect(std::bind(&SignalUIDispatcher<const upnp::Device&>::onItem, &m_DeviceAddedDispatcher, _1), &m_DeviceAddedDispatcher);
    m_DeviceScanner.DeviceDissapearedEvent.connect(std::bind(&SignalUIDispatcher<const upnp::Device&>::onItem, &m_DeviceRemovedDispatcher, _1), &m_DeviceRemovedDispatcher);

    m_DeviceScanner.start();
    m_DeviceScanner.refresh();
}

AddUPnPServerDlg::~AddUPnPServerDlg()
{
    log::debug("Delete server dialog");

    m_DeviceScanner.stop();

    for (auto& browser : m_BrowserMap)
    {
        browser.second->cancel();
    }

    m_Destroy = true;
    for (auto& thr : m_BrowseThreads)
    {
        if (thr.joinable())
        {
            log::debug("Wait for thread");
            thr.join();
            log::debug("Wait for thread done");
        }
    }
    
    if (m_IOwnControlPoint)
    {
        log::debug("delete ctrlpnt");
        delete m_pCtrlPoint;
        log::debug("ctrlpnt deleted");
    }
    else
    {
        //reset controlpoint to avoid receiving devices that would cause a crash
        log::debug("Reset Control point");
        m_pCtrlPoint->reset();
        log::debug("reset done");
    }
    log::debug("Delete server dialog done");
}

bool AddUPnPServerDlg::addItem(Gtk::TreeModel::iterator& currentLevel, const upnp::Item& container, const std::string& deviceId)
{
    const Gtk::TreeNodeChildren& children = (*currentLevel)->children();

    const std::string& id = (*currentLevel)[m_Columns.id];
    if (id == container.getParentId())
    {
        Gtk::TreeModel::Row childrow = *(m_TreeModel->append(children));

        childrow[m_Columns.name]        = container.getTitle();
        childrow[m_Columns.id]          = container.getObjectId();
        childrow[m_Columns.childCount]  = container.getChildCount();
        childrow[m_Columns.deviceId]    = deviceId;
        return true;
    }
    
    
    for (Gtk::TreeModel::iterator iter = children.begin(); iter != children.end(); ++iter)
    {
        if (addItem(iter, container, deviceId))
        {
            return true;
        }
    }

    return false;
}

void AddUPnPServerDlg::onItem(const upnp::Item& container, void* pData)
{
    const std::string* pCurrentDeviceId = reinterpret_cast<const std::string*>(pData);

    const Gtk::TreeNodeChildren& children = m_TreeModel->children();
    for (Gtk::TreeModel::iterator iter = children.begin(); iter != children.end(); ++iter)
    {
        const std::string& deviceId = (*iter)[m_Columns.deviceId];
        if (deviceId == *pCurrentDeviceId)
        {
            if (!addItem(iter, container, *pCurrentDeviceId))
            {
                log::error("Failed to add container to the tree:", container.getTitle());
            }
        }
    }
}

void AddUPnPServerDlg::onUPnPDeviceDiscovered(const upnp::Device& device)
{
    const Gtk::TreeNodeChildren& children = m_TreeModel->children();
    for (Gtk::TreeModel::iterator iter = children.begin(); iter != children.end(); ++iter)
    {
        const std::string& deviceId = (*iter)[m_Columns.deviceId];
        if (deviceId == device.m_UDN)
        {
            //ignore duplicate devices
            return;
        }
    }

    Gtk::TreeModel::iterator iter = m_TreeModel->append();

    const Gtk::TreeModel::Row& row = *iter;
    row[m_Columns.name]         = device.m_FriendlyName;
    row[m_Columns.id]           = upnp::Browser::rootId;
    row[m_Columns.deviceId]     = device.m_UDN;

    auto browser = std::make_shared<upnp::Browser>(*m_pCtrlPoint);
    browser->setDevice(device);
    m_BrowserMap[device.m_UDN] = browser;

    m_BrowseThreads.push_back(std::thread(&AddUPnPServerDlg::fetchDeviceTreeThread, this, browser));
}

void AddUPnPServerDlg::onUPnPDeviceDissapeared(const upnp::Device& device)
{
    const Gtk::TreeNodeChildren& children = m_TreeModel->children();
    for (Gtk::TreeModel::iterator iter = children.begin(); iter != children.end(); ++iter)
    {
        const std::string& deviceId = (*iter)[m_Columns.deviceId];
        if (deviceId == device.m_UDN)
        {
            m_TreeModel->erase(iter);
            break;
        }
    }
}

void AddUPnPServerDlg::onCheckAddSensitivity()
{
    if (m_TreeView.get_selection()->count_selected_rows() > 0)
    {
        Gtk::TreeModel::iterator iter = m_TreeView.get_selection()->get_selected();

        if (!m_CustomServerName)
        {
            const std::string& deviceId = (*iter)[m_Columns.deviceId];
            m_ServerNameEntry.set_text(m_BrowserMap[deviceId]->getDevice().m_FriendlyName);
        }

        if (!m_ServerNameEntry.get_text().empty())
        {
            set_response_sensitive (Gtk::RESPONSE_OK, true);
            return;
        }
    }

    set_response_sensitive(Gtk::RESPONSE_OK, false);
}

upnp::Device AddUPnPServerDlg::getSelectedServerContainer()
{
    assert(m_TreeView.get_selection()->count_selected_rows() == 1);
    Gtk::TreeModel::iterator iter = m_TreeView.get_selection()->get_selected();

    const std::string& udn = (*iter)[m_Columns.deviceId];
    upnp::Device device = m_BrowserMap[udn]->getDevice();
    device.m_UserDefinedName    = m_ServerNameEntry.get_text();
    device.m_ContainerId        = (*iter)[m_Columns.id];

    return device;
}

bool AddUPnPServerDlg::onCustomServerName(GdkEventKey* pEvent)
{
    m_CustomServerName = true;
    return false;
}

void AddUPnPServerDlg::fetchDeviceTreeThread(std::shared_ptr<upnp::Browser> browser)
{
    class ContainerSubscriber : public utils::ISubscriber<upnp::Item>
    {
    public:
        ContainerSubscriber(UIDispatcher<upnp::Item>& dispatcher, const std::string& deviceId)
        : m_Dispatcher(dispatcher)
        , m_DeviceId(deviceId)
        {}

        void onItem(const upnp::Item& container, void* pData)
        {
            long level = reinterpret_cast<long>(pData);
            assert(level < MAX_BROWSE_LEVEL);
            m_ChildNodes[level].push_back(container);
            m_Dispatcher.onItem(container, &const_cast<std::string&>(m_DeviceId));
        }

        UIDispatcher<upnp::Item>&    m_Dispatcher;
        std::vector<upnp::Item>      m_ChildNodes[MAX_BROWSE_LEVEL];
        const std::string&           m_DeviceId;
    };

    ContainerSubscriber subscriber(m_ContainerDispatcher, browser->getDevice().m_UDN);
    long startLevel = 0;
    upnp::Item rootContainer(upnp::Browser::rootId);
    browser->getContainers(subscriber, rootContainer, 0, 0, reinterpret_cast<void*>(startLevel));
    
    for (long level = 1; level < MAX_BROWSE_LEVEL && !m_Destroy; ++level)
    {
        long parentLevel = level - 1;
        for (size_t i = 0; i < subscriber.m_ChildNodes[parentLevel].size() && !m_Destroy; ++i)
        {
            upnp::Item container(subscriber.m_ChildNodes[parentLevel][i].getObjectId());
            container.setTitle(subscriber.m_ChildNodes[parentLevel][i].getTitle());
            browser->getContainers(subscriber, container, 0, 0, reinterpret_cast<void*>(level));
        }
    }
}

}