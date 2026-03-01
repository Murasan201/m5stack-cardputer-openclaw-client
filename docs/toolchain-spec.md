# ツールチェーン仕様書

M5Stack Cardputer（ESP32-S3）向けファームウェアの開発に必要なツール群と、ビルド・フラッシュ書き込み手順をまとめる。

## 1. ハードウェア構成

| 項目 | 値 |
|---|---|
| ボード | M5Stack Cardputer |
| SoC | ESP32-S3FN8（Xtensa LX7 デュアルコア, 240MHz） |
| Flash | 8MB |
| RAM | 320KB SRAM |
| USB | USB-C（USB OTG / Serial / JTAG） |
| ディスプレイ | 1.14" TFT ST7789V2 (240x135) |
| 接続先ホスト | Raspberry Pi 5 (arm64, Debian Bookworm) |

## 2. 必要なツール一覧

### 2.1 システムパッケージ

```bash
sudo apt update
sudo apt install -y python3 python3-venv python3-pip git
```

### 2.2 Python 仮想環境 + PlatformIO

```bash
python3 -m venv .venv
source .venv/bin/activate
pip install platformio
```

| ツール | バージョン | 用途 |
|---|---|---|
| PlatformIO Core (CLI) | 6.1.19 | ビルド・アップロード・シリアルモニタ統合 |
| Python | 3.13+ | PlatformIO 実行環境 |

### 2.3 PlatformIO が自動管理するパッケージ

`pio run` の初回実行時に自動的にダウンロード・インストールされる。

| パッケージ | バージョン | 用途 |
|---|---|---|
| platform: espressif32 | ~6.0 (6.0.1) | ESP32 ファミリー向けプラットフォーム定義 |
| framework: arduino | 2.0.6 | Arduino フレームワーク (ESP32 向け) |
| toolchain-xtensa-esp32s3 | 8.4.0+2021r2-patch5 | Xtensa LX7 クロスコンパイラ (GCC) |
| toolchain-riscv32-esp | 8.4.0+2021r2-patch5 | ULP コプロセッサ向けコンパイラ |
| tool-esptoolpy | 4.4.0 | ESP32 フラッシュ書き込みツール |
| tool-mkspiffs | 2.230.0 | SPIFFS イメージ作成 |
| tool-mklittlefs | 1.203.210628 | LittleFS イメージ作成 |
| tool-mkfatfs | 2.0.1 | FAT イメージ作成 |

### 2.4 ライブラリ依存関係

`platformio.ini` の `lib_deps` で定義。初回ビルド時に自動取得される。

| ライブラリ | バージョン | 用途 |
|---|---|---|
| M5Cardputer | 1.1.1 | Cardputer 制御（M5Unified + M5GFX + キーボード） |
| ArduinoJson | ^7.1.0 (7.4.2) | JSON シリアライズ/デシリアライズ |
| HTTPClient | 2.0.0 | HTTP 通信（Arduino 組み込み） |
| WiFi | 2.0.0 | Wi-Fi 接続（Arduino 組み込み） |

### 2.5 ストレージ配置

Raspberry Pi 上ではディスク容量の都合から、PlatformIO のコアデータを SSD に配置している。

| パス | 配置先 | 内容 |
|---|---|---|
| `/mnt/ssd/.platformio/` | SSD | PlatformIO パッケージ・ツールチェーン |
| プロジェクト内 `.venv/` | microSD | Python 仮想環境（.gitignore 対象） |
| プロジェクト内 `.pio/` | microSD | ビルド成果物キャッシュ（.gitignore 対象） |

SSD 上の PlatformIO コアディレクトリを使うには環境変数を設定する:

```bash
export PLATFORMIO_CORE_DIR=/mnt/ssd/.platformio
```

## 3. platformio.ini 設定

```ini
[platformio]
default_envs = m5stack-cardputer

[env:m5stack-cardputer]
platform = espressif32@~6.0
board = esp32-s3-devkitc-1
framework = arduino
monitor_speed = 115200
build_flags =
    -DESP32S3
    -DCORE_DEBUG_LEVEL=5
    -DARDUINO_USB_CDC_ON_BOOT=1
    -DARDUINO_USB_MODE=1
lib_deps =
    https://github.com/m5stack/M5Cardputer
    bblanchon/ArduinoJson @ ^7.1.0
upload_speed = 1500000
build_unflags =
    -std=gnu++17
```

### ビルドフラグの説明

| フラグ | 説明 |
|---|---|
| `-DESP32S3` | ESP32-S3 固有コードの有効化 |
| `-DCORE_DEBUG_LEVEL=5` | デバッグログレベル（5=VERBOSE） |
| `-DARDUINO_USB_CDC_ON_BOOT=1` | USB CDC（シリアル）をブート時に有効化 |
| `-DARDUINO_USB_MODE=1` | USB モードを有効化（JTAG/シリアル） |

## 4. USB デバイス識別

Cardputer を Raspberry Pi に USB 接続すると以下のデバイスとして認識される。

| デバイス | VID:PID | チップ | 用途 |
|---|---|---|---|
| `/dev/ttyACM*` | `303a:1001` | Espressif USB JTAG/serial debug unit | **書き込み・シリアルモニタ（メイン）** |
| `/dev/ttyUSB*` | `10c4:ea60` | Silicon Labs CP210x | 別のUSBシリアルデバイス（Cardputer以外） |

