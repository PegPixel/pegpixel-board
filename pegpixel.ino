#include <SoftwareSerial.h>

#include <ArduinoJson.h>

#include <Adafruit_NeoPixel.h>
#ifdef __AVR__
  #include <avr/power.h>
#endif

#define NEOPIXEL_PIN            6

#define COLUMNS 4
#define ROWS 4
#define NUM_PIXELS (COLUMNS * ROWS)



Adafruit_NeoPixel pixels = Adafruit_NeoPixel(NUM_PIXELS, NEOPIXEL_PIN, NEO_GRB + NEO_KHZ800);

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
  int baudRate = 9600;
  Serial.setTimeout(10);
  Serial.begin(baudRate);
  
  mySerial.begin(baudRate);
  
  Serial.write("Serial is online\n");
  pixels.begin();
  pixels.show();  
}

String incomingMessages[NUM_PIXELS];
int currentMessageIndex = 0;

void loop() {
  if (mySerial.overflow()) {
   Serial.println("SoftwareSerial overflow!"); 
  }
 drawBluetoothUpdates();
}


void drawBluetoothUpdates(){
  if(mySerial.available()){
    String newMessage = mySerial.readStringUntil('\n');
    ParsedPixel parsedPixel = parseJson(newMessage);
    drawPixel(parsedPixel);
  }
}

  struct ParsedPixel parsedPixel;
ParsedPixel parseMessage(String newMessage) {
  Serial.print("parsing : " );
  Serial.println(newMessage);

  long beforeSimple = millis();

  int x = newMessage.substring(0 ,1).toInt();
  int y = newMessage.substring(1, 2).toInt();
  String selected = newMessage.substring(2, 3);
  int r = newMessage.substring(3, 6).toInt();
  int g = newMessage.substring(6, 9).toInt();
  int b = newMessage.substring(9, 12).toInt();

  parsedPixel.x = x;
  parsedPixel.y = y;
  parsedPixel.selected = selected == "t";
  parsedPixel.r = r;
  parsedPixel.g = g;
  parsedPixel.b = b;
  
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
  parsedPixel.x = ((int)root["x"]);
  parsedPixel.y = ((int)root["y"]);
  parsedPixel.selected = root["s"] == "t";
  parsedPixel.r = root["r"];
  parsedPixel.g = root["g"];
  parsedPixel.b = root["b"];
  
  return parsedPixel;
}


void drawPixel(ParsedPixel parsedPixel){
  int pixelIndex = getPixelIndex(parsedPixel.x, parsedPixel.y);
  if(parsedPixel.selected){
    pixels.setPixelColor(pixelIndex, pixels.Color(parsedPixel.r, parsedPixel.g, parsedPixel.b));
  } else {
    pixels.setPixelColor(pixelIndex, pixels.Color(0, 0, 0));  
  }
  pixels.show();
}


void printToSerial(ParsedPixel parsedPixel){
  Serial.print("pixel - x: ");
  Serial.print(parsedPixel.x);
  Serial.print(" - y: ");
  Serial.print(parsedPixel.y);
  Serial.print(" - selected: ");
  Serial.print(parsedPixel.selected);
  Serial.print(" - r: ");
  Serial.print(parsedPixel.r);
  Serial.print(" - g: ");
  Serial.print(parsedPixel.g);
  Serial.print(" - b: ");
  Serial.println(parsedPixel.b);
}

int getPixelIndex(int column, int row) {
  
  int rowOffset = row * COLUMNS;
  
  if(row % 2 == 0) {
    return rowOffset + column;
  }
  return rowOffset + COLUMNS - 1 - column;
}
