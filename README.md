# M5Stack Cardputer App

このディレクトリは M5Stack Cardputer（https://docs.m5stack.com/ja/core/Cardputer）を使ったアプリ開発用です。Cardputer は M5Stack CoreS3ベースの高性能 SoC を備えたモジュール型コンピュータで、センサ・カメラ・ディスプレイなどを統合したハードウェアです。

## 目的
- Cardputer の GPIO・I2C・SPI を使ったセンサ連携や、タッチスクリーン UI、Bluetooth/Wi-Fi 通信などを組み合わせた実用アプリを prototyping する。Raspberry Pi や StackChan との連携も想定します。

## 構成
- `software/`：Cardputer 上で動く MicroPython/Arduino/ESP-IDF サンプル
- `docs/`：ビルド手順、配線図、デバッグノート
- `firmware/`：必要なバイナリ・ファームウェアを置く場所

## 次のステップ
1. Cardputer の代表的な入出力（タッチ、カメラ、IMU）を把握するための README まとめ
2. 今回搭載したい機能（例: LLM 連携、UI パネル、外部制御）の要件を `docs/requirements.md` に下書き
3. プロトタイプ作成用の開発ブランチ構成を決めて `project/README-index.md` に追記
