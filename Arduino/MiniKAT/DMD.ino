// ---------------------------------------------------------------------------------
// display current car number on DMD
// if current car number is blanks this will be "ALL"
// reduced options for MiniKAT
//
void updateDMD() {
  dmd.clearScreen(true);
  dmd.selectFont(MyBigFont);                                            // big thick type

#ifdef DEBUG
  Serial.print("updateDMD(");
  Serial.print(selectedMessage);
  Serial.println(");");
#endif
  switch (selectedMessage) {
  case NOflag:
    // clear/reset DMD
    // already cleared...
    break;
  case NUMBERonly:
    // number only
    #ifdef DMD_FLASHER
    if(showingNumber) showCarNumber();                                  // else, already cleared
    #else
    showCarNumber();
    #endif
    break;
  case ALLflag:
    // Full course black (ALL)
    if(nextDMDrotate) nextDMDrotate = 0;
    dmd.drawString(0,0,"ALL",3,GRAPHICS_NORMAL);
    break;
  case SOUNDflag:
    // Sound flag for a competitor; S character is 10 px wide
    if(showingNumber) showCarNumber();
    else dmd.drawString(15-dmd.charWidth('S')/2,0,"S",2,GRAPHICS_NORMAL);
    break;
  case TRANSflag:
    // Transponder flag for a competitor; T character is 9 px wide
    if(showingNumber) showCarNumber(); 
    else dmd.drawString(15-dmd.charWidth('T')/2,0,"T",2,GRAPHICS_NORMAL);
    break;
  case SCflag:
    // Full course yellow - Safety Car
    dmd.drawString(0,0,"S.C.",4,GRAPHICS_NORMAL);
    break;
  default:
    // there are no other options, so leave this here as a catch-all to do nothing
    break;
  }
}

// ---------------------------------------------------------------------------------
// display current competitor number, centred
// or "ALL" if no competitor number
//
void showCarNumber() {
  //int yVal = 0;
  int digits = 0;
  int totalWidth = 0;

  // work out car number length (max we care about is 3)
  while(carNumber[5-digits] != ' ' && digits < 3) {
    totalWidth += dmd.charWidth(carNumber[5-digits]);                   // keep track of pixel width of car number
    if(digits < 3) digits++;                      
  }    

  // if there are no digits, we're displaying three dashes
  if(!digits) totalWidth = dmd.charWidth('-') * 3;

#ifdef DEBUG
  for(int i=0; i<6; i++) {
    Serial.print(i);
    Serial.print(':');
    Serial.print(carNumber[i]);
    Serial.print('\t');
  }
  Serial.println();
  Serial.print("digits=");
  Serial.print(digits);
  Serial.print("\ttotalWidth=");
  Serial.print(totalWidth);
#endif

  // work out X
  totalWidth /= 2;                                                      // half the string width
  int xVal = 15 - totalWidth;                                           // centred on 1-wide DMD

#ifdef DEBUG
  Serial.print("\txVal=");
  Serial.println(xVal);
#endif

  if(digits > 0) {
    // let's try this
    char * displayChars = carNumber + 6 - digits;
    dmd.drawString(xVal, 0, displayChars, digits, GRAPHICS_NORMAL);
  } 
  else { 
    dmd.drawString(0,0,"ALL",3,GRAPHICS_NORMAL);
    selectedMessage = ALLflag;
    nextDMDrotate = 0;                                                  // rotating display stops now
  }
}

