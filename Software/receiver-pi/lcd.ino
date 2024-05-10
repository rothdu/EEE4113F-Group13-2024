bool initLCD() {
  int16_t error;
  Wire.begin();
  Wire.beginTransmission(0x27);
  error = Wire.endTransmission();
  #ifdef DEBUG
  Serial.print("Error: ");
  Serial.print(error);
  #endif

  if (error != 0) {
    return false;
  }

  lcd.begin(16, 2);
  lcd.setBacklight(100);
  return true;
}



void showLoading() {
  if (millis() - prevLoadingMillis < 500) return; // only update every 500ms
  prevLoadingMillis = millis();
  lcd.setCursor(15, 0);
  lcd.print(loading[loadingIdx++]);
  loadingIdx = (loadingIdx == 4) ? 0 : loadingIdx;
  lcd.home();
}


void showMenu(int8_t scroll) {
  menuPos = ((menuPos + scroll)%rootMenuLen + rootMenuLen)%rootMenuLen; // double mod deals with negatives
  #ifdef DEBUG
  Serial.print("menuPos: ");
  Serial.println(menuPos);
  #endif
  lcd.home();
  lcd.clear();
  lcd.print(rootMenu[menuPos]);
  lcd.setCursor(15, 0);
  lcd.print(">");
  lcd.setCursor(0, 1);
  lcd.print(rootMenu[(menuPos+1)%rootMenuLen]);
  lcd.home();
  return;
}

