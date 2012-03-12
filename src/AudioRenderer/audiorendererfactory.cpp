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

#include "audiorendererfactory.h"

#include "config.h"

#ifdef WIN32
#include "winconfig.h"
#endif

#ifdef HAVE_OPENAL
#include "openalrenderer.h"
#endif

#ifdef HAVE_ALSA
#include "alsarenderer.h"
#endif

#ifdef HAVE_PULSE
#include "pulserenderer.h"
#endif

#include <stdexcept>

namespace Gejengel
{

AudioRenderer* AudioRendererFactory::create(const std::string& audioBackend, PlaybackEngine& engine)
{
    if (audioBackend == "OpenAL")
    {
#ifdef HAVE_OPENAL
        return new OpenALRenderer();
#else
        throw std::logic_error("AudioRendererFactory: package was not compiled with OpenAl support");
#endif
    }

    if (audioBackend == "Alsa")
    {
#ifdef HAVE_ALSA
        return new AlsaRenderer();
#else
        throw std::logic_error("AudioRendererFactory: package was not compiled with Alsa support");
#endif
    }

    if (audioBackend == "PulseAudio")
    {
#ifdef HAVE_PULSE
        return new PulseRenderer(engine);
#else
        throw std::logic_error("AudioRendererFactory: package was not compiled with PulseAudio support");
#endif
    }

    throw std::logic_error("AudioRendererFactory: Unsupported audio output type provided: " + audioBackend);
}

}
