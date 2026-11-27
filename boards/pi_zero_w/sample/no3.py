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