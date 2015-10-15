// ---------------------------------------------------------------------------------
// BIG FONT (2-line) LCD CHARACTERS method by Adrian Jones, February 2015 
// (Initial idea http://www.instructables.com/id/Custom-Large-Font-For-16x2-LCDs/step5/Arduino-Sketch/)
// 
// Functions supporting 2 line characters
// 
// usage:  writeBigString("B G F N T", 0, 0);
//

// ---------------------------------------------------------------------------------
// Writes big character 'ch' to column x, row y; 
// Returns number of columns used by 'ch'
//
int writeBigChar(char ch, byte x, byte y) {
  if (ch < ' ' || ch > '_') return 0;                     // If outside table range, do nothing
  byte nb = 0;                                            // character byte counter
  byte bb[8];                                             // byte buffer for reading from PROGMEM
  for (byte bc = 0; bc < 8; bc++) {
    bb[bc] = pgm_read_byte( &bigChars[ch - ' '][bc] );    // read 8 bytes from PROGMEM
    if (bb[bc] != 0) nb++;
  }

  byte bc = 0;
  for (byte row = y; row < y + 2; row++) {
    for (byte col = x; col < x + nb / 2; col++ ) {
      lcd.setCursor(col, row);                            // move to position
      lcd.write(bb[bc++]);                                // write byte and increment to next
    }
    //    lcd.setCursor(col, row);
    //    lcd.write(' ');                                 // Write ' ' between letters
  }
  return nb / 2 - 1;                                      // returns number of columns used by char
}

// ---------------------------------------------------------------------------------
// writeBigString: writes out each letter of string
//
void writeBigString(char *str, byte x, byte y) {
  char c;
  while ((c = *str++))
    x += writeBigChar(c, x, y) + 1;
}


