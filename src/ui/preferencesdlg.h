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

#ifndef PREFERENCES_DLG_H
#define PREFERENCES_DLG_H

#include <gtkmm.h>

#include "pluginview.h"
#include "config.h"

#ifdef HAVE_LIBUPNP
#include "upnpview.h"
#endif

namespace Gejengel
{

class IGejengelCore;
class PluginManager;

class PreferencesDlg : public Gtk::Dialog
{
public:
    PreferencesDlg(Gtk::Window& parent, IGejengelCore& core);
    ~PreferencesDlg();

private:
    void init();
    void loadPrefs();
    void storePrefs();
    void onPluginToggled(const Glib::ustring& path);
    void onPluginPreferences(const Glib::ustring& path);
    void onRescanClicked();
    void onTrayToggle();

    Gtk::Table              m_GeneralLayout;
    Gtk::Table              m_PluginsLayout;
    Gtk::Label              m_LibraryLabel;
    Gtk::FileChooserButton  m_LibraryChooser;
    Gtk::Label              m_AudioBackendLabel;
    Gtk::ComboBoxText       m_AudioBackenCombo;
    Gtk::Label              m_AlbumArtLabel;
    Gtk::Entry              m_AlbumArtEntry;
    Gtk::CheckButton        m_ScanAtStartupCheckbox;
    Gtk::CheckButton        m_SaveQueueCheckbox;
    Gtk::Notebook           m_Notebook;
    Gtk::Button             m_RescanButton;
    Gtk::CheckButton        m_TrayIconCheckbox;
    Gtk::CheckButton        m_MinToTrayCheckbox;

    PluginView              m_PluginView;
#ifdef HAVE_LIBUPNP
    UPnPView                m_UPnPView;
#endif

    IGejengelCore&          m_Core;
    PluginManager&          m_PluginMgr;
};

}

#endif
