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

#include "detailedalbumlayout.h"

#include "mainwindow.h"
#include "Core/settings.h"
#include "utils/log.h"
#include "utils/trace.h"
#include "utils/types.h"

namespace Gejengel
{

DetailedAlbumLayout::DetailedAlbumLayout(IGejengelCore& core, PlayQueueModel& playQueueModel, TrackModel& trackModel, AlbumModel& albumModel)
: m_Settings(core.getSettings())
, m_TrackView(trackModel, core.getSettings())
, m_AlbumView(core.getSettings(), albumModel, false)
, m_PlayQueueView(playQueueModel)
, m_NowPlaying()
{
    utils::trace("Create detailed album layout");
    m_AlbumTrackPane.pack1(m_AlbumView, Gtk::SHRINK);
    m_AlbumTrackPane.pack2(m_TrackView, Gtk::EXPAND | Gtk::FILL);

    m_LeftBox.pack_start(m_NowPlaying.getWidget(), Gtk::PACK_SHRINK);
    m_LeftBox.pack_start(m_AlbumTrackPane, Gtk::PACK_EXPAND_WIDGET);
    m_LeftBox.set_spacing(5);

    m_Layout.pack1(m_LeftBox, Gtk::EXPAND);
    m_Layout.pack2(m_PlayQueueView, Gtk::SHRINK);

    m_AlbumView.signalAlbumChanged.connect(sigc::mem_fun(signalAlbumChanged, &sigc::signal<void, const std::string&>::emit));
    m_AlbumView.signalAlbumQueued.connect(sigc::mem_fun(playQueueModel.getQueue(), &PlayQueue::queueAlbum));
    m_TrackView.signalTrackQueued.connect(sigc::mem_fun<const std::string&, int32_t>(playQueueModel.getQueue(), &PlayQueue::queueTrack));

    loadSettings();
}

DetailedAlbumLayout::~DetailedAlbumLayout()
{
    saveSettings();
}

void DetailedAlbumLayout::loadSettings()
{
    m_AlbumTrackPane.set_position(m_Settings.getAsInt("DetailedAlbumLayoutDividerPos", 250));
    m_Layout.set_position(m_Settings.getAsInt("DetailedAlbumLayoutQueueDividerPos", 450));
}

void DetailedAlbumLayout::saveSettings()
{
    m_Settings.set("DetailedAlbumLayoutDividerPos", m_AlbumTrackPane.get_position());
    m_Settings.set("DetailedAlbumLayoutQueueDividerPos", m_Layout.get_position());
}


Gtk::Widget& DetailedAlbumLayout::getWidget()
{
    return m_Layout;
}

void DetailedAlbumLayout::getPlugins(std::vector<GejengelPlugin*>& plugins)
{
    plugins.push_back(&m_NowPlaying);
}

}
