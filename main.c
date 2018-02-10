/**************************************************************************//**
 * Name: Lakhan Shiva Kamireddy
 * Date: 03/26/2017
 * SD Card controller implementation
 * @file
 * @brief
	This is a program for SD card controller
 * @version 1.0.0
 ******************************************************************************
 * @section License
 * <b>Copyright 2017
 *******************************************************************************/
/*****************************************************************************
*
* Copyright (C) 2013 - 2016 Texas Instruments Incorporated - http://www.ti.com/
*
* Redistribution and use in source and binary forms, with or without
* modification, are permitted provided that the following conditions
* are met:
*
* * Redistributions of source code must retain the above copyright
*   notice, this list of conditions and the following disclaimer.
*
* * Redistributions in binary form must reproduce the above copyright
*   notice, this list of conditions and the following disclaimer in the
*   documentation and/or other materials provided with the
*   distribution.
*
* * Neither the name of Texas Instruments Incorporated nor the names of
*   its contributors may be used to endorse or promote products derived
*   from this software without specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
* "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
* LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
* A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
* OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
* SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
* LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
* DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
* THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
* (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
* OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*
******************************************************************************
*
* SPI for SD card code
*
* for GNU compiler
*
******************************************************************************/

#include "msp.h"
#include <stdint.h>
#include <stdbool.h>

void transmit(uint8_t txData){
	              EUSCI_B2->IFG &=~ EUSCI_B_IFG_TXIFG;
				  EUSCI_B2->TXBUF = txData;
	              while(!(EUSCI_B2->IFG & EUSCI_B_IFG_TXIFG)){};
	              EUSCI_B2->IFG &=~ EUSCI_B_IFG_TXIFG;
}

uint8_t receive(){
				  uint8_t rxData;
				  rxData = EUSCI_B2->RXBUF;
				  while(!(EUSCI_B2->IFG & EUSCI_B_IFG_RXIFG) && (rxData==0xFF)){
					  transmit(0xFF);
					  rxData = EUSCI_B2->RXBUF;
				  }
	              EUSCI_B2->IFG &=~ EUSCI_B_IFG_RXIFG;
	              return rxData;
}

void sd_init(){
	uint8_t data = 0xFF;
	uint8_t received_data;
	int i,j;

    j=0;
    EUSCI_B2->IFG &=~ EUSCI_B_IFG_TXIFG;
    for(i=0;i<3000;i++){};
    while(1){
//To put the SD card into SPI mode, we make MOSI as logic 1 and CS as logic 1 for 74 cycles (or more)
//After 74 cycles we set the CS line to 0 and send the CMD0 01 000000 00000000 00000000 00000000 00000000 1001010 1
//This is reset command
		EUSCI_B2->TXBUF = data;
		while(!(EUSCI_B2->IFG & EUSCI_B_IFG_TXIFG)){};
		EUSCI_B2->IFG &=~ EUSCI_B_IFG_TXIFG;
		j++;
		while(!(EUSCI_B2->IFG & EUSCI_B_IFG_RXIFG)){};
		EUSCI_B2->IFG &=~ EUSCI_B_IFG_RXIFG;
		if(j==10){
			for(i=0;i<2000;i++){};
			P1->OUT &= ~BIT5;
			break;
		}
    }
	  EUSCI_B2->IFG &=~ EUSCI_B_IFG_RXIFG;

	  //CMD0
	                transmit(0x40);
	                transmit(0x00);
	                transmit(0x00);
	                transmit(0x00);
	                transmit(0x00);
	                transmit(0x95);
	                received_data = receive();
}



int main(void)
{
	WDTCTL = WDTPW | WDTHOLD;                    // Stop watchdog timer to prevent the WDT from resetting the board. Not used now.

	//******************************************************************************
                  //   MSP432P401R
                  //
                  //   Description: eUSCI SPI mode
                  //
                  //   MSP432 SPI */
    /*Confguration clock 24 Mhz, SMCLK at 12Mhz */
      CS->KEY = CS_KEY_VAL;                   // Unlock CS module for register access
          CS->CTL0 = 0;                           // Reset tuning parameters
          CS->CTL0 = CS_CTL0_DCORSEL_4;           // Set DCO to 24MHz (nominal, center of 8-16MHz range)
          CS->CTL1 |= (CS_CTL1_SELS_3 | CS_CTL1_DIVS_1); //// For SMCLK = 12Mhz, select DCO as source then divide by 2
          CS->KEY = 0;                            // Lock CS module from unintended accesses


          // Configure SPI
          EUSCI_B2->CTLW0 = 0;
          EUSCI_B2->CTLW0 |= EUSCI_B_CTLW0_SWRST; // Put eUSCI in reset
          EUSCI_B2->CTLW0 |= EUSCI_B_CTLW0_MST; //MSP432 in master mode
          EUSCI_B2->CTLW0 |= EUSCI_B_CTLW0_MSB; //MSB first mode
          EUSCI_B2->CTLW0 |= EUSCI_B_CTLW0_SYNC; //Synchronous
          EUSCI_B2->CTLW0 |= (EUSCI_B_CTLW0_CKPL);// | EUSCI_B_CTLW0_CKPH) ; //Clock polarity and Clock phase, make them both 11
          EUSCI_B2->CTLW0 |= EUSCI_B_CTLW0_SSEL__SMCLK; //Clock source selected as ACLK
          EUSCI_B2->BRW = 34; //This divides output frequency, by 34, resulting operating frequency is 333 kHz
          EUSCI_B2->CTLW0 &= ~EUSCI_B_CTLW0_SWRST;// Initialize USCI state machine


          //Configure GPIO pins
          //To configure GPIO P3 that is controlled by PMAPC, we set P3SEL1.x and P3SEL0.x to 01
          P3->SEL0 |= BIT5 | BIT6 | BIT7;
          P3->SEL1 &= ~(BIT5 | BIT6 | BIT7);

          //restart the state machine
          EUSCI_B2->CTLW0 &= ~(UCSWRST);

          //enable interrupts
          UCB2IE |= UCTXIE;
          UCB2IE |= UCRXIE;
          //Configuring the CS GPIO pin
              P1->DIR = BIT5;   //Sets P1.5 as an output pin
              P1->OUT = BIT5;   //Sets P1.5
              sd_init(); //initialize SD card and put it into SPI mode

    while(1);
    return 0;
}


