#include <math.h>                                                     // maths for pulsing the LED
#include <avr/eeprom.h>                                                 // saving and retrieving settings between runs
#include <Wire.h>                                                       // I2C protocol for MCP23008 I/O extenders
#include <SPI.h>                                                        // for DMD display
// NOTE: REQUIRES dimmable modifications to DMD library, described http://forum.freetronics.com/viewtopic.php?f=26&t=5760
#include <DMD.h>                                                        // also, for DMD display
#include <TimerOne.h>                                                   // and THIS, is for the DMD display periodic refreshes
#include <MyBigFont.h>                                                  // 16x10 big font
#include "Arial_black_16.h"
#include "SystemFont5x7.h"                                              // font used for brightness setting confirmation
#include <Adafruit_MCP23008.h>                                          // hotkeys supported by MCP23008
#include <Keypad_MCP.h>                                                 // matrix keypad support for MCP23008
#include <Keypad.h>                                                     // matrix keypad base library
// uncomment the following line to enable serial debugging
// #define DEBUG 1

// uncomment the following line to enable flashing car number display
#define DMD_FLASHER 1

#ifdef DEBUG
#define DEBUG_PRINT(x)  Serial.println (F(x));
#else
#define DEBUG_PRINT(x)
#endif

/*

  __  __  _         _  _  __    _   _____    ____  __  __ ____   __ _
 |  \/  |(_) _ __  (_)| |/ /   / \ |_   _|  |  _ \|  \/  |  _ \ / _| | __ _  __ _
 | |\/| || || '_ \ | || ' /   / _ \  | |    |   | | |\/| | | | | |_| |/ _` |/ _` |
 | |  | || || | | || || . \  / ___ \ | |    | |_| | |  | | |_| |  _| | (_| | (_| |
 |_|  |_||_||_| |_||_||_|\_\/_/   \_\|_|    |____/|_|  |_|____/|_| |_|\__,_|\__, |
                                                                             |___/
 Single DMD SmartyKAT v1.95

 Copyright (c) 2014 Geoff Steele & Lincoln Buell
 MyBigFont.h created by Trevor Queale, released in the public domain and used here with sincere thanks.

 This program is free software: you can redistribute it and/or modify it under the terms
 of the version 3 GNU General Public License as published by the Free Software Foundation.

 This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY;
 without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 See the GNU General Public License for more details.

 You should have received a copy of the GNU General Public License along with this program.
 If not, see <http://www.gnu.org/licenses/>.

 Revision history:
 v1.95 Timing tweaks to logo display and flashing digits
 v1.94 Changed life runtime counter to full screen height scrolling marquee
 v1.93 Added life runtime counter stored in EEPROM, displayed on startup
 v1.92 Compatibility changes for IDE 1.6, minor bugfix
 v1.91 Added flashing car number display
 v1.90 Updates for MK as per Dec 2, 2014 notes
 v1.80
 v1.70 Reintroduced SC hotkey
 v1.60 Reintroduced S & T hotkeys.  Release candidate time!
 v1.50 PWM brightness control returning to modified Freetronics DMD libraries; SmartyKAT logo in startup
 v1.41 PWM brightness control fixed - thanks Trev!
 v1.40 Brightness control via 2 hot keys; on startup returns to last brightness setting stored in EEPROM
 v1.30 Version without hotkey functionality
 v1.21 Fix of number centring by counting character widths
 v1.20 Trial of top to bottom typeface
 v1.11 2nd test, inc bug fixes
 v1.10 Reboot, usinng cut-down SmartyKAT firmware for commonality of hardware
 v1.00 Foundations/Proof of concept

 Arduino pin allocations
 0       Reserved for serial debugging
 1       Reserved for serial debugging
 2
 3
 4
 5       Activity/pulse LED
 6       used by DMD controller
 7       used by DMD controller
 8       used by DMD controller
 9       used by DMD controller
 10      SPI SS, DMD controller
 11      SPI MOSI, DMD controller
 12      SPI MISO, DMD controller
 13      SPI SCK, DMD controller
 A0/D14
 A1/D15
 A2/D16  one wire interface for DS18B20 (unused)
 A3/D17
 A4/D18  I2C SDA (data)used for MCP23008 I/O expanders
 A5/D19  I2C SCL (clock)used for MCP23008 I/O expanders

 Hotkey/keypad actions
 - Keypad can enter car number at any time
 - # backspace, * clears to start over
 - max 3 digit car numbers with possible leading zeroes
 - on Clear or startup "ALL" is displayed instead of a car number

 */

