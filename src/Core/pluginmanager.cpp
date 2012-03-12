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

#include "pluginmanager.h"

#include <cassert>
#include <vector>

#include "settings.h"
#include "gejengel.h"
#include "gejengelplugin.h"
#include "utils/log.h"
#include "utils/trace.h"
#include "utils/stringoperations.h"
#include "utils/numericoperations.h"

using namespace std;
using namespace utils;

namespace Gejengel
{

PluginManager::PluginManager(GejengelCore& core)
: m_Core(core)
{
    utils::trace("Create Plugin manager");
    
#ifdef HAVE_LASTFM
    addPlugin(m_LastFmPlugin, false);
#endif
#ifdef HAVE_LIBNOTIFY
    addPlugin(m_NotificationPlugin, false);
#endif
#ifdef HAVE_DBUS
    addPlugin(m_DBusPlugin, false);
#endif

    loadSettings();

    utils::trace("Initialize plugins");
    initializePlugins();

    utils::trace("Plugin manager ready");
}

PluginManager::~PluginManager()
{
    destroy();
}

void PluginManager::destroy()
{
    destroyPlugins();
}

void PluginManager::loadSettings()
{
    vector<string> enabledPlugins = stringops::tokenize(m_Core.getSettings().get("EnabledPlugins"), ",");

    for (map<GejengelPlugin*, bool>::iterator iter = m_Plugins.begin(); iter != m_Plugins.end(); ++iter)
    {
        if (find(enabledPlugins.begin(), enabledPlugins.end(), iter->first->getName()) != enabledPlugins.end())
        {
            iter->second = true;
        }
    }
}

void PluginManager::saveSettings()
{
    string enabledPlugins;

    for (map<GejengelPlugin*, bool>::iterator iter = m_Plugins.begin(); iter != m_Plugins.end(); ++iter)
    {
        if (iter->second)
        {
            if (!enabledPlugins.empty())
            {
                enabledPlugins += ',';
            }

            enabledPlugins += iter->first->getName();
        }
    }

    m_Core.getSettings().set("EnabledPlugins", enabledPlugins);
}

void PluginManager::initializePlugins()
{
    for (map<GejengelPlugin*, bool>::iterator iter = m_Plugins.begin(); iter != m_Plugins.end(); ++iter)
    {
        if (iter->second)
        {
            if (iter->first->initialize(m_Core))
            {
                log::info("Plugin initialized: " + iter->first->getName());
            }
            else
            {
                log::error("Failed to initialize plugin: " + iter->first->getName());
            }
        }
    }
}

void PluginManager::destroyPlugins()
{
    for (map<GejengelPlugin*, bool>::iterator iter = m_Plugins.begin(); iter != m_Plugins.end(); ++iter)
    {
        if (iter->second)
        {
            iter->first->destroy();
        }
    }
}

void PluginManager::sendPlay(const Track& track)
{
    for (size_t i = 0; i < m_UIPlugins.size(); ++i)
    {
        m_UIPlugins[i]->onPlay(track);
    }

    for (map<GejengelPlugin*, bool>::iterator iter = m_Plugins.begin(); iter != m_Plugins.end(); ++iter)
    {
        if (iter->second)
            iter->first->onPlay(track);
    }
}

void PluginManager::sendPause()
{
    for (size_t i = 0; i < m_UIPlugins.size(); ++i)
    {
        m_UIPlugins[i]->onPause();
    }

    for (map<GejengelPlugin*, bool>::iterator iter = m_Plugins.begin(); iter != m_Plugins.end(); ++iter)
    {
        if (iter->second)
            iter->first->onPause();
    }
}

void PluginManager::sendResume()
{
    for (size_t i = 0; i < m_UIPlugins.size(); ++i)
    {
        m_UIPlugins[i]->onResume();
    }

    for (map<GejengelPlugin*, bool>::iterator iter = m_Plugins.begin(); iter != m_Plugins.end(); ++iter)
    {
        if (iter->second)
            iter->first->onResume();
    }
}

void PluginManager::sendStop()
{
    for (size_t i = 0; i < m_UIPlugins.size(); ++i)
    {
        m_UIPlugins[i]->onStop();
    }

    for (map<GejengelPlugin*, bool>::iterator iter = m_Plugins.begin(); iter != m_Plugins.end(); ++iter)
    {
        if (iter->second)
            iter->first->onStop();
    }
}

void PluginManager::sendProgress(int32_t elapsedSeconds)
{
    for (size_t i = 0; i < m_UIPlugins.size(); ++i)
    {
        m_UIPlugins[i]->onProgress(elapsedSeconds);
    }

    for (map<GejengelPlugin*, bool>::iterator iter = m_Plugins.begin(); iter != m_Plugins.end(); ++iter)
    {
        if (iter->second)
            iter->first->onProgress(elapsedSeconds);
    }
}

void PluginManager::sendVolumeChanged(int32_t volume)
{
    for (size_t i = 0; i < m_UIPlugins.size(); ++i)
    {
        m_UIPlugins[i]->onVolumeChanged(volume);
    }

    for (map<GejengelPlugin*, bool>::iterator iter = m_Plugins.begin(); iter != m_Plugins.end(); ++iter)
    {
        if (iter->second)
            iter->first->onVolumeChanged(volume);
    }
}

void PluginManager::addPlugin(GejengelPlugin& plugin, bool enabled)
{
    m_Plugins[&plugin] = enabled;

    if (!enabled)
        return;

    if (!plugin.initialize(m_Core))
    {
        m_Plugins[&plugin] = false;
        log::error("Failed to initialize plugin: " + plugin.getName());
    }
}

void PluginManager::setUIPlugins(std::vector<GejengelPlugin*>& plugins)
{
    m_UIPlugins = plugins;

    for (size_t i = 0; i < m_UIPlugins.size(); ++i)
    {
        m_UIPlugins[i]->initialize(m_Core);
    }
}

const std::map<GejengelPlugin*, bool>& PluginManager::getPlugins() const
{
    return m_Plugins;
}

bool PluginManager::setPluginEnabled(GejengelPlugin* pPlugin, bool enabled)
{
    assert(m_Plugins.find(pPlugin) != m_Plugins.end());

    if (enabled)
    {
        if (!pPlugin->initialize(m_Core))
        {
            m_Plugins[pPlugin] = false;
            log::error("Failed to initialize plugin: " + pPlugin->getName());
            return false;
        }
        else
        {
            m_Plugins[pPlugin] = true;
            log::info("Plugin initialized: " + pPlugin->getName());
        }
    }
    else
    {
        m_Plugins[pPlugin] = false;
        pPlugin->destroy();
        log::debug("Plugin destroyed: " + pPlugin->getName());
    }

    return true;
}

}
