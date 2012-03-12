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

#ifndef PLUGIN_MANAGER_H
#define PLUGIN_MANAGER_H

#include <string>
#include <map>
#include <vector>

#include "utils/types.h"

#include "config.h"

#ifdef WIN32
    #include "winconfig.h"
#endif

#ifdef HAVE_LASTFM
#include "Plugins/lastfmplugin.h"
#endif

#ifdef HAVE_LIBNOTIFY
#include "Plugins/notificationplugin.h"
#endif

#ifdef HAVE_DBUS
#include "Plugins/dbusplugin.h"
#endif

namespace Gejengel
{

class Track;
class GejengelCore;
class GejengelPlugin;

class PluginManager
{
public:
    PluginManager(GejengelCore& core);
    ~PluginManager();

    void destroy();

    void sendPlay(const Track& track);
    void sendPause();
    void sendResume();
    void sendStop();
    void sendProgress(int32_t elapsedSeconds);
    void sendVolumeChanged(int32_t volume);

    void setUIPlugins(std::vector<GejengelPlugin*>& plugins);
    const std::map<GejengelPlugin*, bool>& getPlugins() const;
    bool setPluginEnabled(GejengelPlugin* plugin, bool enabled);
    void saveSettings();

private:
    void loadSettings();

    void addPlugin(GejengelPlugin& pPlugin, bool enabled);
    void initializePlugins();
    void destroyPlugins();

    GejengelCore&                   m_Core;
    
#ifdef HAVE_LASTFM
    LastFmPlugin                    m_LastFmPlugin;
#endif
#ifdef HAVE_LIBNOTIFY
    NotificationPlugin              m_NotificationPlugin;
#endif    
#ifdef HAVE_DBUS
    DBusPlugin                      m_DBusPlugin;
#endif

    std::map<GejengelPlugin*, bool> m_Plugins;
    std::vector<GejengelPlugin*>    m_UIPlugins;
};

}

#endif

