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