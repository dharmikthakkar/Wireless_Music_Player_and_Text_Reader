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

//Reset P6.0
//DREQ P6.5
//CS P3.2
//BSYNC P6.4


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


          /* initialize systick timer */

          /* Configure SysTick for a 100Hz interrupt.  The FatFs driver wants a 10 ms
           * tick.
           */
          SysTick_setPeriod(3000000 / 100);
          SysTick_enableModule();
          SysTick_enableInterrupt();


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

          //enable interrupts
         // UCB2IE |= UCTXIE;
          //UCB2IE |= UCRXIE;
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






          if(error_flag != -1){
    //power_on();
              fres = f_mount(&FatFs, "", 1);
              fres = f_open(&File[0], my_path, FA_READ);
              fres = f_read(&File[0], Buff, 10, &s1);
              fres = f_close(&File[0]);
             // WriteSci(SCI_VOL, 0x0101);
              WriteSci(SCI_DECODE_TIME, 0);         // Reset DECODE_TIME
              EUSCI_B0->IFG |= EUSCI_B_IFG_TXIFG;
              fres = f_open(&File[1], "bird.wav", FA_READ);
              cnt = 0;
              P6->OUT |= BIT4;
              while(1){
                  fres = f_read(&File[1], Buff2,  128, &s2);
                  cnt = cnt + s2;
                  if(fres || s2 == 0) break;
                  P3->OUT |= BIT2; //Activate CS - make it low
                  //CS->KEY = CS_KEY_VAL;
                  //CS->CTL1 &= 0x8FFFFFFF;
                  //CS->CTL1 |= (CS_CTL1_SELS_3 | CS_CTL1_DIVS_1);
                  //EUSCI_B0->CTLW0 |= EUSCI_B_CTLW0_SWRST;
                  //EUSCI_B0->BRW =1;
                  //EUSCI_B0->CTLW0 &=~ EUSCI_B_CTLW0_SWRST;
                  //CS->KEY = 0;

                  for(i=0; i<4; i++){
                      result_codec = WriteSdi(Buff2+(i*32), min(32, s2));
//                      reg[8]=ReadSci(SCI_HDAT0);
//                      reg[9]=ReadSci(SCI_HDAT1);

                  }
                  while(!(EUSCI_B0->IFG & EUSCI_B_IFG_TXIFG)){};

                  P3->OUT &=~ BIT2; //Activate CS - make it low
                  reg[8]=ReadSci(SCI_HDAT0);
                  reg[9]=ReadSci(SCI_HDAT1);

                  //CS->KEY = CS_KEY_VAL;
                  //CS->CTL1 &= 0x8FFFFFFF;
                  //CS->CTL1 |= (CS_CTL1_SELS_3 | CS_CTL1_DIVS_1);
                  //EUSCI_B2->CTLW0 |= EUSCI_B_CTLW0_SWRST;
                  //EUSCI_B2->BRW =9;
                  //EUSCI_B2->CTLW0 &=~ EUSCI_B_CTLW0_SWRST;
                  //CS->KEY = 0;


              }

              fres = f_close(&File[1]);
            //  P6->OUT |= BIT4;
              WriteSci(SCI_MODE, SM_CANCEL);
          }
   while(1);
    return 0;
}



