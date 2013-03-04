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

#include <csignal>
#include <cerrno>
#include <string>
#include <cstring>
#include <gtkmm/main.h>
#include <libintl.h>
#include <Magick++.h>

#include "config.h"

#ifdef HAVE_GSTREAMER
    #include <gstreamermm/init.h>
#endif

#include "Core/gejengel.h"
#include "utils/log.h"
#include "utils/trace.h"

static bool set_signal_handlers();

using namespace utils;

int main(int argc, char **argv)
{
    utils::trace("Application start");
    
#ifndef WIN32
    if (!set_signal_handlers())
    {
        return -1;
    }

#ifdef HAVE_GETTEXT
    log::info("%s %s", GETTEXT_PACKAGE, GEJENGEL_LOCALEDIR);
    bindtextdomain(GETTEXT_PACKAGE, GEJENGEL_LOCALEDIR);
    bind_textdomain_codeset(GETTEXT_PACKAGE, "UTF-8");
    textdomain(GETTEXT_PACKAGE);
#endif
#else
    Glib::thread_init();
    Magick::InitializeMagick(nullptr);
#endif

#ifdef HAVE_GSTREAMER
    Gst::init(argc, argv);
#endif

    Magick::InitializeMagick(*argv);

    Gtk::Main main(&argc, &argv);

    utils::trace("Create Gejengel core");
    Gejengel::GejengelCore core;
    
    utils::trace("Run gtk main");
    core.run();

    return 0;
}

static void sigterm(int signo)
{
    Gtk::Main::quit();
}

#ifndef WIN32
static bool set_signal_handlers()
{
    struct sigaction sa;

    sa.sa_flags = 0;
    sa.sa_handler = sigterm;
    sigemptyset(&sa.sa_mask);
    sigaddset(&sa.sa_mask, SIGQUIT);
    sigaddset(&sa.sa_mask, SIGTERM);

    if (sigaction(SIGINT, &sa, nullptr) < 0)
    {
        log::error("Can't catch SIGINT: %s", strerror(errno));
        return false;
    }

    sa.sa_flags = 0;
    sa.sa_handler = sigterm;
    sigemptyset(&sa.sa_mask);
    sigaddset(&sa.sa_mask, SIGINT);
    sigaddset(&sa.sa_mask, SIGTERM);

    if (sigaction(SIGQUIT, &sa, nullptr) < 0)
    {
        log::error("Can't catch SIGQUIT: %s", strerror(errno));
        return false;
    }

    sa.sa_handler = sigterm;
    sigemptyset(&sa.sa_mask);
    sigaddset(&sa.sa_mask, SIGINT);
    sigaddset(&sa.sa_mask, SIGQUIT);
    sa.sa_flags = 0;

    if (sigaction(SIGTERM, &sa, nullptr) < 0)
    {
        log::error("Can't catch SIGTERM: %s", strerror(errno));
        return false;
    }

    return true;
}

#endif
