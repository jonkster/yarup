#ifndef _ENCLIB_
#define _ENCLIB_

#include <Arduino.h>
#include <avr/interrupt.h>
#include <stdbool.h>

#define ENC_A A0
#define ENC_B A1
#define ENC_PUSH A2

int getEncoderValue(void);

int getEncoderDir(void);

bool encoderBtnDown(void);

void setupEncoder(void);

#endif

