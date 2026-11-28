/* SPDX-License-Identifier: BSD-2-Clause */
#include <sys/types.h>
#include <sys/sysctl.h>
#include <gio/gio.h>
#include <st/st.h>

typedef struct {
	StLabel *label;
	GCancellable *cancel;
	guint timer;
	gboolean pending;
} Indicator;

static void
read_level(GTask *task, gpointer object, gpointer data, GCancellable *cancel)
{
	int level;
	size_t size = sizeof(level);

	if (sysctlbyname("hw.omnibook_kbdlight_percent", &level, &size, NULL, 0)
	    || size != sizeof(level) || (level != 0 && level != 50 && level != 100))
		level = -1;
	g_task_return_int(task, level);
}

static void
level_ready(GObject *box, GAsyncResult *result, gpointer data)
{
	Indicator *indicator = data;
	int level = g_task_propagate_int(G_TASK(result), NULL);

	indicator->pending = FALSE;
	if (g_cancellable_is_cancelled(indicator->cancel))
		return;
	if (level >= 0) {
		char text[8];
		g_snprintf(text, sizeof(text), "%d%%", level);
		st_label_set_text(indicator->label, text);
	}
	g_object_set(box, "visible", level >= 0, NULL);
}

static gboolean
refresh(gpointer box)
{
	Indicator *indicator = g_object_get_data(box, "kbdlight");

	if (!indicator->pending) {
		GTask *task = g_task_new(box, indicator->cancel, level_ready, indicator);
		indicator->pending = TRUE;
		g_task_run_in_thread(task, read_level);
		g_object_unref(task);
	}
	return G_SOURCE_CONTINUE;
}

static void
on_destroy(ClutterActor *box, gpointer data)
{
	Indicator *indicator = data;
	g_clear_handle_id(&indicator->timer, g_source_remove);
	g_cancellable_cancel(indicator->cancel);
}

static void
free_indicator(gpointer data)
{
	Indicator *indicator = data;
	g_object_unref(indicator->cancel);
	g_free(indicator);
}

ClutterActor *
omnibook_keyboard_indicator_new(void)
{
	Indicator *indicator = g_new0(Indicator, 1);
	ClutterActor *box = g_object_new(ST_TYPE_BOX_LAYOUT, "visible", FALSE,
	    "style-class", "panel-status-indicators-box", NULL);
	ClutterActor *icon = g_object_new(ST_TYPE_ICON, "icon-name",
	    "keyboard-brightness-symbolic", "style-class", "system-status-icon", NULL);

	indicator->label = ST_LABEL(st_label_new(""));
	indicator->cancel = g_cancellable_new();
	clutter_actor_set_y_align(CLUTTER_ACTOR(indicator->label), CLUTTER_ACTOR_ALIGN_CENTER);
	clutter_actor_add_child(box, icon);
	clutter_actor_add_child(box, CLUTTER_ACTOR(indicator->label));
	g_object_set_data_full(G_OBJECT(box), "kbdlight", indicator, free_indicator);
	g_signal_connect(box, "destroy", G_CALLBACK(on_destroy), indicator);
	indicator->timer = g_timeout_add(600, refresh, box);
	refresh(box);
	return box;
}
