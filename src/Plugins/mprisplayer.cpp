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

#include "mprisplayer.h"

#include <dbus/dbus-glib.h>
#include <glib.h>
#include <glib-object.h>
#include <glibmm/convert.h>
#include <cstring>

#include "Core/gejengelcore.h"
#include "MusicLibrary/track.h"

G_DEFINE_TYPE(MprisPlayer, mpris_player, G_TYPE_OBJECT)

enum Capabilities
{
    None        = 0,
    GoNext      = 1,
    GoPrev      = (1 << 1),
    Pause       = (1 << 2),
    Play        = (1 << 3),
    Seek        = (1 << 4),
    Metadata    = (1 << 5),
    TrackList   = (1 << 6)
};

guint trackChangedSignal;
guint statusChangedSignal;
guint capabilitiesChangedSignal;

#define DBUS_STRUCT_INT_INT_INT_INT (dbus_g_type_get_struct("GValueArray", G_TYPE_INT, G_TYPE_INT, G_TYPE_INT, G_TYPE_INT, G_TYPE_INVALID))

static void mpris_player_init(MprisPlayer* pInstance)
{
    pInstance->capabilities = None;
    pInstance->pCore = nullptr;
}

static void mpris_player_class_init(MprisPlayerClass* pInstance)
{
    trackChangedSignal          = g_signal_new("track_change", G_OBJECT_CLASS_TYPE(pInstance), GSignalFlags(G_SIGNAL_RUN_LAST | G_SIGNAL_DETAILED), 0, NULL, NULL, g_cclosure_marshal_VOID__BOXED, G_TYPE_NONE, 1, DBUS_TYPE_G_STRING_VALUE_HASHTABLE);
    statusChangedSignal         = g_signal_new("status_change", G_OBJECT_CLASS_TYPE(pInstance), GSignalFlags(G_SIGNAL_RUN_LAST | G_SIGNAL_DETAILED), 0, NULL, NULL, g_cclosure_marshal_VOID__INT, G_TYPE_NONE, 1, DBUS_STRUCT_INT_INT_INT_INT);
    capabilitiesChangedSignal   = g_signal_new("caps_change", G_OBJECT_CLASS_TYPE(pInstance), GSignalFlags(G_SIGNAL_RUN_LAST | G_SIGNAL_DETAILED), 0, NULL, NULL, g_cclosure_marshal_VOID__INT, G_TYPE_NONE, 1, G_TYPE_INT);
}

bool checkCapabilities(MprisPlayer* pInstance)
{
    gint caps = TrackList;

    Gejengel::PlaybackState state = pInstance->pCore->getPlaybackState();

    switch(state)
    {
    case Gejengel::Paused:
        caps |= Play | Metadata | Seek;
        break;
    case Gejengel::Stopped:
        caps |= Play;
        break;
    case Gejengel::Playing:
        caps |= Pause | Metadata | Seek;
        break;
    }

    caps |= GoNext;

    if (caps != pInstance->capabilities)
    {
        pInstance->capabilities = caps;
        return true;
    }

    return false;
}

gboolean mprisPlayer_next(MprisPlayer* pInstance, GError** pError)
{
    pInstance->pCore->next();
    return TRUE;
}

gboolean mprisPlayer_prev(MprisPlayer* pInstance, GError** pError)
{
    return TRUE;
}

gboolean mprisPlayer_pause(MprisPlayer* pInstance, GError** pError)
{
    pInstance->pCore->pause();
    return TRUE;
}

gboolean mprisPlayer_stop(MprisPlayer* pInstance, GError** pError)
{
    pInstance->pCore->stop();
    return TRUE;
}

gboolean mprisPlayer_play(MprisPlayer* pInstance, GError** pError)
{
    if (pInstance->pCore->getPlaybackState() == Gejengel::Paused)
    {
        pInstance->pCore->resume();
    }
    else
    {
        pInstance->pCore->play();
    }
    return TRUE;
}

gboolean mprisPlayer_get_caps(MprisPlayer* pInstance, gint* pCaps, GError** pError)
{
    checkCapabilities(pInstance);

    *pCaps = pInstance->capabilities;
    return TRUE;
}

gboolean mprisPlayer_volume_set(MprisPlayer* pInstance, gint volume, GError** pError)
{
    pInstance->pCore->setVolume(volume);
    return TRUE;
}

gboolean mprisPlayer_volume_get(MprisPlayer* pInstance, gint* pVolume, GError** pError)
{
    *pVolume = pInstance->pCore->getVolume();
    return TRUE;
}

gboolean mprisPlayer_position_set(MprisPlayer* pInstance, gint ms, GError** pError)
{
    pInstance->pCore->seek(ms / 1000.0);
    return TRUE;
}

gboolean mprisPlayer_position_get(MprisPlayer* pInstance, gint* pMs, GError** pError)
{
    *pMs = pInstance->pCore->getTrackPosition() * 1000;
    return TRUE;
}

gboolean mprisPlayer_repeat(MprisPlayer* pInstance, gboolean repeat, GError** pError)
{
    return TRUE;
}

