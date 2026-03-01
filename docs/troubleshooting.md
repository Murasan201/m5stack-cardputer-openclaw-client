# Cardputer × OpenClaw プロジェクト トラブルシューティング

このページでは本プロジェクトでこれまで対処したエラーと、今後発生が想定されるトラブル・その対応策を整理します。進行中に新しい事案が発生したら随時追記してください。

## 1. すでに体験したエラーと対応

### 1.1 `pio: command not found`
- **症状**: `software/cardputer-client` のビルドを試行するために `pio run` を叩いたところ、システムが `pio` を認識せず、コマンドが実行できなかった。
- **原因**: PlatformIO Core (`pio` CLI) がこのホストにインストールされていない。
- **対応策**:
  1. Python 仮想環境上で `pip install platformio` もしくは `pipx install platformio` を行い、`pio` コマンドを利用可能にする。
  2. あるいは OS 側で PlatformIO を apt/yum などからインストールし、`pio` を PATH に含める。
  3. `software/cardputer-client` 内で `pio run --environment m5stack-cardputer` を実行すればビルドとアップロードが可能になる。
  4. 本環境ではまだ `pio` が無いため、当面はユーザー側のマシンでビルドしていただき、ログを共有いただければアドバイスします。

### 1.2 `ModuleNotFoundError: No module named 'pytest'`
- **症状**: `python -m pytest project/m5stack-cardputer-app/tests` を実行した際に `pytest` モジュールが見つからないエラー。
- **原因**: Python 環境に pytest がインストールされていなかった。
- **対応策**:
  1. リポジトリルートで `python -m venv .venv` で仮想環境を作成し、`. .venv/bin/activate` で有効化。
  2. `pip install pytest` を実行してテスト依存を導入。
  3. README に仮想環境構築手順と実行コマンド (`python -m pytest project/m5stack-cardputer-app/tests`) を追記済み。

### 1.3 `ModuleNotFoundError: No module named 'state_machine'` / `communication`
- **症状**: tests 内で `from state_machine import ...` した際にモジュールが見つからず、pytest のコレクションが失敗。
- **原因**: Python のモジュール検索パスに `project/m5stack-cardputer-app/tests` ディレクトリが存在しておらず、同梱のヘルパモジュールを import できなかった。
- **対応策**:
  1. テストファイルの冒頭で `sys.path` に `pathlib.Path(__file__).resolve().parent` を追加し、同じディレクトリを検索対象に加えた。
  2. あるいは `tests/__init__.py` を利用して `from .state_machine import ...` する構成にすることで解決。
  3. 修正後、7 件の pytest が実行されすべて成功している。

## 2. 今後想定されるエラーと対処

### 2.1 Wi-Fi 接続失敗 / 認証エラー
- **症状**: `NetworkClient::connectWifi` が `WL_CONNECTED` を返さず、`PromptResponse` に「Wi-Fi が接続されていません」というエラー文字列が入る。
- **想定原因**: SSID/PW の誤入力、電波マスターとの距離、Wi-Fi モジュールの初期化失敗。`config.h` の値を書き換えた後に本体をリセットし忘れた可能性も。
- **対応策**:
  1. `src/config.h` の `WIFI_SSID` / `WIFI_PASSWORD` を見直し、正しい情報を再設定。
  2. Cardputer 側で Wi-Fi LED や `M5.Lcd.println` で現在ステータスを確認。
  3. 障害が続く場合は別のアクセスポイントで接続確認を行い、`WIFI_CONNECT_TIMEOUT_MS` を長く取る。
  4. 接続が安定しない環境では有線 USB-OTG など別通信経路への切り替えを検討。

### 2.2 OpenClaw 側への HTTP エラー（500 / 401 / タイムアウト）
- **症状**: `NetworkClient::postPrompt` が `HTTP_CODE_OK` 以外のコードを返し、`response.error` に `http.errorToString(code)` が入る。モード通知でも 401 や 403 が返る可能性。
- **想定原因**: エンドポイント URL (`OPENCLAW_PROMPT_URL`) が誤っている、OpenClaw サーバが応答しない、認証トークンが無効、OpenClaw 側で何か例外。
- **対応策**:
  1. `OPENCLAW_PROMPT_URL` / `OPENCLAW_MODE_URL` が現場の OpenClaw に向いているか確認（IP/ポート/パス）。
  2. 必要なら `OPENCLAW_AUTH_TOKEN` を設定し、OpenClaw 側の `Authorization` チェックに合わせる。
  3. OpenClaw サーバーのログ（`/var/log/openclaw/...`）を確認し、リクエストペイロードの整合性や例外スタックを調べる。
  4. 接続タイムアウトは `PROMPT_RESPONSE_TIMEOUT_MS` の値を伸ばすことでゆとりを持たせ、Wi-Fi の不安定さを吸収。

