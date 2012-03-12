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

#include "albuminfoview.h"

#include <cassert>
#include <glibmm/i18n.h>

#include "MusicLibrary/track.h"
#include "MusicLibrary/album.h"
#include "MusicLibrary/albumart.h"
#include "MusicLibrary/metadata.h"
#include "Core/libraryaccess.h"
#include "Core/gejengelcore.h"
#include "Core/settings.h"
#include "Core/albumartprovider.h"
#include "utils/numericoperations.h"
#include "utils/log.h"
#include "utils/trace.h"
#include "utils/subscriber.h"
#include "sharedfunctions.h"

static const uint32_t ALBUM_ART_SIZE = 160;

using namespace Gtk;
using namespace utils;

namespace Gejengel
{

AlbumInfoView::AlbumInfoView(IGejengelCore& core, TrackModel& trackModel)
: m_MainLayout(false)
, m_Artist("", ALIGN_CENTER)
, m_Album("", ALIGN_CENTER)
, m_Year("", ALIGN_CENTER)
, m_Genre("", ALIGN_CENTER)
, m_QueueImage(Stock::ADD, ICON_SIZE_LARGE_TOOLBAR)
, m_QueueButton(_("Queue album"))
, m_BackButton(Gtk::Stock::GO_BACK)
, m_TrackView(trackModel, core.getSettings())
, m_Core(core)
, m_ArtProvider(core.getAlbumArtProvider())
, m_AlbumId()
{
    utils::trace("Create Album Info view");

    m_Artist.set_ellipsize(Pango::ELLIPSIZE_END);
    m_Album.set_ellipsize(Pango::ELLIPSIZE_END);

    Glib::RefPtr<IconTheme> theme = IconTheme::get_default();
    m_DefaultCover = theme->load_icon("audio-x-generic", ALBUM_ART_SIZE, (IconLookupFlags) 0);
    theme->signal_changed().connect(sigc::mem_fun(*this, &AlbumInfoView::iconThemeChanged));
    m_CoverImage.set(m_DefaultCover);

    m_QueueButton.set_image(m_QueueImage);
    m_QueueButton.signal_clicked().connect(sigc::mem_fun(*this, &AlbumInfoView::queueAlbum));
    m_BackButton.signal_clicked().connect(sigc::mem_fun(signalBackButtonPressed, &SignalBackButtonPressed::emit));
    m_TrackView.signalTrackQueued.connect(sigc::mem_fun(signalTrackQueued, &SignalTrackQueued::emit));

    m_AlbumDetailsLayout.pack_start(m_CoverImage, PACK_SHRINK);
    m_AlbumDetailsLayout.pack_start(m_Artist, PACK_SHRINK);
    m_AlbumDetailsLayout.pack_start(m_Album, PACK_SHRINK);
    m_AlbumDetailsLayout.pack_start(m_Genre, PACK_SHRINK);
    m_AlbumDetailsLayout.pack_start(m_Year, PACK_SHRINK);

    Gtk::Label* pSpacingLabel = manage(new Gtk::Label());
    m_AlbumDetailsLayout.pack_start(*pSpacingLabel, PACK_EXPAND_PADDING);
    m_AlbumDetailsLayout.pack_start(m_QueueButton, PACK_SHRINK);
    m_AlbumDetailsLayout.pack_start(m_BackButton, PACK_SHRINK);

    m_MainLayout.pack_start(m_AlbumDetailsLayout, PACK_SHRINK);
    m_MainLayout.pack_start(m_TrackView, PACK_EXPAND_WIDGET);

    m_MainLayout.set_spacing(5);
    m_MainLayout.set_border_width(3);

    m_Frame.set_shadow_type(SHADOW_ETCHED_IN);
    m_Frame.add(m_MainLayout);

    m_AlbumDetailsLayout.modify_bg(Gtk::STATE_NORMAL, Gdk::Color("DarkSlateGray"));
    m_AlbumDetailsLayout.modify_bg(Gtk::STATE_ACTIVE, Gdk::Color("DarkSlateGray"));

    core.getSettings().getAsVector("AlbumArtFilenames", m_AlbumArtFilenames);
}


void AlbumInfoView::setAlbum(const std::string& albumId)
{
    try
    {
    	m_AlbumId = albumId;
        m_Core.getLibraryAccess().getAlbumAsync(albumId, *this);
		m_CoverImage.set(m_DefaultCover);
    }
    catch (std::exception& e)
    {
        log::error("Error getting album from database: ", e.what());
    }
}

void AlbumInfoView::onDispatchedItem(const Album& album, void* pData)
{
    m_ArtProvider.getAlbumArtFromSource(album, ALBUM_ART_SIZE, *this);

	m_Artist.set_text(album.artist);
	m_Album.set_text(album.title);
	m_Genre.set_text(album.genre);
	if (album.year != 0)
	{
		m_Year.set_text(numericops::toString(album.year));
	}
}

void AlbumInfoView::onDispatchedItem(const AlbumArt& albumArt, void* pData)
{
	try
	{
		m_CoverImage.set(Shared::createCoverPixBufWithOverlay(albumArt, ALBUM_ART_SIZE));
	}
	catch (Gdk::PixbufError&)
	{
		log::error("Failed to load album art");
	}
}


Gtk::Widget& AlbumInfoView::getWidget()
{
    return m_Frame;
}

void AlbumInfoView::iconThemeChanged()
{
    Glib::RefPtr<IconTheme> theme = IconTheme::get_default();
    m_DefaultCover = theme->load_icon("audio-x-generic", ALBUM_ART_SIZE, (IconLookupFlags) 0);
    m_CoverImage.set(m_DefaultCover);
}

void AlbumInfoView::queueAlbum()
{
	signalAlbumQueued.emit(m_AlbumId, -1);
}

}
