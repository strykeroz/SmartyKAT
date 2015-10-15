// ---------------------------------------------------------------------------------
// display current menu selection action on DMD
//
void updateDMD() {
  dmd.clearScreen(true);
  lcd.clear();
  DMDactive = true;

  switch (selectedMessage) {
    case NOflag:
      // clear/reset DMD
      // DMD & LCD already cleared...
      if (nextDMDrotate) nextDMDrotate = 0;
      DMDactive = false;
      break;

    case ALLflag:
      // Full course black (ALL)
      if (nextDMDrotate) nextDMDrotate = 0;
      displayDMDstring("ALL");
      writeBigString("ALL",5,1);
      break;

    case SCflag:
      // Full course yellow & Safety Car
      if (nextDMDrotate) nextDMDrotate = 0;
      displayDMDstring("S.C.");
      writeBigString("S.C.", 5, 1);
      break;

    case Chequer:
      // Chequered flag
      if (!nextDMDrotate) nextDMDrotate = millis() + DMDrotateTime;
      if (showingNumber) {
        for (int xval = 0; xval < 8; xval++) {
          dmd.drawFilledBox(xval * 8, (xval % 2) * 8, xval * 8 + 7, (xval % 2) * 8 + 7, GRAPHICS_NORMAL);
          dmd.drawFilledBox(xval * 8, ((xval + 1) % 2) * 8, xval * 8 + 7, ((xval + 1) % 2) * 8 + 7, GRAPHICS_NOR);
        }
        for (int xval = 0; xval < 8; xval++) {
          dmd.drawFilledBox(xval * 8, (xval % 2) * 8 + 15, xval * 8 + 7, (xval % 2) * 8 + 23, GRAPHICS_NORMAL);
          dmd.drawFilledBox(xval * 8, ((xval + 1) % 2) * 8 + 15, xval * 8 + 7, ((xval + 1) % 2) * 8 + 23, GRAPHICS_NOR);
        }
      }
      else {
        for (int xval = 0; xval < 8; xval++) {
          dmd.drawFilledBox(xval * 8, (xval % 2) * 8, xval * 8 + 7, (xval % 2) * 8 + 7, GRAPHICS_NOR);
          dmd.drawFilledBox(xval * 8, ((xval + 1) % 2) * 8, xval * 8 + 7, ((xval + 1) % 2) * 8 + 7, GRAPHICS_NORMAL);
        }
        for (int xval = 0; xval < 8; xval++) {
          dmd.drawFilledBox(xval * 8, (xval % 2) * 8 + 15, xval * 8 + 7, (xval % 2) * 8 + 23, GRAPHICS_NOR);
          dmd.drawFilledBox(xval * 8, ((xval + 1) % 2) * 8 + 15, xval * 8 + 7, ((xval + 1) % 2) * 8 + 23, GRAPHICS_NORMAL);
        }
      }

      // ...and echo it on the LCD too
      for (int y = 0; y < 4; y++) {
        lcd.setCursor(0,y);
        for (int x = 0; x < 10; x++) {
          if (y==0 || y==2)
            if (showingNumber) {
              lcd.print(' ');
              lcd.print(char(255));
            } else {
              lcd.print(char(255));
              lcd.print(' ');
            }
          else if (showingNumber) {
            lcd.print(char(255));
            lcd.print(' ');
          } else {
            lcd.print(' ');
            lcd.print(char(255));
          }
        }
      }
      break;

    case NUMBERonly:
      // Show just the car number
      if (nextDMDrotate) nextDMDrotate = 0;
      showCarNumber();
      break;

    case OneGo:
      // 1-GO
      if (nextDMDrotate) nextDMDrotate = 0;
      displayDMDstring("1-GO");
      writeBigString("1-GO",4,1);
    break;
    
    case SOUNDflag:
      // Sound flag for a competitor
      if (!nextDMDrotate) nextDMDrotate = millis() + DMDrotateTime;
      if (showingNumber) showCarNumber();
      else {
        displayDMDstring("SND");
        writeBigString("SND",5,1);
      }
      break;

    case TRANSflag:
      // Transponder flag for a competitor
      if (!nextDMDrotate) nextDMDrotate = millis() + DMDrotateTime;
      if (showingNumber) showCarNumber();
      else {
        // displayDMDstring("T");
        // replaced by this wider T
        int thisOffset = 9;
        dmd.drawFilledBox(thisOffset, 0, thisOffset + 17, 3, GRAPHICS_NORMAL); // top
        dmd.drawFilledBox(thisOffset + 6, 3, thisOffset + 10, 31, GRAPHICS_NORMAL); // vertical
        writeBigString("T", 8, 1);
      }
      break;

    case NETflag:
      // net flag
      if (!nextDMDrotate) nextDMDrotate = millis() + DMDrotateTime;
      if (showingNumber) showCarNumber();
      else {
        displayDMDstring("NET");
        writeBigString("NET",5, 1);
      }
      break;

    default:
      // there are no other options, so leave this hear as a catch-all to do nothing
      displayDMDstring("E01");
      writeBigString("Error:",0, 0);
      writeBigString("No Msg",0, 2);
      break;
  }
}

