#include <math.h>                                          // maths for pulsing the LED
#include <avr/pgmspace.h>                                  // for storing and retrieving constants in PROGMEM
#include <avr/eeprom.h>                                    // for storing and retrieving settings in EEPROM
#include <Wire.h>                                          // I2C protocol for LCD and MCP23008 I/O extenders
#include <LiquidCrystal_I2C.h>                             // LCD
#include <SPI.h>                                           // for DMD display
// NOTE: REQUIRES dimmable modifications to DMD library, described http://forum.freetronics.com/viewtopic.php?f=26&t=5760
#include <DMD.h>                                           // DMD array display
#include <TimerOne.h>                                      // and THIS, is for the DMD display periodic refreshes
#include <Adafruit_MCP23008.h>                             // hotkeys supported by MCP23008
#include <Keypad_MCP.h>                                    // matrix keypad support for MCP23008
#include <Keypad.h>                                        // matrix keypad base library
// uncomment the following line to enable serial debugging
// #define DEBUG 1

// uncomment the following line to enable hotkeys to change DMD content without pressing display
// #define HOTKEY-IS-SHORTCUT 1

#ifdef DEBUG
#define DEBUG_PRINT(x)  Serial.println (F(x));
#else
#define DEBUG_PRINT(x)
#endif

/*
  ____                       _         _  __    _  _____   ____  __  __ ____   __ _
 / ___| _ __ ___   __ _ _ __| |_ _   _| |/ /   / \|_   _| |  _ \|  \/  |  _ \ / _| | __ _  __ _
 \___ \| '_ ` _ \ / _` | '__| __| | | | ' /   / _ \ | |   | | | | |\/| | | | | |_| |/ _` |/ _` |
  ___) | | | | | | (_| | |  | |_| |_| | . \  / ___ \| |   | |_| | |  | | |_| |  _| | (_| | (_| |
 |____/|_| |_| |_|\__,_|_|   \__|\__, |_|\_\/_/   \_\_|   |____/|_|  |_|____/|_| |_|\__,_|\__, |
                                  |___/                                                    |___/
                                                                              DMD LED flag v1.25

 Copyright (c) 2015 Geoff Steele (geoffs_aus@hotmail.com) & Lincoln Buell (buell_rtr75@att.net)

 BIG FONT (2-line) LCD CHARACTERS method by Adrian Jones, February 2015
 (Initial idea by http://www.instructables.com/id/Custom-Large-Font-For-16x2-LCDs/step5/Arduino-Sketch/)

 This program is free software: you can redistribute it and/or modify it under the terms
 of the version 3 GNU General Public License as published by the Free Software Foundation.

 This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY;
 without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 See the GNU General Public License for more details.

 You should have received a copy of the GNU General Public License along with this program.
 If not, see <http://www.gnu.org/licenses/>.

 Revision history:
 v1.25 Changed total life runtime so it only appears if any hotkey is pressed while SK logo displayed (during pause at 75% startup)
 v1.24 Changed total life runtime so it only increments while DMD is active - becomes DMD life runtime
 v1.23 Bugfix - display 1-G0 as there's no 'O' in our font.  The same DMD glyph now appears for 0 & O.
 v1.22 Bugfix in menu not showing 1-Go in correct spot
 v1.21 Added 1-GO display
       Resequence menu
 v1.20 Removes time and temperature, for use with SmartyKAT 1.20 controller
       LCD echoes DMD when actively displaying
       New setup menu option to format EEPROM and set defaults
       New setup menu option to keep status LED off when running
       Added life runtime counter in EEPROM, displayed on startup
 v1.13 Bugfix clock updates weren't starting
 v1.12 Bugfix. Added 'D' in bigtype
       fixed T flag
       fixed clock so it updates on the minute & flashes colon on the second
       removed leading 0 in 12 hour time format
 v1.11 Modded version requested by Bob. Hotkey for ALL replaced by checker
 v1.10 Modded version requested by Bob. No temperature reading
       pulse LED on when displaying
       wider T
       SND rather than S
 v1.01 Compatibility with IDE 1.6
       change to menu option sequence (hotkeys after others)
 v1.00 Only change to v0.53 is to make hotkeys not activate immediately - changeable by #define value HOTKEY-IS-SHORTCUT
 v0.53 bug fix, font rework & enhancements before final release (RC1)
 v0.52 bug fix
 v0.51 bug fix - error 85 on temperature sensor
 v0.5  User programmable hotkeys
       time and temperature display
 v0.4  Sync with featureset of MiniKAT 1.60 RC1 (dimming, alternating displays)
 v0.3  Changes to flags as per Peter's feedback
       Change of screen size to 2x2
       text by graphic primatives
 v0.2  Add large font or ALL, competitor number and clock/temp
 v0.1  Foundations

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
 A2/D16
 A3/D17
 A4/D18  I2C SDA (data)used for 20x4 LCD display & MCP23008 I/O expanders
 A5/D19  I2C SCL (clock)used for 20x4 LCD display & MCP23008 I/O expanders

 Notes:
 4 Hotkeys for menu navigation
 - Menu Up, Menu Down, Clear & Display

 Hotkey/keypad actions
 - Only one hotkey action at a time, others ignored
 - Repeat after half a second
 - Keypad can enter car number at any time
 - # clear to start over and * backspace in competitor number entry
 - max 3 digit car numbers with possible leading zeroes

 Display items  (from Peter's feedback)
 - S, Sound (with car number)
 - T, Transponder (with car number)
 - S.C.
 - Net + car #, Window Net + car#
 - ALL

 Setup menu (settings that can be changed & diagnostics)
 - backlight duration on LCD display
 - set DMD brightness

 */
