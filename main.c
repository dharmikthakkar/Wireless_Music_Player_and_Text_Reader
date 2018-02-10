/*
 * -------------------------------------------
 *    MSP432 DriverLib - v3_21_00_05 
 * -------------------------------------------
 *
 * --COPYRIGHT--,BSD,BSD
 * Copyright (c) 2016, Texas Instruments Incorporated
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * *  Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *
 * *  Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * *  Neither the name of Texas Instruments Incorporated nor the names of
 *    its contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 * --/COPYRIGHT--*/
/******************************************************************************
 * MSP432 Empty Project
 *
 * Description: An empty project that uses DriverLib
 *
 *                MSP432P401
 *             ------------------
 *         /|\|                  |
 *          | |                  |
 *          --|RST               |
 *            |                  |
 *            |                  |
 *            |                  |
 *            |                  |
 *            |                  |
 * Author: 
*******************************************************************************/
/* DriverLib Includes */
#include "driverlib.h"

#include "msp.h"
/* Standard Includes */
#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>

/*FAT File system includes*/
#include "diskio.h"
#include "ff.h"
//#include "fatfs/ffconf.h"
#include "integer.h"
//#include "fatfs/option/syscall.c"
//#include "fatfs/option/unicode.c"
//#include "fatfs/option/syscall.c"

/*MP3 Decoder includes*/
#include "vs1063_uc.h"
#include "player.h"

#define VOL_DIFF 500;

/*LCD includes*/
#include "lcd.h"

/*IR Decoder library*/
#include "ir.h"

/*IR Remote Macros*/
const uint16_t BUTTON_POWER = 0xD827; // i.e. 0x10EFD827
const uint16_t BUTTON_A = 0xF807;
const uint16_t BUTTON_B = 0x7887;
const uint16_t BUTTON_C = 0x58A7;
const uint16_t BUTTON_UP = 0xA05F;
const uint16_t BUTTON_DOWN = 0x00FF;
const uint16_t BUTTON_LEFT = 0x10EF;
const uint16_t BUTTON_RIGHT = 0x807F;
const uint16_t BUTTON_CIRCLE = 0x20DF;

#define detect_0 0
#define detect_1 1
#define detect_start 2
#define detect_repeat 3
#define detect_undefined 4

/*Global Variables for IR Decoder*/
uint32_t data_byte = 0x00;
uint16_t button_type = 0x00;
int valid_button = 0;
int irq_i = 0;
int array[34];
int data_received_flag = 0;
volatile int i = 0;
volatile unsigned int start_flag = 0;
uint16_t received_option = 0x00;
int ta_error = 0;
int k =0;
unsigned char current_volume = 0x2424;
char vol_level[3]; //volume level

/*LCD read variables*/
int read_mode = 0; //when read_mode is 1, user is reading a file.
int music_mode = 0;//when music_mode is 1, user is listening to music
int is_paused = 0;//when is paused is 1, user has paused the song that he is currently listening to
int current_music_option = 0; //music options
/*
 * Pin Configurations on our board
 *Reset P6.0
DREQ P6.5
CS P3.2
BSYNC P6.4
 */


extern void disk_timerproc (void);


DWORD AccSize;              /* Work register for fs command */
WORD AccFiles, AccDirs;
FILINFO Finfo;

char Line[256];             /* Console input buffer */
BYTE Buff[20] __attribute__ ((aligned (4))) ;    /* Working buffer */
BYTE Buff2[1024] __attribute__ ((aligned (4))) ;    /* Working buffer */

FATFS FatFs;                /* File system object for each logical drive */
FIL File[2];                /* File objects */
DIR Dir;                    /* Directory object */
FIL my_fil;
const TCHAR *my_path = "test1.txt";
const TCHAR *music_path = "test1.txt";
UINT s1;

volatile UINT Timer;


/*---------------------------------------------*/
/* 1kHz timer process                          */
/*---------------------------------------------*/

void SysTick_Handler (void)
{

    disk_timerproc();   /* Disk timer process */
}



