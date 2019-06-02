#include <TinyWireS.h>
//#include <assert.h>

#define PIN_A 4
#define PIN_B 3
#define ADDR  0x20

volatile unsigned long pulse;
unsigned long i = 0;

void setup() {
  //assert(sizeof(pulse) == 2);
  TinyWireS.begin(ADDR);
  TinyWireS.onRequest(sendPulses);
  TinyWireS.onReceive(reset);
  
  pinMode(PIN_A, INPUT);
  pinMode(PIN_B, INPUT);
  attachInterrupt(PIN_A, pin_a_handler, CHANGE);
  attachInterrupt(PIN_B, pin_b_handler, CHANGE);
}

void reset() {
  pulse = 0;
}

void sendPulses() {
    TinyWireS.send(i);
    i++;
}

void pin_a_handler() {
  if (digitalRead(PIN_A) == HIGH) {
    if (digitalRead(PIN_B) == LOW) {
      pulse++; // CW 
    } else {
      pulse--; // CCW
    }
  } else {
    if (digitalRead(PIN_B) == HIGH) {
      pulse++; // CW
    } else {
      pulse--; // CCW
    }
  }
}

void pin_b_handler() {
  if (digitalRead(PIN_B) == HIGH) {
    if (digitalRead(PIN_A) == HIGH) {
      pulse++; // CW 
    } else {
      pulse--; // CCW
    }
  } else {
    if (digitalRead(PIN_A) == LOW) {
      pulse++; // CW
    } else {
      pulse--; // CCW
    }
  }
}

void loop() {
  TinyWireS_stop_check();
}
