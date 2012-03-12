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

#ifndef DBUS_PLUGIN_H
#define DBUS_PLUGIN_H

#include "Core/gejengelplugin.h"

struct _DBusGConnection;
struct _DBusGProxy;
struct MprisRoot;
struct MprisPlayer;
struct MprisTrackList;

typedef _DBusGConnection DBusGConnection;
typedef _DBusGProxy DBusGProxy;

namespace Gejengel
{
    class Settings;
}

class DBusPlugin : public Gejengel::GejengelPlugin
{
public:
    DBusPlugin();
    ~DBusPlugin();

    bool initialize(Gejengel::IGejengelCore& core);

    std::string getName() const;
    std::string getDescription() const;
    Glib::RefPtr<Gdk::Pixbuf> getIcon() const;

    void onPlay(const Gejengel::Track& track);
    void onPause();
    void onResume();
    void onStop();

    void destroy();

    void showSettingsDialog(Gejengel::Settings& settings);

private:
    DBusGConnection*    m_pConnection;
    DBusGProxy*         m_pProxy;
    DBusGProxy*         m_pMprisProxy;

    MprisRoot*          m_pMprisRoot;
    MprisPlayer*        m_pMprisPlayer;
    MprisTrackList*     m_pMprisTrackList;
};

#endif
