// #define DEBUG

#ifdef DEBUG
// put any smaller debug expressions in here
#endif

// #define UNIT_TESTS

#include <WiFi.h>
#include <AsyncWebServer_RP2040W.h>
#include <SPI.h>
#include <SD.h>

#include <Arduino.h>
#include <Wire.h>
#include <LiquidCrystal_PCF8574.h>

/* LCD */
char loading[4][2] = {"[","|","]","|"};
uint8_t loadingIdx = 0;
uint32_t prevLoadingMillis = 0;
int8_t menuPos = 0;

const char rootMenuLen = 5;
const char rootMenu[rootMenuLen][15] = {
  "GET METADATA",
  "SEND CONFIG",
  "GET PHOTOS",
  "SAMPLE PHOTO",
  "EXIT"
};

LiquidCrystal_PCF8574 lcd(0x27);
// prototypes
bool initLCD();
void showMenu(int8_t scroll);
void showLoading();
void clearBottom();
void showSent();
void updateUI();

/* SD */
// SPI pins
const int _MOSI = 19;
const int _SCK = 18;
const int _CS = 17;
const int _MISO = 16;

// sd-contro.ino prototypes
bool initMicroSDCard();
bool checkAndCreateDir(String dirPath);

/* WEB SERVER */
const char ssid[] = "starling-hub";
const char password[] = "ilovestarlings";
AsyncWebServer server(8080);
File receivedFile;
// request-handlers.ino prototypes
void setupHandlers();
void handleRoot(AsyncWebServerRequest *request);
void handleNotFound(AsyncWebServerRequest *request);
void printWifiStatus();
bool handlePhotoUpload(AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total, bool sample=false);
void handleFileUpload(AsyncWebServerRequest *request);
bool handleNextInstruction(AsyncWebServerRequest *request);
bool handleUpdateConfig(AsyncWebServerRequest *request);

/* GENERAL */
String instructions[] = {"sendMetadata", "updateConfig", "sendPhotos", "samplePhoto", "exit", "wait"};
volatile bool instructionSent = true;
volatile bool instructionComplete = true;
volatile bool oldInstructionSent = true;
volatile bool oldInstructionComplete = true;
typedef enum{
  SEND_METADATA = 0,
  UPDATE_CONFIG = 1,
  SEND_PHOTOS = 2, 
  SAMPLE_PHOTO = 3,
  EXIT = 4, 
  WAIT = 5
  // room to add more if necessary
} t_instruction;
t_instruction intToInstruction(int i) {
  switch (i%(WAIT+1)) {
    case 0:
      return SEND_METADATA;
    case 1:
      return UPDATE_CONFIG;
    case 2:
      return SEND_PHOTOS;
    case 3:
      return SAMPLE_PHOTO;
    case 4:
      return EXIT;
    case 5:
      return WAIT;
  }
  return WAIT;
}
t_instruction currentInstruction = WAIT;


/* BUTTONS */
typedef enum {
  CONFIRM = 0,
  UP = 1,
  DOWN = 2,
  CANCEL = 3, 
  NONE = 4
} t_actions;

t_actions action = NONE;

const uint8_t CONFIRM_PIN = 10;
const uint8_t UP_PIN = 11;
const uint8_t DOWN_PIN = 12;
const uint8_t CANCEL_PIN = 13;

uint32_t prevButtonReadMillis = 0;
const uint16_t debounceTime = 500;

void initButtons();
void getNextAction();

/* RF-LINKER */
uint32_t prevRequestMillis = 0;
uint32_t prevPingMillis = 0;
bool gpio20toggle = false;
volatile bool deviceConnected = false;
void waitForConnection();

void pingDevice();

#ifndef UNIT_TESTS // this is the production code

