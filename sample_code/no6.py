import RPi.GPIO as GPIO        # RPi.GPIOのライブラリ名をGPIOとして使用
import threading
import queue
import time
import sys

LED1 = 17                                                   # LED1のBCM番号の指定
SW1 = 24                                                    # 白のタクトSW(SW1)
GPIO.setmode(GPIO.BCM)                                      # BCM番号をナンバリングで使用
GPIO.setwarnings(False)                                     # 以前の状態の警告文を抑制
GPIO.setup(LED1,GPIO.OUT)                                   # LED1のBCM番号を出力に設定
GPIO.setup(SW1,GPIO.IN,pull_up_down=GPIO.PUD_DOWN)          # SW1の入力をプルダウン設定

interval = [0.5, 0.2, 1.0]

def sw_check(stop_event, cmd_queue):
    raw = 0
    last_state = GPIO.LOW
    while not stop_event.is_set():
        current_state = GPIO.input(SW1)
        if GPIO.HIGH == current_state and GPIO.LOW == last_state :
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
            GPIO.output(LED1,led_state)
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
        stop_event.set()
        thread1.join()                                      #スレッド停止(スイッチ監視)
        thread2.join()                                      #スレッド停止(LED監視)
        GPIO.cleanup()
        sys.exit()

if __name__ == '__main__':
    main()