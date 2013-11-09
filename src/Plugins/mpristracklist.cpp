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

#include "mpristracklist.h"

#include "Core/gejengelcore.h"
#include "Core/playqueue.h"
#include "MusicLibrary/track.h"

#include "utils/log.h"

#include <glibmm/convert.h>

guint tracklistChangedSignal;

G_DEFINE_TYPE (MprisTrackList, mpris_tracklist, G_TYPE_OBJECT);

static void mpris_tracklist_init(MprisTrackList* pInstance)
{
    pInstance->pCore = nullptr;
    pInstance->pSubscriber = nullptr;
}

static void mpris_tracklist_class_init(MprisTrackListClass* pInstance)
{
    tracklistChangedSignal = g_signal_new("track_list_change", G_OBJECT_CLASS_TYPE(pInstance), GSignalFlags(G_SIGNAL_RUN_LAST | G_SIGNAL_DETAILED), 0, nullptr, nullptr, g_cclosure_marshal_VOID__BOXED, G_TYPE_NONE, 1, G_TYPE_INT);
}

void mprisTrackList_subscribe(MprisTrackList* pInstance)
{
    pInstance->pSubscriber = new MprisPlayQueueSubscriber(pInstance);
    pInstance->pCore->getPlayQueue().subscribe(*pInstance->pSubscriber);
}

void mprisTrackList_unsubscribe(MprisTrackList* pInstance)
{
    if (pInstance->pSubscriber)
    {
        pInstance->pCore->getPlayQueue().unsubscribe(*pInstance->pSubscriber);
        delete pInstance->pSubscriber;
        pInstance->pSubscriber = nullptr;
    }
}

gboolean mprisTrackList_del_track(MprisTrackList* pInstance, gint track, GError** pError)
{
    pInstance->pCore->getPlayQueue().removeTrack(track);
    return TRUE;
}

gboolean mprisTrackList_add_track(MprisTrackList* pInstance, const gchar* pUri, gboolean playnow, gint* pFailed, GError** pError)
{
    //string filename = pUri;
    return TRUE;
}

gboolean mprisTrackList_get_length(MprisTrackList* pInstance, gint* pLength, GError** pError)
{
    *pLength = pInstance->pCore->getPlayQueue().getNumberOfTracks();
    return TRUE;
}

gboolean mprisTrackList_get_current_track(MprisTrackList* pInstance, gint* pTrack, GError** pError)
{
    *pTrack = -1;
    return TRUE;
}

gboolean mprisTrackList_set_loop(MprisTrackList* pInstance, gboolean on, GError** pError)
{
    return TRUE;
}

gboolean mprisTrackList_set_random(MprisTrackList* pInstance, gboolean on, GError** pError)
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

gboolean mprisTrackList_get_metadata(MprisTrackList* pInstance, gint trackNr, GHashTable** pMeta, GError** pError)
{
    Gejengel::Track track;
    if (pInstance->pCore->getPlayQueue().getTrackInfo(trackNr, track))
    {
        *pMeta = createTrackMetadata(track);
        return TRUE;
    }

    return FALSE;
}

void mprisTrackList_emit_track_list_change(MprisTrackList* pInstance)
{
    g_signal_emit(pInstance, tracklistChangedSignal, 0, pInstance->pCore->getPlayQueue().getNumberOfTracks());
}

MprisPlayQueueSubscriber::MprisPlayQueueSubscriber(MprisTrackList* playlist)
: m_pMpris(playlist)
{
}

void MprisPlayQueueSubscriber::onTrackQueued(uint32_t index, const Gejengel::Track& track)
{
    if (m_pMpris)
    {
        mprisTrackList_emit_track_list_change(m_pMpris);
    }
}

void MprisPlayQueueSubscriber::onTrackRemoved(uint32_t index)
{
    if (m_pMpris)
    {
        mprisTrackList_emit_track_list_change(m_pMpris);
    }
}

void MprisPlayQueueSubscriber::onTrackMoved(uint32_t sourceIndex, uint32_t destIndex)
{
    if (m_pMpris)
    {
        mprisTrackList_emit_track_list_change(m_pMpris);
    }
}

void MprisPlayQueueSubscriber::onQueueCleared()
{
    if (m_pMpris)
    {
        mprisTrackList_emit_track_list_change(m_pMpris);
    }
}

