#include <Wire.h>
#include <EEPROM.h>
#include "ubitx.h"
#include "nano_gui.h"
#include "ubitx_si5351.h"
#include "ubitx_cat.h"
#include "ubitx_ui.h"
#include "encoder.h"
#include "setup.h"
#include "keyer.h"

#define MONITOR_BAUD 115200

#define ENC_A		(A0)
#define ENC_B		(A1)
#define FBUTTON		(A2)
#define PTT		(A3)
#define ANALOG_KEYER	(A6)
#define ANALOG_SPARE	(A7)

/*#define TFT_CS		(10)
#define TFT_DC		(9)
#define CS_PIN		(8)*/

#define TX_RX		(7)
#define CW_TONE		(6)
#define TX_LPF_A	(5)
#define TX_LPF_B	(4)
#define TX_LPF_C	(3)
#define CW_KEY		(2)

#define INIT_USB_FREQ	(11059200l)
#define LOWEST_FREQ	(100000l)
#define HIGHEST_FREQ	(30000000l)

#define TX_SSB 0
#define TX_CW 1

char		ritOn = 0;
char		vfoActive = VFO_A;
int8_t		meter_reading = 0; // -1 = invisible
unsigned long	vfoA=7150000L;
unsigned long	vfoB=14200000L;
unsigned long	sideTone=800;
unsigned long	usbCarrier;
char		isUsbVfoA=0;
char		isUsbVfoB=1;
unsigned long	frequency;
unsigned long	ritRxFrequency;
unsigned long	ritTxFrequency; 
unsigned long	firstIF =   45005000L;
int		cwMode = 0;


boolean		txCAT = false;	//turned on if the transmitting due to a CAT command
char		inTx = 0;	//it is set to 1 if in transmit mode (whatever the reason : cw, ptt or cat)
int		splitOn = 0;	//working split, uses VFO B as the transmit frequency
char		keyDown = 0;	//in cw mode, denotes the carrier is being transmitted
char		isUSB = 0;	//upper sideband was selected, this is reset to the default for the 
				//frequency when it crosses the frequency border of 10 MHz
byte		menuOn = 0;	//set to 1 when the menu is being displayed, if a menu item sets it to zero, the menu is exited
unsigned long	cwTimeout = 0;  //milliseconds to go before the cw transmit line is released and the radio goes back to rx mode
unsigned long	dbgCount = 0;   //not used now
unsigned char	txFilter = 0;   //which of the four transmit filters are in use
boolean		modeCalibrate = false;


//these are variables that control the keyer behaviour
#define IAMBICB 0x10		// 0 for Iambic A, 1 for Iambic B
int		cwSpeed = 100;	// dot period in milliseconds
extern int32_t	calibration;
int		cwDelayTime = 60;
bool		Iambic_Key = true;
unsigned char	keyerControl = IAMBICB;
unsigned char	doingCAT = 0;


/****************************************************************************************/

/**
 * Original code for nano used global memory buffer to save space on stack.
 * Now we are using Nano Every with more resources, it would be nice to avoid
 * this but replicating old system until working out how to do this.
 */
char c[30], b[30];      
char printBuff[2][20];  //mirrors what is showing on the two lines of the display
int count = 0;          //to generally count ticks, loops, etc

/****************************************************************************************/



void active_delay(int delay_by) {
	unsigned long timeStart = millis();
	while (millis() - timeStart <= (unsigned long)delay_by) {
		delay(10);
		checkCAT();
	}
}

void saveVFOs() {
	if (vfoActive == VFO_A)
		EEPROM.put(VFO_A, frequency);
	else
		EEPROM.put(VFO_A, vfoA);

	if (isUsbVfoA)
		EEPROM.put(VFO_A_MODE, VFO_MODE_USB);
	else
		EEPROM.put(VFO_A_MODE, VFO_MODE_LSB);

	if (vfoActive == VFO_B)
		EEPROM.put(VFO_B, frequency);
	else
		EEPROM.put(VFO_B, vfoB);

	if (isUsbVfoB)
		EEPROM.put(VFO_B_MODE, VFO_MODE_USB);
	else 
		EEPROM.put(VFO_B_MODE, VFO_MODE_LSB);
}

