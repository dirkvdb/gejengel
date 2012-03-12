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

#include "pluginwidget.h"

#include "utils/log.h"
#include "utils/numericoperations.h"
#include "Core/gejengelplugin.h"

#include <glibmm/i18n.h>

using namespace std;

namespace Gejengel
{

PluginWidget::PluginWidget(GejengelPlugin& plugin, bool enabled)
: m_Icon(plugin.getIcon())
, m_InfoLabel("", Gtk::ALIGN_LEFT)
, m_SettingsButton(Gtk::Stock::PROPERTIES)
, m_Plugin(plugin)
{
    m_InfoLabel.set_use_markup(true);
    m_InfoLabel.set_markup(Glib::ustring("<b>") + plugin.getName() + "</b>\n<i>" + plugin.getDescription() + "</i>");
    m_EnabledButton.set_active(enabled);

    m_EnabledButton.signal_toggled().connect(sigc::mem_fun(this, &PluginWidget::onEnabledToggled));

    pack_start(m_Icon, Gtk::PACK_SHRINK);
    pack_start(m_EnabledButton, Gtk::PACK_SHRINK);
    pack_start(m_InfoLabel, Gtk::PACK_EXPAND_WIDGET);

    if (plugin.hasSettingsDialog())
    {
        pack_end(m_SettingsButton, Gtk::PACK_SHRINK);
        m_SettingsButton.signal_clicked().connect(sigc::mem_fun(this, &PluginWidget::onShowProperties));
    }
}

void PluginWidget::onEnabledToggled()
{
    bool isActive = m_EnabledButton.get_active();

    if (!signalPluginToggled.emit(m_Plugin, isActive))
    {
        m_EnabledButton.set_active(!isActive);
    }
}

void PluginWidget::onShowProperties()
{
    signalConfigurePlugin.emit(m_Plugin);
}

}
