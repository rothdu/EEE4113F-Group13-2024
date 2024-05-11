void initWiFi() {
  WiFi.mode(WIFI_AP);

  WiFi.softAP(ssid, password);

  while(WiFi.status() != WL_CONNECTED) {
    delay(300);
  }
  return;
}

void setupHandlers() {

  // handler for root
  server.on("/", HTTP_GET, [](AsyncWebServerRequest * request){
    handleRoot(request);
  });

  // handler for upload_image endpoint (for posting images)
  server.on(
    "/upload_photo",
    HTTP_PUT,
    [](AsyncWebServerRequest * request){// may need to call a separate handler function here
    },
    NULL,
    [](AsyncWebServerRequest * request, uint8_t *data, size_t len, size_t index, size_t total) {
      handlePhotoUpload(request, data, len, index, total);
  });

  server.on(
    "/upload_sample",
    HTTP_PUT,
    [](AsyncWebServerRequest * request){// may need to call a separate handler function here
    },
    NULL,
    [](AsyncWebServerRequest * request, uint8_t *data, size_t len, size_t index, size_t total) {
      handlePhotoUpload(request, data, len, index, total, true); // passing true indicates that it is a sample photo
  });

  server.on(
    "/metadata",
    HTTP_PUT,
    [](AsyncWebServerRequest * request){// may need to call a separate handler function here
    },
    NULL,
    [](AsyncWebServerRequest * request, uint8_t *data, size_t len, size_t index, size_t total) {
      handleMetadataUpload(request, data, len, index, total);
  });

  server.on(
    "/next_instruction",
    HTTP_GET,
    [](AsyncWebServerRequest * request){
      handleNextInstruction(request);
  });
  
  server.on(
    "/update_config",
    HTTP_GET,
    [](AsyncWebServerRequest * request){
      handleUpdateConfig(request);
  });

  server.onNotFound(handleNotFound);

  server.begin();
}

bool handleUpdateConfig(AsyncWebServerRequest *request) {
  prevRequestMillis = millis();
  
  #ifdef DEBUG
  Serial.println("Updating config");
  #endif
  
  if (! request->hasHeader("Device-ID")) {
    #ifdef DEBUG
    Serial.println("Missing Device-ID");
    #endif
    request->send(400);
    return false;
  }

  String deviceID = request->getHeader("Device-ID")->value();
  String configFilePath = String("/") + deviceID + String("/config.json");
  File configFile = SD.open(configFilePath, FILE_READ);

  if (!configFile) {
    #ifdef DEBUG
    Serial.println("Error opening config file");
    #endif
    request->send(404);
    return false;
  }
  // read file
  String responseJson = configFile.readString();
  configFile.close();


  // send response
  request->send(200, "application/json", responseJson);
  return true;
}

bool handleNextInstruction(AsyncWebServerRequest *request) {
  prevRequestMillis = millis();
  deviceConnected = true;
  instructionSent = true;
  request->send(200, "text/plain", instructions[currentInstruction]);
  // good request, send next instruction.
  // TODO: Could add some sort of check of who is sending the request?
  instructionComplete = (currentInstruction == WAIT);

  currentInstruction = WAIT;

  #ifdef DEBUG
  Serial.print("Sending instruction: ");
  Serial.println(instructions[currentInstruction]);
  #endif
  return true;
}

bool handlePhotoUpload(AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total, bool sample) {
  prevRequestMillis = millis();
  if(! (request->hasHeader("Content-Type") && request->hasHeader("Content-Length") && request->hasHeader("Device-ID") && request->hasHeader("Photo-Name"))) {
    #ifdef DEBUG
    Serial.println("Missing request headers");
    #endif
    receivedFile.close();
    request->send(400);
    return false;
  }

  String contentType = request->getHeader("Content-Type")->value();

  if (contentType != String("image/jpeg")) {
    #ifdef DEBUG
    Serial.println("Not image/jpeg");
    #endif
    receivedFile.close();
    request->send(400);
    return false;
  }


  if (index == 0) { // first part
    String deviceID = request->getHeader("Device-ID")->value();
    String photoName = request->getHeader("Photo-Name")->value();
    checkAndCreateDir(String("/") + deviceID);
    String receivedFilePath = String("/") + deviceID + String("/") + photoName;
    SD.remove(receivedFilePath.c_str()); // remove and overwrite
    receivedFile = SD.open(receivedFilePath.c_str(), FILE_WRITE);
  }

  if(!receivedFile) {
    #ifdef DEBUG
    Serial.println("Problem with receivedFile file");
    #endif
    request->send(500);
    return false;
  }
  
  receivedFile.write(data, len);

  if (index + len == total) { // at the end of the file
    #ifdef DEBUG
    Serial.println("Good image upload");
    #endif
    receivedFile.close();
    request->send(201);
  }

  return true;
}

bool handleMetadataUpload(AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total) {
  prevRequestMillis = millis();
  if(! (request->hasHeader("Content-Type") && request->hasHeader("Content-Length") && request->hasHeader("Device-ID"))) {
    #ifdef DEBUG
    Serial.println("Missing request headers");
    #endif
    receivedFile.close();
    request->send(400);
    return false;
  }

  String contentType = request->getHeader("Content-Type")->value();

  if (contentType != String("application/json")) {
    #ifdef DEBUG
    Serial.println("Not application/json");
    #endif
    receivedFile.close();
    request->send(400);
    return false;
  }

  if (index == 0) { // first part
    String deviceID = request->getHeader("Device-ID")->value();
    String dirName = deviceID;
    String receivedFileName = "metadata.json";
    checkAndCreateDir(String("/") + dirName);
    String receivedFilePath = String("/") + deviceID + String("/") + receivedFileName;
    SD.remove(receivedFilePath.c_str()); // remove and overwrite
    receivedFile = SD.open(receivedFilePath.c_str(), FILE_WRITE);
  }

  if(!receivedFile) {
    #ifdef DEBUG
    Serial.println("Problem with receivedFile file");
    #endif
    request->send(500);
    return false;
  }
  
  receivedFile.write(data, len);

  if (index + len == total) { // at the end of the file
    #ifdef DEBUG
    Serial.println("Good metadata upload");
    #endif
    receivedFile.close();
    request->send(201);
  }

  return true;
}

void handleNotFound(AsyncWebServerRequest *request) {
  prevRequestMillis = millis();
  #ifdef DEBUG
  Serial.println("Request not found");
  #endif
  request->send(404);
  return;
}

void handleRoot(AsyncWebServerRequest *request) {
  prevRequestMillis = millis();
  #ifdef DEBUG
  Serial.println("Serving hello world root");
  #endif
  request->send(200, "text/html", "<title>Hello World</title><p>Hello World</p>");
  return;
}