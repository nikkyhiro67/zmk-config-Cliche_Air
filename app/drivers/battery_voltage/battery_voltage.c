#include "battery_voltage.h"
#include <zephyr/logging/log.h>
#include <zephyr/device.h>
#include <zephyr/drivers/adc.h>
#include <zephyr/sys/util.h>

LOG_MODULE_REGISTER(battery_voltage);

#define ADC_RESOLUTION 12
#define ADC_BUFFER_SIZE 1
#define BATTERY_ADC_CHANNEL 2
#define VOLTAGE_DIVIDER_SCALE_NUM 2u

static const struct device *adc_dev;
static bool initialized = false;
static uint16_t sample_buffer[ADC_BUFFER_SIZE];

static int battery_raw_to_mv(uint16_t raw)
{
    const uint32_t vref_mv = 3300u;
    uint32_t max = (1u << ADC_RESOLUTION) - 1u;
    uint32_t mv = ((uint32_t)raw * vref_mv) / max;
    mv = mv * VOLTAGE_DIVIDER_SCALE_NUM;
    return (int)mv;
}

int battery_voltage_init(void)
{
    adc_dev = DEVICE_DT_GET(DT_NODELABEL(adc));
    if (!device_is_ready(adc_dev)) {
        LOG_ERR("ADC device not ready");
        return -ENODEV;
    }

    struct adc_channel_cfg ch_cfg = {
        .gain = ADC_GAIN_1,
        .reference = ADC_REF_INTERNAL,
        .acquisition_time = ADC_ACQ_TIME_DEFAULT,
        .channel_id = BATTERY_ADC_CHANNEL,
    };

    int rc = adc_channel_setup(adc_dev, &ch_cfg);
    if (rc) {
        LOG_ERR("ADC channel setup failed: %d", rc);
        return rc;
    }

    initialized = true;
    LOG_INF("Battery voltage driver initialized (channel %d)", BATTERY_ADC_CHANNEL);
    return 0;
}

int battery_voltage_read_mv(int *mv)
{
    if (!mv) return -EINVAL;
    if (!initialized) {
        int rc = battery_voltage_init();
        if (rc) return rc;
    }

    struct adc_sequence seq = {
        .channels = BIT(BATTERY_ADC_CHANNEL),
        .buffer = sample_buffer,
        .buffer_size = sizeof(sample_buffer),
        .resolution = ADC_RESOLUTION,
    };

    int rc = adc_read(adc_dev, &seq);
    if (rc) {
        LOG_ERR("ADC read failed: %d", rc);
        return rc;
    }

    *mv = battery_raw_to_mv(sample_buffer[0]);
    LOG_DBG("ADC raw=%d -> %d mV", sample_buffer[0], *mv);
    return 0;
}
