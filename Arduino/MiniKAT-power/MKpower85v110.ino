#include <math.h>

/*
 __  __ _  __
|  \/  | |/ /_ __   _____      _____ _ __
| |\/| | ' /| '_ \ / _ \ \ /\ / / _ \ '__|
| |  | | . \| |_) | (_) \ V  V /  __/ |
|_|  |_|_|\_\ .__/ \___/ \_/\_/ \___|_|
            |_|                      v1.10

 MiniKAT power control board firmware.

 ATTiny85 connections
 Leg  Function
 1    Reset, no connection
 2    D3 as A3, voltage measurement input
 3    D4 Battery status LED (PWM capable)
 4    GND
 5    D0 charge voltage present input
 6    no connection
 7    D2 relay trigger output
 8    +5V

 Tested on ATTiny85 running at 8MHz.

 Release history
 1.10 Battery status light becomes a single colour LED
      Power off MK once low power threshold reached
      Do not power back on until after reset, or charge
 1.00 Foundations

 */

// pin definitions
const int RelayPin = 2;
const int BattLEDpin = 4;
const int ChargePin = 0;
const int BattVoltagePin = A3;

// tuning variables
const float BatteryFullVoltage = 4.8;                                   // used for battery voltage calculation
const float BatteryLowVoltage = 3.9;                                    // Warning voltage, also under this point MK won't power back on
const float BatteryCriticalVoltage = 3.6;                               // voltage under which MiniKAT is turned off for undervoltage
const long timeBetweenReadings = 5000;                                  // how often to read battery voltage
const long warningLEDon = 250;                                          // LED flash for battery warning
const long warningLEDoff = 1750;
const long gameoverLEDon = 100;                                         // LED flash for no start possible
const long gameoverLEDoff = 800;

// state variables
long nextReading = 2000;                                                // time at which next battery voltage to be read (first time, 2sec after startup)
boolean wasCharging;                                                    // if there was a voltage on the charge line last time we checked
float batteryVoltage;                                                   // most recent battery voltage reading
boolean poweredOff = true;                                              // true if MK powered off

// ---------------------------------------------------------------------------
void setup() {
  pinMode(RelayPin, OUTPUT);
  pinMode(BattLEDpin, OUTPUT);
  pinMode(ChargePin, INPUT);

  // set initial states
  wasCharging = isCharging();
}

// ---------------------------------------------------------------------------
void loop() {
  if (isCharging() != wasCharging) {
    if (!wasCharging) digitalWrite(RelayPin, LOW);                     // now charging, but wasn't: turn off MiniKAT power to charge
    wasCharging = !wasCharging;                                        // store new state
  }

  if (!wasCharging) {                                                  // not charging
    // not on charge, so battery checking during normal run
    if (millis() >= nextReading) {
      nextReading = millis() + timeBetweenReadings;
      batteryVoltage = BatteryVoltageNow();
    }

    if (batteryVoltage <= BatteryCriticalVoltage && !poweredOff) {    // not yet off, needs to be
      poweredOff = true;
      digitalWrite(RelayPin, LOW);                                    // power off MK
    }

    if (batteryVoltage <= BatteryLowVoltage && !poweredOff)           // flash warning LED
      digitalWrite(BattLEDpin, millis() % (warningLEDon + warningLEDoff) < warningLEDon);

    if (batteryVoltage > BatteryLowVoltage && poweredOff) {           // safe to turn MK on now
      digitalWrite(BattLEDpin, LOW);                                  // turn warning LED off
      digitalWrite(RelayPin, HIGH);                                   // power on MK
      poweredOff = false;
    }

    if (poweredOff) {
      if (!batteryVoltage) digitalWrite(BattLEDpin, HIGH);            // startup condition
      else                                                            // powered off due to critical battery
        digitalWrite(BattLEDpin, millis() % (warningLEDon + warningLEDoff) < warningLEDon);
    }

  } else {                                                            // charging
    // pulse the battery LED
    float val = (exp(sin(millis() / 2000.0 * PI)) - 0.36787944) * 108.0;
    analogWrite(BattLEDpin, val);
  }

}

// ---------------------------------------------------------------------------
// Return true if there's a voltage on the charge socket
//
boolean isCharging() {
  return digitalRead(ChargePin);
}

// ---------------------------------------------------------------------------
// Returns present battery voltage
//
float BatteryVoltageNow() {
  int sensorValue = analogRead(BattVoltagePin);
  return sensorValue * (BatteryFullVoltage / 1023.0);
}

