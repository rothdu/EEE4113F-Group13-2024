void triggerCapture() {
  #ifdef DEBUG
  Serial.println("Capture mode triggered");
  #endif

  if (!initMicroSDCard()) {
    #ifdef DEBUG
    Serial.println("Failed to init SD card");
    #endif
    pirRFSleep(); // can't do much without the microSD
  }

  if (!loadFromConfig()) {
    #ifdef DEBUG
    Serial.println("No config file, default values will be used");
    #endif
    // failure means the default config is used
  }
  #ifdef ATP8
  Serial.print("Exposure: ");
  Serial.println(aecValue);
  Serial.print("Captures per trigger: ");
  Serial.println(capturesPerTrigger);
  Serial.print("Time between triggers: ");
  Serial.println(timeBetweenTriggers);
  #endif

  if (!configESPCamera()) {
    #ifdef DEBUG
    Serial.println("Failed to config camera, sleeping");
    #endif
    pirRFSleep();
  }

  for (uint32_t i = 0; i<capturesPerTrigger; i++) {
    if (!nextPhoto()) {
      pirRFSleep(); // maybe it will work next time...
    }
    delay(50); // short delay between pictures
  }
  timerRFSleep(timeBetweenTriggers);
}

void triggerTransmitter() {
  #ifdef ATP4
  Serial.println("Awoken in transmission mode");
  #endif 

  #ifdef DEBUG
  Serial.println("Transmitter mode triggered");
  #endif
  if (!initMicroSDCard()) {
    #ifdef DEBUG
    Serial.println("Failed to init SD card");
    #endif
    pirRFSleep(); // TODO: Could send a failure message over wifi
  }

  if (!loadFromConfig()) {
  #ifdef DEBUG
  Serial.println("No config file, default values will be used");
  #endif
  // failure means the default config is used
  }

  if (!configESPCamera()) {
    #ifdef DEBUG
    Serial.println("Failed to config camera, sleeping");
    #endif
    pirRFSleep();
  }
  
  if (!initWiFi()) {
    #ifdef DEBUG
    Serial.println("Failed to init WiFi, sleeping");
    #endif
    pirRFSleep();
  }


  for(;;) { // inifinite loop
    uint8_t a;
    for (a = 0; a < numAttempts; a++) {
      if (requestInstruction() == HTTP_CODE_OK) {
        break;
      }
    }
    if (a == numAttempts) { // something has gone wrong - go back to sleep
      
      pirRFSleep();
    }
    #ifdef DEBUG
    Serial.print("Current instruction:");
    Serial.println(currentInstruction);
    #endif
    handleInstruction(currentInstruction);
  }
}

void handleInstruction(t_instruction instruction) {
  switch (instruction) {
    case WAIT: 
      delay(10000);
      break;

    case SEND_METADATA:
      for (uint8_t a = 0; a < numAttempts; a++) {
        if (sendMetadata() == HTTP_CODE_CREATED) {
          #ifdef DEBUG
          Serial.println("Metadata sending successful hopefully");
          #endif
          break;
        }
      }
      break;

    case UPDATE_CONFIG:
      for (uint8_t a = 0; a < numAttempts; a++) {
        if (updateConfig() == 200) {
          break;
        }
      }
      break;

    case SEND_PHOTOS:
      sendAllPhotos(); // could add something that happens if it can't send all photos
      break;

    case SAMPLE_PHOTO:
  
      if(!takeNewPhoto(configDir, sampleName + String(".jpeg"))) {
        break;
      }
      for (uint8_t a = 0; a < numAttempts; a++) {
        if (sendSample(configDir, sampleName) == HTTP_CODE_CREATED) { // 201, resource created
          break;
        }
      }
      break;

    case EXIT: {
      pirRFSleep();
    }
  }
}