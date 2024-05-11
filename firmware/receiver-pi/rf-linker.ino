void waitForConnection() {
  while(!deviceConnected) {
    pingDevice();
  }
}

void pingDevice() {
  if (millis() - prevPingMillis > 200) {
    prevPingMillis = millis();
    digitalWrite(20, gpio20toggle);
    gpio20toggle = !gpio20toggle;
  }
}