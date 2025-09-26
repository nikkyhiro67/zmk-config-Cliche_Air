#include "led_stripnize.h"
#include <zephyr/device.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/logging/log.h>
#include <zephyr/devicetree.h>

LOG_MODULE_REGISTER(led_stripnize);

#define LED_STRIP_NODE DT_NODELABEL(led_strip)

#if !DT_NODE_HAS_STATUS(LED_STRIP_NODE, okay)
#  error "Missing led_strip node in device tree"
#endif

static const struct gpio_dt_spec led_strip_gpio = GPIO_DT_SPEC_GET(LED_STRIP_NODE, gpios);
static bool initialized = false;

int led_strip_init(void)
{
    if (!device_is_ready(led_strip_gpio.port)) {
        LOG_ERR("LED strip gpio port not ready");
        return -ENODEV;
    }
    int rc = gpio_pin_configure_dt(&led_strip_gpio, GPIO_OUTPUT_INACTIVE);
    if (rc) {
        LOG_ERR("Failed to configure led_strip gpio: %d", rc);
        return rc;
    }
    initialized = true;
    LOG_INF("led_stripnize initialized (pin %d)", led_strip_gpio.pin);
    return 0;
}

int led_strip_set_rgb(unsigned char r, unsigned char g, unsigned char b)
{
    if (!initialized) {
        int rc = led_strip_init();
        if (rc) return rc;
    }
    int on = (r | g | b) ? 1 : 0;
    return gpio_pin_set_dt(&led_strip_gpio, on);
}

int led_strip_off(void)
{
    return led_strip_set_rgb(0,0,0);
}
