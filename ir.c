/*
 * ir.c
 *
 *  Created on: Apr 30, 2017
 *      Author: lakhanshiva, dharmik thakkar
 */
#include "msp.h"
#include <stdlib.h>
#include <stdint.h>

void irinit(void){
	/*IR Decoding logic*/
	P5->DIR = ~BIT7;
	P5->OUT = BIT7;
	P5->REN = BIT7;                         // Enable pull-up resistor (P1.1 output high)
	P5->SEL0 = 0;
	P5->SEL1 = 0;
	P5->IES = BIT7;                         // Interrupt on high-to-low transition
	P5->IFG = 0;                            // Clear all P1 interrupt flags
	P5->IE = BIT7;                          // Enable interrupt for P5.7
	 // Configure Timer_A
	TIMER_A0->CTL = TIMER_A_CTL_TASSEL_2 |  // SMCLK , 12 MHz
	TIMER_A_CTL_MC_0 |              // Timer is halted
	TIMER_A_CTL_CLR |               // Clear TAR
	TIMER_A_CTL_ID_2;             //Divide 12MHz by 4
	TA0R = 0; //making timer register 0
	NVIC->ISER[1] = 1 << ((PORT5_IRQn) & 31);
	 // Enable global interrupt
	__enable_irq();
}


