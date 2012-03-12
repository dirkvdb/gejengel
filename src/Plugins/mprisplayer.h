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

#ifndef MPRIS_PLAYER_H
#define MPRIS_PLAYER_H

#include <glib-object.h>
#include <glib.h>

#define DBUS_TYPE_G_STRING_VALUE_HASHTABLE (dbus_g_type_get_map("GHashTable", G_TYPE_STRING, G_TYPE_VALUE))

namespace Gejengel
{
    class IGejengelCore;
    class Track;
}

struct MprisPlayer
{
    GObject                     parent;
    Gejengel::IGejengelCore*    pCore;
    gint                        capabilities;
};

struct MprisPlayerClass
{
    GObjectClass    parent;
};

GType mpris_player_get_type();

gboolean mprisPlayer_next(MprisPlayer* pInstance, GError** pError);
gboolean mprisPlayer_prev(MprisPlayer* pInstance, GError** pError);
gboolean mprisPlayer_pause(MprisPlayer* pInstance, GError** pError);
gboolean mprisPlayer_stop(MprisPlayer* pInstance, GError** pError);
gboolean mprisPlayer_play(MprisPlayer* pInstance, GError** pError);

gboolean mprisPlayer_get_caps(MprisPlayer* pInstance, gint* pCaps, GError** pError);
gboolean mprisPlayer_volume_set(MprisPlayer* pInstance, gint volume, GError** pError);
gboolean mprisPlayer_volume_get(MprisPlayer* pInstance, gint* pVolume, GError** pError);
gboolean mprisPlayer_position_set(MprisPlayer* pInstance, gint ms, GError** pError);
gboolean mprisPlayer_position_get(MprisPlayer* pInstance, gint* pMs, GError** pError);
gboolean mprisPlayer_repeat(MprisPlayer* pInstance, gboolean repeat, GError** pError);
gboolean mprisPlayer_get_metadata(MprisPlayer* pInstance, GHashTable** pMetadata, GError** pError);
gboolean mprisPlayer_get_status(MprisPlayer* pInstance, GValue** pStatus, GError** pError);

gboolean mprisPlayer_emit_caps_change(MprisPlayer* pInstance);
gboolean mprisPlayer_emit_track_change(MprisPlayer* pInstance, const Gejengel::Track& track);
gboolean mprisPlayer_emit_status_change(MprisPlayer* pInstance);

#endif
