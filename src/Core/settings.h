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

#ifndef SETTINGS_H
#define SETTINGS_H

#include <string>
#include <map>
#include <vector>

#include "utils/types.h"

namespace Gejengel
{

class Settings
{
public:
    Settings(const std::string& settingsFile = "");
    ~Settings();

    std::string get(const std::string& setting, const std::string& defaultValue = "") const;
    int32_t getAsInt(const std::string& setting, int32_t defaultValue = 0) const;
    bool getAsBool(const std::string& setting, bool defaultValue) const;
    void getAsVector(const std::string& setting, std::vector<std::string>& array) const;
    
    void set(const std::string& setting, const std::string& value);
    void set(const std::string& setting, int32_t value);
    void set(const std::string& setting, bool value);

    void saveToFile();

private:
    void loadFromFile();
    void loadDefaultSettings();
    void determineSettingsPath();

    std::string                         m_SettingsFile;
    std::map<std::string, std::string>  m_Settings;
};

}

#endif

