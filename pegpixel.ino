#include <SoftwareSerial.h>
#include <ArduinoJson.h>
#include <Adafruit_NeoPixel.h>
#ifdef __AVR__
  #include <avr/power.h>
#endif

#define NEOPIXEL_PIN            6

#define NUMPIXELS      16

Adafruit_NeoPixel pixels = Adafruit_NeoPixel(NUMPIXELS, NEOPIXEL_PIN, NEO_GRBW + NEO_KHZ800);

#define rxPin 10
#define txPin 11

SoftwareSerial mySerial(rxPin, txPin);

struct ParsedPixel {
  int x;
  int y;
  boolean selected;
  int r;
  int g;
  int b;
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
  pixels.setBrightness(255);
  pixels.show();
}


void loop() { 
  if(mySerial.available()){
    String newMessage = mySerial.readStringUntil('\n');
    ParsedPixel parsedPixel = parseJson(newMessage);
    drawPixel(parsedPixel);
  }
}


  #define INPUT_SIZE 32
  
ParsedPixel parseCsv(String newMessage) {
  
  Serial.print("Parsing pixel: ");
  Serial.println(newMessage);
  
 
  char * input = new char [newMessage.length()+1];
  strcpy (input, newMessage.c_str());
  
  char delimiter[] = ",";
  char* currentElement = strtok(input, delimiter);

  struct ParsedPixel parsedPixel;
  
  parsedPixel.x = atoi(currentElement);
  currentElement = strtok(NULL, delimiter);
  parsedPixel.y = atoi(currentElement); 
  currentElement = strtok(NULL, delimiter);
  parsedPixel.selected = *currentElement == 't';  
  currentElement = strtok(NULL, delimiter);
  parsedPixel.r = atoi(currentElement); 
  currentElement = strtok(NULL, delimiter);
  parsedPixel.g = atoi(currentElement); 
  currentElement = strtok(NULL, delimiter);
  parsedPixel.b = atoi(currentElement); 

  Serial.print("Parsed pixel: ");
  Serial.println();
  
  return parsedPixel;
  
}

const size_t bufferSize = JSON_OBJECT_SIZE(6) + 30;
StaticJsonBuffer<bufferSize> jsonBuffer;

ParsedPixel parseJson(String newMessage){
  jsonBuffer.clear();
  JsonObject& root = jsonBuffer.parseObject(newMessage.c_str());

  if (!root.success()) {
    Serial.print("parseObject() failed - message: ");
    Serial.println(newMessage);
  }

  struct ParsedPixel parsedPixel;
  parsedPixel.x = root["x"];
  parsedPixel.y = root["y"];
  parsedPixel.selected = root["s"] == "t";
  parsedPixel.r = root["r"];
  parsedPixel.g = root["g"];
  parsedPixel.b = root["b"];
  
  return parsedPixel;
}

void drawPixel(ParsedPixel parsedPixel){
  int pixelIndex = getPixelIndex(parsedPixel);
  if(parsedPixel.selected){
    pixels.setPixelColor(pixelIndex, pixels.Color(parsedPixel.r, parsedPixel.g, parsedPixel.b));
   
  } else {
    pixels.setPixelColor(pixelIndex, pixels.Color(0, 0, 0));  
    
  }
  pixels.show();
}

int getPixelIndex(ParsedPixel parsedPixel) {
  
  // convert 1-based indices to 0-based
  int x = --parsedPixel.x;
  int y = --parsedPixel.y;
  int xOffset = x * 4;
  
  if(x % 2 == 0) {
    return xOffset + y;
  }
  return xOffset + 3 - y;
}
