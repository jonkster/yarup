#include "ubitx_si5351.h"

#define BB0(x) ((uint8_t)x)             // Bust int32 into Bytes
#define BB1(x) ((uint8_t)(x>>8))
#define BB2(x) ((uint8_t)(x>>16))

#define SI5351BX_ADDR 0x60              // I2C address of Si5351   (typical)
#define SI5351BX_XTALPF 2               // 1:6pf  2:8pf  3:10pf

// If using 27mhz crystal, set XTAL=27000000, MSA=33.  Then vco=891mhz
#define SI5351BX_XTAL 25000000          // Crystal freq in Hz
#define SI5351BX_MSA  35                // VCOA is at 25mhz*35 = 875mhz

// User program may have reason to poke new values into these 3 RAM variables
uint32_t si5351bx_vcoa = (SI5351BX_XTAL*SI5351BX_MSA);  // 25mhzXtal calibrate
uint8_t  si5351bx_rdiv = 0;             // 0-7, CLK pin sees fout/(2**rdiv)
uint8_t  si5351bx_drive[3] = {3, 3, 3}; // 0=2ma 1=4ma 2=6ma 3=8ma for CLK 0,1,2
uint8_t  si5351bx_clken = 0xFF;         // Private, all CLK output drivers off
int32_t calibration = 0;

void i2cWrite(uint8_t reg, uint8_t val) {   // write reg via i2c
  Wire.beginTransmission(SI5351BX_ADDR);
  Wire.write(reg);
  Wire.write(val);
  Wire.endTransmission();
}

void i2cWriten(uint8_t reg, uint8_t *vals, uint8_t vcnt) {  // write array
  Wire.beginTransmission(SI5351BX_ADDR);
  Wire.write(reg);
  while (vcnt--) Wire.write(*vals++);
  Wire.endTransmission();
}


void si5351bx_init() {                  // Call once at power-up, start PLLA
  uint8_t reg;  uint32_t msxp1;
  Wire.begin();
  i2cWrite(149, 0);                     // SpreadSpectrum off
  i2cWrite(3, si5351bx_clken);          // Disable all CLK output drivers
  i2cWrite(183, SI5351BX_XTALPF << 6);  // Set 25mhz crystal load capacitance
  msxp1 = 128 * SI5351BX_MSA - 512;     // and msxp2=0, msxp3=1, not fractional
  uint8_t  vals[8] = {0, 1, BB2(msxp1), BB1(msxp1), BB0(msxp1), 0, 0, 0};
  i2cWriten(26, vals, 8);               // Write to 8 PLLA msynth regs
  i2cWrite(177, 0x20);                  // Reset PLLA  (0x80 resets PLLB)
  // for (reg=16; reg<=23; reg++) i2cWrite(reg, 0x80);    // Powerdown CLK's
  // i2cWrite(187, 0);                  // No fannout of clkin, xtal, ms0, ms4

  //initializing the ppl2 as well
  i2cWriten(34, vals, 8);               // Write to 8 PLLA msynth regs
  i2cWrite(177, 0xa0);                  // Reset PLLA  & PPLB (0x80 resets PLLB)

}

void si5351bx_setfreq(uint8_t clknum, uint32_t fout) {  // Set a CLK to fout Hz
  uint32_t  msa, msb, msc, msxp1, msxp2, msxp3p2top;
  if ((fout < 500000) || (fout > 109000000)) // If clock freq out of range
    si5351bx_clken |= 1 << clknum;      //  shut down the clock
  else {
    msa = si5351bx_vcoa / fout;     // Integer part of vco/fout
    msb = si5351bx_vcoa % fout;     // Fractional part of vco/fout
    msc = fout;             // Divide by 2 till fits in reg
    while (msc & 0xfff00000) {
      msb = msb >> 1;
      msc = msc >> 1;
    }
    msxp1 = (128 * msa + 128 * msb / msc - 512) | (((uint32_t)si5351bx_rdiv) << 20);
    msxp2 = 128 * msb - 128 * msb / msc * msc; // msxp3 == msc;
    msxp3p2top = (((msc & 0x0F0000) << 4) | msxp2);     // 2 top nibbles
    uint8_t vals[8] = { BB1(msc), BB0(msc), BB2(msxp1), BB1(msxp1),
                        BB0(msxp1), BB2(msxp3p2top), BB1(msxp2), BB0(msxp2)
                      };
    i2cWriten(42 + (clknum * 8), vals, 8); // Write to 8 msynth regs
//    if (clknum == 1)      //PLLB | MS src | drive current
//      i2cWrite(16 + clknum, 0x20 | 0x0C | si5351bx_drive[clknum]); // use local msynth   
//    else
      i2cWrite(16 + clknum, 0x0C | si5351bx_drive[clknum]); // use local msynth
   
    si5351bx_clken &= ~(1 << clknum);   // Clear bit to enable clock
  }
  i2cWrite(3, si5351bx_clken);        // Enable/disable clock
}

void si5351_set_calibration(int32_t cal){
    si5351bx_vcoa = (SI5351BX_XTAL * SI5351BX_MSA) + cal; // apply the calibration correction factor
    si5351bx_setfreq(0, usbCarrier);
}

void initOscillators(){
  //initialize the SI5351
  si5351bx_init();
  si5351bx_vcoa = (SI5351BX_XTAL * SI5351BX_MSA) + calibration; // apply the calibration correction factor
  si5351bx_setfreq(0, usbCarrier);
}



