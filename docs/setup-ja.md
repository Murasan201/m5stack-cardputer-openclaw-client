# M5Stack Cardputer の CLI 開発ガイド（Raspberry Pi 接続）

## 1. Cardputer について
- SoC: ESP32-S3FN8（デュアルコア Xtensa LX7、最大 240MHz）。
- インターフェース: 1.14インチ ST7789V2 TFT、56キーキーボード、SPM1423 MEMSマイク、NS4168 スピーカー、赤外線 LED、Grove HY2.0-4P（I2C）、microSD スロット、USB OTG/Serial/JTAG、磁石内蔵のベースと LEGO 拡張。バッテリーは本体 120mAh＋ベース部 1400mAh のデュアル構成。
- 通信: 2.4GHz Wi-Fi、USB シリアル、赤外線制御。
- ソフトウェア: Arduino IDE / PlatformIO / ESP-IDF / UiFlow2 に対応し、公式ライブラリ https://github.com/m5stack/M5Cardputer を利用可能。

## 2. Raspberry Pi との接続手順
1. Cardputer を USB-C 端子で Raspberry Pi に接続（Pi 側は USB-A でも USB-C でも可）。
2. G0 ボタンを押し続けた状態で電源を入れるとダウンロードモードに入る（LED や表示が点灯したらリリース）。
3. `lsusb` と `dmesg | tail` で `/dev/ttyACM0` または `/dev/ttyUSB0` のデバイス名を確認。

```bash
lsusb
dmesg | tail
ls /dev/ttyACM* /dev/ttyUSB*
```

## 3. ラズパイ側の開発環境構築
```bash
sudo apt update
sudo apt install -y python3 python3-venv python3-pip git
python3 -m venv ~/cardputer-env
source ~/cardputer-env/bin/activate
pip install --upgrade pip
pip install esptool platformio
```
- `esptool.py` で ESP32-S3 のフラッシュ/情報取得が可能。フラッシュ用ポートは `/dev/ttyACM0` など。
- `platformio` で `env:m5stack-cardputer`（PlatformIO 用設定）をビルド・アップロード。
- ESP-IDF で開発する場合は公式手順に沿って `idf.py` をインストールし、環境変数を設定する。

## 4. ビルド・フラッシュ作業の例
```
# チップ情報取得
esptool.py --chip esp32s3 --port /dev/ttyACM0 chip_id

# PlatformIO でビルド・アップロード
cd ~/project/m5stack-cardputer-app/software
platformio run --target upload
```
- `platformio.ini` は `env:m5stack-cardputer` を定義し、以下の設定と M5Cardputer ライブラリを含める。
  ```ini
  [env:m5stack-cardputer]
  platform = espressif32@6.7.0
  board = esp32-s3-devkitc-1
  framework = arduino
  build_flags =
      -DESP32S3
      -DCORE_DEBUG_LEVEL=5
      -DARDUINO_USB_CDC_ON_BOOT=1
      -DARDUINO_USB_MODE=1
  lib_deps =
      M5Cardputer=https://github.com/m5stack/M5Cardputer
  upload_speed = 1500000
  ```
- Cardputer をダウンロードモードにしてから `platformio run --target upload` を実行すると自動的に書き込みが始まる。

## 5. プロジェクト構造と今後のステップ
- `software/`: MicroPython／Arduino／PlatformIO のプロジェクト、ソース、`main.cpp` 等。
- `docs/`: 本ドキュメントや要件、配線図、デバッグ記録（ここに開発メモやノウハウを追記）。
- `firmware/`: 追加のファームウェアバイナリやユーザーデモ。

今後やること:
1. `docs/requirements.md` で UI や LLM 連携などの要件を日本語で整理。
2. `software/` にとりあえず UI 制御やセンサ読み取りのサンプルを追加。
3. `project/README-index.md` へ開発ブランチ/リリース方針の追記。

## 6. 参考資料
- 公式 Cardputer サイト: https://docs.m5stack.com/ja/core/Cardputer
- 回路図: https://m5stack-doc.oss-cn-shenzhen.aliyuncs.com/481/Sch_M5Cardputer.pdf
- Arduino/PlatformIO ライブラリ: https://github.com/m5stack/M5Cardputer
