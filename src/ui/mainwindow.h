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

#ifndef MAIN_WINDOW_H
#define MAIN_WINDOW_H

#include <gtkmm.h>
#include <memory>

#include "trackmodel.h"
#include "albummodel.h"
#include "playqueuemodel.h"
#include "uilayoutfactory.h"
#include "systemtray.h"
#include "searchbar.h"
#include "Core/statusreporter.h"
#include "utils/types.h"
#include "librarychangedispatcher.h"

namespace upnp
{
    class Device;
}

namespace Gejengel
{

class Track;
class Album;
class IGejengelCore;
class GejengelPlugin;
class UILayout;
class IAlbumArtProvider;

class MainWindow    : public Gtk::Window
                    , public ILibrarySubscriber
                    , public IStatusReporter
                    , public IScanSubscriber
{
public:
    MainWindow(IGejengelCore& core);
    virtual ~MainWindow();

    void selectedAlbumChanged(const std::string& albumId);
    void showHideWindow();
    void run();
    void quit();
    
    /* StatusReporter interface */
    void reportError(const std::string& msg);
    void reportWarning(const std::string& msg);
    void reportInfo(const std::string& msg);

    /* LibrarySubscriber interface */
    void newTrack(const Track& track);
    void newAlbum(const Album& album);
    void deletedTrack(const std::string& id);
    void deletedAlbum(const std::string& id);
    void updatedAlbum(const Album& album);
    void scanStart(uint32_t numTracks);
    void scanUpdate(uint32_t scannedTracks);
    void scanFinish();
    void scanFailed();
    void libraryCleared();

protected:
    void saveSettings();
    void loadSettings();
    bool on_delete_event(GdkEventAny* pEvent);

    void init();

    void showPreferences();
    void showAbout();
    void scanLibrary(bool freshStart);
    void search();
    void onScanLibrary();
    void onFullscreen();
    void onQueueRandomTracks();
    void onQueueRandomAlbum();
    void onSearchChanged(const Glib::ustring& search);
    void onSearchClose();
    void onLayoutChange();
    bool onKeyPress(GdkEventKey* event);
    bool onWindowStateChanged(GdkEventWindowState* pState);
    void onLocalLibraryActivated();
    void onUPnPServerActivated(const std::shared_ptr<upnp::Device>& server);
    void loadLayout();
    void pushStatusMessage(const Glib::ustring& message);
    void loadLocalLibrary();
    void loadUPnPLibrary(const std::shared_ptr<upnp::Device>& server);
    void updateLibraryMenu();

    IGejengelCore&                      m_Core;
    Glib::RefPtr<Gtk::UIManager>        m_UIManager;
    Glib::RefPtr<Gtk::ActionGroup>      m_ActionGroup;
    Glib::RefPtr<Gtk::ActionGroup>      m_LibraryActionGroup;
    Gtk::VBox                           m_EntireLayout;
    Gtk::VBox                           m_InfoLayout;
    Gtk::Statusbar                      m_StatusBar;
    Gtk::ProgressBar                    m_ScanProgress;
    TrackModel                          m_TrackModel;
    AlbumModel                          m_AlbumModel;
    PlayQueueModel                      m_PlayQueueModel;
    UILayoutFactory::LayoutStyle        m_LayoutStyle;
    std::unique_ptr<UILayout>           m_pLayout;
    uint32_t                            m_TracksToScan;
    SystemTray                          m_SystemTray;
    SearchBar							m_SearchBar;
    bool                            	m_CloseToTray;
    LibraryChangeDispatcher             m_Dispatcher;
    ScanDispatcher                      m_ScanDispatcher;
    PlayQueueDispatcher                 m_PlayQueueDispatcher;
    UIDispatcher<Album>                 m_AlbumDispatcher;
    UIDispatcher<Track>                 m_TrackDispatcher;
    bool                                m_LibraryLoaded;
    Gtk::UIManager::ui_merge_id         m_LibraryMenuMergeId;
};

}

#endif
