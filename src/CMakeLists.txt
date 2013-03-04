CMAKE_MINIMUM_REQUIRED(VERSION 2.8)

SET (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++0x")
SET (CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS} -g -O0 -Wall -Werror -Wfatal-errors -Wno-deprecated-declarations")

FIND_PACKAGE(PkgConfig)
pkg_check_modules(GDKMM gdkmm-2.4 REQUIRED)
pkg_check_modules(GTKMM gtkmm-2.4 REQUIRED)
pkg_check_modules(SQLITE sqlite3 REQUIRED)
pkg_check_modules(TAGLIB taglib REQUIRED)
pkg_check_modules(MAGICK ImageMagick++ REQUIRED)
pkg_check_modules(XDGBASEDIR libxdg-basedir REQUIRED)

pkg_check_modules(FLAC flac++)
pkg_check_modules(MAD mad)
pkg_check_modules(PULSEAUDIO libpulse)
pkg_check_modules(LASTFMLIB liblastfmlib)
pkg_check_modules(LIBNOTIFY libnotify)
pkg_check_modules(DBUSGLIB dbus-glib-1)
pkg_check_modules(AVCODEC libavcodec)
pkg_check_modules(AVFORMAT libavformat)
pkg_check_modules(AVUTIL libavutil)

FIND_PACKAGE(ALSA)
IF (ALSA_FOUND)
    OPTION(HAVE_ALSA "Alsa audio output support" ON)
ENDIF (ALSA_FOUND)

IF (PULSEAUDIO_FOUND)
    OPTION(HAVE_PULSE "Pulse audio output support" ON)
ENDIF (PULSEAUDIO_FOUND)

FIND_PACKAGE(OpenAL)
IF (OPENAL_FOUND)
    OPTION(HAVE_OPENAL "OpenAL audio output support" ON)
ENDIF (OPENAL_FOUND)

IF (AVCODEC_FOUND AND AVFORMAT_FOUND AND AVUTIL_FOUND)
    OPTION(HAVE_FFMPEG "FFmpeg audio decoder support" ON)
    SET(FFMPEG_INCLUDE_DIRS ${AVCODEC_INCLUDE_DIRS} ${AVFORMAT_INCLUDE_DIRS} ${AVUTIL_INCLUDE_DIRS})
    SET(FFMPEG_LIBRARY_DIRS ${AVCODEC_LIBRARY_DIRS} ${AVFORMAT_LIBRARY_DIRS} ${AVUTIL_LIBRARY_DIRS})
    SET(FFMPEG_LIBRARIES ${AVCODEC_LIBRARIES} ${AVFORMAT_LIBRARIES} ${AVUTIL_LIBRARIES})
ENDIF (AVCODEC_FOUND AND AVFORMAT_FOUND AND AVUTIL_FOUND)

IF (FLAC_FOUND)
    OPTION(HAVE_FLAC "Flac audio decoder support" ON)
ENDIF (FLAC_FOUND)

IF (MAD_FOUND)
    OPTION(HAVE_MAD "Mad mp3 audio decoder support" ON)
ENDIF (MAD_FOUND)

IF (UPNP_FOUND)
    OPTION(HAVE_LIBUPNP "UPnP library support" ON)    
ENDIF (UPNP_FOUND)

IF (DBUSGLIB_FOUND)
    OPTION(HAVE_DBUS "DBus control support" ON)
ENDIF (DBUSGLIB_FOUND)

IF (LIBNOTIFY_FOUND)
    OPTION(HAVE_LIBNOTIFY "Notification support" ON)
ENDIF (LIBNOTIFY_FOUND)

IF (LASTFMLIB_FOUND)
    OPTION(HAVE_LASTFMLIB "Lastfm scrobbling support" ON)
ENDIF (LASTFMLIB_FOUND)

IF (XDGBASEDIR_FOUND)
    OPTION(HAVE_XDGBASEDIR "xdg-basedir support" ON)
ENDIF (LASTFMLIB_FOUND)

INCLUDE_DIRECTORIES(
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${GDKMM_INCLUDE_DIRS}
    ${GTKMM_INCLUDE_DIRS}
    ${SQLITE_INCLUDE_DIRS}
    ${TAGLIB_INCLUDE_DIRS}
    ${MAGICK_INCLUDE_DIRS}
    ${XDGBASEDIR_INCLUDE_DIRS}
    ${FFMPEG_INCLUDE_DIRS}
    ${MAD_INCLUDE_DIRS}
    ${ALSA_INCLUDE_DIRS}
    ${OPENAL_INCLUDE_DIR}
    ${PULSEAUDIO_INCLUDE_DIRS}
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
    ${MAGICK_LIBRARY_DIRS}
    ${XDGBASEDIR_LIBRARY_DIRS}
    ${FFMPEG_LIBRARY_DIRS}
    ${FLAC_LIBRARY_DIRS}
    ${PULSEAUDIO_LIBRARY_DIRS}
    ${UPNPFRAMEWORK_LIBRARY_DIRS}
    ${DBUSGLIB_LIBRARY_DIRS}
    ${LASTFMLIB_LIBRARY_DIRS}    
)

SET(LINK_LIBS
    ${GDKMM_LIBRARIES}
    ${GTKMM_LIBRARIES}
    ${SQLITE_LIBRARIES}
    ${TAGLIB_LIBRARIES}
    ${MAGICK_LIBRARIES}
    ${XDGBASEDIR_LIBRARIES}
    ${ALSA_LIBRARIES}
    ${OPENAL_LIBRARY}
    ${FFMPEG_LIBRARIES}
    ${FLAC_LIBRARIES}
    ${MAD_LIBRARIES}
    ${PULSEAUDIO_LIBRARIES}
    ${UTILS_LIBRARIES}
    ${UPNPFRAMEWORK_LIBRARIES}
    ${DBUSGLIB_LIBRARIES}
    ${LASTFMLIB_LIBRARIES}
    ${LIBNOTIFY_LIBRARIES}
)

SET(AUDIODECODER_SRC_LIST
    AudioDecoder/audiodecoderfactory.cpp
    AudioDecoder/filereader.cpp
    AudioDecoder/mpegutils.cpp
    AudioDecoder/readerfactory.cpp
)

IF (HAVE_FFMPEG)
    LIST(APPEND AUDIODECODER_SRC_LIST AudioDecoder/ffmpegdecoder.cpp)
ENDIF(HAVE_FFMPEG)

IF (HAVE_MAD)
    LIST(APPEND AUDIODECODER_SRC_LIST AudioDecoder/maddecoder.cpp)
ENDIF(HAVE_MAD)

IF (HAVE_FLAC)
    LIST(APPEND AUDIODECODER_SRC_LIST AudioDecoder/flacdecoder.cpp)
ENDIF(HAVE_FLAC)

IF (HAVE_LIBUPNP)
    LIST(APPEND AUDIODECODER_SRC_LIST AudioDecoder/httpreader.cpp)
ENDIF(HAVE_LIBUPNP)

SET(AUDIORENDERER_SRC_LIST
    AudioRenderer/audiobuffer.cpp
    AudioRenderer/audioframe.cpp
    AudioRenderer/audiorenderer.cpp
    AudioRenderer/audiorendererfactory.cpp
)

IF (HAVE_ALSA)
    LIST(APPEND AUDIORENDERER_SRC_LIST AudioRenderer/alsarenderer.cpp)
ENDIF (HAVE_ALSA)

IF (HAVE_OPENAL)
    LIST(APPEND AUDIORENDERER_SRC_LIST AudioRenderer/openalrenderer.cpp)
ENDIF (HAVE_OPENAL)

IF (HAVE_PULSE)
    LIST(APPEND AUDIORENDERER_SRC_LIST AudioRenderer/pulserenderer.cpp)
ENDIF (HAVE_PULSE)
    

SET(CORE_SRC_LIST
    Core/albumartgrabber.cpp
    Core/gejengel.cpp
    Core/gejengelplugin.cpp
    Core/libraryaccess.cpp
    Core/playback.cpp
    Core/playback.h
    Core/playbackenginefactory.cpp
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
    MusicLibrary/metadata.cpp
    MusicLibrary/musicdb.cpp
    MusicLibrary/musiclibrary.cpp
    MusicLibrary/musiclibraryfactory.cpp
    MusicLibrary/scanner.cpp
    MusicLibrary/track.cpp
)

IF (HAVE_LIBUPNP)
    SET(MUSICLIBRARY_SRC_LIST
        ${MUSICLIBRARY_SRC_LIST}
        MusicLibrary/upnpalbumfetcher.cpp
        MusicLibrary/upnpmusiclibrary.cpp
    )
ENDIF (HAVE_LIBUPNP)

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

IF (HAVE_LIBUPNP)
    LIST(APPEND UI_SRC_LIST
        ui/addupnpserverdlg.cpp
        #ui/upnpalbumview.cpp
        ui/upnpview.cpp
    )

ENDIF (HAVE_LIBUPNP)

ADD_LIBRARY(audiodecoder    STATIC ${AUDIODECODER_SRC_LIST})
ADD_LIBRARY(audiorenderer   STATIC ${AUDIORENDERER_SRC_LIST})
ADD_LIBRARY(core            STATIC ${CORE_SRC_LIST})
ADD_LIBRARY(musiclibrary    STATIC ${MUSICLIBRARY_SRC_LIST})
ADD_LIBRARY(plugins         STATIC ${PLUGIN_SRC_LIST})
ADD_LIBRARY(ui              STATIC ${UI_SRC_LIST})

ADD_EXECUTABLE(gejengel main.cpp)

TARGET_LINK_LIBRARIES(audiodecoder)
TARGET_LINK_LIBRARIES(audiorenderer)
TARGET_LINK_LIBRARIES(plugins)
TARGET_LINK_LIBRARIES(musiclibrary ${UPNP_FRAMEWORK_LIBRARIES})
TARGET_LINK_LIBRARIES(ui core)
TARGET_LINK_LIBRARIES(core audiodecoder audiorenderer musiclibrary plugins ui utils ${LINK_LIBS})

TARGET_LINK_LIBRARIES(gejengel core)
INSTALL(TARGETS gejengel RUNTIME DESTINATION bin)

