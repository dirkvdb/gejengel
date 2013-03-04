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

#include "sharedfunctions.h"

#include <sstream>
#include <iomanip>

#include "overlay.h"
#include "utils/log.h"
#include "utils/stringoperations.h"
#include "MusicLibrary/album.h"
#include "MusicLibrary/albumart.h"

#include "config.h"

#include <Magick++.h>

using namespace std;
using namespace utils;

namespace Gejengel
{

namespace Shared
{

void durationToString(uint32_t duration, Glib::ustring& durationString)
{
    uint32_t hours   = duration / 3600;
    duration -= hours * 3600;

    uint32_t minutes = duration / 60;
    uint32_t seconds = duration % 60;


    stringstream ss;

    if (hours > 0)
    {
        ss << setfill('0') << hours << ':';
    }
    ss << setw(2) << setfill('0') << minutes << ':' << setw(2) << setfill('0')  << seconds;
    durationString = ss.str();
}

uint32_t durationFromString(const Glib::ustring& durationString)
{
    uint32_t duration = 0;
    std::vector<std::string> times = stringops::tokenize(durationString, ":");

    if (times.size() == 3)
    {
        uint32_t hours   = stringops::toNumeric<uint32_t>(times[0]);
        uint32_t minutes = stringops::toNumeric<uint32_t>(times[1]);
        uint32_t seconds = stringops::toNumeric<uint32_t>(times[2]);

        duration += hours * 3600;
        duration += minutes * 60;
        duration += seconds;
    }
    else if (times.size() == 2)
    {
        uint32_t minutes = stringops::toNumeric<uint32_t>(times[0]);
        uint32_t seconds = stringops::toNumeric<uint32_t>(times[1]);

        duration += minutes * 60;
        duration += seconds;
    }

    return duration;
}

Glib::RefPtr<Gdk::Pixbuf> createCoverPixBuf(const AlbumArt& albumArt, int32_t size)
{
    Glib::RefPtr<Gdk::PixbufLoader> loader = Gdk::PixbufLoader::create();
    loader->set_size(size, size);
    loader->write(&(albumArt.getData().front()), albumArt.getData().size());
    Glib::RefPtr<Gdk::Pixbuf> pixBuf = loader->get_pixbuf();
    loader->close();

    return pixBuf;
}

Glib::RefPtr<Gdk::Pixbuf> createCoverPixBufWithOverlay(const AlbumArt& albumArt, int32_t size)
{
    try
    {
        std::stringstream finalGeometry;
        finalGeometry << size << "x" << size;

        std::stringstream intermediateGeometry;
        intermediateGeometry << overlayWidth << "x" << overlayHeight << "!";
        
        std::stringstream albumArtGeometry;
        albumArtGeometry << coverWidth << "x" << coverHeight << "!";

        Magick::Blob coverImageBlob(&(albumArt.getData().front()), albumArt.getData().size());
        Magick::Image coverImage(coverImageBlob);
        coverImage.scale(Magick::Geometry(albumArtGeometry.str()));

        Magick::Blob cdCaseBlob(cdCaseData, sizeof(cdCaseData));
        Magick::Image cdCase(cdCaseBlob, Magick::Geometry(intermediateGeometry.str()), "PNG");

        Magick::Quantum maxRgb;
        {using namespace Magick; maxRgb = MaxRGB;} // Magick::MaxRGB; quits with the compile-time error "'Quantum' was not declared in this scope"
        Magick::Image result(Magick::Geometry(intermediateGeometry.str()), Magick::Color(0, 0, 0, maxRgb));
        result.composite(coverImage, coverImageOffsetX, coverImageOffsetY, Magick::CopyCompositeOp);
        result.composite(cdCase, 0, 0, Magick::OverCompositeOp);
        result.scale(Magick::Geometry(finalGeometry.str()));

        Magick::Blob data;
        result.write(&data, "PNG");

        Glib::RefPtr<Gdk::PixbufLoader> loader = Gdk::PixbufLoader::create();
        loader->set_size(size, size);
        loader->write((guint8*)data.data(), data.length());
        Glib::RefPtr<Gdk::Pixbuf> pixBuf = loader->get_pixbuf();
        loader->close();

        return pixBuf;
    }
    catch (Magick::Exception& e)
    {
        log::warn("Creating cover with overlay failed: %s", e.what());
        return createCoverPixBuf(albumArt, size);
    }
}

}

}
