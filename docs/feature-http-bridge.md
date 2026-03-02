# 機能仕様書: OpenClaw HTTP Bridge

## 1. 概要

OpenClaw Gateway は WebSocket プロトコルのみをサポートしており、ESP32 の HTTP クライアントから直接通信できない。本ブリッジは Raspberry Pi 上で動作する軽量な HTTP サーバーで、HTTP POST リクエストを受け取り、`openclaw agent` CLI コマンドに変換して実行する。

## 2. 背景

| 課題 | 詳細 |
|---|---|
| プロトコル不一致 | OpenClaw Gateway は `ws://127.0.0.1:18789` で WebSocket のみ対応 |
| HTTP 非サポート | Gateway の HTTP エンドポイントにPOSTすると 405 Method Not Allowed |
| ESP32 の制約 | WebSocket クライアント実装は可能だが、OpenClaw の WS プロトコル仕様が複雑 |

## 3. アーキテクチャ

```
Cardputer                 nginx              HTTP Bridge           OpenClaw
  │                        │                    │                    │
  │─── POST /api/v1/prompt →│                    │                    │
  │                        │─── POST /prompt ──→│                    │
  │                        │                    │── execFile ───────→│
  │                        │                    │   openclaw agent   │
  │                        │                    │   --agent main     │
  │                        │                    │   --message <text> │
  │                        │                    │   --json           │
  │                        │                    │← JSON stdout ─────│
  │                        │← {"response":...} ─│                    │
  │← {"response": "..."}  ─│                    │                    │
```

## 4. コンポーネント詳細

### 4.1 HTTP Bridge サーバー

- **実装ファイル:** `/home/pi/agents/project/openclaw-http-bridge/server.js`
- **ランタイム:** Node.js v22（組み込み `http` / `child_process` モジュールのみ使用）
- **外部依存:** なし（`npm install` 不要）
- **バインド:** `127.0.0.1:18801`（localhost のみ、nginx 経由でのみアクセス可）

### 4.2 API エンドポイント

#### POST /prompt（または /api/v1/prompt）

プロンプトを OpenClaw に送信し、AI の応答を返す。

**リクエスト:**
```
POST /prompt HTTP/1.1
Content-Type: application/json

{"prompt": "今日の天気は？", "language": "ja-JP", "source": "cardputer"}
```

**処理:**
1. リクエストボディの JSON をパース
2. `prompt` フィールドを抽出（必須、文字列型）
3. `openclaw agent --agent main --message <prompt> --json --timeout 120` を `execFile` で実行
4. CLI の標準出力（JSON）をパースし、`result.payloads[0].text` を抽出
5. HTTP 200 でレスポンスを返却

**レスポンス（成功）:**
```json
{"response": "今日は晴れの予報です。"}
```

**レスポンス（エラー）:**
```json
{"error": "エラーメッセージ"}
```

| ステータス | 条件 |
|---|---|
| 200 | 正常応答 |
| 400 | `prompt` フィールドが未指定または非文字列 |
| 502 | OpenClaw CLI がエラー終了（stderr を返却） |

#### GET /health

ヘルスチェック用。

**レスポンス:**
```json
{"status": "ok", "timestamp": "2026-03-02T10:20:02.134Z"}
```

### 4.3 OpenClaw CLI 呼び出し

| パラメータ | 値 |
|---|---|
| コマンド | `/home/pi/.npm-global/bin/openclaw` |
| 引数 | `agent --agent main --message <text> --json --timeout 120` |
| タイムアウト | 120,000 ms |
| 実行方法 | `child_process.execFile`（シェル経由しない） |

**CLI 出力の解析:**

```json
{
  "runId": "...",
  "status": "ok",
  "result": {
    "payloads": [
      {
        "text": "応答テキスト",
        "mediaUrl": null
      }
    ]
  }
}
```

→ `result.payloads[0].text` を抽出してレスポンスとする。パース失敗時は stdout 全体をそのまま返す。

## 5. nginx ルーティング

**設定ファイル:** `/etc/nginx/sites-available/openclaw-proxy`

| パス | プロキシ先 | タイムアウト |
|---|---|---|
| `/api/v1/prompt` | `http://127.0.0.1:18801/prompt` | 接続 5秒 / 読取 120秒 |
| `/`（その他） | `http://127.0.0.1:18789` | 接続 5秒 / 読取 30秒 |

**アクセス制御:** `allow 192.168.x.0/24; deny all;`

## 6. プロセス管理

**systemd ユニット:** `/etc/systemd/system/openclaw-http-bridge.service`

| 設定 | 値 |
|---|---|
| User | pi |
| ExecStart | `/usr/bin/node /home/pi/agents/project/openclaw-http-bridge/server.js` |
| Restart | always（RestartSec=5） |
| After | network.target |
| WantedBy | multi-user.target |
| PATH | `/home/pi/.npm-global/bin` を含む（openclaw コマンド解決用） |

**操作コマンド:**
```bash
sudo systemctl start openclaw-http-bridge     # 起動
sudo systemctl stop openclaw-http-bridge      # 停止
sudo systemctl restart openclaw-http-bridge   # 再起動
sudo systemctl status openclaw-http-bridge    # 状態確認
journalctl -u openclaw-http-bridge -f         # ログ監視
```

## 7. ログ出力

stdout に以下の形式で出力される（journalctl で参照可能）:

```
2026-03-02T10:20:02.134Z [prompt] "今日の天気は？"
2026-03-02T10:20:05.678Z [reply]  "今日は晴れの予報です。"
2026-03-02T10:20:10.000Z [error]  "Command failed: ..."
```

## 8. 制約事項

| 項目 | 内容 |
|---|---|
| 同時処理 | 逐次処理。同時リクエストは Node.js のイベントループで順番に処理 |
| ストリーミング | 非対応。応答全体を受信後に一括返却 |
| セッション | なし。各リクエストは独立した `openclaw agent` プロセスを起動 |
| 認証 | Bridge 自体は認証なし（nginx の IP 制限に依存） |
| プロトコル | HTTP のみ（HTTPS 非対応、LAN 内通信のため許容） |