// I2C addresses for the two MCP23008 ICs
// RTC is at 0x68, 2004 LCD is at 0x27
#define PADI2CADDR 0x21
#define HOTI2CADDR 0x20

// keypad setup
#define ROWS 4                                       //four rows
#define COLS 3                                       //three columns
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
// hotkeys

// possible flags - defines what to display on DMD array & menu items for LCD
#define NOflag 0                                          // clear/reset
#define OneGo 1                                           // 1-GO
#define Chequer 2                                         // checker/chequer
#define SCflag 3                                          // safety car
#define ALLflag 4                                         // ALL
#define NETflag 5                                         // net flag
#define NUMBERonly 6                                      // display car number only
#define TRANSflag 7                                       // transponder flag
#define SOUNDflag 8                                       // sound flag
#define SetupMenu 9                                       // enter setup menu
// navigation & menu switches
#define NOswitch -1                                       // null, no hotkey
#define USR1 0                                            // User hotkey #4
#define USR2 1                                            // User hotkey #3
#define USR3 2                                            // User hotkey #2
#define USR4 3                                            // User hotkey #1
#define MUPswitch 7                                       // Menu UP
#define MDNswitch 6                                       // Menu Down
#define MSELswitch 4                                      // Menu Display/Execute
#define MCLRswitch 5                                      // Menu Clear/Reset
#define switchCount 8
#define menuOptions 10

// day/night mode contants
#define DAYMODE 80                                        // DMD intensity for daylight use
#define NIGHTMODE 10                                      // DMD intensity for night use

// pin definitions
#define pulseLED 5

// for hotkey logic
unsigned long repeatTime = 250;                           // time in ms before an additional button press of same hotkey is valid
unsigned long lastTimePressed[switchCount];               // for debouncing

// message variables
int selectedMessage = 0;                                  // if execute were pressed now, what would display
int nowShowing = 0;                                       // first message displayed at top of LCD
boolean showingNumber = true;                             // used when swapping between car number and T/S to keep track of what's on now
const unsigned long DMDrotateTime = 1000;                 // time between swapping between car number and T/S etc
unsigned long nextDMDrotate;                              // ...and when the next rotate should be
const unsigned long brightnessTimeout = 750;              // wait after brightness set where display continues to show how bright it is
unsigned long brightnessUpdateTime;

