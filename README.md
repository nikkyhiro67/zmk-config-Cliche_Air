README.md

# 🛠️ Cliche_Air LED-Indicator Non-LiPo-Battery-Monitor Firmware v6.0

**最終ビルド成功版（2025-11-3）**

### 🚀 概要

**Cliche_Air LED-Indicator Non-LiPo-Battery-Monitor Firmware v6.0** は、
ZMK Firmware をベースに構築された **左右分離型ワイヤレスキーボードファームウェア** です。

本バージョンでは以下の統合を実現しています：

- 🔋 **非 LiPo バッテリ管理対応 (`zmk,non-lipo-battery`)**
- 💡 **LED インジケータ機能 (`zmk,led-indicator`)**
- ⚙️ **ロータリーエンコーダ／トラックボール搭載**

### 🧱 ファームウェア構成

| ファイル名             | 役割                                                                   |
| ---------------------- | ---------------------------------------------------------------------- |
| `Cliche_Air.dtsi`      | 共通デバイスツリー定義：マトリクス・LED・エンコーダなど                |
| `Cliche_Air_L.overlay` | 左側（Central）構成：エンコーダ・バッテリ監視・LED 制御                |
| `Cliche_Air_R.overlay` | 右側（Peripheral）構成：トラックボール PMW3610・バッテリ監視・LED 制御 |
| `Cliche_Air_L.conf`    | 左側用 Kconfig：エンコーダ・非 LiPo 電池・LED・バッテリ設定            |
| `Cliche_Air_R.conf`    | 右側用 Kconfig：トラックボール・非 LiPo 電池・LED 設定・バッテリ設定   |

### 🔋 非 LiPo バッテリ管理構成

### 外部モジュール

