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

#ifndef TRACK_VIEW_H
#define TRACK_VIEW_H

#include <gtkmm.h>

#include "signals.h"
#include "utils/types.h"

namespace Gejengel
{

class TrackModel;
class Settings;

class TrackView : public Gtk::ScrolledWindow
{
public:
    TrackView(TrackModel& trackModel, Settings& settings);
    ~TrackView();

    SignalTrackQueued signalTrackQueued;

private:
    void loadSettings();
    void saveSettings();

    void onRowActivated(const Gtk::TreeModel::Path& path, Gtk::TreeViewColumn* column);
    void onRowAdded(const Gtk::TreePath&, const Gtk::TreeIter&);
    void onRowDeleted(const Gtk::TreePath&);
    bool onButtonPress(GdkEventButton* pEvent);
    bool onKeyPress(GdkEventKey* pEvent);
    void onGetDragData(const Glib::RefPtr<Gdk::DragContext>&, Gtk::SelectionData& selection_data, guint, guint);
    void onDragBegin(const Glib::RefPtr<Gdk::DragContext>& context);
    void queueSelected();
    void queuePath(const Gtk::TreeModel::iterator& iter);
    void onHeaderEnable(int32_t headerId);
    static gboolean onHeaderClick(GtkWidget* pWidget, GdkEventButton* pEvent, gpointer pData);

    Gtk::TreeView   m_TreeView;
    TrackModel&     m_TrackModel;
    Gtk::Menu       m_PopupMenu;
    Gtk::Menu       m_HeaderMenu;
    Settings&       m_Settings;
    bool            m_Initialized;
};

}

#endif
