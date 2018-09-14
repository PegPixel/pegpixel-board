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

struct ParsedPixel {
  int x;
  int y;
  boolean selected;
};

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
    ParsedPixel parsedPixel = parseJson(newMessage);
    drawPixel(parsedPixel);
  }
}


const size_t bufferSize = JSON_OBJECT_SIZE(3) + 20;
StaticJsonBuffer<bufferSize> jsonBuffer;

ParsedPixel parseJson(String newMessage){
  jsonBuffer.clear();
  JsonObject& root = jsonBuffer.parseObject(newMessage.c_str());

  if (!root.success()) {
    Serial.print("parseObject() failed - message: ");
    Serial.println(newMessage);
  } else {
    Serial.print("parsing: ");
    root.printTo(Serial);
    Serial.print("\n");
  }

  struct ParsedPixel parsedPixel;
  parsedPixel.x = root["x"];
  parsedPixel.y = root["y"];
  parsedPixel.selected = root["s"] == "t";
  
  return parsedPixel;
}

void drawPixel(ParsedPixel parsedPixel){
  int pixelIndex = getPixelIndex(parsedPixel);
  if(parsedPixel.selected){
    pixels.setPixelColor(pixelIndex, pixels.Color(51, 102, 255));
   
  } else {
    pixels.setPixelColor(pixelIndex, pixels.Color(0, 0, 0));  
    
  }
  pixels.show();
}

int getPixelIndex(ParsedPixel parsedPixel) {
  
  // convert 1-based indices to 0-based
  int x = --parsedPixel.x;
  int y = --parsedPixel.y;
  
  if(x % 2 == 0) {
    int pixelIndex = x * 4 + y;
    Serial.println("line 0 or 2");
    
    Serial.print("pixelindex: ");
    Serial.println(pixelIndex);
    return pixelIndex;
  }
  int pixelIndex = x * 4 + (4 - (y + 1));
  Serial.print("pixelindex:");
  Serial.println(pixelIndex);
  return pixelIndex;

}
