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

#include "cellrendererhoverbutton.h"

#include "utils/log.h"
#include "utils/trace.h"
#include "utils/numericoperations.h"

#include "mouseawaretreeview.h"

#include <assert.h>
#include <gtkmm/stock.h>
#include <gtkmm/button.h>
#include <gtkmm/icontheme.h>
#include <gtkmm/treeviewcolumn.h>
#include <gdkmm/general.h>
#include <cairomm/cairomm.h>

using namespace std;
using namespace Gtk;
using namespace Glib;
using namespace utils;

namespace Gejengel
{

extern Gtk::Button* pButton; //OMG please fix this!

CellRendererHoverButton::CellRendererHoverButton(int32_t iconSize, MouseInfo& mouseInfo)
: Glib::ObjectBase(typeid(CellRendererHoverButton))
, m_IconSize(iconSize)
, m_IconBorderProperty(*this, "icon_border", false)
, m_CanHoverProperty(*this, "can_hover", true)
, m_IconProperty(*this, "icon", Glib::RefPtr<Gdk::Pixbuf>())
, m_Line1Property(*this, "line1", "")
, m_Line2Property(*this, "line2", "")
, m_BgInfoProperty(*this, "bginfo", "")
, m_MouseInfo(mouseInfo)
, m_HoverMode(OnButton)
, m_Line1Markup(0)
, m_Line2Markup(0)
{
    utils::trace("Create Hover button cell renderer");
    
    property_mode() = Gtk::CELL_RENDERER_MODE_ACTIVATABLE;
    property_xpad() = 2;
    property_ypad() = 2;
    property_xalign() = 0.0;
    property_yalign() = 0.5;
}

CellRendererHoverButton::~CellRendererHoverButton()
{
}

void CellRendererHoverButton::setLine1Markup(uint32_t markupFlags)
{
    m_Line1Markup = markupFlags;
}

void CellRendererHoverButton::setLine2Markup(uint32_t markupFlags)
{
    m_Line2Markup = markupFlags;
}

void CellRendererHoverButton::setHoverMode(HoverMode mode)
{
    m_HoverMode = mode;
}

void CellRendererHoverButton::setHoverIcon(const Glib::ustring& iconName)
{
    m_HoverIconName = iconName;
    m_HoverIcon = Glib::RefPtr<Gdk::Pixbuf>();
}

void CellRendererHoverButton::setDefaultIcon(const Glib::ustring& iconName)
{
    m_DefaultIconName = iconName;
    m_DefaultIcon = Glib::RefPtr<Gdk::Pixbuf>();
}

Glib::PropertyProxy<bool> CellRendererHoverButton::property_icon_border()
{
    return m_IconBorderProperty.get_proxy();
}

Glib::PropertyProxy<bool> CellRendererHoverButton::property_can_hover()
{
    return m_CanHoverProperty.get_proxy();
}

Glib::PropertyProxy<Glib::RefPtr<Gdk::Pixbuf> > CellRendererHoverButton::property_icon()
{
    return m_IconProperty.get_proxy();
}

Glib::PropertyProxy<Glib::ustring> CellRendererHoverButton::property_line1()
{
    return m_Line1Property.get_proxy();
}

Glib::PropertyProxy<Glib::ustring> CellRendererHoverButton::property_line2()
{
    return m_Line2Property.get_proxy();
}

Glib::PropertyProxy<Glib::ustring> CellRendererHoverButton::property_bginfo()
{
    return m_BgInfoProperty.get_proxy();
}

static void markupLine(stringstream& ss, const Glib::ustring& line, uint32_t markupFlags)
{
    if (markupFlags & CellRendererHoverButton::Big)
        ss << "<big>";
    else if (markupFlags & CellRendererHoverButton::Small)
        ss << "<small>";

    if (markupFlags & CellRendererHoverButton::Bold)
        ss << "<b>";

    if (markupFlags & CellRendererHoverButton::Italic)
        ss << "<i>";

    ss << Glib::Markup::escape_text(line);

    if (markupFlags & CellRendererHoverButton::Italic)
        ss << "</i>";

    if (markupFlags & CellRendererHoverButton::Bold)
        ss << "</b>";

    if (markupFlags & CellRendererHoverButton::Big)
        ss << "</big>";
    else if (markupFlags & CellRendererHoverButton::Small)
        ss << "</small>";
}

Glib::RefPtr<Pango::Layout> CellRendererHoverButton::createLayout(Widget& widget) const
{
    RefPtr<Pango::Layout> layout = widget.create_pango_layout("");

    stringstream ss;
    markupLine(ss, m_Line1Property.get_value(), m_Line1Markup);
    ss << endl;
    markupLine(ss, m_Line2Property.get_value(), m_Line2Markup);

    layout->set_markup(ss.str());
    return layout;
}

Glib::RefPtr<Pango::Layout> CellRendererHoverButton::createBgLayout(Widget& widget, uint32_t height) const
{
    RefPtr<Pango::Layout> layout = widget.create_pango_layout("");

    stringstream ss;
    ss << "<span font=\"" << height << "px\" weight=\"heavy\">" << m_BgInfoProperty.get_value() << "</span>";

    layout->set_markup(ss.str());
    return layout;
}

void CellRendererHoverButton::get_size_vfunc(Widget& widget, const Gdk::Rectangle* cellArea, int32_t* xOffset, int32_t* yOffset, int32_t* width, int32_t* height) const
{
    Glib::RefPtr<Pango::Layout> layout = createLayout(widget);
    Pango::Rectangle rect = layout->get_pixel_logical_extents();

    //setting the width to the actual size of the contents causes that the mainwindow
    //can not be made smaller than this value, which can be annoying when there is big text
    //the width seems to act like a minimum value
    *width = 50;
    *width = (property_xpad() * 3) + m_IconSize + rect.get_width();
    *height = (property_ypad() * 2) + max(m_IconSize, rect.get_height());

    if (cellArea)
    {
        if (xOffset)
        {
            *xOffset = max(0, int32_t(property_xalign() * (cellArea->get_width() - *width)));
        }

        if (yOffset)
        {
            *yOffset = max(0, int32_t(property_yalign() * (cellArea->get_height() - rect.get_height())));
        }
    }
}

static bool positionInRectangle(const MouseInfo& pos, const Gdk::Rectangle& rect)
{
    int32_t rectX = rect.get_x();
    int32_t rectY = rect.get_y();
    return ((pos.x >= rectX) && (pos.x <= rectX + rect.get_width())
        &&  (pos.y >= rectY) && (pos.y <= rectY + rect.get_height()));
}

void CellRendererHoverButton::render_vfunc(const RefPtr<Gdk::Drawable>& window, Widget& widget, const Gdk::Rectangle& bgArea, const Gdk::Rectangle& cellArea, const Gdk::Rectangle& exposeArea, CellRendererState flags)
{
    const unsigned int xPad = property_xpad();
    const unsigned int yPad = property_ypad();

    int32_t albumYOffset = yPad;
    Glib::RefPtr<Pango::Layout> layout = createLayout(widget);
    Pango::Rectangle rect = layout->get_pixel_logical_extents();
    if (rect.get_height() > m_IconSize)
    {
        albumYOffset += (rect.get_height() - m_IconSize) / 2;
    }

    Glib::RefPtr<Pango::Layout> bgLayout = createBgLayout(widget, rect.get_height() - 8);
    Pango::Rectangle bgTextRect = bgLayout->get_pixel_logical_extents();

    Gdk::Rectangle iconRect(cellArea.get_x() + xPad, cellArea.get_y() + albumYOffset, m_IconSize, m_IconSize);
    const int32_t iconX = iconRect.get_x();
    const int32_t iconY = iconRect.get_y();

    bool mouseInIcon = positionInRectangle(m_MouseInfo, iconRect);
    bool mouseInCell = positionInRectangle(m_MouseInfo, cellArea);

    if (mouseInCell)
    {
        m_MouseInfo.isOnButton = mouseInIcon;
        m_MouseInfo.isInRegion = m_HoverMode == OnCell ? mouseInCell : mouseInIcon;
        
        if (mouseInIcon)
        {
            m_MouseInfo.buttonName = "unqueue";
        }
    }
    else
    {
        m_MouseInfo.isOnButton = false;
        m_MouseInfo.isInRegion = false;
    }

    if (m_MouseInfo.hasMoved && !mouseInIcon)
    {
        m_MouseInfo.hasMoved = false;
        return;
    }

    int32_t xOffset = 0, yOffset = 0, width = 0, height = 0;
    get_size(widget, cellArea, xOffset, yOffset, width, height);

    width = cellArea.get_width();

    width  -= xPad * 2;
    height -= yPad * 2;

    if (width <= 0 || height <= 0)
    {
        return;
    }

    layout->set_width((widget.get_width() - m_IconSize - 4 * xPad) * Pango::SCALE);
    layout->set_ellipsize(Pango::ELLIPSIZE_END);

    //log::debug(__FUNCTION__);
    //log::debug("xoffset: " + toString(xOffset) + " yOffset " + toString(yOffset));
    //log::debug("xpad: " + toString(xPad) + " yPad " + toString(yPad));
    //log::debug("  Width: " + toString(width) + " " + toString(layout->get_width()));
    //log::debug("rectw: ", rect.get_width(), "recth", rect.get_height());

    //log::debug("widgw:", widget.get_width(), "widgh:", widget.get_height());
    //log::debug("bgarw:", bgArea.get_width(), "bgarh:", bgArea.get_height());
    //log::debug("cellw:", cellArea.get_width(), "cellh:", cellArea.get_height());
    //log::debug("cellx:", cellArea.get_x(), "celly:",  cellArea.get_y());
    //log::debug(string("Mouse in cell: ") + toString(mouseInCell) + string(" Mouse in region: ") + toString(m_MouseInfo.isInRegion));

    Glib::RefPtr<Gdk::GC> gc = widget.get_style()->get_fg_gc(widget.get_state());

    Gdk::Color fgColor = widget.get_style()->get_fg(widget.get_state());
    Cairo::RefPtr<Cairo::Context> ctxt = window->create_cairo_context();
    ctxt->set_source_rgba(fgColor.get_red_p(), fgColor.get_green_p(), fgColor.get_blue_p(), 0.15);
    ctxt->move_to(cellArea.get_x() + widget.get_width() - bgTextRect.get_width() - 2*xPad, cellArea.get_y() + (cellArea.get_height() - bgTextRect.get_height()) / 2);
    bgLayout->show_in_cairo_context(ctxt);

    const Gtk::StateType state = widget.get_state();
    const Glib::RefPtr<Gtk::Style> style = widget.get_style();
    Gdk::Color textColor = style->get_text(state);

    
    ctxt->move_to(cellArea.get_x() + (3 * xPad) + m_IconSize + xOffset, cellArea.get_y() + yOffset);
    ctxt->set_source_rgb(   static_cast<double>(textColor.get_red()) / 65535.0,
                            static_cast<double>(textColor.get_green()) / 65535.0,
                            static_cast<double>(textColor.get_blue()) / 65535.0 );
    layout->show_in_cairo_context(ctxt);

    if (m_CanHoverProperty.get_value() && (mouseInIcon || (mouseInCell && m_HoverMode == OnCell)))
    {
        RefPtr<Gdk::Window> win = RefPtr<Gdk::Window>::cast_dynamic(window);
        if (win)
        {
            StateType state = (m_MouseInfo.isPressed && mouseInIcon) ? STATE_ACTIVE : STATE_PRELIGHT;
            if (pButton)
            {
#if GTKMM_MAJOR_VERSION > 2
                pButton->get_style()->paint_box(ctxt, state, Gtk::SHADOW_NONE, widget, "button", iconX, iconY, m_IconSize, m_IconSize);
#else
                pButton->get_style()->paint_box(win, state, Gtk::SHADOW_OUT, cellArea, widget, "button", iconX, iconY, m_IconSize, m_IconSize);
#endif                
            }
        }

        if (!m_HoverIcon)
        {
            Glib::RefPtr<IconTheme> theme = IconTheme::get_default();
            m_HoverIcon = theme->load_icon(m_HoverIconName, m_IconSize - 8, (IconLookupFlags) 0);
        }

        if (m_HoverIcon)
        {
            Gdk::Cairo::set_source_pixbuf(ctxt, m_HoverIcon, iconX + 4, iconY + 4);
            ctxt->paint();
        }
    }
    else
    {
        if (m_IconProperty.get_value())
        {
            Gdk::Cairo::set_source_pixbuf(ctxt, m_IconProperty.get_value(), iconX, iconY);
            ctxt->paint();

            if (property_icon_border() == true)
            {
                ctxt->set_line_width(1.0);
                ctxt->set_source_rgb(0, 0, 0);
                ctxt->rectangle(iconX + 0.5, iconY + 0.5, m_IconSize, m_IconSize);
                ctxt->stroke();
            }
        }
        else
        {
            if (!m_DefaultIcon)
            {
                Glib::RefPtr<IconTheme> theme = IconTheme::get_default();
                m_DefaultIcon = theme->load_icon(m_DefaultIconName, m_IconSize, (IconLookupFlags) 0);
            }

            if (m_DefaultIcon)
            {
                Gdk::Cairo::set_source_pixbuf(ctxt, m_DefaultIcon, iconX, iconY);
                ctxt->paint();
            }
        }
    }
}

}
