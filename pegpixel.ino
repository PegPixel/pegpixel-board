#include <SoftwareSerial.h>
#include <ArduinoJson.h>
#include <Adafruit_NeoPixel.h>
#ifdef __AVR__
  #include <avr/power.h>
#endif

#define NEOPIXEL_PIN            6

#define NUMPIXELS      16

Adafruit_NeoPixel pixels = Adafruit_NeoPixel(NUMPIXELS, NEOPIXEL_PIN, NEO_GRB + NEO_KHZ800);

#define rxPin 10
#define txPin 11

SoftwareSerial mySerial(rxPin, txPin);

void setup() {
  pinMode(rxPin, INPUT);
  pinMode(txPin, OUTPUT);
  int baudRate = 19200;
  Serial.setTimeout(10);
  Serial.begin(baudRate);
  
  mySerial.begin(baudRate);
  
  Serial.write("Serial is online\n");
  pixels.begin();
  pixels.show();
}


void loop() { 
  if(mySerial.available()){
    String newMessage = mySerial.readStringUntil('\n');
    JsonObject& json = parseJson(newMessage);
    drawPixel(json);
  }
}


JsonObject& parseJson(String newMessage){
  const size_t bufferSize = JSON_OBJECT_SIZE(1) + 10;
  DynamicJsonBuffer jsonBuffer(bufferSize);
  
  JsonObject& root = jsonBuffer.parseObject(newMessage.c_str());

  if (!root.success()) {
    Serial.print("parseObject() failed - ");
  } else {
    Serial.print("parsing: ");
    root.printTo(Serial);
    Serial.print("\n");
  }
  return root;
}

void drawPixel(JsonObject& json){
  int pixelIndex = getPixelIndex(json);
  if(json["s"] == "t"){
    pixels.setPixelColor(pixelIndex, pixels.Color(51, 102, 255));
   
  } else {
    pixels.setPixelColor(pixelIndex, pixels.Color(0, 0, 0));  
    
  }
  pixels.show();
}

int getPixelIndex(JsonObject& json) {
  int x = json["x"];
  int y = json["y"];
  
  // convert 1-based indices to 0-based
  x--;y--;
  
  if(x % 2 == 0) {
    int pixelIndex = x * 4 + y;
    Serial.println("line 0 or 2");
    
    Serial.print("pixelindex: ");
    Serial.println(pixelIndex);
    return pixelIndex;
  }
  int pixelIndex = x * 4 + (4 - (y+1));
  Serial.print("pixelindex:");
  Serial.println(pixelIndex);
  return pixelIndex;

}
