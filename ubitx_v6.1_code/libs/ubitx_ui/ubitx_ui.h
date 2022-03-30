#ifndef _UBITX_UI_H_
#define _UBITX_UI_H_

#include <Arduino.h>
#include <EEPROM.h>
#include "morse.h"
#include "ubitx.h"
#include "nano_gui.h"

/**
 * The user interface of the ubitx consists of the encoder, the push-button on top of it
 * and the 16x2 LCD display.
 * The upper line of the display is constantly used to display frequency and status
 * of the radio. Occasionally, it is used to provide a two-line information that is 
 * quickly cleared up.
 */

void drawCommandbar(char *text); 

int getValueByKnob(int minimum, int maximum, int step_size,  int initial, char* prefix, char *postfix) ; 

void printCarrierFreq(unsigned long freq); 

void displayDialog(char *title, char *instructions); 

void displayVFO(int vfo); 

void displayRIT(); 

void drawTx();

void drawStatusbar(); 

void guiUpdate(); 

void updateDisplay() ; 

void ritToggle(struct Button *b); 

void splitToggle(struct Button *b); 

void vfoReset(); 

void cwToggle(struct Button *b); 

void sidebandToggle(struct Button *b); 

void redrawVFOs(); 

void switchBand(long bandfreq); 

int setCwSpeed(); 

void setCwTone(); 

void doCommand(struct Button *b); 

void  checkTouch(); 

void drawFocus(int ibtn, int color); 

void doCommands(); 

#endif
