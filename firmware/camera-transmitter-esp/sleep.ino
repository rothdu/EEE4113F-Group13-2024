uint8_t deepSleepAwakenPin() {
  return log(esp_sleep_get_ext1_wakeup_status())/log(2);
}

void deinitSleep() {
  rtc_gpio_deinit(GPIO_NUM_2);
  rtc_gpio_deinit(GPIO_NUM_4);
  rtc_gpio_deinit(GPIO_NUM_13);
  esp_sleep_disable_wakeup_source(ESP_SLEEP_WAKEUP_ALL);
}

void timerRFSleep(uint16_t time) {
  #ifdef DEBUG
  Serial.println("Sleeping with timer and RF wakeup mode");
  #endif
  esp_sleep_enable_ext1_wakeup(RF_BITMASK,ESP_EXT1_WAKEUP_ANY_HIGH);
  if ((double)SD_MMC.usedBytes()/(double)4250e6 > 0.99) {
    #ifdef DEBUG
    Serial.println("Out of storage, not enabling PIR wakeup");
    #endif
    delay(1000);
    esp_deep_sleep_start();
  }
  esp_sleep_enable_timer_wakeup(time*1e6); // seconds converted to microseconds
  delay(1000);



  esp_deep_sleep_start();
}

void pirRFSleep() {
  #ifdef DEBUG
  Serial.println("Sleeping with PIR and RF wakeup mode");
  #endif
  esp_sleep_enable_ext1_wakeup(PIR_RF_BITMASK,ESP_EXT1_WAKEUP_ANY_HIGH);
  delay(1000);
  esp_deep_sleep_start();
}