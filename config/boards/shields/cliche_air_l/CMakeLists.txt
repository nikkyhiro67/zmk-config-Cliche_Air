/dts-v1/;
/ {
    /include/ "../cliche_air/cliche_air.dtsi"

    chosen {
        zmk,kscan = &kscan0;
    };

    &kscan0 {
        /* 左手の row/col を上書き（回路図に合わせて最終確認） */
        row-gpios = <&gpio0 2 (GPIO_ACTIVE_HIGH | GPIO_PULL_DOWN)>,
                    <&gpio0 3 (GPIO_ACTIVE_HIGH | GPIO_PULL_DOWN)>,
                    <&gpio0 4 (GPIO_ACTIVE_HIGH | GPIO_PULL_DOWN)>,
                    <&gpio0 5 (GPIO_ACTIVE_HIGH | GPIO_PULL_DOWN)>,
                    <&gpio0 6 (GPIO_ACTIVE_HIGH | GPIO_PULL_DOWN)>;
        col-gpios = <&gpio0 7 (GPIO_ACTIVE_HIGH | GPIO_PULL_UP)>,
                    <&gpio0 8 (GPIO_ACTIVE_HIGH | GPIO_PULL_UP)>,
                    <&gpio0 9 (GPIO_ACTIVE_HIGH | GPIO_PULL_UP)>,
                    <&gpio0 10 (GPIO_ACTIVE_HIGH | GPIO_PULL_UP)>,
                    <&gpio0 11 (GPIO_ACTIVE_HIGH | GPIO_PULL_UP)>,
                    <&gpio0 12 (GPIO_ACTIVE_HIGH | GPIO_PULL_UP)>;
        diode-direction = "col2row";
    };

    /* 左手はトラックボール無し（ある場合はここに追記） */
    trackball0: trackball {
        status = "disabled";
    };

    battery_voltage: battery_voltage {
        io-channels = <&adc 1>;
        status = "okay";
    };
};
