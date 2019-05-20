import RPI.GPIO as GPIO
from time import sleep

class L298N:
    def __init__(self, pinA, pinB):
        self.pinA = pinA
        self.pinB = pinB

        en = 25
        GPIO.setmode(GPIO.BCM)
        GPIO.setup(self.pinA, GPIO.OUT)
        GPIO.setup(self.pinB, GPIO.OUT)
        GPIO.setup(en, GPIO.OUT)
        GPIO.output(self.pinA, GPIO.LOW)
        GPIO.output(self.pinB, GPIO.LOW)

        self.p = GPIO.PWM(en, 1000)
        self.p.start(25)

        return

    def stop():
        GPIO.output(self.pinA, GPIO.LOW)
        GPIO.output(self.pinB, GPIO.LOW)
        return

    def forward():
        GPIO.output(self.pinA, GPIO.HIGH)
        GPIO.output(self.pinB, GPIO.LOW)
        return

    def backward():
        GPIO.output(self.pinA, GPIO.LOW)
        GPIO.output(self.pinB, GPIO.HIGH)
        return

    def changeDuty(dc=10):
        self.p.ChangeDutyCycle(dc)
        return

    def clean():
        GPIO.cleanup([self.pinA, self.pinB]);
        return
