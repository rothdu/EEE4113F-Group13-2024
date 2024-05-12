// init the SD card
bool initMicroSDCard() {
  // Start the MicroSD card

  // enable GPIO1 which disables the PNP switches
  // GPIO1 is also used for UART so cannot use in DEBUG mode
  #if !defined(DEBUG) && !defined(ATPS)
  pinMode(1, OUTPUT);
  digitalWrite(1, HIGH);
  #endif
  
  #ifdef DEBUG
  Serial.println("Mounting MicroSD Card");
  #endif

  // Check for mount failure
  if (!SD_MMC.begin()) {
    #ifdef DEBUG
    Serial.println("MicroSD Card Mount Failed");
    #endif
    return false;
  }

  // check for no card
  uint8_t cardType = SD_MMC.cardType();
  if (cardType == CARD_NONE) {
    
    Serial.println("No MicroSD Card found");
    return false;
  }

  #ifdef DEBUG
  Serial.println("Made it past SD card mounting");
  #endif

  return true;
}

bool deleteAndRecreateNumPhotos() {
  fs::FS &fs = SD_MMC;
  fs.remove(numPhotosPath.c_str()); // remove any existing file /directory
  File numPhotosFile = fs.open(numPhotosPath.c_str(), FILE_WRITE); // open new file for writing
  // if it fails to open this file in write mode... the same photo file will be overwritten again and again
  if (numPhotosFile) {
    numPhotosFile.print(newNumPhotosJson);
    numPhotosFile.close();
    return true;
  }
  numPhotosFile.close();
  return false;
}

bool checkAndCreateDir(String dirPath) {
  fs::FS &fs = SD_MMC;

  // Check for photo directory and create if necessary
  File dirFile = fs.open(dirPath.c_str(), FILE_READ);

  if (!dirFile) {
    #ifdef DEBUG
    Serial.println("No dir... creating");
    #endif
    dirFile.close();
    if (!fs.mkdir(dirPath.c_str())) {
      return false;
    };
  }
  else if (!dirFile.isDirectory()) {
    #ifdef DEBUG
    Serial.println("Photo dir is a file... removing and creating dir");
    #endif
    dirFile.close();
    if (!fs.remove(dirPath.c_str())) {
      return false;
    };
    if (!fs.mkdir(dirPath.c_str())) {
      return false;
    };
  }

  return true;
}

bool setNumPhotos(uint32_t numPhotos) {
  if (!checkAndCreateDir(configDir)) {
    #ifdef DEBUG
    Serial.println("Couldn't create config DIR");
    #endif
    // this is a major fail...
    return false;
  }
  
  fs::FS &fs = SD_MMC;
  File numPhotosFile = fs.open(numPhotosPath.c_str(), FILE_WRITE);
  
  // if this doesn't open correctly, not much we can do... will just have to try again next time

  // create numphotos file from scratch if necessary
  if (!numPhotosFile || numPhotosFile.isDirectory() || numPhotosFile.size() >32 ) {
    #ifdef DEBUG
    Serial.println("numphotos invalid... creating new");
    #endif
    numPhotosFile.close();
    deleteAndRecreateNumPhotos();
    numPhotosFile = fs.open(numPhotosPath.c_str(), FILE_WRITE); // re-open
  }


  // Create JSON doc and object
  JsonDocument numPhotosDoc;
  // JsonObject numPhotosObj = numPhotosDoc.as<JsonObject>();

  numPhotosDoc["numPhotos"] = numPhotos; // save to JSON

  #ifdef DEBUG
  Serial.print("New value stored in numPhotos doc:");
  Serial.println(numPhotosDoc["numPhotos"].as<int>());
  #endif


  // write JSON doc to file
  if (serializeJson(numPhotosDoc, numPhotosFile) == 0) {
    #ifdef DEBUG
    Serial.println("Failed to write new num photos file");
    #endif
    return false;
  }

  numPhotosFile.close();
  numPhotosDoc.clear();

  return true;
}

uint32_t getNumPhotos() {

  if (!checkAndCreateDir(configDir)) {
    #ifdef DEBUG
    Serial.println("Couldn't create config DIR");
    #endif
    // this is a major fail...
    return 0;
  }

  fs::FS &fs = SD_MMC;
  File numPhotosFile = fs.open(numPhotosPath.c_str(), FILE_READ);
  
  if (!numPhotosFile || numPhotosFile.isDirectory() || numPhotosFile.size() >32 ) {
    #ifdef DEBUG
    Serial.println("numphotos invalid... creating new");
    #endif
    numPhotosFile.close();
    deleteAndRecreateNumPhotos();
    return 0;
  }

  // create JsonDocument for deserialization
  JsonDocument numPhotosDoc;
  DeserializationError error = deserializeJson(numPhotosDoc, numPhotosFile);
  if (error) {
    #ifdef DEBUG
    Serial.println("deserializeJson() failed");
    #endif
    numPhotosFile.close();
    deleteAndRecreateNumPhotos();
    return 0;
  }

  uint32_t numPhotos = 0;
  JsonObject numPhotosObj = numPhotosDoc.as<JsonObject>();

  // grab current num photos
  if (numPhotosObj["numPhotos"].is<int>()) {
    #ifdef DEBUG
    Serial.println("Found a valid value for numphotos in json file");
    #endif
    if (numPhotosObj["numPhotos"] >= 0) {
      numPhotos = numPhotosObj["numPhotos"];
    }
  }

  numPhotosFile.close();
  numPhotosDoc.clear();
  return numPhotos; // will be 0 if value in json file is <0
}
