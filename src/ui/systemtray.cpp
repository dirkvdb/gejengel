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

#include "systemtray.h"

#include <cassert>
#include <glibmm/i18n.h>

#include "config.h"

#ifdef WIN32
#include "winconfig.h"
#endif

#include "Core/gejengelcore.h"
#include "Core/albumartprovider.h"
#include "ui/sharedfunctions.h"
#include "utils/log.h"
#include "utils/trace.h"
#include "utils/types.h"
#include "MusicLibrary/album.h"

static const int32_t ICON_SIZE = 38;

using namespace std;
using namespace Gtk;
using namespace utils;

SystemTray::SystemTray()
: m_pCore(nullptr)
, m_pMenu(nullptr)
{
    utils::trace("Create SystemTray");
    initPopupMenu();

    log::debug("GTKMM Major", GTKMM_MAJOR_VERSION, "Minor", GTKMM_MINOR_VERSION, "Micro", GTKMM_MICRO_VERSION);
}

SystemTray::~SystemTray()
{
    destroy();
}

bool SystemTray::initialize(Gejengel::IGejengelCore& core)
{
    utils::trace("Init SystemTray");
    m_pCore = &core;
    m_StatusIcon = StatusIcon::create(Stock::CDROM);
    m_TooltipText = PACKAGE_NAME;
    m_StatusIcon->set_tooltip_text(PACKAGE_NAME);

#if (GTKMM_MAJOR_VERSION == 2 && GTKMM_MINOR_VERSION >= 16) || GTKMM_MAJOR_VERSION > 2
    m_StatusIcon->signal_button_press_event().connect(sigc::mem_fun(*this, &SystemTray::onButtonPress));
    m_StatusIcon->signal_scroll_event().connect(sigc::mem_fun(*this, &SystemTray::onScroll));
#endif

    m_StatusIcon->signal_activate().connect(sigc::mem_fun(*this, &SystemTray::onActivate));
    m_StatusIcon->signal_popup_menu().connect(sigc::mem_fun(*this, &SystemTray::onPopupMenu));
    g_signal_connect(G_OBJECT(m_StatusIcon->gobj()), "query-tooltip", GTK_SIGNAL_FUNC(SystemTray::onTooltip), this);

    return true;
}

std::string SystemTray::getName() const
{
    return _("System tray");
}

std::string SystemTray::getDescription() const
{
    return _("Put an icon in the system tray");
}

void SystemTray::onPlay(const Gejengel::Track& track)
{
    try
    {
        stringstream ss;
        ss << "<b>" << Glib::Markup::escape_text(track.artist) << "</b>" << endl << Glib::Markup::escape_text(track.title);

        m_TooltipText = ss.str();
        m_ActionGroup->get_action("ContextPlay")->property_stock_id() = Stock::MEDIA_PAUSE;

        m_pCore->getAlbumArtProvider().getAlbumArt(track, *this);
    }
    catch (std::exception& e)
    {
        log::error("SystemTray: Failed to get album:", e.what());
    }
}

void SystemTray::onPause()
{
    m_ActionGroup->get_action("ContextPlay")->property_stock_id() = Stock::MEDIA_PLAY;
}

void SystemTray::onResume()
{
    m_ActionGroup->get_action("ContextPlay")->property_stock_id() = Stock::MEDIA_PAUSE;
}

void SystemTray::onStop()
{
    m_TooltipText = PACKAGE_NAME;
    m_ActionGroup->get_action("ContextPlay")->property_stock_id() = Stock::MEDIA_PLAY;
}

void SystemTray::onDispatchedItem(const Gejengel::AlbumArt& art, void* pData)
{
	if (art.getDataSize() > 0)
	{
		try
		{
			m_AlbumArt = Gejengel::Shared::createCoverPixBufWithOverlay(art, ICON_SIZE);
		}
		catch (Gdk::PixbufError&)
		{
			m_AlbumArt = Glib::RefPtr<Gdk::Pixbuf>();
		}
	}
	else
	{
		m_AlbumArt = Glib::RefPtr<Gdk::Pixbuf>();
	}
}

