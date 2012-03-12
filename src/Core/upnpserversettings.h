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

#ifndef UPNP_SERVER_SETTINGS_H
#define UPNP_SERVER_SETTINGS_H

#include "config.h"

#ifdef HAVE_LIBUPNP

#include <string>
#include <vector>

#include "utils/types.h"
#include "upnp/upnpdevice.h"

namespace Gejengel
{

class UPnPServerSettings
{
public:
    UPnPServerSettings(const std::string& settingsFile = "");
    ~UPnPServerSettings();

    upnp::Device getServer(const std::string& userDefinedName) const;
    std::vector<upnp::Device> getServers() const;
    void setServers(std::vector<upnp::Device>& servers);

    void saveToFile();

private:
    void loadFromFile();
    void determineSettingsPath();

    std::string                 m_SettingsFile;
    std::vector<upnp::Device>   m_Servers;
};

}

#endif
#endif

