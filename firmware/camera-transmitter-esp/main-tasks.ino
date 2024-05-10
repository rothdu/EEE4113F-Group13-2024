void triggerCapture() {
  #ifdef DEBUG
  Serial.println("Capture mode triggered");
  #endif

  if (!initMicroSDCard()) {
    pirRFSleep(); // can't do much without the microSD
  }

  if (!loadFromConfig()) {
    // failure means the default config is used
  }

  for (uint32_t i = 0; i<capturesPerTrigger; i++) {
    if (!nextPhoto()) {
      // handle not being able to take the next photo
    }
    vTaskDelay(50/portTICK_PERIOD_MS); // short delay between pictures
  }
  timerRFSleep(timeBetweenTriggers);
}

void triggerTransmitter() {
  #ifdef DEBUG
  Serial.println("Transmitter mode triggered");
  #endif
  if (!initMicroSDCard()) {
    pirRFSleep(); // TODO: Could send a failure message over wifi
  }
  if (!initWiFi()) {
    pirRFSleep();
  }
  
  for(;;) { // inifinite loop
    uint8_t a;
    for (a = 0; a < numAttempts; a++) {
      if (requestInstruction() == HTTP_CODE_OK) {
        break;
      }
    }
    if (a == numAttempts) { // something has gone wrong - do the going to sleep thing
      break; // change this to sleep
    }
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
        if (sendMetadata() == 200) {
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
  
      if(!takeNewPhoto(configDir, sampleName)) {
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