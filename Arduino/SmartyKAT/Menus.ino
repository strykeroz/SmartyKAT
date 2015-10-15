// ---------------------------------------------------------------------------------
// display updated view of main menu
//
void updateDisplay() {
  // refresh menu options that are visible
  char buffer[13];                                                    // for loading menu items out of PROGMEM
  char blankLine[13] = "            ";                                // empty menu line
  if (selectedMessage <= nowShowing) nowShowing = selectedMessage - 1; // new window with selected 2nd from top
  if (nowShowing < 0) nowShowing = 0;                                 // unless it's the first
  if (selectedMessage > nowShowing + 2) nowShowing = selectedMessage - 2; // new window with selected 2 from bottom
  // range checks
  if (nowShowing > menuOptions) nowShowing = menuOptions;
  if (nowShowing < 0) nowShowing = 0;

  for (int i = nowShowing; i < nowShowing + 4; i++) {
    lcd.setCursor(1, i - nowShowing);
    if (i < menuOptions) strcpy_P(buffer, (char*)pgm_read_word(&(menuItems[i])));
    else strcpy(buffer, blankLine);
    lcd.print(buffer);
  }

  // refresh menu selectors !!!
  for (int i = 0; i < 4; i++) {
    lcd.setCursor(0, i);
    if (i == selectedMessage - nowShowing) lcd.print(F(">"));
    else lcd.print(F(" "));
    lcd.setCursor(13, i);
    if (i == selectedMessage - nowShowing) lcd.print(F("<"));
    else lcd.print(F(" "));
  }

  // repaint car number box
  lcd.setCursor(15, 0);
  lcd.write(0);
  lcd.write(1);
  lcd.write(1);
  lcd.write(1);
  lcd.write(2);
  lcd.setCursor(15, 1);
  lcd.print(char(255));
  lcd.setCursor(15, 2);
  lcd.print(char(255));
  lcd.setCursor(19, 1);
  lcd.print(char(255));
  lcd.setCursor(19, 2);
  lcd.print(char(255));
  lcd.setCursor(15, 3);
  lcd.write(3);
  lcd.write(4);
  lcd.write(4);
  lcd.write(4);
  lcd.write(5);

  // repaint car number inside box
  lcd.setCursor(16, 1);
  lcd.print(F("CAR"));
  lcd.setCursor(16, 2);
  lcd.print(carNumber[3]);
  lcd.print(carNumber[4]);
  lcd.print(carNumber[5]);
}

// ---------------------------------------------------------------------------------
// format EEPROM and return all settings to factory defaults
// mostly useful if SmartyKAT moves to a new version with a new EEPROM record definition
//
void setDefaults() {
  lcd.clear();
  lcd.print(F("Setting defaults:   "));
  lcd.setCursor(0, 2);
  lcd.print(F("Formatting EEPROM..."));
  initialiseEEPROM();                                   // wipes EEPROM
  lcd.setCursor(0, 2);
  lcd.print(F("Formatting complete."));
  lcd.setCursor(0, 3);
  lcd.print(F("Saving default setup"));
  setEEPROMdefaults();                                  // save default values
  lcd.setCursor(0, 3);
  lcd.print(F("Process complete.   "));
  delay(500);
}

// ---------------------------------------------------------------------------------
// set backlight menu
// chooses and sets backlight balue.  Returns true if backlight value changed.
//
boolean backLightmenu() {
  boolean alldone = false;
  boolean keepNewBL = false;
  int newBLchoice = savedValues.LCDbacklight;
  char * BLoptions[4] = {
    "backlight always off", "backlight always on ", "stay on for 30 secs ", "stay on for 1 minute"
  };

  lcd.clear();
  lcd.print(F("Console Backlight"));
  // loop for input
  while (!alldone) {
    lcd.setCursor(0, 1);
    lcd.print(BLoptions[newBLchoice]);
    menuFooter();

    // check for hotkey
    switch (readHotkeys()) {
      case MCLRswitch:
        lcd.setCursor(0, 3);
        lcd.print(F("  ...CANCELLED...   "));
        alldone = true;
        break;
      case MUPswitch:
        newBLchoice++;
        if (newBLchoice > 3) newBLchoice = 0;
        break;
      case MDNswitch:
        newBLchoice--;
        if (newBLchoice < 0) newBLchoice = 3;
        break;
      case MSELswitch:
        lcd.setCursor(0, 3);
        lcd.print(F("    ...SAVING...    "));
        alldone = true;
        keepNewBL = true;
        break;
    }
  }

  // save or exit
  delay(500);                                   // long enough to read message
  if (keepNewBL) {
    savedValues.LCDbacklight = newBLchoice;
    writeSavedValues();
  }
  return keepNewBL;
}