// ---------------------------------------------------------------------------------
// display thisWord, centred on the 2x2 DMD array
// uses calls to displayDMDchar to make double-tall/doublewide text up to 4 characters
//
void displayDMDstring(char * thisWord) {
  int startingChar[] = {24, 16, 8, 0};                                // what X offset on DMD for each possible string length
  int Xstart = startingChar[strlen(thisWord) - 1];                    // work out where to display the first character
  for (int i = 0; thisWord[i]; i++) displayDMDchar(thisWord[i], Xstart + i * 16);
}

// ---------------------------------------------------------------------------------
// display current competitor number, centred, on both LCD & DMD
//
void showCarNumber() {
  //int yVal = 0;
  int digits = 0;

  // if(!savedValues.numberFirst) yVal = 17;

  // work out car number length (max we care about is 3)
  while (carNumber[5 - digits] != ' ' && digits < 3) if (digits < 3) digits++;

  if (digits > 0) {
    // let's try this
    char * displayChars = carNumber + 6 - digits;
    displayDMDstring(displayChars);
    int test;
    while(displayChars[test]) test++;
    //test++;
    writeBigString(displayChars,(9-3*test/2), 1);
  }
  else {
    displayDMDstring("---");
    writeBigString("---",5,1);
  }
}

// ---------------------------------------------------------------------------------
// display a 2-DMD tall character at a particular location across the display
// tall characters are rendered using graphics primatives
//
void displayDMDchar(char thisChar, int thisOffset) {
  if(thisChar == 'O' || thisChar == 'o') thisChar = '0';                          // reuse zero glyph for O
  switch (thisChar) {
    case '0':
      dmd.drawFilledBox(thisOffset + 2, 0, thisOffset + 12, 3, GRAPHICS_NORMAL); // top
      dmd.drawFilledBox(thisOffset, 2, thisOffset + 3, 29, GRAPHICS_NORMAL);     // vertical-left
      dmd.drawFilledBox(thisOffset + 11, 2, thisOffset + 14, 29, GRAPHICS_NORMAL); // vertical-right
      dmd.drawFilledBox(thisOffset + 2, 28, thisOffset + 12, 31, GRAPHICS_NORMAL); // base bit
      // outside corner smoothing pixels
      dmd.writePixel(thisOffset + 1, 1, GRAPHICS_NORMAL, true );                // left top, outside
      dmd.writePixel(thisOffset + 13, 1, GRAPHICS_NORMAL, true);                // right top, outside
      dmd.writePixel(thisOffset + 1, 30, GRAPHICS_NORMAL, true );               // left bottom, outside
      dmd.writePixel(thisOffset + 13, 30, GRAPHICS_NORMAL, true );              // right bottom, outside
      // inside corner smoothing pixels
      dmd.writePixel(thisOffset + 4, 4, GRAPHICS_NORMAL, true );                // left top, inside
      dmd.writePixel(thisOffset + 10, 4, GRAPHICS_NORMAL, true );               // right top, inside
      dmd.writePixel(thisOffset + 4, 27, GRAPHICS_NORMAL, true );               // left bottom, inside
      dmd.writePixel(thisOffset + 10, 27, GRAPHICS_NORMAL, true );              // right bottom, inside
      break;

    case '1':
      dmd.drawFilledBox(thisOffset + 2, 0, thisOffset + 5, 3, GRAPHICS_NORMAL); // top
      dmd.drawFilledBox(thisOffset + 4, 2, thisOffset + 7, 28, GRAPHICS_NORMAL); // vertical
      dmd.drawFilledBox(thisOffset, 28, thisOffset + 11, 31, GRAPHICS_NORMAL); // base bit
      // outside corner smoothing pixels
      dmd.writePixel(thisOffset + 6, 1, GRAPHICS_NORMAL, true);                // right top, outside
      break;

    case '2':
      dmd.drawFilledBox(thisOffset, 0, thisOffset + 12, 3, GRAPHICS_NORMAL);   // top
      dmd.drawFilledBox(thisOffset + 11, 2, thisOffset + 14, 15, GRAPHICS_NORMAL); // vertical-1
      dmd.drawFilledBox(thisOffset + 2, 14, thisOffset + 12, 17, GRAPHICS_NORMAL); // middle horizontal
      dmd.drawFilledBox(thisOffset, 16, thisOffset + 3, 29, GRAPHICS_NORMAL);  // vertical-2
      dmd.drawFilledBox(thisOffset + 2, 28, thisOffset + 14, 31, GRAPHICS_NORMAL); // base bit
      // outside corner smoothing pixels
      dmd.writePixel(thisOffset + 13, 1, GRAPHICS_NORMAL, true);                // right top, outside
      dmd.writePixel(thisOffset + 1, 30, GRAPHICS_NORMAL, true);                // left bottom, outside
      dmd.writePixel(thisOffset + 1, 15, GRAPHICS_NORMAL, true);                // left middle, outside
      dmd.writePixel(thisOffset + 13, 16, GRAPHICS_NORMAL, true);               // right middle, outside
      // inside corner smoothing pixels
      dmd.writePixel(thisOffset + 10, 4, GRAPHICS_NORMAL, true );               // right top, inside
      dmd.writePixel(thisOffset + 4, 27, GRAPHICS_NORMAL, true );               // left bottom, inside
      dmd.writePixel(thisOffset + 10, 13, GRAPHICS_NORMAL, true );              // right middletop, inside
      dmd.writePixel(thisOffset + 4, 18, GRAPHICS_NORMAL, true );              // left middlelow, inside
      break;

    case '3':
      dmd.drawFilledBox(thisOffset, 0, thisOffset + 12, 3, GRAPHICS_NORMAL);   // top
      dmd.drawFilledBox(thisOffset + 11, 2, thisOffset + 14, 29, GRAPHICS_NORMAL); // vertical
      dmd.drawFilledBox(thisOffset + 4, 14, thisOffset + 14, 17, GRAPHICS_NORMAL); // middle horizontal
      dmd.drawFilledBox(thisOffset, 28, thisOffset + 12, 31, GRAPHICS_NORMAL); // base bit
      // outside corner smoothing pixels
      dmd.writePixel(thisOffset + 13, 1, GRAPHICS_NORMAL, true);                // right top, outside
      dmd.writePixel(thisOffset + 13, 30, GRAPHICS_NORMAL, true );              // right bottom, outside
      // inside corner smoothing pixels
      dmd.writePixel(thisOffset + 10, 4, GRAPHICS_NORMAL, true );               // right top, inside
      dmd.writePixel(thisOffset + 10, 27, GRAPHICS_NORMAL, true );              // right bottom, inside
      break;

    case '4':
      dmd.drawFilledBox(thisOffset, 0, thisOffset + 3, 15, GRAPHICS_NORMAL);   // vertical-short
      dmd.drawFilledBox(thisOffset + 2, 14, thisOffset + 14, 17, GRAPHICS_NORMAL); // middle horizontal
      dmd.drawFilledBox(thisOffset + 11, 0, thisOffset + 14, 31, GRAPHICS_NORMAL); // vertical-long
      // outside corner smoothing pixels
      dmd.writePixel(thisOffset + 1, 16, GRAPHICS_NORMAL, true );                // left top, outside
      break;

    case '5':
      dmd.drawFilledBox(thisOffset, 0, thisOffset + 13, 3, GRAPHICS_NORMAL);   // top
      dmd.drawFilledBox(thisOffset, 3, thisOffset + 3, 15, GRAPHICS_NORMAL);   // vertical-1
      dmd.drawFilledBox(thisOffset + 2, 14, thisOffset + 12, 17, GRAPHICS_NORMAL); // middle horizontal
      dmd.drawFilledBox(thisOffset + 11, 16, thisOffset + 14, 29, GRAPHICS_NORMAL); // vertical-2
      dmd.drawFilledBox(thisOffset, 28, thisOffset + 12, 31, GRAPHICS_NORMAL); // base bit
      // outside corner smoothing pixels
      dmd.writePixel(thisOffset + 1, 16, GRAPHICS_NORMAL, true);                // half left, outside
      dmd.writePixel(thisOffset + 13, 15, GRAPHICS_NORMAL, true );              // right middle, outside
      dmd.writePixel(thisOffset + 13, 30, GRAPHICS_NORMAL, true );              // right bottom, outside
      // inside corner smoothing pixels
      dmd.writePixel(thisOffset + 10, 18, GRAPHICS_NORMAL, true );              // right half, inside lower
      dmd.writePixel(thisOffset + 10, 27, GRAPHICS_NORMAL, true );               // right bottom, inside
      break;

    case '6':
      dmd.drawFilledBox(thisOffset + 2, 0, thisOffset + 13, 3, GRAPHICS_NORMAL); // top
      dmd.drawFilledBox(thisOffset, 2, thisOffset + 3, 29, GRAPHICS_NORMAL);     // vertical-long
      dmd.drawFilledBox(thisOffset + 3, 14, thisOffset + 12, 17, GRAPHICS_NORMAL); // middle horizontal
      dmd.drawFilledBox(thisOffset + 11, 16, thisOffset + 14, 29, GRAPHICS_NORMAL); // vertical-short
      dmd.drawFilledBox(thisOffset + 2, 28, thisOffset + 12, 31, GRAPHICS_NORMAL); // base bit
      // outside corner smoothing pixels
      dmd.writePixel(thisOffset + 1, 1, GRAPHICS_NORMAL, true );                // left top, outside
      dmd.writePixel(thisOffset + 1, 30, GRAPHICS_NORMAL, true );               // left bottom, outside
      dmd.writePixel(thisOffset + 13, 30, GRAPHICS_NORMAL, true );              // right bottom, outside
      dmd.writePixel(thisOffset + 13, 15, GRAPHICS_NORMAL, true );              // right middle, outside
      // inside corner smoothing pixels
      dmd.writePixel(thisOffset + 4, 4, GRAPHICS_NORMAL, true );                // left top, inside
      dmd.writePixel(thisOffset + 4, 18, GRAPHICS_NORMAL, true );                // left middlelow, inside
      dmd.writePixel(thisOffset + 10, 18, GRAPHICS_NORMAL, true );               // right middlelow, inside
      dmd.writePixel(thisOffset + 4, 27, GRAPHICS_NORMAL, true );               // left bottom, inside
      dmd.writePixel(thisOffset + 10, 27, GRAPHICS_NORMAL, true );              // right bottom, inside

      break;

    case '7':
      dmd.drawFilledBox(thisOffset, 0, thisOffset + 12, 3, GRAPHICS_NORMAL);   // top
      dmd.drawFilledBox(thisOffset + 11, 2, thisOffset + 14, 31, GRAPHICS_NORMAL); // vertical
      // outside corner smoothing pixels
      dmd.writePixel(thisOffset + 13, 1, GRAPHICS_NORMAL, true);                // right top, outside
      break;

    case '8':
      dmd.drawFilledBox(thisOffset + 2, 0, thisOffset + 12, 3, GRAPHICS_NORMAL); // top
      dmd.drawFilledBox(thisOffset, 2, thisOffset + 3, 29, GRAPHICS_NORMAL);     // vertical-left
      dmd.drawFilledBox(thisOffset + 3, 14, thisOffset + 10, 17, GRAPHICS_NORMAL); // middle horizontal
      dmd.drawFilledBox(thisOffset + 11, 2, thisOffset + 14, 29, GRAPHICS_NORMAL); // vertical-right
      dmd.drawFilledBox(thisOffset + 2, 28, thisOffset + 12, 31, GRAPHICS_NORMAL); // base bit
      // outside corner smoothing pixels
      dmd.writePixel(thisOffset + 1, 1, GRAPHICS_NORMAL, true );                // left top, outside
      dmd.writePixel(thisOffset + 13, 1, GRAPHICS_NORMAL, true);                // right top, outside
      dmd.writePixel(thisOffset + 1, 30, GRAPHICS_NORMAL, true );               // left bottom, outside
      dmd.writePixel(thisOffset + 13, 30, GRAPHICS_NORMAL, true );              // right bottom, outside
      // inside corner smoothing pixels
      dmd.writePixel(thisOffset + 4, 4, GRAPHICS_NORMAL, true );                // left top, inside
      dmd.writePixel(thisOffset + 10, 4, GRAPHICS_NORMAL, true );               // right top, inside
      dmd.writePixel(thisOffset + 4, 13, GRAPHICS_NORMAL, true );               // left middletop, inside
      dmd.writePixel(thisOffset + 10, 13, GRAPHICS_NORMAL, true );              // right middletop, inside
      dmd.writePixel(thisOffset + 4, 18, GRAPHICS_NORMAL, true );               // left middlelow, inside
      dmd.writePixel(thisOffset + 10, 18, GRAPHICS_NORMAL, true );              // right middlelow, inside
      dmd.writePixel(thisOffset + 4, 27, GRAPHICS_NORMAL, true );               // left bottom, inside
      dmd.writePixel(thisOffset + 10, 27, GRAPHICS_NORMAL, true );              // right bottom, inside
      break;

    case '9':
      dmd.drawFilledBox(thisOffset + 2, 0, thisOffset + 12, 3, GRAPHICS_NORMAL); // top
      dmd.drawFilledBox(thisOffset, 2, thisOffset + 3, 15, GRAPHICS_NORMAL);     // vertical-short
      dmd.drawFilledBox(thisOffset + 2, 14, thisOffset + 13, 17, GRAPHICS_NORMAL); // middle horizontal
      dmd.drawFilledBox(thisOffset + 11, 2, thisOffset + 14, 31, GRAPHICS_NORMAL); // vertical-long
      // outside corner smoothing pixels
      dmd.writePixel(thisOffset + 1, 1, GRAPHICS_NORMAL, true );                // left top, outside
      dmd.writePixel(thisOffset + 13, 1, GRAPHICS_NORMAL, true);                // right top, outside
      dmd.writePixel(thisOffset + 1, 16, GRAPHICS_NORMAL, true);                // half left, outside
      // inside corner smoothing pixels
      dmd.writePixel(thisOffset + 4, 4, GRAPHICS_NORMAL, true );                // left top, inside
      dmd.writePixel(thisOffset + 10, 4, GRAPHICS_NORMAL, true );               // right top, inside
      dmd.writePixel(thisOffset + 4, 13, GRAPHICS_NORMAL, true );               // left half, inside
      dmd.writePixel(thisOffset + 10, 13, GRAPHICS_NORMAL, true );              // right half, inside
      break;

    case 'A':
      dmd.drawFilledBox(thisOffset, 14, thisOffset + 3, 31, GRAPHICS_NORMAL);   // vertical-left
      dmd.drawFilledBox(thisOffset + 3, 16, thisOffset + 10, 19, GRAPHICS_NORMAL); // middle horizontal
      dmd.drawFilledBox(thisOffset + 11, 14, thisOffset + 14, 31, GRAPHICS_NORMAL); // vertical-right
      dmd.drawFilledBox(thisOffset + 6, 0, thisOffset + 7, 3, GRAPHICS_NORMAL); // top
      // diagonals, left side
      dmd.drawLine(thisOffset, 14, thisOffset + 6, 0, GRAPHICS_NORMAL);
      dmd.drawLine(thisOffset + 1, 14, thisOffset + 6, 1, GRAPHICS_NORMAL);
      dmd.drawLine(thisOffset + 2, 14, thisOffset + 7, 1, GRAPHICS_NORMAL);
      // diagonals, right side
      dmd.drawLine(thisOffset + 13, 14, thisOffset + 7, 0, GRAPHICS_NORMAL);
      dmd.drawLine(thisOffset + 12, 14, thisOffset + 7, 1, GRAPHICS_NORMAL);
      dmd.drawLine(thisOffset + 11, 14, thisOffset + 6, 1, GRAPHICS_NORMAL);
      break;

    case 'C':
      dmd.drawFilledBox(thisOffset + 2, 0, thisOffset + 13, 3, GRAPHICS_NORMAL); // top
      dmd.drawFilledBox(thisOffset, 2, thisOffset + 3, 29, GRAPHICS_NORMAL);     // vertical
      dmd.drawFilledBox(thisOffset + 2, 28, thisOffset + 13, 31, GRAPHICS_NORMAL); // bottom
      dmd.writePixel(thisOffset + 14, 1, GRAPHICS_NORMAL, 1);
      dmd.writePixel(thisOffset + 14, 2, GRAPHICS_NORMAL, 1);
      dmd.writePixel(thisOffset + 14, 29, GRAPHICS_NORMAL, 1);
      dmd.writePixel(thisOffset + 14, 30, GRAPHICS_NORMAL, 1);
      // outside corner smoothing pixels
      dmd.writePixel(thisOffset + 1, 1, GRAPHICS_NORMAL, true );                // left top, outside
      dmd.writePixel(thisOffset + 1, 30, GRAPHICS_NORMAL, true );               // left bottom, outside
      // inside corner smoothing pixels
      dmd.writePixel(thisOffset + 4, 4, GRAPHICS_NORMAL, true );                // left top, inside
      dmd.writePixel(thisOffset + 4, 27, GRAPHICS_NORMAL, true );               // left bottom, inside
      break;

    case 'D':
      dmd.drawFilledBox(thisOffset, 0, thisOffset + 12, 3, GRAPHICS_NORMAL);     // top
      dmd.drawFilledBox(thisOffset, 0, thisOffset + 3, 31, GRAPHICS_NORMAL);     // vertical-left
      dmd.drawFilledBox(thisOffset + 11, 2, thisOffset + 14, 29, GRAPHICS_NORMAL); // vertical-right
      dmd.drawFilledBox(thisOffset + 2, 28, thisOffset + 12, 31, GRAPHICS_NORMAL); // base bit
      // outside corner smoothing pixels
      dmd.writePixel(thisOffset + 13, 1, GRAPHICS_NORMAL, true);                // right top, outside
      dmd.writePixel(thisOffset + 13, 30, GRAPHICS_NORMAL, true );              // right bottom, outside
      // inside corner smoothing pixels
      dmd.writePixel(thisOffset + 10, 4, GRAPHICS_NORMAL, true );               // right top, inside
      dmd.writePixel(thisOffset + 10, 27, GRAPHICS_NORMAL, true );              // right bottom, inside
      break;

    case 'E':
      dmd.drawFilledBox(thisOffset, 0, thisOffset + 14, 3, GRAPHICS_NORMAL);   // top
      dmd.drawFilledBox(thisOffset, 3, thisOffset + 3, 28, GRAPHICS_NORMAL);   // vertical
      dmd.drawFilledBox(thisOffset + 3, 14, thisOffset + 10, 17, GRAPHICS_NORMAL); // middle horizontal
      dmd.drawFilledBox(thisOffset, 28, thisOffset + 14, 31, GRAPHICS_NORMAL); // base bit
      break;

    case 'F':
      dmd.drawFilledBox(thisOffset, 0, thisOffset + 14, 3, GRAPHICS_NORMAL);   // top
      dmd.drawFilledBox(thisOffset, 3, thisOffset + 3, 31, GRAPHICS_NORMAL);   // vertical
      dmd.drawFilledBox(thisOffset + 3, 14, thisOffset + 10, 17, GRAPHICS_NORMAL); // middle horizontal
      break;

    case 'G':
      dmd.drawFilledBox(thisOffset + 2, 0, thisOffset + 12, 3, GRAPHICS_NORMAL); // top
      dmd.drawFilledBox(thisOffset, 2, thisOffset + 3, 29, GRAPHICS_NORMAL);    // vertical
      dmd.drawFilledBox(thisOffset + 2, 29, thisOffset + 14, 31, GRAPHICS_NORMAL); // bottom
      dmd.drawFilledBox(thisOffset + 11, 17, thisOffset + 14, 29, GRAPHICS_NORMAL); // right vertical
      dmd.drawFilledBox(thisOffset + 8, 15, thisOffset + 12, 18, GRAPHICS_NORMAL); // horizontal
      dmd.drawFilledBox(thisOffset + 6, 15, thisOffset + 8, 19, GRAPHICS_NORMAL); // serif
      dmd.writePixel(thisOffset + 13, 1, GRAPHICS_NORMAL, 1);
      dmd.writePixel(thisOffset + 13, 2, GRAPHICS_NORMAL, 1);
      // outside corner smoothing pixels
      dmd.writePixel(thisOffset + 1, 1, GRAPHICS_NORMAL, true );                // left top, outside
      dmd.writePixel(thisOffset + 1, 30, GRAPHICS_NORMAL, true );               // left bottom, outside
      dmd.writePixel(thisOffset + 13, 16, GRAPHICS_NORMAL, true );              // right middle, outside
      // inside corner smoothing pixels
      dmd.writePixel(thisOffset + 4, 4, GRAPHICS_NORMAL, true );                // left top, inside
      dmd.writePixel(thisOffset + 4, 28, GRAPHICS_NORMAL, true );               // left bottom, inside
      break;

    case 'L':
      dmd.drawFilledBox(thisOffset, 28, thisOffset + 14, 31, GRAPHICS_NORMAL); // bottom
      dmd.drawFilledBox(thisOffset, 0, thisOffset + 3, 28, GRAPHICS_NORMAL);   // vertical
      break;

    case 'N':
      dmd.drawFilledBox(thisOffset, 0, thisOffset + 3, 31, GRAPHICS_NORMAL);   // vertical-left
      dmd.drawFilledBox(thisOffset + 11, 0, thisOffset + 14, 31, GRAPHICS_NORMAL); // vertical-right
      // diagonals
      dmd.drawLine(thisOffset + 2, 1, thisOffset + 11 , 28, GRAPHICS_NORMAL);
      dmd.drawLine(thisOffset + 2, 2, thisOffset + 11 , 29, GRAPHICS_NORMAL);
      dmd.drawLine(thisOffset + 2, 3, thisOffset + 11 , 30, GRAPHICS_NORMAL);
      dmd.drawLine(thisOffset + 2, 4, thisOffset + 11 , 31, GRAPHICS_NORMAL);
      dmd.drawLine(thisOffset + 2, 0, thisOffset + 11 , 27, GRAPHICS_NORMAL);
      break;

    case 'S':
      dmd.drawFilledBox(thisOffset + 2, 0, thisOffset + 13, 3, GRAPHICS_NORMAL); // top
      dmd.drawFilledBox(thisOffset, 2, thisOffset + 3, 15, GRAPHICS_NORMAL);     // vertical-left
      dmd.drawFilledBox(thisOffset + 2, 14, thisOffset + 12, 17, GRAPHICS_NORMAL); // middle horizontal
      dmd.drawFilledBox(thisOffset + 11, 16, thisOffset + 14, 29, GRAPHICS_NORMAL); // vertical-right
      dmd.drawFilledBox(thisOffset + 1, 28, thisOffset + 12, 31, GRAPHICS_NORMAL); // bottom
      dmd.writePixel(thisOffset + 14, 1, GRAPHICS_NORMAL, 1);
      dmd.writePixel(thisOffset + 14, 2, GRAPHICS_NORMAL, 1);
      dmd.writePixel(thisOffset, 29, GRAPHICS_NORMAL, 1);
      dmd.writePixel(thisOffset, 30, GRAPHICS_NORMAL, 1);
      // outside corner smoothing pixels
      dmd.writePixel(thisOffset + 1, 1, GRAPHICS_NORMAL, true );                // left top, outside
      dmd.writePixel(thisOffset + 13, 15, GRAPHICS_NORMAL, true );              // right middle, outside
      dmd.writePixel(thisOffset + 13, 30, GRAPHICS_NORMAL, true );              // right bottom, outside
      dmd.writePixel(thisOffset + 1, 16, GRAPHICS_NORMAL, true);                // half left, outside
      // inside corner smoothing pixels
      dmd.writePixel(thisOffset + 4, 4, GRAPHICS_NORMAL, true );                // left top, inside
      dmd.writePixel(thisOffset + 4, 13, GRAPHICS_NORMAL, true );               // left half, inside
      dmd.writePixel(thisOffset + 10, 27, GRAPHICS_NORMAL, true );              // right bottom, inside
      dmd.writePixel(thisOffset + 10, 18, GRAPHICS_NORMAL, true );              // right half, inside lower
      break;

    case 'T':
      dmd.drawFilledBox(thisOffset + 1, 0, thisOffset + 14, 3, GRAPHICS_NORMAL); // top
      dmd.drawFilledBox(thisOffset + 6, 3, thisOffset + 9, 31, GRAPHICS_NORMAL); // vertical
      break;

    case ':':
      dmd.drawFilledBox(thisOffset, 9, thisOffset + 2, 13, GRAPHICS_NORMAL);   // top vertical
      dmd.drawFilledBox(thisOffset, 17, thisOffset + 2, 21, GRAPHICS_NORMAL);   // bottom vertical
      break;

    case '.':
      dmd.drawFilledBox(thisOffset, 17, thisOffset + 2, 19, GRAPHICS_NORMAL);   // dot
      break;

    default:
      // display a dash for unknown characters
      dmd.drawFilledBox(thisOffset, 13, thisOffset + 10, 17, GRAPHICS_NORMAL);
  }
}


