/* DriverLib Includes */
#include "driverlib.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include "player.h"
#include "vs1063a-patches.plg"

#define FILE_BUFFER_SIZE 512
#define SDI_MAX_TRANSFER_SIZE 32
#define SDI_END_FILL_BYTES_FLAC 12288
#define SDI_END_FILL_BYTES       2050
#define REC_BUFFER_SIZE 512


#define SPEED_SHIFT_CHANGE 128

/* How many transferred bytes between collecting data.
   A value between 1-8 KiB is typically a good value.
   If REPORT_ON_SCREEN is defined, a report is given on screen each time
   data is collected. */
#define REPORT_INTERVAL 4096
#if 1
#define REPORT_ON_SCREEN
#endif

/* Define PLAYER_USER_INTERFACE if you want to have a user interface in your
   player. */
#if 1
#define PLAYER_USER_INTERFACE
#endif

/* Define RECORDER_USER_INTERFACE if you want to have a user interface in your
   player. */
#if 1
#define RECORDER_USER_INTERFACE
#endif


#define min(a,b) (((a)<(b))?(a):(b))



enum AudioFormat {
  afUnknown,
  afRiff,
  afOggVorbis,
  afMp1,
  afMp2,
  afMp3,
  afAacMp4,
  afAacAdts,
  afAacAdif,
  afFlac,
  afWma,
} audioFormat = afUnknown;

const char *afName[] = {
  "unknown",
  "RIFF",
  "Ogg",
  "MP1",
  "MP2",
  "MP3",
  "AAC MP4",
  "AAC ADTS",
  "AAC ADIF",
  "FLAC",
  "WMA",
};

/*Produces a delay between instructions*/
void delay(int delay_s){
	int i;
	for(i=0; i<50*delay_s; i++){

	}
}

/*
 * Initializes SPI communication with VS1063 IC
 */
void spiinit(){
//Confguration clock 24 Mhz, SMCLK at 12Mhz

      CS->KEY = CS_KEY_VAL;                   // Unlock CS module for register access
          CS->CTL0 = 0;                           // Reset tuning parameters
          CS->CTL0 = CS_CTL0_DCORSEL_4;           // Set DCO to 24MHz (nominal, center of 8-16MHz range)
          CS->CTL1 |= (CS_CTL1_SELS_3 | CS_CTL1_DIVS_1); //// For SMCLK = 12Mhz, select DCO as source then divide by 2
          CS->KEY = 0;                            // Lock CS module from unintended accesses


          // Configure SPI
          EUSCI_B0->CTLW0 = 0;
          EUSCI_B0->CTLW0 |= EUSCI_B_CTLW0_SWRST; // Put eUSCI in reset
          EUSCI_B0->CTLW0 |= EUSCI_B_CTLW0_MST; //MSP432 in master mode
          EUSCI_B0->CTLW0 |= EUSCI_B_CTLW0_MSB; //MSB first mode
          EUSCI_B0->CTLW0 |= EUSCI_B_CTLW0_SYNC; //Synchronous
          EUSCI_B0->CTLW0 |= (EUSCI_B_CTLW0_CKPH);// | EUSCI_B_CTLW0_CKPH) ; //Clock polarity and Clock phase, make them both 10
          EUSCI_B0->CTLW0 |= EUSCI_B_CTLW0_SSEL__SMCLK; //Clock source selected as ACLK
          EUSCI_B0->BRW = 6; //This divides output frequency, by 34, resulting operating frequency is 333 kHz
          EUSCI_B0->CTLW0 &= ~EUSCI_B_CTLW0_SWRST;// Initialize USCI state machine


          //Configure GPIO pins
          //To configure GPIO P3 that is controlled by PMAPC, we set P1SEL1.x and P1SEL0.x to 01
          P1->SEL0 |= BIT5 | BIT6 | BIT7;
          P1->SEL1 &= ~(BIT5 | BIT6 | BIT7);

          //restart the state machine
          EUSCI_B0->CTLW0 &= ~(UCSWRST);

}

/*
 * Transmit data to the VS1063 decoder
 */
void WriteSpiByteSDI(u_int8 data){
	   int i=0;
	  while(!(EUSCI_B0->IFG & EUSCI_B_IFG_TXIFG)){
		  i++;
		  if(i==100){
			  break;
		  }
	  }
	  EUSCI_B0->TXBUF = data;

}

/*
 * Transmit a byte of data to VS1063 decoder
 */
void WriteSpiByte(u_int8 data){

	  EUSCI_B0->IFG &=~ EUSCI_B_IFG_TXIFG;
	  EUSCI_B0->TXBUF = data;
	  while(!(EUSCI_B0->IFG & EUSCI_B_IFG_TXIFG)){};
	  EUSCI_B0->IFG &=~ EUSCI_B_IFG_TXIFG;

}


