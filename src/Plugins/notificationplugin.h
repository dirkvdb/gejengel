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

#ifndef NOTIFICATION_PLUGIN_H
#define NOTIFICATION_PLUGIN_H

#include "Core/gejengelplugin.h"
#include "ui/dispatchedsubscriber.h"
#include "MusicLibrary/albumart.h"
#include <gdkmm/pixbuf.h>
#include <libnotify/notify.h>

namespace Gejengel
{
    class GejenglCore;
}

class NotificationPlugin 	: public Gejengel::GejengelPlugin
							, public Gejengel::IDispatchedSubscriber<Gejengel::AlbumArt>
{
public:
    NotificationPlugin();
    ~NotificationPlugin();

    bool initialize(Gejengel::IGejengelCore& core);
    void destroy();

    std::string getName() const;
    std::string getDescription() const;
    Glib::RefPtr<Gdk::Pixbuf> getIcon() const;

    void onPlay(const Gejengel::Track& track);
    void onDispatchedItem(const Gejengel::AlbumArt& art, void* pData = nullptr);

private:
    void iconThemeChanged();
    static void onActionCb(NotifyNotification* pNotification, gchar* pAction, gpointer pData);

    Gejengel::IGejengelCore*    m_pCore;
    Glib::RefPtr<Gdk::Pixbuf>   m_DefaultAlbumArt;
    NotifyNotification*         m_pNotification;
    uint32_t                    m_AlbumArtSize;
    Gejengel::Track				m_CurrentTrack;
};

#endif
