// function for configuring the camera... will have to go through and choose good config values
bool configESPCamera() {
  // Configure Camera parameters
 
  // Object to store the camera configuration parameters
  camera_config_t config;

  // Assign pin numbers 
  config.ledc_channel = LEDC_CHANNEL_0;
  config.ledc_timer = LEDC_TIMER_0;
  config.pin_d0 = Y2_GPIO_NUM;
  config.pin_d1 = Y3_GPIO_NUM;
  config.pin_d2 = Y4_GPIO_NUM;
  config.pin_d3 = Y5_GPIO_NUM;
  config.pin_d4 = Y6_GPIO_NUM;
  config.pin_d5 = Y7_GPIO_NUM;
  config.pin_d6 = Y8_GPIO_NUM;
  config.pin_d7 = Y9_GPIO_NUM;
  config.pin_xclk = XCLK_GPIO_NUM;
  config.pin_pclk = PCLK_GPIO_NUM;
  config.pin_vsync = VSYNC_GPIO_NUM;
  config.pin_href = HREF_GPIO_NUM;
  config.pin_sscb_sda = SIOD_GPIO_NUM;
  config.pin_sscb_scl = SIOC_GPIO_NUM;
  config.pin_pwdn = PWDN_GPIO_NUM;
  config.pin_reset = RESET_GPIO_NUM;

  config.xclk_freq_hz = 20000000;
  config.pixel_format = PIXFORMAT_JPEG; // Choices are YUV422, GRAYSCALE, RGB565, JPEG
 
  // Select lower framesize if the camera doesn't support PSRAM
  // AI thinker should support PSRAM so this is mostly for code reuseability
  if (psramFound()) {
    config.frame_size = FRAMESIZE_UXGA; // Resolution
    config.jpeg_quality = 10; //10-63 lower number = higher quality
    config.fb_count = 2;
  } else {
    config.frame_size = FRAMESIZE_SVGA;
    config.jpeg_quality = 12;
    config.fb_count = 1;
  }
 
  // Initialize the Camera
  esp_err_t err = esp_camera_init(&config);
  if (err != ESP_OK) {
    #ifdef DEBUG
    Serial.print("Camera init failed");
    #endif
    return false;
  }
 
  // Camera quality adjustments
  sensor_t * s = esp_camera_sensor_get();

  s->set_brightness(s, brightness);   // BRIGHTNESS (-2 to 2)
  s->set_contrast(s, contrast); // CONTRAST (-2 to 2)
  s->set_saturation(s, saturation);   // SATURATION (-2 to 2)
  s->set_special_effect(s, specialEffects);   // SPECIAL EFFECTS (0 - No Effect, 1 - Negative, 2 - Grayscale, 3 - Red Tint, 4 - Green Tint, 5 - Blue Tint, 6 - Sepia)
  s->set_whitebal(s, whiteBalance); // WHITE BALANCE (0 = Disable , 1 = Enable)
  s->set_awb_gain(s, awbGain); // AWB GAIN (0 = Disable , 1 = Enable)
  s->set_wb_mode(s, wbMode); // WB MODES (0 - Auto, 1 - Sunny, 2 - Cloudy, 3 - Office, 4 - Home)
  s->set_exposure_ctrl(s, exposureCtrl);   // EXPOSURE CONTROLS (0 = Disable , 1 = Enable)
  s->set_aec2(s, aec2);   // AEC2 (0 = Disable , 1 = Enable)
  s->set_ae_level(s, aeLevel);   // AE LEVELS (-2 to 2)
  s->set_aec_value(s, aecValue);   // AEC VALUES (0 to 1200)
  s->set_gain_ctrl(s, gainCtrl);   // GAIN CONTROLS (0 = Disable , 1 = Enable)
  s->set_agc_gain(s, agcGain);   // AGC GAIN (0 to 30)
  s->set_gainceiling(s, (gainceiling_t)0);   // GAIN CEILING (0 to 6)
  s->set_bpc(s, bpc);   // BPC (0 = Disable , 1 = Enable)
  s->set_wpc(s, wpc);   // WPC (0 = Disable , 1 = Enable)
  s->set_raw_gma(s, rawGma);   // RAW GMA (0 = Disable , 1 = Enable)
  s->set_lenc(s, lenc);   // LENC (0 = Disable , 1 = Enable)
  s->set_hmirror(s, hmirror);   // HORIZ MIRROR (0 = Disable , 1 = Enable)
  s->set_vflip(s, vflip);   // VERT FLIP (0 = Disable , 1 = Enable)
  s->set_dcw(s, dcw);   // DCW (0 = Disable , 1 = Enable)
  s->set_colorbar(s, colorbar);   // COLOR BAR PATTERN (0 = Disable , 1 = Enable)

  return true;
}

bool nextPhoto() {
  // Check for photo directory and create if necessary
  if (!checkAndCreateDir(photosDir)) {
    #ifdef DEBUG
    Serial.println("Failed to create photo DIR");
    #endif
    return false;
  }

  fs::FS &fs = SD_MMC;

  File photosDirFile = fs.open(photosDir.c_str());

  if (!photosDirFile) {
    #ifdef DEBUG
    Serial.println("Failed to open Photos DIR");
    #endif
    return false;
    // this is a major fail, because we should have just created the DIR...
  }

  if (!photosDirFile.openNextFile()) { // photos DIR is empty
    if (!checkAndCreateDir(configDir)) {
      #ifdef DEBUG
      Serial.println("Failed to create config DIR");
      #endif
      return false;
    }
    #ifdef DEBUG
    Serial.println("Empty photos directory - re-initialising num photos");
    #endif
    deleteAndRecreateNumPhotos(); // re-initialise the number of photos taken
  }
  photosDirFile.close();

  uint32_t nextNum = getNumPhotos() + 1;
  
  if (!setNumPhotos(nextNum)) {
    #ifdef DEBUG
    Serial.println("Failed to set next num photos");
    #endif
    return false;
  }

  return takeNewPhoto(photosDir, photoName);
}

// for taking a new photo
bool takeNewPhoto(String thisPhotoDir, String thisPhotoName) {
  // Take Picture with Camera
  if (!checkAndCreateDir(thisPhotoDir)) {
    #ifdef DEBUG
    Serial.println("Failed to create this photo DIR");
    #endif
    return false;
  }

  String path = thisPhotoDir + String("/") + thisPhotoName + String(".jpeg");
  // Setup frame buffer
  camera_fb_t  * fb = esp_camera_fb_get();
 
  if (!fb) {
    #ifdef DEBUG
    Serial.println("Camera capture failed");
    #endif
    return false;
  }
 
  // Save picture to microSD card
  fs::FS &fs = SD_MMC;
  File file = fs.open(path.c_str(), FILE_WRITE);
  if (!file) {
    #ifdef DEBUG
    Serial.println("Failed to open file in write mode");
    #endif
    return false;
  }

  file.write(fb->buf, fb->len); // payload (image), payload length

  // Close the file
  file.close();
 
  // Return the frame buffer back to the driver for reuse
  esp_camera_fb_return(fb);

  // return true (for success!)
  return true;
}