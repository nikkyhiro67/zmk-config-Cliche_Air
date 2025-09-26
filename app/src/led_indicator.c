#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <zephyr/device.h>
#include <zephyr/drivers/gpio.h>

#include "battery_voltage.h"
#include "ext_power_transient.h"
#include "led_stripnize.h"
#include "led_indicator.h"

#ifdef CONFIG_BT
#include <bluetooth/conn.h>
#include <bluetooth/bluetooth.h>
#endif

LOG_MODULE_REGISTER(led_indicator);

/* LED modes */
enum led_mode { LED_MODE_NONE=0, LED_MODE_ON, LED_MODE_BLINK, LED_MODE_FLASH2 };

struct led_state { uint8_t r,g,b; enum led_mode mode; int period_ms; int flash_count; };

static struct led_state active = {0,0,0, LED_MODE_NONE, 0, 0};
static struct k_timer blink_timer;
static bool blink_on = false;
static int current_batt_mv = -1;
static bool ble_connected = false;
static int active_layer = 0;
static bool battery_critical_shutdown = false;

static void apply_led_state(void);
static void blink_timer_handler(struct k_timer *t);
static void start_blink_timer(int period_ms);
static void update_battery_state(void);
static void recompute_indicator(void);

static void set_color_now(uint8_t r, uint8_t g, uint8_t b)
{
    if (battery_critical_shutdown) {
        led_strip_off();
        return;
    }
    ext_power_enable(true);
    led_strip_set_rgb(r,g,b);
}

static void apply_led_state(void)
{
    switch (active.mode) {
    case LED_MODE_NONE: led_strip_off(); break;
    case LED_MODE_ON: set_color_now(active.r, active.g, active.b); break;
    case LED_MODE_BLINK: if (blink_on) set_color_now(active.r, active.g, active.b); else led_strip_off(); break;
    case LED_MODE_FLASH2:
        if (active.flash_count > 0) {
            if (blink_on) set_color_now(active.r, active.g, active.b);
            else { led_strip_off(); active.flash_count--; }
        } else {
            led_strip_off();
            ext_power_enable(false);
            battery_critical_shutdown = true;
            active.mode = LED_MODE_NONE;
        }
        break;
    default: led_strip_off(); break;
    }
}

static void blink_timer_handler(struct k_timer *t) { ARG_UNUSED(t); blink_on = !blink_on; apply_led_state(); }

static void start_blink_timer(int period_ms)
{
    if (period_ms <= 0) { k_timer_stop(&blink_timer); blink_on = false; return; }
    k_timer_start(&blink_timer, K_MSEC(period_ms/2), K_MSEC(period_ms/2));
}

static void update_battery_state(void)
{
    int mv = 0;
    if (battery_voltage_read_mv(&mv) != 0) return;
    current_batt_mv = mv;
    LOG_DBG("Battery: %d mV", mv);

    if (battery_critical_shutdown) { active.mode = LED_MODE_NONE; start_blink_timer(0); return; }

    if (mv > 3700) {
        active.r = 0; active.g = 255; active.b = 0;
        active.mode = LED_MODE_ON; start_blink_timer(0); apply_led_state();
    } else if (mv > 3500) {
        active.r = 255; active.g = 255; active.b = 0;
        active.mode = LED_MODE_BLINK; active.period_ms = 1000; start_blink_timer(1000);
    } else if (mv > 3300) {
        active.r = 255; active.g = 0; active.b = 0;
        active.mode = LED_MODE_BLINK; active.period_ms = 500; start_blink_timer(500);
    } else {
        active.r = 255; active.g = 0; active.b = 0;
        active.mode = LED_MODE_FLASH2; active.period_ms = 500; active.flash_count = 4; start_blink_timer(500);
    }
}

#ifdef CONFIG_BT
static void bt_connected(struct bt_conn *conn, uint8_t err) { ARG_UNUSED(conn); if (err==0) { ble_connected = true; if (current_batt_mv <= 0) { active.r=0;active.g=0;active.b=255; active.mode = LED_MODE_ON; apply_led_state(); } } }
static void bt_disconnected(struct bt_conn *conn, uint8_t reason) { ARG_UNUSED(conn); ARG_UNUSED(reason); ble_connected = false; if (current_batt_mv <= 0) { active.mode = LED_MODE_NONE; apply_led_state(); } }
static struct bt_conn_cb conn_callbacks = { .connected = bt_connected, .disconnected = bt_disconnected };
#endif

static void recompute_indicator(void)
{
    if (current_batt_mv > 0) return;
    if (ble_connected) { active.r=0;active.g=0;active.b=255; active.mode=LED_MODE_ON; apply_led_state(); return; }

    switch (active_layer) {
    case 0: active.r=255;active.g=255;active.b=255; active.mode=LED_MODE_ON; break;
    case 1: active.r=128;active.g=0;active.b=128; active.mode=LED_MODE_ON; break;
    case 2: active.r=0;active.g=255;active.b=255; active.mode=LED_MODE_ON; break;
    case 3: active.r=255;active.g=165;active.b=0; active.mode=LED_MODE_ON; break;
    default: active.mode=LED_MODE_NONE; break;
    }
    apply_led_state();
}

void led_indicator_set_layer(uint8_t layer) { active_layer = layer; if (current_batt_mv <= 0 && !ble_connected && !battery_critical_shutdown) recompute_indicator(); }
void led_indicator_update_now(void) { update_battery_state(); recompute_indicator(); }

K_THREAD_STACK_DEFINE(led_thread_stack, 1024);
static struct k_thread led_thread_data;

static void led_thread(void *p1, void *p2, void *p3)
{
    ARG_UNUSED(p1); ARG_UNUSED(p2); ARG_UNUSED(p3);
    while (1) {
        update_battery_state();
        recompute_indicator();
        k_msleep(1000);
    }
}

static int led_indicator_init(const struct device *dev)
{
    ARG_UNUSED(dev);
    ext_power_init();
    led_strip_init();
    battery_voltage_init();
#ifdef CONFIG_BT
    bt_conn_cb_register(&conn_callbacks);
#endif
    k_timer_init(&blink_timer, blink_timer_handler, NULL);
    k_thread_create(&led_thread_data, led_thread_stack, K_THREAD_STACK_SIZEOF(led_thread_stack),
                    led_thread, NULL, NULL, NULL, 5, 0, K_NO_WAIT);
    return 0;
}
SYS_INIT(led_indicator_init, APPLICATION, CONFIG_APPLICATION_INIT_PRIORITY);