/**
 * Select the tx harmonic filters
 * The four harmonic filters use only three relays
 * the four LPFs cover 30-21 Mhz, 18 - 14 Mhz, 7-10 MHz and 3.5 to 5 Mhz
 * Briefly, it works like this, 
 * - When KT1 is OFF, the 'off' position routes the PA output through the 30 MHz LPF
 * - When KT1 is ON, it routes the PA output to KT2. Which is why you will see that
 *   the KT1 is on for the three other cases.
 * - When the KT1 is ON and KT2 is off, the off position of KT2 routes the PA output
 *   to 18 MHz LPF (That also works for 14 Mhz) 
 * - When KT1 is On, KT2 is On, it routes the PA output to KT3
 * - KT3, when switched on selects the 7-10 Mhz filter
 * - KT3 when switched off selects the 3.5-5 Mhz filter
 * See the circuit to understand this
 */

void setTXFilters(unsigned long freq) {
	if (freq > 21000000L) {  // the default filter is with 35 MHz cut-off
		digitalWrite(TX_LPF_A, 0);
		digitalWrite(TX_LPF_B, 0);
		digitalWrite(TX_LPF_C, 0);
	} else if (freq >= 14000000L) { //thrown the KT1 relay on, the 30 MHz LPF is bypassed and the 14-18 MHz LPF is allowd to go through
		digitalWrite(TX_LPF_A, 1);
		digitalWrite(TX_LPF_B, 0);
		digitalWrite(TX_LPF_C, 0);
	} else if (freq > 7000000L) {
		digitalWrite(TX_LPF_A, 0);
		digitalWrite(TX_LPF_B, 1);
		digitalWrite(TX_LPF_C, 0);    
	} else {
		digitalWrite(TX_LPF_A, 0);
		digitalWrite(TX_LPF_B, 0);
		digitalWrite(TX_LPF_C, 1);    
	}
}


void setTXFilters_v5(unsigned long freq) {

	if (freq > 21000000L) {  // the default filter is with 35 MHz cut-off
		digitalWrite(TX_LPF_A, 0);
		digitalWrite(TX_LPF_B, 0);
		digitalWrite(TX_LPF_C, 0);
	} else if (freq >= 14000000L) { //thrown the KT1 relay on, the 30 MHz LPF is bypassed and the 14-18 MHz LPF is allowd to go through
		digitalWrite(TX_LPF_A, 1);
		digitalWrite(TX_LPF_B, 0);
		digitalWrite(TX_LPF_C, 0);
	} else if (freq > 7000000L) {
		digitalWrite(TX_LPF_A, 0);
		digitalWrite(TX_LPF_B, 1);
		digitalWrite(TX_LPF_C, 0);    
	} else {
		digitalWrite(TX_LPF_A, 0);
		digitalWrite(TX_LPF_B, 0);
		digitalWrite(TX_LPF_C, 1);    
	}
}

/**
 * This is the most frequently called function that configures the 
 * radio to a particular frequeny, sideband and sets up the transmit filters
 * 
 * The transmit filter relays are powered up only during the tx so they dont
 * draw any current during rx. 
 * 
 * The carrier oscillator of the detector/modulator is permanently fixed at
 * uppper sideband. The sideband selection is done by placing the second oscillator
 * either 12 Mhz below or above the 45 Mhz signal thereby inverting the sidebands 
 * through mixing of the second local oscillator.
 */

void setFrequency(unsigned long f) {
	uint64_t osc_f, firstOscillator, secondOscillator;
	setTXFilters(f);

	if (isUSB) {
		if (cwMode) {
			si5351bx_setfreq(2, firstIF  + f + sideTone);
		} else {
			si5351bx_setfreq(2, firstIF  + f);
		}
		si5351bx_setfreq(1, firstIF + usbCarrier);
	} else {
		if (cwMode) {
			si5351bx_setfreq(2, firstIF  + f + sideTone);      
		} else {
			si5351bx_setfreq(2, firstIF + f);
		}
		si5351bx_setfreq(1, firstIF - usbCarrier);
	}
	frequency = f;
}

