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

#include "dbusplugin.h"

#include <stdexcept>
#include <cassert>

#include <glibmm/i18n.h>
#include <gtkmm/icontheme.h>
#include <gdkmm/pixbufloader.h>

#include <dbus/dbus-glib.h>

#include "Core/settings.h"
#include "utils/log.h"
#include "utils/trace.h"

#include "mprisroot.h"
#include "mprisplayer.h"
#include "mprisroot-glue.h"
#include "mprisplayer-glue.h"
#include "mpristracklist.h"
#include "mpristracklist-glue.h"

using namespace std;
using namespace utils;

DBusPlugin::DBusPlugin()
: m_pConnection(nullptr)
, m_pProxy(nullptr)
, m_pMprisRoot(nullptr)
, m_pMprisPlayer(nullptr)
, m_pMprisTrackList(nullptr)
{
    utils::trace("Create DBusPlugin");
    m_HasSettingsDialog = false;
}

DBusPlugin::~DBusPlugin()
{
    destroy();
}

bool DBusPlugin::initialize(Gejengel::IGejengelCore& core)
{
    utils::trace("Init DBusPlugin");

    GError* pError = nullptr;
    m_pConnection = dbus_g_bus_get(DBUS_BUS_SESSION, &pError);

    if (m_pConnection == nullptr)
    {
        log::error("Failed to open dbus connection: %s", pError->message);
        g_error_free(pError);
        return false;
    }

    m_pProxy = dbus_g_proxy_new_for_name(m_pConnection, "org.freedesktop.DBus", "/org/freedesktop/DBus", "org.freedesktop.DBus");

    guint requestResult;
    dbus_g_proxy_call(m_pProxy, "RequestName", &pError, G_TYPE_STRING, "org.mpris.gejengel", G_TYPE_UINT, 0, G_TYPE_INVALID, G_TYPE_UINT, &requestResult, G_TYPE_INVALID);
    if (pError)
    {
        log::error("Error requesting name: %s", pError->message);
        g_error_free(pError);
        return false;
    }

    if (!requestResult)
    {
        log::error("Failed to request a name");
        return false;
    }

    m_pMprisRoot = reinterpret_cast<MprisRoot*>(g_object_new(mpris_root_get_type(), nullptr));
    m_pMprisPlayer = reinterpret_cast<MprisPlayer*>(g_object_new(mpris_player_get_type(), nullptr));
    m_pMprisTrackList = reinterpret_cast<MprisTrackList*>(g_object_new(mpris_tracklist_get_type(), nullptr));

    m_pMprisRoot->pCore = &core;
    m_pMprisPlayer->pCore = &core;
    m_pMprisTrackList->pCore = &core;

    dbus_g_object_type_install_info(mpris_root_get_type(), &dbus_glib_mprisRoot_object_info);
    dbus_g_object_type_install_info(mpris_player_get_type(), &dbus_glib_mprisPlayer_object_info);
    dbus_g_object_type_install_info(mpris_tracklist_get_type(), &dbus_glib_mprisTrackList_object_info);
    dbus_g_connection_register_g_object(m_pConnection, "/", G_OBJECT(m_pMprisRoot));
    dbus_g_connection_register_g_object(m_pConnection, "/Player", G_OBJECT(m_pMprisPlayer));
    dbus_g_connection_register_g_object(m_pConnection, "/TrackList", G_OBJECT(m_pMprisTrackList));

    m_pMprisProxy = dbus_g_proxy_new_for_name(m_pConnection, "org.mpris.gejengel", "/Player", "org.freedesktop.MediaPlayer");
    dbus_g_proxy_add_signal(m_pMprisProxy, "StatusChange", G_TYPE_INT, G_TYPE_INVALID);
    dbus_g_proxy_add_signal(m_pMprisProxy, "CapsChange", G_TYPE_INT, G_TYPE_INVALID);
    dbus_g_proxy_add_signal(m_pMprisProxy, "TrackChange", DBUS_TYPE_G_STRING_VALUE_HASHTABLE, G_TYPE_INVALID);

    mprisTrackList_subscribe(m_pMprisTrackList);

    return true;
}

void DBusPlugin::destroy()
{
    if (m_pMprisTrackList)
    {
        mprisTrackList_unsubscribe(m_pMprisTrackList);
        g_object_unref(m_pMprisTrackList);
        m_pMprisTrackList = nullptr;
    }

    if (m_pMprisPlayer)
    {
        g_object_unref(m_pMprisPlayer);
        m_pMprisPlayer = nullptr;
    }

    if (m_pMprisRoot)
    {
        g_object_unref(m_pMprisRoot);
        m_pMprisRoot = nullptr;
    }
}

std::string DBusPlugin::getName() const
{
    return "DBus";
}

std::string DBusPlugin::getDescription() const
{
    return _("Implements the MPRIS specification allowing control through dbus");
}

Glib::RefPtr<Gdk::Pixbuf> DBusPlugin::getIcon() const
{
    Glib::RefPtr<Gtk::IconTheme> theme = Gtk::IconTheme::get_default();
    Glib::RefPtr<Gdk::Pixbuf> icon;
    
    try
    {
        icon = theme->load_icon("emblem-system", 32, Gtk::IconLookupFlags(0));
    }
    catch (Glib::Error& e)
    {
        log::error("Failed to load icon: %s", e.what());
    }
    
    return icon;
}

void DBusPlugin::onPlay(const Gejengel::Track& track)
{
    mprisPlayer_emit_status_change(m_pMprisPlayer);
    mprisPlayer_emit_track_change(m_pMprisPlayer, track);
}

void DBusPlugin::onPause()
{
    mprisPlayer_emit_status_change(m_pMprisPlayer);
    mprisPlayer_emit_caps_change(m_pMprisPlayer);
}

void DBusPlugin::onResume()
{
    mprisPlayer_emit_status_change(m_pMprisPlayer);
    mprisPlayer_emit_caps_change(m_pMprisPlayer);
}

void DBusPlugin::onStop()
{
    mprisPlayer_emit_status_change(m_pMprisPlayer);
    mprisPlayer_emit_caps_change(m_pMprisPlayer);
}

void DBusPlugin::showSettingsDialog(Gejengel::Settings& settings)
{
}
