# M5Stack Cardputer App ジャーナル

## 2026-03-01
### 1. 依頼受領
- Slack で Cardputer プロジェクトを確認後、「要件定義書」「テスト」「ソースコード」「ビルド＋トラブル対応」の順で進める要望を受領。
- 以降の作業は `project/m5stack-cardputer-app` 配下に記録することとした。

### 2. ドキュメントとテストの整備
- `docs/requirements.md` にプロンプトモード／日本語入力／送受信 UI・状態遷移・セキュリティ要件を整理した要件定義書を追加。
- `tests/` ディレクトリにステートマシン＋通信イベントのユニットテストを作成し、`.venv` + `pytest` で 7 件がパス。
- `docs/troubleshooting.md` にこれまでに出たエラー（PlatformIOなし、pytestなし、モジュールインポート失敗など）と Wi-Fi/HTTP/JSON/表示/ボタンなど今後想定されるトラブルと対応策を記録。

### 3. Cardputer クライアントのソース作成
- `software/cardputer-client/` に PlatformIO プロジェクトを追加し、`dialogue_manager`, `prompt_input`, `display_manager`, `network_client`, `main.cpp` を構築。A/B/C キー操作、LCD 表示、Wi-Fi + HTTP 通信を組み込み。
- README に操作フロー・実機テスト項目・ビルド手順を記載。

### 4. PlatformIO の導入とビルド
- `.venv` に `platformio` をインストールし、`.platformio` を SSD 上へ移動してディスク不足を解消。`pio --version` で 6.1.19 を確認。
- `pio run --environment m5stack-cardputer` で必要なツールチェイン／ライブラリを順次導入。
- `display_manager.cpp` で未定義だった `TFT_LIME` を `TFT_GREEN` に変更してコンパイル。ビルドは成功して `firmware.bin` を生成（RAM 14.5%、Flash 30.1% 使用）。
- PlatformIO ビルドと Python テストの両方が通ったことを Slack に報告。

### 5. 次のステップ
- `software/cardputer-client/src/config.h` に Wi-Fi 要件・OpenClaw URL・認証トークンを反映してから、PlatformIO による `pio run --target upload --environment m5stack-cardputer` で Cardputer に書き込む。
- 実機では A ボタンで対話モード開始、B で候補/確定/A+B で削除、C で送信し、LCD に OpenClaw の応答が表示されるかを確認。
- 必要に応じて JSON/HTTP 応答処理や IME 候補の拡張、バグ修正を加えていく。
