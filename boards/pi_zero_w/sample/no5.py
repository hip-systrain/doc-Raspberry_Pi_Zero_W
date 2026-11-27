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