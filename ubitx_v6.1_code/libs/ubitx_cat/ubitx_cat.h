#ifndef _UBITX_CAT_H_
#define _UBITX_CAT_H_

#include <Arduino.h>
#include "ubitx.h"
#include "nano_gui.h"

/**
 * The CAT protocol is used by many radios to provide remote control to comptuers through
 * the serial port.
 * 
 * This is very much a work in progress. Parts of this code have been liberally
 * borrowed from other GPLicensed works like hamlib.
 * 
 * WARNING : This is an unstable version and it has worked with fldigi, 
 * it gives time out error with WSJTX 1.8.0  
 */

void checkCAT();

#endif
