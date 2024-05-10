void initButtons() {
  pinMode(CONFIRM_PIN, INPUT_PULLUP);
  pinMode(UP_PIN, INPUT_PULLUP);
  pinMode(DOWN_PIN, INPUT_PULLUP);
  pinMode(CANCEL_PIN, INPUT_PULLUP);
}

void getNextAction() {
  
  if (millis() - prevButtonReadMillis < debounceTime) return; // only read if after debounce time
  prevButtonReadMillis = millis();
  #ifdef DEBUG
  Serial.print("Confirm status: ");
  Serial.println(digitalRead(CONFIRM_PIN));
  #endif

  if (digitalRead(CANCEL_PIN) == LOW) action = CANCEL;
  else if (digitalRead(CONFIRM_PIN) == LOW) action = CONFIRM;
  else if (digitalRead(DOWN_PIN) == LOW) action = DOWN;
  else if (digitalRead(UP_PIN) == LOW) action = UP;
  return;
}

void updateUI() {
  // #ifdef DEBUG
  // Serial.print("Action: ");
  // Serial.println(action);
  // #endif
  getNextAction();

  if (!instructionSent) {
    #ifdef DEBUG
    Serial.println("Waiting for instructionSent");
    #endif
    oldInstructionSent = instructionSent;
    switch (action) {
      case CANCEL:
      currentInstruction = WAIT;
      instructionSent = true;
      break;
    }
    action = NONE;
    showLoading();
    return;
  }

  if (instructionSent && !oldInstructionSent) {
    showMenu(0); // TODO: May have to change this up if showing actual actions after an operation
    oldInstructionSent = instructionSent;
  }

  switch (action) {
    case CONFIRM:
    #ifdef DEBUG
    Serial.println("Action confirm");
    #endif
    currentInstruction = intToInstruction(menuPos);
    instructionSent = false;
    break;

    case UP:
    showMenu(1);
    break;;

    case DOWN:
    showMenu(-1);
    break;
  }
  action = NONE;
  return;
}