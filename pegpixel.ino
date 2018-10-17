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



#define BALL_UP 0
#define BALL_RIGHT 1
#define BALL_LEFT 2
#define BALL_DOWN 3

struct Ball {
  int x;
  int y;
  int direction;
};

struct Ball ball;

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
  randomSeed(analogRead(A0));
  ball = {random(0, COLUMNS), random(0, ROWS), random(0, BALL_DOWN)};


  int beforeJson = millis();
  parseJson("{\"x\":1,\"y\":1,\"s\":\"t\",\"r\":255,\"g\":0,\"b\":\"0\"}");
  int afterJson = millis();
  
  int beforeSimple = millis();
  parseMessage("12t255000000");
  int afterSimple = millis();

  
  Serial.print("json: ");
  Serial.println(afterJson - beforeJson);
  Serial.print("simple: ");
  Serial.println(afterSimple - beforeSimple);
  
}

String incomingMessages[NUM_PIXELS];
int currentMessageIndex = 0;

void loop() {
  if (mySerial.overflow()) {
   Serial.println("SoftwareSerial overflow!"); 
  }
 //drawBallUpdates();
 drawBluetoothUpdates();
 // pulseRed(0, false);

}


void drawBluetoothUpdates(){
  if(mySerial.available()){
    String newMessage = mySerial.readStringUntil('\n');
    Serial.println(newMessage);
    ParsedPixel pixel = parseMessage(newMessage);
    drawPixel(pixel);
 
  }
}

ParsedPixel parseMessage(String newMessage) {
  Serial.print("parsing : " );
  Serial.println(newMessage);

  char* message = newMessage.c_str();

  

  int x = newMessage.substring(0, 1).toInt();
  int y = newMessage.substring(1, 2).toInt();
  String selected = newMessage.substring(2, 3);
  int r = newMessage.substring(3, 6).toInt();
  int g = newMessage.substring(6, 9).toInt();
  int b = newMessage.substring(9, 12).toInt();

  
  struct ParsedPixel parsedPixel;
  parsedPixel.x = --x;
  parsedPixel.y = --y;
  parsedPixel.selected = selected == "t";
  parsedPixel.r = r;
  parsedPixel.g = g;
  parsedPixel.b = b;
  
  return parsedPixel;
}

const size_t bufferSize = JSON_OBJECT_SIZE(6) + 30;
StaticJsonBuffer<bufferSize> jsonBuffer;

ParsedPixel parseJson(String newMessage){
  
  Serial.print("parsing as json : " );
  Serial.println(newMessage);
  jsonBuffer.clear();
  JsonObject& root = jsonBuffer.parseObject(newMessage.c_str());

  if (!root.success()) {
    Serial.print("parseObject() failed - message: ");
    Serial.println(newMessage);
  }

  struct ParsedPixel parsedPixel;
  parsedPixel.x = ((int)root["x"]) - 1;
  parsedPixel.y = ((int)root["y"]) - 1;
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
  Serial.print(parsedPixel.x);
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

  do{
    for (int i = 0; i < SINE_TABLE_SIZE; i++){
      uint8_t red = pgm_read_byte(&_sineTable[i]);
      //setAllPixels(red, 0, 0);
      pixels.setPixelColor(pixelIndex, red, 0, 0);
      pixels.show();
      delay(10);
    }
  }while(infinitely);
}
 
void setAllPixels(int red, int green, int blue) {
  for (int i = 0; i < NUM_PIXELS; i++){
    pixels.setPixelColor(i, red, green, blue); 
  }
}



void drawBallUpdates(){
  
  int pixelIndex = getPixelIndex(ball.x, ball.y);
  pixels.setPixelColor(pixelIndex, pixels.Color(0, 0, 0));  
  Serial.print("x:");
  Serial.print(ball.x);
  Serial.print(" y:");
  Serial.print(ball.y);
  Serial.print(" direction:");
  Serial.println(ball.direction);
  switch (ball.direction){
    case BALL_UP: ball.y++;break;
    case BALL_RIGHT: ball.x++;break;
    case BALL_LEFT: ball.x--;break;
    case BALL_DOWN: ball.y--;break;
  }
  detectBorderAndSwitchDirections();
  pixelIndex = getPixelIndex(ball.x, ball.y);
  pixels.setPixelColor(pixelIndex, pixels.Color(0, 0, 64));  
  pixels.show();
  delay(200);
}

void detectBorderAndSwitchDirections(){
  switch(ball.x) {
    case -1: 
      ball.x++;
      ball.direction = newRandomDirection(ball.direction); break;
    case ROWS: 
      ball.x--;
      ball.direction = newRandomDirection(ball.direction); break;
  }
  switch(ball.y) {
    case -1:
      ball.y++;
      ball.direction = newRandomDirection(ball.direction); break;
    case ROWS: 
      ball.y--;
      ball.direction = newRandomDirection(ball.direction); break;
  }
}

int newRandomDirection(int oldDirection){
  int newDirection = random(0, BALL_DOWN);
  if(newDirection != oldDirection){
    return newDirection;
  } else {
    return newRandomDirection(oldDirection);
  }
}
