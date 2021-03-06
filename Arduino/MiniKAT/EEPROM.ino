// =================================================================================
// functions for storing and retrieving settings

// ---------------------------------------------------------------------------------
// retrieve saved values from EEPROM into calibration data structure.  
// returns true if successful or false as error condition
//
boolean readSavedValues() {
  DEBUG_PRINT(">> reading EEPROM")
  // determine if EEPROM has been initialised
  int datacount = 0;
  int idx = 0;
  for(int i = 0; i < EEPROMmax; i++) if(eeprom_read_byte((uint8_t *)i) != EEPROMblank) {
    datacount++;
    if(datacount==1) idx = i;
  }
  // ensure the data stored in EEPROM is the right number of bytes (drop out of function if not)
  if(datacount != sizeof(savedValues)) return false;                                 // error condition code
  
  // since idx is now verified, it's safe to store global index value
  EEPROMidx = idx;
  // !!!
  #ifdef DEBUG
  Serial.print("\tReading EEPROM @");
  Serial.println(idx);
  #endif
  // now, read & reconstruct values
  eeprom_read_block((void*)&savedValues, (void*)EEPROMidx, sizeof(savedValues));

  // all good
  return true;
}

// ---------------------------------------------------------------------------------
// update value stored in EEPROM using current global values
//
void writeSavedValues() {
  DEBUG_PRINT(">> writing to EEPROM")
  uint8_t newIDX = EEPROMidx + sizeof(savedValues);                                  // calculate next EEPROM address 
  if(newIDX + sizeof(savedValues) > EEPROMmax) newIDX = 0;                           // wrap around if it's not going to fit
  // !!!
  #ifdef DEBUG
  Serial.print("\tReading EEPROM @");
  Serial.println(newIDX);
  #endif
  eeprom_write_block((const void*)&savedValues, (void*)newIDX, sizeof(savedValues)); // write new values
  // overwrite previous values with blanks
  for(int i = 0; i < sizeof(savedValues); i++) eeprom_write_byte((uint8_t *)(EEPROMidx + i), EEPROMblank);
  EEPROMidx = newIDX;                                                                // remember new address for next time
}

// --------------------------------------------------------------------------------
// Overwrite all EEPROM locations with with the default value
//
void initialiseEEPROM() {
  DEBUG_PRINT(">> formatting EEPROM...")
  for(int idx=0; idx < EEPROMmax; idx++) eeprom_write_byte((uint8_t *)idx, EEPROMblank);
  firstRun = true;
  DEBUG_PRINT("formating done.")
}

// --------------------------------------------------------------------------------
// set values to defaults and save
//
void setEEPROMdefaults() {
    savedValues.DMDbrightness = DAYMODE;                  // full brightness
//    savedValues.USR[0] = NUMBERonly;                      // default programmable hotkeys
//    savedValues.USR[1] = SOUNDflag;
//    savedValues.USR[2] = TRANSflag;
//    savedValues.USR[3] = ALLflag;
    writeSavedValues();                                   // and...save
}