/*
 * Read a byte of data to VS1063 decoder
 */
u_int8 ReadSpiByte(void){

	  u_int8 data;
	  data = EUSCI_B0->RXBUF;
	  while(!(EUSCI_B0->IFG & EUSCI_B_IFG_RXIFG) && (data==0xFF)){
	  }
	  data = EUSCI_B0->RXBUF;
	  EUSCI_B0->IFG &=~ EUSCI_B_IFG_RXIFG;
	  return data;

}


/*
 Reads SCI data from the VS1063 decoder
 */
u_int16 ReadSci(u_int8 addr){

	u_int16 result;
	while(P6->IN & BIT5 == 0); //Wait until DREQ is high
	P3->OUT &= ~BIT2; //Activate CS - make it low
	EUSCI_B0->IFG &=~ EUSCI_B_IFG_RXIFG;
	WriteSpiByte(0x03); //Read Command Code
	WriteSpiByte(addr); //SCI Register number
	EUSCI_B0->TXBUF = 0xFF;
	while(!(EUSCI_B0->IFG & EUSCI_B_IFG_TXIFG)){};
	EUSCI_B0->TXBUF = 0xFF;
	while(!(EUSCI_B0->IFG & EUSCI_B_IFG_TXIFG)){};
    EUSCI_B0->IFG &=~ EUSCI_B_IFG_TXIFG;
	result = (u_int16)ReadSpiByte() <<8;
	EUSCI_B0->TXBUF = 0xFF;
	result |= ReadSpiByte();
	delay(10);
	P3->OUT |= BIT2; //Deactivate CS - make it high
	return result;

}

/*
 * Writes SCI data to VS1063 decoder IC
 */
void WriteSci(u_int8 addr, u_int16 data){

	while(P6->IN & BIT5 == 0); //Wait until DREQ is high
	P3->OUT &= ~BIT2; //Activate CS - make it low
	WriteSpiByte(0x02); //Write command code
	WriteSpiByte(addr); //SCI register number
	WriteSpiByte((u_int8)(data >> 8));
	WriteSpiByte((u_int8)(data & 0xFF));
	delay(10);
	P3->OUT |= BIT2; //Deactivate CS - make it high

}

/*Writes SDI data to VS1063 decoder IC*/
int WriteSdi(const u_int8 *data, u_int8 bytes){

	u_int8 i;

	if(bytes > 32)
		return -1;  //Error, too many bytes to transfer

	while(P6->IN & BIT5 == 0); //Wait until DREQ is high
	for(i=0; i<bytes; i++){
		WriteSpiByteSDI(*data++);
	}
	return 0; //OK

}

void SaveUIState(void){
	//Functionality not used for project, reserved for future use
}
void RestoreUIState(void){
	//Functionality not used for project, reserved for future use
}
int GetUICommand(void){
	//Functionality not used for project, reserved for future use
	return 0;
}


/*
  Read 32-bit increasing counter value from addr.
  Because the 32-bit value can change while reading it,
  read MSB's twice and decide which is the correct one.
*/
u_int32 ReadVS10xxMem32Counter(u_int16 addr) {
  u_int16 msbV1, lsb, msbV2;
  u_int32 res;

  WriteSci(SCI_WRAMADDR, addr+1);
  msbV1 = ReadSci(SCI_WRAM);
  WriteSci(SCI_WRAMADDR, addr);
  lsb = ReadSci(SCI_WRAM);
  msbV2 = ReadSci(SCI_WRAM);
  if (lsb < 0x8000U) {
    msbV1 = msbV2;
  }
  res = ((u_int32)msbV1 << 16) | lsb;
  
  return res;
}


/*
  Read 32-bit non-changing value from addr.
*/
u_int32 ReadVS10xxMem32(u_int16 addr) {
  u_int16 lsb;
  WriteSci(SCI_WRAMADDR, addr);
  lsb = ReadSci(SCI_WRAM);
  return lsb | ((u_int32)ReadSci(SCI_WRAM) << 16);
}


/*
  Read 16-bit value from addr.
*/
u_int16 ReadVS10xxMem(u_int16 addr) {
  WriteSci(SCI_WRAMADDR, addr);
  return ReadSci(SCI_WRAM);
}


/*
  Write 16-bit value to given VS10xx address
*/
void WriteVS10xxMem(u_int16 addr, u_int16 data) {
  WriteSci(SCI_WRAMADDR, addr);
  WriteSci(SCI_WRAM, data);
}

/*
  Write 32-bit value to given VS10xx address
*/
void WriteVS10xxMem32(u_int16 addr, u_int32 data) {
  WriteSci(SCI_WRAMADDR, addr);
  WriteSci(SCI_WRAM, (u_int16)data);
  WriteSci(SCI_WRAM, (u_int16)(data>>16));
}