// items from main LCD menu
const char string_0[] PROGMEM = "Clear/reset ";
const char string_1[] PROGMEM = "1-Go        ";
const char string_2[] PROGMEM = "Checker     ";
const char string_3[] PROGMEM = "Safety Car  ";
const char string_4[] PROGMEM = "All         ";
const char string_5[] PROGMEM = "Net         ";
const char string_6[] PROGMEM = "Car # only  ";
const char string_7[] PROGMEM = "Transponder ";
const char string_8[] PROGMEM = "Sound       ";
const char string_9[] PROGMEM = "Set options ";
const char* const menuItems[menuOptions] PROGMEM = {
  string_0,
  string_1,
  string_2,
  string_3,
  string_4,
  string_5,
  string_6,
  string_7,
  string_8,
  string_9
};

// flags each menu item relates to
const int menuAction[menuOptions] = {MCLRswitch, NUMBERonly, TRANSflag, SOUNDflag, ALLflag, SCflag, Chequer, NETflag, SetupMenu};

// status variables
unsigned long statusLEDtimer = 0L;                        // time when status LED was last activated
unsigned long LEDduration = 250L;                         // time after which status LED is extinguished
char carNumber[7] = "      ";                             // car number includes 3 undo slots, initially blanks
unsigned long lastRuntimeTick;                            // used to update runtime counter every minute

// DMD
DMD dmd(2, 2);                                            // 4x DMD displays, 2 across x 2 vertical

// I2C character LCD
LiquidCrystal_I2C lcd(0x27, 20, 4);                       // set the LCD address to 0x27 for a 20 chars and 4 line display
// custom characters
const char custom[][8] PROGMEM = {                        // Custom character definitions
  { 0x1F, 0x1F, 0x1F, 0x00, 0x00, 0x00, 0x00, 0x00 }, // char 1
  { 0x18, 0x1C, 0x1E, 0x1F, 0x1F, 0x1F, 0x1F, 0x1F }, // char 2
  { 0x1F, 0x1F, 0x1F, 0x1F, 0x1F, 0x0F, 0x07, 0x03 }, // char 3
  { 0x00, 0x00, 0x00, 0x00, 0x00, 0x1F, 0x1F, 0x1F }, // char 4
  { 0x1F, 0x1F, 0x1F, 0x1F, 0x1F, 0x1E, 0x1C, 0x18 }, // char 5
  { 0x1F, 0x1F, 0x1F, 0x00, 0x00, 0x00, 0x1F, 0x1F }, // char 6
  { 0x1F, 0x00, 0x00, 0x00, 0x00, 0x1F, 0x1F, 0x1F }, // char 7
  { 0x03, 0x07, 0x0F, 0x1F, 0x1F, 0x1F, 0x1F, 0x1F }  // char 8
};

// BIG FONT Character Set
// - Each character coded as 1-4 byte sets {top_row(0), top_row(1)... bot_row(0), bot_row(0)..."}
// - number of bytes terminated with 0x00; Character is made from [number_of_bytes/2] wide, 2 high
// - codes are: 0x01-0x08 => custom characters, 0x20 => space, 0xFF => black square

