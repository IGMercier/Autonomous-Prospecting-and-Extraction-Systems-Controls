import RPi.GPIO as gpio
import time
gpio.setmode(gpio.BOARD)
stp = 36
sdir = 35
gpio.setup(stp, gpio.OUT)
gpio.setup(sdir, gpio.OUT)
p = gpio.PWM(stp, 1000)
gpio.output(sdir, gpio.HIGH)
p.start(50)
time.sleep(2)
p.stop()
gpio.cleanup()