/*---------------------------------------------------------*/
/* User Provided RTC Function for FatFs module             */
/*---------------------------------------------------------*/
/* This is a real time clock service to be called from     */
/* FatFs module. Any valid time must be returned even if   */
/* the system does not support an RTC.                     */
/* This function is not required in read-only cfg.         */


DWORD get_fattime (void)
{
 /*   RTC rtc;

     Get local time
    rtc_gettime(&rtc);

     Pack date and time into a DWORD variable
    return    ((DWORD)(rtc.year - 1980) << 25)
            | ((DWORD)rtc.month << 21)
            | ((DWORD)rtc.mday << 16)
            | ((DWORD)rtc.hour << 11)
            | ((DWORD)rtc.min << 5)
            | ((DWORD)rtc.sec >> 1);
*/
    return 0;
 }


/*--------------------------------------------------------------------------*/
/* Monitor                                                                  */
/*--------------------------------------------------------------------------*/

static
FRESULT scan_files (
    char* path      /* Pointer to the path name working buffer */
)
{
    DIR dirs;
    FRESULT res;
    BYTE i;


    if ((res = f_opendir(&dirs, path)) == FR_OK) {
        while (((res = f_readdir(&dirs, &Finfo)) == FR_OK) && Finfo.fname[0]) {
            if (Finfo.fattrib & AM_DIR) {
                AccDirs++;
                i = strlen(path);
                path[i] = '/'; strcpy(&path[i+1], Finfo.fname);
                res = scan_files(path);
                path[i] = '\0';
                if (res != FR_OK) break;
            } else {
            /*  xprintf("%s/%s\n", path, fn); */
                AccFiles++;
                AccSize += Finfo.fsize;
            }
        }
    }

    return res;
}



static
void put_rc (FRESULT rc)
{
    const char *str =
        "OK\0" "DISK_ERR\0" "INT_ERR\0" "NOT_READY\0" "NO_FILE\0" "NO_PATH\0"
        "INVALID_NAME\0" "DENIED\0" "EXIST\0" "INVALID_OBJECT\0" "WRITE_PROTECTED\0"
        "INVALID_DRIVE\0" "NOT_ENABLED\0" "NO_FILE_SYSTEM\0" "MKFS_ABORTED\0" "TIMEOUT\0"
        "LOCKED\0" "NOT_ENOUGH_CORE\0" "TOO_MANY_OPEN_FILES\0" "INVALID_PARAMETER\0";
    FRESULT i;

    for (i = 0; i != rc && *str; i++) {
        while (*str++) ;
    }
    xprintf("rc=%u FR_%s\n", (UINT)rc, str);
}

unsigned char min(unsigned char x, unsigned char y){
    if(x>y) return y;
    else return x;
}

/*IR decoding logic*/

uint16_t decode(){
	int j;
	if(data_received_flag){
		for(j=0;j<32;j++){
			//decode each bit
			if(array[j] > 800 && array[j] < 2500){
						//This is a 0 bit
						data_byte |= detect_0;
						array[j] = 0;
					}
			else if(array[j] > 3000 && array[j] < 5500){
						//This is a 1 bit
						data_byte |= detect_1;
						array[j] = 0;
					}
			else{
						//unable to decode the bit
					}
			if(j<31){
				data_byte <<= 1;
			}
		}
		if((data_byte & 0xFFFF0000) >> 16 == 0x10EF){
			button_type = data_byte & 0x0000FFFF;
			data_byte = 0x00;
			valid_button = 0;
			P1->OUT &= ~BIT0;
		}
		else{
			button_type = 0x0000;
			valid_button = -1;
		}
		irq_i =0;
		data_received_flag = 0;
		P5->IE = BIT7; //Enable interrupt
		NVIC->ISER[1] = 1 << ((PORT5_IRQn) & 31);
		return button_type;
	}
	else return 0;
}

