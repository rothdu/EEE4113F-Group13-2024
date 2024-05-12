/** 
 * This will load configuration settings for various important operating parameters
 * If the config file doesn't exist, or the items of the file can't be parsed correctly
 * the default values defined in the primary sketch are left as it
 */
bool loadFromConfig() {
  fs::FS &fs = SD_MMC;
  #ifdef DEBUG
  Serial.println("Loading from config");
  #endif

  if (!checkAndCreateDir(configDir)) {
    #ifdef DEBUG
    Serial.println("Couldn't create config DIR");
    #endif
    // this is a major fail...
    return 0;
  }

  File configFile = fs.open(configFilePath.c_str(), FILE_READ); // open file

  #ifdef DEBUG
  Serial.println("Theoretically opened config file");
  #endif

  if (!configFile) {
    #ifdef DEBUG
    Serial.println("No config file found");
    #endif
    return false; 
  }

  if (configFile.isDirectory()) {
    #ifdef DEBUG
    Serial.println("Config file is a directory?");
    #endif
    return false;
  }

  size_t len = configFile.size(); // check length

  if (len > 2048) {
    #ifdef DEBUG
    Serial.println("Config document too large");
    #endif
    return false;
  }
  
  // will it matter that this is a uint8_t and not a char*?
  // doesn't seem to
  // initialise it with double the required length... since ArduinoJson apparently writes to the original buffer
  uint8_t* configBuf = (uint8_t*)malloc(2*len);
  // create buffer on heap... could be quite large
  if (configBuf == nullptr) {
    #ifdef DEBUG
    Serial.println("Could not allocate memory for config file");
    #endif
    return false;
  } 
  configFile.read(configBuf, len);

  // store on heap to enable dynamic memory allocation at runtime
  JsonDocument configDoc;
  // store on heap since > 1KiB
  DeserializationError err = deserializeJson(configDoc, configBuf);

  if (err) {
    #ifdef DEBUG
    Serial.println("Error deserialising JSON");
    // could possibly report the error code, although I don't think that's particularly important
    #endif
    return false;
  }
  
  // create JSON object which can check for types of keys
  JsonObject configObj = configDoc.as<JsonObject>();

  // run network config
  if (configObj["network"].is<JsonObject>()) {
    JsonObject networkObj = configObj["network"];
    networkConfig(networkObj);
  }

  // run camera config
  if (configObj["camera"].is<JsonObject>()) {
    JsonObject cameraObj = configObj["camera"];
    cameraConfig(cameraObj);
  }

  if (configObj["cameraTiming"].is<JsonObject>()) {
    JsonObject cameraTimingObj = configObj["cameraTiming"];
    cameraTimingConfig(cameraTimingObj);
  }

  // get device ID
  if (configObj["deviceID"].is<String>()) {
    deviceID = configObj["deviceID"].as<String>();
  }

  
  // close the config file - DO NOT DO THIS BEFORE CLOSING THE JSON DOC
  free(configBuf);
  configFile.close();
  configDoc.clear();
  return true;
}

bool networkConfig(JsonObject networkObj) {

  if (networkObj.isNull()) {
    #ifdef DEBUG
    Serial.println("No network settings found");
    #endif
    return false;
  }

  if (networkObj["ssid"].is<String>()) {
    ssid = networkObj["ssid"].as<String>();
  }

  if (networkObj["password"].is<String>()) {
    password = networkObj["password"].as<String>();
  }

  if (networkObj["serverAddress"].is<String>()) {
    serverAddress = networkObj["serverAddress"].as<String>();
  }

  return true;
}

