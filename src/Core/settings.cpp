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

#include "settings.h"

#include <fstream>
#include <sstream>

#include "config.h"

#ifdef WIN32
#include "winconfig.h"
#endif

#include "utils/log.h"
#include "utils/fileoperations.h"
#include "utils/stringoperations.h"
#include "utils/numericoperations.h"

#define CONFIG_FILE     "gejengel.cfg"
#define DB_FILE         "gejengel.db"

using namespace std;
using namespace utils;

namespace Gejengel
{

Settings::Settings(const string& settingsFile)
: m_SettingsFile(settingsFile)
{
    if (m_SettingsFile.empty())
    {
        determineSettingsPath();
    }

    loadFromFile();
}

Settings::~Settings()
{
    saveToFile();
}

std::string Settings::get(const std::string& setting, const std::string& defaultValue) const
{
    std::map<std::string, std::string>::const_iterator iter = m_Settings.find(setting);
    if (iter != m_Settings.end())
    {
        return iter->second;
    }

    return defaultValue;
}

int32_t Settings::getAsInt(const std::string& setting, int32_t defaultValue) const
{
    int32_t result = defaultValue;

    std::map<std::string, std::string>::const_iterator iter = m_Settings.find(setting);
    if (iter != m_Settings.end())
    {
        result = stringops::toNumeric<int32_t>(iter->second);
    }

    return result;
}

bool Settings::getAsBool(const std::string& setting, bool defaultValue) const
{
    bool result = defaultValue;

    std::map<std::string, std::string>::const_iterator iter = m_Settings.find(setting);
    if (iter != m_Settings.end())
    {
        string value = stringops::lowercase(iter->second);
        if (value == "true")
        {
            result = true;
        }
        else if (value == "false")
        {
            result = false;
        }
    }

    return result;
}

void Settings::getAsVector(const std::string& setting, std::vector<std::string>& array) const
{
	array.clear();
	std::string value = get(setting);

	if (!value.empty())
	{
		array = stringops::tokenize(value, ";");
		for (size_t i = 0; i < array.size(); ++i)
		{
			stringops::trim(array[i]);
		}
	}
}


void Settings::set(const std::string& setting, const std::string& value)
{
    m_Settings[setting] = value;
}

void Settings::set(const std::string& setting, int32_t value)
{
    set(setting, numericops::toString(value));
}

void Settings::set(const std::string& setting, bool value)
{
    set(setting, string(value ? "true" : "false"));
}

void Settings::saveToFile()
{
    ofstream file(m_SettingsFile.c_str(), ios_base::trunc);
    if (!file.is_open())
    {
        log::error("Failed to open settings file for saving: " + m_SettingsFile);
        return;
    }

    for (std::map<std::string, std::string>::iterator iter = m_Settings.begin(); iter != m_Settings.end(); ++iter)
    {
        file << iter->first << "=" << iter->second << endl;
    }
}

void Settings::loadFromFile()
{
    loadDefaultSettings();

    ifstream settingsFile(m_SettingsFile.c_str());
    if (!settingsFile.is_open())
    {
        log::info("No config file present, relying on default values");
        return;
    }

    string line;
    while(getline(settingsFile, line))
    {
        if (line.empty())
            continue;

        size_t pos = line.find('=');
        if (pos == string::npos)
        {
            log::warn("Warning: ignoring malformed line in config file: " + line);
        }

        string setting = line.substr(0, pos);
        string value = line.substr(pos + 1);
        stringops::trim(setting);
        stringops::trim(value);
        m_Settings[setting] = value;
    }
}

void Settings::loadDefaultSettings()
{
    string configDir = fileops::combinePath(fileops::getConfigDirectory(), PACKAGE);
    string dataDir = fileops::combinePath(fileops::getDataDirectory(), PACKAGE);
    m_Settings["DBFile"] = fileops::combinePath(dataDir, DB_FILE);
    m_Settings["ScanAtStartup"] = "false";

#ifdef HAVE_PULSE
    m_Settings["AudioBackend"] = "PulseAudio";
#endif
#ifdef HAVE_OPENAL
    m_Settings["AudioBackend"] = "OpenAL";
#endif
#ifdef HAVE_ALSA
    m_Settings["AudioBackend"] = "Alsa";
#endif

    m_Settings["PlaybackEngine"] = "FFmpeg";
}

void Settings::determineSettingsPath()
{
    string configDir = fileops::combinePath(fileops::getConfigDirectory(), PACKAGE);
    string dataDir = fileops::combinePath(fileops::getDataDirectory(), PACKAGE);
    
    if (!fileops::pathExists(configDir))
    {
        fileops::createDirectory(configDir);
    }
    
    if (!fileops::pathExists(dataDir))
    {
        fileops::createDirectory(dataDir);
    }

    m_SettingsFile = fileops::combinePath(configDir, CONFIG_FILE);
}

}