void getvolumerange(){
	if(current_volume >= 0x0000 && current_volume <= 0x1000){
		vol_level[0] = '6';
		vol_level[1] = '\0';
	}
	else if(current_volume > 0x1000 && current_volume <= 0x2000){
		vol_level[0] = '5';
		vol_level[1] = '\0';
	}
	else if(current_volume > 0x2000 && current_volume <= 0x3000){
		vol_level[0] = '4';
		vol_level[1] = '\0';
	}
	else if(current_volume > 0x3000 && current_volume <= 0x4000){
		vol_level[0] = '3';
		vol_level[1] = '\0';
	}
	else if(current_volume > 0x4000 && current_volume <= 0x5000){
		vol_level[0] = '2';
		vol_level[1] = '\0';
	}
	else{
		vol_level[0] = '1';
		vol_level[1] = '\0';
	}
}

int playmusic(int option){
	FRESULT fres;
	unsigned int cnt = 0;
	UINT s2;
	int result_codec=0;
	unsigned short reg[16];

	if(option == 0){
		music_path = "bird.wav";
	}
	else if(option == 1){
		music_path = "cat.wav";
	}
	else if(option == 2){
		music_path = "rooster.wav";
	}
	else{
		music_path = "bird.wav";
	}
								 WriteSci(SCI_DECODE_TIME, 0);         // Reset DECODE_TIME
				                 EUSCI_B0->IFG |= EUSCI_B_IFG_TXIFG;
				                 fres = f_open(&File[1], music_path, FA_READ);
				                 cnt = 0;
				                 P6->OUT |= BIT4;
				                 while(1){
				                     fres = f_read(&File[1], Buff2,  128, &s2);
				                     cnt = cnt + s2;
				                     if(fres || s2 == 0) break;
				                     P3->OUT |= BIT2; //Deactivate CS - make it high

				                     for(i=0; i<4; i++){
				                    	 while(1){
				                    		 if(data_received_flag == 1 || is_paused){//if the song is paused, it will enter this while loop

				                    			 received_option = decode();
				                    			 if(received_option == 0x20DF){
				                    			 			   if(is_paused) {
				                    			 				   is_paused = 0;
				                    			 				   break;
				                    			 			   }
				                    			 			   else is_paused = 1;
				                    			  }
				                    		 }
				                    		 else{
				                    			 break;
				                    		 }
				                    	 }
				                         result_codec = WriteSdi(Buff2+(i*32), min(32, s2));
				                     }
				                     while(!(EUSCI_B0->IFG & EUSCI_B_IFG_TXIFG)){};

				                     P3->OUT &=~ BIT2; //Activate CS - make it low
				                     reg[8]=ReadSci(SCI_HDAT0);
				                     reg[9]=ReadSci(SCI_HDAT1);
				                 }

				                     fres = f_close(&File[1]);
				                     //  P6->OUT |= BIT4;
				                     WriteSci(SCI_MODE, SM_CANCEL);
				                     WriteSci(SCI_MODE, SM_SDINEW|SM_SDISHARE|SM_TESTS|SM_RESET);
				                     return 0;
}