const char bigChars[][8] PROGMEM = {
  { 0x20, 0x20, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }, // Space
  { 0xFF, 0x07, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }, // !
  { 0x05, 0x05, 0x20, 0x20, 0x00, 0x00, 0x00, 0x00 }, // "
  { 0x04, 0xFF, 0x04, 0xFF, 0x04, 0x01, 0xFF, 0x01 }, // #
  { 0x08, 0xFF, 0x06, 0x07, 0xFF, 0x05, 0x00, 0x00 }, // $
  { 0x01, 0x20, 0x04, 0x01, 0x04, 0x01, 0x20, 0x04 }, // %
  { 0x08, 0x06, 0x02, 0x20, 0x03, 0x07, 0x02, 0x04 }, // &
  { 0x05, 0x20, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }, // '
  { 0x08, 0x01, 0x03, 0x04, 0x00, 0x00, 0x00, 0x00 }, // (
  { 0x01, 0x02, 0x04, 0x05, 0x00, 0x00, 0x00, 0x00 }, // )
  { 0x01, 0x04, 0x04, 0x01, 0x04, 0x01, 0x01, 0x04 }, // *
  { 0x04, 0xFF, 0x04, 0x01, 0xFF, 0x01, 0x00, 0x00 }, // +
  { 0x20, 0x05, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }, //
  { 0x04, 0x04, 0x04, 0x20, 0x20, 0x20, 0x00, 0x00 }, // -
  { 0x20, 0x04, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }, // .
  { 0x20, 0x20, 0x04, 0x01, 0x04, 0x01, 0x20, 0x20 }, // /
  { 0x08, 0x01, 0x02, 0x03, 0x04, 0x05, 0x00, 0x00 }, // 0
  { 0x01, 0x02, 0x20, 0x04, 0xFF, 0x04, 0x00, 0x00 }, // 1
  { 0x06, 0x06, 0x02, 0xFF, 0x07, 0x07, 0x00, 0x00 }, // 2
  { 0x01, 0x06, 0x02, 0x04, 0x07, 0x05, 0x00, 0x00 }, // 3
  { 0x03, 0x04, 0xFF, 0x20, 0x20, 0xFF, 0x00, 0x00 }, // 4
  { 0xFF, 0x06, 0x06, 0x07, 0x07, 0x05, 0x00, 0x00 }, // 5
  { 0x08, 0x06, 0x06, 0x03, 0x07, 0x05, 0x00, 0x00 }, // 6
  { 0x01, 0x01, 0x02, 0x20, 0x08, 0x20, 0x00, 0x00 }, // 7
  { 0x08, 0x06, 0x02, 0x03, 0x07, 0x05, 0x00, 0x00 }, // 8
  { 0x08, 0x06, 0x02, 0x07, 0x07, 0x05, 0x00, 0x00 }, // 9
  { 0xA5, 0xA5, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }, // :
  { 0x04, 0x05, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }, // ;
  { 0x20, 0x04, 0x01, 0x01, 0x01, 0x04, 0x00, 0x00 }, // <
  { 0x04, 0x04, 0x04, 0x01, 0x01, 0x01, 0x00, 0x00 }, // =
  { 0x01, 0x04, 0x20, 0x04, 0x01, 0x01, 0x00, 0x00 }, // >
  { 0x01, 0x06, 0x02, 0x20, 0x07, 0x20, 0x00, 0x00 }, // ?
  { 0x08, 0x06, 0x02, 0x03, 0x04, 0x04, 0x00, 0x00 }, // @
  { 0x08, 0x06, 0x02, 0xFF, 0x20, 0xFF, 0x00, 0x00 }, // A
  { 0xFF, 0x06, 0x05, 0xFF, 0x07, 0x02, 0x00, 0x00 }, // B
  { 0x08, 0x01, 0x01, 0x03, 0x04, 0x04, 0x00, 0x00 }, // C
  { 0xFF, 0x01, 0x02, 0xFF, 0x04, 0x05, 0x00, 0x00 }, // D
  { 0xFF, 0x06, 0x06, 0xFF, 0x07, 0x07, 0x00, 0x00 }, // E
  { 0xFF, 0x06, 0x06, 0xFF, 0x20, 0x20, 0x00, 0x00 }, // F
  { 0x08, 0x01, 0x01, 0x03, 0x04, 0x02, 0x00, 0x00 }, // G
  { 0xFF, 0x04, 0xFF, 0xFF, 0x20, 0xFF, 0x00, 0x00 }, // H
  { 0x01, 0xFF, 0x01, 0x04, 0xFF, 0x04, 0x00, 0x00 }, // I
  { 0x20, 0x20, 0xFF, 0x04, 0x04, 0x05, 0x00, 0x00 }, // J
  { 0xFF, 0x04, 0x05, 0xFF, 0x20, 0x02, 0x00, 0x00 }, // K
  { 0xFF, 0x20, 0x20, 0xFF, 0x04, 0x04, 0x00, 0x00 }, // L
  { 0x08, 0x03, 0x05, 0x02, 0xFF, 0x20, 0x20, 0xFF }, // M
  { 0xFF, 0x02, 0x20, 0xFF, 0xFF, 0x20, 0x03, 0xFF }, // N
  { 0x08, 0x01, 0x02, 0x03, 0x04, 0x05, 0x00, 0x00 }, // 0
  { 0x08, 0x06, 0x02, 0xFF, 0x20, 0x20, 0x00, 0x00 }, // P
  { 0x08, 0x01, 0x02, 0x20, 0x03, 0x04, 0xFF, 0x04 }, // Q
  { 0xFF, 0x06, 0x02, 0xFF, 0x20, 0x02, 0x00, 0x00 }, // R
  { 0x08, 0x06, 0x06, 0x07, 0x07, 0x05, 0x00, 0x00 }, // S
  { 0x01, 0xFF, 0x01, 0x20, 0xFF, 0x20, 0x00, 0x00 }, // T
  { 0xFF, 0x20, 0xFF, 0x03, 0x04, 0x05, 0x00, 0x00 }, // U
  { 0x03, 0x20, 0x20, 0x05, 0x20, 0x02, 0x08, 0x20 }, // V
  { 0xFF, 0x20, 0x20, 0xFF, 0x03, 0x08, 0x02, 0x05 }, // W
  { 0x03, 0x04, 0x05, 0x08, 0x20, 0x02, 0x00, 0x00 }, // X
  { 0x03, 0x04, 0x05, 0x20, 0xFF, 0x20, 0x00, 0x00 }, // Y
  { 0x01, 0x06, 0x05, 0x08, 0x07, 0x04, 0x00, 0x00 }, // Z
  { 0xFF, 0x01, 0xFF, 0x04, 0x00, 0x00, 0x00, 0x00 }, // [
  { 0x01, 0x04, 0x20, 0x20, 0x20, 0x20, 0x01, 0x04 }, // Backslash
  { 0x01, 0xFF, 0x04, 0xFF, 0x00, 0x00, 0x00, 0x00 }, // ]
  { 0x08, 0x02, 0x20, 0x20, 0x00, 0x00, 0x00, 0x00 }, // ^
  { 0x20, 0x20, 0x20, 0x04, 0x04, 0x04, 0x00, 0x00 }  // _
};

