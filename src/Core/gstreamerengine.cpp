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

#include "gstreamerengine.h"

#include "gejengel.h"
#include "utils/log.h"

#include <gstreamermm/alsasink.h>
#include <gstreamermm/bus.h>

using namespace std;

namespace Gejengel
{

GStreamerEngine::GStreamerEngine(GejengelCore& core)
: PlaybackEngine(core)
, m_PlayBin(Gst::PlayBin2::create("playbin"))
, m_WatchId(0)
{
    if(!m_PlayBin)
    {
        throw(logic_error("Failed to create the gstreamer playbin"));
    }

    Glib::RefPtr<Gst::Bus> bus = m_PlayBin->get_bus();
    m_WatchId = bus->add_watch(sigc::mem_fun(*this, &GStreamerEngine::onBusMessage));

    m_PlayBin->signal_about_to_finish().connect(sigc::mem_fun(*this, &GStreamerEngine::onAboutTofinish));
}


GStreamerEngine::~GStreamerEngine()
{
    if (m_PlayBin)
    {
        m_PlayBin->set_state(Gst::STATE_NULL);
    }
}

bool GStreamerEngine::onBusMessage(const Glib::RefPtr<Gst::Bus>& bus, const Glib::RefPtr<Gst::Message>& message)
{
    switch (message->get_message_type())
    {
    case Gst::MESSAGE_EOS:
        log::debug("End of stream");
        stop();
        break;
    case Gst::MESSAGE_ERROR:
    {
        Glib::RefPtr<Gst::MessageError> msgError = Glib::RefPtr<Gst::MessageError>::cast_dynamic(message);
        log::error(msgError ? msgError->parse().what() : "GStreamer error occured");
        stop();
        break;
    }
    case Gst::MESSAGE_WARNING:
    {
        Glib::RefPtr<Gst::MessageWarning> msgWarn = Glib::RefPtr<Gst::MessageWarning>::cast_dynamic(message);
        log::warn(msgWarn ? msgWarn->parse().what() : "GStreamer warning occured");
        break;
    }
    //case Gst::MESSAGE_STATE_CHANGED:
    //{
        //Glib::RefPtr<Gst::MessageStateChanged> stateChangeMsg = Glib::RefPtr<Gst::MessageStateChanged>::cast_dynamic(message);
        //if (stateChangeMsg)
        //{
            //Gst::State oldState, newState, pendingState;
            //stateChangeMsg->parse(oldState, newState, pendingState);
            //log::debug("State changed from", getStateString(oldState), "to", getStateString(newState));
            ////log::debug("State changed from", getStateString(stateChangeMsg->parse_old()), "to", getStateString(stateChangeMsg->parse()));
        //}
        //break;
    //}
    case Gst::MESSAGE_DURATION:
    {
        Glib::RefPtr<Gst::MessageDuration> durationMsg = Glib::RefPtr<Gst::MessageDuration>::cast_dynamic(message);
        if (durationMsg)
        {
            log::debug("Duration:", durationMsg->parse() / Gst::SECOND);
        }
        break;
    }
    default:
        //log::debug("Ignored message", message->get_message_type());
        break;
    }

    return true;
}

bool GStreamerEngine::onTimeout()
{
    m_Core.dispatchProgress();
    return true;
}

void GStreamerEngine::onAboutTofinish()
{
    log::debug(__FUNCTION__);
    startNewTrack();
}


bool GStreamerEngine::startNewTrack()
{
    if (!m_Core.getNextTrack(m_CurrentTrack))
    {
        log::debug("End of playqueue");
        return false;
    }

    m_PlayBin->property_uri() = Glib::filename_to_uri(m_CurrentTrack.filepath);

    if (!setState(Gst::STATE_PLAYING))
    {
        return false;
    }

    //update track with gstreamers duration (is often different from taglibs duration)
    //remark: disabled because gstreamers duration seems even worse
    //m_CurrentTrack.durationInSec = static_cast<uint32_t>(getDuration());
    m_Core.dispatchNewTrackStarted();

    return true;
}

void GStreamerEngine::play()
{
    log::debug(__FUNCTION__);
    if (startNewTrack())
    {
        onTimeout();
        m_TimeoutConnection = Glib::signal_timeout().connect_seconds(sigc::mem_fun(*this, &GStreamerEngine::onTimeout), 1);
    }
}

void GStreamerEngine::pause()
{
    setState(Gst::STATE_PAUSED);
}

void GStreamerEngine::resume()
{
    setState(Gst::STATE_PLAYING);
}

void GStreamerEngine::stop()
{
    setState(Gst::STATE_READY);
    m_TimeoutConnection.disconnect();
}

void GStreamerEngine::next()
{
    stop();
    play();
}

void GStreamerEngine::prev()
{
}

bool GStreamerEngine::isPaused()
{
    return getState() == Gst::STATE_PAUSED;
}

bool GStreamerEngine::isPlaying()
{
    return getState() == Gst::STATE_PLAYING;
}

Gst::State GStreamerEngine::getState()
{
    Gst::State state;
    Gst::State pending;
    Gst::StateChangeReturn ret = m_PlayBin->get_state(state, pending, 1 * Gst::SECOND);

    if (ret == Gst::STATE_CHANGE_SUCCESS)
    {
        log::debug("State =", getStateString(state));
        return state;
    }
    else if (ret == Gst::STATE_CHANGE_ASYNC)
    {
        log::debug("Query state failed, still performing change");
    }
    else
    {
        log::debug("Query state failed, hard failure");
    }

    return Gst::STATE_NULL;
}

bool GStreamerEngine::setState(Gst::State state)
{
    Gst::StateChangeReturn ret = m_PlayBin->set_state(state);

    if (ret == Gst::STATE_CHANGE_SUCCESS)
    {
        log::debug("State =", getStateString(state));
    }
    else if (ret == Gst::STATE_CHANGE_ASYNC)
    {
        Gst::State actualState = getState();
        if (state != actualState)
        {
            log::error("Failed to set state to", getStateString(state), ". Actual state =", getStateString(actualState));
            return false;
        }
    }
    else if (ret == Gst::STATE_CHANGE_FAILURE || ret == Gst::STATE_CHANGE_NO_PREROLL)
    {
        log::error("Failed to set state to", getStateString(state), ": hard failure");
        return false;
    }

    return true;
}

std::string GStreamerEngine::getStateString(Gst::State state)
{
    switch (state)
    {
    case Gst::STATE_VOID_PENDING:
        return "VOID_PENDING";
    case Gst::STATE_NULL:
        return "NULL";
    case Gst::STATE_READY:
        return "READY";
    case Gst::STATE_PAUSED:
        return "PAUSED";
    case Gst::STATE_PLAYING:
        return "PLAYING";
    default:
        return "UNKNOWN";
    }
}

void GStreamerEngine::seek(double seconds)
{
    if(!m_PlayBin->seek(Gst::FORMAT_TIME, Gst::SEEK_FLAG_FLUSH | Gst::SEEK_FLAG_KEY_UNIT, static_cast<gint64>(seconds * Gst::SECOND)))
    {
        log::error("Failed to seek to position:", seconds);
    }
}

double GStreamerEngine::getCurrentTime()
{
    gint64 pos = 0;
    Gst::Format format = Gst::FORMAT_TIME;

    if (m_PlayBin->query_position(format, pos))
    {
        return static_cast<double>(pos / Gst::SECOND);
    }

    log::error("query_position failed");
    return 0.0;
}

double GStreamerEngine::getDuration()
{
    Gst::Format format = Gst::FORMAT_TIME;
    gint64 duration;

    if (m_PlayBin->query_duration(format, duration))
    {
        if (format == Gst::FORMAT_TIME)
        {
            log::debug("duration", duration, duration / Gst::SECOND);
            return static_cast<double>(duration / Gst::SECOND);
        }
        else
        {
            log::warn("getDuration: Could not get requested format");
        }
    }

    return 0.0;
}

void GStreamerEngine::setVolume(int32_t volume)
{
    m_PlayBin->property_volume() = static_cast<double>(volume / 100.0);
    m_Core.dispatchVolumeChanged();
}

int32_t GStreamerEngine::getVolume() const
{
    return static_cast<int32_t>(m_PlayBin->property_volume() * 100);
}

const Track& GStreamerEngine::getTrack() const
{
    return m_CurrentTrack;
}

}
