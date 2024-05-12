// define this to enable debug output on Serial
// #define DEBUG

// define this to run the unit test code rather than the production code
// #define UNIT_TESTS

// define this for ATP1
// #define ATP1

// define this for ATP7
// #define ATP7

#if defined(ATP1) || defined(ATP7)
#define ATPS
#endif
/* MAIN TASKS */
typedef enum{
  SEND_METADATA = 0,
  UPDATE_CONFIG = 1,
  SEND_PHOTOS = 2, 
  SAMPLE_PHOTO = 3,
  EXIT = 4,
  WAIT = 5
  // room to add more if necessary
} t_instruction;
t_instruction currentInstruction = WAIT;
void triggerCapture();
void triggerTransmitter();
void handleInstruction(t_instruction instruction);

/** LIBRARY INCLUDES **/
//Base library
#include <Arduino.h>
// Wifi libraries
#include <WiFi.h>
#include <HTTPClient.h>
// Camera libraries
#include "esp_camera.h"
#include "soc/soc.h"
#include "soc/rtc_cntl_reg.h"
#include "driver/rtc_io.h"
// MicroSD Libraries
#include "FS.h"
#include "SD_MMC.h"
#include <ArduinoJson.h>
 
// Counter for picture number
unsigned int pictureCount = 0;

/* SD CARD */
String configDir = "/config";
String numPhotosPath = "/config/numPhotos.json";
String newNumPhotosJson = "{\"numPhotos\":0}";
/* sd-control.ino prototypes */
bool initMicroSDCard();
bool setNumPhotos(uint32_t numPhotos);
uint32_t getNumPhotos();
bool deleteAndRecreateNumPhotos();
bool checkAndCreateDir(String dirPath);


/* CONFIG FILE */
String configFilePath = "/config/config.json";
/* config-file-init.ino prototypes */
bool loadFromConfig();
bool networkConfig(JsonObject networkObj);
bool cameraConfig(JsonObject cameraObj);
bool cameraTimingConfig(JsonObject cameraTimingObj);

/* NETWORK */
String ssid = "starling-hub";
String password = "ilovestarlings";
String serverAddress = "http://192.168.42.1:8080"; // server IP, can up updated via config
WiFiClient client; // client for reading stream from server
HTTPClient http;
const uint8_t numAttempts = 3; // global number of attempts at a web request before giving up and going back to sleep
uint8_t numWiFiFails = 0;
/* web-requests.ino prototypes */
int16_t sendPhoto(String thisPhotoDir, String thisPhotoName, uint32_t photoNum);
uint32_t sendAllPhotos();
int16_t updateConfig();
int16_t requestInstruction();
bool initWiFi();
void onWiFiEvent(WiFiEvent_t event);

/* CAM */
// pin definitions for AI Thinker ESP-CAM
#define PWDN_GPIO_NUM     32
#define RESET_GPIO_NUM    -1
#define XCLK_GPIO_NUM      0
#define SIOD_GPIO_NUM     26
#define SIOC_GPIO_NUM     27
#define Y9_GPIO_NUM       35
#define Y8_GPIO_NUM       34
#define Y7_GPIO_NUM       39
#define Y6_GPIO_NUM       36
#define Y5_GPIO_NUM       21
#define Y4_GPIO_NUM       19
#define Y3_GPIO_NUM       18
#define Y2_GPIO_NUM        5
#define VSYNC_GPIO_NUM    25
#define HREF_GPIO_NUM     23
#define PCLK_GPIO_NUM     22
// default camera settings (modifiable from config file)
int8_t brightness = 0;
int8_t contrast = 0;
int8_t saturation = 0;
uint8_t specialEffects = 0;
uint8_t whiteBalance = 1;
uint8_t awbGain = 1;
uint8_t wbMode = 0;
uint8_t exposureCtrl = 0;
uint8_t aec2 = 0;
int8_t aeLevel = 0;
uint16_t aecValue = 1200;
uint8_t gainCtrl = 1;
uint8_t agcGain = 0;
uint8_t gainCeiling = 0;
uint8_t bpc = 0;
uint8_t wpc = 1;
uint8_t rawGma = 1;
uint8_t lenc = 1;
uint8_t hmirror = 0;
uint8_t vflip = 0;
uint8_t dcw = 1;
uint8_t colorbar = 0; 
uint8_t jpegQuality = 10;
// timing config options
uint16_t timeBetweenTriggers = 60; // in seconds
uint16_t capturesPerTrigger = 3;
// device ID
String deviceID = "cam1";
// photo information
String photosDir = "/photos";
String photoName = "starlingcam"; // start of the name of each photo being saved
String sampleName = "sample";
/* camera-control.ino prototypes */
bool configESPCamera();
bool takeNewPhoto(String thisPhotoDir, String thisPhotoName);
bool nextPhoto();



