#include <stdio.h>

#include <gst/gst.h>

static GstElement* playBin;
static GMainLoop* mainloop;
static GstBus* bus;
static gchar* filename = NULL;

static const char* STATE_VOID_PENDING_STR   = "VOID_PENDING";
static const char* STATE_NULL_STR           = "STATE_NULL";
static const char* STATE_READY_STR          = "STATE_READY";
static const char* STATE_PAUSED_STR         = "STATE_PAUSED";
static const char* STATE_PLAYING_STR        = "STATE_PLAYING";
static const char* STATE_UNKNOWN_STR        = "UNKNOWN";

static const char* getStateString(GstState state)
{
    switch (state)
    {
    case GST_STATE_VOID_PENDING:
        return STATE_VOID_PENDING_STR;
    case GST_STATE_NULL:
        return STATE_NULL_STR;
    case GST_STATE_READY:
        return STATE_READY_STR;
    case GST_STATE_PAUSED:
        return STATE_PAUSED_STR;
    case GST_STATE_PLAYING:
        return STATE_PLAYING_STR;
    default:
        return STATE_UNKNOWN_STR;
    }
}

GstState getState()
{
    GstState state;
    GstState pending;
    GstStateChangeReturn ret = gst_element_get_state(playBin, &state, &pending, 10 * GST_SECOND);

    if (ret == GST_STATE_CHANGE_SUCCESS)
    {
        printf("State = %s\n", getStateString(state));
        return state;
    }
    else if (ret == GST_STATE_CHANGE_ASYNC)
    {
        printf("Query state failed, still performing change\n");
    }
    else
    {
        printf("Query state failed, hard failure\n");
    }

    return GST_STATE_NULL;
}

void setState(GstState state)
{
    GstStateChangeReturn ret = gst_element_set_state(playBin, state);
    gst_element_set_state(playBin, state);

    if (ret == GST_STATE_CHANGE_SUCCESS)
    {
        printf("State = %s\n", getStateString(state));
    }
    else if (ret == GST_STATE_CHANGE_ASYNC)
    {
        GstState actualState = getState();
        if (state != actualState)
        {
            printf("Failed to set state to %s\n", getStateString(state));
            return;
        }
    }
    else if (ret == GST_STATE_CHANGE_FAILURE || ret == GST_STATE_CHANGE_NO_PREROLL)
    {
        printf("Failed to set state to %s hard failure\n", getStateString(state));
        return;
    }

    return;
}

gboolean onBusMessage(GstBus* bus, GstMessage* message, gpointer data)
{
    switch (message->type)
    {
    case GST_MESSAGE_EOS:
        printf("End of stream\n");
        setState(GST_STATE_READY);
        g_object_set(G_OBJECT (playBin), "uri", filename, NULL);
        setState(GST_STATE_PLAYING);
        break;
    case GST_MESSAGE_ERROR:
    {
        GError* error = NULL;
        gchar* debug = NULL;
        gst_message_parse_error(message, &error, &debug);
        printf("Error occured %s\n", error->message);
        g_error_free(error);
        g_free(debug);
        exit(0);
        break;
    }
    default:
        break;
    }

    return TRUE;
}

int main(int argc, char* argv[])
{
    gst_init(&argc, &argv);

    if (argc < 2)
    {
        printf("Usage: %s filename\n", argv[0]);
        return -1;
    }

    filename = argv[1];
    mainloop = g_main_loop_new(NULL, FALSE);

    playBin = gst_element_factory_make ("playbin2", "player");
    g_assert(playBin);
    bus = gst_element_get_bus(playBin);
    g_assert(bus);

    gst_bus_add_watch(bus, onBusMessage, NULL);

    filename = g_filename_to_uri(argv[1], NULL, NULL);
    g_assert(filename);
    
    g_object_set(G_OBJECT (playBin), "uri", filename, NULL);

    setState(GST_STATE_PLAYING);

    g_main_loop_run(mainloop);

    return 0;
}

