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

#include "upnpalbumview.h"

#include <ostream>
#include <inttypes.h>
#include <glibmm/ustring.h>
#include <glibmm/i18n.h>

#include "Core/settings.h"
#include "utils/log.h"
#include "utils/trace.h"
#include "MusicLibrary/upnpmusiclibrary.h"
#include "MusicLibrary/upnplibrarysource.h"

using namespace std;

namespace Gejengel
{

UPnPAlbumView::UPnPAlbumView(Settings& settings, AlbumModel& albumModel, MusicLibrary& library)
: Gtk::VBox()
, m_Settings(settings)
, m_AlbumView(settings, albumModel, true)
, m_Library(dynamic_cast<UPnPMusicLibrary&>(library))
, m_TreeModel(Gtk::ListStore::create(m_Columns))
{
    utils::trace("Create UPnP Album view");

    m_ServerCombo.set_model(m_TreeModel);    
    loadServers();

    pack_start(m_ServerCombo, false, true);
    pack_start(m_AlbumView, true, true);

    loadSettings();

    m_AlbumView.signalAlbumChanged.connect(sigc::mem_fun(signalAlbumChanged, &sigc::signal<void, uint32_t>::emit));
    m_AlbumView.signalAlbumQueued.connect(sigc::mem_fun(signalAlbumQueued, &sigc::signal<void, uint32_t, int32_t>::emit));

    show_all_children();
}


UPnPAlbumView::~UPnPAlbumView()
{
    saveSettings();
}

void UPnPAlbumView::loadSettings()
{
    std::string server = m_Settings.get("UPnPServer");

    Gtk::TreeModel::Children children = m_TreeModel->children();
    for (Gtk::TreeModel::Children::iterator iter = children.begin(); iter != children.end(); ++iter)
    {
        Gtk::TreeModel::Row row = *iter;
        if (row[m_Columns.m_UDN] == server)
        {
            m_ServerCombo.set_active(iter);

            UPnPDevice dev;
            dev.m_UDN = server;
            m_Library.setSource(UPnPLibrarySource(dev));
            break;
        }
    }
}

void UPnPAlbumView::saveSettings()
{
    Gtk::TreeModel::iterator iter = m_ServerCombo.get_active();
    if (iter)
    {
        Gtk::TreeModel::Row row = *iter;
        if (row)
        {
            Glib::ustring udn = row[m_Columns.m_UDN];
            m_Settings.set("UPnPServer", udn);
        }
    }
}

void UPnPAlbumView::loadServers()
{
    std::list<UPnPDevice> servers;
    m_Library.getServers(servers);

    m_TreeModel->clear();  

    for (std::list<UPnPDevice>::iterator iter = servers.begin(); iter != servers.end(); ++iter)
    {
        Gtk::TreeModel::Row row = *(m_TreeModel->append());
        row[m_Columns.m_UDN]            = iter->m_UDN;
        row[m_Columns.m_FriendlyName]   = iter->m_FriendlyName;
        //TODO: get that server icon
    }
}

}
