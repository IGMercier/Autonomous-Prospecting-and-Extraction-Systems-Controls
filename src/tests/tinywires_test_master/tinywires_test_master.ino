
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

char c_buf[64]; // for creating messages

void setup()
{
  // set pin modes 
  pinMode(SLAVE_RESET_PIN,OUTPUT);

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

void loop()
{
  uint8_t i;
  uint8_t req_rtn;       // num bytes returned by requestFrom() call
  uint8_t out[16];  // data written from master
  uint8_t in[16];   // data read back from slave

  // generate, save, and send N random byte values
  Wire.beginTransmission(I2C_SLAVE_ADDR);
  for (i = 0; i < 10; i++)
    Wire.write(out[i] = i);
  Wire.endTransmission();
  
  //delay (10);  // optional delay if required by slave (like sample ADC)

  // read N bytes from slave
  req_rtn = Wire.requestFrom(I2C_SLAVE_ADDR, 10);      // Request N bytes from slave
  for (i = 0; i < req_rtn; i++) {
    in[i] = Wire.read();
    Serial.print(in[i]);
    Serial.print("\n");
  }

  // delay 1 second so user can watch results
  delay(1000);
}
