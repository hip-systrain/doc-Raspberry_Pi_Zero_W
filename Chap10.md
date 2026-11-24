## 10.Python編【追加章】
### 10.0. はじめに
なぜPythonを使うのかメリットを書く
### 10.1. 準備
ここでは、本課題を達成するのに必要なPythonの環境構築を行う。
#### 10.1.1. Pythonをインストールする。
前提条件として下記コマンドを実行し、バージョンが出力されていた場合は実施する必要がない。
```bash
python --version
Python 3.13.5
```
下記コマンドを実行し、aptを更新する。
```bash
sudo apt update
```
sudo(管理者権限)でコマンドを実行する為、自分のユーザーのパスワードを入力し、エンターを押す。
```bash
[sudo] password for <usr>:
```
下記ログが出力されることを確認すること
```bash
142 packages can be upgraded. Run 'apt list --upgradable' to see them.
```
下記コマンドを実行
```bash
sudo apt install -y python3 python3-pip python3-venv
```
```bash
python3 is already the newest version (3.13.5-1).
python3 set to manually installed.
python3-pip is already the newest version (25.1.1+dfsg-1+rpt1).
python3-pip set to manually installed.
python3-venv is already the newest version (3.13.5-1).
Summary:
  Upgrading: 0, Installing: 0, Removing: 0, Not Upgrading: 142
```
外部ライブラリのインストール
・gpiozero:
・serial:Pythonでシリアル通信を行うのに必要
```bash
sudo apt install -y python3-gpiozero python3-serial

python3-gpiozero is already the newest version (2.0.1-0+rpt1+trixie).
python3-serial is already the newest version (3.5-2).
python3-serial set to manually installed.
Summary:
  Upgrading: 0, Installing: 0, Removing: 0, Not Upgrading: 142
```
確認できたら完了。

