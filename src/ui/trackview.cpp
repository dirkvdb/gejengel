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

#include "trackview.h"

#include <ostream>
#include <glibmm/i18n.h>

#include "trackmodel.h"
#include "Core/settings.h"
#include "utils/log.h"
#include "utils/trace.h"
#include "utils/stringoperations.h"

using namespace std;
using namespace utils;

namespace Gejengel
{
    
TrackView::TrackView(TrackModel& trackModel, Settings& settings)
: ScrolledWindow()
, m_TrackModel(trackModel)
, m_Settings(settings)
{
    utils::trace("Create Track view");
    set_shadow_type(Gtk::SHADOW_IN);

    m_TreeView.set_model(trackModel.getStore());
    m_TreeView.set_rules_hint(true);
    m_TreeView.get_selection()->unselect_all();
    m_TreeView.get_selection()->set_mode(Gtk::SELECTION_MULTIPLE);
    m_TreeView.columns_autosize();

    std::vector<Gtk::TargetEntry> targets;
    targets.push_back(Gtk::TargetEntry("PlayQueue", Gtk::TARGET_SAME_APP | Gtk::TARGET_OTHER_WIDGET));
    m_TreeView.enable_model_drag_source(targets, Gdk::MODIFIER_MASK, Gdk::ACTION_COPY);
    
    {
        Gtk::Menu::MenuList& menulist = m_PopupMenu.items();
        menulist.push_back(Gtk::Menu_Helpers::ImageMenuElem (_("Add to queue"), *Gtk::manage(new Gtk::Image(Gtk::Stock::ADD, Gtk::BuiltinIconSize(Gtk::ICON_SIZE_MENU))), sigc::mem_fun(*this, &TrackView::queueSelected)));
    }
    m_PopupMenu.accelerate(*this);
    
    {
        Gtk::Menu::MenuList& menulist = m_HeaderMenu.items();
        // This order must be the same as the "add" order in the TrackModel to assure matching ids!
        menulist.push_back(Gtk::Menu_Helpers::CheckMenuElem(_("Track Number"), sigc::bind(sigc::mem_fun(*this, &TrackView::onHeaderEnable), m_TrackModel.columns().trackNr.index())));
        menulist.push_back(Gtk::Menu_Helpers::CheckMenuElem(_("Artist"), sigc::bind(sigc::mem_fun(*this, &TrackView::onHeaderEnable), m_TrackModel.columns().artist.index())));
        menulist.push_back(Gtk::Menu_Helpers::CheckMenuElem(_("Title"), sigc::bind(sigc::mem_fun(*this, &TrackView::onHeaderEnable), m_TrackModel.columns().title.index())));
        menulist.push_back(Gtk::Menu_Helpers::CheckMenuElem(_("Album"), sigc::bind(sigc::mem_fun(*this, &TrackView::onHeaderEnable), m_TrackModel.columns().album.index())));
        menulist.push_back(Gtk::Menu_Helpers::CheckMenuElem(_("Duration"), sigc::bind(sigc::mem_fun(*this, &TrackView::onHeaderEnable), m_TrackModel.columns().duration.index())));        
        menulist.push_back(Gtk::Menu_Helpers::CheckMenuElem(_("Genre"), sigc::bind(sigc::mem_fun(*this, &TrackView::onHeaderEnable), m_TrackModel.columns().genre.index())));
        menulist.push_back(Gtk::Menu_Helpers::CheckMenuElem(_("Disc Number"), sigc::bind(sigc::mem_fun(*this, &TrackView::onHeaderEnable), m_TrackModel.columns().discNr.index())));
        menulist.push_back(Gtk::Menu_Helpers::CheckMenuElem(_("Bitrate"), sigc::bind(sigc::mem_fun(*this, &TrackView::onHeaderEnable), m_TrackModel.columns().bitrate.index())));
        menulist.push_back(Gtk::Menu_Helpers::CheckMenuElem(_("Path"), sigc::bind(sigc::mem_fun(*this, &TrackView::onHeaderEnable), m_TrackModel.columns().filepath.index())));
        menulist.push_back(Gtk::Menu_Helpers::CheckMenuElem(_("Album Artist"), sigc::bind(sigc::mem_fun(*this, &TrackView::onHeaderEnable), m_TrackModel.columns().albumArtist.index())));
        menulist.push_back(Gtk::Menu_Helpers::CheckMenuElem(_("Composer"), sigc::bind(sigc::mem_fun(*this, &TrackView::onHeaderEnable), m_TrackModel.columns().composer.index())));
    }
    m_HeaderMenu.accelerate(*this);

    loadSettings();

    std::vector<const Gtk::TreeViewColumn*> columns = m_TreeView.get_columns();
    for (size_t i = 0; i < columns.size(); ++i)
    {
        m_TreeView.get_column(i)->set_resizable(true);
        m_TreeView.get_column(i)->set_reorderable(true);
        
        GtkWidget* pButton = m_TreeView.get_column(i)->gobj()->button;
        if (pButton)
        {
            g_signal_connect(G_OBJECT(pButton), "button-press-event", GTK_SIGNAL_FUNC(TrackView::onHeaderClick), this);
        }
    }

    add(m_TreeView);

    //only show scrollbars when needed
    set_policy(Gtk::POLICY_AUTOMATIC, Gtk::POLICY_AUTOMATIC);

    m_TreeView.signal_key_press_event().connect(sigc::mem_fun(*this, &TrackView::onKeyPress), false);
    m_TreeView.signal_row_activated().connect(sigc::mem_fun(*this, &TrackView::onRowActivated));
    m_TreeView.signal_button_press_event().connect(sigc::mem_fun(*this, &TrackView::onButtonPress), false);
    m_TreeView.signal_drag_begin().connect(sigc::mem_fun(*this, &TrackView::onDragBegin));
    m_TreeView.signal_drag_data_get().connect(sigc::mem_fun(*this, &TrackView::onGetDragData));

    m_TrackModel.getStore()->signal_row_inserted().connect(sigc::mem_fun(*this, &TrackView::onRowAdded));
    m_TrackModel.getStore()->signal_row_deleted().connect(sigc::mem_fun(*this, &TrackView::onRowDeleted));

    show_all_children();

    utils::trace("Track view created");
}


TrackView::~TrackView()
{
    saveSettings();
}

void TrackView::loadSettings()
{
    std::string columns = m_Settings.get("TrackViewColumns", "0,1,2,3,4,5,6");
    std::vector<std::string> indexes = stringops::tokenize(columns, ",");
    
    Gtk::Menu::MenuList& menulist = m_HeaderMenu.items();
    
    for (size_t i = 0; i < indexes.size(); ++i)
    {
        int32_t id = stringops::toNumeric<int32_t>(indexes[i]);        
        if (id < m_TrackModel.columns().id.index() - 1)
        {
            dynamic_cast<Gtk::CheckMenuItem&>(menulist[id]).set_active(true);
        }
    }
    
    int sortId = m_Settings.getAsInt("TrackViewSortColumnId", m_TrackModel.columns().trackNr.index());
    int sortOrder = m_Settings.getAsInt("TrackViewSortColumnOrder", Gtk::SORT_ASCENDING);
    Gtk::SortType order = static_cast<Gtk::SortType>(sortOrder);
    m_TrackModel.setSortColumn(sortId, order);
}

void TrackView::saveSettings()
{
    std::stringstream ss;
    
    std::vector<const Gtk::TreeViewColumn*> columns = m_TreeView.get_columns();
    for (size_t i = 0; i < columns.size(); ++i)
    {
        if (!ss.str().empty())
        {
            ss << ',';
        }
        
        Gtk::TreeViewColumn* pCol = m_TreeView.get_column(i);
        ss << reinterpret_cast<int64_t>(pCol->property_user_data().get_value());
    }
    
    m_Settings.set("TrackViewColumns", ss.str());
    
    int32_t sortId;
    Gtk::SortType sortOrder;
    m_TrackModel.getSortColumn(sortId, sortOrder);
    
    m_Settings.set("TrackViewSortColumnId", sortId);
    m_Settings.set("TrackViewSortColumnOrder", static_cast<int32_t>(sortOrder));
}

bool TrackView::onButtonPress(GdkEventButton* pEvent)
{
    if ((pEvent->type == GDK_BUTTON_PRESS) && (pEvent->button == 3))
    {
        m_PopupMenu.popup(pEvent->button, pEvent->time);

        Gtk::TreeModel::Path mousePath;
        Gtk::TreeViewColumn* pCol;
        int x, y;
        if (m_TreeView.get_path_at_pos(static_cast<int>(pEvent->x), static_cast<int>(pEvent->y), mousePath, pCol, x, y))
        {
            if (m_TreeView.get_selection()->is_selected(mousePath))
            {
                return true;
            }
        }
    }

    return false;
}

bool TrackView::onKeyPress(GdkEventKey* pEvent)
{
    switch (pEvent->keyval)
    {
    case GDK_KP_Enter:
    case GDK_Return:
        queueSelected();
        return true;
    default:
        return false;
    }
}


void TrackView::onRowActivated(const Gtk::TreeModel::Path& activatedPath, Gtk::TreeViewColumn* column)
{
    queueSelected();
}

void TrackView::onRowAdded(const Gtk::TreePath&, const Gtk::TreeIter&)
{
    m_TreeView.columns_autosize();
}

void TrackView::onRowDeleted(const Gtk::TreePath&)
{
    m_TreeView.columns_autosize();
}

void TrackView::queueSelected()
{
    m_TreeView.get_selection()->selected_foreach_iter(sigc::mem_fun(*this, &TrackView::queuePath));
}

void TrackView::queuePath(const Gtk::TreeModel::iterator& iter)
{
    const std::string& id = (*iter)[m_TrackModel.columns().id];
    signalTrackQueued.emit(id, -1);
}

void TrackView::onDragBegin(const Glib::RefPtr<Gdk::DragContext>& context)
{
    Gtk::TreeModel::Path path = *(m_TreeView.get_selection()->get_selected_rows().begin());
    Gtk::TreeModel::iterator iter = m_TrackModel.getStore()->get_iter(path);

    Glib::RefPtr<Pango::Layout> layout = m_TreeView.create_pango_layout("");
    stringstream ss;
    ss  << "<small>" << Glib::Markup::escape_text((*iter)[m_TrackModel.columns().artist]) << endl
        << Glib::Markup::escape_text((*iter)[m_TrackModel.columns().title]) << "</small>";
    layout->set_markup(ss.str());
    Pango::Rectangle rect = layout->get_pixel_logical_extents();

    int xPad = 5, yPad = 5;
    int width = rect.get_width() + 2*xPad;
    int height = rect.get_height() + 2*yPad;

    Glib::RefPtr<Gdk::Window> window = m_TreeView.get_window();
    Glib::RefPtr<Gdk::Pixmap> pixmap = Gdk::Pixmap::create(window, width, height);
    Glib::RefPtr<Gdk::GC> gc = m_TreeView.get_style()->get_fg_gc(m_TreeView.get_state());
    Gdk::Color bgColor = m_TreeView.get_style()->get_bg(m_TreeView.get_state());

    Cairo::RefPtr<Cairo::Context> cc = pixmap->create_cairo_context();
    cc->set_source_rgb(bgColor.get_red_p(), bgColor.get_green_p(), bgColor.get_blue_p());
    cc->rectangle(0, 0, width, height);
    cc->fill();

    pixmap->draw_layout(gc, xPad, yPad, layout);
    pixmap->draw_rectangle(gc, false, 0, 0, width - 1, height - 1);

    Glib::RefPtr<Gdk::Colormap> colormap = m_TreeView.get_default_colormap();
    Glib::RefPtr<Gdk::Bitmap> bitmap;

    context->set_icon(colormap, pixmap, bitmap, 0, 0);
}

void TrackView::onGetDragData(const Glib::RefPtr<Gdk::DragContext>&, Gtk::SelectionData& selection_data, guint, guint)
{
    Gtk::TreeModel::Path path = *(m_TreeView.get_selection()->get_selected_rows().begin());
    Gtk::TreeModel::iterator iter = m_TrackModel.getStore()->get_iter(path);
    const std::string& trackId = (*iter)[m_TrackModel.columns().id];

    selection_data.set(selection_data.get_target(), "0" + trackId);
}

void TrackView::onHeaderEnable(int32_t headerId)
{
    size_t numHeaders = m_TreeView.get_columns().size();
    
    Gtk::TreeViewColumn* pColToRemove = nullptr;
    
    std::vector<Gtk::TreeViewColumn*> columns = m_TreeView.get_columns();
    for (size_t i = 0; i < columns.size(); ++i)
    {
        int64_t id = reinterpret_cast<int64_t>(m_TreeView.get_column(i)->property_user_data().get_value());
        if (static_cast<int32_t>(id) == headerId)
        {
            pColToRemove = m_TreeView.get_column(i);
            break;
        }
    }

    if (!pColToRemove)
    {
        if (headerId == m_TrackModel.columns().artist.index())
        {
            m_TreeView.append_column(_("Artist"), m_TrackModel.columns().artist);
            m_TreeView.get_column(numHeaders)->set_sort_column(m_TrackModel.columns().artist);
        }
        else if (headerId == m_TrackModel.columns().title.index())
        {
            m_TreeView.append_column(_("Title"), m_TrackModel.columns().title);
            m_TreeView.get_column(numHeaders)->set_sort_column(m_TrackModel.columns().title);
        }
        else if (headerId == m_TrackModel.columns().album.index())
        {
            m_TreeView.append_column(_("Album"), m_TrackModel.columns().album);
            m_TreeView.get_column(numHeaders)->set_sort_column(m_TrackModel.columns().album);
        }
        else if (headerId == m_TrackModel.columns().trackNr.index())
        {
            m_TreeView.append_column_numeric("#", m_TrackModel.columns().trackNr, "%d");
            m_TreeView.get_column(numHeaders)->set_sort_column(m_TrackModel.columns().trackNr);
        }
        else if (headerId == m_TrackModel.columns().discNr.index())
        {
            m_TreeView.append_column_numeric(_("Disc"), m_TrackModel.columns().discNr, "%d");
            m_TreeView.get_column(numHeaders)->set_clickable(true);
        }
        else if (headerId == m_TrackModel.columns().genre.index())
        {
            m_TreeView.append_column(_("Genre"), m_TrackModel.columns().genre);
            m_TreeView.get_column(numHeaders)->set_sort_column(m_TrackModel.columns().genre);
        }
        else if (headerId == m_TrackModel.columns().duration.index())
        {
            m_TreeView.append_column(_("Duration"), m_TrackModel.columns().duration);
            m_TreeView.get_column(numHeaders)->set_sort_column(m_TrackModel.columns().duration);
        }
        else if (headerId == m_TrackModel.columns().bitrate.index())
        {
            m_TreeView.append_column(_("Bitrate"), m_TrackModel.columns().bitrate);
            m_TreeView.get_column(numHeaders)->set_sort_column(m_TrackModel.columns().bitrate);
        }
        else if (headerId == m_TrackModel.columns().filepath.index())
        {
            m_TreeView.append_column(_("Path"), m_TrackModel.columns().filepath);
            m_TreeView.get_column(numHeaders)->set_sort_column(m_TrackModel.columns().filepath);
        }
        else if (headerId == m_TrackModel.columns().albumArtist.index())
        {
            m_TreeView.append_column(_("Album Artist"), m_TrackModel.columns().albumArtist);
            m_TreeView.get_column(numHeaders)->set_sort_column(m_TrackModel.columns().albumArtist);
        }
        else if (headerId == m_TrackModel.columns().composer.index())
        {
            m_TreeView.append_column(_("Composer"), m_TrackModel.columns().composer);
            m_TreeView.get_column(numHeaders)->set_sort_column(m_TrackModel.columns().composer);
        }
        else
        {
            log::error("Invalid header id received:", headerId);
            return;
        }
        
        Gtk::TreeViewColumn* pCol = m_TreeView.get_column(numHeaders);
        pCol->property_user_data() = reinterpret_cast<void*>(headerId);
        pCol->set_resizable(true);
        pCol->set_reorderable(true);
        
        GtkWidget* pButton = pCol->gobj()->button;
        if (pButton)
        {
            g_signal_connect(G_OBJECT(pButton), "button-press-event", GTK_SIGNAL_FUNC(TrackView::onHeaderClick), this);
        }
        else
        {
            log::error("Olabakkes, no button?");
        }
    }
    else
    {
        m_TreeView.remove_column(*pColToRemove);
    }
}

gboolean TrackView::onHeaderClick(GtkWidget* pWidget, GdkEventButton* pEvent, gpointer pData)
{
    if (pEvent->button == 3 && pEvent->type == GDK_BUTTON_PRESS)
    {
        TrackView* pInstance = reinterpret_cast<TrackView*>(pData);
        pInstance->m_HeaderMenu.popup(pEvent->button, pEvent->time);
    }
    
    return FALSE;
}

}
