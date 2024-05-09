bool initWiFi() {
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
  Serial.println();
  #endif

  return true;
}

void onWiFiEvent(WiFiEvent_t event) {
  switch (event) {
    case SYSTEM_EVENT_STA_DISCONNECTED:
      #ifdef DEBUG
      Serial.print("WiFi begin failed ");
      #endif
      // major wifi fail... figure out how to handle this later
      // probably a freeRTOS task which will be killed if WiFi reconnects?
      break;
    case SYSTEM_EVENT_STA_GOT_IP:
      #ifdef DEBUG
      Serial.print("WiFi begin succeeded ");
      #endif
      // Connected successfully
      break;
    default:
      break;
       // Display additional events???      
  }
}

uint32_t sendAllPhotos() {
  fs::FS &fs = SD_MMC;
  uint32_t photoNum = getNumPhotos();
  String thisPhotoName;
  while (photoNum >= 1) { // uint so always >= 0... so test for equality, last image should be the 0th one
    thisPhotoName = photoName + String(photoNum) + String(".jpeg");
    int16_t responseCode = sendPhoto(photosDir, thisPhotoName, photoNum);
    #ifdef DEBUG
    Serial.print("Code: ");
    Serial.print(responseCode);
    Serial.print(" Num: ");
    Serial.println(photoNum);
    #endif
    uint8_t a;
    for (a = 0; a < numAttempts; a++) {
      if (responseCode == HTTP_CODE_OK) {
        fs.remove(thisPhotoName);
        break;
      }
    }
    if (a == numAttempts) { // means a failure
      break; // exit the while loop, unable to finish sending photos
    }
    photoNum--;
  }
  setNumPhotos(photoNum);
  return photoNum;

}

/* ACTUAL REQUESTS */

int16_t sendPhoto(String thisPhotoDir, String thisPhotoName, uint32_t photoNum) {

  String filePath = thisPhotoDir + String("/") + thisPhotoName;
  // double check there are no problems with the file
  fs::FS &fs = SD_MMC;
  File file = fs.open(filePath.c_str());
  if (!file || file.isDirectory()) {
    #ifdef DEBUG
    Serial.println("Failed to open image path");
    #endif
    return 0;
  }

  // try to connect to server
  
  if (!http.begin(client, serverAddress + "/upload_photo")) {
    #ifdef DEBUG
    Serial.println("Failed to connect to server");
    #endif
    return 0;
  }

  // find file size
  uint32_t fileLen = file.size();

  // add HTTP headers
  http.addHeader("Content-Type", "image/jpeg");
  http.addHeader("Content-Length", String(fileLen));
  http.addHeader("Photo-Name", thisPhotoName);
  http.addHeader("Device-ID", deviceID);
  http.addHeader("Photo-Number", String(photoNum));
  // create buffer on PSRAM to store file
  uint8_t* fileBuf = (uint8_t*)ps_malloc(fileLen);

  // read the file
  file.read(fileBuf, fileLen);
  // close the file
  file.close();
  // Send file via POST
  int16_t responseCode = http.POST(fileBuf, fileLen);
  free(fileBuf);
  http.end();
  return responseCode;
}

int16_t sendSample(String thisPhotoDir, String thisPhotoName) {

  String filePath = thisPhotoDir + String("/") + thisPhotoName;
  // double check there are no problems with the file
  fs::FS &fs = SD_MMC;
  File file = fs.open(filePath.c_str());
  if (!file || file.isDirectory()) {
    #ifdef DEBUG
    Serial.println("Failed to open image path");
    #endif
    return 0;
  }

  // try to connect to server
  
  if (!http.begin(client, serverAddress + "/upload_sample")) {
    #ifdef DEBUG
    Serial.println("Failed to connect to server");
    #endif
    return 0;
  }

  // find file size
  uint32_t fileLen = file.size();

  // add HTTP headers
  http.addHeader("Content-Type", "image/jpeg");
  http.addHeader("Content-Length", String(fileLen));
  http.addHeader("Photo-Name", thisPhotoName);
  http.addHeader("Device-ID", deviceID);
  // create buffer on PSRAM to store file
  uint8_t* fileBuf = (uint8_t*)ps_malloc(fileLen);

  // read the file
  file.read(fileBuf, fileLen);
  // close the file
  file.close();
  // Send file via POST
  int16_t responseCode = http.POST(fileBuf, fileLen);
  free(fileBuf);
  http.end();
  return responseCode;
}

