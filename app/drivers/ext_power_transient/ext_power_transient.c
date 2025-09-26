#include "ext_power_transient.h"
#include <zephyr/device.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/logging/log.h>
#include <zephyr/devicetree.h>

LOG_MODULE_REGISTER(ext_power);

#define EXT_POWER_NODE DT_NODELABEL(ext_power)

#if !DT_NODE_HAS_STATUS(EXT_POWER_NODE, okay)
#  error "Missing ext_power node"
#endif

static const struct gpio_dt_spec ext_power_gpio = GPIO_DT_SPEC_GET(EXT_POWER_NODE, gpios);
static bool initialized = false;

int ext_power_init(void)
{
    if (!device_is_ready(ext_power_gpio.port)) {
        LOG_ERR("ext_power gpio port not ready");
        return -ENODEV;
    }
    int rc = gpio_pin_configure_dt(&ext_power_gpio, GPIO_OUTPUT_INACTIVE);
    if (rc) {
        LOG_ERR("Failed to configure ext_power gpio: %d", rc);
        return rc;
    }
    initialized = true;
    LOG_INF("ext_power initialized (pin %d)", ext_power_gpio.pin);
    return 0;
}

int ext_power_enable(bool on)
{
    if (!initialized) {
        int rc = ext_power_init();
        if (rc) return rc;
    }
    return gpio_pin_set_dt(&ext_power_gpio, on ? 1 : 0);
}
