#include <SoftwareSerial.h>
#include <ArduinoJson.h>

#define rxPin 10
#define txPin 11

SoftwareSerial mySerial =  SoftwareSerial(rxPin, txPin);

void setup() {
  pinMode(rxPin, INPUT);
  pinMode(txPin, OUTPUT);
  int baudRate = 19200;
  
  Serial.begin(baudRate);
  
  mySerial.begin(baudRate);
  
  Serial.write("Serial is online\n");
}

void loop() {
  boolean newMessage = readFromBluetooth();
  if(newMessage == true){
    parseJson();
  }
}

String receivedMessage;
boolean inObject = false;

boolean readFromBluetooth() {
  if(mySerial.available()){
    char readChar = mySerial.read();
    // Ignore start and end of array
    if(readChar == '[' || readChar == ']') {
      return false;
    }
    // ignore comma separating objects
    if(readChar == ',' && !inObject) {
      return false;
    }
    //Serial.write(readChar);
    receivedMessage.concat(readChar);
    if(readChar == '{'){
      inObject = true;
    }
    if(readChar == '}'){
      Serial.println("new message!");
      inObject = false;
      return true;
    }
  }
  return false;
}


JsonObject& parseJson(){
  const size_t bufferSize = JSON_OBJECT_SIZE(1) + 10;
  DynamicJsonBuffer jsonBuffer(bufferSize);
  
  JsonObject& root = jsonBuffer.parseObject(receivedMessage);

  if (!root.success()) {
    Serial.println("parseObject() failed");
  } else {
    root.printTo(Serial);
  }
  return root;
}