/* DEEP SLEEP */
#define PIR_RF_BITMASK 0b10000000010100 // = GPIO2(RTC12) + GPIO4(RTC10) + GPIO13(RTC14) = 2^1 + 2^3
#define PIR_BITMASK 0x10100 // GPIO2 + GPIO4
#define RF_BITMASK 0b10000000000000 // GPIO13
#define PIR_1_PIN 2 // note - must also change RTC deinit in sleep deinit
#define PIR_2_PIN 4
#define RF_PIN 13
/* sleep.ino prototypes */
void timerRFSleep(uint16_t time);
void pirRFSleep();
uint8_t deepSleepAwakenPin();
void deinitSleep();

/* START SETUP AND LOOP MAIN */
#ifndef UNIT_TESTS
void setup() {
  // Disable brownout detector
  WRITE_PERI_REG(RTC_CNTL_BROWN_OUT_REG, 0);

  #if defined(DEBUG) || defined(ATPS)
  Serial.begin(115200);
  while (!Serial) {
    sleep(200);
  }
  #endif
  #ifdef ATPS // sleep to allow time to trigger GPIO1 switch
  sleep(5000);
  #endif
  esp_sleep_wakeup_cause_t wakeupReason = esp_sleep_get_wakeup_cause();
  deinitSleep(); // TODO: Is there a chance this could mess checking the awakening pin?
  if (wakeupReason == ESP_SLEEP_WAKEUP_TIMER) {
    #ifdef DEBUG
    Serial.println("Awoken by timer, sleeping");
    #endif
    pirRFSleep(); // immediately go back to sleep with PIR trigger
  }
  else if (wakeupReason != ESP_SLEEP_WAKEUP_EXT1) {
    #ifdef DEBUG
    Serial.println("Not awoken by deep sleep, sleeping");
    #endif
    pirRFSleep();
  }
  // Start Serial Monitor
  uint8_t awakenPin = deepSleepAwakenPin();

  switch(awakenPin) {
    case PIR_1_PIN:
    case PIR_2_PIN:
      triggerCapture();
      break;
    case RF_PIN:
      triggerTransmitter();
      break;
  }
}

void loop() {
  vTaskDelete(NULL);
  // actual operation loop is inside the different trigger modes in main-tasks
}
#endif
/* END SETUP AND LOOP MAIN */

/* START UNIT TESTING */
#ifdef UNIT_TESTS
// choose the correct define to run the desired test
// #define CONFIG_TEST // test status - PASSED
// #define FILE_SENT_TEST // test status - PASSED, possible changes incoming
// #define READ_NUM_IMAGES_TEST // test status - PASSED
// #define TAKE_PHOTO_TEST // test status - PASSED
// #define MANY_PHOTOS_TEST // test status - PASSED
// #define SEND_ALL_FILES_TEST // test status - PASSED
// #define UPLOAD_TO_PI_TEST // test status - PASSED
// #define UPDATE_CONFIG_TEST // test status - PASSED
// #define CAMERA_QUALITY_TESTING
// #define SEND_METADATA_TEST
#define TRANSMITTER_MODE_TEST
// #define CAPTURE_MODE_TEST
/* still to run:
  Each instruction individually
  trigger capture mode
  trigger transmitter mode
  deep sleep things
*/

