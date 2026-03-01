# テスト概要

このディレクトリには、Cardputer ↔ OpenClaw の対話機能を想定した単体テストが含まれています。実機検証はユーザー自身が行う予定のため、以下のコードベースのテストで設計の妥当性を保証します。

## 用意したテスト
- `state_machine.py`: セッション状態の許容トランジションを定義し、異常遷移や連続遷移の検証を行う。  
- `communication.py`: Cardputer から OpenClaw に送るイベント／応答メッセージの構築ヘルパを提供し、テストで payload の構造を確認。
- `test_state_machine.py`: `StateMachine` の初期状態、正しい遷移、誤った遷移時の例外、連続シーケンスの検証。
- `test_communication.py`: prompt/mode/response イベントに含まれるフィールドとタイムスタンプ形式を検証。

## 実行方法
1. Python 3.10+ を使う想定です。`python -m venv .venv` などで仮想環境を作り、`. .venv/bin/activate` で有効化したあとに `pip install pytest` を実行してください。  
2. リポジトリルートで `python -m pytest project/m5stack-cardputer-app/tests` を実行します。

```bash
cd /home/pi/.openclaw/workspace
python -m pytest project/m5stack-cardputer-app/tests
```

## 実機での最終確認（ユーザー担当）
1. Cardputer でプロンプトモードキーを押し、LCD に対話モード起動メッセージが表示されることを確認。  
2. 日本語入力を行いながら LCD 上に文字が出ること（変換中・確定など）。  
3. 送信ボタンを押すと OpenClaw に日本語プロンプトが送られ、応答が LCD に表示されること。  
4. 通信エラー時は再試行メッセージ／状態遷移が発生すること。  
5. 対話終了時は状態が待機に戻ること。

必要あれば、実機テストのチェックリストを `project/m5stack-cardputer-app/docs/` へ移しても構いません。ご希望があればタスク化します。