### 10.2.1. 題材1:赤LEDを0.5秒ごとに点滅
```py
# no1.py
import time
from gpiozero import LED

COUNT = 5                            # 繰り返し
r_led = LED(17)                      # LED1のBCM番号の指定

try:
   print("LED点灯テスト")
   for i in range(COUNT, 0, -1):
      remain = COUNT - i + 1
      print("点灯回数", remain)
      r_led.on()
      time.sleep(0.5)
      r_led.off()
      time.sleep(0.5)
finally:
   r_led.close()
```
### 10.2.2. 題材2:白スイッチを押している間だけ赤LEDを点灯
```py
# no2.py
# coding: UTF-8
from gpiozero import LED, Button
import time
import sys

# 出力ピン番号をBCMで指定する
LED1 = 17  # 赤のLED
SW1 = 24   # 白のタクトSW(SW1)
SLEEP_TIME = 0.05

# デバイスの初期化
led = LED(LED1)
button = Button(SW1, pull_up=False)

try:
    print("SW1を押す:LED1(赤)点灯\nCtrl+C:プログラム終了 ")
    while True:
        if button.is_pressed:         
            led.on()
        else:
            led.off()   
        time.sleep(SLEEP_TIME)    

except KeyboardInterrupt:
    print("\n動作を終了します。")

finally:
    # クリーンアップして終了
    led.close()
    button.close()
    sys.exit()
```
### 10.2.3. 題材3:白スイッチを押した回数をパソコンに表示
```py
# no3.py
import serial
from gpiozero import Button
import traceback
import time

SW1 = 24
MESSEAGE = "Trigger SW Detected!"
count = 0

button = Button(SW1, pull_up=False)

class UART:
    def __init__(self):
        try:
            self.uartport = serial.Serial('/dev/ttyS0', 115200, timeout=10)
        except serial.SerialException:
            print(traceback.format_exc())
            exit(1)
        
        self.uartport.reset_output_buffer()
        time.sleep(1)
    def send_serial(self, count):
        try:
            print(MESSEAGE)
            data = f"Count:{count}\n".encode("utf-8")
            self.uartport.write(data)            
        except serial.SerialException:
            print(traceback.format_exc())
    def close(self):
        if hasattr(self, "uartport") and self.uartport.is_open:
            self.uartport.close()

def main(count):
    uart = UART()
    try:
        while True:
            time.sleep(0.1)
            if button.is_pressed:
                count +=1
                uart.send_serial(count)
                while button.is_pressed:
                    time.sleep(0.05)
    except KeyboardInterrupt:
        print("\n終了")
    finally:
        button.close()
        uart.close()

if __name__ == '__main__':
    main(count)
```
### 10.2.4. 題材4:一定の周期で赤LEDを点滅
```py
# no4.py
# coding: UTF-8
import struct
from gpiozero import LED
import time
import sys
import os

# 出力ピン番号をBCMで指定する
LED1 = 17  # 赤のLED
SLEEP_TIME = 0.05
clock = time.CLOCK_MONOTONIC
sec_initial  = 0.5
sec_interval = 0.5
bite = 8

# デバイスの初期化
led = LED(LED1)

def main():
    fd = os.timerfd_create(clock)
    os.timerfd_settime(fd, initial=sec_initial, interval=sec_interval)
    led_state = False
    led.off()

    try:
        print("Ctrl+C:プログラム終了\n")
        while True:
            buf = os.read(fd, bite)
            total = struct.unpack("Q", buf)[0]

            if (0 != total):
                led_state = not led_state
                led.value = led_state
    except KeyboardInterrupt:
        print("\n動作を終了します。")
        led.close()
        sys.exit()

if __name__ == '__main__':
    main()
```
### 10.2.5. 題材5:LEDとスイッチを別々のスレッドで動かす
```py
# no5.py
from gpiozero import LED, Button
import threading
import time
import sys

LED1 = 17                                                   # LED1のBCM番号の指定
SW1 = 24                                                    # 白のタクトSW(SW1)

led = LED(LED1)
button = Button(SW1, pull_up=False)

def sw_check(stop_event):
    last_state = False
    while not stop_event.is_set():
        current_state = button.is_pressed
        if current_state and not last_state :
            print("ボタン押下検知")
        last_state = current_state
        time.sleep(0.1)

def led_check(stop_event):
    led_state = False
    while not stop_event.is_set():
        led_state = not led_state
        led.value = led_state
        if True == led_state:
            print("LED点灯")
        else:
            print("LED消灯")
        stop_event.wait(timeout=0.5)
        
def main():
    stop_event = threading.Event()                         #イベント停止フラグ(初期値はFalse)を新規作成する
    thread1 = threading.Thread(target=sw_check,            #スレッド生成(スイッチ監視)
                               args=(stop_event,))
    thread2 = threading.Thread(target=led_check,           #スレッド生成(LED監視)
                               args=(stop_event,))
    
    try:
        print("Ctrl+C:プログラム終了\n")
        thread1.start()                                     #スレッド開始(スイッチ監視)
        thread2.start()                                     #スレッド開始(LED監視)
        while True:
            time.sleep(1)
    except KeyboardInterrupt:
        print("\n動作を終了します。")
        stop_event.set()
        thread1.join()                                      #スレッド停止(スイッチ監視)
        thread2.join()   
        led.close()                                   #スレッド停止(LED監視)
        button.close()
        sys.exit()

if __name__ == '__main__':
    main()
```
### 10.2.6. 題材6:白スイッチでLEDの点滅の速さを切り替える
```py
from gpiozero import LED, Button
import threading
import queue
import time
import sys

LED1 = 17                                                   # LED1のBCM番号の指定
SW1 = 24                                                    # 白のタクトSW(SW1)
led = LED(LED1)
button = Button(SW1, pull_up=False)

interval = [0.5, 0.2, 1.0]

def sw_check(stop_event, cmd_queue):
    raw = 1
    last_state = False
    while not stop_event.is_set():
        current_state = button.is_pressed
        if current_state and not last_state :
            print(f"ボタン押下検知\n点灯間隔:{interval[raw]}秒\n")
            cmd_queue.put(interval[raw])                   #キューにプッシュ
            raw = (raw + 1) % len(interval)
        last_state = current_state
        time.sleep(0.1)
        

def led_check(stop_event, cmd_queue):
    led_state = False
    current_interval = interval[0]
    last_toggle = time.monotonic()
    while not stop_event.is_set():
        try:
            new_interval = cmd_queue.get(timeout=0.05)     #キューから取得する(0.05秒おきに現在の点灯間隔を取得する)
            current_interval = new_interval            
        except queue.Empty:
            pass
        
        if (current_interval <= time.monotonic() - last_toggle):
            led_state = not led_state
            led.value = led_state
            last_toggle = time.monotonic()
        
def main():
    stop_event = threading.Event()                         #イベント停止フラグ(初期値はFalse)を新規作成する
    cmd_queue  = queue.Queue()                             #スレッド間通信用のキューを生成
    thread1 = threading.Thread(target=sw_check,            #スレッド生成(スイッチ監視)
                               args=(stop_event, cmd_queue))
    thread2 = threading.Thread(target=led_check,           #スレッド生成(LED監視)
                               args=(stop_event, cmd_queue))
    
    try:
        print("Ctrl+C:プログラム終了\n")
        thread1.start()                                     #スレッド開始(スイッチ監視)
        thread2.start()                                     #スレッド開始(LED監視)
        while True:
            time.sleep(1)
    except KeyboardInterrupt:
        print("\n動作を終了します。")
    finally:
        stop_event.set()
        thread1.join()                                      #スレッド停止(スイッチ監視)
        thread2.join()                                      #スレッド停止(LED監視)
        led.close()                                   
        button.close()
        sys.exit()

if __name__ == '__main__':
    main()
```
### 10.3.0. 所感
・CとPythonを比較した感想を書く。
Pythonはデバッグを行わずに、そのまま実行できるのが利点。

### 参考文献
Raspberry PiのセットアップからPython環境のインストールまで
https://qiita.com/nsuhara/items/05a2b41d94ced1f54316
おまいらのthreading.Eventの使い方は間違っている
https://qiita.com/tag1216/items/2dcb112f8018eb19a999
threading --- スレッドベースの並列処理
https://docs.python.org/ja/3/library/threading.html
queue --- 同期キュークラス
https://docs.python.org/ja/3/library/queue.html