// ---------------------------------------------------------------------------------
// Change brightness of DMD, & display percentage brightness.
// From Trevor's notes: Most control is achieved at the lower range values 
// (around 10 as a minimum useable, and 1024 giving 100% brightness, and 100 
// giving about half brightness) with the scale looking like it is logarithmic 
// rather than linear. I found the following steps to be an acceptable 
// range: 10, 16, 25, 40, 64, 100, 160, 250, 400, 640, 1024.
// Rather than a direct calculation we'll map percentages to these values
//
void updateBrightness() {
  const int dimmerValues[] = { 
    0, 16, 25, 40, 64, 100, 160, 250, 400, 640, 1024  };
  int Brightness = dimmerValues[savedValues.DMDbrightness / 10];
#ifdef DEBUG
  Serial.print("Index: ");
  Serial.print(savedValues.DMDbrightness / 10);
  Serial.print(", Brightness = ");
  Serial.print(Brightness);
  Serial.print("/1024\tsavedValues.DMDbrightness = ");
  Serial.print(savedValues.DMDbrightness);
  Serial.println("%");
  Serial.println("\nPossible values:");
  for(int idx=0; idx<=100; idx+=10) {
    Serial.print("Index: ");
    Serial.print(idx / 10);
    Serial.print(", Brightness = ");
    Serial.print(dimmerValues[idx / 10]);
    Serial.print("/1024\tidx = ");
    Serial.print(idx);
    Serial.println("%");
  }
#endif
  Timer1.setPwmDuty(PIN_DMD_nOE, Brightness);
  dmd.clearScreen(true);
  dmd.selectFont(System5x7);
  if(savedValues.DMDbrightness == DAYMODE)
  dmd.drawString(  2,  1, "Day  ", 5, GRAPHICS_NORMAL);
  else
  dmd.drawString(  2,  1, "Night", 5, GRAPHICS_NORMAL);
  dmd.drawString(  2,  9, "Mode", 5, GRAPHICS_NORMAL);
  /*
  if(savedValues.DMDbrightness==100) dmd.drawChar(2,  9, '0'+((savedValues.DMDbrightness%1000) /100),  GRAPHICS_NORMAL );   // hundreds
  dmd.drawChar( 7,  9, '0'+((savedValues.DMDbrightness%100)  /10),   GRAPHICS_NORMAL );   // tens
  dmd.drawChar( 12,  9, '0'+ (savedValues.DMDbrightness%10),         GRAPHICS_NORMAL );   // units
  dmd.drawChar( 17, 9, '%', GRAPHICS_NORMAL);
  */
  brightnessUpdateTime = millis();
}

