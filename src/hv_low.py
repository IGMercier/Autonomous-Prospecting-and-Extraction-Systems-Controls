import RPi.GPIO as gpio
import time
gpio.setmode(gpio.BCM)
gpio.setup(5, gpio.OUT)
gpio.setup(6, gpio.OUT)
gpio.setup(12, gpio.OUT)
gpio.setup(13, gpio.OUT)
gpio.output(5, gpio.LOW)
gpio.output(6, gpio.LOW)
gpio.output(12, gpio.LOW)
gpio.output(12, gpio.LOW)
time.sleep(10)
gpio.cleanup()
