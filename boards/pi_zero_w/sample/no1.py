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