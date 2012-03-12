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

#include "preferencesdlg.h"

#include <cassert>
#include <sstream>
#include <glibmm/i18n.h>

#include "utils/log.h"
#include "utils/numericoperations.h"
#include "Core/pluginmanager.h"
#include "Core/settings.h"
#include "Core/gejengel.h"
#include "Core/albumartprovider.h"

#ifdef WIN32
#include "winconfig.h"
#endif

using namespace Gtk;
using namespace std;

namespace Gejengel
{

PreferencesDlg::PreferencesDlg(Gtk::Window& parent, IGejengelCore& core)
: Gtk::Dialog(_("Preferences"), parent, true)
, m_GeneralLayout(11, 3, false)
, m_PluginsLayout(4, 2, false)
, m_LibraryLabel(_("Library location:"), ALIGN_LEFT)
, m_LibraryChooser(_("Select library location"), FILE_CHOOSER_ACTION_SELECT_FOLDER)
, m_AudioBackendLabel(_("Audio Backend:"), ALIGN_LEFT)
, m_AlbumArtLabel(_("Album art filenames ( seperate by ; )"), ALIGN_LEFT)
, m_ScanAtStartupCheckbox(_("Scan library at startup"))
, m_SaveQueueCheckbox(_("Save play queue on exit"))
, m_RescanButton()
, m_TrayIconCheckbox(_("Show tray icon"))
, m_MinToTrayCheckbox(_("Close to tray"))
, m_PluginView(core.getPluginManager(), core.getSettings())
#ifdef HAVE_LIBUPNP
, m_UPnPView(core)
#endif
, m_Core(core)
, m_PluginMgr(core.getPluginManager())
{
    set_title(string(PACKAGE_NAME) + _(" Preferences"));
    set_size_request(400, 400);
    set_resizable(true);

    init();
    loadPrefs();

    show_all_children();
}

PreferencesDlg::~PreferencesDlg()
{
    storePrefs();
}

void PreferencesDlg::init()
{
#ifdef HAVE_ALSA
    m_AudioBackenCombo.append_text("Alsa");
#endif
#ifdef HAVE_OPENAL
    m_AudioBackenCombo.append_text("OpenAL");
#endif
#ifdef HAVE_PULSE
    m_AudioBackenCombo.append_text("PulseAudio");
#endif

    m_GeneralLayout.attach(m_LibraryLabel,                  0, 1,  0,  1, FILL, FILL);
    m_GeneralLayout.attach(m_LibraryChooser,                1, 2,  0,  1, FILL | EXPAND, FILL);
    m_GeneralLayout.attach(m_RescanButton,                  2, 3,  0,  1, SHRINK, FILL);
    m_GeneralLayout.attach(m_ScanAtStartupCheckbox,         0, 3,  1,  2, FILL | EXPAND, FILL);
    m_GeneralLayout.attach(m_SaveQueueCheckbox,             0, 3,  2,  3, FILL | EXPAND, FILL);
    m_GeneralLayout.attach(*Gtk::manage(new HSeparator()),  0, 3,  3,  4, FILL | EXPAND, FILL);
    m_GeneralLayout.attach(m_AudioBackendLabel,             0, 1,  4,  5, FILL, FILL);
    m_GeneralLayout.attach(m_AudioBackenCombo,              1, 3,  4,  5, FILL | EXPAND, FILL);
    m_GeneralLayout.attach(*Gtk::manage(new HSeparator()),  0, 3,  5,  6, FILL | EXPAND, FILL);
    m_GeneralLayout.attach(m_AlbumArtLabel,                 0, 3,  6,  7, FILL | EXPAND, FILL);
    m_GeneralLayout.attach(m_AlbumArtEntry,                 0, 3,  7,  8, FILL | EXPAND, FILL);
    m_GeneralLayout.attach(*Gtk::manage(new HSeparator()),  0, 3,  8,  9, FILL | EXPAND, FILL);
    m_GeneralLayout.attach(m_TrayIconCheckbox,              0, 3,  9, 10, FILL | EXPAND, FILL);
    m_GeneralLayout.attach(m_MinToTrayCheckbox,             0, 3, 10, 11, FILL | EXPAND, FILL);

    m_GeneralLayout.set_border_width(5);
    m_GeneralLayout.set_col_spacings(10);
    m_GeneralLayout.set_row_spacings(2);
    m_GeneralLayout.set_row_spacing(2, 10);
    m_GeneralLayout.set_row_spacing(3, 10);
    m_GeneralLayout.set_row_spacing(4, 10);
    m_GeneralLayout.set_row_spacing(5, 10);
    m_GeneralLayout.set_row_spacing(7, 10);
    m_GeneralLayout.set_row_spacing(8, 10);

    m_Notebook.append_page(m_GeneralLayout, _("General"));
    m_Notebook.append_page(m_PluginView, _("Plugins"));
#ifdef HAVE_LIBUPNP
    m_Notebook.append_page(m_UPnPView, _("UPnP"));
#endif
    m_Notebook.set_border_width(5);

    m_RescanButton.set_image(*Gtk::manage(new Image(Gtk::Stock::REFRESH, Gtk::ICON_SIZE_SMALL_TOOLBAR)));
    m_RescanButton.set_tooltip_text(_("Completely rescan the library, clearing the current information"));
    m_RescanButton.signal_clicked().connect(sigc::mem_fun(*this, &PreferencesDlg::onRescanClicked));
    
    m_TrayIconCheckbox.signal_toggled().connect(sigc::mem_fun(*this, &PreferencesDlg::onTrayToggle));

    get_vbox()->pack_start(m_Notebook, FILL | EXPAND, FILL | EXPAND);
    add_button(Stock::CLOSE, RESPONSE_CLOSE);
}

void PreferencesDlg::loadPrefs()
{
    Settings& settings = m_Core.getSettings();

    m_AudioBackenCombo.set_active_text(settings.get("AudioBackend"));
    m_LibraryChooser.set_current_folder(settings.get("MusicLibrary"));
    m_AlbumArtEntry.set_text(settings.get("AlbumArtFilenames", "cover.jpg;cover.png"));
    m_ScanAtStartupCheckbox.set_active(settings.getAsBool("ScanAtStartup", false));
    m_SaveQueueCheckbox.set_active(settings.getAsBool("SaveQueueOnExit", false));
    m_TrayIconCheckbox.set_active(settings.getAsBool("TrayIcon", true));
    m_MinToTrayCheckbox.set_active(settings.getAsBool("CloseToTray", true));
    onTrayToggle();
}

void PreferencesDlg::storePrefs()
{
    Settings& settings = m_Core.getSettings();

    settings.set("AudioBackend",        m_AudioBackenCombo.get_active_text());
    settings.set("MusicLibrary",        m_LibraryChooser.get_current_folder());
    settings.set("AlbumArtFilenames",   m_AlbumArtEntry.get_text());
    settings.set("ScanAtStartup",       m_ScanAtStartupCheckbox.get_active());
    settings.set("SaveQueueOnExit",     m_SaveQueueCheckbox.get_active());
    settings.set("TrayIcon",            m_TrayIconCheckbox.get_active());
    settings.set("CloseToTray",      m_MinToTrayCheckbox.get_active());

    m_PluginMgr.saveSettings();
}

void PreferencesDlg::onRescanClicked()
{
    //m_Core.getMusicLibrary().scan(true);
}

void PreferencesDlg::onTrayToggle()
{
    m_MinToTrayCheckbox.set_sensitive(m_TrayIconCheckbox.get_active());
}

}