/**
 * startTx is called by the PTT, cw keyer and CAT protocol to
 * put the uBitx in tx mode. It takes care of rit settings, sideband settings
 * Note: In cw mode, doesnt key the radio, only puts it in tx mode
 * CW offest is calculated as lower than the operating frequency when in LSB mode, and vice versa in USB mode
 */

void startTx(byte txMode) {
	unsigned long tx_freq = 0;  
	digitalWrite(TX_RX, 1);
	inTx = 1;
	if (ritOn) {
		ritRxFrequency = frequency;
		setFrequency(ritTxFrequency);
	} else 
	{
		if (splitOn == 1) {
			if (vfoActive == VFO_B) {
				vfoActive = VFO_A;
				isUSB = isUsbVfoA;
				frequency = vfoA;
			}
			else if (vfoActive == VFO_A) {
				vfoActive = VFO_B;
				frequency = vfoB;
				isUSB = isUsbVfoB;        
			}
		}
		setFrequency(frequency);
	}

	if (txMode == TX_CW) {
		digitalWrite(TX_RX, 0);

		//turn off the second local oscillator and the bfo
		si5351bx_setfreq(0, 0);
		si5351bx_setfreq(1, 0);

		//shif the first oscillator to the tx frequency directly
		//the key up and key down will toggle the carrier unbalancing
		//the exact cw frequency is the tuned frequency + sidetone
		if (isUSB) {
			si5351bx_setfreq(2, frequency + sideTone);
		} else {
			si5351bx_setfreq(2, frequency - sideTone);
		}

		delay(20);
		digitalWrite(TX_RX, 1);     
	}
	drawTx();
}

void stopTx() {
	inTx = 0;

	digitalWrite(TX_RX, 0);
	si5351bx_setfreq(0, usbCarrier);

	if (ritOn) {
		setFrequency(ritRxFrequency);
	} else {
		if (splitOn == 1) {
			//vfo Change
			if (vfoActive == VFO_B) {
				vfoActive = VFO_A;
				frequency = vfoA;
				isUSB = isUsbVfoA;        
			}
			else if (vfoActive == VFO_A) {
				vfoActive = VFO_B;
				frequency = vfoB;
				isUSB = isUsbVfoB;        
			}
		}
		setFrequency(frequency);
	}
	drawTx();
}

/**
 * ritEnable is called with a frequency parameter that determines
 * what the tx frequency will be
 */
void ritEnable(unsigned long f) {
	ritOn = 1;
	//save the non-rit frequency back into the VFO memory
	//as RIT is a temporary shift, this is not saved to EEPROM
	ritTxFrequency = f;
}

// this is called by the RIT menu routine
void ritDisable() {
	if (ritOn) {
		ritOn = 0;
		setFrequency(ritTxFrequency);
		updateDisplay();
	}
}

/**
 * The PTT is checked only if we are not already in a cw transmit session
 * If the PTT is pressed, we shift to the ritbase if the rit was on
 * flip the T/R line to T and update the display to denote transmission
 */

void checkPTT() {	

	//we don't check for ptt when transmitting cw
	if (cwTimeout > 0)
		return;

	if (digitalRead(PTT) == 0 && inTx == 0) {
		startTx(TX_SSB);
		active_delay(50); //debounce the PTT
	}

	if (digitalRead(PTT) == 1 && inTx == 1)
		stopTx();
}