/*
  Loads a plugin.
  This function is called in init software function
*/
void LoadPlugin(const u_int16 *d, u_int16 len) {
  int i = 0;

  while (i<len) {
    unsigned short addr, n, val;
    addr = d[i++];
    n = d[i++];
    if (n & 0x8000U) { /* RLE run, replicate n samples */
      n &= 0x7FFF;
      val = d[i++];
      while (n--) {
        WriteSci(addr, val);
      }
    } else {           /* Copy run, copy n samples */
      while (n--) {
        val = d[i++];
        WriteSci(addr, val);
      }
    }
  }
}




enum PlayerStates {
  psPlayback = 0,
  psUserRequestedCancel,
  psCancelSentToVS10xx,
  psStopped
} playerState;



void VS1063PlayFile(FILE *readFp) {
//Functionality not used for project, reserved for future use
}



void VS1063RecordFile(FILE *writeFp) {
//Functionality not used for project, reserved for future use
}





/*

  Hardware Initialization for VS1063.

  
*/
int VSTestInitHardware(void) {
  /* Write here your microcontroller code which puts VS10xx in hardware
     reset anc back (set xRESET to 0 for at least a few clock cycles,
     then to 1). */
  P6->DIR &= ~BIT5; //Makes DREQ (P6.5) as input
  P3->DIR |= BIT2; //Makes CS (P3.2) as output
//Pin 6.0 is connected to XRESET / RST -> When it is driven low that means we have reset the circuit.
  P6->DIR |= BIT0;   //makes RST (P6.0) as output
  P6->DIR |= BIT4;   //makes XDCS (P6.4) as output
  P6->OUT &= ~BIT0;
  delay(100);

//Now we assert XRESET by making it high
  P6->OUT |= BIT0;

//Now DREQ will stay down for around 22000 clock cycles (give 1.8 ms delay)
delay(300);
  spiinit();
  return 0;
}



/* Note: code SS_VER=2 is used for both VS1002 and VS1011e */
const u_int16 chipNumber[16] = {
  1001, 1011, 1011, 1003, 1053, 1033, 1063, 1103,
  0, 0, 0, 0, 0, 0, 0, 0
};

/*

  Software Initialization for VS1063.

  Note that you need to check whether SM_SDISHARE should be set in
  your application or not.
  
*/
int VSTestInitSoftware(void) {
  u_int16 ssVer;

  /* Start initialization with a dummy read, which makes sure our
     microcontoller chips selects and everything are where they
     are supposed to be and that VS10xx's SCI bus is in a known state. */
  ReadSci(SCI_MODE);

  /* First real operation is a software reset. After the software
     reset we know what the status of the IC is. You need, depending
     on your application, either set or not set SM_SDISHARE. See the
     Datasheet for details. */
  WriteSci(SCI_MODE, SM_SDINEW|SM_SDISHARE|SM_TESTS|SM_RESET);
  ReadSci(SCI_MODE);

  /* A quick sanity check: write to two registers, then test if we
     get the same results. Note that if you use a too high SPI
     speed, the MSB is the most likely to fail when read again. */
  WriteSci(SCI_AICTRL1, 0xABAD);
  WriteSci(SCI_AICTRL2, 0x7E57);
  if (ReadSci(SCI_AICTRL1) != 0xABAD || ReadSci(SCI_AICTRL2) != 0x7E57) {
    return 1;
  }
  WriteSci(SCI_AICTRL1, 0);
  WriteSci(SCI_AICTRL2, 0);

  /* Check VS1063 type */
  ssVer = ((ReadSci(SCI_STATUS) >> 4) & 15);
  if (chipNumber[ssVer]) {
    if (chipNumber[ssVer] != 1063) {
      return 1;
    }
  } else {
    return 1;
  }

  /* Set the clock. Until this point we need to run SPI slow so that
     we do not exceed the maximum speeds mentioned in
     Chapter SPI Timing Diagram in the Datasheet. */

  WriteSci(SCI_CLOCKF,
           HZ_TO_SC_FREQ(12288000) | SC_MULT_53_40X | SC_ADD_53_15X);

  /* Now when we have upped the VS10xx clock speed, the microcontroller
     SPI bus can run faster. Do that before you start playing or
     recording files. */

  /* Set up other parameters. */
 WriteVS10xxMem(PAR_CONFIG1, PAR_CONFIG1_AAC_SBR_SELECTIVE_UPSAMPLE);

  /* Set volume level at -6 dB of maximum */
  WriteSci(SCI_VOL, 0x2424);

  /* Now it's time to load the proper patch set. */
  LoadPlugin(plugin, sizeof(plugin)/sizeof(plugin[0]));

  /* We're ready to go. */
  return 0;
}




int VSTestHandleFile(const char *fileName, int record) {
  //Functionality not used for project, reserved for future use
  return 0;
}