#ifdef MANY_PHOTOS_TEST
uint32_t photoNum = 0;
#endif
void setup() {
  WRITE_PERI_REG(RTC_CNTL_BROWN_OUT_REG, 0);
  // Start Serial Monitor
  #ifdef DEBUG
  Serial.begin(115200);
  while (!Serial) {
    delay(200);
  }
  Serial.println("Started serial monitor");
  #endif

  #ifdef CONFIG_TEST
  initMicroSDCard();
  loadFromConfig();
  #endif

  #ifdef FILE_SENT_TEST
  initMicroSDCard();
  loadFromConfig();

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  #ifdef DEBUG
  Serial.print("Connecting to wifi...");
  #endif
  while(WiFi.status() != WL_CONNECTED) {
    delay(300);
    #ifdef DEBUG
    Serial.print(".");
    #endif
  }
  #ifdef DEBUG
  Serial.println("");
  #endif
  String filePath = "/sample.jpeg";
  #ifdef DEBUG
  Serial.println(serverAddress);
  #endif
  sendAndDeleteImage(filePath);
  #endif // end FILE_SENT_TEST

  #ifdef READ_NUM_IMAGES_TEST
  initMicroSDCard();
  #endif // end READ_NUM_IMAGES_TEST

  #ifdef TAKE_PHOTO_TEST
  initMicroSDCard();
  configESPCamera();
  takeNewPhoto(String("/sampleCam.jpeg"));
  Serial.println("Taken photo...hopefully");
  #endif // end TAKE_PHOTO_TEST

  #ifdef MANY_PHOTOS_TEST
  initMicroSDCard();
  configESPCamera();
  #endif // end MANY_PHOTOS_TEST

  #ifdef SEND_ALL_FILES_TEST
  initMicroSDCard();
  loadFromConfig();
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  #ifdef DEBUG
  Serial.println(ssid);
  Serial.println(password);
  Serial.print("Connecting to wifi...");
  #endif
  while(WiFi.status() != WL_CONNECTED) {
    delay(300);
    #ifdef DEBUG
    Serial.print(".");
    #endif
  }
  uint32_t successNum = sendAllPhotos();
  #ifdef DEBUG
  Serial.println(successNum);
  #endif
  #endif // end SNED_ALL_FILES_TEST

  #ifdef UPLOAD_TO_PI_TEST
  initMicroSDCard();
  loadFromConfig();
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  #ifdef DEBUG
  Serial.println(ssid);
  Serial.println(password);
  Serial.print("Connecting to wifi...");
  #endif
  while(WiFi.status() != WL_CONNECTED) {
    delay(300);
    #ifdef DEBUG
    Serial.print(".");
    #endif
  }
  #endif // end UPLOAD_TO_PI_TEST

  #ifdef UPDATE_CONFIG_TEST
  initMicroSDCard();
  loadFromConfig();
  initWiFi();
  #ifdef DEBUG
  Serial.println(updateConfig());
  // check SD card to see if it contains the updated config file
  #endif
  #endif // end UPDATE_CONFIG_TEST

  #ifdef CAMERA_QUALITY_TESTING
  initMicroSDCard();
  loadFromConfig();
  configESPCamera();

  #ifdef DEBUG
  Serial.println("Made it past config");
  Serial.println("Config:");
  Serial.println(brightness);
  Serial.println(contrast);
  Serial.println(saturation);
  #endif

  takeNewPhoto(configDir, sampleName);
  #endif // end CAMERA_QUALITY_TESTING

  #ifdef SEND_METADATA_TEST
  initMicroSDCard();
  loadFromConfig();
  configESPCamera();
  initWiFi();
  handleInstruction(SEND_METADATA);
  #endif // end SEND_METADATA_TEST

  #ifdef TRANSMITTER_MODE_TEST
  triggerTransmitter();
  #endif // end TRANSMITTER_MODE_TEST

  #ifdef CAPTURE_MODE_TEST
  triggerCapture();
  #endif
}

void loop() {
  #ifdef CONFIG_TEST
  Serial.println(ssid); // should be changed
  Serial.println(password); // should be changed
  Serial.println(brightness); // should be changed
  Serial.println(contrast); // should be default
  Serial.println(saturation); // should be default
  Serial.println(aecValue); // should be default
  delay(5000);
  #endif // end CONFIG_TEST

  #ifdef READ_NUM_IMAGES_TEST
  Serial.println("Starting test");
  uint32_t num = getNumPhotos();
  Serial.println(num);
  setNumPhotos(num);
  delay(5000);
  #endif // end READ_NUM_IMAGES_TEST

  #ifdef MANY_PHOTOS_TEST
  uint32_t nextPhotoNum = getNumPhotos();
  if (nextPhoto()) {
    Serial.println("Next photo taken");
    Serial.print("Photo num: ");
    Serial.println(nextPhotoNum);
  }
  else{
    Serial.println("Problem taking next photo");
  }
  #endif

  #ifdef UPLOAD_TO_PI_TEST
  int16_t responseCode = sendPhoto(String("sample.jpeg"));
  #ifdef DEBUG
  Serial.print("Response code: ");
  Serial.println(responseCode);
  #endif
  delay(5000);
  #endif // end UPLOAD_TO_PI_TEST

}
#endif
/* END UNIT TESTING */
