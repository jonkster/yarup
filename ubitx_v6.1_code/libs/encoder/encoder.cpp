#include "encoder.h"

volatile bool enc_aState = false;
volatile bool enc_bState = false;
volatile bool prevEnc_aState = false;
volatile bool prevEnc_bState = false;

volatile int encoderPos = 0;
volatile int encoderLastPos = 0;

#define BOUNCE_DELAY 200
volatile unsigned long tNow = 0;
volatile unsigned long tPrev = 0;
volatile bool fast = false;

void encISR(void);

int calcEncoderDelta(int aNow, int aPrev, int bNow, int bPrev) {
	if ((aPrev == LOW) && (aNow == HIGH)) {
		if (bNow == LOW) {
			return -1;
		} else {
			return 1;
		}
	} else if ((aPrev == HIGH) && (aNow == LOW)) {
		if (bNow == HIGH) {
			return -1;
		} else {
			return 1;
		}
	} else if ((bPrev == LOW) && (bNow == HIGH)) {
		if (aNow == HIGH) {
			return -1;
		} else {
			return 1;
		}
	}  else if ((bPrev == HIGH) && (bNow == LOW)) {
		if (aNow == LOW) {
			return -1;
		} else {
			return 1;
		}
	}
	return 0;
}

void checkChanges(void) {
	if ((enc_aState != prevEnc_aState) ||  (enc_bState != prevEnc_bState)) {
		encoderLastPos = encoderPos;
		int delta = calcEncoderDelta(enc_aState, prevEnc_aState, enc_bState, prevEnc_bState);
		Serial.println(delta);
		encoderPos += delta;
		prevEnc_aState = enc_aState;
		prevEnc_bState = enc_bState;
	}
}

int getEncoderDir(void) {
	int mag = (encoderLastPos - encoderPos);
	encoderLastPos = encoderPos;
	if (fast) {
		return mag * 3;
	}
	return mag;
}

int getEncoderValue(void) {
	return encoderPos;
}

bool encoderBtnDown(void) {
  return ! digitalRead(ENC_PUSH);
}

void setupEncoder(void) {
	pinMode(ENC_A, INPUT_PULLUP);
	pinMode(ENC_B, INPUT_PULLUP);
	pinMode(ENC_PUSH, INPUT_PULLUP);

	cli();
	attachInterrupt(ENC_A, encISR, CHANGE);
	//attachInterrupt(ENC_B, encISR, CHANGE);
	sei();
}

void encISR(void)
{
	tNow = micros();
	if ((tNow - tPrev) > BOUNCE_DELAY) {
		enc_aState = digitalRead(ENC_A);
		enc_bState = digitalRead(ENC_B);
		checkChanges();
		fast = (tNow - tPrev) < (5 * BOUNCE_DELAY);
	}
	tPrev = tNow;
}


