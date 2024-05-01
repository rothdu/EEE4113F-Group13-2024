#define DEBUG

#ifdef DEBUG
#define DEBUG_1
#endif

#include <Arduino.h>
#include <SPI.h> // SD card
#include <SD.h> // SD card
#include <Wire.h> //LCD (I2C)
#include <PicoLCD_I2C.h>

void setup() {
  Serial.begin(9600); // for status updates/debugging

  // initialise SPI for SD card
  SPI.begin();

  // Initialise SD card

  if(!SD.begin(SD_CS_PIN)) {
    #ifdef DEBUG_1
      Serial.println("Failed to initilaise SD card");
    #endif
    return;
  }

  #ifdef DEBUG_1

}

void loop() {
  // put your main code here, to run repeatedly:

}
