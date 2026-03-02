# CLAUDE.md

このファイルは Claude Code がこのリポジトリで作業する際のガイドとして機能します。

## プロジェクト概要

M5Stack Cardputer を OpenClaw AI エージェントの対話端末として使用するシステム。
Cardputer（ESP32-S3）のファームウェアと、Raspberry Pi 上の HTTP ブリッジサーバーで構成される。

## 機密情報の取り扱い（必須）

### コミット・プッシュ前のチェック（厳守）

**コミットおよびプッシュを行う前に、必ず以下を確認すること:**

1. **`git diff --cached` で差分を確認し、以下の情報が含まれていないことを検証する:**
   - Wi-Fi SSID / パスワード
   - OpenClaw Bearer トークン / API キー
   - 実際の LAN IP アドレス（192.168.x.x 等）
   - MAC アドレス
   - その他のパスワード・秘密鍵・認証情報

2. **`config.h` は絶対にコミットしない。** `.gitignore` に含まれているが、`git add -A` や `git add .` で誤って追加されないよう注意する。ステージングは個別ファイル指定で行うこと。

3. **ドキュメント（journal.md, troubleshooting.md 等）に実際の IP・MAC・認証情報を記載しない。** プレースホルダ（`YOUR_RASPI_IP`、`xx:xx:xx:xx:xx:xx`、`<token>` 等）を使用する。

### 機密ファイル

| ファイル | 状態 | 備考 |
|---|---|---|
| `software/cardputer-client/src/config.h` | .gitignore 対象 | Wi-Fi PW、Bearer トークン、実 IP を含む |
| `software/cardputer-client/src/config.h.example` | Git 追跡対象 | プレースホルダ値のみ |

## ビルド・書き込み

```bash
source .venv/bin/activate
export PLATFORMIO_CORE_DIR=/mnt/ssd/.platformio
cd software/cardputer-client
pio run --environment m5stack-cardputer                                    # ビルド
pio run --target upload --environment m5stack-cardputer --upload-port /dev/ttyACM1  # 書き込み
```

## Cardputer 固有の注意点

- **`M5Cardputer.begin(true)` / `M5Cardputer.update()` を使うこと。** `M5.begin()` / `M5.update()` ではキーボードが動作しない。
- **物理ボタン A/B/C は存在しない。** 入力は 56 キーキーボードのみ。
- 日本語表示には `M5.Lcd.setFont(&fonts::efontJA_12)` を使用。

## サーバーサイド

- HTTP Bridge: `/home/pi/agents/project/openclaw-http-bridge/server.js`（systemd: `openclaw-http-bridge`）
- nginx: `/etc/nginx/sites-available/openclaw-proxy`（ポート 18800）
- OpenClaw Gateway: `ws://127.0.0.1:18789`（WebSocket のみ、HTTP REST API なし）

## ドキュメント

- `docs/system-spec.md` — システム仕様書
- `docs/feature-romaji-input.md` — ローマ字入力 機能仕様書
- `docs/feature-http-bridge.md` — HTTP Bridge 機能仕様書
- `docs/toolchain-spec.md` — ツールチェーン仕様書
- `docs/requirements.md` — 要求仕様書
- `docs/troubleshooting.md` — トラブルシューティング
