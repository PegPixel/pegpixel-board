#include <ArduinoJson.h>
#include <BluetoothSerial.h>
#include <NeoPixelBrightnessBus.h>
#include <Wire.h>
#include <neotimer.h>

const int HALL_REGISTER_ADR=0x1F;
const int HALL_REGISTER_SIZE_BYTE=4;
const int NUMBER_OF_HALL_SENSORS=2;
const int HALL_SENSOR_OFFSET=0x40;

#ifdef __AVR__
  #include <avr/power.h>
#endif

#define NEOPIXEL_PIN            19

#define COLUMNS 7
#define ROWS 5
#define NUM_PIXELS (COLUMNS * ROWS)

NeoPixelBrightnessBus<NeoGrbFeature, Neo800KbpsMethod> pixels(NUM_PIXELS, NEOPIXEL_PIN);
NeoGamma<NeoGammaTableMethod> colorGamma;

Neotimer timer;

BluetoothSerial mySerial;

struct ParsedPixel {
  int x;
  int y;
  boolean selected;
  int r;
  int g;
  int b;
};

void setup() {
  timer.set(500);
  
  Serial.setTimeout(10);
  Serial.begin(115200);

  Wire.begin();
  
  mySerial.begin("pegpixel-board");
  
  Serial.write("Serial is online\n");
  pixels.Begin();
  pixels.Show();  
}

void loop() {
  if (timer.repeat())
    drawHallSensorUpdates();
  
  drawBluetoothUpdates();
  updatePixelBrightness();
}

void drawHallSensorUpdates() {
  for (int i = 0; i < NUMBER_OF_HALL_SENSORS; i++)
    if (readSensor(i + HALL_SENSOR_OFFSET)) {
      pixels.SetPixelColor(i, createCorrectedColor(0, 0, 0));
      pixels.Show();
    }
}

void drawBluetoothUpdates(){
  if(mySerial.available()){
    String newMessage = mySerial.readStringUntil('\n');
    Serial.println(newMessage);
    ParsedPixel parsedPixel = parseJson(newMessage);
    drawPixel(parsedPixel);
  }
}

ParsedPixel parseMessage(String newMessage) {
  int x = newMessage.substring(0 ,1).toInt();
  int y = newMessage.substring(1, 2).toInt();
  String selected = newMessage.substring(2, 3);
  int r = newMessage.substring(3, 6).toInt();
  int g = newMessage.substring(6, 9).toInt();
  int b = newMessage.substring(9, 12).toInt();

  struct ParsedPixel parsedPixel;
  parsedPixel.x = x;
  parsedPixel.y = y;
  parsedPixel.selected = selected == "t";
  parsedPixel.r = r;
  parsedPixel.g = g;
  parsedPixel.b = b;
  
  return parsedPixel;
}

const size_t bufferSize = JSON_OBJECT_SIZE(6) + 40;
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
    pixels.SetPixelColor(pixelIndex, createCorrectedColor(parsedPixel.r, parsedPixel.g, parsedPixel.b));
  } else {
    pixels.SetPixelColor(pixelIndex, createCorrectedColor(0, 0, 0));  
  }
  pixels.Show();
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
      pixels.SetPixelColor(pixelIndex, createCorrectedColor(red, 0, 0));
      pixels.Show();
      delay(10);
    }
  }while(infinitely);
}

int currentSineIndex = 0;
 
void updatePixelBrightness(){
  if(millis() % 5 == 0){
    if(currentSineIndex < SINE_TABLE_SIZE -1){
      currentSineIndex++;    
    } else {
      currentSineIndex = 0;
    }
    uint8_t sine = pgm_read_byte(&_sineTable[currentSineIndex]);
    uint8_t brightness = map(sine, 0, 255, 32, 255);
    pixels.SetBrightness(brightness);
    pixels.Show();
  }
}


RgbColor createCorrectedColor(int red, int green, int blue){
  return colorGamma.Correct(RgbColor(red, green, blue));
}


bool readSensor(int sensorAdr) {
  Wire.beginTransmission(sensorAdr);
  Wire.write(HALL_REGISTER_ADR);
  Wire.endTransmission(false);
  Wire.requestFrom(sensorAdr, HALL_REGISTER_SIZE_BYTE, true);  
  
  int bytesAvailable = Wire.available();
  byte data[bytesAvailable];
  for (int i = 0; i < bytesAvailable; i++)
    data[i] = Wire.read();
    
  if (data[2] == 15 && data[3] == 252)
    return false;
  else
    return data[2] > 14;
}