static void addStringToMetadata(GHashTable* pHt, gchar* pName, const std::string& value)
{
    GValue* pValue = g_new0(GValue, 1);
    g_value_init(pValue, G_TYPE_STRING);
    g_value_set_string(pValue, value.c_str());
    g_hash_table_insert(pHt, pName, pValue);
}

static void addIntToMetadata(GHashTable* pHt, gchar* pName, int value)
{
    GValue* pValue = g_new0(GValue, 1);
    g_value_init(pValue, G_TYPE_INT);
    g_value_set_int(pValue, value);
    g_hash_table_insert(pHt, pName, pValue);
}

static GHashTable* createTrackMetadata(const Gejengel::Track& track)
{
    GHashTable* pMetadata = g_hash_table_new(g_str_hash, g_str_equal);

    if (track.filepath.empty())
    {
        return pMetadata;
    }

    try
    {
        addStringToMetadata(pMetadata, (gchar*) "location",    Glib::filename_to_uri(track.filepath).c_str());
    }
    catch (Glib::ConvertError&)
    {
        addStringToMetadata(pMetadata, (gchar*) "location",    track.filepath.c_str());
    }

    addStringToMetadata(pMetadata, (gchar*) "artist",      track.artist.c_str());
    addStringToMetadata(pMetadata, (gchar*) "title",       track.title.c_str());
    addStringToMetadata(pMetadata, (gchar*) "album",       track.album.c_str());

    if (!track.genre.empty())
    {
        addStringToMetadata(pMetadata, (gchar*) "genre", track.genre.c_str());
    }

    if (track.trackNr != 0)
    {
        addIntToMetadata(pMetadata, (gchar*) "tracknumber", track.trackNr);
    }

    if (track.durationInSec != 0)
    {
        addIntToMetadata(pMetadata, (gchar*) "time", track.durationInSec);
    }

    if (track.year != 0)
    {
        addIntToMetadata(pMetadata, (gchar*) "year", track.year);
    }

    if (track.sampleRate != 0)
    {
        addIntToMetadata(pMetadata, (gchar*) "audio-samplerate", track.sampleRate);
    }

    if (track.bitrate != 0)
    {
        addIntToMetadata(pMetadata, (gchar*) "audio-bitrate", track.bitrate * 1000);
    }

    return pMetadata;
}

gboolean mprisPlayer_get_metadata(MprisPlayer* pInstance, GHashTable** pMetadata, GError** pError)
{
    Gejengel::Track track;
    pInstance->pCore->getCurrentTrack(track);

    *pMetadata = createTrackMetadata(track);
    
    return TRUE;
}

gboolean mprisPlayer_get_status(MprisPlayer* pInstance, GValue** pStatus, GError** pError)
{
    static GValue value;
    memset(&value, 0, sizeof(GValue));
    g_value_init(&value, DBUS_STRUCT_INT_INT_INT_INT);
    
    int playStatus = 2;
    switch (pInstance->pCore->getPlaybackState())
    {
    case Gejengel::Playing:
        playStatus = 0;
        break;
    case Gejengel::Paused:
        playStatus = 1;
        break;
    case Gejengel::Stopped:
        playStatus = 2;
        break;
    }

    g_value_take_boxed(&value, dbus_g_type_specialized_construct(DBUS_STRUCT_INT_INT_INT_INT));
    dbus_g_type_struct_set(&value, 0, playStatus, 1, 0, 2, 0, 3, 0, G_MAXUINT);
    g_signal_emit(pInstance, statusChangedSignal, 0, g_value_get_boxed(&value));
    
    *pStatus = reinterpret_cast<GValue*>(g_value_get_boxed(&value));

    return TRUE;
}

gboolean mprisPlayer_emit_caps_change(MprisPlayer* pInstance)
{
    if (checkCapabilities(pInstance))
    {
        g_signal_emit(pInstance, capabilitiesChangedSignal, 0, pInstance->capabilities);
        return TRUE;
    }

    return FALSE;
}

gboolean mprisPlayer_emit_track_change(MprisPlayer* pInstance, const Gejengel::Track& track)
{
    GHashTable* pMetadata = createTrackMetadata(track);
    g_signal_emit(pInstance, trackChangedSignal, 0, pMetadata);
    return TRUE;
}

gboolean mprisPlayer_emit_status_change(MprisPlayer* pInstance)
{
    //ugly duplication, but I have no interest in learning this gobject cr*p
    //segfaults when trying to make member value of the GValue
    
    static GValue value;
    memset(&value, 0, sizeof(GValue));
    g_value_init(&value, DBUS_STRUCT_INT_INT_INT_INT);
    
    int playStatus = 2;
    switch (pInstance->pCore->getPlaybackState())
    {
    case Gejengel::Playing:
        playStatus = 0;
        break;
    case Gejengel::Paused:
        playStatus = 1;
        break;
    case Gejengel::Stopped:
        playStatus = 2;
        break;
    }

    g_value_take_boxed(&value, dbus_g_type_specialized_construct(DBUS_STRUCT_INT_INT_INT_INT));
    dbus_g_type_struct_set(&value, 0, playStatus, 1, 0, 2, 0, 3, 0, G_MAXUINT);
    g_signal_emit(pInstance, statusChangedSignal, 0, g_value_get_boxed(&value));
    
    return TRUE;
}
