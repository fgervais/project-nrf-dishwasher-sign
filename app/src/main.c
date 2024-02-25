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


int suspend_display(const struct device *display_dev,
		    const struct device *display_bus_dev)
{
	int ret;

	ret = pm_device_action_run(display_dev, PM_DEVICE_ACTION_SUSPEND);
	if (ret < 0) {
		LOG_ERR("Could not suspend the display");
		return ret;
	}

	ret = pm_device_action_run(display_bus_dev, PM_DEVICE_ACTION_SUSPEND);
	if (ret < 0) {
		LOG_ERR("Could not suspend the display bus");
		return ret;
	}

	return 0;
}

int resume_display(const struct device *display_dev,
		   const struct device *display_bus_dev)
{
	int ret;

	ret = pm_device_action_run(display_dev, PM_DEVICE_ACTION_RESUME);
	if (ret < 0) {
		LOG_ERR("Could not resume the display");
		return ret;
	}

	ret = pm_device_action_run(display_bus_dev, PM_DEVICE_ACTION_RESUME);
	if (ret < 0) {
		LOG_ERR("Could not resume the display bus");
		return ret;
	}

	return 0;
}

int main(void)
{
	const struct device *wdt = DEVICE_DT_GET(DT_NODELABEL(wdt0));
#if defined(CONFIG_APP_SUSPEND_CONSOLE)
	const struct device *cons = DEVICE_DT_GET(DT_CHOSEN(zephyr_console));
#endif
	const struct device *display_dev = DEVICE_DT_GET(DT_CHOSEN(zephyr_display));
	const struct device *display_bus_dev = DEVICE_DT_GET(DT_NODELABEL(spi0));

	lv_obj_t *label;
	uint32_t events;
	int main_wdt_chan_id = -1;
	int screen_text_pos = 0;
	const char *label_texts[] = {"Ready!", "Cleaning..."};
	k_timepoint_t screen_refresh_timepoint;


	init_watchdog(wdt, &main_wdt_chan_id);

	LOG_INF("\n\n🚀 MAIN START (%s) 🚀\n", APP_VERSION_FULL);

	if (app_event_manager_init()) {
		LOG_ERR("Event manager not initialized");
	} else {
		module_set_state(MODULE_STATE_READY);
	}

	label = lv_label_create(lv_scr_act());

	lv_label_set_text(label, label_texts[screen_text_pos]);
	lv_obj_align(label, LV_ALIGN_CENTER, 0, -10);

	// lv_obj_clean(lv_scr_act());

	lv_task_handler();
	screen_refresh_timepoint = sys_timepoint_calc(K_HOURS(12));

	suspend_display(display_dev, display_bus_dev);

	LOG_INF("🎉 init done 🎉");

#if defined(CONFIG_APP_SUSPEND_CONSOLE)
	k_sleep(K_SECONDS(3));
	pm_device_action_run(cons, PM_DEVICE_ACTION_SUSPEND);
#endif

	thread_analyzer_print();

	while (1) {
		LOG_INF("💤 waiting for events");
		events = k_event_wait(&screen_events,
				(SCREEN_TOGGLE_EVENT),
				true,
				K_SECONDS(CONFIG_APP_MAIN_LOOP_PERIOD_SEC));

		if (events & SCREEN_TOGGLE_EVENT) {
			resume_display(display_dev, display_bus_dev);

			screen_text_pos ^= 1;
			lv_label_set_text(label, label_texts[screen_text_pos]);
			lv_task_handler();
			screen_refresh_timepoint = sys_timepoint_calc(
							K_HOURS(12));

			suspend_display(display_dev, display_bus_dev);
		}
		else if (sys_timepoint_expired(screen_refresh_timepoint)) {
			resume_display(display_dev, display_bus_dev);

			LOG_INF("🖥️ screen saver refresh");
			lv_obj_invalidate(lv_scr_act());
			lv_task_handler();
			screen_refresh_timepoint = sys_timepoint_calc(
							K_HOURS(12));

			suspend_display(display_dev, display_bus_dev);
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