// ---------------------------------------------------------------------------------
// Display SmartyKAT logo
//
void SplashScreen(int Xoffset, int Yoffset) {
  dmd.clearScreen(true);
  // row y=0
  dmd.writePixel(Xoffset+26,Yoffset+0,GRAPHICS_NORMAL, true);
  dmd.writePixel(Xoffset+27,Yoffset+0,GRAPHICS_NORMAL, true);
  // row y=1
  dmd.writePixel(Xoffset+8,Yoffset+1,GRAPHICS_NORMAL, true);
  dmd.writePixel(Xoffset+16,Yoffset+1,GRAPHICS_NORMAL, true);
  dmd.writePixel(Xoffset+25,Yoffset+1,GRAPHICS_NORMAL, true);
  dmd.writePixel(Xoffset+26,Yoffset+1,GRAPHICS_NORMAL, true);
  // row 2
  dmd.drawLine(Xoffset+6,Yoffset+2,Xoffset+9,Yoffset+2, GRAPHICS_NORMAL);
  dmd.drawLine(Xoffset+15,Yoffset+2,Xoffset+17,Yoffset+2, GRAPHICS_NORMAL);
  dmd.writePixel(Xoffset+24,Yoffset+2,GRAPHICS_NORMAL, true);
  dmd.writePixel(Xoffset+25,Yoffset+2,GRAPHICS_NORMAL, true);
  // rows 3 & 4
  dmd.drawFilledBox(Xoffset+6,Yoffset+3,Xoffset+10,Yoffset+4,GRAPHICS_NORMAL);
  dmd.drawFilledBox(Xoffset+14,Yoffset+3,Xoffset+18,Yoffset+4,GRAPHICS_NORMAL);
  dmd.drawFilledBox(Xoffset+24,Yoffset+3,Xoffset+25,Yoffset+4,GRAPHICS_NORMAL);
  // row 5
  dmd.writePixel(Xoffset+25,Yoffset+5,GRAPHICS_NORMAL, true);
  dmd.writePixel(Xoffset+26,Yoffset+5,GRAPHICS_NORMAL, true);  
  // row 6
  dmd.drawLine(Xoffset+7,Yoffset+6,Xoffset+11,Yoffset+6,GRAPHICS_NORMAL);
  dmd.drawLine(Xoffset+13,Yoffset+6,Xoffset+17,Yoffset+6,GRAPHICS_NORMAL);
  dmd.drawLine(Xoffset+26,Yoffset+6,Xoffset+27,Yoffset+6,GRAPHICS_NORMAL);
  // row 7
  dmd.writePixel(Xoffset+3,Yoffset+7,GRAPHICS_NORMAL,true);
  dmd.drawLine(Xoffset+8,Yoffset+7,Xoffset+10,Yoffset+7,GRAPHICS_NORMAL);
  dmd.drawLine(Xoffset+14,Yoffset+7,Xoffset+16,Yoffset+7,GRAPHICS_NORMAL);
  dmd.writePixel(Xoffset+21,Yoffset+7,GRAPHICS_NORMAL,true);
  dmd.drawLine(Xoffset+27,Yoffset+7,Xoffset+28,Yoffset+7,GRAPHICS_NORMAL);
  // row 8
  dmd.writePixel(Xoffset+4,Yoffset+8,GRAPHICS_NORMAL,true);
  dmd.writePixel(Xoffset+8,Yoffset+8,GRAPHICS_NORMAL,true);
  dmd.writePixel(Xoffset+15,Yoffset+8,GRAPHICS_NORMAL,true);
  dmd.writePixel(Xoffset+20,Yoffset+8,GRAPHICS_NORMAL,true);
  dmd.drawLine(Xoffset+28,Yoffset+8, Xoffset+29,Yoffset+8,GRAPHICS_NORMAL);
  // row 9
  dmd.drawLine(Xoffset+5,Yoffset+9,Xoffset+6,Yoffset+9,GRAPHICS_NORMAL);
  dmd.writePixel(Xoffset+12,Yoffset+9,GRAPHICS_NORMAL,true);
  dmd.drawLine(Xoffset+18,Yoffset+9,Xoffset+19,Yoffset+9,GRAPHICS_NORMAL);
  dmd.drawLine(Xoffset+29,Yoffset+9,Xoffset+30,Yoffset+9,GRAPHICS_NORMAL);
  // row 10
  dmd.writePixel(Xoffset+3,Yoffset+10,GRAPHICS_NORMAL,true);
  dmd.drawLine(Xoffset+11,Yoffset+10,Xoffset+13,Yoffset+10,GRAPHICS_NORMAL);
  dmd.writePixel(Xoffset+21,Yoffset+10,GRAPHICS_NORMAL, true);
  dmd.drawLine(Xoffset+29,Yoffset+10,Xoffset+30,Yoffset+10,GRAPHICS_NORMAL);
  // row 11
  dmd.drawLine(Xoffset+4,Yoffset+11,Xoffset+5,Yoffset+11,GRAPHICS_NORMAL);
  dmd.writePixel(Xoffset+8,Yoffset+11,GRAPHICS_NORMAL,true);
  dmd.writePixel(Xoffset+16,Yoffset+11,GRAPHICS_NORMAL,true);
  dmd.drawLine(Xoffset+18,Yoffset+11,Xoffset+20,Yoffset+11,GRAPHICS_NORMAL);  
  dmd.drawLine(Xoffset+28,Yoffset+11,Xoffset+29,Yoffset+11,GRAPHICS_NORMAL);  
  // row 12
  dmd.writePixel(Xoffset+6,Yoffset+12,GRAPHICS_NORMAL,true);  
  dmd.drawLine(Xoffset+8,Yoffset+12,Xoffset+9,Yoffset+12,GRAPHICS_NORMAL);
  dmd.drawLine(Xoffset+15,Yoffset+12,Xoffset+16,Yoffset+12,GRAPHICS_NORMAL);
  dmd.drawLine(Xoffset+27,Yoffset+12,Xoffset+28,Yoffset+12,GRAPHICS_NORMAL);  
  // row 13
  dmd.drawLine(Xoffset+9,Yoffset+13,Xoffset+15,Yoffset+13,GRAPHICS_NORMAL);
  dmd.drawLine(Xoffset+26,Yoffset+13,Xoffset+27,Yoffset+13,GRAPHICS_NORMAL);    
  // rows 14 & 15
  dmd.drawLine(Xoffset+10,Yoffset+14,Xoffset+14,Yoffset+14,GRAPHICS_NORMAL);
  dmd.drawFilledBox(Xoffset+19,Yoffset+14,Xoffset+25,Yoffset+15,GRAPHICS_NORMAL);
  dmd.writePixel(Xoffset+26,Yoffset+14,GRAPHICS_NORMAL, true);
}



