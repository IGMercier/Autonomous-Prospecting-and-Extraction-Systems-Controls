import RPi.GPIO as gpio
import time
gpio.setmode(gpio.BOARD)
pdir = 37
pspd = 40 
gpio.setup(pdir, gpio.OUT)
gpio.setup(pspd, gpio.OUT)
p = gpio.PWM(pspd, 9600)
gpio.output(pdir, gpio.HIGH)
p.start(100)
time.sleep(3)
gpio.output(pdir, gpio.LOW)
time.sleep(3)
p.stop()
gpio.cleanup()
