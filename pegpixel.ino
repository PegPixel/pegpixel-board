#include <SoftwareSerial.h>

#include <ArduinoJson.h>
#include <Adafruit_NeoPixel.h>
#ifdef __AVR__
  #include <avr/power.h>
#endif

#define NEOPIXEL_PIN            6

#define COLUMNS 7
#define ROWS 5
#define NUM_PIXELS (COLUMNS * ROWS)


Adafruit_NeoPixel pixels = Adafruit_NeoPixel(NUM_PIXELS, NEOPIXEL_PIN, NEO_GRBW + NEO_KHZ800);

#define rxPin 10 // This is where the TX Pin from The BT Device is connected
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
  pixels.show();
}


void loop() {
  /*
  if(mySerial.available()){
    String newMessage = mySerial.readStringUntil('\n');
    ParsedPixel parsedPixel = parseJson(newMessage);
    drawPixel(parsedPixel);
  }
  */
  pulseRed(6, true);
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
  int column = --parsedPixel.x;
  int row = --parsedPixel.y;
  int rowOffset = row * COLUMNS;
  
  if(row % 2 == 0) {
    return rowOffset + column;
  }
  return rowOffset + COLUMNS - 1 - column;
}

#define SINE_TABLE_SIZE 256

static const uint8_t PROGMEM _sineTable[SINE_TABLE_SIZE] = {
  128,131,134,137,140,143,146,149,152,155,158,162,165,167,170,173,
  176,179,182,185,188,190,193,196,198,201,203,206,208,211,213,215,
  218,220,222,224,226,228,230,232,234,235,237,238,240,241,243,244,
  245,246,248,249,250,250,251,252,253,253,254,254,254,255,255,255,
  255,255,255,255,254,254,254,253,253,252,251,250,250,249,248,246,
  245,244,243,241,240,238,237,235,234,232,230,228,226,224,222,220,
  218,215,213,211,208,206,203,201,198,196,193,190,188,185,182,179,
  176,173,170,167,165,162,158,155,152,149,146,143,140,137,134,131,
  128,124,121,118,115,112,109,106,103,100, 97, 93, 90, 88, 85, 82,
   79, 76, 73, 70, 67, 65, 62, 59, 57, 54, 52, 49, 47, 44, 42, 40,
   37, 35, 33, 31, 29, 27, 25, 23, 21, 20, 18, 17, 15, 14, 12, 11,
   10,  9,  7,  6,  5,  5,  4,  3,  2,  2,  1,  1,  1,  0,  0,  0,
    0,  0,  0,  0,  1,  1,  1,  2,  2,  3,  4,  5,  5,  6,  7,  9,
   10, 11, 12, 14, 15, 17, 18, 20, 21, 23, 25, 27, 29, 31, 33, 35,
   37, 40, 42, 44, 47, 49, 52, 54, 57, 59, 62, 65, 67, 70, 73, 76,
   79, 82, 85, 88, 90, 93, 97,100,103,106,109,112,115,118,121,124};

void pulseRed(int pixelIndex, boolean infinitely){

  while(infinitely){
    for (int i = 0; i < SINE_TABLE_SIZE; i++){
      uint8_t red = pgm_read_byte(&_sineTable[i]);
      //setAllPixels(red, 0, 0);
      pixels.setPixelColor(pixelIndex, red, 0, 0);
      pixels.show();
      delay(10);
    }
  }
}
 
void setAllPixels(int red, int green, int blue) {
  for (int i = 0; i < NUM_PIXELS; i++){
    pixels.setPixelColor(i, red, green, blue); 
  }
}
