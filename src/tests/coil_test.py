import RPi.GPIO as gpio
import time, sys
gpio.setmode(gpio.BOARD)
pdir = 37
pspd = 40 
gpio.setup(pdir, gpio.OUT)
gpio.setup(pspd, gpio.OUT)
p = gpio.PWM(pspd, 9600)
gpio.output(pdir, gpio.HIGH)
p.start(100)
if len(sys.argv) > 1:
    time.sleep(int(sys.argv[1]))
else:
    time.sleep(3)
gpio.output(pdir, gpio.LOW)
time.sleep(3)
p.stop()
gpio.cleanup()
