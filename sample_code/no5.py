import RPi.GPIO as GPIO        # RPi.GPIOのライブラリ名をGPIOとして使用
import threading
import time
import sys

LED1 = 17                      # LED1のBCM番号の指定
SW1 = 24                       # 白のタクトSW(SW1)
GPIO.setmode(GPIO.BCM)         # BCM番号をナンバリングで使用
GPIO.setwarnings(False)        # 以前の状態の警告文を抑制
GPIO.setup(LED1,GPIO.OUT)      # LED1のBCM番号を出力に設定
GPIO.setup(SW1,GPIO.IN,pull_up_down=GPIO.PUD_DOWN)

def sw_check(stop_event, led_event):
    last_state = GPIO.LOW
    while not stop_event.is_set():
        current_state = GPIO.input(SW1)
        if GPIO.HIGH == current_state and GPIO.LOW == last_state :
            print("ボタン押下検知")
            led_event.set()
        last_state = current_state
        time.sleep(0.1)
        

def led_check(stop_event, led_event):
    led_state = False
    while not stop_event.is_set():
        led_event.wait(timeout=0.1)
        if led_event.is_set():
            led_state = not led_state
            GPIO.output(LED1,led_state)
            if True == led_state:
                print("LED点灯")
            else:
                print("LED消灯")
            led_event.clear()
        
def main():
    stop_event = threading.Event()
    led_event  = threading.Event()
    thread1 = threading.Thread(target=sw_check, args=(stop_event, led_event))
    thread2 = threading.Thread(target=led_check, args=(stop_event, led_event))
    
    try:
        print("Ctrl+C:プログラム終了\n")
        thread1.start()
        thread2.start()
        while True:
            time.sleep(1)
    except KeyboardInterrupt:
        print("\n動作を終了します。")
        stop_event.set()
        thread1.join()
        thread2.join()
        GPIO.cleanup()
        sys.exit()

if __name__ == '__main__':
    main()