// ---------------------------------------------------------------------------------
// Change brightness of DMD, & display brightness mode on DMD
// From Trevor's notes: Most control is achieved at the lower range values
// (around 10 as a minimum useable, and 1024 giving 100% brightness, and 100
// giving about half brightness) with the scale looking like it is logarithmic
// rather than linear. I found the following steps to be an acceptable
// range: 10, 16, 25, 40, 64, 100, 160, 250, 400, 640, 1024.
// Rather than a direct calculation we'll map percentages to these values
//
boolean setBrightnessmenu() {
  boolean alldone = false;
  boolean keepNew = false;
  boolean newValue = (savedValues.DMDbrightness == DAYMODE);
  lcd.clear();

  // loop for input
  while (!alldone) {
    updateLED();

    lcd.setCursor(0, 0);
    lcd.print(F("Set Panel Brightness"));
    lcd.setCursor(4, 1);
    if (newValue) lcd.print(F(" Day Mode "));
    else         lcd.print(F("Night Mode"));
    menuFooter();

    // check for hotkey
    switch (readHotkeys()) {
      case MCLRswitch:
        lcd.setCursor(0, 3);
        lcd.print(F("  ...CANCELLED...   "));
        alldone = true;
        break;
      case MUPswitch:
        newValue = !newValue;
        break;
      case MDNswitch:
        newValue = !newValue;
        break;
      case MSELswitch:
        lcd.setCursor(0, 3);
        lcd.print(F("    ...SAVING...    "));
        alldone = true;
        keepNew = true;
        break;
    }
  }

  // save or exit
  delay(500);                                   // long enough to read message
  if (keepNew) {
    if (newValue) savedValues.DMDbrightness = DAYMODE;
    else savedValues.DMDbrightness = NIGHTMODE;
    writeSavedValues();                        // remember the new value in EEPROM for later
    updateBrightness();                        // actually change the brightness
  }

  return keepNew;

}

// ---------------------------------------------------------------------------------
// Deactivate/activate status LED
//
boolean setPulseLED() {
  boolean alldone = false;
  boolean keepNew = false;
  boolean newValue = savedValues.usePulseLED;
  lcd.clear();

  // loop for input
  while (!alldone) {
    updateLED();

    lcd.setCursor(0, 0);
    lcd.print(F("Use Status LED:"));
    lcd.setCursor(2, 1);
    if (newValue) lcd.print(F("   Always   "));
    else         lcd.print(F("Startup only"));
    menuFooter();

    // check for hotkey
    switch (readHotkeys()) {
      case MCLRswitch:
        lcd.setCursor(0, 3);
        lcd.print(F("  ...CANCELLED...   "));
        alldone = true;
        break;
      case MUPswitch:
        newValue = !newValue;
        break;
      case MDNswitch:
        newValue = !newValue;
        break;
      case MSELswitch:
        lcd.setCursor(0, 3);
        lcd.print(F("    ...SAVING...    "));
        alldone = true;
        keepNew = true;
        break;
    }
  }

  // save or exit
  delay(500);                                   // long enough to read message
  if (keepNew) {
    savedValues.usePulseLED = newValue;
    writeSavedValues();                        // remember the new value in EEPROM for later
  }

  return keepNew;

}

// ---------------------------------------------------------------------------------
// setup menu
// 1. Reset to defaults
// 2. Set backlight
// 3. Set DMD brightness
// 4. Set status LED

void doSetupMenu() {
  int menuSelection = 0;
  boolean alldone = false;
  int activeHotkey;

  lcd.clear();

  while (!alldone) {
    updateLED();
    lcd.setCursor(1, 0);
    lcd.print(F("Reset to defaults"));
    lcd.setCursor(1, 1);
    lcd.print(F("Set backlight    "));
    lcd.setCursor(1, 2);
    lcd.print(F("Panel Brightness "));
    lcd.setCursor(1, 3);
    lcd.print(F("Set Status LED   "));

    for (int i = 0; i < 4; i++) {
      lcd.setCursor(0, i);
      if (i == menuSelection) lcd.print(F(">"));
      else lcd.print(F(" "));
      lcd.setCursor(19, i);
      if (i == menuSelection) lcd.print(F("<"));
      else lcd.print(F(" "));
    }

    activeHotkey = readHotkeys();
    switch (activeHotkey) {
      case MDNswitch:
        menuSelection++;
        if (menuSelection > 3) menuSelection = 0;
        break;
      case MUPswitch:
        menuSelection--;
        if (menuSelection < 0) menuSelection = 3;
        break;
      case MSELswitch:
        switch (menuSelection) {
          case 0:
            setDefaults();
            break;
          case 1:
            backLightmenu();
            break;
          case 2:
            setBrightnessmenu();
          default:
            setPulseLED();
            break;
        }
        lcd.clear();
        break;
      case MCLRswitch:
        alldone = true;
        break;
    }
  }
  lcd.clear();
  updateDisplay();
}


// ---------------------------------------------------------------------------------
// Take care of the repetitive stuff at bottom of menu
//
void menuFooter() {
  lcd.setCursor(1, 2);
  lcd.print(F("Up/Dn: change"));
  lcd.setCursor(0, 3);
  if (millis() % 2000 > 1000) lcd.print(F("Disp: select        "));
  else lcd.print(F("Clear/Reset: cancel "));
}
