[sekigon-gonnoc/zmk-feature-non-lipo-battery-management](https://github.com/sekigon-gonnoc/zmk-feature-non-lipo-battery-management)

**dtsi に追記**

```dtsi
non_lipo_battery: non_lipo_battery {
    compatible = "zmk,non-lipo-battery";
    io-channels = <&adc 0>;
    status = "okay";
};
```

### ⚡ バッテリー電圧 SOC 取得 API と ZMK イベント発行

[nikkyhiro67/zmk-module-battery-monitor](https://github.com/nikkyhiro67/zmk-module-battery-monitor/blob/main/src/battery_monitor.c)

**.overlay に追記**

```.overlay
battery_monitor: battery_monitor {
    compatible = "zmk,battery-monitor";
    manager = <&non_lipo_battery>;
    status = "okay";
};
```

### 💡LED インジケータ構成

[nikkyhiro67/zmk-module-led-indicator](https://github.com/nikkyhiro67/zmk-module-led-indicator)

**dtsi に追記**

```.overlay
led_indicator: led_indicator {
    compatible = "zmk,led-indicator";
    led-strip = <&sk6812_led>;
    battery = <&battery_monitor>;
    power-gpios = <&gpio1 0 GPIO_ACTIVE_HIGH>; /* LED_EN */
    status = "okay";
};
```

・SK6812 / WS2812 対応  
・カラーフィードバックによる状態表示  
・消費電力を抑えるため、電源制御 GPIO を併用（LED_EN）

### 🌈 バッテリレベル別 LED カラー点灯仕様表

バッテリ残量 電圧範囲 (mV) LED カラー 意味  
100%～ 85% 4200〜4000 🟢 緑 フル充電状態  
84%～ 65% 3999〜3800 🟢 黄緑 高残量  
64%～ 45% 3799〜3600 🟡 黄 通常動作範囲  
44%～ 25% 3599〜3400 🟠 橙 低下中（注意）  
24%～ 10% 3399〜3300 🔴 赤 残量低下警告  
9%以下 < 3300 ⚫ 消灯 バッテリ切れ（自動省電力モード）

---

### ⚙️ `Cliche_Air.dtsi`

共通構成を司る基盤ファイル。  
行列スキャン・LED 定義・エンコーダ・matrix_transform を包括。  
各 overlay から `#include "Cliche_Air.dtsi"` により参照。

主なポイント：

- **matrix-transform 共通化**（左右 col-offset 切替対応）
- **共通 LED ノード（sk6812_led）を disabled 定義** → overlay 側で有効化または独自ノード置換
- **共通行ピン設定（col は overlay で再定義）**

### 🩵 `Cliche_Air_L.overlay`（左：Central）

左側は**Central（親）**として動作。  
エンコーダ・非 LiPo バッテリ・LED インジケータを統合。

```dts
non_lipo_battery_left: non_lipo_battery_left {
    compatible = "zmk,non-lipo-battery";
    io-channels = <&adc 0>;
    status = "okay";
};

battery_monitor_left: battery_monitor_left {
    compatible = "zmk,battery-monitor";
    manager = <&non_lipo_battery_left>;
    status = "okay";
};

led_indicator_left: led_indicator_left {
    compatible = "zmk,led-indicator";
    led-strip = <&sk6812_led>;
    battery = <&battery_monitor_left>;
    power-gpios = <&gpio1 0 GPIO_ACTIVE_HIGH>;
    status = "okay";
};

&adc {
    status = "okay";
};
```

🔹 特徴

- 共通 LED (`&sk6812_led`) を利用して指示灯制御
- 非 LiPo バッテリを`battery-monitor`経由で取得
- `encoder0` alias を設定し、ZMK の sensor 連携を容易に

### 🩷 `Cliche_Air_R.overlay`（右：Peripheral）

右側は**Peripheral（子）**として動作。  
トラックボール（PMW3610）＋独自 LED ＋非 LiPo 電池モニタ構成。

```dts
sk6812_led_right: led_strip {
    compatible = "zmk,led-strip";
    label = "LED_STRIP_RIGHT";
    gpios = <&gpio0 16 GPIO_ACTIVE_HIGH>;
    chain-length = <10>;
    color-order = "GRB";
    status = "okay";
};

non_lipo_battery_right: non_lipo_battery_right {
    compatible = "zmk,non-lipo-battery";
    io-channels = <&adc 0>;
    status = "okay";
};

battery_monitor_right: battery_monitor_right {
    compatible = "zmk,battery-monitor";
    manager = <&non_lipo_battery_right>;
    status = "okay";
};

led_indicator_right: led_indicator_right {
    compatible = "zmk,led-indicator";
    led-strip = <&sk6812_led_right>;
    battery = <&battery_monitor_right>;
    power-gpios = <&gpio1 0 GPIO_ACTIVE_HIGH>;
    status = "okay";
};
```

🔹 特徴

- 独立した LED ノード `sk6812_led_right` を採用
- SPI バスで PMW3610 トラックボールを接続
- `col-offset = <6>` により keymap の右手側分担を定義

---

### ⚡ `Cliche_Air_L.conf`（左設定）

🔹 特徴

- 中央（Central）側バッテリレポート機能を有効化
- EC11 エンコーダ用スレッドトリガを有効化
- 非 LiPo 電池設定を明示

### ⚡ `Cliche_Air_R.conf`（右設定）

🔹 特徴

- PMW3610 トラックボール構成に完全対応
- RGBLED にバッテリ状態およびレイヤーカラー反映
- 非 LiPo 電池管理を左右で統一化

---

### 👨‍💻 作者・クレジット

Firmware Architect nikkyhiro67  
Firmware Co-Designer (AI Support) ChatGPT GPT-5  
Base Platform ZMK Firmware

Additional Modules  
・[sekigon-gonnoc/zmk-feature-non-lipo-battery-management](https://github.com/sekigon-gonnoc/zmk-feature-non-lipo-battery-management) 　@sekigon-gonnoc

Reference  
・[zmk-config-moNa2](https://github.com/sayu-hub/zmk-config-moNa2) @shakushakupanda  
・[zmk-config-roBa](https://github.com/kumamuk-git/zmk-config-roBa) @kumamuk-git  
・[zmk-keyboard-dya-dash](https://github.com/cormoran/dya-dash-keyboard/tree/main/firmware) @cormoran

---

### ⚖License

This project is licensed under the [MIT License](./LICENSE).
Includes components from [ZMK Firmware](https://zmk.dev/) under the same license.
