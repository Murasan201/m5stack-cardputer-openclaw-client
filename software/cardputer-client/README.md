# Cardputer ↔ OpenClaw プロンプトクライアント

Cardputer 上で OpenClaw に日本語プロンプトを送り、LCD に応答を表示するための PlatformIO プロジェクトです。A/B/C ボタンと 1.14" TFT を活用し、プロンプトモード起動・候補選択・送信・応答表示の一連フローを組み立てています。

## 構成
- `src/config.h`: Wi-Fi／OpenClaw 接続情報、タイムアウトなどを定義。ビルド前にここに値を設定します。
- `src/main.cpp`: M5Cardputer のセットアップとループ。
- `src/dialogue_manager.*`: セッション状態とボタン処理、入出力フローの制御。
- `src/prompt_input.*`: 日本語候補を切り替えてバッファに蓄える簡易 IME。
- `src/display_manager.*`: LCD にモード/入力/応答/エラーを描画。
- `src/network_client.*`: Wi-Fi 接続、OpenClaw への POST（モードイベント + プロンプト）、レスポンスパース。

## セットアップ & ビルド
```bash
git clone ... # 既存リポジトリを使う
cd project/m5stack-cardputer-app/software/cardputer-client
pio run --target upload
```
1. `src/config.h` に Wi-Fi SSID/PASS、`OPENCLAW_PROMPT_URL`、必要なら `OPENCLAW_AUTH_TOKEN` を書き換えます。2. Cardputer をダウンロードモードにして `platformio run --target upload --environment m5stack-cardputer` で書き込みます。

## 操作フロー
1. `A` ボタン（プロンプトモードキー）を押して対話モードを開始。LCD に候補・入力欄・操作説明が出ます。
2. `A` ボタンで日本語文字候補（あいうえお〜）を巡回。`B` ボタンで選択（`A` を押しながら`B` で削除）。`C` ボタンで OpenClaw へ送信。
3. OpenClaw 応答が返ってきたら LCD に表示。`A` ボタンで次の対話に戻れます。
4. エラー時は赤文字で案内、`A` ボタンで再試行できます。

## 実機テスト項目
- Wi-Fi 未接続時に `NetworkClient` が再接続を試みるか。
- `A`→`B`→`C` の流れで任意の文字列が `OPENCLAW_PROMPT_URL` に送信され、レスポンスが表示されるか。
- `A` を押し続けた状態で `B` を押すとバックスペースされるか。

必要であれば、`src/prompt_input.h` を拡張して物理 56 キーキーボードやタッチベースの IME を接続しても構いません。