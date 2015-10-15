// =================================================================================
// functions for storing and retrieving alarm settings

// ---------------------------------------------------------------------------------
// retrieve saved values from EEPROM into calibration data structure.  
// returns true if successful or false as error condition
//
boolean readSavedValues() {
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
  // now, read & reconstruct values
  eeprom_read_block((void*)&savedValues, (void*)EEPROMidx, sizeof(savedValues));

  // all good
  return true;
}

// ---------------------------------------------------------------------------------
// update value stored in EEPROM using current global values
//
void writeSavedValues() {
  uint8_t newIDX = EEPROMidx + sizeof(savedValues);                                  // calculate next EEPROM address 
  if(newIDX + sizeof(savedValues) > EEPROMmax) newIDX = 0;                           // wrap around if it's not going to fit
  eeprom_write_block((const void*)&savedValues, (void*)newIDX, sizeof(savedValues)); // write new values
  // overwrite previous values with blanks
  for(int i = 0; i < sizeof(savedValues); i++) eeprom_write_byte((uint8_t *)(EEPROMidx + i), EEPROMblank);
  EEPROMidx = newIDX;                                                                // remember new address for next time
}

// --------------------------------------------------------------------------------
// Overwrite all EEPROM locations with with the default value
//
void initialiseEEPROM() {
  for(int idx=0; idx < EEPROMmax; idx++) eeprom_write_byte((uint8_t *)idx, EEPROMblank);
  firstRun = true;
}

// --------------------------------------------------------------------------------
// set values to defaults and save
//
void setEEPROMdefaults() {
    savedValues.LCDbacklight = 1;                         // backlight always on by default
    savedValues.DMDbrightness = DAYMODE;                  // high brightness
    savedValues.USR[USR1] = NUMBERonly;
    savedValues.USR[USR2] = TRANSflag;
    savedValues.USR[USR3] = SOUNDflag;
    savedValues.USR[USR4] = Chequer;
    savedValues.usePulseLED = true;
    writeSavedValues();                                   // and...save
}






