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

### 5. 実機への書き込み
- PlatformIO (.venv) を再構築し、`pio run` でビルド成功。
- `pio run --target upload --upload-port /dev/ttyACM1` で Cardputer (ESP32-S3 rev v0.2, MAC: xx:xx:xx:xx:xx:xx) への書き込みに成功。

### 6. 日本語表示の文字化け修正
- デフォルトフォント（ASCII のみ）では日本語が文字化けしていた。
- `M5.Lcd.setFont(&fonts::efontJA_12)` に変更し、M5GFX 内蔵の日本語フォントを使用。
- LCD 座標もフォントサイズに合わせて再調整。Flash 使用量 30.1% → 39.3%（フォントデータ分）。

### 7. GitHub リポジトリ作成
- リポジトリ: `Murasan201/m5stack-cardputer-openclaw-client`
- Initial commit + ツールチェーン仕様書 (`docs/toolchain-spec.md`) をプッシュ。
- README.md を英語に全面書き換え。

### 8. OpenClaw 接続設定
- OpenClaw gateway は `127.0.0.1:18789`（loopback）のため、Cardputer から直接接続不可。
- nginx リバースプロキシを導入（ポート 18800、LAN `192.168.11.0/24` のみ許可）。
- `config.h` に Wi-Fi SSID/PW、OpenClaw プロキシ URL、Bearer トークンを設定。
- `config.h` は `.gitignore` に追加し、`config.h.example` をテンプレートとして提供。
- ビルド＋書き込み成功。実機で OpenClaw との通信が可能な状態。

### 9. キーボード入力の修正
- 実機テストでボタン A/B/C を押しても無反応であることが判明。
- **原因**: Cardputer には物理 BtnA/B/C がなく、56 キーキーボードのみ。`M5.BtnA.wasPressed()` は常に false。
- `M5Cardputer.Keyboard` API に全面切替。キーボードの文字入力、Enter（送信）、Del（削除）、Tab（モード切替）で操作する仕様に変更。
- さらに `main.cpp` で `M5.begin()` / `M5.update()` を使っていたが、これではキーボードが初期化・更新されない。`M5Cardputer.begin(true)` / `M5Cardputer.update()` に修正して解決。

### 10. ローマ字→ひらがな変換の実装
- Tab 候補方式（あ〜つ 18 文字のみ）では実用性が低いため、ローマ字入力→ひらがな自動変換エンジンを実装。
- 対応: 基本五十音、濁音・半濁音、拗音（sha, chi, nya 等）、促音（っ: 子音連打）。
- Tab キーで日本語 `[あ]` ↔ 英語 `[A]` モード切替。
- **バグ修正**: `n` 単体がテーブルにあったため即「ん」に変換され、「に」等が入力不可だった。`n` をテーブルから除外し、後続文字で判定する方式に修正。

### 11. Wi-Fi 接続状態の可視化
- 実機テストで Enter 送信時に「Wi-Fi が接続されていません」エラーが発生。
- `network_client.cpp` を改修: 起動時に Wi-Fi 接続状態（SSID・接続中・成功/失敗・IP アドレス）を LCD に表示するようにした。
- `WiFi.disconnect(true)` で前セッションをクリアしてから再接続する処理を追加。
- 再試行間隔を 1 秒→3 秒に変更。
- ビルド＋書き込み成功（RAM 14.8%、Flash 40.0%）。

### 12. OpenClaw 通信エラーの原因特定と HTTP ブリッジ構築
- Wi-Fi 接続成功後、プロンプト送信で 405 Method Not Allowed が返っていた。
- **根本原因**: OpenClaw gateway は WebSocket プロトコル (`ws://127.0.0.1:18789`) のみ対応。HTTP REST API (`/api/v1/prompt`) は存在しない。
- **解決策**: HTTP→CLI ブリッジサーバーを構築。
  - `project/openclaw-http-bridge/server.js` — Node.js 組み込み `http` モジュール（外部依存なし）
  - `POST /prompt` → `openclaw agent --agent main --message <text> --json` を spawn
  - レスポンスを `{"response": "..."}` 形式で返却（Cardputer の既存パーサと互換）
  - ポート: `127.0.0.1:18801`、systemd サービスとして自動起動
- nginx 設定更新: `/api/v1/prompt` → `127.0.0.1:18801` にルーティング（LAN 制限維持）
- Cardputer 側: タイムアウトを 12 秒→120 秒に延長、エラー表示に HTTP コード追加、`notifyModeStatus` を無効化（ブリッジ非対応）
- **実機テスト成功**: Cardputer からプロンプト送信 → OpenClaw の応答が LCD に表示された。

### 13. 次のステップ
- 漢字変換（辞書ベース or サーバー側変換）の検討。
- 画面スクロール、応答が長い場合の対応。
- 応答待ち中のローディング表示。
