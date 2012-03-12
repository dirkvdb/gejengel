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

#ifndef SYSTEM_TRAY_H
#define SYSTEM_TRAY_H

#include <gtkmm.h>

#include "Core/gejengelplugin.h"
#include "dispatchedsubscriber.h"
#include "MusicLibrary/albumart.h"

namespace Gejengel
{
    class IGejengelCore;
}

class SystemTray 	: public Gejengel::GejengelPlugin
					, public Gejengel::IDispatchedSubscriber<Gejengel::AlbumArt>
{
public:
    SystemTray();
    ~SystemTray();

    bool initialize(Gejengel::IGejengelCore& core);

    std::string getName() const;
    std::string getDescription() const;

    void onPlay(const Gejengel::Track& track);
    void onPause();
    void onResume();
    void onStop();

    void onDispatchedItem(const Gejengel::AlbumArt& art, void* pData = nullptr);

    void destroy();
    
    void show();
    void hide();

private:
    void initPopupMenu();
    void playPause();
    void next();
    void quit();
    void onPopupMenu(guint button, guint32 activateTime);
    void onActivate();
    bool onButtonPress(GdkEventButton* pEvent);
    bool onScroll(GdkEventScroll* pEvent);
    static gboolean onTooltip(GtkStatusIcon* pStatusIcon, gint x, gint y, gboolean keyboardMode, GtkTooltip* pTooltip, gpointer pData);

    Gejengel::IGejengelCore*        m_pCore;
    Gtk::Menu*                      m_pMenu;
    Glib::RefPtr<Gtk::StatusIcon>   m_StatusIcon;
    Glib::RefPtr<Gtk::UIManager>    m_UIManager;
    Glib::RefPtr<Gtk::ActionGroup>  m_ActionGroup;
    Glib::RefPtr<Gdk::Pixbuf>       m_DefaultAlbumArt;
    Glib::RefPtr<Gdk::Pixbuf>       m_AlbumArt;
    Glib::ustring                   m_TooltipText;
};

#endif
