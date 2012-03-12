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

#include "pluginview.h"

#include "utils/log.h"
#include "utils/numericoperations.h"
#include "Core/gejengelplugin.h"
#include "Core/pluginmanager.h"
#include "pluginwidget.h"

#include <glibmm/i18n.h>

using namespace std;
using namespace utils;

namespace Gejengel
{

PluginView::PluginView(PluginManager& pluginMgr, Settings& settings)
: m_Plugins(false, 5)
, m_PluginMgr(pluginMgr)
, m_Settings(settings)
{
    set_policy(Gtk::POLICY_AUTOMATIC, Gtk::POLICY_AUTOMATIC);
    set_shadow_type(Gtk::SHADOW_NONE);

    for (std::map<GejengelPlugin*, bool>::const_iterator iter = pluginMgr.getPlugins().begin(); iter != pluginMgr.getPlugins().end(); ++iter)
    {
        PluginWidget* pWidget = new PluginWidget(*(iter->first), iter->second);
        pWidget->signalConfigurePlugin.connect(sigc::mem_fun(this, &PluginView::onConfigurePlugin));
        pWidget->signalPluginToggled.connect(sigc::mem_fun(this, &PluginView::onPluginToggled));

        m_Plugins.pack_start(*Gtk::manage(pWidget), Gtk::PACK_SHRINK);
    }

    m_Plugins.set_size_request(get_width(), -1);

    set_border_width(5);
    set_shadow_type(Gtk::SHADOW_NONE);
    m_Plugins.set_border_width(5);

    add(m_Plugins);
}

void PluginView::onConfigurePlugin(GejengelPlugin& plugin)
{
    plugin.showSettingsDialog(m_Settings);
}

bool PluginView::onPluginToggled(GejengelPlugin& plugin, bool enabled)
{
    return m_PluginMgr.setPluginEnabled(&plugin, enabled);
}

}
