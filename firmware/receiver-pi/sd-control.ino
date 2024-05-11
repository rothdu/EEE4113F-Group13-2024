// init the SD card
bool initMicroSDCard() {
  // Start the MicroSD card
  #ifdef DEBUG
  Serial.println("Mounting MicroSD Card");
  #endif

  SPI.setRX(_MISO);
  Serial.println("1");
  SPI.setTX(_MOSI);
  Serial.println("2");
  SPI.setSCK(_SCK);
  #ifdef DEBUG
  Serial.println("Setup SPI for SD card");
  #endif
  // Check for mount failure
  if (!SD.begin(_CS)) {
    #ifdef DEBUG
    Serial.println("MicroSD Card Mount Failed");
    #endif
    return false;
  }

  #ifdef DEBUG
  Serial.println("Initialisation complete");
  #endif

  return true;
}

bool checkAndCreateDir(String dirPath) {
  File dirFile = SD.open(dirPath.c_str());

  if (!dirFile) {
    dirFile.close(); // this may not be necessary
    SD.mkdir(dirPath.c_str());
    return true;
  }

  if (!dirFile.isDirectory()) {
    dirFile.close();
    if(!SD.remove(dirPath.c_str())) {
      return false;
    }
    SD.mkdir(dirPath.c_str());
    return true;
  }

  return true;
}