// TODO overwrites the config without checking for validity... could change at some point for robustness
int16_t updateConfig() {
  if (!http.begin(client, serverAddress + "/update_config")) {
    #ifdef DEBUG
    Serial.println("Failed to connect to server");
    #endif
    return 0;
  }

  http.addHeader("Device-ID", deviceID);

  // inform the thing to collect headers
  const char *headerKeys[] = {"Content-Type"};
  // TODO: Handle different capitalisation
  http.collectHeaders(headerKeys, 1);
  int16_t responseCode = http.GET();

  if (responseCode != HTTP_CODE_OK) {
    #ifdef DEBUG
    Serial.print("Bad response code: ");
    Serial.println(responseCode);
    #endif
    http.end();
    return responseCode;
  }
  // get length of document (is -1 when Server sends no Content-Length header)
  int len = http.getSize();

  if (len == -1) { // no or bad content-length
    #ifdef DEBUG
    Serial.println("Bad or missing Content-Length");
    #endif
    http.end();
    return 0;
  }

  #ifdef DEBUG
  Serial.print("Num headers: ");
  Serial.println(http.headers());
  for (int i = 0; i<http.headers(); i++) {
    Serial.println(http.header(i));
  }
  #endif

  if (!http.hasHeader("Content-Type")) { // no content-type
    #ifdef DEBUG
    Serial.println("Missing Content-Type");
    #endif
    http.end();
    return 0;
  }

  if (http.header("Content-Type") != String("application/json")) { // content type wrong
    #ifdef DEBUG
    Serial.println("Wrong Content-Type");
    #endif
    http.end();
    return 0;
  }

  fs::FS &fs = SD_MMC;
  // create buffer for read
  uint8_t* configBuf = (uint8_t*)malloc(len);

  // get tcp stream
  WiFiClient *stream = http.getStreamPtr();

  // read data from server
  uint32_t c = stream->readBytes(configBuf, len);

  // remove and overwrite config file
  fs.remove(configFilePath.c_str());
  File configFile = fs.open(configFilePath.c_str(), FILE_WRITE); // open file
  configFile.write(configBuf, len);
  configFile.close(); // close file

  free(configBuf); // de-allocate configBuf memory
  http.end();
  return responseCode;
}

int16_t requestInstruction() {
  if (!http.begin(client, serverAddress + "/next_instruction")) {
    #ifdef DEBUG
    Serial.println("Failed to connect to server");
    #endif
    return 0;
  }

  http.addHeader("Device-ID", deviceID);

  // inform the thing to collect headers
  const char *headerKeys[] = {"Content-Type"};
  // TODO: Handle different capitalisation
  http.collectHeaders(headerKeys, 1);
  int16_t responseCode = http.GET();

  if (responseCode != HTTP_CODE_OK) {
    #ifdef DEBUG
    Serial.print("Bad response code: ");
    Serial.println(responseCode);
    #endif
    http.end();
    return responseCode;
  }
  // get length of document (is -1 when Server sends no Content-Length header)
  int len = http.getSize();

  if (len == -1 || len > 32) { // no or bad content-length
    #ifdef DEBUG
    Serial.println("Bad or missing Content-Length");
    #endif
    http.end();
    return 0;
  }

  if (!http.hasHeader("Content-Type")) { // no content-type
    #ifdef DEBUG
    Serial.println("Missing Content-Type");
    #endif
    http.end();
    return 0;
  }

  if (http.header("Content-Type") != String("text/plain")) { // content type wrong
    #ifdef DEBUG
    Serial.println("Wrong Content-Type");
    #endif
    http.end();
    return 0;
  }

  String instructionStr = http.getString();

  http.end();
  if (instructionStr == String("wait")) currentInstruction = WAIT;
  else if (instructionStr == String("sendMetadata"))  currentInstruction = SEND_METADATA;
  else if (instructionStr == String("updateConfig")) currentInstruction = UPDATE_CONFIG;
  else if (instructionStr == String("sendPhotos")) currentInstruction = SEND_PHOTOS;
  else if (instructionStr == String("samplePhoto")) currentInstruction = SAMPLE_PHOTO;
  else if (instructionStr == String("exit")) currentInstruction = EXIT;
  else return 0;
  return responseCode;
}

int16_t sendMetadata() {
  if (!http.begin(client, serverAddress + "/metadata")) {
    #ifdef DEBUG
    Serial.println("Failed to connect to server");
    #endif
    return 0;
  }

  http.addHeader("Device-ID", deviceID);
  // Create JSON doc and object


  JsonDocument metadataDoc;
  // JsonObject numPhotosObj = numPhotosDoc.as<JsonObject>();

  metadataDoc["numPhotos"] = getNumPhotos(); // save to JSON
  metadataDoc["bytesUsed"] = SD_MMC.usedBytes();
  metadataDoc["bytesTotal"] = SD_MMC.totalBytes();

  String payload; // should be fairly short so can store in a String

  // write JSON doc to payload
  if (serializeJson(metadataDoc, payload) == 0) {
    #ifdef DEBUG
    Serial.println("Failed to create valid payload for metadata");
    #endif
    http.end();
    return 0;
  }
  metadataDoc.clear();
  int16_t responseCode = http.POST(payload);
  http.end();
  return responseCode;

}