// I2C addresses for the two MCP23008 ICs
// RTC is at 0x68, 2004 LCD is at 0x27
#define PADI2CADDR 0x21
#define HOTI2CADDR 0x20

// keypad setup
const byte ROWS = 4;                                                    //four rows
const byte COLS = 3;                                                    //three columns
char keys[ROWS][COLS] = {
  {
    '1', '2', '3'
  }
  ,
  {
    '4', '5', '6'
  }
  ,
  {
    '7', '8', '9'
  }
  ,
  {
    '*', '0', '#'
  }
};
byte rowPins[ROWS] = {
  6, 5, 4, 3
};   //connect to the row pinouts of the keypad
byte colPins[COLS] = {
  2, 1, 0
};      //connect to the column pinouts of the keypad

// instance of MCP23008 keypad
Keypad_MCP keypad = Keypad_MCP( makeKeymap(keys), rowPins, colPins, ROWS, COLS, PADI2CADDR );
// instance of MCP23008 for driving hotkeys
Adafruit_MCP23008 mcp;
// state constants for referencing switches
// Input #0 is pin 10 of MCP23008, input #7 is pin 17
// possible flags - defines what to display on DMD array & menu items for LCD
#define NOflag 0                                                        // clear/reset
#define NUMBERonly 1                                                    // display car number only
#define TRANSflag 2                                                     // transponder flag
#define SOUNDflag 3                                                     // sound flag
#define ALLflag 4                                                       // ALL
#define SCflag 5                                                        // safety car
#define Chequer 6                                                       // checker/chequer
#define ONEGOflag 7                                                     // 1 lap to go
#define NETflag 8                                                       // net flag
#define TimeNTemp 9                                                     // just display current time & temperature
#define SetHotkeys 10                                                   // choosing which flags are on hotkeys
#define SetupMenu 11                                                    // enter setup menu
// navigation & menu switches
#define NOswitch -1                                                     // null, no hotkey
#define USR1 0                                                          // User programmable hotkey #1
#define USR2 1                                                          // User programmable hotkey #2
#define USR3 2                                                          // User programmable hotkey #3
#define USR4 3                                                          // User programmable hotkey #4
#define MUPswitch 4                                                     // Menu UP (not used MiniKAT)
#define MDNswitch 5                                                     // Menu Down (not used on MiniKAT)
#define MSELswitch 6                                                    // Menu Display/Execute (not used on MiniKAT)
#define MCLRswitch 7                                                    // Menu Clear/Reset (not used on MiniKAT)
#define switchCount 8

// day/night mode contants
#define DAYMODE 80                                                      // DMD intensity for daylight use
#define NIGHTMODE 10                                                    // DMD intensity for night use

// Tailor DMD flashing times with these (ms)
#define DMD_ONTIME 750
#define DMD_OFFTIME 250

// for hotkey logic
unsigned long repeatTime = 250;                                         // time in ms before an additional button press of same hotkey is valid
unsigned long lastTimePressed[switchCount];                             // for debouncing
int selectedMessage = NUMBERonly;                                       // what should the miniKAT be displaying now?
boolean showingNumber = true;                                           // used when swapping between car number and T/S to keep track of what's on now
const long DMDrotateTime = 750;                                         // time between swapping between car number and T/S etc
long nextDMDrotate;                                                     // ...and when the next rotate should be
unsigned long brightnessTimeout = 750;                                  // wait after brightness set where switch won't respond & we display change of brightness info
unsigned long brightnessUpdateTime;

// pin definitions
const int pulseLED = 5;

// status variables
long statusLEDtimer = 0L;                                               // time when status LED was last activated
long LEDduration = 250L;                                                // time after which status LED is extinguished
char carNumber[7] = "      ";                                           // car number includes 3 undo slots, initially blanks
long lastRuntimeTick;                                                   // used to update runtime counter every minute

// DMD
//Fire up the DMD library as dmd
DMD dmd(1, 1);                                                          // 1x DMD displays only

// EEPROM saved settings
struct settings_t {
  int DMDbrightness;                                                    // backlight PWM setting, 10-100%
  int runningHours;                                                     // active time in hours and minutes
  int runningMinutes;
}
savedValues;

// EEPROM variables & constants
uint8_t EEPROMblank = 255;                                              // used to 'zero' out unused EEPROM locations
const int EEPROMmax = 1023;                                             // largest EEPROM cell ID (ATmega328)
int EEPROMidx = 0;                                                      // index to where current settings are held in EEPROM
boolean firstRun = false;                                               // true if EEPROM has been freshly formatted

// ---------------------------------------------------------------------------------
// DMD refresh scanning is driven by the Timer1 interrupt every 5ms
//
void ScanDMD() {
  dmd.scanDisplayBySPI();
}

