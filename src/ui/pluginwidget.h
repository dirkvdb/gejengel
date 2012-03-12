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

#ifndef PLUGIN_WIDGET_H
#define PLUGIN_WIDGET_H

#include <gtkmm.h>

#include "signals.h"

namespace Gejengel
{

class GejengelPlugin;

class PluginWidget : public Gtk::HBox
{
public:
    PluginWidget(GejengelPlugin& plugin, bool enabled);

    typedef sigc::signal<void, GejengelPlugin&> SignalConfigurePlugin;
    typedef sigc::signal<bool, GejengelPlugin&, bool> SignalPluginToggled;

    SignalConfigurePlugin signalConfigurePlugin;
    SignalPluginToggled signalPluginToggled;
private:

    void onEnabledToggled();
    void onShowProperties();

    Gtk::Image                  m_Icon;
    Gtk::Label                  m_InfoLabel;
    Gtk::Button                 m_SettingsButton;
    Gtk::CheckButton            m_EnabledButton;
    GejengelPlugin&             m_Plugin;
};

}

#endif
