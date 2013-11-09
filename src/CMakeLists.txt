CMAKE_MINIMUM_REQUIRED(VERSION 2.8)

SET (CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS} -g -O0 -Wall -Werror -Wfatal-errors -Wno-deprecated-declarations")

FIND_PACKAGE(PkgConfig)
FIND_PACKAGE(ImageMagick COMPONENTS Magick++)

pkg_check_modules(GDKMM gdkmm-2.4 REQUIRED)
pkg_check_modules(GTKMM gtkmm-2.4 REQUIRED)
pkg_check_modules(SQLITE sqlite3 REQUIRED)
pkg_check_modules(TAGLIB taglib REQUIRED)
pkg_check_modules(XDGBASEDIR libxdg-basedir REQUIRED)

pkg_check_modules(LASTFMLIB liblastfmlib)
pkg_check_modules(LIBNOTIFY libnotify)
pkg_check_modules(DBUSGLIB dbus-glib-1)

IF (DBUSGLIB_FOUND)
    OPTION(HAVE_DBUS "DBus control support" ON)
ENDIF (DBUSGLIB_FOUND)

IF (LIBNOTIFY_FOUND)
    OPTION(HAVE_LIBNOTIFY "Notification support" ON)
ENDIF (LIBNOTIFY_FOUND)

IF (LASTFMLIB_FOUND)
    OPTION(HAVE_LASTFMLIB "Lastfm scrobbling support" ON)
ENDIF (LASTFMLIB_FOUND)

INCLUDE_DIRECTORIES(
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${GDKMM_INCLUDE_DIRS}
    ${GTKMM_INCLUDE_DIRS}
    ${SQLITE_INCLUDE_DIRS}
    ${TAGLIB_INCLUDE_DIRS}
    ${ImageMagick_Magick++_INCLUDE_DIR}
    ${AUDIO_INCLUDE_DIRS}
    ${IMAGE_INCLUDE_DIRS}
    ${UTILS_INCLUDE_DIRS}
    ${UPNPFRAMEWORK_INCLUDE_DIRS}
    ${DBUSGLIB_INCLUDE_DIRS}
    ${LASTFMLIB_INCLUDE_DIRS}
    ${LIBNOTIFY_INCLUDE_DIRS}
)

LINK_DIRECTORIES(
    ${CMAKE_CURRENT_BINARY_DIR}
    ${GDKMM_LIBRARY_DIRS}
    ${GTKMM_LIBRARY_DIRS}
    ${SQLITE_LIBRARY_DIRS}
    ${TAGLIB_LIBRARY_DIRS}
    ${DBUSGLIB_LIBRARY_DIRS}
    ${LASTFMLIB_LIBRARY_DIRS}    
)

SET(LINK_LIBS
    ${GDKMM_LIBRARIES}
    ${GTKMM_LIBRARIES}
    ${SQLITE_LIBRARIES}
    ${TAGLIB_LIBRARIES}
    ${ImageMagick_Magick++_LIBRARY}
    ${XDGBASEDIR_LIBRARIES}
    ${AUDIO_LIBRARIES}    
    ${UTILS_LIBRARIES}
    ${UPNPFRAMEWORK_LIBRARIES}
    ${IMAGE_LIBRARIES}
    ${DBUSGLIB_LIBRARIES}
    ${LASTFMLIB_LIBRARIES}
    ${LIBNOTIFY_LIBRARIES}
)

SET(CORE_SRC_LIST
    Core/albumartgrabber.cpp
    Core/gejengel.cpp
    Core/gejengelplugin.cpp
    Core/libraryaccess.cpp
    Core/playqueue.cpp
    Core/pluginmanager.cpp
    Core/settings.cpp
    Core/upnpserversettings.cpp
)

SET(MUSICLIBRARY_SRC_LIST
    MusicLibrary/album.cpp
    MusicLibrary/albumart.cpp
    MusicLibrary/filesystemmusiclibrary.cpp
    MusicLibrary/libraryitem.cpp
    MusicLibrary/musicdb.cpp
    MusicLibrary/musiclibrary.cpp
    MusicLibrary/musiclibraryfactory.cpp
    MusicLibrary/scanner.cpp
    MusicLibrary/track.cpp
)

IF (UPNPSUPPORT)
    SET(MUSICLIBRARY_SRC_LIST
        ${MUSICLIBRARY_SRC_LIST}
        MusicLibrary/upnpalbumfetcher.cpp
        MusicLibrary/upnpmusiclibrary.cpp
    )
ENDIF (UPNPSUPPORT)

SET(PLUGIN_SRC_LIST dummy.cpp)

IF(HAVE_DBUS)
    LIST(APPEND PLUGIN_SRC_LIST
        Plugins/dbusplugin.cpp
        Plugins/mprisplayer.cpp
        Plugins/mprisroot.cpp
        Plugins/mpristracklist.cpp
    )
ENDIF(HAVE_DBUS)

IF(HAVE_LASTFMLIB)
    LIST(APPEND PLUGIN_SRC_LIST Plugins/lastfmplugin.cpp)
ENDIF(HAVE_LASTFMLIB)

IF(HAVE_LIBNOTIFY)
    LIST(APPEND PLUGIN_SRC_LIST Plugins/notificationplugin.cpp)
ENDIF(HAVE_LIBNOTIFY)

SET(UI_SRC_LIST
    ui/albuminfoview.cpp
    ui/albummodel.cpp
    ui/albumview.cpp
    ui/basiclayout.cpp
    ui/cellrendereralbum.cpp
    ui/cellrendererhoverbutton.cpp
    ui/detailedalbumlayout.cpp
    ui/detailedalbumview.cpp
    ui/librarychangedispatcher.cpp
    ui/mainwindow.cpp
    ui/mouseawaretreeview.cpp
    ui/nowplayingview.cpp
    ui/playqueuemodel.cpp
    ui/playqueueview.cpp
    ui/pluginview.cpp
    ui/pluginwidget.cpp
    ui/preferencesdlg.cpp
    ui/searchbar.cpp
    ui/sharedfunctions.cpp
    ui/simplealbumlayout.cpp
    ui/statusbar.cpp
    ui/systemtray.cpp
    ui/trackmodel.cpp
    ui/trackview.cpp
    ui/uilayoutfactory.cpp
    ui/uilayoutfactory.h
)

IF (UPNPSUPPORT)
    LIST(APPEND UI_SRC_LIST
        ui/addupnpserverdlg.cpp
        #ui/upnpalbumview.cpp
        ui/upnpview.cpp
    )

ENDIF (UPNPSUPPORT)

ADD_EXECUTABLE(gejengel
    main.cpp
    ${CORE_SRC_LIST}
    ${MUSICLIBRARY_SRC_LIST}
    ${PLUGIN_SRC_LIST}
    ${UI_SRC_LIST}
)

TARGET_LINK_LIBRARIES(gejengel ${LINK_LIBS})
INSTALL(TARGETS gejengel RUNTIME DESTINATION bin)

