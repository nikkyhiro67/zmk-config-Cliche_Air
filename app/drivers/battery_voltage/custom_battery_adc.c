/*
 * Custom Battery ADC Driver for Cliche_Air
 * ----------------------------------------
 * 独自ADC読み取りにより定期的に電圧を測定し、
 * ZMKイベントシステムを通してBLEバッテリーレベルを更新します。
 */

#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/drivers/adc.h>
#include <zephyr/logging/log.h>
#include <zephyr/sys/util.h>

#include <zmk/event_manager.h>
#include <zmk/events/battery_state_changed.h>
#include <zmk/battery.h>

LOG_MODULE_REGISTER(custom_battery_adc, CONFIG_ZMK_LOG_LEVEL);

#define ADC_NODE DT_NODELABEL(adc)
#define ADC_CHANNEL_ID 2           /* 実際のADCチャンネルに合わせて変更 */
#define ADC_RESOLUTION 12
#define ADC_REF_MV 3300
#define VOLTAGE_DIVIDER_RATIO 2.0f /* 分圧比（ハード構成に合わせて調整） */
#define BATTERY_UPDATE_INTERVAL_MS 60000 /* 60秒ごとに更新 */

static const struct device *adc_dev;
static uint16_t sample_buffer;
static struct adc_channel_cfg adc_cfg = {
    .gain = ADC_GAIN_1,
    .reference = ADC_REF_INTERNAL,
    .acquisition_time = ADC_ACQ_TIME_DEFAULT,
    .channel_id = ADC_CHANNEL_ID,
};

static bool initialized = false;

/*----------------------------------
 * 電圧値をミリボルトに変換
 *----------------------------------*/
static int raw_to_mv(uint16_t raw)
{
    uint32_t max = (1 << ADC_RESOLUTION) - 1;
    float mv = ((float)raw / max) * ADC_REF_MV * VOLTAGE_DIVIDER_RATIO;
    return (int)mv;
}

/*----------------------------------
 * バッテリ初期化
 *----------------------------------*/
static int custom_battery_adc_init(void)
{
    adc_dev = DEVICE_DT_GET(ADC_NODE);
    if (!device_is_ready(adc_dev)) {
        LOG_ERR("ADC not ready");
        return -ENODEV;
    }

    int ret = adc_channel_setup(adc_dev, &adc_cfg);
    if (ret < 0) {
        LOG_ERR("ADC channel setup failed: %d", ret);
        return ret;
    }

    initialized = true;
    LOG_INF("Custom battery ADC initialized (channel %d)", ADC_CHANNEL_ID);
    return 0;
}

/*----------------------------------
 * 電圧読み取り関数
 *----------------------------------*/
static int read_battery_mv(int *mv)
{
    if (!initialized) {
        int ret = custom_battery_adc_init();
        if (ret < 0)
            return ret;
    }

    struct adc_sequence seq = {
        .channels = BIT(ADC_CHANNEL_ID),
        .buffer = &sample_buffer,
        .buffer_size = sizeof(sample_buffer),
        .resolution = ADC_RESOLUTION,
    };

    int ret = adc_read(adc_dev, &seq);
    if (ret < 0) {
        LOG_ERR("ADC read failed: %d", ret);
        return ret;
    }

    *mv = raw_to_mv(sample_buffer);
    return 0;
}

/*----------------------------------
 * 電圧→パーセンテージ換算
 *----------------------------------*/
static int mv_to_percent(int mv)
{
    if (mv >= 4200) return 100;
    if (mv <= 3300) return 0;
    return (mv - 3300) * 100 / (4200 - 3300);
}

/*----------------------------------
 * 定期測定スレッド
 *----------------------------------*/
static void battery_monitor_thread(void)
{
    while (true) {
        int mv = 0;
        if (read_battery_mv(&mv) == 0) {
            int percent = mv_to_percent(mv);
            LOG_INF("Battery: %dmV (%d%%)", mv, percent);
            /* イベント送信でZMKバッテリー更新 */
            struct battery_state_changed *event =
                new_battery_state_changed((uint8_t)percent, (uint16_t)mv);
            ZMK_EVENT_RAISE(event);
        } else {
            LOG_WRN("Failed to read battery voltage");
        }

        k_sleep(K_MSEC(BATTERY_UPDATE_INTERVAL_MS));
    }
}

K_THREAD_DEFINE(custom_battery_monitor_thread_id,
                1024,
                battery_monitor_thread,
                NULL, NULL, NULL,
                K_LOWEST_APPLICATION_THREAD_PRIO,
                0, 0);