//check if the encoder button was pressed
void checkButton() {
	int i, t1, t2, knob, new_knob;

	//only if the button is pressed
	if (! encoderBtnDown())
		return;

	active_delay(50);
	if (! encoderBtnDown()) //debounce
		return;

	//disengage any CAT work
	doingCAT = 0;

	int downTime = 0;
	while(encoderBtnDown()) {
		active_delay(10);
		downTime++;
		if (downTime > 300) {
			doSetup2();
			return;
		}
	}
	active_delay(100);

	doCommands();

	//wait for the button to go up again
	while(encoderBtnDown())
		active_delay(10);

	active_delay(50);//debounce
}

void switchVFO(int vfoSelect) {
	if (vfoSelect == VFO_A) {
		if (vfoActive == VFO_B) {
			vfoB = frequency;
			isUsbVfoB = isUSB;
			EEPROM.put(VFO_B, frequency);
			if (isUsbVfoB) {
				EEPROM.put(VFO_B_MODE, VFO_MODE_USB);
			} else {
				EEPROM.put(VFO_B_MODE, VFO_MODE_LSB);
			}
		}
		vfoActive = VFO_A;
		frequency = vfoA;
		isUSB = isUsbVfoA;
	} else {
		if (vfoActive == VFO_A) {
			vfoA = frequency;
			isUsbVfoA = isUSB;
			EEPROM.put(VFO_A, frequency);
			if (isUsbVfoA) {
				EEPROM.put(VFO_A_MODE, VFO_MODE_USB);
			} else {
				EEPROM.put(VFO_A_MODE, VFO_MODE_LSB);
			}
		}
		vfoActive = VFO_B;
		frequency = vfoB;
		isUSB = isUsbVfoB;
	}

	setFrequency(frequency);
	redrawVFOs();
	saveVFOs();
}

/**
 * The tuning jumps by 50 Hz on each step when you tune slowly
 * As you spin the encoder faster, the jump size also increases 
 * This way, you can quickly move to another band by just spinning the 
 * tuning knob
 */

void doTuning() {
	int s;
	static unsigned long prev_freq;
	static unsigned long nextFrequencyUpdate = 0;

	unsigned long now = millis();

	if (now >= nextFrequencyUpdate && prev_freq != frequency) {
		updateDisplay();
		nextFrequencyUpdate = now + 500;
		prev_freq = frequency;
	}

	s = getEncoderDir();
	if (!s)
		return;

	doingCAT = 0; // go back to manual mode if you were doing CAT
	prev_freq = frequency;


	if (s > 10)
		frequency += 200l * s;
	else if (s > 5)
		frequency += 100l * s;
	else if (s > 0)
		frequency += 50l * s;
	else if (s < -10)
		frequency += 200l * s;
	else if (s < -5)
		frequency += 100l * s;
	else if (s  < 0)
		frequency += 50l * s;

	if (prev_freq < 10000000l && frequency > 10000000l)
		isUSB = true;

	if (prev_freq > 10000000l && frequency < 10000000l)
		isUSB = false;

	setFrequency(frequency);    
}


/**
 * RIT only steps back and forth by 100 hz at a time
 */
void doRIT() {
	unsigned long newFreq;

	int knob = getEncoderDir();
	unsigned long old_freq = frequency;

	if (knob < 0)
		frequency -= 100l;
	else if (knob > 0)
		frequency += 100;

	if (old_freq != frequency) {
		setFrequency(frequency);
		updateDisplay();
	}
}

/**
 * The settings are read from EEPROM. The first time around, the values may not be 
 * present or out of range, in this case, some intelligent defaults are copied into the 
 * variables.
 */
