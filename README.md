README.md

# 🛠️ Cliche_Air LED-Indicator Non-LiPo-Battery-Monitor Firmware v5.0

**最終ビルド成功版（2025-10-25）**

### 🚀 概要

**Cliche_Air LED-Indicator Non-LiPo-Battery-Monitor Firmware v5.0** は、
ZMK Firmware をベースに構築された **左右分離型ワイヤレスキーボードファームウェア** です。

本バージョンでは以下の統合を実現しています：

- 🔋 **非 LiPo バッテリ管理対応 (`zmk,non-lipo-battery`)**
- 💡 **LED インジケータ機能 (`zmk,led-indicator`)**
- ⚙️ **エンコーダ／トラックボール完全動作**
- 🧩 **左右マトリクスの正確なピンマッピング**
- 🧠 **中央・周辺ユニット間の完全同期**

### 🧱 ファームウェア構成

| ファイル名             | 役割                                                                          |
| ---------------------- | ----------------------------------------------------------------------------- |
| `Cliche_Air.dtsi`      | 共通デバイスツリー定義（行列・LED・alias など）                               |
| `Cliche_Air_L.overlay` | 左手側ユニット構成（エンコーダ EC11 ＋ ADC ＋ LED ＋ Battery Monitor）        |
| `Cliche_Air_R.overlay` | 右手側ユニット構成（トラックボール PMW3610 ＋ ADC ＋ LED ＋ Battery Monitor） |
| `.conf` / `.keymap`    | ZMK レイヤー構成ファイル                                                      |
| `west.yml`             | モジュール参照設定（非 LiPo バッテリ管理拡張含む）                            |

### 🔋 非 LiPo バッテリ管理構成

### 外部モジュール

[sekigon-gonnoc/zmk-feature-non-lipo-battery-management](https://github.com/sekigon-gonnoc/zmk-feature-non-lipo-battery-management)

**dtsi に追記**

```dtsi
non_lipo_battery: non_lipo_battery {
    compatible = "zmk,non-lipo-battery";
    io-channels = <&adc 0>;
    full-mv = <4200>;
    empty-mv = <3300>;
    status = "okay";
};
```

### ⚡ バッテリー電圧 SOC 取得 API と ZMK イベント発行

[nikkyhiro67/zmk-module-battery-monitor](https://github.com/nikkyhiro67/zmk-module-battery-monitor/blob/main/src/battery_monitor.c)

**dtsi に追記**

```dtsi
battery_monitor: battery_monitor {
    compatible = "zmk,battery-monitor";
    manager = <&non_lipo_battery>;
    status = "okay";
};
```

### 💡LED インジケータ構成

[nikkyhiro67/zmk-module-led-indicator](https://github.com/nikkyhiro67/zmk-module-led-indicator)

**dtsi に追記**

```dtsi
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

### 📘 デバイスツリーで設定する .dtsi .overlay

**dtsi に追記**

```dtsi
aliases {
    led-indicator = &led_indicator;
    battery-monitor = &battery_monitor;
};
```

**overlay に追記**

```overlay
chosen {
    zmk,battery = &non_lipo_battery;
};

aliases {
    led-indicator = &led_indicator;
    battery-monitor = &battery_monitor;
};
```

### ⭐ prj.conf 　 Kconfig 設定で有効にする

**① LED 周りの依存関係を明示化**

```conf
CONFIG_LED_STRIP=y
CONFIG_LED=y
CONFIG_PWM=y
```

Zephyr の led_strip ドライバを正しく有効にするために必須。  
（Cliche_Air は RGB ストリップ駆動前提のため入れておくと安全）

**② ログレベル最適化**

```conf
CONFIG_LOG=y
CONFIG_LOG_DEFAULT_LEVEL=3
CONFIG_ZMK_LOG_LEVEL=3
```

通常運用では「3: INFO」が最適（デバッグ時のみ 4 に変更）  
この設定で LOG_INF() が有効になります。

**③ 初期化順序の安定化**

```conf
CCONFIG_APPLICATION_INIT_PRIORITY=80
```

ZMK コアの BLE/Battery 初期化完了後に LED 初期化を行うための設定。  
これがないと device_is_ready(led_strip_dev) が false のままになるケースがあります。

**④ 分割構成のバッテリ同期**

```conf
CONFIG_ZMK_SPLIT_BLE_CENTRAL_BATTERY_LEVEL_PROXY=y
CONFIG_ZMK_SPLIT_BLE_CENTRAL_BATTERY_LEVEL_FETCHING=y
```

これにより左右分割間でバッテリ残量が正しく同期されます。  
master 側 LED が slave 側バッテリ残量にも追従。

**⑤ 電源管理（省電力連携）**

```conf
CONFIG_PM=y
CONFIG_PM_DEVICE=y
```

バッテリ機器では省電力制御必須。  
将来的にスリープ／復帰時の LED 点滅などにも対応可能。

**⑥ battery_monitor.c の更新周期と一致**

```conf
CONFIG_ZMK_BATTERY_MONITOR_INTERVAL_SEC=30
CONFIG_ZMK_LED_INDICATOR_UPDATE_INTERVAL=1000
```

30 秒ごとに battery_monitor が SoC 更新イベントを発行。  
LED は 1 秒ごとに再描画（状態変化時は即反映）。

### 👨‍💻 作者・クレジット

Firmware Architect nikkyhiro67
Firmware Co-Designer (AI Support) ChatGPT GPT-5
Base Platform ZMK Firmware

Additional Modules zmk-feature-non-lipo-battery-management 　@sekigon-gonnoc

Reference
・mk-config-moNa2 @shakushakupanda
・zmk-config-roBa @kumamuk-git
・zmk-keyboard-dya-dash @cormoran

### ⚖License

This project is licensed under the [MIT License](./LICENSE).
Includes components from [ZMK Firmware](https://zmk.dev/) under the same license.