bool cameraConfig(JsonObject cameraObj) {
  if (cameraObj.isNull()) {
    #ifdef DEBUG
    Serial.println("No camera settings found");
    #endif
    return false;
  }

  int16_t temp;

  // BRIGHTNESS (-2 to 2)
  if (cameraObj["brightness"].is<int>()) {
    temp = cameraObj["brightness"];
    if (temp >= -2 && temp <= 2) {
      brightness = temp;
    }
  }

  // CONTRAST (-2 to 2)
  if (cameraObj["contrast"].is<int>()) {
    temp = cameraObj["contrast"];
    if (temp >= -2 && temp <= 2) {
      contrast = temp;
    }
  }

  // SATURATION (-2 to 2)
  if (cameraObj["saturation"].is<int>()) {
    temp = cameraObj["saturation"];
    if (temp >= -2 && temp <= 2) {
      saturation = temp;
    }  
  }

  // SPECIAL EFFECTS (0 - No Effect, 1 - Negative, 2 - Grayscale, 3 - Red Tint, 4 - Green Tint, 5 - Blue Tint, 6 - Sepia)
  if (cameraObj["specialEffects"].is<int>()) {
    temp = cameraObj["specialEffects"]; 
    if (temp >= 0 && temp <= 6) {
      specialEffects = temp;
    }  
  }

  // WHITE BALANCE (0 = Disable , 1 = Enable)
  if (cameraObj["whiteBalance"].is<int>()) {
    temp = cameraObj["whiteBalance"]; 
    if (temp >= 0 && temp <= 1) {
      whiteBalance = temp;
    }  
  } 
  
  // AWB GAIN (0 = Disable , 1 = Enable)
  if (cameraObj["awbGain"].is<int>()) {
    temp = cameraObj["awbGain"];
    if (temp >= 0 && temp <= 1) {
      awbGain = temp;
    } 
  }

  // WB MODES (0 - Auto, 1 - Sunny, 2 - Cloudy, 3 - Office, 4 - Home)
  if (cameraObj["wbMode"].is<int>()) {
    temp = cameraObj["wbMode"];
    if (temp >= 0 && temp <= 4) {
      wbMode = temp;
    } 
  }

  // EXPOSURE CONTROLS (0 = Disable , 1 = Enable)
  if (cameraObj["exposureCtrl"].is<int>()) {
    temp = cameraObj["exposureCtrl"];
    if (temp >= 0 && temp <= 1) {
      exposureCtrl = temp;
    }  
  }

  // AEC2 (0 = Disable , 1 = Enable)
  if (cameraObj["aec2"].is<int>()) {
    temp = cameraObj["aec2"];
    if (temp >= 0 && temp <= 1) {
      aec2 = temp;
    } 
  }

  // AE LEVELS (-2 to 2)
  if (cameraObj["aeLevel"].is<int>()) {
    temp = cameraObj["aeLevel"];
    if (temp >= -2 && temp <= 2) {
      aeLevel = temp;
    }  
  }

  // AEC VALUES (0 to 1200)
  if (cameraObj["aecValue"].is<int>()) {
    temp = cameraObj["aecValue"]; 
    if (temp >= 0 && temp <= 1200) {
      aecValue = temp;
    } 
  }

  // GAIN CONTROLS (0 = Disable , 1 = Enable)
  if (cameraObj["gainCtrl"].is<int>()) {
    temp = cameraObj["gainCtrl"];
    if (temp >= 0 && temp <= 1) {
      gainCtrl = temp;
    } 
  }

  // AGC GAIN (0 to 30)
  if (cameraObj["agcGain"].is<int>()) {
    temp = cameraObj["agcGain"];  
    if (temp >= 0 && temp <= 30) {
      agcGain = temp;
    } 
  }

  // GAIN CEILING (0 to 6)
  if (cameraObj["gainCeiling"].is<int>()) {
    temp = cameraObj["gainCeiling"];
    if (temp >= 0 && temp <= 6) {
      gainCeiling = temp;
    } 
  }

  // BPC (0 = Disable , 1 = Enable)
  if (cameraObj["bpc"].is<int>()) {
    temp = cameraObj["bpc"];
    if (temp >= 0 && temp <= 1) {
      bpc = temp;
    } 
  }

  // WPC (0 = Disable , 1 = Enable)
  if (cameraObj["wpc"].is<int>()) {
    temp = cameraObj["wpc"]; 
    if (temp >= 0 && temp <= 1) {
      wpc = temp;
    }  
  }

  // RAW GMA (0 = Disable , 1 = Enable)
  if (cameraObj["rawGma"].is<int>()) {
    temp = cameraObj["rawGma"];
    if (temp >= 0 && temp <= 1) {
      rawGma = temp;
    }
  }

  // LENC (0 = Disable , 1 = Enable)
  if (cameraObj["lenc"].is<int>()) {
    temp = cameraObj["lenc"];
    if (temp >= 0 && temp <= 1) {
      lenc = temp;
    }
  }

  // HORIZ MIRROR (0 = Disable , 1 = Enable)
  if (cameraObj["hmirror"].is<int>()) {
    temp = cameraObj["hmirror"];
    if (temp >= 0 && temp <= 1) {
      hmirror = temp;
    }
  }

  // VERT FLIP (0 = Disable , 1 = Enable)
  if (cameraObj["vflip"].is<int>()) {
    temp = cameraObj["vflip"];
    if (temp >= 0 && temp <= 1) {
      vflip = temp;
    }
  }

  // DCW (0 = Disable , 1 = Enable)
  if (cameraObj["dcw"].is<int>()) {
    temp = cameraObj["dcw"];
    if (temp >= 0 && temp <= 1) {
      dcw = temp;
    }
  }

  // COLOR BAR PATTERN (0 = Disable , 1 = Enable)
  if (cameraObj["colorbar"].is<int>()) {
    temp = cameraObj["colorbar"];
    if (temp >= 0 && temp <= 1) {
      colorbar = temp;
    }
  }

  //jpeg Quality 1-63; low = better
  if (cameraObj["jpegQuality"].is<int>()) {
    temp = cameraObj["jpegQuality"];
    if (temp >= 1 && temp <= 63) {
      jpegQuality = temp;
    }
  }

  return true;
}

bool cameraTimingConfig(JsonObject cameraTimingObj) {

  int32_t temp;
  if (cameraTimingObj["timeBetweenTriggers"].is<int>()) {
    temp = cameraTimingObj["timeBetweenTriggers"];
    if (temp >=0 && temp <= 600) {
      timeBetweenTriggers = temp;
    }
  }

    if (cameraTimingObj["capturesPerTrigger"].is<int>()) {
    temp = cameraTimingObj["capturesPerTrigger"];
    if (temp >=0 && temp <= 0xffff) {
      capturesPerTrigger = temp;
    }
  }
}