void initSettings() {
	byte x;
	//read the settings from the eeprom and restore them
	//if the readings are off, then set defaults
	EEPROM.get(MASTER_CAL, calibration);
	EEPROM.get(USB_CAL, usbCarrier);
	EEPROM.get(VFO_A, vfoA);
	EEPROM.get(VFO_B, vfoB);
	EEPROM.get(CW_SIDETONE, sideTone);
	EEPROM.get(CW_SPEED, cwSpeed);
	EEPROM.get(CW_DELAYTIME, cwDelayTime);

	// the screen calibration parameters : int slope_x=104, slope_y=137, offset_x=28, offset_y=29;

	if (usbCarrier > 11060000l || usbCarrier < 11048000l)
		usbCarrier = 11052000l;
	if (vfoA > 35000000l || 3500000l > vfoA)
		vfoA = 7150000l;
	if (vfoB > 35000000l || 3500000l > vfoB)
		vfoB = 14150000l;  
	if (sideTone < 100 || 2000 < sideTone) 
		sideTone = 800;
	if (cwSpeed < 10 || 1000 < cwSpeed) 
		cwSpeed = 100;
	if (cwDelayTime < 10 || cwDelayTime > 100)
		cwDelayTime = 50;

	/*
	 * The VFO modes are read in as either 2 (USB) or 3(LSB), 0, the default
	 * is taken as 'uninitialized
	 */

	EEPROM.get(VFO_A_MODE, x);

	switch(x) {
		case VFO_MODE_USB:
			isUsbVfoA = 1;
			break;
		case VFO_MODE_LSB:
			isUsbVfoA = 0;
			break;
		default:
			if (vfoA > 10000000l)
				isUsbVfoA = 1;
			else 
				isUsbVfoA = 0;      
	}

	EEPROM.get(VFO_B_MODE, x);
	switch(x) {
		case VFO_MODE_USB:
			isUsbVfoB = 1;
			break;
		case VFO_MODE_LSB:
			isUsbVfoB = 0;
			break;
		default:
			if (vfoA > 10000000l)
				isUsbVfoB = 1;
			else 
				isUsbVfoB = 0;      
	}

	//set the current mode
	isUSB = isUsbVfoA;

	/*
	 * The keyer type splits into two variables
	 */
	EEPROM.get(CW_KEY_TYPE, x);

	if (x == 0) {
		Iambic_Key = false;
	} else if (x == 1) {
		Iambic_Key = true;
		keyerControl &= ~IAMBICB;
	} else if (x == 2) {
		Iambic_Key = true;
		keyerControl |= IAMBICB;
	}

}

void initPorts() {

	analogReference(DEFAULT);

	/*pinMode(ENC_A, INPUT_PULLUP);
	pinMode(ENC_B, INPUT_PULLUP);
	pinMode(FBUTTON, INPUT_PULLUP);*/

	pinMode(PTT, INPUT_PULLUP);

	pinMode(CW_TONE, OUTPUT);  
	digitalWrite(CW_TONE, 0);

	pinMode(TX_RX,OUTPUT);
	digitalWrite(TX_RX, 0);

	pinMode(TX_LPF_A, OUTPUT);
	pinMode(TX_LPF_B, OUTPUT);
	pinMode(TX_LPF_C, OUTPUT);
	digitalWrite(TX_LPF_A, 0);
	digitalWrite(TX_LPF_B, 0);
	digitalWrite(TX_LPF_C, 0);

	pinMode(CW_KEY, OUTPUT);
	digitalWrite(CW_KEY, 0);
}


void setup() {
	Serial.begin(MONITOR_BAUD);
	Serial.flush();  

	displayInit();

	initSettings();
	initPorts();     
	setupEncoder();
	initOscillators();
	frequency = vfoA;
	setFrequency(vfoA);

	if (encoderBtnDown()) {
		setupTouch();
		isUSB = 1;
		setFrequency(10000000l);
		setupFreq();
		isUSB = 0;
		setFrequency(7100000l);
		setupBFO();
	}
	guiUpdate();
	displayRawText("v6.1", 270, 210, DISPLAY_LIGHTGREY, DISPLAY_NAVY);
}


void loop() { 
	Serial.print(".");
	if (cwMode) {
		cwKeyer(); 
	} else if (!txCAT) {
		checkPTT();
	}

	Serial.print("-");
	checkButton();
	Serial.print("/");

	if (! inTx){
		if (ritOn) {
			doRIT();
		} else {
			doTuning();
		}
		checkTouch();
	}
	Serial.print("\\");

	checkCAT();

}
