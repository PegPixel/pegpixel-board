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
  
  Serial.begin(baudRate);
  
  mySerial.begin(baudRate);
  
  Serial.write("Serial is online\n");
  pixels.begin();
  pixels.show();
}


void loop() {

  /*
  if(mySerial.available()){
    char readChar = mySerial.read();
    Serial.write(readChar); 
  }
  */

  //readCharAndDrawPixels();  
  readLineAndDrawPixels();
}


void readCharAndDrawPixels(){
  if(mySerial.overflow()){
    Serial.println("Overflow has occurred!");
  }
  if(mySerial.available()){
    char readChar = readFromBluetooth();
    boolean newMessage = preparseMessage(readChar);

    //if(false == true){
    if(newMessage == true){
      JsonObject& json = parseJson();
      drawPixel(json);
      resetReceivedMessage();
    }
  }
}

void readLineAndDrawPixels(){

  if(mySerial.available()){
    String readString = readStringFromBluetooth();
    
    for(int i = 0; i < readString.length(); i++){
        
      boolean newMessage = preparseMessage(readString[i]);
      
      if(newMessage == true){
        JsonObject& json = parseJson();
        drawPixel(json);
        resetReceivedMessage();
      }
    }
  }
}

//String receivedMessage;
const int receivedMessageSize = 32;
char receivedMessage[receivedMessageSize] = "";
int indexInReceivedMessage= 0;

boolean inObject = false;

char readFromBluetooth() {
  char readChar = mySerial.read();
  Serial.write(readChar);
  return readChar;
}

String readStringFromBluetooth() {
  String readString = mySerial.readString();
  Serial.write(readString.c_str());
  return readString;
}

boolean preparseMessage(char readChar) {
// Ignore start and end of array
  if(readChar == '[' || readChar == ']') {
    return false;
  }
  // ignore comma separating objects
  if(readChar == ',' && !inObject) {
    return false;
  }
  addReceivedChar(readChar);
  if(readChar == '{'){
    inObject = true;
  }
  if(readChar == '}'){
    Serial.print("new message: ");
    inObject = false;
    return true;
  }
}

void addReceivedChar(char c) {
    //receivedMessage.concat(c);
    receivedMessage[indexInReceivedMessage++] = c;
}


JsonObject& parseJson(){
  const size_t bufferSize = JSON_OBJECT_SIZE(1) + 10;
  DynamicJsonBuffer jsonBuffer(bufferSize);
  
  Serial.println("parsing new message..");
  JsonObject& root = jsonBuffer.parseObject(receivedMessage);

  if (!root.success()) {
    Serial.print("parseObject() failed - ");
  } else {
    root.printTo(Serial);
    Serial.print("\n");
  }
  return root;
}

//uint32_t pixelState[NUMPIXELS] = pixels.Color(0, 0, 0);

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

void resetReceivedMessage() {
  receivedMessage[32] = "";
  indexInReceivedMessage= 0;
}
