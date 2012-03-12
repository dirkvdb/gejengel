#include <gtest/gtest.h>
#include <fstream>

#include "Utils/fileoperations.h"
#include "Core/settings.h"

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

using namespace std;
using namespace FileOperations;
using namespace Gejengel;

#define TEST_SETTINGS "test.setting"

TEST(SettingsTest, LoadDefaults)
{
    {
        Settings settings("noexist");

        string home = getHomeDirectory();

        EXPECT_EQ("", settings.get("MusicLibrary"));
        EXPECT_EQ(home + "/.local/share/gejengel/gejengel.db", settings.get("DBFile"));
    #ifdef HAVE_ALSA
        EXPECT_EQ("Alsa", settings.get("AudioBackend"));
    #else
        EXPECT_EQ("OpenAl", settings.get("AudioBackend"));
    #endif
        EXPECT_FALSE(pathExists("noexist"));
    }

    EXPECT_TRUE(pathExists("noexist"));
    deleteFile("noexist");
}

TEST(SettingsTest, LoadSave)
{
    string home = getHomeDirectory();

    {
        ofstream file(TEST_SETTINGS);
        file    << "MusicLibrary=/home/path oh yeah " << endl
                << "AudioBackend = alsa ";
    }

    {
        Settings settings(TEST_SETTINGS);

        ASSERT_EQ("/home/path oh yeah", settings.get("MusicLibrary"));
        ASSERT_EQ(home + "/.local/share/gejengel/gejengel.db", settings.get("DBFile"));
        ASSERT_EQ("alsa", settings.get("AudioBackend"));

        settings.set("MusicLibrary", string("/home/tata/music"));
        settings.set("AudioBackend", string("openal"));
        settings.saveToFile();
    }

    {
        Settings settings(TEST_SETTINGS);
        ASSERT_EQ("/home/tata/music", settings.get("MusicLibrary"));
        ASSERT_EQ("openal", settings.get("AudioBackend"));
    }

    deleteFile(TEST_SETTINGS);
}
