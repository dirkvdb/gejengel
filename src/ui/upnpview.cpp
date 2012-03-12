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

#include "upnpview.h"

#include "addupnpserverdlg.h"
#include "utils/log.h"
#include "Core/gejengel.h"
#include "Core/libraryaccess.h"
#include "Core/upnpserversettings.h"

#include <cassert>
#include <glibmm/i18n.h>

using namespace std;

namespace Gejengel
{

UPnPView::UPnPView(IGejengelCore& core)
: m_ServerSettings(core.getUPnPServerSettings())
, m_Library(core.getLibraryAccess())
, m_Layout(false, 5)
, m_ButtonsBox(false, 5)
, m_AddButton(Gtk::Stock::ADD)
, m_DeleteButton(Gtk::Stock::DELETE)
, m_pDialog(nullptr)
{
    set_border_width(5);
    set_shadow_type(Gtk::SHADOW_NONE);
    
    m_TreeModel = Gtk::ListStore::create(m_Columns);
    m_TreeView.set_model(m_TreeModel);
    
    Gtk::TreeView::Column* pColumn = Gtk::manage(new Gtk::TreeView::Column("Server"));
    pColumn->pack_start(m_Columns.serverIcon, false); //false = don't expand.
    pColumn->pack_start(m_Columns.serverName);

    m_TreeView.append_column(*pColumn);
    m_TreeView.set_headers_visible(false);
    
    m_TreeViewFrame.set_shadow_type(Gtk::SHADOW_IN);
    m_TreeViewFrame.add(m_TreeView);
    
    m_Layout.pack_start(m_TreeViewFrame, true, true);
    m_Layout.pack_start(m_ButtonsBox, Gtk::PACK_SHRINK);
    m_ButtonsBox.pack_start(m_AddButton, Gtk::PACK_SHRINK);
    m_ButtonsBox.pack_start(m_DeleteButton, Gtk::PACK_SHRINK);
    
    m_DeleteButton.set_sensitive(false);
    
    m_TreeView.get_selection()->signal_changed().connect(sigc::mem_fun(*this, &UPnPView::onSelectionChanged));
    m_AddButton.signal_clicked().connect(sigc::mem_fun(*this, &UPnPView::onAddServer));
    m_DeleteButton.signal_clicked().connect(sigc::mem_fun(*this, &UPnPView::onDeleteServer));
    
    add(m_Layout);
    
    loadServers();
}

UPnPView::~UPnPView()
{
    saveServers();
}

void UPnPView::loadServers()
{
    std::vector<upnp::Device> servers = m_ServerSettings.getServers();
    for (auto server : servers)
    {
        Gtk::TreeModel::Row row     = *(m_TreeModel->append());
        row[m_Columns.serverName]   = server.m_UserDefinedName;
        row[m_Columns.server]       = server;
    }
}

void UPnPView::saveServers()
{
    std::vector<upnp::Device> servers;
    
    Gtk::TreeModel::Children rows = m_TreeModel->children();
    for (Gtk::TreeModel::iterator iter = rows.begin(); iter != rows.end(); ++iter)
    {
        servers.push_back((*iter)[m_Columns.server]);
    }
    
    m_ServerSettings.setServers(servers);
    m_ServerSettings.saveToFile();
}

void UPnPView::onSelectionChanged()
{
    m_DeleteButton.set_sensitive(m_TreeView.get_selection()->count_selected_rows() > 0);
}

void UPnPView::onAddServer()
{
	AddUPnPServerDlg dlg(m_Library.getLibrary());
	dlg.signal_response().connect(sigc::mem_fun(*this, &UPnPView::onAddDialogClosed));
	m_pDialog = &dlg;
	dlg.run();
}

void UPnPView::onDeleteServer()
{
    assert(m_TreeView.get_selection()->count_selected_rows() == 1);
    m_TreeModel->erase(m_TreeView.get_selection()->get_selected());
}

void UPnPView::onAddDialogClosed(int response)
{
    if (response == Gtk::RESPONSE_OK)
    {
        upnp::Device server = m_pDialog->getSelectedServerContainer();
        Gtk::TreeModel::Row row     = *(m_TreeModel->append());
        row[m_Columns.serverName]   = server.m_UserDefinedName;
        row[m_Columns.server]       = server;
    }
    
    m_pDialog = nullptr;
}

}