void SystemTray::destroy()
{
    hide();
    m_StatusIcon.reset();
}

void SystemTray::show()
{
    if (m_StatusIcon)
    {
        m_StatusIcon->set_visible(true);
    }
}

void SystemTray::hide()
{
    if (m_StatusIcon)
    {
        m_StatusIcon->set_visible(false);
    }
}

void SystemTray::onPopupMenu(guint button, guint32 activateTime)
{
    m_pMenu->popup(button, activateTime);
}

void SystemTray::onActivate()
{
    m_pCore->showHideWindow();
}

bool SystemTray::onButtonPress(GdkEventButton* pEvent)
{
    if (pEvent->button == 2) //middle mouse pressed
    {
        playPause();
    }
    else if (pEvent->button == 9) //forward button
    {
        m_pCore->next();
    }

    return false;
}

bool SystemTray::onScroll(GdkEventScroll* pEvent)
{
    if (pEvent->direction == GDK_SCROLL_UP)
    {
        m_pCore->setVolume(m_pCore->getVolume() + 10);
    }
    else if (pEvent->direction == GDK_SCROLL_DOWN)
    {
        m_pCore->setVolume(m_pCore->getVolume() - 10);
    }

    return false;
}

gboolean SystemTray::onTooltip(GtkStatusIcon* pStatusIcon, gint x, gint y, gboolean keyboardMode, GtkTooltip* pTooltip, gpointer pData)
{
    SystemTray* pTray = reinterpret_cast<SystemTray*>(pData);

    gtk_tooltip_set_markup(pTooltip, pTray->m_TooltipText.c_str());

    if (pTray->m_AlbumArt)
    {
        gtk_tooltip_set_icon(pTooltip, pTray->m_AlbumArt->gobj());
    }
    else
    {
        if (!pTray->m_DefaultAlbumArt)
        {
            Glib::RefPtr<IconTheme> theme = IconTheme::get_default();
            pTray->m_DefaultAlbumArt = theme->load_icon("audio-x-generic", ICON_SIZE, IconLookupFlags(0));
        }

        if (pTray->m_DefaultAlbumArt)
        {
            gtk_tooltip_set_icon(pTooltip, pTray->m_DefaultAlbumArt->gobj());
        }
    }

    return TRUE;
}

void SystemTray::playPause()
{
    m_pCore->getPlaybackState() == Gejengel::Paused ? m_pCore->resume() : m_pCore->pause();
}

void SystemTray::next()
{
    m_pCore->next();
}

void SystemTray::quit()
{
    m_pCore->quitApplication();
}

void SystemTray::initPopupMenu()
{
    m_ActionGroup = Gtk::ActionGroup::create();

    m_ActionGroup->add(Gtk::Action::create("ContextMenu", "Context Menu"));
    m_ActionGroup->add(Gtk::Action::create("ContextPlay", Stock::MEDIA_PLAY), sigc::mem_fun(*this, &SystemTray::playPause));
    m_ActionGroup->add(Gtk::Action::create("ContextNext", Stock::MEDIA_NEXT), sigc::mem_fun(*this, &SystemTray::next));
    m_ActionGroup->add(Gtk::Action::create("ContextQuit", Stock::QUIT), sigc::mem_fun(*this, &SystemTray::quit));

    m_UIManager = Gtk::UIManager::create();
    m_UIManager->insert_action_group(m_ActionGroup);

    Glib::ustring uiInfo =
        "<ui>"
        "  <popup name='PopupMenu'>"
        "      <menuitem action='ContextPlay'/>"
        "      <menuitem action='ContextNext'/>"
        "      <separator/>"
        "      <menuitem action='ContextQuit'/>"
        "  </popup>"
        "</ui>";

    try
    {
        m_UIManager->add_ui_from_string(uiInfo);
    }
    catch(const Glib::Error& ex)
    {
        log::error("Building menus failed:", ex.what());
    }

    m_pMenu = dynamic_cast<Gtk::Menu*>(m_UIManager->get_widget("/PopupMenu"));
    if(!m_pMenu)
    {
        log::warn("System tray: PopupMenu not found");
    }
}