// EEPROM variables & constants
uint8_t EEPROMblank = 255;                                           // used to 'zero' out unused EEPROM locations
const int EEPROMmax = 1023;                                          // largest EEPROM cell ID (ATmega328)
int EEPROMidx = 0;                                                   // index to where current scores are held in EEPROM
boolean firstRun = false;                                            // true if EEPROM has been freshly formatted

struct settings_t {
  int LCDbacklight;                                                  // backlight setting, 0=always off; 1=always on; 2=30, 3=60sec after last keypress
  int DMDbrightness;                                                 // percentage brightness of DMD array
  boolean usePulseLED;                                               // whether pulse LED is used or not
  int USR[4];                                                        // hotkey actions
  int runningHours;                                                  // active time in hours and minutes
  int runningMinutes;
}
savedValues;

unsigned long LCDbacklightTimer = 0;                                 // time of most recent keypress
boolean DMDactive = false;                                           // is DMD blanked, or showing something?

// ---------------------------------------------------------------------------------
// DMD refresh scanning is driven by the Timer1 interrupt every 5ms
// Note: SPI clock divider edited in Freetronics library DMD.cpp to be DIV2
//
void ScanDMD() {
  dmd.scanDisplayBySPI();
}

// ---------------------------------------------------------------------------------
//
void setup() {
#ifdef DEBUG
  Serial.begin(9600);                                     // !!! debugging
#endif

  // Setup pulse LED
  pinMode(pulseLED, OUTPUT);
  digitalWrite(pulseLED, true);

  // Start console LCD display
  Wire.begin();

  lcd.init();                                             // start the lcd
  lcd.backlight();                                        // and fire up the backlight
  lcd.clear();
  lcd.print(F(" SmartyKAT LED Flag"));
  lcd.setCursor(8, 1);
  lcd.print(F("v1.25"));
  lcd.setCursor(4, 2);
  lcd.print(F("___start-up___"));
  int progressStep = 0;
  graphProgress(3, 1, 8, 150);

  //initialize TimerOne's interrupt/CPU usage used to scan and refresh the display
  Timer1.initialize(5000);                                // period in microseconds to call ScanDMD. Anything longer than 5000 (5ms) and you can see flicker.
  Timer1.pwm(PIN_DMD_nOE, 1024);                          // set initial brightness to flat out (overwritten soon)
  Timer1.attachInterrupt(ScanDMD);                        // attach the Timer1 interrupt to ScanDMD which goes to dmd.scanDisplayBySPI()

  //clear/init the DMD pixels held in RAM
  dmd.clearScreen(true);                                  //true is normal (all pixels off), false is negative (all pixels on)
  graphProgress(3, 2, 8, 150);

  // read settings from EEPROM
  EEPROMidx -= sizeof(savedValues);                       // starting location will become 0 for first write, OR will start where data found
  if (!readSavedValues()) {                               // if there's nothing stored there's work to do
    lcd.setCursor(0, 2);
    lcd.print(F("Formatting EEPROM..."));
    initialiseEEPROM();                                   // wipes EEPROM
    setEEPROMdefaults();                                  // save default values since EEPROM load failed
    lcd.setCursor(0, 2);
    lcd.print(F("Formatting complete."));
  }

  // create LCD display custom characters
  byte bb[8];                                                  // byte buffer for reading from PROGMEM
  for (byte nb = 0; nb < 8; nb++ ) {                           // create 8 custom characters
    for (byte bc = 0; bc < 8; bc++) bb[bc] = pgm_read_byte( &custom[nb][bc] );
    lcd.createChar ( nb + 1, bb );
  }

  graphProgress(3, 3, 8, 150);

  // start keypad instance
  keypad.begin();
  graphProgress(3, 4, 8, 150);

  // setup all console hotkey switches
  mcp.begin();                                            // use default address 0
  for (int i = 0; i < switchCount; i++) {
    mcp.pinMode(i, INPUT);                                //
    mcp.pullUp(i, HIGH);                                  // internal 100K pull-up active
    lastTimePressed[i] = 0;                               // all switches at this stage have never been pressed
  }
  graphProgress(3, 5, 8, 150);

  // SmartyKAT logo
  SplashScreen(15, 8);                                    // display DMD splash screen
  graphProgress(3, 6, 8, 150);
  long endWait = millis() + 2000L;
  // show lifetime run counter if hotkey touched in next 2 seconds (or...show SmartyKAT logo for 2 seconds)
  while (millis() < endWait) {
    if (readHotkeys() != NOswitch) {
      lcd.setCursor(0, 1);
      lcd.print(F("Total display time:"));
      lcd.setCursor(0, 2);
      for (int i = 0; i < 20; i++) lcd.print(' ');            // clear the line
      lcd.setCursor(0, 2);
      if (savedValues.runningHours) {
        lcd.print(savedValues.runningHours);
        lcd.print(F(" hrs, "));
      }
      lcd.print(savedValues.runningMinutes);
      lcd.print(F(" mins"));
      graphProgress(3, 7, 8, 150);
      delay(1500);                                          // allow time to read
    }
  }
      
  lcd.clear();
  updateBrightness();                                       // set the brightness level to what it was when last used; no DMD confirmation
  updateDMD();                                              // display initial DMD content

  statusLEDtimer = millis();
  digitalWrite(pulseLED, false);

  // setup is complete
  graphProgress(3, 8, 8, 250);                            // 100%
  lcd.clear();
  updateDisplay();

  // !!! debugging
  DEBUG_PRINT("setup() end");

}

