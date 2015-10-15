
// ---------------------------------------------------------------------------------
// set the intensity of the pulse LED based on the current millis() clock value
// and update the state of the LCD backlight as required
//
void updateLED() {
  unsigned long backlightTimeout[2] = {30000L, 60000L};
  // pulse LED
  if (savedValues.usePulseLED) {
    if (!DMDactive) {
      float pulseIntensity = (exp(sin(millis() / 1000.0 * PI)) - 0.36787944) * 108.0;
      analogWrite(pulseLED, pulseIntensity);
    } else digitalWrite(pulseLED, true);
  } else digitalWrite(pulseLED, false);

  // LCD backlight
  // backlight setting, 0=always off; 1=always on; 2=30, 3=60sec after last keypress
  switch (savedValues.LCDbacklight) {
    case 0:
      lcd.noBacklight();
      break;
    case 1:
      lcd.backlight();
      break;
    default:
      if (millis() - backlightTimeout[savedValues.LCDbacklight - 2] > LCDbacklightTimer) lcd.noBacklight();
      else lcd.backlight();
      break;
  }
}

// ---------------------------------------------------------------------------------
// returns a hotkey found that's pressed but wasn't earlier (only the last one found,
// there is no test for multiple keypresses).  Else returns -1 if none have changed.
// Simple software debouncing used, and updates currently recorded values across all hotkeys
//
int readHotkeys() {
  int activeKey = NOswitch;
  // remember: internal pull-ups make hotkeys true by default
  for (int i = 0; i < switchCount; i++) {
    if (!mcp.digitalRead(i)) {                                               // something has been pressed
      delay(5);                                                              //
      if (!mcp.digitalRead(i)) {                                             // and it's still pressed after debounce retry
        if (lastTimePressed[i] + repeatTime <= millis() || lastTimePressed[i] == 0) {
          activeKey = i;                                                     // keep for return value only if it's a new key press
          lastTimePressed[i] = millis();                                     // remember new button press time
        }
      }
    }
    else lastTimePressed[i] = 0;                                           // button has been released therefore is TRUE/HIGH
  }
  return activeKey;                                  // will still be -1 if none were pressed
}

// --------------------------------------------------------------------------------------------
// Display a progress bar graph based on the relative values of progress and target passed
// then pause for waitTime milliseconds
//
void graphProgress(int LCDrow, int progress, int target, unsigned long waitTime) {
  lcd.setCursor(0, LCDrow);
  lcd.print(F("["));
  for (int i = 1; i <= progress / (target / 14.0); i++) lcd.print(char(255));
  for (int i = 0; i < 14 - progress / (target / 14.0); i++) lcd.print(F("."));
  lcd.print(F("]"));
  float percentVal = (progress) * 100.0;
  percentVal /= target;
  if (percentVal < 10) lcd.print(F(" "));             // space padded to right justified
  if (percentVal < 100 ) lcd.print(F(" "));
  lcd.print(percentVal, 0);
  lcd.print(F("%"));

  // hang for a bit, maybe
  unsigned long waitUntil = millis() + waitTime;
  while (millis() < waitUntil);
}

// --------------------------------------------------------------------------------------------
// fill the line of the LCD specified by displayLine (0, 1) with spaces
// leaves cursor at the start of the blank line
//
void lcdClearLine(int displayLine) {
  lcd.setCursor(0, displayLine);
  for (int i = 0; i < 20; i++) lcd.print(F(" ")); // !!! change back to 15 if using 1602 display
  lcd.setCursor(0, displayLine);
}






