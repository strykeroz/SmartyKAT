
// ---------------------------------------------------------------------------------
// set the intensity of the pulse LED based on the current millis() clock value
//
void updateLED() {
  float pulseIntensity = (exp(sin(millis()/1000.0*PI)) - 0.36787944)*108.0;
  analogWrite(pulseLED, pulseIntensity);  
}


// --------------------------------------------------------------------------------------------
// Returns the number of available bytes of RAM
// Serial.println(freeRam());
//
int freeRam () {
  extern int __heap_start, *__brkval; 
  int v; 
  return (int) &v - (__brkval == 0 ? (int) &__heap_start : (int) __brkval); 
}