// ---------------------------------------------------------------------------------
//
void loop() {
  boolean LCDupdateNeeded = false;                        // dirty flag for display update
  updateLED();                                            // sets intensity of pulse LED, LCD backlight

  // update lifetime run counter if DMD is displaying
  if (DMDactive) {                                        // Only incrementing if DMD is displaying
    if (!lastRuntimeTick) lastRuntimeTick = millis();     // DMD has only just become active

    if (millis() - lastRuntimeTick > 60000) {             // a minute has passed
      lastRuntimeTick += 60000;
      savedValues.runningMinutes++;
      if (savedValues.runningMinutes == 60) {             // hour rollover
        savedValues.runningHours++;
        savedValues.runningMinutes = 0;
      }
      writeSavedValues();                                 // record new runtime to EEPROM
    }
  } else lastRuntimeTick = 0;                             // zeroed when DMD not active

  // check for Hotkey activity
  int activeHotkey = readHotkeys();
  if (activeHotkey != NOswitch) {                         // we have one
    LCDbacklightTimer = millis();
    LCDupdateNeeded = true;

    switch (activeHotkey) {
      case MCLRswitch:
        selectedMessage = NOflag;                                // blanks out DMD
        updateDMD();                                             // make changes on DMD
        break;
      case MDNswitch:
        selectedMessage++;
        if (selectedMessage == menuOptions) selectedMessage = 0; // menu wrap
        break;
      case MUPswitch:
        selectedMessage--;
        if (selectedMessage < 0) selectedMessage = menuOptions - 1; // menu wrap
        break;
      case MSELswitch:
        LCDupdateNeeded = false;                            // no need to update LCD
        if (selectedMessage == SetupMenu) doSetupMenu();
        switch (selectedMessage) {
          case SetupMenu:
            doSetupMenu();
            break;
          default:
            updateDMD();                                   // make changes on DMD for selected menu option
            break;
        }
        break;
      default:
        // addresses USR hotkeys
        if (selectedMessage == savedValues.USR[activeHotkey]) LCDupdateNeeded = false;                // same as current menu option; no change
        else selectedMessage = savedValues.USR[activeHotkey];                                         // there has been a change

#ifdef HOTKEY-IS-SHORTCUT
        updateDMD();                                    // make changes on DMD for any hotkey specifically dealt with above
#endif
        break;
    }
  }

  // check for keypad activity
  char key = keypad.getKey();
  if (key) {
    LCDbacklightTimer = millis();
    LCDupdateNeeded = true;
    switch (key) {
      case '#':                                            // erase all
        sprintf(carNumber, "      ");
        break;
      case '*':                                            // backspace
        {
          // move all values 1 char to the right
          for (int i = 5; i > 0; i--) carNumber[i] = carNumber[i - 1];
          // replace the rightmost non-space, with a space
          boolean paddingDone = false;
          for (int i = 0; i < 5; i++)
            if (carNumber[i] != ' ' && !paddingDone) {
              paddingDone = true;
              carNumber[i] = ' ';
            }
          break;
        }
      default:                                             // save key to leftmost digit of car number and shuffle others down
        for (int i = 0; i < 5; i++) carNumber[i] = carNumber[i + 1];
        carNumber[5] = key;
        break;
    }
  }

  if (nextDMDrotate && millis() >= nextDMDrotate) {      // update variables if we're swapping display content
    nextDMDrotate += DMDrotateTime;
    showingNumber = !showingNumber;
    updateDMD();
  }

  // update menu if required
  if (LCDupdateNeeded || !DMDactive) updateDisplay();
}



















