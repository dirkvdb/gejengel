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

#ifndef MOUSE_AWARE_TREEVIEW
#define MOUSE_AWARE_TREEVIEW

#include <gtkmm.h>

#include "signals.h"

namespace Gejengel
{

struct MouseInfo
{
    MouseInfo(int32_t posX = 0, int32_t posY = 0) : x(posX), y(posY), isOnButton(false), isInRegion(false), hasMoved(false), isPressed(false) {}
    int32_t x;
    int32_t y;
    bool isOnButton;
    bool isInRegion;
    bool hasMoved;
    bool isPressed;
    Glib::ustring buttonName;
};

class MouseAwareTreeView : public Gtk::TreeView
{
public:
    MouseAwareTreeView();
    MouseInfo& getMouseInfo();

    SignalCellButtonClicked signalCellButtonClicked;
protected:
    bool on_event(GdkEvent* pEvent);
    bool on_button_press_event(GdkEventButton* event);
    bool on_button_release_event(GdkEventButton* event);
private:
    bool queueDrawIfNeccesary(int32_t x, int32_t y, Glib::ustring* pPath = nullptr);
    void redraw();

    MouseInfo       m_MouseInfo;
    bool            m_CellChanged;
    int32_t         m_CurrentCell;
};

}

#endif
