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

#ifndef MPRIS_TRACKLIST_H
#define MPRIS_TRACKLIST_H

#include <glib-object.h>
#include <glib.h>

#include "Core/playqueue.h"

namespace Gejengel
{
    class IGejengelCore;
    class Track;
}

struct MprisTrackList;

class MprisPlayQueueSubscriber : public Gejengel::PlayQueueSubscriber
{
public:
    MprisPlayQueueSubscriber(MprisTrackList* pInstance);

    void onTrackQueued(uint32_t index, const Gejengel::Track& track);
    void onTrackRemoved(uint32_t index);
    void onTrackMoved(uint32_t sourceIndex, uint32_t destIndex);
    void onQueueCleared();

private:
    MprisTrackList* m_pMpris;
};

struct MprisTrackList
{
    GObject                     parent;
    Gejengel::IGejengelCore*    pCore;
    MprisPlayQueueSubscriber*   pSubscriber;
};

struct MprisTrackListClass
{
    GObjectClass parent;
};

GType mpris_tracklist_get_type(void);

gboolean mprisTrackList_del_track(MprisTrackList* pInstance, gint track, GError** pError);
gboolean mprisTrackList_add_track(MprisTrackList* pInstance, const gchar* pUri, gboolean playnow, gint* pFailed, GError** pError);
gboolean mprisTrackList_get_length(MprisTrackList* pInstance, gint* pLength, GError** pError);
gboolean mprisTrackList_get_current_track(MprisTrackList* pInstance, gint* pTrack, GError** pError);
gboolean mprisTrackList_set_loop(MprisTrackList* pInstance, gboolean on, GError** pError);
gboolean mprisTrackList_set_random(MprisTrackList* pInstance, gboolean on, GError** pError);
gboolean mprisTrackList_get_metadata(MprisTrackList* pInstance, gint trackNr, GHashTable** pMeta, GError** pError);

void mprisTrackList_subscribe(MprisTrackList* pInstance);
void mprisTrackList_unsubscribe(MprisTrackList* pInstance);
void mprisTrackList_emit_track_list_change(MprisTrackList* pInstance);

#endif