// ---------------------------------------------------------------------------------
// Change brightness of DMD
// From Trevor's notes: Most control is achieved at the lower range values
// (around 10 as a minimum useable, and 1024 giving 100% brightness, and 100
// giving about half brightness) with the scale looking like it is logarithmic
// rather than linear. I found the following steps to be an acceptable
// range: 10, 16, 25, 40, 64, 100, 160, 250, 400, 640, 1024.
// Rather than a direct calculation we'll map percentages to these values
//
void updateBrightness() {
  const int dimmerValues[] = {10, 16, 25, 40, 64, 100, 160, 250, 400, 640, 1024};
  int Brightness = dimmerValues[savedValues.DMDbrightness / 10];
  Timer1.setPwmDuty(PIN_DMD_nOE, Brightness);
}

// ---------------------------------------------------------------------------------
// Display SmartyKAT logo
//
void SplashScreen(int Xoffset, int Yoffset) {
  dmd.clearScreen(true);
  // row y=0
  dmd.writePixel(Xoffset + 26, Yoffset + 0, GRAPHICS_NORMAL, true);
  dmd.writePixel(Xoffset + 27, Yoffset + 0, GRAPHICS_NORMAL, true);
  // row y=1
  dmd.writePixel(Xoffset + 8, Yoffset + 1, GRAPHICS_NORMAL, true);
  dmd.writePixel(Xoffset + 16, Yoffset + 1, GRAPHICS_NORMAL, true);
  dmd.writePixel(Xoffset + 25, Yoffset + 1, GRAPHICS_NORMAL, true);
  dmd.writePixel(Xoffset + 26, Yoffset + 1, GRAPHICS_NORMAL, true);
  // row 2
  dmd.drawLine(Xoffset + 6, Yoffset + 2, Xoffset + 9, Yoffset + 2, GRAPHICS_NORMAL);
  dmd.drawLine(Xoffset + 15, Yoffset + 2, Xoffset + 17, Yoffset + 2, GRAPHICS_NORMAL);
  dmd.writePixel(Xoffset + 24, Yoffset + 2, GRAPHICS_NORMAL, true);
  dmd.writePixel(Xoffset + 25, Yoffset + 2, GRAPHICS_NORMAL, true);
  // rows 3 & 4
  dmd.drawFilledBox(Xoffset + 6, Yoffset + 3, Xoffset + 10, Yoffset + 4, GRAPHICS_NORMAL);
  dmd.drawFilledBox(Xoffset + 14, Yoffset + 3, Xoffset + 18, Yoffset + 4, GRAPHICS_NORMAL);
  dmd.drawFilledBox(Xoffset + 24, Yoffset + 3, Xoffset + 25, Yoffset + 4, GRAPHICS_NORMAL);
  // row 5
  dmd.writePixel(Xoffset + 25, Yoffset + 5, GRAPHICS_NORMAL, true);
  dmd.writePixel(Xoffset + 26, Yoffset + 5, GRAPHICS_NORMAL, true);
  // row 6
  dmd.drawLine(Xoffset + 7, Yoffset + 6, Xoffset + 11, Yoffset + 6, GRAPHICS_NORMAL);
  dmd.drawLine(Xoffset + 13, Yoffset + 6, Xoffset + 17, Yoffset + 6, GRAPHICS_NORMAL);
  dmd.drawLine(Xoffset + 26, Yoffset + 6, Xoffset + 27, Yoffset + 6, GRAPHICS_NORMAL);
  // row 7
  dmd.writePixel(Xoffset + 3, Yoffset + 7, GRAPHICS_NORMAL, true);
  dmd.drawLine(Xoffset + 8, Yoffset + 7, Xoffset + 10, Yoffset + 7, GRAPHICS_NORMAL);
  dmd.drawLine(Xoffset + 14, Yoffset + 7, Xoffset + 16, Yoffset + 7, GRAPHICS_NORMAL);
  dmd.writePixel(Xoffset + 21, Yoffset + 7, GRAPHICS_NORMAL, true);
  dmd.drawLine(Xoffset + 27, Yoffset + 7, Xoffset + 28, Yoffset + 7, GRAPHICS_NORMAL);
  // row 8
  dmd.writePixel(Xoffset + 4, Yoffset + 8, GRAPHICS_NORMAL, true);
  dmd.writePixel(Xoffset + 8, Yoffset + 8, GRAPHICS_NORMAL, true);
  dmd.writePixel(Xoffset + 15, Yoffset + 8, GRAPHICS_NORMAL, true);
  dmd.writePixel(Xoffset + 20, Yoffset + 8, GRAPHICS_NORMAL, true);
  dmd.drawLine(Xoffset + 28, Yoffset + 8, Xoffset + 29, Yoffset + 8, GRAPHICS_NORMAL);
  // row 9
  dmd.drawLine(Xoffset + 5, Yoffset + 9, Xoffset + 6, Yoffset + 9, GRAPHICS_NORMAL);
  dmd.writePixel(Xoffset + 12, Yoffset + 9, GRAPHICS_NORMAL, true);
  dmd.drawLine(Xoffset + 18, Yoffset + 9, Xoffset + 19, Yoffset + 9, GRAPHICS_NORMAL);
  dmd.drawLine(Xoffset + 29, Yoffset + 9, Xoffset + 30, Yoffset + 9, GRAPHICS_NORMAL);
  // row 10
  dmd.writePixel(Xoffset + 3, Yoffset + 10, GRAPHICS_NORMAL, true);
  dmd.drawLine(Xoffset + 11, Yoffset + 10, Xoffset + 13, Yoffset + 10, GRAPHICS_NORMAL);
  dmd.writePixel(Xoffset + 21, Yoffset + 10, GRAPHICS_NORMAL, true);
  dmd.drawLine(Xoffset + 29, Yoffset + 10, Xoffset + 30, Yoffset + 10, GRAPHICS_NORMAL);
  // row 11
  dmd.drawLine(Xoffset + 4, Yoffset + 11, Xoffset + 5, Yoffset + 11, GRAPHICS_NORMAL);
  dmd.writePixel(Xoffset + 8, Yoffset + 11, GRAPHICS_NORMAL, true);
  dmd.writePixel(Xoffset + 16, Yoffset + 11, GRAPHICS_NORMAL, true);
  dmd.drawLine(Xoffset + 18, Yoffset + 11, Xoffset + 20, Yoffset + 11, GRAPHICS_NORMAL);
  dmd.drawLine(Xoffset + 28, Yoffset + 11, Xoffset + 29, Yoffset + 11, GRAPHICS_NORMAL);
  // row 12
  dmd.writePixel(Xoffset + 6, Yoffset + 12, GRAPHICS_NORMAL, true);
  dmd.drawLine(Xoffset + 8, Yoffset + 12, Xoffset + 9, Yoffset + 12, GRAPHICS_NORMAL);
  dmd.drawLine(Xoffset + 15, Yoffset + 12, Xoffset + 16, Yoffset + 12, GRAPHICS_NORMAL);
  dmd.drawLine(Xoffset + 27, Yoffset + 12, Xoffset + 28, Yoffset + 12, GRAPHICS_NORMAL);
  // row 13
  dmd.drawLine(Xoffset + 9, Yoffset + 13, Xoffset + 15, Yoffset + 13, GRAPHICS_NORMAL);
  dmd.drawLine(Xoffset + 26, Yoffset + 13, Xoffset + 27, Yoffset + 13, GRAPHICS_NORMAL);
  // rows 14 & 15
  dmd.drawLine(Xoffset + 10, Yoffset + 14, Xoffset + 14, Yoffset + 14, GRAPHICS_NORMAL);
  dmd.drawFilledBox(Xoffset + 19, Yoffset + 14, Xoffset + 25, Yoffset + 15, GRAPHICS_NORMAL);
  dmd.writePixel(Xoffset + 26, Yoffset + 14, GRAPHICS_NORMAL, true);
}


