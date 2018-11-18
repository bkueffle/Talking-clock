/*
 * ECE 5250
 * Final Project
 * Talking clock
 * 
 * talking_clock.cpp
 * Contains clock related functions
 * 
 * Benjamin Kueffler
 * Raymond Mak
 * Rafi Meguerdijian
 * Eugene Chiu
 */

#include <talking_clock.h>

/* Function Implementation */

// Sets the LED display to display different text indications, depending on what mode was selected
// Inputs : disp - Display LED object, mode - integer presenting operating time/language
void ModeToDisplay(TM1637Display* disp, uint8_t mode)
{
  (mode < sizeof(kModeMsg)) ? (*disp).setSegments(kModeMsg[mode], 4, 0) : (*disp).setSegments(kModeErrMsg, 4, 0);
}