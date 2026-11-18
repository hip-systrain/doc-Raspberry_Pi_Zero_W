## 1. Raspberry Pi 側の設定（シリアル通信の有効化）

### シリアルコンソールの有効化
1. 端末で `sudo raspi-config` を実行する。
2. **3 Interface Options** ➔ **I6 Serial Port** を選択する。
3. 「ログインシェルをシリアル経由で使えるようにするか？」に **はい (Yes)** と答える。

### UARTの有効化確認
設定ファイルを確認・追記する：

```bash
cat /boot/config.txt | grep uart
```

> ※ない場合は、`sudo nano /boot/config.txt` などで末尾に `enable_uart=1` を追記する。

### 再起動
設定を反映させるため再起動する：

```bash
sudo reboot
```

### 割り当ての確認
再起動後、以下のコマンドで `serial0` が `ttyS0` に結びついていることを確認する：

```bash
ls -l /dev/serial*
```

* 出力例: `serial0 -> ttyS0`

---

## 2. 物理配線の確認（クロス接続）

USBシリアル変換モジュール（CH340）とRaspberry Piのピンを正しく交差させて接続します。

* 変換モジュールの **TX** ➔ ラズパイの **RX**
* 変換モジュールの **RX** ➔ ラズパイの **TX**
* 変換モジュールの **GND** ➔ ラズパイの **GND**

---

## 3. Windows 側（Tera Term）の接続設定

1. Tera Termを起動し、最初の「新しい接続」画面は一旦キャンセルで閉じる。
2. メニューの **[設定]** ➔ **[シリアルポート]** を開く。
3. 以下のパラメータを設定する：
   * **Port**: 認識されているCOMポート（例：COM3 など）
   * **Baud rate (スピード)**: `115200`
   * **その他**: Data=`8bit`, Parity=`none`, Stop=`1bit`, Flow control=`none`

# Pythonサンプルプログラム
1. No1 赤LEDを0.5秒ごとに点滅
2. No2 白スイッチを押している間だけ赤LEDを点灯
3. No3 白スイッチを押した回数をパソコンに表示
4. No4 一定の周期で赤LEDを点滅
--- 下記作成中 ---
5. No5
6. No6
