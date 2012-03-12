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

#include "albumview.h"

#include <ostream>
#include <glibmm/ustring.h>
#include <glibmm/i18n.h>

#include "albummodel.h"
#include "Core/settings.h"
#include "utils/log.h"
#include "utils/trace.h"
#include "utils/types.h"

using namespace std;

namespace Gejengel
{

AlbumView::AlbumView(Settings& settings, AlbumModel& albumModel)
: ScrolledWindow()
, m_Settings(settings)
, m_AlbumModel(albumModel)
{
    utils::trace("Create Album view");
    set_shadow_type(Gtk::SHADOW_IN);

    m_TreeView.set_model(albumModel.getStore());
    m_TreeView.set_rules_hint(true);

    m_TreeView.append_column(_("Album"), m_AlbumModel.columns().title);
    m_TreeView.append_column(_("Artist"), m_AlbumModel.columns().artist);

    m_TreeView.get_column(0)->set_sort_column(m_AlbumModel.columns().title);
    m_TreeView.get_column(1)->set_sort_column(m_AlbumModel.columns().artist);

    m_TreeView.get_column(0)->set_resizable(true);
    m_TreeView.get_column(1)->set_resizable(true);

    add(m_TreeView);
    //only show scrollbars when needed
    set_policy(Gtk::POLICY_AUTOMATIC, Gtk::POLICY_AUTOMATIC);

    m_TreeView.get_selection()->signal_changed().connect(sigc::mem_fun(*this, &AlbumView::onSelectionChanged));
    m_TreeView.signal_row_activated().connect(sigc::mem_fun(*this, &AlbumView::onRowActivated));

    loadSettings();

    show_all_children();
}


AlbumView::~AlbumView()
{
    saveSettings();
}

void AlbumView::loadSettings()
{
    int32_t id = m_Settings.getAsInt("AlbumViewSortColumn", 1);
    Gtk::SortType type = static_cast<Gtk::SortType>(m_Settings.getAsInt("AlbumViewSortColumnDir", Gtk::SORT_ASCENDING));

    if (id == Gtk::TreeSortable::DEFAULT_SORT_COLUMN_ID)
    {
        id = 1;
        type = Gtk::SORT_ASCENDING;
    }
    
    m_AlbumModel.setSortColumn(id, type);
    m_TreeView.get_column(id - 1)->set_sort_order(type);
}

void AlbumView::saveSettings()
{
    int32_t id;
    Gtk::SortType type;
    m_AlbumModel.getSortColumn(id, type);

    m_Settings.set("AlbumViewSortColumn", id);
    m_Settings.set("AlbumViewSortColumnDir", static_cast<int32_t>(type));
}

void AlbumView::onSelectionChanged()
{
    Gtk::TreeModel::iterator iter = m_TreeView.get_selection()->get_selected();
    if (iter)
    {
        signalAlbumChanged.emit((*iter)[m_AlbumModel.columns().id]);
    }
}

void AlbumView::onRowActivated(const Gtk::TreeModel::Path& slectedPath, Gtk::TreeViewColumn* column)
{
    signalAlbumQueued.emit((*(m_AlbumModel.getStore()->get_iter(slectedPath)))[m_AlbumModel.columns().id], -1);
}

}
