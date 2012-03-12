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

#include "notificationplugin.h"

#include <sstream>
#include <glibmm/i18n.h>
#include <gtkmm/stock.h>
#include <gtkmm/invisible.h>
#include <gtkmm/icontheme.h>
#include <gdkmm/pixbuf.h>
#include <gdkmm/pixbufloader.h>

#include "config.h"

#include "utils/log.h"
#include "utils/trace.h"
#include "Core/albumartprovider.h"
#include "ui/sharedfunctions.h"
#include "MusicLibrary/album.h"

using namespace std;
using namespace Gtk;
using namespace utils;

NotificationPlugin::NotificationPlugin()
: m_pCore(nullptr)
, m_pNotification(nullptr)
, m_AlbumArtSize(48)
{
    utils::trace("Create NotificationPlugin");
    Glib::RefPtr<IconTheme> theme = IconTheme::get_default();
    theme->signal_changed().connect(sigc::mem_fun(*this, &NotificationPlugin::iconThemeChanged));
}

NotificationPlugin::~NotificationPlugin()
{
    destroy();
}

bool NotificationPlugin::initialize(Gejengel::IGejengelCore& core)
{
    utils::trace("Init NotificationPlugin");
    m_pCore = &core;

    bool serverSupportsActions = false;
    bool ok = notify_init(PACKAGE_NAME) == TRUE;
    if (ok)
    {
        GList* pCaps = notify_get_server_caps();
        if (pCaps != nullptr)
        {
            for(GList* c = pCaps; c != nullptr; c = c->next)
            {
                if( strcmp((char*)c->data, "actions" ) == 0)
                {
                    serverSupportsActions = true;
                    break;
                }
            }
        }

        gchar* pName, *pDummy1, *pDummy2, *pDummy3;
        if (notify_get_server_info(&pName, &pDummy1, &pDummy2, &pDummy3) == TRUE)
        {
            if (!g_strcmp0("notify-osd", pName))
            {
                m_AlbumArtSize = 96;
            }

            g_free(pName); g_free(pDummy1); g_free(pDummy2); g_free(pDummy3);
        }
    }

    m_pNotification = notify_notification_new("dummy", nullptr, nullptr);
    notify_notification_set_timeout(m_pNotification, NOTIFY_EXPIRES_DEFAULT);
    notify_notification_set_urgency(m_pNotification, NOTIFY_URGENCY_NORMAL);

    if (serverSupportsActions)
    {
        notify_notification_add_action(m_pNotification, "gtk-media-next-ltr", "Skip", NotificationPlugin::onActionCb, this, nullptr);
    }

    return ok && m_pNotification != nullptr;
}

void NotificationPlugin::destroy()
{
    if (m_pNotification)
    {
        notify_notification_close(m_pNotification, nullptr);
        m_pNotification = nullptr;
    }

    if (notify_is_initted())
    {
        notify_uninit();
    }
}

std::string NotificationPlugin::getName() const
{
    return _("Notification plugin");
}

std::string NotificationPlugin::getDescription() const
{
    return _("Show notifications for played tracks");
}

Glib::RefPtr<Gdk::Pixbuf> NotificationPlugin::getIcon() const
{
    Glib::RefPtr<IconTheme> theme = IconTheme::get_default();
    return theme->load_icon("emblem-important", 32, IconLookupFlags(0));
}

void NotificationPlugin::onPlay(const Gejengel::Track& track)
{
	if (!m_pCore)
	{
		return;
	}

	m_CurrentTrack = track;
	m_pCore->getAlbumArtProvider().getAlbumArt(m_CurrentTrack, *this);
}

void NotificationPlugin::onDispatchedItem(const Gejengel::AlbumArt& art, void* pData)
{
	gchar* pArtist = g_markup_escape_text(m_CurrentTrack.artist.c_str(), m_CurrentTrack.artist.size());

	stringstream info;
	info    << "<i>" << pArtist << "</i>" << endl
			<< m_CurrentTrack.album;
	if (m_CurrentTrack.year != 0)
	{
		info << endl << m_CurrentTrack.year;
	}

	notify_notification_update(m_pNotification, m_CurrentTrack.title.c_str(), info.str().c_str(), nullptr);

	try
	{
		if (art.getData().size() > 0)
		{
			Glib::RefPtr<Gdk::Pixbuf> image = Gejengel::Shared::createCoverPixBufWithOverlay(art, m_AlbumArtSize);
			notify_notification_set_icon_from_pixbuf(m_pNotification, image->gobj());
		}
		else
		{
			if (!m_DefaultAlbumArt)
			{
				Glib::RefPtr<IconTheme> theme = IconTheme::get_default();
				m_DefaultAlbumArt = theme->load_icon("audio-x-generic", m_AlbumArtSize, (IconLookupFlags) 0);
			}
			notify_notification_set_icon_from_pixbuf(m_pNotification, m_DefaultAlbumArt->gobj());
		}
	}
	catch (...)
	{
		if (!m_DefaultAlbumArt)
		{
			Glib::RefPtr<IconTheme> theme = IconTheme::get_default();
			m_DefaultAlbumArt = theme->load_icon("audio-x-generic", m_AlbumArtSize, (IconLookupFlags) 0);
		}
		notify_notification_set_icon_from_pixbuf(m_pNotification, m_DefaultAlbumArt->gobj());
	}

	if (!notify_notification_show(m_pNotification, nullptr))
	{
		log::error("Failed to send notification");
	}

	g_free(pArtist);
}

void NotificationPlugin::iconThemeChanged()
{
    Glib::RefPtr<IconTheme> theme = IconTheme::get_default();
    m_DefaultAlbumArt = theme->load_icon("audio-x-generic", m_AlbumArtSize, (IconLookupFlags) 0);
}

void NotificationPlugin::onActionCb(NotifyNotification* pNotification, gchar* pAction, gpointer pData)
{
    if (string(pAction) == "gtk-media-next-ltr")
    {
        log::debug("Skip track");
        NotificationPlugin* pPlugin = reinterpret_cast<NotificationPlugin*>(pData);
        pPlugin->m_pCore->next();
    }

    log::debug("Close notification");
    notify_notification_close(pNotification, nullptr);
}
