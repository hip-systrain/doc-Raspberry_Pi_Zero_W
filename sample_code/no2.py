# coding: UTF-8
import RPi.GPIO as GPIO
import time
import sys
import os

# 出力ピン番号をBCMで指定する
LED1 = 17  # 赤のLED
SW1 = 24   # 白のタクトSW(SW1)
SLEEP_TIME = 0.05

# GPIOの設定
GPIO.setmode(GPIO.BCM)
GPIO.setwarnings(False)
GPIO.setup(LED1,GPIO.OUT)
# SW1の入力をプルダウン設定
GPIO.setup(SW1,GPIO.IN,pull_up_down=GPIO.PUD_DOWN)

try:
    print("SW1を押す:LED1(赤)点灯\nCtrl+C:プログラム終了 ")
    while True:
        if ( GPIO.input(SW1) == GPIO.HIGH ):            
            GPIO.output(LED1,True)
        else:
            GPIO.output(LED1,False)   
        time.sleep(SLEEP_TIME)    

except KeyboardInterrupt:
    print("\n動作を終了します。")

finally:
    # クリーンアップして終了
    GPIO.cleanup()
    sys.exit()