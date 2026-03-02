# システム仕様書

M5Stack Cardputer × OpenClaw 対話クライアント

## 1. システム概要

M5Stack Cardputer（ESP32-S3 搭載小型端末）を OpenClaw AI エージェントの手持ち対話端末として使用するシステム。ユーザーは Cardputer の 56 キーキーボードからローマ字入力で日本語プロンプトを入力し、Wi-Fi 経由で Raspberry Pi 上の OpenClaw に送信、AI からの応答を LCD に表示する。

## 2. システム構成

### 2.1 全体アーキテクチャ

```
┌─────────────────────┐       Wi-Fi        ┌──────────────────────────────┐
│  M5Stack Cardputer   │  ──── HTTP POST ──→ │  Raspberry Pi 5              │
│  (ESP32-S3FN8)       │  ←── JSON resp ──  │                              │
│                      │   port 18800       │  nginx (port 18800)          │
│  ・56キーキーボード    │                    │    ↓ /api/v1/prompt          │
│  ・1.14" TFT LCD     │                    │  HTTP Bridge (port 18801)    │
│  ・Wi-Fi 802.11b/g/n │                    │    ↓ execFile                │
└─────────────────────┘                    │  openclaw agent CLI          │
                                           │    ↓ WebSocket               │
                                           │  OpenClaw Gateway (18789)    │
                                           │    ↓                         │
                                           │  LLM (gpt-5.1-codex-mini)   │
                                           └──────────────────────────────┘
```

### 2.2 コンポーネント一覧

| コンポーネント | 実行場所 | 技術 | 役割 |
|---|---|---|---|
| Cardputer Client | M5Stack Cardputer | C++ / Arduino / PlatformIO | UI・入力・通信 |
| nginx | Raspberry Pi | nginx | リバースプロキシ・アクセス制御 |
| HTTP Bridge | Raspberry Pi | Node.js (v22) | HTTP→CLI 変換 |
| OpenClaw Gateway | Raspberry Pi | openclaw-gateway (npm) | AI エージェント基盤 |

### 2.3 ハードウェア仕様

**M5Stack Cardputer:**

| 項目 | 値 |
|---|---|
| SoC | ESP32-S3FN8（Xtensa LX7 デュアルコア, 240MHz） |
| Flash | 8MB |
| RAM | 320KB SRAM |
| ディスプレイ | 1.14" TFT ST7789V2（240×135 ピクセル） |
| 入力 | 56 キーキーボード（QWERTY 配列） |
| 通信 | Wi-Fi 802.11 b/g/n、Bluetooth 5.0 LE |
| USB | USB-C（USB OTG / Serial / JTAG） |
| 電源 | 内蔵リチウムバッテリー / USB-C 給電 |

**Raspberry Pi 5:**

| 項目 | 値 |
|---|---|
| OS | Debian Bookworm (arm64) |
| ストレージ | microSD 28GB（ルート）+ SSD 117GB（/mnt/ssd） |
| ネットワーク | Wi-Fi (LAN) + Tailscale (WAN) |

## 3. 通信仕様

### 3.1 通信フロー

```
Cardputer → (Wi-Fi) → nginx:18800 → bridge:18801 → openclaw CLI → Gateway:18789 (WS)
```

### 3.2 HTTP API（Cardputer → nginx → Bridge）

**エンドポイント:** `POST /api/v1/prompt`

**リクエスト:**
```json
{
  "prompt": "入力テキスト",
  "language": "ja-JP",
  "source": "cardputer"
}
```

**レスポンス（成功時）:**
```json
{
  "response": "OpenClaw からの応答テキスト"
}
```

**レスポンス（エラー時）:**
```json
{
  "error": "エラーメッセージ"
}
```

**HTTP ステータスコード:**

| コード | 意味 |
|---|---|
| 200 | 成功 |
| 400 | リクエスト不正（prompt フィールド欠損） |
| 502 | OpenClaw CLI 実行エラー |
| 404 | エンドポイント不明 |

**ヘルスチェック:** `GET /health` → `{"status": "ok", "timestamp": "..."}`

### 3.3 タイムアウト設定

| 区間 | タイムアウト |
|---|---|
| Cardputer HTTP 接続 | 120 秒 |
| Cardputer HTTP 読取 | 120 秒 |
| nginx → Bridge 接続 | 5 秒 |
| nginx → Bridge 読取 | 120 秒 |
| Bridge → openclaw CLI | 120 秒 |

### 3.4 セキュリティ

| レイヤー | 対策 |
|---|---|
| ネットワーク | nginx で LAN（192.168.x.0/24）のみ許可、他は deny |
| ファイアウォール | UFW でポート 18800 を LAN のみ許可 |
| 認証 | config.h に Bearer トークンを設定可（現在は Bridge 側で不要） |
| 機密管理 | config.h は .gitignore 対象、config.h.example をテンプレートとして提供 |
| Gateway | loopback バインド（127.0.0.1 のみ） |

## 4. Cardputer Client 内部構成

### 4.1 モジュール構成

```
main.cpp
  └── DialogueManager    … セッション状態管理・入力ハンドリング
        ├── DisplayManager    … LCD 描画
        ├── NetworkClient     … Wi-Fi 接続・HTTP 通信
        └── PromptInput       … テキスト入力・ローマ字→ひらがな変換
```

