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

#include "playqueueview.h"

#include <cassert>
#include <glibmm/i18n.h>

#include "sharedfunctions.h"
#include "MusicLibrary/track.h"
#include "utils/log.h"
#include "utils/trace.h"
#include "utils/stringoperations.h"
#include "Core/settings.h"

using namespace Gtk;
using namespace utils;

namespace Gejengel
{

PlayQueueView::PlayQueueView(PlayQueueModel& model)
: m_Model(model)
, m_PlayQueue(model.getQueue())
, m_CellRenderer(24, m_TreeView.getMouseInfo())
, m_QueueColumn(_("Play queue"), m_CellRenderer)
{
    utils::trace("Create PlayQueue view");
    set_shadow_type(SHADOW_IN);
    set_policy(POLICY_NEVER, POLICY_AUTOMATIC);

    m_TreeView.set_rules_hint(true);
    m_TreeView.get_selection()->set_mode(SELECTION_MULTIPLE);
    m_TreeView.set_model(m_Model.getStore());

    std::vector<Gtk::TargetEntry> dropTargets;
    dropTargets.push_back(Gtk::TargetEntry("PlayQueue", Gtk::TARGET_SAME_APP));
    dropTargets.push_back(Gtk::TargetEntry("PlayQueuePath", Gtk::TARGET_SAME_WIDGET));
    m_TreeView.enable_model_drag_dest(dropTargets, Gdk::ACTION_COPY);

    std::vector<Gtk::TargetEntry> dragTargets;
    dragTargets.push_back(Gtk::TargetEntry("PlayQueuePath", Gtk::TARGET_SAME_WIDGET));
    m_TreeView.enable_model_drag_source(dragTargets, Gdk::MODIFIER_MASK, Gdk::ACTION_MOVE);

    m_CellRenderer.property_icon_border() = true;
    m_CellRenderer.setHoverIcon("list-remove");
    m_CellRenderer.setDefaultIcon("audio-x-generic");
    m_CellRenderer.setLine1Markup(CellRendererHoverButton::Small);
    m_CellRenderer.setLine2Markup(CellRendererHoverButton::Small);

    m_TreeView.append_column(m_QueueColumn);
    m_QueueColumn.add_attribute(m_CellRenderer.property_icon(), m_Model.columns().albumArt);
    m_QueueColumn.add_attribute(m_CellRenderer.property_line1(), m_Model.columns().artist);
    m_QueueColumn.add_attribute(m_CellRenderer.property_line2(), m_Model.columns().title);
    m_QueueColumn.add_attribute(m_CellRenderer.property_bginfo(), m_Model.columns().duration);

    {
        Gtk::Menu::MenuList& menulist = m_PopupMenu.items();
        menulist.push_back(Gtk::Menu_Helpers::StockMenuElem(Stock::DELETE, sigc::mem_fun(*this, &PlayQueueView::removeSelectedRows)));
        menulist.push_back(Gtk::Menu_Helpers::StockMenuElem(Stock::CLEAR, sigc::mem_fun(*this, &PlayQueueView::removeAllRows)));
    }
    m_PopupMenu.accelerate(*this);

    m_TreeView.signal_key_press_event().connect(sigc::mem_fun(*this, &PlayQueueView::onKeyPress), false);
    m_TreeView.signal_button_press_event().connect(sigc::mem_fun(*this, &PlayQueueView::onButtonPress), false);
    m_TreeView.signal_drag_data_get().connect(sigc::mem_fun(*this, &PlayQueueView::onGetDragData));
    m_TreeView.signal_drag_data_received().connect(sigc::mem_fun(*this, &PlayQueueView::onDrop));


    m_TreeView.signalCellButtonClicked.connect(sigc::mem_fun(*this, &PlayQueueView::onTrackUnQueued));

    m_Model.signalModelUpdated.connect(sigc::mem_fun(*this, &PlayQueueView::onModelUpdated));

    add(m_TreeView);
    show_all_children();
}

bool PlayQueueView::onKeyPress(GdkEventKey* pEvent)
{
    switch (pEvent->keyval)
    {
    case GDK_KP_Delete:
    case GDK_Delete:
        removeSelectedRows();
        return true;
    default:
        return false;
    }
}

bool PlayQueueView::onButtonPress(GdkEventButton* pEvent)
{
    if ((pEvent->type == GDK_BUTTON_PRESS) && (pEvent->button == 3))
    {
        m_PopupMenu.popup(pEvent->button, pEvent->time);

        TreeModel::Path mousePath;
        TreeViewColumn* pCol;
        int x, y;
        if (m_TreeView.get_path_at_pos(static_cast<int32_t>(pEvent->x), static_cast<int32_t>(pEvent->y), mousePath, pCol, x, y))
        {
            if (m_TreeView.get_selection()->is_selected(mousePath))
            {
                return true;
            }
        }
    }

    return false;
}

void PlayQueueView::onTrackUnQueued(const Glib::ustring& trackPath, const Glib::ustring& buttonName)
{
    assert(buttonName == "unqueue");
    uint32_t index = stringops::toNumeric<uint32_t>(trackPath);

    m_PlayQueue.removeTrack(index);
    m_TreeView.columns_autosize();
}

void PlayQueueView::onModelUpdated()
{
    uint32_t queueLength = m_Model.getQueueLength();

    if (queueLength != 0)
    {
        Glib::ustring duration;
        Shared::durationToString(queueLength, duration);
        m_QueueColumn.set_title(_("Play queue") + Glib::ustring(" (") + duration + ")");
    }
    else
    {
        m_QueueColumn.set_title(_("Play queue"));
    }
}

void PlayQueueView::removeSelectedRows()
{
    std::vector<uint32_t> indexes;

    Glib::RefPtr<TreeSelection> selection = m_TreeView.get_selection();
    TreeSelection::ListHandle_Path rows = selection->get_selected_rows();

    for (TreeSelection::ListHandle_Path::const_iterator iter = rows.begin(); iter != rows.end(); ++iter)
    {
        uint32_t index = stringops::toNumeric<uint32_t>((*iter).to_string());
        indexes.push_back(index);
    }

    m_PlayQueue.removeTracks(indexes);

    m_TreeView.columns_autosize();
}

void PlayQueueView::removeAllRows()
{
    m_PlayQueue.clear();
    m_TreeView.columns_autosize();
}

void PlayQueueView::onGetDragData(const Glib::RefPtr<Gdk::DragContext>&, Gtk::SelectionData& selection_data, guint, guint)
{
    Gtk::TreeModel::Path path = *(m_TreeView.get_selection()->get_selected_rows().begin());
    selection_data.set("PlayQueuePath", std::string(path.to_string()));
}

void PlayQueueView::onDrop(const Glib::RefPtr<Gdk::DragContext>& context, int32_t x, int32_t y, const Gtk::SelectionData& selectionData, guint info, guint time)
{
    bool accept = false;

    Gdk::ListHandle_AtomString targets = context->get_targets();
    for(Gdk::ListHandle_AtomString::const_iterator iter = targets.begin(); iter != targets.end(); ++iter)
    {
        std::string atom(*iter);
        if (atom == "PlayQueue")
        {
            accept = handleExternalDrop(context, selectionData, x, y);
            break;
        }
        else if (atom == "PlayQueuePath")
        {
            accept = handleInternalDrop(context, selectionData, x, y);
            break;
        }
    }

    if (accept)
    {
        context->drag_finish(true, false, time);
    }
    else
    {
        context->drag_refuse(time);
    }
}

bool PlayQueueView::handleInternalDrop(const Glib::RefPtr<Gdk::DragContext>& context, const Gtk::SelectionData& selectionData, int x, int y)
{
    Glib::ustring sourcePath = selectionData.get_data_as_string();

    if (sourcePath.empty())
    {
        return false;
    }

    uint32_t sourceIndex = stringops::toNumeric<uint32_t>(sourcePath);
    uint32_t dropIndex = determineDropIndex(x, y);

    if (sourceIndex < dropIndex)
    {
        --dropIndex;
    }

    m_PlayQueue.moveTrack(sourceIndex, dropIndex);

    return true;
}

bool PlayQueueView::handleExternalDrop(const Glib::RefPtr<Gdk::DragContext>& context, const Gtk::SelectionData& selectionData, int x, int y)
{
    std::string data = selectionData.get_data_as_string();
    uint32_t dropIndex = determineDropIndex(x, y);

    if (data[0] == '0')
    {
        m_PlayQueue.queueTrack(data.substr(1), dropIndex);
    }
    else if (data[0] == '1')
    {
        m_PlayQueue.queueAlbum(data.substr(1), dropIndex);
    }
    else
    {
        assert(false && "Invalid type in drag data");
    }

    return true;
}

uint32_t PlayQueueView::determineDropIndex(int32_t x, int32_t y)
{
    uint32_t dropIndex = -1;

    Gtk::TreeModel::Path dropPath;
    Gtk::TreeViewDropPosition dropPosition;
    if (m_TreeView.get_dest_row_at_pos(x, y, dropPath, dropPosition))
    {
        dropIndex = pathToIndex(dropPath);
        if (dropPosition == Gtk::TREE_VIEW_DROP_AFTER || dropPosition == Gtk::TREE_VIEW_DROP_INTO_OR_AFTER)
        {
            ++dropIndex;
        }
    }

    return dropIndex;
}

uint32_t PlayQueueView::pathToIndex(Gtk::TreePath path)
{
    return stringops::toNumeric<uint32_t>(path.to_string());
}

}
