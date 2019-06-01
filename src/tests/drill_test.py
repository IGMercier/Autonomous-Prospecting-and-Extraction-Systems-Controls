import RPi.GPIO as gpio
import time
gpio.setmode(gpio.BOARD)
gpio.setup(7, gpio.OUT)#en
gpio.setup(11, gpio.OUT)#spd
p = gpio.PWM(7, 6000)
gpio.output(11, gpio.HIGH)
'''
for i in range(11):
    p.start(10*i)
    print(10*i)
    time.sleep(2)
'''
p.start(50)
time.sleep(25)
gpio.output(11, gpio.LOW)
p.stop()
print("done\n")
gpio.cleanup()
