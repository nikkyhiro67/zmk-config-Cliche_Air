README.md

# 🛠️ Cliche_Air LED-Indicator Non-LiPo-Battery-Monitor Firmware v5.0

**最終ビルド成功版（2025-10-25）**

##🚀 概要

**Cliche_Air LED-Indicator Non-LiPo-Battery-Monitor Firmware v5.0** は、
ZMK Firmware をベースに構築された **左右分離型ワイヤレスキーボードファームウェア** です。

本バージョンでは以下の統合を実現しています：

- 🔋 **非 LiPo バッテリ管理対応 (`zmk,non-lipo-battery`)**
- 💡 **LED インジケータ機能 (`zmk,led-indicator`)**
- ⚙️ **エンコーダ／トラックボール完全動作**
- 🧩 **左右マトリクスの正確なピンマッピング**
- 🧠 **中央・周辺ユニット間の完全同期**

🧱 ファームウェア構成

| ファイル名             | 役割                                                                          |
| ---------------------- | ----------------------------------------------------------------------------- |
| `Cliche_Air.dtsi`      | 共通デバイスツリー定義（行列・LED・alias など）                               |
| `Cliche_Air_L.overlay` | 左手側ユニット構成（エンコーダ EC11 ＋ ADC ＋ LED ＋ Battery Monitor）        |
| `Cliche_Air_R.overlay` | 右手側ユニット構成（トラックボール PMW3610 ＋ ADC ＋ LED ＋ Battery Monitor） |
| `.conf` / `.keymap`    | ZMK レイヤー構成ファイル                                                      |
| `west.yml`             | モジュール参照設定（非 LiPo バッテリ管理拡張含む）                            |

##⚡ 非 LiPo バッテリ管理構成

### 外部モジュール

[sekigon-gonnoc/zmk-feature-non-lipo-battery-management](https://github.com/sekigon-gonnoc/zmk-feature-non-lipo-battery-management)

### 設定概要

````dts
non_lipo_battery: non_lipo_battery {
    compatible = "zmk,non-lipo-battery";
    io-channels = <&adc 0>;
    full-mv = <4200>;
    empty-mv = <3300>;
    status = "okay";
};

##💡LEDインジケータ構成

### 設定概要
```dts
led_indicator: led_indicator {
    compatible = "zmk,led-indicator";
    led-strip = <&sk6812_led>;
    battery = <&battery_monitor>;
    power-gpios = <&gpio1 0 GPIO_ACTIVE_HIGH>; /* LED_EN */
    status = "okay";
};

・SK6812 / WS2812 対応
・カラーフィードバックによる状態表示
・消費電力を抑えるため、電源制御GPIOを併用（LED_EN）

##🌈 バッテリレベル別 LED カラー点灯仕様表

バッテリ残量	電圧範囲 (mV)	LEDカラー	意味
100%～85%	4200〜4000	🟢 緑	フル充電状態
84%～65%	3999〜3800	🟢 黄緑	高残量
64%～45%	3799〜3600	🟡 黄	通常動作範囲
44%～25%	3599〜3400	🟠 橙	低下中（注意）
24%～10%	3399〜3300	🔴 赤	残量低下警告
9%以下	< 3300	⚫ 消灯	バッテリ切れ（自動省電力モード）

##👨‍💻 作者・クレジット

Firmware Architect	nikkyhiro67
Firmware Co-Designer (AI Support)	ChatGPT GPT-5
Base Platform	ZMK Firmware

Additional Modules	zmk-feature-non-lipo-battery-management　@sekigon-gonnoc

Reference　zmk-config-moNa2　     @shakushakupanda
		  zmk-config-roBa	      @kumamuk-git
 		  zmk-keyboard-dya-dash @cormoran

## ⚖License
This project is licensed under the [MIT License](./LICENSE).
Includes components from [ZMK Firmware](https://zmk.dev/) under the same license.
````
