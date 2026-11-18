import serial
import RPi.GPIO as GPIO
import traceback
import time

SW1 = 24
MESSEAGE = "Trigger SW Detected!"

GPIO.setmode(GPIO.BCM)
GPIO.setup(SW1,GPIO.IN,pull_up_down=GPIO.PUD_DOWN)

class UART:
    def __init__(self):
        try:
            self.uartport = serial.Serial('/dev/ttyS0', 115200, timeout=10)
        except serial.SerialException:
            print(traceback.format_exc())
            exit(1)
        
        self.uartport.reset_output_buffer()
        time.sleep(1)
    def send_serial(self, message):
        try:
            print(MESSEAGE)
            data = (message + "\n").encode("utf-8")
            self.uartport.write(data)            
        except serial.SerialException:
            print(traceback.format_exc())
def main():
    uart = UART()
    try:
        while True:
            time.sleep(0.5)
            if ( GPIO.input(SW1) == GPIO.HIGH ):
                uart.send_serial(MESSEAGE)
                while GPIO.input(SW1) == GPIO.HIGH:
                    time.sleep(0.1)
    except KeyboardInterrupt:
        print("\n終了")
    finally:
        GPIO.cleanup()

if __name__ == '__main__':
    main()