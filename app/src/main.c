// #include <lvgl.h>
#include <zephyr/drivers/display.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/kernel.h>
#include <zephyr/pm/device.h>

#include <app_event_manager.h>

#define MODULE main
#include <caf/events/module_state_event.h>
#include <caf/events/button_event.h>

#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(main, LOG_LEVEL_DBG);

#include <app_version.h>


int main(void)
{
// #if defined(CONFIG_APP_SUSPEND_CONSOLE)
// 	const struct device *cons = DEVICE_DT_GET(DT_CHOSEN(zephyr_console));
// #endif
// 	const struct device *display_dev = DEVICE_DT_GET(DT_CHOSEN(zephyr_display));
// 	lv_obj_t *hello_world_label;


// 	if (app_event_manager_init()) {
// 		LOG_ERR("Event manager not initialized");
// 	} else {
// 		module_set_state(MODULE_STATE_READY);
// 	}

	LOG_INF("\n\n🚀 MAIN START (%s) 🚀\n", APP_VERSION_FULL);

	LOG_INF("🎉 init done 🎉");

	return 0;

// #if defined(CONFIG_APP_SUSPEND_CONSOLE)
// 	pm_device_action_run(cons, PM_DEVICE_ACTION_SUSPEND);
// #endif

// 	hello_world_label = lv_label_create(lv_scr_act());

// 	lv_label_set_text(hello_world_label, "Hello world!");
// 	lv_obj_align(hello_world_label, LV_ALIGN_CENTER, 0, 0);

// 	lv_task_handler();
// 	display_blanking_off(display_dev);

// 	return 0;
}

static bool event_handler(const struct app_event_header *eh)
{
	const struct button_event *evt;

	if (is_button_event(eh)) {
		evt = cast_button_event(eh);

		if (evt->pressed) {
			LOG_INF("Button pressed");
		}
	}

	return true;
}

APP_EVENT_LISTENER(MODULE, event_handler);
APP_EVENT_SUBSCRIBE(MODULE, button_event);
