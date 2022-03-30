#ifndef _SETUP_H_
#define _SETUP_H_

#include <Arduino.h>
#include <EEPROM.h>
#include "morse.h"
#include "ubitx.h"
#include "nano_gui.h"

/** Menus
 *  The Radio menus are accessed by tapping on the function button. 
 *  - The main loop() constantly looks for a button press and calls doMenu() when it detects
 *  a function button press. 
 *  - As the encoder is rotated, at every 10th pulse, the next or the previous menu
 *  item is displayed. Each menu item is controlled by it's own function.
 *  - Eache menu function may be called to display itself
 *  - Each of these menu routines is called with a button parameter. 
 *  - The btn flag denotes if the menu itme was clicked on or not.
 *  - If the menu item is clicked on, then it is selected,
 *  - If the menu item is NOT clicked on, then the menu's prompt is to be displayed
 */

void setupExit();

void setupFreq();

void setupBFO();

void setupCwDelay();

void setupKeyer();

void drawSetupMenu();

void movePuck(int i);

void doSetup2();

#endif
