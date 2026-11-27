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
        current_state = button.is_pressed
        if current_state:
            led.on()
            if not last_state:
                print("LED点灯")
        else:
            led.off()

        last_state = current_state
        time.sleep(SLEEP_TIME)

except KeyboardInterrupt:
    print("\n動作を終了します。")

finally:
    # クリーンアップして終了
    led.close()
    button.close()
    sys.exit()