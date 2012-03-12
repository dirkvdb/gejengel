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

#include "mouseawaretreeview.h"

#include "utils/log.h"
#include "utils/numericoperations.h"

#include <gtkmm/treepath.h>

using namespace std;
using namespace utils;

namespace Gejengel
{

MouseAwareTreeView::MouseAwareTreeView()
: TreeView()
, m_CellChanged(false)
, m_CurrentCell(-1)
{
    add_events(Gdk::LEAVE_NOTIFY_MASK | Gdk::SCROLL_MASK);

    signal_map().connect(sigc::mem_fun(*this, &MouseAwareTreeView::redraw));
}

MouseInfo& MouseAwareTreeView::getMouseInfo()
{
    return m_MouseInfo;
}

void MouseAwareTreeView::redraw()
{
    m_MouseInfo.x = -1;
    m_MouseInfo.y = -1;
    m_MouseInfo.isOnButton = false;
    m_MouseInfo.isInRegion = false;
    m_MouseInfo.hasMoved = false;
    m_MouseInfo.isPressed = false;
    queue_draw();
}

bool MouseAwareTreeView::on_event(GdkEvent* pEvent)
{
    //Getting a button in a custom cellrenderer to react to mouse movements is messy :-)
    if (pEvent->type == GDK_MOTION_NOTIFY)
    {
        GdkEventMotion* pMotion = reinterpret_cast<GdkEventMotion*>(pEvent);
        queueDrawIfNeccesary(static_cast<int32_t>(pMotion->x), static_cast<int32_t>(pMotion->y));
        m_MouseInfo.hasMoved = !m_CellChanged;
    }
    else if (pEvent->type == GDK_LEAVE_NOTIFY)
    {
        m_MouseInfo.x = m_MouseInfo.y = -1;
        m_MouseInfo.hasMoved = false;
        queue_draw();
    }
    else if (pEvent->type == GDK_SCROLL)
    {
        m_MouseInfo.hasMoved = false;
        queue_draw();
    }

    return false;
}

bool MouseAwareTreeView::on_button_press_event(GdkEventButton* pEvent)
{
    if (pEvent->button == 1)
    {
        m_MouseInfo.isPressed = true;
        m_MouseInfo.hasMoved = false;
        if (m_MouseInfo.isInRegion)
        {
            if (queueDrawIfNeccesary(static_cast<int32_t>(pEvent->x), static_cast<int32_t>(pEvent->y), nullptr))
            {
                return true;
            }
        }
    }

    return TreeView::on_button_press_event(pEvent);
}

bool MouseAwareTreeView::on_button_release_event(GdkEventButton* pEvent)
{
    if (pEvent->button == 1)
    {
        m_MouseInfo.isPressed = false;
        m_MouseInfo.hasMoved = false;
        if (m_MouseInfo.isInRegion)
        {
            Glib::ustring viewPath;
            queueDrawIfNeccesary(static_cast<int32_t>(pEvent->x), static_cast<int32_t>(pEvent->y), &viewPath);
            if (m_MouseInfo.isOnButton && !viewPath.empty())
            {
                signalCellButtonClicked.emit(viewPath, m_MouseInfo.buttonName);
            }
        }
    }
    return TreeView::on_button_release_event(pEvent);
}

bool MouseAwareTreeView::queueDrawIfNeccesary(int32_t x, int32_t y, Glib::ustring* pPath)
{
    Gtk::TreeModel::Path mousePath;
    Gtk::TreeViewColumn* pColumn;
    Gdk::Rectangle rect;

    convert_bin_window_to_widget_coords (x, y, m_MouseInfo.x, m_MouseInfo.y);

    if (get_path_at_pos(x, y, mousePath, pColumn, x, y))
    {
        int32_t offsetX, offsetY;
        convert_bin_window_to_widget_coords(0, 0, offsetX, offsetY);

        m_MouseInfo.x -= offsetX;
        m_MouseInfo.y -= offsetY;

        get_cell_area(mousePath, *pColumn, rect);
        queue_draw_area(rect.get_x() + offsetX, rect.get_y() + offsetY, rect.get_width(), rect.get_height());
        if (rect.get_y() != m_CurrentCell)
        {
            m_CurrentCell = rect.get_y();
            m_CellChanged = true;
        }

        if (pPath)
        {
            *pPath = mousePath.to_string();
        }
        return true;
    }

    return false;
}

}
