/* app/drivers/pmw3610/pmw3610.h - compatibility shim for uploaded pmw3610.c */

#ifndef PMW3610_H_
#define PMW3610_H_

#include <zephyr/drivers/spi.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/device.h>
#include <zephyr/types.h>
#include <zephyr/kernel.h>

/* Registers / constants (approximate) */
#define PMW3610_REG_SPI_CLK_ON_REQ    0x3A
#define PMW3610_SPI_CLOCK_CMD_ENABLE  0x01
#define PMW3610_SPI_CLOCK_CMD_DISABLE 0x00

#define PMW3610_REG_RES_STEP          0x20

#define PMW3610_REG_POWER_UP_RESET    0x3A
#define PMW3610_POWERUP_CMD_RESET     0x5A

#define PMW3610_REG_OBSERVATION      0x0A
#define PMW3610_REG_PRODUCT_ID       0x00
#define PMW3610_PRODUCT_ID           0x01

#define PMW3610_REG_PERFORMANCE      0x0D

#define PMW3610_REG_RUN_DOWNSHIFT    0x29
#define PMW3610_REG_REST1_DOWNSHIFT  0x2A
#define PMW3610_REG_REST2_DOWNSHIFT  0x2B
#define PMW3610_REG_REST1_RATE       0x2C
#define PMW3610_REG_REST2_RATE       0x2D
#define PMW3610_REG_REST3_RATE       0x2E

#define PMW3610_REG_MOTION_BURST     0x50
#define PMW3610_BURST_SIZE           8

#define PMW3610_X_L_POS   1
#define PMW3610_XY_H_POS  2
#define PMW3610_Y_L_POS   3
#define PMW3610_SHUTTER_H_POS 4
#define PMW3610_SHUTTER_L_POS 5

#define PMW3610_MIN_CPI 200
#define PMW3610_MAX_CPI 12000

#define PMW3610_SVALUE_TO_CPI(val) ((int)((val).val1))
#define PMW3610_SVALUE_TO_TIME(val) ((int)((val).val1))

#define SPI_WRITE_BIT 0x80u

struct pixart_config {
    struct spi_dt_spec spi;
    struct gpio_dt_spec irq_gpio;
    uint32_t cpi;
    uint32_t evt_type;
    uint32_t x_input_code;
    uint32_t y_input_code;
    bool force_awake;
    bool disable_burst_read;
};

struct pixart_data {
    const struct device *dev;
    struct k_work_delayable init_work;
    struct k_work trigger_work;
    struct gpio_callback irq_gpio_cb;
    bool ready;
    bool sw_smart_flag;
    uint8_t async_init_step;
    int err;
};

#ifndef CONFIG_PMW3610_INIT_POWER_UP_EXTRA_DELAY_MS
#define CONFIG_PMW3610_INIT_POWER_UP_EXTRA_DELAY_MS 0
#endif

#ifndef CONFIG_PMW3610_REPORT_INTERVAL_MIN
#define CONFIG_PMW3610_REPORT_INTERVAL_MIN 0
#endif

#ifndef CONFIG_PMW3610_LOG_LEVEL
#define CONFIG_PMW3610_LOG_LEVEL 3
#endif

#endif /* PMW3610_H_ */
