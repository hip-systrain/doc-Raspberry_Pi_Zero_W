import RPi.GPIO as GPIO        # RPi.GPIOのライブラリ名をGPIOとして使用
import time                    # time.sleep用にtimeライブラリを使用

COUNT = 5                     # COUNTを10回にセット
LED1 = 17                      # LED1のBCM番号の指定
GPIO.setmode(GPIO.BCM)         # BCM番号をナンバリングで使用
GPIO.setwarnings(False)        # 以前の状態の警告文を抑制
GPIO.setup(LED1,GPIO.OUT)      # LED1のBCM番号を出力に設定

kaisu = COUNT                  # COUNTをループの回数に設定
for _ in range(COUNT):         # kaisuが0になるまでループ
    print("ledtest",kaisu)     # print文でkaisuの値を表示
    GPIO.output(LED1,1)        # LED1のBCM番号に1を出力（LED点灯）
    time.sleep(0.5)            # 1秒間スリープ
    GPIO.output(LED1,0)        # LED2のBCM番号に1を出力（LED点灯）
    time.sleep(0.5)            # 1秒間スリープ
    GPIO.output(LED1,1)        # LED1のBCM番号に1を出力（LED点灯）
    time.sleep(0.5)            # 1秒間スリープ
    GPIO.output(LED1,0)        # LED2のBCM番号に1を出力（LED点灯）
    time.sleep(0.5)            # 1秒間スリープ
    kaisu = kaisu-1            # kaisuの値を1減らす
GPIO.cleanup()                 # クリーンアップ（初期設定に戻す）