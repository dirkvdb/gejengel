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

#ifndef CELL_RENDERER_ALBUM_H
#define CELL_RENDERER_ALBUM_H

#include <gdkmm.h>
#include <gtkmm.h>

#include "utils/types.h"

namespace Gejengel
{

class IAlbumArtProvider;
class AlbumModel;
struct MouseInfo;

class CellRendererAlbum : public Gtk::CellRenderer
{
public:
    enum Size
    {
        Large = 64,
        Small = 32
    };

    CellRendererAlbum(AlbumModel& model, MouseInfo& mousePos, bool showInfoButton);
    ~CellRendererAlbum();

    void setSize(Size size);
    Size getSize();

    Glib::PropertyProxy<Glib::ustring>              	property_id();
    Glib::PropertyProxy<Glib::RefPtr<Gdk::Pixbuf> > 	property_album_art();
    Glib::PropertyProxy<Glib::ustring>                  property_title();
    Glib::PropertyProxy<Glib::ustring>                  property_artist();
    Glib::PropertyProxy<uint32_t>                       property_year();
    Glib::PropertyProxy<Glib::ustring>                  property_genre();

    void get_size_vfunc(Gtk::Widget& widget, const Gdk::Rectangle* cellArea,
                        int32_t* x_offset, int32_t* y_offset,
                        int32_t* width, int32_t* height) const;
    void render_vfunc(  const Glib::RefPtr<Gdk::Drawable>& window, Gtk::Widget& widget,
                        const Gdk::Rectangle& bgArea, const Gdk::Rectangle& cellArea,
                        const Gdk::Rectangle& exposeArea, Gtk::CellRendererState flags);

    void setButtonStyle(Glib::RefPtr<Gtk::Style> style);
    
    static std::string QueueAlbum;
    static std::string ShowTrackInfo;

private:
    void loadIcons();
    Glib::RefPtr<Pango::Layout> createLayout(Gtk::Widget& widget) const;

    Glib::Property<Glib::ustring>               	m_IdProperty;
   	Glib::Property<Glib::RefPtr<Gdk::Pixbuf> >  	m_AlbumArtProperty;
    Glib::Property<Glib::ustring>                   m_TitleProperty;
    Glib::Property<Glib::ustring>                   m_ArtistProperty;
    Glib::Property<uint32_t>                        m_YearProperty;
    Glib::Property<Glib::ustring>                   m_GenreProperty;
    Glib::RefPtr<Gdk::Pixbuf>                       m_DefaultAlbumArt;
    Glib::RefPtr<Gdk::Pixbuf>                       m_QueueImage;
    Glib::RefPtr<Gdk::Pixbuf>                       m_TrackInfoImage;
    Glib::RefPtr<Gtk::Style>                        m_ButtonStyle;
    AlbumModel&                                     m_AlbumModel;
    MouseInfo&                                      m_MouseInfo;
    Size                                            m_Size;
    int32_t                                         m_AlbumArtSize;
    int32_t                                         m_HoverSize;
    int32_t                                         m_TrackInfoSize;
    int32_t                                         m_TrackInfoIconSize;
    int32_t                                         m_QueueButtonSize;
    int32_t                                         m_QueueIconSize;
    int32_t                                         m_HoverOffset;
    int32_t                                         m_OverlayOffset;
    bool 											m_ShowInfoButton;
};

}

#endif
