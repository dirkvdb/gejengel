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

#ifdef WIN32
#include "winconfig.h"
#endif

#include "mainwindow.h"

#include <cassert>
#include <sstream>
#include <glibmm/i18n.h>

#include "uilayout.h"
#include "preferencesdlg.h"
#include "Core/gejengelcore.h"
#include "Core/settings.h"
#include "MusicLibrary/musiclibrary.h"
#include "Core/pluginmanager.h"
#include "Core/libraryaccess.h"
#include "Core/upnpserversettings.h"
#include "utils/log.h"
#include "utils/trace.h"

#ifdef HAVE_LIBUPNP
#include "MusicLibrary/upnpmusiclibrary.h"
#include "MusicLibrary/upnplibrarysource.h"
#include "upnp/upnpdevice.h"
#endif

using namespace Gtk;
using namespace std;
using namespace utils;

namespace Gejengel
{

MainWindow::MainWindow(IGejengelCore& core)
: m_Core(core)
, m_AlbumModel(core.getAlbumArtProvider())
, m_PlayQueueModel(core.getPlayQueue(), core.getAlbumArtProvider())
, m_LayoutStyle(UILayoutFactory::None)
, m_CloseToTray(true)
, m_PlayQueueDispatcher(m_PlayQueueModel)
, m_AlbumDispatcher(m_AlbumModel)
, m_TrackDispatcher(m_TrackModel)
, m_LibraryLoaded(false)
, m_LibraryMenuMergeId(0)
{
    utils::trace("Create Mainwindow");
    set_title(PACKAGE_NAME);
    set_resizable(true);

    Glib::set_application_name(PACKAGE_NAME);
    //set_default_icon_name("cdrom");
    //Gtk::Invisible inv;
    //set_icon(inv.render_icon(Stock::CDROM, ICON_SIZE_DIALOG));

    m_Dispatcher.addSubscriber(*this);
    m_ScanDispatcher.addSubscriber(*this);
    m_Core.getPlayQueue().subscribe(m_PlayQueueDispatcher);

    utils::trace("Init ui");
    init();

    loadSettings();

    add(m_EntireLayout);

    utils::trace("Show children");
    show_all_children();

    if (m_Core.getSettings().getAsBool("ScanAtStartup", false))
    {
        scanLibrary(false);
    }

    signal_key_press_event().connect(sigc::mem_fun(*this, &MainWindow::onKeyPress), false);
    signal_window_state_event().connect(sigc::mem_fun(*this, &MainWindow::onWindowStateChanged));

    m_SearchBar.signalSearchChanged.connect(sigc::mem_fun(*this, &MainWindow::onSearchChanged));
    m_SearchBar.signalClose.connect(sigc::mem_fun(*this, &MainWindow::onSearchClose));

    utils::trace("Mainwindow created");
}

MainWindow::~MainWindow()
{
    saveSettings();
}

void MainWindow::loadSettings()
{
    int32_t width = m_Core.getSettings().getAsInt("MainWindowWidth", 650);
    int32_t height = m_Core.getSettings().getAsInt("MainWindowHeight", 480);
    UILayoutFactory::LayoutStyle style = static_cast<UILayoutFactory::LayoutStyle>(m_Core.getSettings().getAsInt("LayoutStyle", UILayoutFactory::DetailedAlbums));

    //toggling the radio action causes the right layout to be loaded, the simple layout needs to be disabled first as it is active by default
    switch(style)
    {
    case UILayoutFactory::SimpleAlbums:
        Glib::RefPtr<RadioAction>::cast_dynamic(m_UIManager->get_action("/MenuBar/ViewMenu/SimpleAlbum"))->set_active(false);
        Glib::RefPtr<RadioAction>::cast_dynamic(m_UIManager->get_action("/MenuBar/ViewMenu/SimpleAlbum"))->set_active(true);
        break;
    case UILayoutFactory::Basic:
        Glib::RefPtr<RadioAction>::cast_dynamic(m_UIManager->get_action("/MenuBar/ViewMenu/Basic"))->set_active(true);
        break;
    case UILayoutFactory::DetailedAlbums:
    default:
        Glib::RefPtr<RadioAction>::cast_dynamic(m_UIManager->get_action("/MenuBar/ViewMenu/DetailedAlbum"))->set_active(true);
    }

    LibraryType libType = m_Core.getLibraryAccess().getLibraryType();
    switch(libType)
    {
#ifdef HAVE_LIBUPNP
    case UPnP:
    {
        std::string upnpServer = m_Core.getSettings().get("UPnPServerName");
        Glib::RefPtr<RadioAction> action = Glib::RefPtr<RadioAction>::cast_dynamic(m_UIManager->get_action("/MenuBar/LibraryMenu/" + upnpServer));
        if (action)
        {
            action->set_active(true);
            // only break if the server was found, if it wasn't found we fall through to Local db
            break;
        }
        log::error("UPnP server from preferences (%s) was not found, falling back to local filesystem", upnpServer);
    }
#endif
    case Local:
    default:
        // no need to set the radio button this is default
        loadLocalLibrary();
    }

    m_CloseToTray = m_Core.getSettings().getAsBool("CloseToTray", true);

    set_default_size(width, height);
}

void MainWindow::saveSettings()
{
    m_Core.getSettings().set("MainWindowWidth", get_width());
    m_Core.getSettings().set("MainWindowHeight", get_height());
    m_Core.getSettings().set("LayoutStyle", m_LayoutStyle);
}

bool MainWindow::on_delete_event(GdkEventAny* pEvent)
{
	if (m_CloseToTray)
	{
		if (!get_skip_taskbar_hint())
		{
			set_skip_taskbar_hint(true);
			iconify();
		}
	}
	else
	{
		quit();
	}

    return m_CloseToTray;
}

void MainWindow::run()
{
    Main::run(*this);
}

void MainWindow::quit()
{
    Main::quit();
}

void MainWindow::pushStatusMessage(const Glib::ustring& message)
{
    m_StatusBar.pop();
    m_StatusBar.push(message);
}

void MainWindow::showPreferences()
{
    string libraryBefore = m_Core.getSettings().get("MusicLibrary");
    bool sysTrayBefore   = m_Core.getSettings().getAsBool("TrayIcon", true);

    {
        PreferencesDlg dlg(*this, m_Core);
        dlg.run();
    }

    string libraryAfter = m_Core.getSettings().get("MusicLibrary");
    bool sysTrayAfter   = m_Core.getSettings().getAsBool("TrayIcon", true);

    if (libraryBefore != libraryAfter)
    {
        log::info("Library location changed from %s to %s", libraryBefore, libraryAfter);
        scanLibrary(true);
    }

    if (sysTrayBefore != sysTrayAfter)
    {
        sysTrayAfter ? m_SystemTray.show() : m_SystemTray.hide();
    }

    m_CloseToTray = sysTrayAfter ? m_Core.getSettings().getAsBool("CloseToTray", true) : false;

    updateLibraryMenu();

    m_Core.getSettings().saveToFile();
}

void MainWindow::showAbout()
{
    Gtk::AboutDialog aboutDlg;
    aboutDlg.set_transient_for(*this);
    aboutDlg.set_name(PACKAGE_NAME);
    aboutDlg.set_version(PACKAGE_VERSION);
    aboutDlg.set_copyright("Dirk Vanden Boer");
    aboutDlg.set_license("GPLv2");
    aboutDlg.set_website("http://code.google.com/p/gejengel/");

    std::list<Glib::ustring> authors;
    authors.push_back("Dirk Vanden Boer");
    aboutDlg.set_authors(authors);

    std::list<Glib::ustring> artists;
    artists.push_back("gorvan.com (Last.fm logo)");
    aboutDlg.set_artists(artists);

    aboutDlg.set_translator_credits("Spanish: mati86dl");

    aboutDlg.run();
}

void MainWindow::onScanLibrary()
{
    scanLibrary(false);
}

void MainWindow::scanLibrary(bool freshStart)
{
    try
    {
        Glib::RefPtr<Action> scanAction = Glib::RefPtr<Action>::cast_dynamic(m_UIManager->get_action("/MenuBar/FileMenu/FileRescanLibrary"));
        scanAction->set_sensitive(false);

        pushStatusMessage(_("Starting library scan, obtaining directory information"));
        m_Core.getLibraryAccess().scan(freshStart, m_ScanDispatcher);
    }
    catch (logic_error& e)
    {
        pushStatusMessage(string(_("Scan library failed: ")) + e.what());
    }
}

void MainWindow::newTrack(const Track& track)
{
    pushStatusMessage(_("Added ") + track.artist + " - " + track.title);
}

void MainWindow::newAlbum(const Album& album)
{
    m_AlbumModel.addAlbum(album);
}

void MainWindow::deletedTrack(const std::string& id)
{
    m_TrackModel.deleteTrack(id);
}

void MainWindow::deletedAlbum(const std::string& id)
{
    m_AlbumModel.deleteAlbum(id);
}

void MainWindow::updatedAlbum(const Album& album)
{
    m_AlbumModel.updateAlbum(album);
}

void MainWindow::scanStart(uint32_t numTracks)
{
    m_TracksToScan = numTracks;
    m_ScanProgress.set_fraction(0.0);
    m_ScanProgress.set_text(_("Scanning library"));
    pushStatusMessage("");
    m_InfoLayout.pack_start(m_ScanProgress, Gtk::PACK_SHRINK);
    m_ScanProgress.show();
}

void MainWindow::scanUpdate(uint32_t scannedTracks)
{
    m_ScanProgress.set_fraction(static_cast<double>(scannedTracks) / m_TracksToScan);
}

void MainWindow::scanFinish()
{
	if (m_ScanProgress.is_visible())
	{
		m_InfoLayout.remove(m_ScanProgress);
	}
	
    pushStatusMessage(_("Library scan finished"));

    Glib::RefPtr<Action> scanAction = Glib::RefPtr<Action>::cast_dynamic(m_UIManager->get_action("/MenuBar/FileMenu/FileRescanLibrary"));
    scanAction->set_sensitive(true);
}

void MainWindow::scanFailed()
{
    m_InfoLayout.remove(m_ScanProgress);
    pushStatusMessage(_("Library scan failed"));

    Glib::RefPtr<Action> scanAction = Glib::RefPtr<Action>::cast_dynamic(m_UIManager->get_action("/MenuBar/FileMenu/FileRescanLibrary"));
    scanAction->set_sensitive(true);
}

void MainWindow::libraryCleared()
{
    m_AlbumModel.clear();
    m_TrackModel.clear();
    m_Core.getPlayQueue().clear();
}

void MainWindow::selectedAlbumChanged(const std::string& albumId)
{
    m_TrackModel.clear();
    m_TrackModel.setSelectedAlbumId(albumId);
    m_Core.getLibraryAccess().getTracksFromAlbumAsync(albumId, m_TrackDispatcher);
}

void MainWindow::showHideWindow()
{
    if (get_skip_taskbar_hint())
    {
        set_skip_taskbar_hint(false);
        present();
    }
    else
    {
        set_skip_taskbar_hint(true);
        iconify();
    }
}

void MainWindow::init()
{
    m_ActionGroup = Gtk::ActionGroup::create();
    m_LibraryActionGroup = Gtk::ActionGroup::create();

    m_ActionGroup->add(Action::create("FileMenu", _("_File")));
    m_ActionGroup->add(Action::create("FileRescanLibrary", Gtk::Stock::REFRESH, _("_Rescan library")), Gtk::AccelKey('r', Gdk::CONTROL_MASK), sigc::mem_fun(*this, &MainWindow::onScanLibrary));
    m_ActionGroup->add(Action::create("FileQuit", Gtk::Stock::QUIT), sigc::mem_fun(*this, &MainWindow::quit));

    m_ActionGroup->add(Action::create("EditMenu", _("_Edit")));
    m_ActionGroup->add(Action::create("EditPreferences", Gtk::Stock::PREFERENCES), Gtk::AccelKey('p', Gdk::CONTROL_MASK), sigc::mem_fun(*this, &MainWindow::showPreferences));

    m_ActionGroup->add(Action::create("ViewMenu", _("_View")));
    m_ActionGroup->add(ToggleAction::create("ViewFullscreen", Gtk::Stock::FULLSCREEN), Gtk::AccelKey("F11"), sigc::mem_fun(*this, &MainWindow::onFullscreen));

    Gtk::RadioButtonGroup layoutGroup;
    m_ActionGroup->add(RadioAction::create(layoutGroup, "SimpleAlbum", _("Simple album view")), Gtk::AccelKey("F5"), sigc::mem_fun(*this, &MainWindow::onLayoutChange));
    m_ActionGroup->add(RadioAction::create(layoutGroup, "DetailedAlbum", _("Detailed album view")), Gtk::AccelKey("F6"), sigc::mem_fun(*this, &MainWindow::onLayoutChange));
    m_ActionGroup->add(RadioAction::create(layoutGroup, "Basic", _("Basic view")), Gtk::AccelKey("F7"), sigc::mem_fun(*this, &MainWindow::onLayoutChange));

#ifdef HAVE_LIBUPNP
    Gtk::RadioButtonGroup libraryTypeGroup;
    m_ActionGroup->add(Action::create("LibraryMenu", _("_Library")));
    m_ActionGroup->add(RadioAction::create(libraryTypeGroup, "Local", _("Local filesystem")), sigc::mem_fun(*this, &MainWindow::onLocalLibraryActivated));
#endif
    
    m_ActionGroup->add(Action::create("ToolsMenu", _("_Tools")));
    m_ActionGroup->add(Action::create("ToolsSearch", Gtk::Stock::FIND), Gtk::AccelKey('f', Gdk::CONTROL_MASK), sigc::mem_fun(*this, &MainWindow::search));
    m_ActionGroup->add(Action::create("ToolsRandomTracks", _("Queue random tracks")), Gtk::AccelKey('t', Gdk::CONTROL_MASK), sigc::mem_fun(*this, &MainWindow::onQueueRandomTracks));
    m_ActionGroup->add(Action::create("ToolsRandomAlbum", _("Queue random album")), Gtk::AccelKey('b', Gdk::CONTROL_MASK), sigc::mem_fun(*this, &MainWindow::onQueueRandomAlbum));
    m_ActionGroup->add(Action::create("ToolsClearQueue", _("Clear play queue")), Gtk::AccelKey('c', Gdk::CONTROL_MASK | Gdk::SHIFT_MASK), sigc::mem_fun(m_Core.getPlayQueue(), &PlayQueue::clear));

    m_ActionGroup->add(Action::create("HelpMenu", _("_Help")));
    m_ActionGroup->add(Action::create("HelpAbout", Gtk::Stock::ABOUT), sigc::mem_fun(*this, &MainWindow::showAbout));

    m_UIManager = Gtk::UIManager::create();
    m_UIManager->insert_action_group(m_ActionGroup);
    m_UIManager->insert_action_group(m_LibraryActionGroup);
    add_accel_group(m_UIManager->get_accel_group());

    std::stringstream uiInfo;
    uiInfo <<
        "<ui>"
        "  <menubar name='MenuBar'>"
        "    <menu action='FileMenu'>"
        "      <menuitem action='FileRescanLibrary'/>"
        "      <separator/>"
        "      <menuitem action='FileQuit'/>"
        "    </menu>"
        "    <menu action='EditMenu'>"
        "      <menuitem action='EditPreferences'/>"
        "    </menu>"
#ifdef HAVE_LIBUPNP
        "    <menu action='LibraryMenu'/>"
#endif
        "    <menu action='ViewMenu'>"
        "      <menuitem action='ViewFullscreen'/>"
        "      <menuitem action='SimpleAlbum'/>"
        "      <menuitem action='DetailedAlbum'/>"
        "      <menuitem action='Basic'/>"
        "    </menu>"
        "    <menu action='ToolsMenu'>"
        "      <menuitem action='ToolsSearch'/>"
        "      <menuitem action='ToolsRandomTracks'/>"
        "      <menuitem action='ToolsRandomAlbum'/>"
        "      <menuitem action='ToolsClearQueue'/>"
        "    </menu>"
        "    <menu action='HelpMenu'>"
        "      <menuitem action='HelpAbout'/>"
        "    </menu>"
        "  </menubar>"
        "</ui>";

    try
    {
        m_UIManager->add_ui_from_string(uiInfo.str());
        updateLibraryMenu();
    }
    catch(const Glib::Error& ex)
    {
        log::error("Building menus failed: %s" + ex.what());
    }

    Gtk::Widget* pMenubar = m_UIManager->get_widget("/MenuBar");

    m_InfoLayout.set_border_width(5);
    m_InfoLayout.set_spacing(5);

    m_EntireLayout.pack_start(*pMenubar, PACK_SHRINK);
    m_EntireLayout.pack_start(m_InfoLayout, PACK_EXPAND_WIDGET);
    m_EntireLayout.pack_start(m_StatusBar, PACK_SHRINK);
}

void MainWindow::updateLibraryMenu()
{
#ifdef HAVE_LIBUPNP
    // remove all existing library locations
    Glib::ListHandle<Glib::RefPtr<Action> > actions = m_LibraryActionGroup->get_actions();
    for (Glib::ListHandle<Glib::RefPtr<Action> >::iterator iter = actions.begin(); iter != actions.end(); ++iter)
    {
        m_LibraryActionGroup->remove(*iter);
    }

    Gtk::RadioButtonGroup libraryTypeGroup;
    Glib::RefPtr<RadioAction> action = Gtk::RadioAction::create(libraryTypeGroup, "Local", _("Local filesystem"));
    m_LibraryActionGroup->add(action, sigc::mem_fun(*this, &MainWindow::onLocalLibraryActivated));

    UPnPServerSettings& serverSettings = m_Core.getUPnPServerSettings();
    auto servers = serverSettings.getServers();

    // add new library locations
    for (auto& dev : servers)
    {
        Glib::RefPtr<RadioAction> action = Gtk::RadioAction::create(libraryTypeGroup, dev->m_UserDefinedName, dev->m_UserDefinedName + " (UPnP)");
        m_LibraryActionGroup->add(action, sigc::bind<-1>(sigc::mem_fun(*this, &MainWindow::onUPnPServerActivated), dev));
    }

    if (m_LibraryMenuMergeId != 0)
    {
        m_UIManager->remove_ui(m_LibraryMenuMergeId);
    }

    std::stringstream ss;
    ss << "<ui><menubar name='MenuBar'>"
       << "<menu action='LibraryMenu'>"
       << "<menuitem action='Local'/>";

    for (auto& dev : servers)
    {
        ss << "<menuitem action='" << dev->m_UserDefinedName << "'/>";
    }

    ss << "</menu></menubar></ui>";

    try
    {
        m_LibraryMenuMergeId = m_UIManager->add_ui_from_string(ss.str());
    }
    catch (const Glib::Error& ex)
    {
        log::error("Failed to add library locations to the menubar: %s", ex.what());
    }
#endif
}

void MainWindow::onFullscreen()
{
    Glib::RefPtr<ToggleAction> toggle = Glib::RefPtr<ToggleAction>::cast_dynamic(m_UIManager->get_action("/MenuBar/ViewMenu/ViewFullscreen"));
    toggle->get_active() ? fullscreen() : unfullscreen();
}

void MainWindow::onQueueRandomTracks()
{
    m_Core.getPlayQueue().queueRandomTracks(10);
}

void MainWindow::onQueueRandomAlbum()
{
    m_Core.getPlayQueue().queueRandomAlbum();
}

void MainWindow::search()
{
	Glib::RefPtr<Action> searchAction = Glib::RefPtr<Action>::cast_dynamic(m_UIManager->get_action("/MenuBar/ToolsMenu/ToolsSearch"));
    searchAction->set_sensitive(false);
	
	m_InfoLayout.pack_end(m_SearchBar, Gtk::PACK_SHRINK);
	m_InfoLayout.show_all_children();
	
	m_SearchBar.setFocus();
}

void MainWindow::onSearchClose()
{
	Glib::RefPtr<Action> searchAction = Glib::RefPtr<Action>::cast_dynamic(m_UIManager->get_action("/MenuBar/ToolsMenu/ToolsSearch"));
    searchAction->set_sensitive(true);
    
    m_InfoLayout.remove(m_SearchBar);
}

void MainWindow::onSearchChanged(const Glib::ustring& searchString)
{
    log::debug("Search: %s", searchString);

    if (!m_LibraryLoaded)
    {
        //workaround for initial search changed event on load with empty string which causes library
        //to be loaded twice increasing startup time signficantly
        return;
    }

    try
    {
        m_AlbumModel.clear();
        m_TrackModel.clear();
        m_TrackModel.clearSelectedAlbumId();

        if (searchString.empty())
        {
            m_Core.getLibraryAccess().getAlbumsAsync(m_AlbumDispatcher);
            return;
        }

        if (searchString.size() > 2)
        {
            m_Core.getLibraryAccess().search(searchString, m_TrackModel, m_AlbumModel);
        }
    }
    catch (exception& e)
    {
        pushStatusMessage(string(_("Failed to perform search query: ")) + e.what());
    }
}

void MainWindow::onLayoutChange()
{
    if (Glib::RefPtr<RadioAction>::cast_dynamic(m_UIManager->get_action("/MenuBar/ViewMenu/SimpleAlbum"))->get_active())
    {
        if (m_LayoutStyle != UILayoutFactory::SimpleAlbums)
        {
            m_LayoutStyle = UILayoutFactory::SimpleAlbums;
        }
        else
        {
            return;
        }
    }
    else if (Glib::RefPtr<RadioAction>::cast_dynamic(m_UIManager->get_action("/MenuBar/ViewMenu/DetailedAlbum"))->get_active())
    {
        if (m_LayoutStyle != UILayoutFactory::DetailedAlbums)
        {
            m_LayoutStyle = UILayoutFactory::DetailedAlbums;
        }
        else
        {
            return;
        }
    }
    else if (Glib::RefPtr<RadioAction>::cast_dynamic(m_UIManager->get_action("/MenuBar/ViewMenu/Basic"))->get_active())
    {
        if (m_LayoutStyle != UILayoutFactory::Basic)
        {
            m_LayoutStyle = UILayoutFactory::Basic;
        }
        else
        {
            return;
        }
    }

    loadLayout();
}

void MainWindow::onLocalLibraryActivated()
{
    LibraryType currentType = m_Core.getLibraryAccess().getLibraryType();

    if (currentType != Local && Glib::RefPtr<RadioAction>::cast_dynamic(m_UIManager->get_action("/MenuBar/LibraryMenu/Local"))->get_active())
    {
        m_Core.getLibraryAccess().setLibraryType(Local);
        loadLocalLibrary();
    }
}

void MainWindow::loadLocalLibrary()
{
    try
    {
        log::debug("Load local db");
        m_TrackModel.clear();
        m_AlbumModel.clear();

        m_Core.getLibraryAccess().addLibrarySubscriber(m_Dispatcher);
        m_Core.getLibraryAccess().getAlbumsAsync(m_AlbumDispatcher);
        m_LibraryLoaded = true;
    }
    catch (logic_error& e)
    {
        pushStatusMessage(string(_("Failed to switch to local filesystem library: ")) + e.what());
    }
}

#ifdef HAVE_LIBUPNP
void MainWindow::onUPnPServerActivated(const std::shared_ptr<upnp::Device>& server)
{
	LibraryAccess& lib = m_Core.getLibraryAccess();
    LibraryType currentType = lib.getLibraryType();

    Glib::RefPtr<RadioAction> action = Glib::RefPtr<RadioAction>::cast_dynamic(m_UIManager->get_action("/MenuBar/LibraryMenu/" + server->m_UserDefinedName));
    if (action && action->get_active())
    {
        if (currentType != UPnP)
        {
            lib.setLibraryType(UPnP);
        }

        loadUPnPLibrary(server);
    }
}

void MainWindow::loadUPnPLibrary(const std::shared_ptr<upnp::Device>& server)
{
    try
    {
        log::debug("Load UPnP server: %s", server->m_UserDefinedName);

        m_Core.getSettings().set("UPnPServerName", server->m_UserDefinedName);

        m_TrackModel.clear();
        m_AlbumModel.clear();

        UPnPLibrarySource source(server);
        m_Core.getLibraryAccess().setSource(source);
        m_Core.getLibraryAccess().getAlbumsAsync(m_AlbumDispatcher);
        m_LibraryLoaded = true;
    }
    catch (logic_error& e)
    {
        pushStatusMessage(string(_("Failed to switch to UPnP server library: ")) + e.what());
    }
}
#endif

void MainWindow::loadLayout()
{
    if (m_pLayout)
    {
        m_InfoLayout.remove(m_pLayout->getWidget());
    }
    
    m_pLayout.reset(UILayoutFactory::create(m_LayoutStyle, m_Core, m_PlayQueueModel, m_TrackModel, m_AlbumModel));
    m_InfoLayout.pack_start(m_pLayout->getWidget(), PACK_EXPAND_WIDGET);
    show_all_children();

    vector<GejengelPlugin*> plugins;
    m_pLayout->getPlugins(plugins);
    plugins.push_back(&m_SystemTray);
    m_Core.getPluginManager().setUIPlugins(plugins);

    if (m_Core.getSettings().getAsBool("TrayIcon", true))
    {
        m_SystemTray.show();
    }
    else
    {
        m_SystemTray.hide();
    }

    m_pLayout->signalAlbumChanged.connect(sigc::mem_fun(*this, &MainWindow::selectedAlbumChanged));

    onSearchChanged("");
}

bool MainWindow::onKeyPress(GdkEventKey* pEvent)
{
    switch (pEvent->keyval)
    {
	case GDK_slash:
	case GDK_question:
	{
		Glib::RefPtr<Action> searchAction = Glib::RefPtr<Action>::cast_dynamic(m_UIManager->get_action("/MenuBar/ToolsMenu/ToolsSearch"));
		if (searchAction->get_sensitive())
		{
			search();
			return true;
		}
	}
    default:
        return false;
    }
    
    return false;
}

bool MainWindow::onWindowStateChanged(GdkEventWindowState* pState)
{
    return false;
}

void MainWindow::reportError(const std::string& msg)
{
    pushStatusMessage(msg);
}

void MainWindow::reportWarning(const std::string& msg)
{
    pushStatusMessage(msg);
}

void MainWindow::reportInfo(const std::string& msg)
{
    pushStatusMessage(msg);
}


}
