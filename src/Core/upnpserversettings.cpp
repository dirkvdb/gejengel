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

#include "upnpserversettings.h"

#ifdef HAVE_LIBUPNP

#include <fstream>
#include <sstream>

#ifdef WIN32
#include "winconfig.h"
#endif

#include "utils/log.h"
#include "utils/fileoperations.h"
#include "utils/stringoperations.h"

#define CONFIG_FILE     "servers.cfg"

using namespace std;
using namespace utils;


namespace Gejengel
{

UPnPServerSettings::UPnPServerSettings(const string& settingsFile)
: m_SettingsFile(settingsFile)
{
    if (m_SettingsFile.empty())
    {
        determineSettingsPath();
    }

    loadFromFile();
}

UPnPServerSettings::~UPnPServerSettings()
{
}

std::shared_ptr<upnp::Device> UPnPServerSettings::getServer(const std::string& userDefinedName) const
{
    for (auto& server : m_Servers)
    {
        if (server->m_UserDefinedName == userDefinedName)
        {
            return server;
        }
    }

    throw std::logic_error("Server not found: " + userDefinedName);
}

std::vector<std::shared_ptr<upnp::Device>> UPnPServerSettings::getServers() const
{
    return m_Servers;
}

void UPnPServerSettings::setServers(std::vector<std::shared_ptr<upnp::Device>>& servers)
{
    m_Servers = servers;
}

void UPnPServerSettings::saveToFile()
{
    ofstream file(m_SettingsFile.c_str(), ios_base::trunc);
    if (!file.is_open())
    {
        log::error("Failed to open settings file for saving: %s", m_SettingsFile);
        return;
    }

    for (auto& device: m_Servers)
    {
        if (device->implementsService(upnp::ServiceType::ContentDirectory))
        {
            auto url = device->m_Services[upnp::ServiceType::ContentDirectory].m_ControlURL;
            file << device->m_UserDefinedName << "=" << device->m_UDN << "=" << device->m_ContainerId << "=" << url << endl;
        }
    }
}

void UPnPServerSettings::loadFromFile()
{
    ifstream settingsFile(m_SettingsFile.c_str());
    if (!settingsFile.is_open())
    {
        log::info("No config file present, relying on default values");
        return;
    }

    string line;
    while (getline(settingsFile, line))
    {
        if (line.empty())
        {
            continue;
        }

        std::vector<std::string> items = stringops::tokenize(line, "=");
        if (items.size() != 4)
        {
            log::warn("Warning: ignoring malformed line in config file: %s", line);
            continue;
        }

        upnp::Service service;
        service.m_ControlURL      = stringops::trim(items[3].c_str());
        
        auto server = std::make_shared<upnp::Device>();
        server->m_UserDefinedName    = stringops::trim(items[0].c_str());
        server->m_UDN                = stringops::trim(items[1].c_str());
        server->m_ContainerId        = stringops::trim(items[2].c_str());
        server->m_Services[upnp::ServiceType::ContentDirectory] = service;
        m_Servers.push_back(server);
    }
}

void UPnPServerSettings::determineSettingsPath()
{
    string configDir = fileops::combinePath(fileops::getConfigDirectory(), PACKAGE);

    if (!fileops::pathExists(configDir))
    {
        fileops::createDirectory(configDir);
    }

    m_SettingsFile = fileops::combinePath(configDir, CONFIG_FILE);
}

}

#endif
