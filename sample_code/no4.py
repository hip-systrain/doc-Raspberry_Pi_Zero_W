# coding: UTF-8
import struct
import RPi.GPIO as GPIO
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

# GPIOの設定                # LED1のBCM番号の指定
GPIO.setmode(GPIO.BCM)         # BCM番号をナンバリングで使用
GPIO.setwarnings(False)        # 以前の状態の警告文を抑制
GPIO.setup(LED1,GPIO.OUT)      # LED1のBCM番号を出力に設定

def main():
    fd = os.timerfd_create(clock)
    os.timerfd_settime(fd, initial=sec_initial, interval=sec_interval)
    led_state = False
    GPIO.output(LED1,led_state)

    try:
        print("Ctrl+C:プログラム終了\n")
        while True:
            buf = os.read(fd, bite)
            total = struct.unpack("Q", buf)[0]

            if (0 != total):
                led_state = not led_state
                GPIO.output(LED1,led_state)
    except KeyboardInterrupt:
        print("\n動作を終了します。")
        GPIO.cleanup()
        sys.exit()

if __name__ == '__main__':
    main()