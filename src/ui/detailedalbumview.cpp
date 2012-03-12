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

#include "detailedalbumview.h"

#include <cassert>
#include <ostream>
#include <glibmm/ustring.h>
#include <glibmm/i18n.h>
#include <gtkmm/menu_elems.h>
#include <gtkmm/treeviewcolumn.h>
#include <gtkmm/radiobuttongroup.h>

#include "albummodel.h"
#include "utils/log.h"
#include "utils/trace.h"
#include "Core/settings.h"

using namespace std;
using namespace Gtk;
using namespace utils;

namespace Gejengel
{

DetailedAlbumView::DetailedAlbumView(Settings& settings, AlbumModel& albumModel, bool showInfoButton)
: ScrolledWindow()
, m_Settings(settings)
, m_AlbumModel(albumModel)
, m_CellRenderer(albumModel, m_TreeView.getMouseInfo(), showInfoButton)
, m_LargeViewItem(m_ViewRadioGroup, _("Large"), sigc::mem_fun(*this, &DetailedAlbumView::onLargeView))
, m_SmallViewItem(m_ViewRadioGroup, _("Small"), sigc::mem_fun(*this, &DetailedAlbumView::onSmallView))
, m_ArtistSortItem(m_SortRadioGroup, _("Artist"), sigc::mem_fun(*this, &DetailedAlbumView::onSortChanged))
, m_TitleSortItem(m_SortRadioGroup, _("Title"), sigc::mem_fun(*this, &DetailedAlbumView::onSortChanged))
, m_DateSortItem(m_SortRadioGroup, _("Date added"), sigc::mem_fun(*this, &DetailedAlbumView::onSortChanged))
, m_SortColumn(-1)
{
    utils::trace("Create Detailed album view");
    set_shadow_type(Gtk::SHADOW_IN);

    m_TreeView.set_model(albumModel.getStore());
    m_TreeView.set_rules_hint(true);

    std::vector<Gtk::TargetEntry> targets;
    targets.push_back(Gtk::TargetEntry("PlayQueue", Gtk::TARGET_SAME_APP | Gtk::TARGET_OTHER_WIDGET));
    m_TreeView.enable_model_drag_source(targets, Gdk::MODIFIER_MASK, Gdk::ACTION_COPY);

    m_TreeView.signalCellButtonClicked.connect(sigc::mem_fun(*this, &DetailedAlbumView::onCellButtonClicked));
    m_TreeView.signal_drag_data_get().connect(sigc::mem_fun(*this, &DetailedAlbumView::onGetDragData));

    TreeViewColumn* column = new TreeViewColumn(_("Albums"), m_CellRenderer);
    m_TreeView.append_column(*Gtk::manage(column));
    column->add_attribute(m_CellRenderer.property_album_art(), m_AlbumModel.columns().albumArt);
    column->add_attribute(m_CellRenderer.property_title(), m_AlbumModel.columns().title);
    column->add_attribute(m_CellRenderer.property_artist(), m_AlbumModel.columns().artist);
    column->add_attribute(m_CellRenderer.property_year(), m_AlbumModel.columns().year);
    column->add_attribute(m_CellRenderer.property_genre(), m_AlbumModel.columns().genre);
    column->add_attribute(m_CellRenderer.property_id(), m_AlbumModel.columns().id);

    {
        Gtk::Menu::MenuList& menuList = m_PopupMenu.items();

        menuList.push_back(Gtk::Menu_Helpers::MenuElem(_("View")));
        Gtk::MenuItem* pMenuItem = &menuList.back();

        Gtk::Menu* pSubMenu = Gtk::manage(new Gtk::Menu());
        pMenuItem->set_submenu(*pSubMenu);

        Gtk::Menu_Helpers::MenuList items = pSubMenu->items();
        items.push_back(m_LargeViewItem);
        items.push_back(m_SmallViewItem);

        menuList.push_back(Gtk::Menu_Helpers::MenuElem(_("Sort by")));
        pMenuItem = &menuList.back();

        pSubMenu = Gtk::manage(new Gtk::Menu());
        pMenuItem->set_submenu(*pSubMenu);

        items = pSubMenu->items();
        items.push_back(m_TitleSortItem);
        items.push_back(m_ArtistSortItem);
        items.push_back(m_DateSortItem);
    }
    m_PopupMenu.accelerate(*this);

    //only show scrollbars when needed
    set_policy(Gtk::POLICY_NEVER, Gtk::POLICY_AUTOMATIC);

    m_TreeView.get_selection()->signal_changed().connect(sigc::mem_fun(*this, &DetailedAlbumView::onSelectionChanged));
    m_TreeView.signal_row_activated().connect(sigc::mem_fun(*this, &DetailedAlbumView::onRowActivated));
    m_TreeView.signal_button_press_event().connect(sigc::mem_fun(*this, &DetailedAlbumView::onButtonPress), false);

    loadSettings();

    add(m_TreeView);
    show_all_children();
}

DetailedAlbumView::~DetailedAlbumView()
{
    saveSettings();
}

void DetailedAlbumView::loadSettings()
{
    CellRendererAlbum::Size size = static_cast<CellRendererAlbum::Size>(m_Settings.getAsInt("DetailedAlbumViewSize", static_cast<int>(CellRendererAlbum::Large)));
    m_CellRenderer.setSize(size);

    if (size == CellRendererAlbum::Small)
    {
        Glib::RefPtr<Gtk::RadioMenuItem> item = Glib::RefPtr<Gtk::RadioMenuItem>::cast_dynamic(m_SmallViewItem.get_child());
        assert(item);
        item->set_active(true);
        onSmallView();
    }
    else
    {
        Glib::RefPtr<Gtk::RadioMenuItem> item = Glib::RefPtr<Gtk::RadioMenuItem>::cast_dynamic(m_LargeViewItem.get_child());
        assert(item);
        item->set_active(true);
        onLargeView();
    }

    m_SortColumn = m_Settings.getAsInt("DetailedAlbumViewSortColumn");

    if (m_SortColumn == m_AlbumModel.columns().artist.index())
    {
        Glib::RefPtr<Gtk::RadioMenuItem> item = Glib::RefPtr<Gtk::RadioMenuItem>::cast_dynamic(m_ArtistSortItem.get_child());
        if (item)
        {
            item->set_active(true);
        }
    }
    else if (m_SortColumn == m_AlbumModel.columns().dateAdded.index())
    {
        Glib::RefPtr<Gtk::RadioMenuItem> item = Glib::RefPtr<Gtk::RadioMenuItem>::cast_dynamic(m_DateSortItem.get_child());
        if (item)
        {
            item->set_active(true);
        }
    }
    else
    {
        Glib::RefPtr<Gtk::RadioMenuItem> item = Glib::RefPtr<Gtk::RadioMenuItem>::cast_dynamic(m_TitleSortItem.get_child());
        if (item)
        {
            item->set_active(true);
        }
    }

    Gtk::SortType type = static_cast<Gtk::SortType>(m_Settings.getAsInt("DetailedAlbumViewSortColumnDir"));

    m_TreeView.get_column(0)->set_sort_column(m_SortColumn);
    m_AlbumModel.setSortColumn(m_SortColumn, type);
}

void DetailedAlbumView::saveSettings()
{
    m_Settings.set("DetailedAlbumViewSize",  static_cast<int>(m_CellRenderer.getSize()));
    m_Settings.set("DetailedAlbumViewSortColumn", m_TreeView.get_column(0)->get_sort_column_id());
    m_Settings.set("DetailedAlbumViewSortColumnDir", static_cast<int>(m_TreeView.get_column(0)->get_sort_order()));
}

void DetailedAlbumView::onSelectionChanged()
{
    Gtk::TreeModel::iterator iter = m_TreeView.get_selection()->get_selected();
    if (iter)
    {
        signalAlbumChanged.emit((*iter)[m_AlbumModel.columns().id]);
    }
}

void DetailedAlbumView::onRowActivated(const Gtk::TreeModel::Path& rowPath, Gtk::TreeViewColumn* column)
{
    signalAlbumQueued.emit((*(m_AlbumModel.getStore()->get_iter(rowPath)))[m_AlbumModel.columns().id], -1);
}

void DetailedAlbumView::onCellButtonClicked(const Glib::ustring& rowPath, const Glib::ustring& buttonName)
{
    if (buttonName == CellRendererAlbum::QueueAlbum)
    {
        log::info("Queue album:", rowPath);
        signalAlbumQueued.emit((*(m_AlbumModel.getStore()->get_iter(Gtk::TreePath(rowPath))))[m_AlbumModel.columns().id], -1);
    }
    else if (buttonName == CellRendererAlbum::ShowTrackInfo)
    {
        signalAlbumInfoRequested.emit((*(m_AlbumModel.getStore()->get_iter(Gtk::TreePath(rowPath))))[m_AlbumModel.columns().id]);
    }
}

void DetailedAlbumView::onLargeView()
{
    Glib::RefPtr<Gtk::RadioMenuItem> item = Glib::RefPtr<Gtk::RadioMenuItem>::cast_dynamic(m_LargeViewItem.get_child());
    if (item->get_active())
    {
        m_AlbumModel.setAlbumArtSize(CellRendererAlbum::Large);
        m_CellRenderer.setSize(CellRendererAlbum::Large);
        m_TreeView.queue_draw();
    }
}

void DetailedAlbumView::onSmallView()
{
    Glib::RefPtr<Gtk::RadioMenuItem> item = Glib::RefPtr<Gtk::RadioMenuItem>::cast_dynamic(m_SmallViewItem.get_child());
    if (item->get_active())
    {
        m_AlbumModel.setAlbumArtSize(CellRendererAlbum::Small);
        m_CellRenderer.setSize(CellRendererAlbum::Small);
        m_TreeView.queue_draw();
    }
}

void DetailedAlbumView::onSortChanged()
{
    int sortId = -1;
    Glib::RefPtr<Gtk::RadioMenuItem> item = Glib::RefPtr<Gtk::RadioMenuItem>::cast_dynamic(m_TitleSortItem.get_child());
    if (item && item->get_active() && m_SortColumn != m_AlbumModel.columns().title.index())
    {
        sortId = m_AlbumModel.columns().title.index();
        goto finish;
    }

    item = Glib::RefPtr<Gtk::RadioMenuItem>::cast_dynamic(m_ArtistSortItem.get_child());
    if (item && item->get_active() && m_SortColumn != m_AlbumModel.columns().artist.index())
    {
        sortId = m_AlbumModel.columns().artist.index();
        goto finish;
    }

    item = Glib::RefPtr<Gtk::RadioMenuItem>::cast_dynamic(m_DateSortItem.get_child());
    if (item && item->get_active() && m_SortColumn != m_AlbumModel.columns().dateAdded.index())
    {
        sortId = m_AlbumModel.columns().dateAdded.index();
        goto finish;
    }

finish:
    if (sortId >= 0)
    {
        m_TreeView.get_column(0)->set_sort_column(sortId);
        m_AlbumModel.setSortColumn(sortId, m_TreeView.get_column(0)->get_sort_order());
        m_SortColumn = sortId;
    }
}

bool DetailedAlbumView::onButtonPress(GdkEventButton* pEvent)
{
    if ((pEvent->type == GDK_BUTTON_PRESS) && (pEvent->button == 3))
    {
        m_PopupMenu.popup(pEvent->button, pEvent->time);
    }

    return false;
}

void DetailedAlbumView::onGetDragData(const Glib::RefPtr<Gdk::DragContext>&, Gtk::SelectionData& selection_data, guint, guint)
{
    Gtk::TreeModel::Path path = *(m_TreeView.get_selection()->get_selected_rows().begin());
    Gtk::TreeModel::iterator iter = m_AlbumModel.getStore()->get_iter(path);
    const std::string& trackId = (*iter)[m_AlbumModel.columns().id];
    
    selection_data.set(selection_data.get_target(), "1" + trackId);
}

}
