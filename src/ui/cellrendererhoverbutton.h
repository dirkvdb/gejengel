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

#ifndef CELL_RENDERER_HOVER_BUTTON_H
#define CELL_RENDERER_HOVER_BUTTON_H

#include <gtkmm.h>
#include <gdkmm.h>

#include "utils/types.h"

namespace Gejengel
{

struct MouseInfo;

class CellRendererHoverButton : public Gtk::CellRenderer
{
public:
    enum HoverMode
    {
        OnCell,
        OnButton
    };

    enum Markup
    {
        Bold        = 0x01,
        Italic      = 0x02,
        Small       = 0x04,
        Big         = 0x08,
    };

    CellRendererHoverButton(int32_t iconSize, MouseInfo& mouseInfo);
    ~CellRendererHoverButton();

    void setLine1Markup(uint32_t markupFlags);
    void setLine2Markup(uint32_t markupFlags);

    void setHoverMode(HoverMode mode);
    void setHoverIcon(const Glib::ustring& iconName);
    void setDefaultIcon(const Glib::ustring& iconName);

    Glib::PropertyProxy<bool> property_icon_border();
    Glib::PropertyProxy<bool> property_can_hover();
    Glib::PropertyProxy<Glib::RefPtr<Gdk::Pixbuf> > property_icon();
    Glib::PropertyProxy<Glib::ustring> property_line1();
    Glib::PropertyProxy<Glib::ustring> property_line2();
    Glib::PropertyProxy<Glib::ustring> property_bginfo();

    void get_size_vfunc(Gtk::Widget& widget, const Gdk::Rectangle* cellArea,
                        int32_t* x_offset, int32_t* y_offset,
                        int32_t* width, int32_t* height) const;
    void render_vfunc(  const Glib::RefPtr<Gdk::Drawable>& window, Gtk::Widget& widget,
                        const Gdk::Rectangle& bgArea, const Gdk::Rectangle& cellArea,
                        const Gdk::Rectangle& exposeArea, Gtk::CellRendererState flags);

private:
    void iconThemeChanged();
    Glib::RefPtr<Pango::Layout> createLayout(Gtk::Widget& widget) const;
    Glib::RefPtr<Pango::Layout> createBgLayout(Gtk::Widget& widget, uint32_t height) const;

    int32_t                                     m_IconSize;
    Glib::Property<bool>                        m_IconBorderProperty;
    Glib::Property<bool>                        m_CanHoverProperty;
    Glib::Property<Glib::RefPtr<Gdk::Pixbuf> >  m_IconProperty;
    Glib::Property<Glib::ustring>               m_Line1Property;
    Glib::Property<Glib::ustring>               m_Line2Property;
    Glib::Property<Glib::ustring>               m_BgInfoProperty;
    Glib::RefPtr<Gdk::Pixbuf>                   m_HoverIcon;
    Glib::ustring                               m_HoverIconName;
    Glib::RefPtr<Gdk::Pixbuf>                   m_DefaultIcon;
    Glib::ustring                               m_DefaultIconName;
    MouseInfo&                                  m_MouseInfo;
    HoverMode                                   m_HoverMode;

    uint32_t                                    m_Line1Markup;
    uint32_t                                    m_Line2Markup;
};

}

#endif