int main(void)
{

    char *ptr, *ptr2;
    signed int error_flag = 0;
    long p1, p2, p3;
    BYTE res, b, drv = 0;
    UINT s1, s2, cnt, blen = sizeof Buff;
    static const BYTE ft[] = {0, 12, 16, 32};
    DWORD ofs = 0, sect = 0, blk[2];
    FATFS *fs;
  //  RTC rtc;
    FRESULT fres;
    int result_codec=0;
    unsigned char i=0;
    unsigned short reg[16];

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

          P1->DIR |= BIT0; //Set GPIO Pin as output
          P1->OUT &= ~BIT0; //Make the GPIO pin low.

          /* initialize systick timer */

          /* Configure SysTick for a 100Hz interrupt.  The FatFs driver wants a 10 ms
           * tick.
           */
          SysTick_setPeriod(3000000 / 100);
          SysTick_enableModule();
          SysTick_enableInterrupt();

          //LCD initialization
          lcdinit();
          lcdmenu();
          lcdpopulatefiles(1);

          /*IR decoding initialization*/
          irinit();

          // Configure SPI
          EUSCI_B2->CTLW0 = 0;
          EUSCI_B2->CTLW0 |= EUSCI_B_CTLW0_SWRST; // Put eUSCI in reset
          EUSCI_B2->CTLW0 |= EUSCI_B_CTLW0_MST; //MSP432 in master mode
          EUSCI_B2->CTLW0 |= EUSCI_B_CTLW0_MSB; //MSB first mode
          EUSCI_B2->CTLW0 |= EUSCI_B_CTLW0_SYNC; //Synchronous
          EUSCI_B2->CTLW0 |= (EUSCI_B_CTLW0_CKPL);// | EUSCI_B_CTLW0_CKPH) ; //Clock polarity and Clock phase, make them both 10
          EUSCI_B2->CTLW0 |= EUSCI_B_CTLW0_SSEL__SMCLK; //Clock source selected as ACLK
          EUSCI_B2->BRW = 34; //This divides output frequency, by 34, resulting operating frequency is 333 kHz
          EUSCI_B2->CTLW0 &= ~EUSCI_B_CTLW0_SWRST;// Initialize USCI state machine


          //Configure GPIO pins
          //To configure GPIO P3 that is controlled by PMAPC, we set P3SEL1.x and P3SEL0.x to 01
          P3->SEL0 |= BIT5 | BIT6 | BIT7;
          P3->SEL1 &= ~(BIT5 | BIT6 | BIT7);

          //restart the state machine
          EUSCI_B2->CTLW0 &= ~(UCSWRST);

          //Configuring the CS GPIO pin
              P3->DIR |= BIT0;   //Sets P3.0 as an output pin

              if (VSTestInitHardware() || VSTestInitSoftware()) {
              //printf("Failed initializing VS10xx, exiting\n");
              //exit(EXIT_FAILURE);
                   error_flag = -1;
              }
              EUSCI_B0->CTLW0 |= EUSCI_B_CTLW0_SWRST;
              EUSCI_B0->BRW =100;
              EUSCI_B0->CTLW0 &=~ EUSCI_B_CTLW0_SWRST;
              reg[0]=ReadSci(SCI_MODE);
              reg[1]=ReadSci(SCI_STATUS);
              reg[2]=ReadSci(SCI_BASS);
              reg[3]=ReadSci(SCI_CLOCKF);
              reg[4]=ReadSci(SCI_DECODE_TIME);
              reg[5]=ReadSci(SCI_AUDATA);
              reg[6]=ReadSci(SCI_WRAM);
              reg[7]=ReadSci(SCI_WRAMADDR);
              reg[8]=ReadSci(SCI_HDAT0);
              reg[9]=ReadSci(SCI_HDAT1);
              reg[10]=ReadSci(SCI_AIADDR);
              reg[11]=ReadSci(SCI_VOL);
              reg[12]=ReadSci(SCI_AICTRL0);
              reg[13]=ReadSci(SCI_AICTRL1);
              reg[14]=ReadSci(SCI_AICTRL2);
              reg[15]=ReadSci(SCI_AICTRL3);

              fres = f_mount(&FatFs, "", 1);

   while(1){
	   if(data_received_flag == 1){
		   received_option = decode();
		   if(received_option == 0xF807){ //option A is chosen
			   //play a song
			   lcdclear();
			   lcdputstr("Playing music...", 0x00);
			   music_mode = 1;
			   playmusic(0);
		   }
		   else if(received_option == 0x7887){ //option B is chosen
			   //read a text file
			   lcdclear();
			   lcdputstr("Reading text file...",0x00);
			   //Delayms();
			   read_mode = 1;
			   lcdreadtextfile(1);
		   }
		   else if(received_option == 0x58A7){ //option C is chosen
			   //exit to menu
			   read_mode = 0;
			   music_mode = 0;
			   is_paused = 0;
			   lcdmenu();
			   lcdpopulatefiles(1);
		   }
		   else if(received_option == 0xA05F){
			   //scroll up
			   if(music_mode == 1){
				   //increase the volume
				   P3->OUT &= ~ BIT2;
				   current_volume = current_volume - VOL_DIFF;
				   getvolumerange();
				   lcdputstr(vol_level,0x1A);
				   WriteSci(SCI_VOL, current_volume);
				   P3->OUT |=  BIT2;
			   }
		   }
		   else if(received_option == 0x00FF){
			   //scroll down
			   if(read_mode == 1){
				   lcdscrolldown();
			   }
			   else if(music_mode == 1){
				   //decrease the volume
				   P3->OUT &= ~ BIT2;
				   current_volume = current_volume + VOL_DIFF;
				   getvolumerange();
				   lcdputstr(vol_level,0x1A);
				   WriteSci(SCI_VOL, current_volume);
				   P3->OUT |=  BIT2;
			   }
		   }
		   else if(received_option == 0x807F){
			   //play next
			  if(music_mode == 1){
				   lcdputstr("Playing next song...", 0x00);
				   			   music_mode = 1;
				   			   if(current_music_option >=3){
				   				   current_music_option = 0;
				   			   }
				   			   else{
				   				   current_music_option++;
				   			   }
				   			   playmusic(current_music_option);

			   }
		   }//end of else if
		   else if(received_option == 0x20DF){
			   if(is_paused) is_paused = 0;
			   else is_paused = 1;
		   }
	   }
   }//end of while
    return 0;
}

