#include <Wire.h>
 
byte RxByte;
 
void I2C_RxHandler(int numBytes)
{
  byte RxByte;
  while(Wire.available()) {  // Read Any Received Data
    RxByte = Wire.read();
    Serial.println(RxByte);
  }
}

void I2C_TxHandler(void)
{
  Wire.write(192);
}

 
void setup() {
  Serial.begin(115200);
  Wire.begin(0x55); // Initialize I2C (Slave Mode: address=0x55 )
  Wire.onReceive(I2C_RxHandler);
  Wire.onRequest(I2C_TxHandler);
}
 
void loop() {
  // Nothing To Be Done Here
}
