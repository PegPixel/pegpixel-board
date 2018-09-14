#include <SoftwareSerial.h>
#include <ArduinoJson.h>
#include <Adafruit_NeoPixel.h>
#ifdef __AVR__
  #include <avr/power.h>
#endif

// Which pin on the Arduino is connected to the NeoPixels?
// On a Trinket or Gemma we suggest changing this to 1
#define PIN            6

// How many NeoPixels are attached to the Arduino?
#define NUMPIXELS      7

// When we setup the NeoPixel library, we tell it how many pixels, and which pin to use to send signals.
// Note that for older NeoPixel strips you might need to change the third parameter--see the strandtest
// example for more information on possible values.
Adafruit_NeoPixel pixels = Adafruit_NeoPixel(NUMPIXELS, PIN, NEO_GRB + NEO_KHZ800);

int delayval = 500; // delay for half a second

#define rxPin 10
#define txPin 11

SoftwareSerial mySerial(rxPin, txPin);

void setup() {
    // This is for Trinket 5V 16MHz, you can remove these three lines if you are not using a Trinket
#if defined (__AVR_ATtiny85__)
  if (F_CPU == 16000000) clock_prescale_set(clock_div_1);
#endif
  // End of trinket special code

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
    boolean newMessage = readCharArrayFromBluetooth();
    if(newMessage == true){
      JsonObject& json = parseJson();
      drawPixel(json);
    }
  }
}

const int readStringSize = 64;
char readString[readStringSize] = "";

const int minimumMessageSize = 21; 
boolean readCharArrayFromBluetooth() {
  int numberOfReadBytes = mySerial.readBytesUntil('\n', readString, readStringSize);
  return numberOfReadBytes >= minimumMessageSize;
}

JsonObject& parseJson(){
  const size_t bufferSize = JSON_OBJECT_SIZE(1) + 10;
  DynamicJsonBuffer jsonBuffer(bufferSize);
  
  JsonObject& root = jsonBuffer.parseObject(readString);

  if (!root.success()) {
    Serial.print("parseObject() failed - ");
  } else {
    Serial.print("parsing: ");
    root.printTo(Serial);
    Serial.print("\n");
  }
  readString[readStringSize] = "";
  return root;
}

void drawPixel(JsonObject& json){
  int y = json["y"];
  y--; // The incoming array starts at 1. We need 0

  if(json["s"] == "t"){
    pixels.setPixelColor(y, pixels.Color(51, 102, 255));
   
  } else {
    pixels.setPixelColor(y, pixels.Color(0, 0, 0));  
    
  }
  
  pixels.show();
}
