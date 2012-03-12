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

#include "nowplayingview.h"

#include <cassert>
#include <gtkmm/stock.h>
#include <gtkmm/image.h>
#include <gtkmm/icontheme.h>
#include <gdkmm/pixbuf.h>
#include <gdkmm/pixbufloader.h>
#include <glibmm/i18n.h>

#include "MusicLibrary/track.h"
#include "MusicLibrary/album.h"
#include "Core/gejengelcore.h"
#include "Core/albumartprovider.h"
#include "utils/numericoperations.h"
#include "utils/log.h"
#include "utils/trace.h"
#include "sharedfunctions.h"

#define ALBUM_ART_SIZE 80

using namespace Gtk;
using namespace utils;

namespace Gejengel
{

Gtk::Button* pButton = nullptr;

static void setMarkedUpGreyText(Gtk::Label& label, const Glib::ustring& text)
{
    Gdk::Color color = label.get_style()->get_text(Gtk::STATE_INSENSITIVE);
    label.set_markup(Glib::ustring("<small><span color=\"") + color.to_string() + "\"><i>" + Glib::Markup::escape_text(text) + "</i>:</span></small>");
}

static void setMarkedUpText(Gtk::Label& label, const Glib::ustring& text)
{
    label.set_markup(Glib::ustring("<small>" + Glib::Markup::escape_text(text) + "</small>"));
}

static Gtk::Label* createMarkedUpLabel(const Glib::ustring& text)
{
    Gtk::Label* pLabel = new Gtk::Label("", ALIGN_RIGHT);

    pLabel->set_use_markup(true);
    setMarkedUpGreyText(*pLabel, text);

    return pLabel;
}


NowPlayingView::NowPlayingView()
: m_MainLayout(false)
, m_ButtonLayout(4, 4, false)
, m_TextInfoLayout(2, 5, false)
, m_Artist("", ALIGN_LEFT)
, m_Title("", ALIGN_LEFT)
, m_Album("", ALIGN_LEFT)
, m_Year("", ALIGN_LEFT)
, m_Track("", ALIGN_LEFT)
, m_Disc("", ALIGN_LEFT)
, m_pDiscLabel(nullptr)
, m_PlayingProgress(0.0, 1.0, 0.01)
, m_PlayImage(Stock::MEDIA_PLAY, ICON_SIZE_LARGE_TOOLBAR)
, m_PauseImage(Stock::MEDIA_PAUSE, ICON_SIZE_LARGE_TOOLBAR)
, m_StopImage(Stock::MEDIA_STOP, ICON_SIZE_LARGE_TOOLBAR)
, m_NextImage(Stock::MEDIA_NEXT, ICON_SIZE_LARGE_TOOLBAR)
, m_VolumeButton()
, m_VolumeAdjustment(99.0, 0.0, 100.0, 2.0)
, m_pCore(nullptr)
, m_Paused(false)
, m_Seeking(false)
, m_TrackDuration(0)
{
    utils::trace("Create Now playing view");

    m_PlayPauseButton.set_image(m_PlayImage);
    m_PlayPauseButton.set_size_request(50, -1);
    m_StopButton.set_image(m_StopImage);
    m_NextButton.set_image(m_NextImage);

    m_Artist.set_ellipsize(Pango::ELLIPSIZE_END);
    m_Title.set_ellipsize(Pango::ELLIPSIZE_END);
    m_Album.set_ellipsize(Pango::ELLIPSIZE_END);
    
    m_Artist.set_use_markup(true);
    m_Title.set_use_markup(true);
    m_Album.set_use_markup(true);
    m_Year.set_use_markup(true);
    m_Track.set_use_markup(true);
    m_Disc.set_use_markup(true);

    m_TextInfoLayout.set_homogeneous(false);

    pButton = &m_NextButton;

    Glib::RefPtr<IconTheme> theme = IconTheme::get_default();
    //m_DefaultCover = theme->load_icon("audio-x-generic", ALBUM_ART_SIZE, (IconLookupFlags) 0);
    //utils::trace("Now playing view: connect icon theme change signal");
    //theme->signal_changed().connect(sigc::mem_fun(*this, &NowPlayingView::iconThemeChanged));
    //m_CoverImage.set(m_DefaultCover);

    m_PlayPauseButton.signal_clicked().connect(sigc::mem_fun(*this, &NowPlayingView::onPlayPausePress));
    m_StopButton.signal_clicked().connect(sigc::mem_fun(*this, &NowPlayingView::onStopPress));
    m_NextButton.signal_clicked().connect(sigc::mem_fun(*this, &NowPlayingView::onNextPress));

    m_PlayingProgress.signal_button_press_event().connect(sigc::mem_fun(*this, &NowPlayingView::onProgressButtonPress), false);
    m_PlayingProgress.signal_button_release_event().connect(sigc::mem_fun(*this, &NowPlayingView::onProgressButtonRelease), false);
    m_PlayingProgress.signal_change_value().connect(sigc::mem_fun(*this, &NowPlayingView::onMoveProgressSlider));
    m_PlayingProgress.set_draw_value(false);

    m_TextInfoLayout.set_col_spacings(5);
    m_TextInfoLayout.set_row_spacings(0);

    m_TextInfoLayout.attach(*Gtk::manage(createMarkedUpLabel(_("Artist"))), 0, 1, 0, 1, FILL, SHRINK | EXPAND);
    m_TextInfoLayout.attach(*Gtk::manage(createMarkedUpLabel(_("Title"))), 0, 1, 1, 2, FILL, SHRINK | EXPAND);
    m_TextInfoLayout.attach(*Gtk::manage(createMarkedUpLabel(_("Album"))), 0, 1, 2, 3, FILL, SHRINK | EXPAND);
    m_TextInfoLayout.attach(*Gtk::manage(createMarkedUpLabel(_("Year"))), 0, 1, 3, 4, FILL, SHRINK | EXPAND);
    m_TextInfoLayout.attach(*Gtk::manage(createMarkedUpLabel(_("Track"))), 0, 1, 4, 5, FILL, SHRINK | EXPAND);

    m_pDiscLabel = createMarkedUpLabel(_("Disc"));

    m_TrackDiscLayout.set_spacing(5);
    m_TrackDiscLayout.pack_start(m_Track, PACK_SHRINK);
    m_TrackDiscLayout.pack_start(*Gtk::manage(m_pDiscLabel), PACK_SHRINK);
    m_TrackDiscLayout.pack_start(m_Disc, PACK_SHRINK);

    m_TextInfoLayout.attach(m_Artist, 1, 2, 0, 1, FILL | EXPAND, SHRINK | EXPAND);
    m_TextInfoLayout.attach(m_Title, 1, 2, 1, 2, FILL | EXPAND, SHRINK | EXPAND);
    m_TextInfoLayout.attach(m_Album, 1, 2, 2, 3, FILL | EXPAND, SHRINK | EXPAND);
    m_TextInfoLayout.attach(m_Year, 1, 2, 3, 4, FILL | EXPAND, SHRINK | EXPAND);
    m_TextInfoLayout.attach(m_TrackDiscLayout, 1, 2, 4, 5, FILL | EXPAND, SHRINK | EXPAND);

    m_ButtonLayout.attach(m_StopButton, 0, 1, 0, 1, SHRINK, FILL);
    m_ButtonLayout.attach(m_PlayPauseButton, 1, 2, 0, 1, SHRINK, FILL);
    m_ButtonLayout.attach(m_NextButton, 2, 3, 0, 1, SHRINK, FILL);
    m_ButtonLayout.attach(m_VolumeButton, 3, 4, 0, 1, SHRINK, FILL);
    m_ButtonLayout.attach(m_PlayingProgress, 0, 4, 1, 2, FILL, FILL);
    m_ButtonLayout.attach(m_ProgressLabel, 0, 4, 2, 3, FILL | EXPAND, FILL);

    m_MainLayout.pack_start(m_CoverImage, PACK_SHRINK);
    m_MainLayout.pack_start(m_TextInfoLayout, PACK_EXPAND_WIDGET);
    m_MainLayout.pack_start(m_ButtonLayout, PACK_SHRINK);

    m_MainLayout.set_spacing(5);
    m_MainLayout.set_border_width(3);

    m_Frame.set_shadow_type(SHADOW_ETCHED_IN);
    m_Frame.add(m_MainLayout);

    m_PlayPauseButton.grab_focus();
    m_PlayingProgress.set_sensitive(false);

    onProgress(0);

    m_VolumeButton.set_adjustment(m_VolumeAdjustment);
    m_VolumeButton.signal_value_changed().connect(sigc::mem_fun(*this, &NowPlayingView::onVolumeChange));

    m_pDiscLabel->set_child_visible(false);
}

Widget& NowPlayingView::getWidget()
{
    return m_Frame;
}

bool NowPlayingView::initialize(IGejengelCore& core)
{
    m_pCore = &core;

    Track track;
    core.getCurrentTrack(track);
    onPlay(track);
    onVolumeChanged(core.getVolume());
    return true;
}

std::string NowPlayingView::getName() const
{
    return "Now playing view";
}

std::string NowPlayingView::getDescription() const
{
    return "You'll never see this";
}

void NowPlayingView::onPlay(const Track& track)
{
    updateTrackInfo(track);
    updatePlaybackState();
}

void NowPlayingView::updateTrackInfo(const Track& track)
{
    setMarkedUpText(m_Artist, track.artist);
    setMarkedUpText(m_Title, track.title);
    setMarkedUpText(m_Album, track.album);

    if (!track.id.empty())
    {
        setMarkedUpText(m_Year, track.year == 0 ? _("Unknown") : numericops::toString(track.year));
        setMarkedUpText(m_Track, track.trackNr == 0 ? _("Unknown") : numericops::toString(track.trackNr));
        setMarkedUpText(m_Disc, track.discNr == 0 ? "" : numericops::toString(track.discNr));
        m_pDiscLabel->set_child_visible(track.discNr != 0);

        m_pCore->getAlbumArtProvider().getAlbumArt(track, *this);
    }
    else
    {
        m_Year.set_text("");
        m_Track.set_text("");
        m_Disc.set_text("");
        m_pDiscLabel->set_child_visible(false);
    }

    m_TrackDuration = track.durationInSec;
    Shared::durationToString(track.durationInSec, m_TrackDurationString);
    updateProgressText(0);
}

void NowPlayingView::onDispatchedItem(const AlbumArt& albumArt, void* pData)
{
	try
	{
		m_CoverImage.set(Shared::createCoverPixBufWithOverlay(albumArt, ALBUM_ART_SIZE));
	}
	catch (Gdk::PixbufError&)
	{
		log::error("Failed to load album art");
		m_CoverImage.set(m_DefaultCover);
	}
}

void NowPlayingView::updatePlaybackState()
{
    PlaybackState state = m_pCore->getPlaybackState();

    m_Paused = state == Paused;
    m_PlayPauseButton.set_image(state == Playing ? m_PauseImage : m_PlayImage);
    m_PlayingProgress.set_sensitive(state == Playing);
}

void NowPlayingView::onPause()
{
    m_PlayPauseButton.set_image(m_PlayImage);
    m_Paused = true;
}

void NowPlayingView::onResume()
{
    m_PlayPauseButton.set_image(m_PauseImage);
    m_Paused = false;
}

void NowPlayingView::onStop()
{
    m_Artist.set_text("");
    m_Title.set_text("");
    m_Album.set_text("");
    m_Year.set_text("");
    m_Track.set_text("");
    m_Disc.set_text("");
    m_pDiscLabel->set_child_visible(false);
    m_TrackDuration = 0;
    m_PlayingProgress.set_value(0);
    m_ProgressLabel.set_text("");
    m_PlayingProgress.set_sensitive(false);
    m_PlayPauseButton.set_image(m_PlayImage);
    m_CoverImage.set(m_DefaultCover);
}

void NowPlayingView::onProgress(int32_t elapsedSeconds)
{
    if (m_Seeking)
    {
        return;
    }

    updateProgressText(elapsedSeconds);
}

void NowPlayingView::onVolumeChanged(int32_t volume)
{
    m_VolumeButton.set_value(volume);
}

void NowPlayingView::updateProgressText(int32_t elapsedSeconds)
{
    numericops::clip(elapsedSeconds, 0, m_TrackDuration);

    double progress = m_TrackDuration == 0 ? 0.0 : static_cast<double>(elapsedSeconds) / m_TrackDuration;
    m_PlayingProgress.set_value(progress);
    m_ProgressLabel.set_text(formatProgress(elapsedSeconds));
}

void NowPlayingView::onVolumeChange(double volume)
{
    if (m_pCore)
    {
        m_pCore->setVolume(static_cast<int>(volume));
    }
}

Glib::ustring NowPlayingView::formatProgress(int32_t elapsedSeconds)
{
    if (m_TrackDuration == 0)
    {
        return "";
    }

    Glib::ustring elapsedTime;
    Shared::durationToString(elapsedSeconds, elapsedTime);

    Glib::ustring result = elapsedTime + " / " + m_TrackDurationString;
    return result;
}

void NowPlayingView::destroy()
{
    m_pCore = nullptr;
}

void NowPlayingView::onPlayPausePress()
{
    assert(m_pCore);
    if (m_TrackDuration == 0)
    {
        m_pCore->play();
    }
    else
    {
        m_Paused ? m_pCore->resume() : m_pCore->pause();
    }
}

void NowPlayingView::onStopPress()
{
    assert(m_pCore);
    m_pCore->stop();
}

void NowPlayingView::onNextPress()
{
    assert(m_pCore);
    m_pCore->next();
}

bool NowPlayingView::onMoveProgressSlider(ScrollType type, double value)
{
    assert(m_pCore);

    switch(type)
    {
    case SCROLL_JUMP:
    case SCROLL_STEP_FORWARD:
    case SCROLL_STEP_BACKWARD:
    case SCROLL_STEP_LEFT:
    case SCROLL_STEP_RIGHT:
    case SCROLL_PAGE_FORWARD:
    case SCROLL_PAGE_BACKWARD:
    case SCROLL_PAGE_LEFT:
    case SCROLL_PAGE_RIGHT:
    case SCROLL_PAGE_UP:
    case SCROLL_PAGE_DOWN:
        updateProgressText(static_cast<int32_t>(m_TrackDuration * value));
        if (!m_Seeking)
            m_pCore->seek(value * m_TrackDuration);
        return true;
    default:
        return false;
    }
}

bool NowPlayingView::onProgressButtonPress(GdkEventButton* pEvent)
{
    if (pEvent->type == GDK_BUTTON_PRESS)
    {
        m_Seeking = true;
    }

    return false;
}

bool NowPlayingView::onProgressButtonRelease(GdkEventButton* pEvent)
{
    if (pEvent->type == GDK_BUTTON_RELEASE)
    {
        m_Seeking = false;
        m_pCore->seek(m_PlayingProgress.get_value() * m_TrackDuration);
    }

    return false;
}

void NowPlayingView::iconThemeChanged()
{
    Glib::RefPtr<IconTheme> theme = IconTheme::get_default();
    m_DefaultCover = theme->load_icon("audio-x-generic", ALBUM_ART_SIZE, (IconLookupFlags) 0);
    m_CoverImage.set(m_DefaultCover);
}

}
