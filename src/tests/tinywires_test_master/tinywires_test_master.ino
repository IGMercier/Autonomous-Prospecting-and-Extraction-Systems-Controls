
//
#include <Wire.h>

// BREADBOARD SETUP:
// Arduino Uno R3 (D18/SDA) = I2C SDA 
//     connect to SDA on slave with external pull-up (~4.7K)
// Arduino Uno R3 (D19/SCL) = I2C SCL 
//     connect to SCL on slave with external pull-up (~4.7K)
// Arduino Uno R3 (D2)
//     connect to !RST on slave
//     Can alternatively connect !RST on slave to the Ardiuno "!RESET" pin

#define I2C_SLAVE_ADDR  0x26            // i2c slave address (38, 0x26)

#define SLAVE_RESET_PIN 2

unsigned long pulse = 0;



void setup()
{
  // set pin modes 
  pinMode(SLAVE_RESET_PIN,OUTPUT);
  pinMode( 9 , OUTPUT );
  pinMode( 11 , OUTPUT );

  // init the serial port
  Serial.begin(115200);

  // init the Wire object (for I2C)
  Wire.begin(); 
  
  // reset the slave
  digitalWrite(SLAVE_RESET_PIN, LOW);
  delay(10);
  digitalWrite(SLAVE_RESET_PIN, HIGH);
  
  // wait for slave to finish any init sequence
  delay(2000);
  
}

void loop() {
  digitalWrite(9, LOW);
  digitalWrite(11, LOW);
  
  for (int i = 0; i < 10; i++) {
    digitalWrite(9, HIGH);
    delay(50);
    digitalWrite(11, HIGH);
    delay(50);
    digitalWrite(9, LOW);
    delay(50);
    digitalWrite(11, LOW);
    delay(50);
  }
  int count = Wire.requestFrom(I2C_SLAVE_ADDR, 4);
  pulse = 0;
  while(Wire.available()) {
    pulse = pulse << 4;
    pulse |= Wire.read();
  }
  Serial.print("pulse count = ");
  Serial.println(pulse);
  delay(2000);  
  
}