### 4.2 セッション状態遷移

```
         任意キー押下
 [Idle] ──────────→ [Prompt]
   ↑                   │
   │                   │ Enter
   │                   ↓
   │              [Sending]
   │               ╱     ╲
   │         成功 ╱       ╲ 失敗
   │            ↓           ↓
   │     [Responding]   [Error]
   │          │            │
   └──────────┘            │
      任意キー押下          │
   ←───────────────────────┘
         任意キー押下
```

| 状態 | 説明 | LCD 表示 |
|---|---|---|
| Idle | 起動直後・待機中 | 「OpenClaw Client」「何かキーを押して開始」 |
| Prompt | テキスト入力中 | 入力バッファ・モード表示・ヘルプ |
| Sending | OpenClaw 応答待ち | 「OpenClaw に送信中...」 |
| Responding | 応答表示中 | 応答テキスト・「何かキーで再入力」 |
| Error | エラー表示中 | エラーメッセージ・「何かキーで再試行」 |

### 4.3 キー操作一覧

| キー | Prompt 状態での動作 | 他の状態での動作 |
|---|---|---|
| 文字キー (a-z, 0-9, 記号) | 文字入力（日本語モード時はローマ字変換） | Prompt 状態へ遷移 |
| Enter | プロンプト送信 | Prompt 状態へ遷移 |
| Del (Backspace) | 1 文字削除（UTF-8 対応） | ― |
| Tab | 入力モード切替（日本語 ↔ ASCII） | ― |

### 4.4 リソース使用量

| リソース | 使用量 | 上限 | 使用率 |
|---|---|---|---|
| RAM | 48,640 bytes | 327,680 bytes | 14.8% |
| Flash | 1,337,009 bytes | 3,342,336 bytes | 40.0% |

## 5. サーバーサイド構成

### 5.1 HTTP Bridge

- **実装:** Node.js 組み込み `http` モジュール（外部依存なし）
- **バインド:** 127.0.0.1:18801
- **処理:** HTTP POST → `openclaw agent --agent main --message <text> --json` を `child_process.execFile` で実行
- **応答パース:** CLI の JSON 出力から `result.payloads[0].text` を抽出
- **プロセス管理:** systemd サービス `openclaw-http-bridge.service`（自動起動・自動再起動）

### 5.2 nginx リバースプロキシ

- **ポート:** 18800（外部公開）
- **ルーティング:**
  - `/api/v1/prompt` → `127.0.0.1:18801/prompt`（HTTP Bridge）
  - `/`（その他）→ `127.0.0.1:18789`（OpenClaw ダッシュボード）
- **アクセス制御:** LAN のみ許可

### 5.3 OpenClaw Gateway

- **プロトコル:** WebSocket（`ws://127.0.0.1:18789`）
- **バインド:** loopback のみ
- **認証:** トークンベース
- **モデル:** gpt-5.1-codex-mini（openai-codex プロバイダ）

## 6. ファイル構成

```
m5stack-cardputer-app/
├── software/cardputer-client/
│   ├── platformio.ini              … ビルド設定
│   └── src/
│       ├── main.cpp                … エントリポイント
│       ├── config.h                … 認証情報（.gitignore 対象）
│       ├── config.h.example        … 設定テンプレート
│       ├── dialogue_manager.h/cpp  … セッション状態管理
│       ├── display_manager.h/cpp   … LCD 描画
│       ├── network_client.h/cpp    … Wi-Fi・HTTP 通信
│       └── prompt_input.h/cpp      … テキスト入力・ローマ字変換
├── docs/
│   ├── requirements.md             … 要求仕様書
│   ├── system-spec.md              … 本書（システム仕様書）
│   ├── feature-romaji-input.md     … ローマ字入力 機能仕様書
│   ├── feature-http-bridge.md      … HTTP Bridge 機能仕様書
│   ├── toolchain-spec.md           … ツールチェーン仕様書
│   └── troubleshooting.md          … トラブルシューティング
├── journal.md                      … 開発ジャーナル
├── tests/                          … Python テスト
├── .gitignore
└── README.md
```

## 7. ビルド・デプロイ

### 7.1 Cardputer ファームウェア

```bash
source .venv/bin/activate
export PLATFORMIO_CORE_DIR=/mnt/ssd/.platformio
cd software/cardputer-client
pio run --environment m5stack-cardputer                                    # ビルド
pio run --target upload --environment m5stack-cardputer --upload-port /dev/ttyACM1  # 書き込み
```

### 7.2 サーバーサイド

```bash
# HTTP Bridge
sudo systemctl start openclaw-http-bridge    # 起動
sudo systemctl status openclaw-http-bridge   # 状態確認

# nginx
sudo nginx -t && sudo systemctl reload nginx  # 設定反映
```

## 8. 制約事項・既知の制限

| 項目 | 内容 |
|---|---|
| 漢字変換 | 未実装。ひらがなのみ入力可能 |
| 画面スクロール | 未実装。応答は最大 8 行まで表示 |
| 応答待ちUI | ローディングアニメーションなし（固定テキスト表示） |
| 暗号化 | HTTP（非 TLS）。LAN 内通信のため現状許容 |
| 同時接続 | Bridge は逐次処理（同時リクエスト非対応） |
| セッション維持 | なし。各プロンプトは独立したリクエスト |
