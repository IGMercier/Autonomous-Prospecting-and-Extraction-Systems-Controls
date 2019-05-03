import RPi.GPIO as gpio
import time
gpio.setmode(gpio.BOARD)
gpio.setup(29, gpio.OUT)
gpio.setup(31, gpio.OUT)
gpio.setup(32, gpio.OUT)
gpio.setup(33, gpio.OUT)
gpio.output(29, gpio.HIGH)
gpio.output(31, gpio.HIGH)
gpio.output(32, gpio.HIGH)
gpio.output(33, gpio.HIGH)
time.sleep(10)
gpio.cleanup()
