#include <iostream>

#include <gstreamermm.h>
#include <gstreamermm/playbin2.h>

using namespace std;

Glib::RefPtr<Glib::MainLoop> mainloop;
Glib::RefPtr<Gst::PlayBin2> playBin;
Glib::RefPtr<Gst::Bus> bus;
std::string filename;

static std::string getStateString(Gst::State state)
{
    switch (state)
    {
    case Gst::STATE_VOID_PENDING:
        return "VOID_PENDING";
    case Gst::STATE_nullptr:
        return "nullptr";
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

Gst::State getState()
{
    Gst::State state;
    Gst::State pending;
    Gst::StateChangeReturn ret = playBin->get_state(state, pending, 1 * Gst::SECOND);

    if (ret == Gst::STATE_CHANGE_SUCCESS)
    {
        cout << "State = " << getStateString(state) << endl;
        return state;
    }
    else if (ret == Gst::STATE_CHANGE_ASYNC)
    {
        cerr << "Query state failed, still performing change" << endl;
    }
    else
    {
        cerr << "Query state failed, hard failure" << endl;
    }

    return Gst::STATE_nullptr;
}

bool setState(Gst::State state)
{
    Gst::StateChangeReturn ret = playBin->set_state(state);

    if (ret == Gst::STATE_CHANGE_SUCCESS)
    {
        cout << "State = " << getStateString(state) << endl;
    }
    else if (ret == Gst::STATE_CHANGE_ASYNC)
    {
        Gst::State actualState = getState();
        if (state != actualState)
        {
            cerr << "Failed to set state to " << getStateString(state) << endl;
            return false;
        }
    }
    else if (ret == Gst::STATE_CHANGE_FAILURE || ret == Gst::STATE_CHANGE_NO_PREROLL)
    {
        cerr << "Failed to set state to " << getStateString(state) << ": hard failure" << endl;
        return false;
    }

    return true;
}

bool onBusMessage(const Glib::RefPtr<Gst::Bus>& bus, const Glib::RefPtr<Gst::Message>& message)
{
    switch (message->get_message_type())
    {
    case Gst::MESSAGE_EOS:
        cout << "End of stream" << endl;
        setState(Gst::STATE_READY);
        playBin->property_uri() = Glib::filename_to_uri(filename);
        setState(Gst::STATE_PLAYING);
        break;
    case Gst::MESSAGE_STATE_CHANGED:
    {
        Glib::RefPtr<Gst::MessageStateChanged> stateChangeMsg = Glib::RefPtr<Gst::MessageStateChanged>::cast_dynamic(message);
        if (stateChangeMsg)
        {
            // This works fine
            //Gst::State oldState, newState, pendingState;
            //stateChangeMsg->parse(oldState, newState, pendingState);

            // This will segfault on my machine if uncommented
            //cout << "State changed from " << getStateString(stateChangeMsg->parse_old()) << " to " << getStateString(stateChangeMsg->parse()) << endl;
        }
        break;
    }
    default:
        break;
    }

    return true;
}

int main(int argc, char* argv[])
{
    Gst::init(argc, argv);

    if (argc < 2)
    {
        cout << "Usage: " << argv[0] << " filename" << endl;
        return -1;
    }

    filename = argv[1];
    mainloop = Glib::MainLoop::create();

    playBin = Gst::PlayBin2::create("playbin");
    bus = playBin->get_bus();
    bus->add_watch(sigc::ptr_fun(onBusMessage));

    playBin->property_uri() = Glib::filename_to_uri(filename);
    setState(Gst::STATE_PLAYING);

    mainloop->run();

    // Clean up nicely:
    cout << "Cleanup" << endl;
    setState(Gst::STATE_nullptr);

    return 0;
}

