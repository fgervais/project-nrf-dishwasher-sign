#include <lvgl.h>
#include <zephyr/drivers/display.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/drivers/watchdog.h>
#include <zephyr/kernel.h>
#include <zephyr/pm/device.h>
#include <zephyr/debug/thread_analyzer.h>

#include <app_event_manager.h>

#define MODULE main
#include <caf/events/module_state_event.h>
#include <caf/events/button_event.h>

#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(main, LOG_LEVEL_DBG);

#include <app_version.h>

#include "init.h"


#define SCREEN_TOGGLE_EVENT		BIT(0)


static K_EVENT_DEFINE(screen_events);


int main(void)
{
	const struct device *wdt = DEVICE_DT_GET(DT_NODELABEL(wdt0));
#if defined(CONFIG_APP_SUSPEND_CONSOLE)
	const struct device *cons = DEVICE_DT_GET(DT_CHOSEN(zephyr_console));
#endif
	const struct device *display_dev = DEVICE_DT_GET(DT_CHOSEN(zephyr_display));

	lv_obj_t *hello_world_label;
	uint32_t events;
	int main_wdt_chan_id = -1;
	bool screen_is_ready = true;


	init_watchdog(wdt, &main_wdt_chan_id);

	LOG_INF("\n\n🚀 MAIN START (%s) 🚀\n", APP_VERSION_FULL);

	if (app_event_manager_init()) {
		LOG_ERR("Event manager not initialized");
	} else {
		module_set_state(MODULE_STATE_READY);
	}

	LOG_INF("🎉 init done 🎉");

#if defined(CONFIG_APP_SUSPEND_CONSOLE)
	pm_device_action_run(cons, PM_DEVICE_ACTION_SUSPEND);
#endif

	hello_world_label = lv_label_create(lv_scr_act());

	lv_label_set_text(hello_world_label, "Ready!");
	lv_obj_align(hello_world_label, LV_ALIGN_CENTER, 0, -10);

	// lv_obj_clean(lv_scr_act());

	lv_task_handler();
	display_blanking_off(display_dev);

	thread_analyzer_print();

	while (1) {
		LOG_INF("💤 waiting for events");
		events = k_event_wait(&screen_events,
				(SCREEN_TOGGLE_EVENT),
				true,
				K_SECONDS(CONFIG_APP_MAIN_LOOP_PERIOD_SEC));

		if (events & SCREEN_TOGGLE_EVENT) {
			if (screen_is_ready) {
				lv_label_set_text(hello_world_label, "Cleaning...");
				screen_is_ready = false;
			}
			else {
				lv_label_set_text(hello_world_label, "Ready!");
				screen_is_ready = true;
			}

			lv_task_handler();
		}

		LOG_INF("🦴 feed watchdog");
		wdt_feed(wdt, main_wdt_chan_id);
	}

	return 0;
}

static bool event_handler(const struct app_event_header *eh)
{
	const struct button_event *evt;

	if (is_button_event(eh)) {
		evt = cast_button_event(eh);

		if (evt->pressed) {
			LOG_INF("🛎️  Button pressed");
			k_event_post(&screen_events, SCREEN_TOGGLE_EVENT);
		}
	}

	return true;
}

APP_EVENT_LISTENER(MODULE, event_handler);
APP_EVENT_SUBSCRIBE(MODULE, button_event);
