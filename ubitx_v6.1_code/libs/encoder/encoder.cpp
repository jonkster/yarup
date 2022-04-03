#include "encoder.h"

volatile bool enc_aState = false;
volatile bool enc_bState = false;
volatile bool prevEnc_aState = false;
volatile bool prevEnc_bState = false;

volatile int encoderPos = 0;
volatile int encoderLastPos = 0;
volatile int encoderLastDelta = 0;

volatile unsigned long tPrev = 0;

void encISR(void);

// rotary encode state table
const int8_t dirTable[16] = { 0, +1, -1, 0, -1, 0, 0, +1, +1, 0, 0, -1, 0, -1, +1, 0 };

// bounce delay in micro seconds
#define BOUNCE_DELAY_US 1

void inhibitRead() {
	tPrev = micros();
}
unsigned long turnDelay() {
	return (micros() - tPrev);
}

bool inhibited() {
	unsigned long tNow = micros();
	return (turnDelay()) < BOUNCE_DELAY_US;
}


int calcEncoderDelta(int aNow, int aPrev, int bNow, int bPrev) {
	uint8_t bin = (aPrev << 3) + (bPrev << 2) + (aNow << 1) + bNow;
	int delta = dirTable[bin];
	if (delta == 0) {
		// this an invalid state.  Delay and assume state was as per previous read
		inhibitRead();
		enc_aState = prevEnc_aState;
		enc_bState = prevEnc_aState;
		return 0;
	}
	encoderLastDelta = delta;
	return delta;
}


void checkChanges(void) {
	if ((enc_aState != prevEnc_aState) ||  (enc_bState != prevEnc_bState)) {
		encoderPos += calcEncoderDelta(enc_aState, prevEnc_aState, enc_bState, prevEnc_bState);
		prevEnc_aState = enc_aState;
		prevEnc_bState = enc_bState;
	}
}

int getEncoderDir(void) {
	int mag = (encoderLastPos - encoderPos);
	encoderLastPos = encoderPos;
	if (mag > 0) {
		return 1;
	} else if (mag < 0) {
		return -1;
	} else {
		return 0;
	}
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
	attachInterrupt(ENC_B, encISR, CHANGE);
	sei();
}

void encISR(void)
{
	cli();
	if (! inhibited()) {
		enc_aState = digitalRead(ENC_A);
		enc_bState = digitalRead(ENC_B);
		checkChanges();
	}
	inhibitRead();
	sei();
}


