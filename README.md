# Cliche_Air LED Indicator Firmware v4

This package contains:

- DeviceTree overlays (config/boards/shields/cliche_air/)
- prj.conf and left/right confs
- Custom drivers (app/drivers/): battery_voltage, ext_power_transient, led_stripnize, pmw3610
- LED indicator app (app/src/led_indicator.c)
- Pinmap CSV (config/boards/shields/cliche_air/cliche_air_pinmap_confirmed.csv) -- PLEASE VERIFY

IMPORTANT:

- Verify pin mappings in the CSV against your PCB/board before building.
- PMW3610 driver is SPI-based; if your board uses SDIO, the driver must be adapted.
- Build this within your ZMK workspace (west build) and provide logs if errors occur.

  10.26 色々々修正
