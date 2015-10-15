// ---------------------------------------------------------------------------------
// returns a hotkey found that's pressed but wasn't earlier (only the last one found,
// there is no test for multiple keypresses).  Else returns -1 if none have changed.
// Simple software debouncing used, and updates currently recorded values across all hotkeys
//
int readHotkeys() {
  int activeKey = NOswitch;
  // remember: internal pull-ups make hotkeys true by default
  for(int i = 0; i<switchCount; i++) {                                                      
    if(!mcp.digitalRead(i)) {                                                // something has been pressed
      delay(5);                                                              // 
      if(!mcp.digitalRead(i)) {                                              // and it's still pressed after debounce retry
        if(lastTimePressed[i] + repeatTime <= millis() || lastTimePressed[i] == 0) {
          activeKey = i;                                                     // keep for return value only if it's a new key press
          lastTimePressed[i] = millis();                                     // remember new button press time
        }
      }
    } 
    else lastTimePressed[i] = 0;                                             // button has been released therefore is TRUE/HIGH
  }

  return activeKey;                                                           // will still be -1 if none were pressed
}