// ---------------------------------------------------------------------------------
//
void setup() {
#ifdef DEBUG
  Serial.begin(9600);                                                   // !!! debugging
  Serial.print(F("setup() start; RAM:"));                               // !!! debugging
  Serial.println(freeRam());
#endif

  // Setup pulse LED
  pinMode(pulseLED, OUTPUT);
  digitalWrite(pulseLED, true);

  // Start I2C
  Wire.begin();

  // start keypad instance
  keypad.begin();

  // setup all console hotkey switches (even though we only use 4)
  mcp.begin();                                                          // use default address 0
  for (int i = 0; i < switchCount; i++) {
    mcp.pinMode(i, INPUT);                                              //
    mcp.pullUp(i, HIGH);                                                // internal 100K pull-up active
    lastTimePressed[i] = 0;                                             // all switches at this stage have never been pressed
  }

  //initialize TimerOne's interrupt/CPU usage used to scan and refresh the display
  Timer1.initialize(5000);                                              // period in microseconds to call ScanDMD. Anything longer than 5000 (5ms) and you can see flicker.
  Timer1.pwm(PIN_DMD_nOE, 1024);                                        // set initial brightness
  Timer1.attachInterrupt(ScanDMD);                                      // attach the Timer1 interrupt to ScanDMD which goes to dmd.scanDisplayBySPI()

  //clear/init the DMD pixels held in RAM
  dmd.clearScreen(true);                                                // true is normal (all pixels off), false is negative (all pixels on)

  DEBUG_PRINT("> reading EEPROM...")

  // read settings from EEPROM
  EEPROMidx -= sizeof(savedValues);                                     // starting location will become 0 for first write, OR will start where data found
  if (!readSavedValues()) {                                             // if there's nothing stored there's work to do
    DEBUG_PRINT(">> No saved value found. Initialising EEPROM");
    initialiseEEPROM();                                                 // wipes EEPROM
    setEEPROMdefaults();                                                // save default values since EEPROM load failed
  }
  DEBUG_PRINT("> done.")

  statusLEDtimer = millis();
  digitalWrite(pulseLED, false);

  // SmartyKAT logo
  SplashScreen(0, 0);
  long endWait = millis() + 2000L;

  // show lifetime run counter if hotkey touched in next 2 seconds (or...show SmartyKAT logo for 2 seconds)
  while (millis() < endWait) {
    if (readHotkeys() != NOswitch) {
      dmd.clearScreen(true);                                                // clears the version string text still on screen (v1.92 bugfix)
      dmd.selectFont(Arial_Black_16);                                       // big thick type
      char lifeMsg[40] = "Total Runtime:                         ";         // message template
      int MsgIdx = 15;                                                      // starting position in message
      // display lifetime run counter in hours and minutes
      if (savedValues.runningHours > 9999) lifeMsg[MsgIdx++] = '0' + ((savedValues.runningHours % 100000) / 10000); // hours ten thousands
      if (savedValues.runningHours > 999) lifeMsg[MsgIdx++] =  '0' + ((savedValues.runningHours % 10000) / 1000); // hours thousands
      if (savedValues.runningHours > 99) lifeMsg[MsgIdx++] =   '0' + ((savedValues.runningHours % 1000) / 100);   // hours hundreds
      if (savedValues.runningHours > 9) lifeMsg[MsgIdx++] =    '0' + ((savedValues.runningHours % 100) / 10);     // hours tens
      if (savedValues.runningHours > 0) {
        char tmpMsgH[12] = " Hours and ";
        lifeMsg[MsgIdx++] = '0' + savedValues.runningHours % 10;                                                  // hours singles
        for (int i = 0; tmpMsgH[i]; i++) lifeMsg[MsgIdx++] = tmpMsgH[i];
      }
      if (savedValues.runningMinutes > 9) lifeMsg[MsgIdx++] = '0' + ((savedValues.runningMinutes % 100) / 10);    // minutes tens
      lifeMsg[MsgIdx++] = '0' + (savedValues.runningMinutes % 10);                                                // minutes singles

      char tmpMsgM[] = " Minutes";
      for (int i = 0; tmpMsgM[i]; i++) lifeMsg[MsgIdx++] = tmpMsgM[i];

      dmd.drawMarquee(lifeMsg, MsgIdx, 31, 0);                              // display marquee
      long start = millis();
      long timer = start;
      boolean ret = false;
      while (!ret) {
        if ((timer + 60) < millis()) {
          ret = dmd.stepMarquee(-1, 0);
          timer = millis();
        }
      }
      break; // out of while
    }
  } // while

  // startup text confirming version
  dmd.selectFont(System5x7);
  dmd.clearScreen(true);
  dmd.drawString( 1,  5, "v1.95", 5, GRAPHICS_NORMAL );
  delay(1000);
  dmd.clearScreen(true);

  updateBrightness();                                                     // set the brightness level to what it was when last used
  updateDMD();



}

