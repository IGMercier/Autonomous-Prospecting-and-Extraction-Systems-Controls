import RPi.GPIO as gpio
import time
gpio.setmode(gpio.BOARD)
pdir = 35
pspd = 36
gpio.setup(pdir, gpio.OUT)#en
gpio.setup(pspd, gpio.OUT)#spd
p = gpio.PWM(pspd, 1000)
gpio.output(pdir, gpio.LOW)
p.start(50)
time.sleep(1)
p.stop()
print("done\n")
gpio.cleanup()
