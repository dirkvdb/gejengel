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

#include "simplealbumlayout.h"

#include "mainwindow.h"
#include "Core/gejengelcore.h"
#include "Core/settings.h"
#include "utils/log.h"
#include "utils/trace.h"

namespace Gejengel
{

SimpleAlbumLayout::SimpleAlbumLayout(IGejengelCore& core, PlayQueueModel& playQueueModel, TrackModel& trackModel, AlbumModel& albumModel)
: m_Settings(core.getSettings())
, m_AlbumView(core.getSettings(), albumModel, true)
, m_PlayQueueView(playQueueModel)
, m_NowPlaying()
, m_AlbumInfoView(core, trackModel)
{
    utils::trace("Create detailed album layout");
    m_AlbumQueuePane.pack1(m_AlbumView, Gtk::SHRINK);
    m_AlbumQueuePane.pack2(m_PlayQueueView, Gtk::SHRINK);

    m_Layout.pack_start(m_NowPlaying.getWidget(), Gtk::PACK_SHRINK);
    m_Layout.pack_start(m_AlbumQueuePane, Gtk::PACK_EXPAND_WIDGET);
    m_Layout.set_spacing(5);

    m_AlbumView.signalAlbumQueued.connect(sigc::mem_fun(playQueueModel.getQueue(), &PlayQueue::queueAlbum));
    m_AlbumView.signalAlbumInfoRequested.connect(sigc::mem_fun(*this, &SimpleAlbumLayout::showAlbumInfo));
    m_AlbumInfoView.signalBackButtonPressed.connect(sigc::mem_fun(*this, &SimpleAlbumLayout::showAlbums));
    m_AlbumInfoView.signalAlbumQueued.connect(sigc::mem_fun(playQueueModel.getQueue(), &PlayQueue::queueAlbum));
    m_AlbumInfoView.signalTrackQueued.connect(sigc::mem_fun<const std::string&, int32_t>(playQueueModel.getQueue(), &PlayQueue::queueTrack));

    loadSettings();
}

SimpleAlbumLayout::~SimpleAlbumLayout()
{
    saveSettings();
}

void SimpleAlbumLayout::loadSettings()
{
    m_AlbumQueuePane.set_position(m_Settings.getAsInt("SimpleAlbumLayoutDividerPos", 250));
}

void SimpleAlbumLayout::saveSettings()
{
    m_Settings.set("SimpleAlbumLayoutDividerPos", m_AlbumQueuePane.get_position());
}

void SimpleAlbumLayout::showAlbumInfo(const std::string& albumId)
{
    m_Layout.remove(m_AlbumQueuePane);
    m_Layout.add(m_AlbumInfoView.getWidget());
    m_AlbumInfoView.setAlbum(albumId);
    signalAlbumChanged.emit(albumId); 
    m_Layout.show_all_children();
}

void SimpleAlbumLayout::showAlbums()
{
    m_Layout.remove(m_AlbumInfoView.getWidget());
    m_Layout.add(m_AlbumQueuePane);
    
    m_Layout.show_all_children();
}

Gtk::Widget& SimpleAlbumLayout::getWidget()
{
    return m_Layout;
}

void SimpleAlbumLayout::getPlugins(std::vector<GejengelPlugin*>& plugins)
{
    plugins.push_back(&m_NowPlaying);
}

}
