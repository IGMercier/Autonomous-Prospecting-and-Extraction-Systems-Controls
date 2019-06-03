import RPi.GPIO as gpio
import time
gpio.setmode(gpio.BOARD)
tim = 38
gpio.setup(tim, gpio.OUT)
gpio.output(tim, gpio.HIGH)
time.sleep(5)
gpio.output(tim, gpio.LOW)
time.sleep(5)
gpio.cleanup()
