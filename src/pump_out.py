import RPi.GPIO as gpio
import time
gpio.setmode(gpio.BOARD)
pdir = 15
pspd = 12 
gpio.setup(pdir, gpio.OUT)
gpio.setup(pspd, gpio.OUT)
p = gpio.PWM(pspd, 9600)
gpio.output(pdir, gpio.HIGH)
p.start(100)
time.sleep(60)
p.stop()
gpio.cleanup()