void setup() {
  #ifdef DEBUG
  Serial.begin(115200);
  while (!Serial) {
    delay(200);
  }
  Serial.println("Initialised Serial monitor");
  #endif
  initWiFi();
  if (!initMicroSDCard()) return;
  if (!initLCD()) return;
  setupHandlers();
  initButtons();
  showMenu(0);
  pinMode(20, OUTPUT); // GPIO20 for RF linker
  waitForConnection();

}

void loop() {
  updateUI();
  if (millis() - prevRequestMillis > 30000) { // no connection for 30 seconds...
    deviceConnected = false;
    waitForConnection();
  }
}
#endif // end production code

#ifdef UNIT_TESTS // Unit test code

// #define SD_COPY_TEST // test status - PASSED
// #define LCD_DISPLAY_TEST // test status - PASSED
// #define FILE_UPLOAD_TEST // test status - PASSED
// #define MENU_TEST // test status - PASSED
// #define SERVER_ENDPOINTS_TEST test status - PASSED

#ifdef MENU_TEST
uint32_t dummyMillis = 0;
uint32_t prevInstructionSentDummy = true;
void dummyInstructionSent() {
  if (!instructionSent && prevInstructionSentDummy) {
    dummyMillis = millis();
    prevInstructionSentDummy = false;
    return;
  }
  if (!instructionSent && millis() - dummyMillis > 10000) {
    instructionSent = true;
    prevInstructionSentDummy = true;
  }
}

#endif
void setup() {
  #ifdef DEBUG
  Serial.begin(115200);
  while (!Serial) {
    delay(200);
  }
  Serial.println("Unit test has begun");
  #endif

  #ifdef SD_COPY_TEST
  initMicroSDCard();

  File origFile = SD.open("/sample.jpeg");
  uint32_t fileLen = origFile.size();
  uint8_t* fileBuf = (uint8_t*)malloc(fileLen);
  origFile.read(fileBuf, fileLen);
  origFile.close();

  File copyFile = SD.open("/copy.jpeg", FILE_WRITE);
  copyFile.write(fileBuf, fileLen);
  copyFile.close();
  #ifdef DEBUG
  Serial.println("Done");
  #endif
  #endif // end SD_COPY_TEST

  #ifdef LCD_DISPLAY_TEST
  
  int error;
  Wire.begin();
  Wire.beginTransmission(0x27);
  error = Wire.endTransmission();
  #ifdef DEBUG
  Serial.print("Error: ");
  Serial.print(error);
  #endif

  if (error == 0) {
    #ifdef DEBUG
    Serial.println(": LCD found.");
    #endif
    lcd.begin(16, 2);  // initialize the lcd

  } else {
    #ifdef DEBUG
    Serial.println(": LCD not found.");
    #endif
  }
  #endif // end LCD_DISPLAY_TEST

  #ifdef FILE_UPLOAD_TEST
  WiFi.mode(WIFI_AP);

  WiFi.softAP(ssid, password);

  while(WiFi.status() != WL_CONNECTED) {
    delay(300);
  }

  #ifdef DEBUG
  Serial.print("AP started with IP:");
  Serial.println(WiFi.softAPIP());
  #endif
  initMicroSDCard();
  setupHandlers();
  #endif // end FILE_UPLOAD_TEST

  #ifdef MENU_TEST
  initLCD();
  initButtons();
  showMenu(0);
  #ifdef DEBUG
  Serial.println("Finished init");
  #endif
  #endif // end MENU_TEST

  #ifdef SERVER_ENDPOINTS_TEST
  initWiFi();
  initMicroSDCard();
  setupHandlers();

  currentInstruction = SAMPLE_PHOTO; // change this to test each instruction
  #endif // end SERVER_ENDPOINTS_TEST
}

void loop() {
  #ifdef LCD_DISPLAY_TEST
  lcd.setBacklight(255);
  lcd.home();
  lcd.clear();
  lcd.print("LCD test");
  delay(1000);
  lcd.print("!");
  delay(1000);
  #endif

  #ifdef MENU_TEST
  updateUI();
  dummyInstructionSent();
  #endif
}
#endif // end unit test code