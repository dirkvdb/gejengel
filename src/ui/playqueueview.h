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

#ifndef PLAY_QUEUE_VIEW_H
#define PLAY_QUEUE_VIEW_H

#include <gtkmm.h>

#include "utils/types.h"
#include "playqueuemodel.h"
#include "mouseawaretreeview.h"
#include "cellrendererhoverbutton.h"

namespace Gejengel
{

class Track;

class PlayQueueView : public Gtk::ScrolledWindow
{
public:
    PlayQueueView(PlayQueueModel& model);

private:
    static uint32_t pathToIndex(Gtk::TreePath path);
    uint32_t determineDropIndex(int32_t x, int32_t y);

    bool onKeyPress(GdkEventKey* pEvent);
    bool onButtonPress(GdkEventButton* event);
    void onGetDragData(const Glib::RefPtr<Gdk::DragContext>&, Gtk::SelectionData& selection_data, guint, guint);
    void onTrackUnQueued(const Glib::ustring& path, const Glib::ustring& buttonName);
    void onModelUpdated();
    void onDrop(const Glib::RefPtr<Gdk::DragContext>& context, int32_t x, int32_t y, const Gtk::SelectionData& selectionData, guint info, guint time);
    void removeSelectedRows();
    void removeAllRows();
    bool handleInternalDrop(const Glib::RefPtr<Gdk::DragContext>& context, const Gtk::SelectionData& selectionData, int32_t x, int32_t y);
    bool handleExternalDrop(const Glib::RefPtr<Gdk::DragContext>& context, const Gtk::SelectionData& selectionData, int32_t x, int32_t y);

    MouseAwareTreeView      m_TreeView;
    PlayQueueModel&         m_Model;
    PlayQueue&              m_PlayQueue;
    Gtk::Menu               m_PopupMenu;
    CellRendererHoverButton m_CellRenderer;
    Gtk::TreeViewColumn     m_QueueColumn;
};

}

#endif