### 2.3 JSON パースエラー / 応答フィールド欠落
- **症状**: `deserializeJson` が `DeserializationError` を返す、または `parsed.containsKey("response")` が false になり、画面に `http.getString()` の生データを表示するだけになる。
- **想定原因**: OpenClaw 回答が JSON 形式でない（プレーンテキスト）、または `response` フィールド名を変更している。文字コードや BOM の影響で parse に失敗することも。
- **対応策**:
  1. OpenClaw に送る `prompt` の payload 形式をドキュメントと照合し、必要なら `network_client.cpp` の解析部を調整。
  2. `response` フィールドがなければ `http.getString()` をそのまま `response.text` に入れるフォールバックを設け済み。
  3. 回答が長文の場合は `DisplayManager::printWrapped` で複数行出力して画面からはみ出さないようにしている。

### 2.4 LCD 表示のリフレッシュ/文字化け
- **症状**: 対話モード中にテキストが残る・表示がちらつく・日本語が化ける。
- **想定原因**: Buffer クリア処理が足りない、TFT のバックグラウンド色とテキスト色が被る、またはフォントに含まれない文字を表示しようとしている。
- **対応策**:
  1. `DisplayManager::showPromptMode` では画面全体を `TFT_NAVY` で塗りつぶした後に描画しているので、末尾が残る場合は `clearRegion` を呼ぶ。
  2. **日本語文字化けの解決済み事例**: デフォルトフォント（ASCII のみ）では日本語が化ける。`M5.Lcd.setFont(&fonts::efontJA_12)` で M5GFX 内蔵の日本語フォント（efont）を使用すること。利用可能なサイズ: `efontJA_10`, `efontJA_12`, `efontJA_14`, `efontJA_16`, `efontJA_24`。
  3. 文字列が 320px を超える場合は `printWrapped` で折り返し済みだが、必要ならスクロール機能を追加する。

### 2.5 キーボード入力が無反応（解決済み）
- **症状**: キーを押しても画面に一切変化がない。
- **原因（1）**: 当初 `M5.BtnA.wasPressed()` / `M5.BtnB` / `M5.BtnC` で入力判定をしていたが、**Cardputer には物理ボタン A/B/C が存在しない**（56 キーキーボードのみ）。そのため `wasPressed()` は常に false。
- **対応策（1）**: `M5Cardputer.Keyboard` API に全面切替。`M5Cardputer.Keyboard.isChange()` / `keysState()` でキー入力を検出し、Enter（送信）、Del（削除）、Tab（モード切替）、文字キー（入力）で操作する方式に変更。
- **原因（2）**: API を切り替えても依然無反応だった。`main.cpp` で `M5.begin()` / `M5.update()` を使っていたが、これでは**キーボードが初期化・更新されない**。
- **対応策（2）**: `M5Cardputer.begin(true)` / `M5Cardputer.update()` に変更して解決。`M5Cardputer` クラスを使わないとキーボードドライバが有効にならない。
- **教訓**: Cardputer 開発では必ず `M5Cardputer.begin()` / `M5Cardputer.update()` を使用すること。`M5.begin()` / `M5.update()` では LCD は動くがキーボードは動かない。

### 2.7 ローマ字入力で「ん」が即確定し「に」「な」等が入力できない（解決済み）
- **症状**: 日本語モードで `n` を押すと即座に「ん」に変換され、「に」(`ni`)、「な」(`na`) 等の `n` で始まるひらがなが入力できない。
- **原因**: ローマ字→ひらがな変換テーブルに `{"n", "ん"}` が含まれていたため、`n` 単体で即座にマッチして変換されていた。
- **対応策**:
  1. テーブルから `{"n", "ん"}` エントリを除外。
  2. `n` の変換を特別扱いに変更: `n` + 子音（`a/i/u/e/o/y/n` 以外）→「ん」+ 残りを継続変換。`n` + 母音 → 通常のな行変換（`na`→な、`ni`→に 等）。
  3. `nn` → 「ん」（テーブルに残存）。
  4. 送信時・モード切替時に `flushPending()` で残った `n` を「ん」に変換。
