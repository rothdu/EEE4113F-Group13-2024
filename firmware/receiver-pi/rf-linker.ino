void waitForConnection() {
  while(!deviceConnected) {
    lcd.clear();
    pingDevice();
    showLoading();
  }
}

void pingDevice() {
  if (millis() - prevPingMillis > 200) {
    prevPingMillis = millis();
    digitalWrite(20, gpio20toggle);
    gpio20toggle = !gpio20toggle;
    lcd.home();
    lcd.print("CONNECTING");
  }
}