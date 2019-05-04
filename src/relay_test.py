import RPi.GPIO as gpio
import time
gpio.setmode(gpio.BCM)
gpio.setup(23, gpio.OUT)
gpio.setup(24, gpio.OUT)
gpio.output(23, gpio.HIGH)
gpio.output(24, gpio.HIGH)
time.sleep(300)   
gpio.cleanup()