//PORT 5 interrupt handler service routine
void PORT5_IRQHandler(void){
	 P5->IE &= ~BIT7; //Disable interrupts
		    P5->IFG &= ~ BIT7; //Disable interrupt flag
		    NVIC->ICER[1] = 1 << ((PORT5_IRQn) & 31);
		    TIMER_A0->CTL |= TIMER_A_CTL_MC_0; //Halting the timer
		    TIMER_A0->CTL = TIMER_A_CTL_CLR; //making timer register 0
		    TIMER_A0->CTL = TIMER_A_CTL_TASSEL_2 |  TIMER_A_CTL_ID_2;             //Divide 12MHz by 4 // SMCLK , 12 MHz
		    TIMER_A0->CTL |= TIMER_A_CTL_MC_2; //Starts timer in Continuous mode (count till FFFFh)
		    while(!(P5->IN & BIT7)){
		        if(TA0R > 30000){
		            break;
		        }
		    }
		    k=TA0R;
		    if( k<=31500 && k>= 15000){
		        TIMER_A0->CTL = TIMER_A_CTL_MC_0; //Halting the timer
		        TIMER_A0->CTL = TIMER_A_CTL_CLR; //making timer register 0
		        TIMER_A0->CTL = TIMER_A_CTL_TASSEL_2 | TIMER_A_CTL_ID_2;             //Divide 12MHz by 4 // SMCLK , 12 MHz
		        TIMER_A0->CTL |= TIMER_A_CTL_MC_2; //Starts timer in Continuous mode (count till FFFFh)
		        while((P5->IN & BIT7));         // WAIT FOR START CONDITION TO END
		        k=TA0R;
		        if(k > 9000 && k<16000){
		            for(i=0; i<32; i++){
		            while(!(P5->IN & BIT7)); // WAIT FOR BIT TO START
		            TIMER_A0->CTL = TIMER_A_CTL_MC_0; //Halting the timer
		            TIMER_A0->CTL = TIMER_A_CTL_CLR; //making timer register 0
		            TIMER_A0->CTL = TIMER_A_CTL_TASSEL_2 |  TIMER_A_CTL_ID_2;             //Divide 12MHz by 4// SMCLK , 12 MHz
		            TIMER_A0->CTL |= TIMER_A_CTL_MC_2; //Starts timer in Continuous mode (count till FFFFh)
		            while((P5->IN & BIT7)); // WAIT FOR BIT TO END
		            array[i] = TIMER_A0->R;
		            }
		            data_received_flag = 1;
		        }
		    }
		    P5->IE |= BIT7; //Disable interrupts
		    P5->IFG &= ~ BIT7; //Disable interrupt flag
		    NVIC->ISER[1] = 1 << ((PORT5_IRQn) & 31);
}




