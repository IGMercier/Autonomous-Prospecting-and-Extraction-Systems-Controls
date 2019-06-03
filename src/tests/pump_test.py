import RPi.GPIO as gpio
import time
gpio.setmode(gpio.BOARD)
pdir = 12
pspd = 15
gpio.setup(pdir, gpio.OUT)#en
gpio.setup(pspd, gpio.OUT)#spd
p = gpio.PWM(pspd, 1000)
gpio.output(pdir, gpio.HIGH)
p.start(100)
time.sleep(2)
p.stop()
print("done\n")
gpio.cleanup()