- **関連ファイル**: `src/prompt_input.cpp` の `tryConvertRomaji()` と `flushPending()`

### 2.8 Wi-Fi 未接続のまま送信してエラー（解決済み）
- **症状**: プロンプト入力後 Enter で送信すると「Wi-Fi が接続されていません」と表示され送信できない。
- **原因**: `NetworkClient::begin()` での Wi-Fi 接続が失敗していたが、画面に接続状態が表示されないためユーザーが気付けなかった。
- **対応策**:
  1. `begin()` で `WiFi.disconnect(true)` を呼び前セッションをクリアしてから再接続。
  2. 起動時に LCD に「Wi-Fi 接続中...」→成功時「接続成功: (IPアドレス)」/ 失敗時「接続失敗」を表示。
  3. `ensureConnected()` の再試行間隔を 1 秒→3 秒に変更。

### 2.9 OpenClaw への送信でエラー発生（未解決）
- **症状**: Wi-Fi 接続成功後にプロンプトを送信すると「エラー発生」と表示され応答が得られない。
- **想定原因**:
  1. nginx プロキシ → OpenClaw gateway 間の通信失敗
  2. OpenClaw API のエンドポイント (`/api/v1/prompt`) やペイロード形式の不一致
  3. Bearer トークンの認証失敗（401/403）
  4. OpenClaw gateway 自体が停止中
  5. HTTPレスポンスのタイムアウト（`PROMPT_RESPONSE_TIMEOUT_MS` = 12秒）
- **次回調査手順**:
  1. Raspberry Pi から `curl` で手動 API テスト: `curl -X POST http://127.0.0.1:18789/api/v1/prompt -H "Content-Type: application/json" -H "Authorization: Bearer ..." -d '{"prompt":"test"}'`
  2. nginx アクセスログ確認: `tail /var/log/nginx/access.log`
  3. OpenClaw gateway のステータス確認: `systemctl status openclaw` or プロセス確認
  4. エラー画面に HTTP ステータスコードを表示する改修を検討
- **関連ファイル**: `src/network_client.cpp`、`/etc/nginx/sites-available/openclaw-proxy`

### 2.6 OpenClaw gateway にローカルネットワークから接続できない
- **症状**: Cardputer から `http://YOUR_RASPI_IP:18789` に接続しても応答がない。
- **原因**: OpenClaw gateway は `bind: "loopback"` で `127.0.0.1` のみリッスンしている。
- **対応策（採用済み）**:
  1. nginx リバースプロキシをポート 18800 で設置し、`proxy_pass http://127.0.0.1:18789` でローカルに中継。
  2. nginx 設定で `allow 192.168.11.0/24; deny all;` によりローカルネットワークのみ許可。
  3. UFW で `18800/tcp` を `192.168.11.0/24` からのみ許可。
  4. `config.h` の URL を `http://YOUR_RASPI_IP:18800/api/v1/prompt` に設定。
- **関連ファイル**: `/etc/nginx/sites-available/openclaw-proxy`

## 3. ビルド／実行環境の状況と備考
- **Python テスト**: `.venv` に `pytest` を入れた状態で `python -m pytest tests` を実行し、7 件のテストはすべてパスしている。仮想環境構築手順は `tests/README.md` に記載。
- **PlatformIO ビルド**: `.venv` に PlatformIO 6.1.19 をインストール済み。ツールチェーンは SSD (`/mnt/ssd/.platformio`) に配置。
  ```bash
  source .venv/bin/activate
  export PLATFORMIO_CORE_DIR=/mnt/ssd/.platformio
  cd software/cardputer-client
  pio run --environment m5stack-cardputer
  pio run --target upload --environment m5stack-cardputer --upload-port /dev/ttyACM1
  ```
- **認証情報**: `config.h` に Wi-Fi パスワードと OpenClaw トークンを含むため `.gitignore` で除外。`config.h.example` をテンプレートとして使用すること。
- **詳細なツールチェーン情報**: `docs/toolchain-spec.md` を参照。

以上の内容を参照しつつ、トラブルが発生した場合はこのドキュメントを更新してください。