// ---------------------------------------------------------------------------------
//
void loop() {
  boolean DMDupdateNeeded = false;
  updateLED();

  if (millis() - lastRuntimeTick > 60000) {                               // update lifetime run counter
    lastRuntimeTick += 60000;
    savedValues.runningMinutes++;
    if (savedValues.runningMinutes == 60) {
      savedValues.runningHours++;
      savedValues.runningMinutes = 0;
    }
    writeSavedValues();
  }

  // check for Hotkey activity
  int activeHotkey = readHotkeys();
  if (activeHotkey != NOswitch) {                                     // we have one
#ifdef DEBUG
    Serial.print(F("loop() ActiveHotkey detected: "));                // !!! debugging
    Serial.print(activeHotkey);                                       // !!! debugging
    Serial.print(F("; RAM:"));                                        // !!! debugging
    Serial.println(freeRam());                                        // !!! debugging
#endif
    int oldValue = savedValues.DMDbrightness;                         // used to test for change
    switch (activeHotkey) {
      case USR1:                                                      // sound
        selectedMessage = SOUNDflag;
        nextDMDrotate = millis();                                     // start rotating display immediately
        break;
      case USR2:                                                      // transponder
        selectedMessage = TRANSflag;
        nextDMDrotate = millis();                                     // start rotating display immediately
        break;
      case USR3:                                                      // transponder
        selectedMessage = SCflag;
        nextDMDrotate = 0;                                            // stop rotating display now
        DMDupdateNeeded = true;
        break;
      case USR4:                                                      // Daylight mode
        if (millis() - brightnessTimeout > brightnessUpdateTime) {
          if (savedValues.DMDbrightness != NIGHTMODE)
            savedValues.DMDbrightness = NIGHTMODE;
          else
            savedValues.DMDbrightness = DAYMODE;
          writeSavedValues();                                         // save to EEPROM to remember at next power up
          brightnessUpdateTime = millis();                            // dead time such that this switch stops responding for a while
          updateBrightness();                                         // make change
        }
        break;
    }
  }

  if (brightnessUpdateTime) {                                         // we're still displaying the brightness change message
    if (millis() - brightnessTimeout > brightnessUpdateTime) {
      brightnessUpdateTime = 0;
      DMDupdateNeeded = true;
    }
  }

  // check for keypad activity
  char key = keypad.getKey();
  if (key) {
    DMDupdateNeeded = true;                                           // we'll need to show edit on DMD
    if (selectedMessage == ALLflag || selectedMessage == SCflag) selectedMessage = NUMBERonly; // if displaying ALL or SC - stop that now
    switch (key) {
      case '#':                                                       // erase all
        sprintf(carNumber, "      ");                                 // number string all blanked out
        break;
      case '*':                                                       // backspace
        {
          // move all values 1 char to the right
          for (int i = 5; i > 0; i--) carNumber[i] = carNumber[i - 1];
          carNumber[0] = ' ';
          break;
        }
      default:                                                        // save key to rightmost digit of car number and shuffle others down
        for (int i = 0; i < 5; i++) carNumber[i] = carNumber[i + 1];
        carNumber[5] = key;
        break;
    }
#ifdef DEBUG
    Serial.print(F("loop() Car number: <"));                          // !!! debugging
    Serial.print(carNumber);                                          // !!! debugging
    Serial.println(F(">"));                                           // !!! debugging
    Serial.println(freeRam());
#endif
  }

#ifdef DMD_FLASHER
  // number rotating
  if (selectedMessage == NUMBERonly) {
    if ((millis() % (DMD_ONTIME + DMD_OFFTIME) < DMD_ONTIME) && !showingNumber) {
      DMDupdateNeeded = true;
      showingNumber = true;
    }
    if ((millis() % (DMD_ONTIME + DMD_OFFTIME) > DMD_ONTIME) && showingNumber) {
      DMDupdateNeeded = true;
      showingNumber = false;
    }
  }
#endif

  if (nextDMDrotate && millis() >= nextDMDrotate) {                   // update variables if we're swapping display content
    nextDMDrotate = millis() + DMDrotateTime;
    showingNumber = !showingNumber;
    DMDupdateNeeded = true;
  }
  if (DMDupdateNeeded) updateDMD();                                   // show new content on DMD

}




















