void initButtons() {
  pinMode(CONFIRM_PIN, INPUT_PULLUP);
  pinMode(UP_PIN, INPUT_PULLUP);
  pinMode(DOWN_PIN, INPUT_PULLUP);
  pinMode(CANCEL_PIN, INPUT_PULLUP);
}

void getNextAction() {
  
  if (millis() - prevButtonReadMillis < debounceTime) return; // only read if after debounce time
  prevButtonReadMillis = millis();

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

    // detect a "rising & falling edge" of instructionSent
  if (instructionSent != oldInstructionSent) {
    #ifdef DEBUG
    Serial.println("instructionSent edge");
    #endif
    if (instructionSent) showSent();
    else clearBottom();
    oldInstructionSent = instructionSent;
  }
  // detect a "rising edge" of instructionComplete
  if (instructionComplete != oldInstructionComplete) {
    #ifdef DEBUG
    Serial.println("instructionComplete edge");
    #endif
    if (instructionComplete) showMenu(0); // TODO: May have to change this up if showing actual actions after an operation
    oldInstructionComplete = instructionComplete;
  }

  if (!instructionComplete) {
    #ifdef DEBUG
    // Serial.println("Waiting for instruction to complete");
    #endif
    if (!instructionSent) {
    switch (action) {
      case CANCEL:
      currentInstruction = WAIT;
      instructionSent = true;
      oldInstructionSent = true;
      showMenu(0);
      break;
    }
    }
    action = NONE;
    showLoading();
    return;
  }



  switch (action) {
    case CONFIRM:
    #ifdef DEBUG
    Serial.println("Action confirm");
    #endif
    currentInstruction = intToInstruction(menuPos);
    instructionComplete = false;
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