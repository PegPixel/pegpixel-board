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

boolean readFromBluetooth() {
  if(mySerial.available()){
    char readChar = mySerial.read();
    if(readChar == '[' || readChar == ']' || readChar == ',') {
      return false;
    }
    Serial.write(readChar);
    receivedMessage.concat(readChar);
    if(readChar == '}'){
      Serial.println("new message!");
      return true;
    }
  }
  return false;
}


void parseJson(){
  const size_t bufferSize = JSON_OBJECT_SIZE(1) + 10;
  DynamicJsonBuffer jsonBuffer(bufferSize);
  
  JsonObject& root = jsonBuffer.parseObject(receivedMessage);

  if (!root.success()) {
    Serial.println("parseObject() failed");
    return;
  } else {
    Serial.println("message parsed");    
    root.printTo(Serial);
  }
}
