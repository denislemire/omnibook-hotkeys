#include <sys/types.h>
#include <fcntl.h>
#include <pulse/pulseaudio.h>
#include <string.h>
#include <unistd.h>

static const char source[] = "oss_input.dsp0";
static const char led[] = "/dev/led/omnibook-micmute";

static void
sync_led(pa_context *ctx, const pa_source_info *info, int end, void *data)
{
	int fd;
	char state;

	if (end != 0 || info == NULL || strcmp(info->name, source) != 0)
		return;
	state = info->mute ? '1' : '0';
	fd = open(led, O_WRONLY | O_CLOEXEC);
	if (fd < 0 || write(fd, &state, 1) != 1)
		pa_mainloop_quit(data, 1);
	if (fd >= 0)
		close(fd);
	(void)ctx;
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
source_changed(pa_context *ctx, pa_subscription_event_type_t event,
    uint32_t index, void *data)
{
	if ((event & PA_SUBSCRIPTION_EVENT_TYPE_MASK) != PA_SUBSCRIPTION_EVENT_REMOVE)
		release_op(pa_context_get_source_info_by_index(ctx, index, sync_led,
		    data), data);
}

static void
subscribed(pa_context *ctx, int success, void *data)
{
	if (success)
		release_op(pa_context_get_source_info_by_name(ctx, source, sync_led,
		    data), data);
	else
		pa_mainloop_quit(data, 1);
}

static void
connection_changed(pa_context *ctx, void *data)
{
	switch (pa_context_get_state(ctx)) {
	case PA_CONTEXT_READY:
		pa_context_set_subscribe_callback(ctx, source_changed, data);
		release_op(pa_context_subscribe(ctx, PA_SUBSCRIPTION_MASK_SOURCE,
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
		    "OmniBook microphone LED");
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
