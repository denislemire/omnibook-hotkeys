#include <sys/types.h>
#include <fcntl.h>
#include <pulse/pulseaudio.h>
#include <string.h>
#include <unistd.h>

static const char source[] = "oss_input.dsp0";
static const char mic_led[] = "/dev/led/omnibook-micmute";

static void
write_led(const char *path, int muted, pa_mainloop *loop)
{
	char state = muted ? '1' : '0';
	int fd = open(path, O_WRONLY | O_CLOEXEC);

	if (fd < 0 || write(fd, &state, 1) != 1)
		pa_mainloop_quit(loop, 1);
	if (fd >= 0)
		close(fd);
}

static void
sync_led(pa_context *ctx, const pa_source_info *info, int end, void *data)
{
	(void)ctx;
	if (end == 0 && info != NULL && strcmp(info->name, source) == 0)
		write_led(mic_led, info->mute, data);
}

static void
sync_speaker(pa_context *ctx, const pa_sink_info *info, int end, void *data)
{
	(void)ctx;
	if (end == 0 && info != NULL)
		write_led("/dev/led/omnibook-speaker-mute", info->mute, data);
}

static void
release_op(pa_operation *op, pa_mainloop *loop)
{
	if (op != NULL)
		pa_operation_unref(op);
	else
		pa_mainloop_quit(loop, 1);
}

static void
audio_changed(pa_context *ctx, pa_subscription_event_type_t event,
    uint32_t index, void *data)
{
	if ((event & PA_SUBSCRIPTION_EVENT_FACILITY_MASK) != PA_SUBSCRIPTION_EVENT_SOURCE) {
		release_op(pa_context_get_sink_info_by_name(ctx, "@DEFAULT_SINK@",
		    sync_speaker, data), data);
		return;
	}
	if ((event & PA_SUBSCRIPTION_EVENT_TYPE_MASK) != PA_SUBSCRIPTION_EVENT_REMOVE)
		release_op(pa_context_get_source_info_by_index(ctx, index, sync_led,
		    data), data);
}

static void
subscribed(pa_context *ctx, int success, void *data)
{
	if (success) {
		release_op(pa_context_get_sink_info_by_name(ctx, "@DEFAULT_SINK@",
		    sync_speaker, data), data);
		release_op(pa_context_get_source_info_by_name(ctx, source, sync_led,
		    data), data);
	} else
		pa_mainloop_quit(data, 1);
}

static void
connection_changed(pa_context *ctx, void *data)
{
	switch (pa_context_get_state(ctx)) {
	case PA_CONTEXT_READY:
		pa_context_set_subscribe_callback(ctx, audio_changed, data);
		release_op(pa_context_subscribe(ctx, PA_SUBSCRIPTION_MASK_SOURCE |
		    PA_SUBSCRIPTION_MASK_SINK | PA_SUBSCRIPTION_MASK_SERVER,
		    subscribed, data), data);
		break;
	case PA_CONTEXT_FAILED:
	case PA_CONTEXT_TERMINATED:
		pa_mainloop_quit(data, 1);
		break;
	default:
		break;
	}
}

int
main(void)
{
	for (;;) {
		pa_mainloop *loop = pa_mainloop_new();
		if (loop == NULL)
			return (1);
		pa_context *ctx = pa_context_new(pa_mainloop_get_api(loop),
		    "OmniBook audio mute LEDs");
		if (ctx == NULL)
			return (1);
		pa_context_set_state_callback(ctx, connection_changed, loop);
		if (pa_context_connect(ctx, NULL, PA_CONTEXT_NOFLAGS, NULL) == 0)
			pa_mainloop_run(loop, NULL);
		pa_context_disconnect(ctx);
		pa_context_unref(ctx);
		pa_mainloop_free(loop);
		sleep(3);
	}
}
