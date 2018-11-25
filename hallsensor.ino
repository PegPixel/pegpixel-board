#include <Wire.h>
const int SENSOR_ADR=0x42;
const int HALL_REGISTER_ADR=0x1F;
const int HALL_REGISTER_SIZE_BYTE=4;

void setup() {
  Wire.begin();       
  Serial.begin(115200); 
}

void loop() {
  Wire.beginTransmission(SENSOR_ADR);
  Wire.write(HALL_REGISTER_ADR);
  Wire.endTransmission(false);
  Wire.requestFrom(SENSOR_ADR, HALL_REGISTER_SIZE_BYTE, true);  
  
  while (Wire.available()) { 
    int i = Wire.read();
    Serial.print(i);
  }
  Serial.println(" fertig ");
  delay(500);
}
