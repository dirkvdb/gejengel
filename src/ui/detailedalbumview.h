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

#ifndef DETAILED_ALBUM_VIEW_H
#define DETAILED_ALBUM_VIEW_H


#include <gtkmm.h>

#include "mouseawaretreeview.h"
#include "signals.h"
#include "cellrendereralbum.h"
#include "utils/types.h"

namespace Gejengel
{

class Settings;
class AlbumModel;

class DetailedAlbumView : public Gtk::ScrolledWindow
{
public:
    DetailedAlbumView(Settings& settings, AlbumModel& albumModel, bool showInfoButton);
    ~DetailedAlbumView();

    SignalAlbumChanged signalAlbumChanged;
    SignalAlbumQueued signalAlbumQueued;
    SignalAlbumInfoRequested signalAlbumInfoRequested;

private:
    void saveSettings();
    void loadSettings();

    void onSelectionChanged();
    void onRowActivated(const Gtk::TreeModel::Path& rowPath, Gtk::TreeViewColumn* column);
    void onCellButtonClicked(const Glib::ustring& rowPath, const Glib::ustring& buttonName);
    void onLargeView();
    void onSmallView();
    void onSortChanged();
    void onGetDragData(const Glib::RefPtr<Gdk::DragContext>&, Gtk::SelectionData& selection_data, guint, guint);
    bool onButtonPress(GdkEventButton* pEvent);

    Settings&                           m_Settings;
    MouseAwareTreeView                  m_TreeView;
    AlbumModel&                         m_AlbumModel;
    Gtk::Menu                           m_PopupMenu;
    CellRendererAlbum                   m_CellRenderer;
    Gtk::RadioMenuItem::Group           m_ViewRadioGroup;
    Gtk::RadioMenuItem::Group           m_SortRadioGroup;
    Gtk::Menu_Helpers::RadioMenuElem    m_LargeViewItem;
    Gtk::Menu_Helpers::RadioMenuElem    m_SmallViewItem;
    Gtk::Menu_Helpers::RadioMenuElem    m_ArtistSortItem;
    Gtk::Menu_Helpers::RadioMenuElem    m_TitleSortItem;
    Gtk::Menu_Helpers::RadioMenuElem    m_DateSortItem;
    int32_t                             m_SortColumn;
};

}

#endif
