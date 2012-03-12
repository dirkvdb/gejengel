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

#include "cellrendereralbum.h"

#include "utils/log.h"
#include "utils/trace.h"
#include "utils/numericoperations.h"
#include "MusicLibrary/album.h"
#include "albummodel.h"
#include "mouseawaretreeview.h"

#include <cairomm/cairomm.h>
#include <assert.h>

using namespace std;
using namespace utils;


namespace Gejengel
{

string CellRendererAlbum::QueueAlbum = "queue";
string CellRendererAlbum::ShowTrackInfo = "info";

extern Gtk::Button* pButton; //OMG please fix this!

CellRendererAlbum::CellRendererAlbum(AlbumModel& model, MouseInfo& mousePos, bool showInfoButton)
: Glib::ObjectBase(typeid(CellRendererAlbum))
, m_IdProperty(*this, "id", "0")
, m_AlbumArtProperty(*this, "album", Glib::RefPtr<Gdk::Pixbuf>())
, m_TitleProperty(*this, "title", "")
, m_ArtistProperty(*this, "artist", "")
, m_YearProperty(*this, "year", 0)
, m_GenreProperty(*this, "genre", "")
, m_AlbumModel(model)
, m_MouseInfo(mousePos)
, m_Size(Large)
, m_AlbumArtSize(0)
, m_HoverSize(0)
, m_TrackInfoSize(32)
, m_TrackInfoIconSize(24)
, m_QueueButtonSize(0)
, m_QueueIconSize(0)
, m_HoverOffset(0)
, m_OverlayOffset(0)
, m_ShowInfoButton(showInfoButton)
{
    utils::trace("Create Album cellrenderer");
    
    setSize(m_Size);

    Glib::RefPtr<Gtk::IconTheme> theme = Gtk::IconTheme::get_default();
    theme->signal_changed().connect(sigc::mem_fun(*this, &CellRendererAlbum::loadIcons));

    property_xalign() = 0.0;
    property_yalign() = 0.5;

    utils::trace("Album cellrenderer created");
}

CellRendererAlbum::~CellRendererAlbum()
{
}

void CellRendererAlbum::setSize(Size size)
{
    switch (size)
    {
    case Large:
        m_AlbumArtSize = 64;
        m_HoverSize = 64;
        m_QueueButtonSize = 32;
        m_QueueIconSize = 32;
        m_HoverOffset = 0;
        property_xpad() = 5;
        property_ypad() = 5;
        break;
    case Small:
        m_AlbumArtSize = 32;
        m_HoverSize = 32;
        m_QueueButtonSize = 32;
        m_QueueIconSize = 16;
        m_HoverOffset = 0;
        property_xpad() = 2;
        property_ypad() = 2;
        break;
    default:
        return;
    }

    m_OverlayOffset = (m_HoverSize - m_QueueButtonSize) / 2;
    m_Size = size;

    //clear images so they will be reloaded again on the next paint
    m_DefaultAlbumArt = Glib::RefPtr<Gdk::Pixbuf>();
    m_QueueImage = Glib::RefPtr<Gdk::Pixbuf>();
}

CellRendererAlbum::Size CellRendererAlbum::getSize()
{
    return m_Size;
}

void CellRendererAlbum::loadIcons()
{
    Glib::RefPtr<Gtk::IconTheme> theme = Gtk::IconTheme::get_default();
    m_DefaultAlbumArt = theme->load_icon("audio-x-generic", m_AlbumArtSize, (Gtk::IconLookupFlags) 0);
    m_QueueImage = theme->load_icon("list-add", m_QueueIconSize, (Gtk::IconLookupFlags) 0);
    
    try {
		m_TrackInfoImage = theme->load_icon("music-library", m_TrackInfoIconSize, (Gtk::IconLookupFlags) 0);
	}
	catch (Glib::Error&)
	{
		m_TrackInfoImage = theme->load_icon("format-justify-fill", m_TrackInfoIconSize, (Gtk::IconLookupFlags) 0);
	}
}

Glib::PropertyProxy<Glib::ustring> CellRendererAlbum::property_id()
{
    return m_IdProperty.get_proxy();
}

Glib::PropertyProxy<Glib::RefPtr<Gdk::Pixbuf> > CellRendererAlbum::property_album_art()
{
    return m_AlbumArtProperty.get_proxy();
}

Glib::PropertyProxy<Glib::ustring> CellRendererAlbum::property_title()
{
    return m_TitleProperty.get_proxy();
}

Glib::PropertyProxy<Glib::ustring> CellRendererAlbum::property_artist()
{
    return m_ArtistProperty.get_proxy();
}

Glib::PropertyProxy<uint32_t> CellRendererAlbum::property_year()
{
    return m_YearProperty.get_proxy();
}

Glib::PropertyProxy<Glib::ustring> CellRendererAlbum::property_genre()
{
    return m_GenreProperty.get_proxy();
}

static bool positionInRectangle(const MouseInfo& pos, const Gdk::Rectangle& rect)
{
    int32_t rectX = rect.get_x();
    int32_t rectY = rect.get_y();
    return ((pos.x >= rectX) && (pos.x <= rectX + rect.get_width())
        &&  (pos.y >= rectY) && (pos.y <= rectY + rect.get_height()));
}

Glib::RefPtr<Pango::Layout> CellRendererAlbum::createLayout(Gtk::Widget& widget) const
{
    stringstream ss;
    ss << "<b>" << Glib::Markup::escape_text(m_TitleProperty.get_value()) << "</b>\n<i>" << Glib::Markup::escape_text(m_ArtistProperty.get_value()) << "</i>";

    if (m_Size == Large)
    {
        ss << "\n<small>";
        if (m_YearProperty.get_value() != 0)
        {
            ss << numericops::toString(m_YearProperty.get_value()) << "\n";
        }
        ss << Glib::Markup::escape_text(m_GenreProperty.get_value()) << "</small>";
    }

    Glib::RefPtr<Pango::Layout> layout = widget.create_pango_layout("");
    layout->set_markup(ss.str());

    return layout;
}

void CellRendererAlbum::get_size_vfunc(Gtk::Widget& widget, const Gdk::Rectangle* cellArea, int32_t* xOffset, int32_t* yOffset, int32_t* width, int32_t* height) const
{
    Glib::RefPtr<Pango::Layout> layout = createLayout(widget);
    Pango::Rectangle rect = layout->get_pixel_logical_extents();

    //setting the width to the actual size of the contents causes that the mainwindow
    //can not be made smaller than this value, which can be annoying when there is big text
    //the width seems to act like a minimum value 
    *width = 50;
    //*width = (property_xpad() * 3) + m_AlbumArtSize + rect.get_width();
    *height = (property_ypad() * 2) + max(m_AlbumArtSize, rect.get_height());

    if (cellArea)
    {
        if (xOffset)
        {
            *xOffset = 0;
        }

        if (yOffset)
        {
            *yOffset = max(0, int(property_yalign() * (cellArea->get_height() - rect.get_height())));
        }
    }
}

void CellRendererAlbum::render_vfunc(const Glib::RefPtr<Gdk::Drawable>& window, Gtk::Widget& widget, const Gdk::Rectangle& bgArea, const Gdk::Rectangle& cellArea, const Gdk::Rectangle& exposeArea, Gtk::CellRendererState flags)
{
    const uint32_t xPad = property_xpad();
    const uint32_t yPad = property_ypad();

    Glib::RefPtr<Pango::Layout> layout = createLayout(widget);
    Pango::Rectangle rect = layout->get_pixel_logical_extents();

    int32_t albumYOffset = yPad;
    if (rect.get_height() > m_AlbumArtSize)
    {
        albumYOffset += (rect.get_height() - m_AlbumArtSize) / 2;
    }

    Gdk::Rectangle albumRect(cellArea.get_x() + xPad, cellArea.get_y() + albumYOffset, m_AlbumArtSize, m_AlbumArtSize);
    const bool mouseInAlbumArt = positionInRectangle(m_MouseInfo, albumRect);
    const bool mouseInCell = positionInRectangle(m_MouseInfo, cellArea);

    const int32_t trackInfoYOffset = yPad + ((rect.get_height() - m_TrackInfoSize) / 2);
    Gdk::Rectangle trackInfoRect(cellArea.get_width() - xPad - m_TrackInfoSize, cellArea.get_y() + trackInfoYOffset, m_TrackInfoSize, m_TrackInfoSize);
    const bool mouseInTrackInfo = m_ShowInfoButton && positionInRectangle(m_MouseInfo, trackInfoRect);

    if (mouseInCell)
    {
        m_MouseInfo.isOnButton = mouseInAlbumArt || mouseInTrackInfo;
        m_MouseInfo.isInRegion = mouseInAlbumArt || mouseInTrackInfo;
        
        if (mouseInAlbumArt)
        {
            m_MouseInfo.buttonName = QueueAlbum;
        }
        else if (mouseInTrackInfo)
        {
            m_MouseInfo.buttonName = ShowTrackInfo;
        }
    }

    if (m_MouseInfo.hasMoved && !mouseInAlbumArt)
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

    layout->set_width((widget.get_width() - m_AlbumArtSize - 4*xPad) * Pango::SCALE);
    layout->set_ellipsize(Pango::ELLIPSIZE_END);

    //log::debug("xoffset:", xOffset, "yOffset", yOffset);
    //log::debug("xpad:", xPad, "yPad", yPad);
    //log::debug("Width:", width, layout->get_width());
    //log::debug("cellw:", cellArea.get_width(), "cellh", cellArea.get_height());

    // add 0.5 to the coordinates to avoid anti-aliasing in cairo
    Cairo::RefPtr<Cairo::Context> ctxt = window->create_cairo_context();

    const Gtk::StateType state = widget.get_state();
    const Glib::RefPtr<Gtk::Style> style = widget.get_style();
    Gdk::Color textColor = style->get_text(state);
    
    ctxt->move_to(cellArea.get_x() + 3 * xPad + m_AlbumArtSize + xOffset, cellArea.get_y() + yOffset);
    ctxt->set_source_rgb(   static_cast<double>(textColor.get_red()) / 65535.0,
                            static_cast<double>(textColor.get_green()) / 65535.0,
                            static_cast<double>(textColor.get_blue()) / 65535.0 );
    layout->show_in_cairo_context(ctxt);

    double x = cellArea.get_x() + xPad;
    double y = cellArea.get_y() + albumYOffset;

    if (!mouseInAlbumArt || (m_HoverSize != m_AlbumArtSize))
    {
        if (m_AlbumArtProperty.get_value())
        {
            Gdk::Cairo::set_source_pixbuf(ctxt, m_AlbumArtProperty.get_value(), x, y);
            ctxt->paint();
			ctxt->set_line_width(1.0);
            ctxt->set_source_rgb(0, 0, 0);
            ctxt->rectangle(x + 0.5, y + 0.5, m_AlbumArtSize, m_AlbumArtSize);
			ctxt->stroke();
        }
        else
        {
            if (!m_DefaultAlbumArt)
            {
                loadIcons();
            }

            Gdk::Cairo::set_source_pixbuf(ctxt, m_DefaultAlbumArt, x, y);
            ctxt->paint();
        }
    }

    if (mouseInAlbumArt)
    {
        Glib::RefPtr<Gdk::Window> win = Glib::RefPtr<Gdk::Window>::cast_dynamic(window);
        if (win)
        {
            Gtk::StateType state = m_MouseInfo.isPressed && (m_MouseInfo.buttonName == QueueAlbum)  ? Gtk::STATE_ACTIVE : Gtk::STATE_PRELIGHT;
            if (pButton)
            {
#if GTKMM_MAJOR_VERSION > 2
                pButton->get_style()->paint_box(ctxt, state, Gtk::SHADOW_NONE, widget, "button", cellArea.get_x() + xPad + m_HoverOffset, cellArea.get_y() + albumYOffset + m_HoverOffset, m_HoverSize, m_HoverSize);
#else
                pButton->get_style()->paint_box(win, state, Gtk::SHADOW_NONE, cellArea, widget, "button", cellArea.get_x() + xPad + m_HoverOffset, cellArea.get_y() + albumYOffset + m_HoverOffset, m_HoverSize, m_HoverSize);
#endif
            }
        }

        if (!m_QueueImage)
        {
            loadIcons();
        }
        uint32_t iconOffset = (m_QueueButtonSize - m_QueueIconSize) / 2;
        Gdk::Cairo::set_source_pixbuf(ctxt, m_QueueImage, x + m_HoverOffset + m_OverlayOffset + iconOffset, y + m_HoverOffset + m_OverlayOffset + iconOffset);
        ctxt->paint();
    }

    if (m_ShowInfoButton && mouseInCell)
    {
        Glib::RefPtr<Gdk::Window> win = Glib::RefPtr<Gdk::Window>::cast_dynamic(window);
        if (win)
        {
            Gtk::StateType state = m_MouseInfo.isPressed && (m_MouseInfo.buttonName == ShowTrackInfo) ? Gtk::STATE_ACTIVE : Gtk::STATE_PRELIGHT;
            if (pButton)
            {
#if GTKMM_MAJOR_VERSION > 2
                pButton->get_style()->paint_box(ctxt, state, Gtk::SHADOW_NONE, widget, "button", cellArea.get_width() - xPad - m_TrackInfoSize, cellArea.get_y() + trackInfoYOffset, m_TrackInfoSize, m_TrackInfoSize);
#else
                pButton->get_style()->paint_box(win, state, Gtk::SHADOW_NONE, cellArea, widget, "button", cellArea.get_width() - xPad - m_TrackInfoSize, cellArea.get_y() + trackInfoYOffset, m_TrackInfoSize, m_TrackInfoSize);
#endif
            }
        }

		if (!m_TrackInfoImage)
		{
			loadIcons();
		}
		Gdk::Cairo::set_source_pixbuf(ctxt, m_TrackInfoImage, cellArea.get_width() - xPad - m_TrackInfoSize + 4, cellArea.get_y() + trackInfoYOffset + 4);
		ctxt->paint();
    }
}

void CellRendererAlbum::setButtonStyle(Glib::RefPtr<Gtk::Style> style)
{
    if (!style)
    {
        log::debug("BAD STYLE");
    }
    else
    {
        m_ButtonStyle = style->copy();
    }
}

}
