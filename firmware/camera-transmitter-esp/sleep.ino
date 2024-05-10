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
  Serial.println("Sleeping with PIR and RF wakeup mode");
  esp_sleep_enable_ext1_wakeup(RF_BITMASK,ESP_EXT1_WAKEUP_ANY_HIGH);
  esp_sleep_enable_timer_wakeup(time*1e6); // seconds converted to microseconds
  delay(1000);
  esp_deep_sleep_start();
  #endif
}

void pirRFSleep() {
  #ifdef DEBUG
  Serial.println("Sleeping with PIR and RF wakeup mode");
  #endif
  esp_sleep_enable_ext1_wakeup(PIR_RF_BITMASK,ESP_EXT1_WAKEUP_ANY_HIGH);
  delay(1000);
  esp_deep_sleep_start();
}