確認コマンド:

```bash
# USB デバイス一覧
lsusb

# シリアルポート確認
ls /dev/ttyACM* /dev/ttyUSB*

# デバイス詳細
udevadm info --name=/dev/ttyACM1
```

### パーミッション

ユーザーが `dialout` および `plugdev` グループに所属している必要がある:

```bash
groups $USER
# 出力に dialout, plugdev が含まれていること

# 含まれていない場合
sudo usermod -aG dialout,plugdev $USER
# ログアウト→ログインで反映
```

## 5. ビルド手順

### 5.1 環境の準備（初回のみ）

```bash
cd /home/pi/.openclaw/workspace/project/m5stack-cardputer-app

# Python 仮想環境の作成と PlatformIO インストール
python3 -m venv .venv
source .venv/bin/activate
pip install platformio

# PlatformIO コアディレクトリを SSD に向ける
export PLATFORMIO_CORE_DIR=/mnt/ssd/.platformio
```

### 5.2 ビルド

```bash
source .venv/bin/activate
export PLATFORMIO_CORE_DIR=/mnt/ssd/.platformio

cd software/cardputer-client
pio run --environment m5stack-cardputer
```

成功時の出力例:

```
RAM:   [=         ]  14.5% (used 47440 bytes from 327680 bytes)
Flash: [===       ]  30.1% (used 1004801 bytes from 3342336 bytes)
========================= [SUCCESS] Took 48.26 seconds =========================
```

ビルド成果物: `.pio/build/m5stack-cardputer/firmware.bin`

## 6. フラッシュ書き込み手順

### 6.1 通常モード書き込み（推奨）

Cardputer が通常起動している状態で書き込む。esptool が自動的にリセットを制御する。

```bash
source .venv/bin/activate
export PLATFORMIO_CORE_DIR=/mnt/ssd/.platformio

cd software/cardputer-client
pio run --target upload --environment m5stack-cardputer --upload-port /dev/ttyACM1
```

成功時の出力例:

```
Serial port /dev/ttyACM1
Connecting...
Chip is ESP32-S3 (revision v0.2)
Features: WiFi, BLE
Crystal is 40MHz
MAC: xx:xx:xx:xx:xx:xx
Uploading stub...
Stub running...
Changing baud rate to 1500000
...
Wrote 1005184 bytes (641999 compressed) at 0x00010000 in 4.3 seconds
...
Hard resetting via RTS pin...
========================= [SUCCESS] Took 21.08 seconds =========================
```

書き込み後、Cardputer は自動リセットされ新しいファームウェアで起動する。

### 6.2 ダウンロードモード書き込み（通常モードで失敗した場合）

1. Cardputer の **G0 ボタンを押し続ける**
2. その状態で **電源を入れる**（またはリセット）
3. ダウンロードモードに入ったら G0 ボタンを離す
4. 書き込みコマンドを実行:

```bash
pio run --target upload --environment m5stack-cardputer --upload-port /dev/ttyACM1
```

### 6.3 esptool 単体での書き込み（トラブルシューティング用）

PlatformIO を介さず、ビルド済みバイナリを直接書き込む場合:

```bash
source .venv/bin/activate

esptool.py --chip esp32s3 --port /dev/ttyACM1 --baud 1500000 \
  write_flash \
  0x0000 .pio/build/m5stack-cardputer/bootloader.bin \
  0x8000 .pio/build/m5stack-cardputer/partitions.bin \
  0xe000 ~/.platformio/packages/framework-arduinoespressif32/tools/partitions/boot_app0.bin \
  0x10000 .pio/build/m5stack-cardputer/firmware.bin
```

### 6.4 フラッシュの完全消去

ファームウェアを完全に消去してクリーンな状態に戻す:

```bash
esptool.py --chip esp32s3 --port /dev/ttyACM1 erase_flash
```

## 7. シリアルモニタ

書き込み後のデバッグログやシリアル出力を確認する:

```bash
pio device monitor --port /dev/ttyACM1 --baud 115200
```

終了は `Ctrl+]`。

## 8. ビルド＋書き込みの一括実行

ビルドと書き込みをワンコマンドで実行するショートカット:

```bash
source .venv/bin/activate && \
export PLATFORMIO_CORE_DIR=/mnt/ssd/.platformio && \
cd /home/pi/.openclaw/workspace/project/m5stack-cardputer-app/software/cardputer-client && \
pio run --target upload --environment m5stack-cardputer --upload-port /dev/ttyACM1
```

## 9. トラブルシューティング

| 症状 | 原因 | 対処 |
|---|---|---|
| `Permission denied: '/dev/ttyACM1'` | ユーザーが dialout/plugdev 未所属 | `sudo usermod -aG dialout,plugdev $USER` 後に再ログイン |
| `A]fatal error occurred: Failed to connect` | Cardputer がダウンロードモードでない or ポートが違う | G0+リセットでダウンロードモードにする / `ls /dev/ttyACM*` でポート確認 |
| `No such file or directory: pio` | venv が有効化されていない | `source .venv/bin/activate` |
| ディスク容量不足でビルド失敗 | microSD 満杯 | `PLATFORMIO_CORE_DIR=/mnt/ssd/.platformio` を設定 |
| ライブラリ取得失敗 | ネットワーク未接続 | Wi-Fi / 有線接続を確認 |
