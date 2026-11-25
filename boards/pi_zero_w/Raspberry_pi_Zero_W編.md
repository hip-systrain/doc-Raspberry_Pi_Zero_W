## 7. トラブルシューティング

| 症状 | 原因 | 対処 |
|---|---|---|
| Raspberry Pi Zero W の電源を入れられない | 電源用 USB ケーブルが接続されていないため。 | micro-B USB ケーブルと電源を確認する。Raspberry Pi Zero W の電源端子に接続する。(GPIOピンからは給電できない。) |
| SSH 接続できない | Wi-Fi の SSID やパスワードが正しく設定されていないため。 | Raspberry Pi Imager でWi-Fi の SSID・パスワードを再設定する。PCとRaspberry Pi Zero Wの両方で研修で指定されたネットワークを使用する。 |
| ping コマンドでは応答があるが、SSH 接続できない | Raspberry Pi Zero W がネットワークに接続されていても、接続の準備が完了していないため。 | Raspberry Pi Zero Wの起動完了を待ち、再度 SSH 接続する。 |
| SSH 接続先が想定と異なる Raspberry Pi Zero W になる | 複数の Raspberry Pi Zero W に同じホスト名を設定しているため。 | Raspberry Pi Zero W ごとに異なるホスト名を設定する。接続先の IP アドレスやホスト名を確認してから SSH 接続する。 |
| GPIO 制御プログラムをコンパイルすると `fatal error: gpiod.h: No such file or directory` が表示される | libgpiod の開発用ヘッダーファイルがインストールされていないため。 | Raspberry Pi Zero W のターミナルで次のコマンドを実行する。<br>`sudo apt install -y libgpiod-dev`<br>インストール後、再度コンパイルする。 |
| GDB の `info locals` や `info registers` で変数・レジスタを確認できない | プログラムが終了している、または確認対象の処理位置に停止していないため。 | ブレークポイントを設定し、プログラムの実行中に停止させてから確認する。 |
| GPIO 制御プログラムを実行しても LED が点灯しない | 直前に実行した GPIO 操作コマンドが GPIO を使用したままになっている可能性があるため。 | GPIO を操作しているコマンドやプログラムを終了する。改善しない場合は Raspberry Pi Zero W を再起動する。プログラムでは GPIO の使用終了処理を行う。 |
| シリアル接続の Tera Term 画面に何も表示されない | Raspberry Pi Zero W のシリアルポートやシリアルログインが無効になっているため | SSH 接続で Raspberry Pi Zero W に接続し、`sudo raspi-config` を実行する。`Interface Options` → `Serial Port` を選択し、シリアルログインとシリアルポートの設定を有効にする。設定後、Raspberry Pi Zero W を再起動する。 |
| シリアル接続で `login:` は表示されるが、ユーザー名を入力できない | PC から Raspberry Pi Zero W への送信経路が接続されていないため。 | Raspberry Pi Zero W の GPIO15 RXD（物理ピン10）と、CH340E の TXD を接続する。Raspberry Pi Zero W の GPIO14 TXD（物理ピン8）と CH340E の RXD、GND 同士も接続する。 |
| Tera Term で新しい接続を作成すると、シリアルを選択できない | 別のアプリケーションが使用しているため。 | Tera Term の接続をすべて閉じてから再度接続する。 |
