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

#ifndef GEJENGEL_PLUGIN_H
#define GEJENGEL_PLUGIN_H

#include <gdkmm/pixbuf.h>

#include "MusicLibrary/track.h"
#include "gejengelcore.h"

namespace Gejengel
{

class GejengelPlugin
{
public:
    GejengelPlugin();
    virtual ~GejengelPlugin();

    virtual bool initialize(IGejengelCore& core) = 0;

    virtual std::string getName() const = 0;
    virtual std::string getDescription() const = 0;

    virtual void onPlay(const Track& track) {};
    virtual void onPause() {};
    virtual void onResume() {};
    virtual void onStop() {};
    virtual void onProgress(int32_t elapsedSeconds) {};
    virtual void onVolumeChanged(int32_t volume) {};
    virtual Glib::RefPtr<Gdk::Pixbuf> getIcon() const;

    virtual void destroy() = 0;

    bool hasSettingsDialog() const;
    virtual void showSettingsDialog(Settings& settings);

protected:
    bool m_HasSettingsDialog;
};

}

#endif
