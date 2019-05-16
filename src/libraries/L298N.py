import RPI.GPIO as GPIO
from time import sleep

p = None

def setup(pinA, pinB):
    global p

    en = 25
    GPIO.setmode(GPIO.BCM)
    GPIO.setup(pinA, GPIO.OUT)
    GPIO.setup(pinB, GPIO.OUT)
    GPIO.setup(en, GPIO.OUT)
    GPIO.output(pinA, GPIO.LOW)
    GPIO.output(pinB, GPIO.LOW)

    p = GPIO.PWM(en, 1000)
    p.start(25)
    return


def stop(pinA, pinB):
    GPIO.output(pinA, GPIO.LOW)
    GPIO.output(pinB, GPIO.LOW)
    return

def forward(pinA, pinB):
    GPIO.output(pinA, GPIO.HIGH)
    GPIO.output(pinB, GPIO.LOW)
    return

def backward(pinA, pinB):
    GPIO.output(pinA, GPIO.LOW)
    GPIO.output(pinB, GPIO.HIGH)
    return

def changeDuty(dc):
    global p
    p.ChangeDutyCycle(dc)
    return
