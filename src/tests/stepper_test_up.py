import RPi.GPIO as gpio
import time, sys
gpio.setmode(gpio.BOARD)
stp = 36
sdir = 35
gpio.setup(stp, gpio.OUT)
gpio.setup(sdir, gpio.OUT)
p = gpio.PWM(stp, 1000)
gpio.output(sdir, gpio.LOW)
p.start(50)
if len(sys.argv) > 1:
    time.sleep(float(sys.argv[1]))
else:
    time.sleep(.4)
p.stop()
gpio.cleanup()
