. Raspberry Pi 側の設定（シリアル通信の有効化）

シリアルコンソールの有効化

端末で sudo raspi-config を実行する。

3 Interface Options ➔ I6 Serial Port を選択。

「ログインシェルをシリアル経由で使えるようにするか？」に はい (Yes) と答える。

UARTの有効化確認

設定ファイルを確認・追記する：

Bash
cat /boot/config.txt | grep uart

※ない場合は、sudo nano /boot/config.txt などで末尾に enable_uart=1 を追記する。

再起動

設定を反映させるため再起動する：

Bash
sudo reboot

割り当ての確認

再起動後、以下のコマンドで serial0 が ttyS0 に結びついていることを確認する：

Bash
ls -l /dev/serial*

(出力例: serial0 -> ttyS0)
2. 物理配線の確認（クロス接続）

USBシリアル変換モジュール（CH340）とRaspberry Piのピンを正しく交差させて接続します。

変換モジュールの TX ➔ ラズパイの RX

変換モジュールの RX ➔ ラズパイの TX

変換モジュールの GND ➔ ラズパイの GND
3. Windows 側（Tera Term）の接続設定

Tera Termを起動し、最初の「新しい接続」画面は一旦キャンセルで閉じる。

メニューの [設定] ➔ [シリアルポート] を開く。

以下のパラメータを設定する：

Port: 認識されているCOMポート（例：COM3 など）

Baud rate (スピード): 115200

（その他：Data=8bit, Parity=none, Stop=1bit, Flow control=none）