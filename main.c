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

/* Standard Includes */
#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>

/*FAT File system includes*/
#include "fatfs/diskio.h"
#include "fatfs/ff.h"
#include "fatfs/ffconf.h"
#include "fatfs/integer.h"
#include "fatfs/option/syscall.c"
#include "fatfs/option/unicode.c"
#include "fatfs/option/syscall.c"

/*Header file*/
#include "spi.h"

/*-----------------------------------------------------------------------*/
/* Low level disk I/O module skeleton for FatFs     (C)ChaN, 2016        */
/*-----------------------------------------------------------------------*/
/* If a working storage control module is available, it should be        */
/* attached to the FatFs via a glue function rather than modifying it.   */
/* This is an example of glue functions to attach various exsisting      */
/* storage control modules to the FatFs module with a defined API.       */
/*-----------------------------------------------------------------------*/


/* Definitions of physical drive number for each drive */
#define DEV_MMC		0	/* Example: Map MMC/SD card to physical drive 0 */

/*MACROS*/
#define SD_CARD_FILE_ADDRESS 32832

/*Global variable declaration*/
uint8_t receiveBuffer[514];
FIL *fp;
FATFS *fs;

void delay(int delay_s){
	int i;
	for(i=0; i<50*delay_s; i++){

	}
}

/*
    Name: delay_us()
    Description: Delays in micro seconds
    Input: int delay
    Return value: void
*/
static void delay_us(int delay){
    int i;
    for(i=0;i<delay/1000; i++){
    }
}

void Delayms(int delay){
    int i;
    for(i=0;i<delay; i++){
    }
}

char readlcddata(void){
    char c;
    P5->OUT |= BIT5; //Enable = 1
    Delayms(20);
    P4->DIR = 0x00; //Making input
    c = P4->IN; //reading input
    Delayms(200);
    P5->OUT &= ~BIT5; //Enable = 0
    return c;
}

/*
    Name: lcdBusyWait()
    Description: Polls the LCD busy flag. Function does not return until LCD
                 controller is ready to accept another command
    Input: unsigned char
    Return value: void
*/
void lcdBusyWait(){
char cmd;
P5->OUT &= ~BIT0; //RS = 0 //command
P5->OUT |=  BIT1; //RW = 1 //read
cmd = readlcddata();
while((cmd & 0x80) == 0x80){ //checking if BF is set
    cmd = readlcddata();
    P5->OUT &= ~BIT5; //Enable = 0
    /*P5->OUT |= BIT5; //Enable = 1
    P5->OUT &= ~BIT5; //Enable = 0*/
};
 /*unsigned int i;
 for(i=0; i<5000; i++){

 }*/
}

/*
    Name: putcommand()
    Description: puts command to LCD address location 0x8000
    Input: unsigned char
    Return value: void
*/
void putcommand(unsigned char x){
	//Using port 4 to put the word to LCD
	//unsigned int i =0;
	P5->OUT |= BIT5; //Enable = 1
	Delayms(20);
	P4->DIR = BIT0 | BIT1 | BIT2 | BIT3 | BIT4 | BIT5 | BIT6 | BIT7;
	P4->OUT = x;
	Delayms(200);
	P5->OUT &= ~BIT5; //Enable = 0
}

/*
    Name: commandWrite()
    Description: Writes command to the LCD
    Input: unsigned char cmd
    Return Value: void
*/
void commandWrite(unsigned char cmd){
	P5->OUT &= ~BIT0; //RS = 0
	P5->OUT &= ~BIT1; //RW = 0
	/*P5->OUT |= BIT5; //Enable = 1
	Delayms(200);
	P5->OUT &= ~BIT5; //Enable = 0
	delay_us(2000);
	P5->OUT |= BIT5; //Enable = 1
	Delayms(200);*/
	//P5->OUT &= ~BIT2; //Enable = 0

	P5->OUT &= ~BIT0; //RS = 0 //command
	P5->OUT &= ~BIT1; //RW = 0 //write
    //write command to lcd
    putcommand(cmd);
}

/*
    Name: putdata()
    Description: puts data to LCD address location 0xFF00
    Input: unsigned char
    Return value: void
*/
void putdata(unsigned char x){
	//Using port 4 to put the word to LCD
	//unsigned int i =0;
	P5->OUT |= BIT5; //Enable = 1
	Delayms(20);
	P4->DIR = BIT0 | BIT1 | BIT2 | BIT3 | BIT4 | BIT5 | BIT6 | BIT7;
	P4->OUT = x;
	Delayms(200);
	P5->OUT &= ~BIT5; //Enable = 0
}


/*
    Name: datawrite()
    Description: Writes data to LCD
    Input: unsigned char Data
    Return Value: void
*/
void dataWrite(unsigned char Data){
	P5->OUT &= ~BIT0; //RS = 0 //data
	P5->OUT |= BIT1; //RW = 1 //write
    putdata(Data);
}


/*
    Name: dataRead()
    Description: Writes data to LCD
    Input: unsigned char Data
    Return Value: void
*/
unsigned char dataRead(void){
    lcdBusyWait();
    P5->OUT |= BIT0; //RS = 1 //data
    P5->OUT |= BIT1; //RW = 1 //read
    return readlcddata();
}

/*
    Name: lcdputch()
    Description: Writes the specific character to the current
                 LCD cursor position. Should call lcdbusywait()
    Input: void
    Return Value: void
*/
void lcdputch(char ch){
    lcdBusyWait();
    P5->OUT |= BIT0; //RS = 1 //data
    P5->OUT &= ~BIT1; //RW = 0 //write
    putdata(ch);
}

/*
    Name: lcdgotoaddr()
    Description: Sets the cursor to the LCD DDRAM address corresponding
                 to the LCD screen. Should call lcdbusywait()
    Input: unsigned char addr
    Return Value: void
*/
void lcdgotoaddr(unsigned char addr){
    //calls set DD RAM address command
    //command is 1 DDRAM address or 1 A A A A A A A
    lcdBusyWait();
    addr |= 0x80;
    commandWrite(addr);
}

/*
    Name: lcdputstr()
    Description: Clears the LCD display and prints on the screen
    Input: char *str
    Return Value: void
*/
void lcdputstr(char *str) {
    unsigned int i = 0;
    int lineNumber = 1;
    char c;
    lcdBusyWait();
    P5->OUT |= BIT0; //RS = 1 //data
    P5->OUT &= ~BIT1; //RW = 0 //write
    c = *str;
    while(c){
        if(i % 16 == 0){
            if(i!=0)
                lineNumber++;
            if(i % 64 == 0){
                lcdgotoaddr(0x00);
                lineNumber = 1;
            }
            if(lineNumber == 1){
                lcdgotoaddr(0x00);
            }
            else if(lineNumber == 2){
                lcdgotoaddr(0x40);
            }
            else if(lineNumber == 3){
                lcdgotoaddr(0x10);
            }
            else if(lineNumber == 4){
                lcdgotoaddr(0x50);
                lineNumber = 1;
            }
        }
        i++;
        lcdputch(c);
        c=*(str+i);
    }
}


void lcdinit(){
	P5->DIR = BIT0 | BIT1 | BIT5;   //Sets P5.0,5.1, 5.5 as output pins
	P4->DIR = 0xFF;
	P4->OUT = 0x00;
	P5->OUT &= ~BIT5; //Enable = 0
	P5->OUT &= ~BIT0; //RS = 0
	P5->OUT &= ~BIT1; //RW = 0
	//1. Power on
	    //2. Wait for more than 15 ms after Vcc rises to 4.5 V
	    Delayms(200);
	    //3. RS RW DB7 DB6 DB5 DB4 DB3 DB2 DB1 DB0 - 000011****
	    commandWrite(0x30);
	    //4. Wait for more than 4.1 ms
	    Delayms(80);
	    //5. RS RW DB7 DB6 DB5 DB4 DB3 DB2 DB1 DB0 - 000011****
	    commandWrite(0x30);
	    //6. Wait for more than 100 micro s
	    delay_us(2000);
	    //7. RS RW DB7 DB6 DB5 DB4 DB3 DB2 DB1 DB0 - 000011****
	    commandWrite(0x30);
	    lcdBusyWait();
	    //8. Function set command
	    //Function set is 0 0 1 DL N F * *
	    //DL - sets interface data length, DL = 0 means 4-bit interface, DL = 1 means 8-bit interface
	    //N - sets number of display line, N = 0 is 1/8 duty means 1 line, N = 1 is 1/16 duty means 2 lines
	    //F - sets the character font, F = 0 means 5x7 dots, F = 1 means 5x10 dots
	    //0011100 is 0x38 (DL = 1, N = 1, F = 0)
	    commandWrite(0x38);
	    lcdBusyWait();
	    //9. Display turn OFF
	    //Display On Off control is 0 0 0 0 1 D C B
	    //D - sets on or off the display, D = 0 means display OFF, D = 1 means display ON
	    //C - cursor on or off, C = 0 means cursor OFF, C = 1 means cursor ON
	    //B - blink of cursor position character, B = 0 means cursor blink OFF, B = 1 means cursor blink ON
	    //00001000 (D=0, C=0, B=0) turns the display off is 0x08 in hex
	    commandWrite(0x08);
	    lcdBusyWait();
	    //10. Display turn ON
	    //Display On Off control is 0 0 0 0 1 D C B
	    //D - sets on or off the display, D = 0 means display OFF, D = 1 means display ON
	    //C - cursor on or off, C = 0 means cursor OFF, C = 1 means cursor ON
	    //B - blink of cursor position character, B = 0 means cursor blink OFF, B = 1 means cursor blink ON
	    //00001111 (D=1, C=1, B=1) turns the display on, cursor on and blinks the cursor is 0x0F in hex
	    commandWrite(0x0F);
	    lcdBusyWait();
	    //11. Entry mode set command
	    //0	0 0	0 0	0 0	1 I/D S
	    //I/D sets the cursor move direction, I/D = 1 increments cursor position, I/D = 0 decrements cursor position
	    //S is for shifting display, S=0 means no display shift, S=1 means display shifts
	    //0000000110 is entry mode set, with cursor moving to right. It is 0x06 in hex
	    commandWrite(0x06);
	    lcdBusyWait();
	    //12. Clear screen
	    commandWrite(0x01);
	    lcdBusyWait();
	    //13. Cursor home
	    commandWrite(0x02);

}

/*-----------------------------------------------------------------------*/
/* Get Drive Status                                                      */
/*-----------------------------------------------------------------------*/

DSTATUS disk_status (
	BYTE pdrv		/* Physical drive nmuber to identify the drive */
)
{
	DSTATUS stat = STA_NOINIT;
	if (pdrv != 0) return STA_NOINIT;	/* Supports only single drive */
	return stat;
}



/*-----------------------------------------------------------------------*/
/* Inidialize a Drive                                                    */
/*-----------------------------------------------------------------------*/

DSTATUS disk_initialize (
	BYTE pdrv				/* Physical drive nmuber to identify the drive */
)
{
	DSTATUS stat = STA_NOINIT;
	if (pdrv != 0) return STA_NOINIT;	/* Supports only single drive */
	if (stat & STA_NODISK) return stat;	/* No card in the socket */
	return 0;
}



/*-----------------------------------------------------------------------*/
/* Read Sector(s)                                                        */
/*-----------------------------------------------------------------------*/

DRESULT disk_read (
	BYTE pdrv,		/* Physical drive nmuber to identify the drive */
	BYTE *buff,		/* Data buffer to store read data */
	DWORD sector,	/* Start sector in LBA */
	UINT count		/* Number of sectors to read */
)
{
	uint32_t addr;
	DSTATUS stat = STA_NOINIT;
	if (pdrv || !count) return RES_PARERR;
	if (stat & STA_NOINIT) return RES_NOTRDY;
	DRESULT res;
	if (count == 1) {		/* Single block read */
			 /* READ_SINGLE_BLOCK */
		readSector(addr);
				count = 1;
	}
	else {				/* Multiple block read */
			/*if (send_cmd(CMD18, sector) == 0) {	/* READ_MULTIPLE_BLOCK
				do {
					if (!rcvr_datablock(buff, 512)) break;
					buff += 512;
				} while (--count);
				send_cmd(CMD12, 0);				/* STOP_TRANSMISSION
			}*/
		   readSector(addr);
		   count = 1;
	}
	return RES_OK;
}



/*-----------------------------------------------------------------------*/
/* Write Sector(s)                                                       */
/*-----------------------------------------------------------------------*/

DRESULT disk_write (
	BYTE pdrv,			/* Physical drive nmuber to identify the drive */
	const BYTE *buff,	/* Data to be written */
	DWORD sector,		/* Start sector in LBA */
	UINT count			/* Number of sectors to write */
)
{
	uint32_t address;
	uint8_t data;
	DSTATUS stat = STA_NOINIT;
	if (pdrv || !count) return RES_PARERR;
	if (stat & STA_NOINIT) return RES_NOTRDY;
	if (stat & STA_PROTECT) return RES_WRPRT;
	DRESULT res;
	int result;
	if (count == 1) {		/* Single block write */
			send_cmd24(address, data);
	}
		else {				/* Multiple block write */
			 send_cmd24(address, data);
	}
	return RES_OK;
}



/*-----------------------------------------------------------------------*/
/* Miscellaneous Functions                                               */
/*-----------------------------------------------------------------------*/

DRESULT disk_ioctl (
	BYTE pdrv,		/* Physical drive nmuber (0..) */
	BYTE cmd,		/* Control code */
	void *buff		/* Buffer to send/receive control data */
)
{
	DSTATUS stat = STA_NOINIT;
	DRESULT res;
	int result;
	if (pdrv) return RES_PARERR;
	if (stat & STA_NOINIT) return RES_NOTRDY;

	return RES_PARERR;
}

DWORD get_fattime(){
	return 0;
}



void transmit(uint16_t txData){
	              EUSCI_B2->IFG &=~ EUSCI_B_IFG_TXIFG;
				  EUSCI_B2->TXBUF = txData;
	              while(!(EUSCI_B2->IFG & EUSCI_B_IFG_TXIFG)){};
	              EUSCI_B2->IFG &=~ EUSCI_B_IFG_TXIFG;
}

int receive_multiple_bytes(int len){

  int i =0;
  EUSCI_B2->IFG |= EUSCI_B_IFG_TXIFG;
  for(i=0;i<len+2;i++){
	  while(!(EUSCI_B2->IFG & EUSCI_B_IFG_TXIFG)){};
	  EUSCI_B2->TXBUF = 0xFF;
	  while(!(EUSCI_B2->IFG & EUSCI_B_IFG_RXIFG)){};
	  receiveBuffer[i] = EUSCI_B2->RXBUF;
  }
  EUSCI_B2->IFG &=~ EUSCI_B_IFG_TXIFG;
  EUSCI_B2->IFG &=~ EUSCI_B_IFG_RXIFG;
  return len;

}

uint16_t receive(){
	uint16_t rxData;
	rxData = EUSCI_B2->RXBUF;
	while(!(EUSCI_B2->IFG & EUSCI_B_IFG_RXIFG) && (rxData==0xFF)){
		transmit(0xFF);
		rxData = EUSCI_B2->RXBUF;
	}
	EUSCI_B2->IFG &=~ EUSCI_B_IFG_RXIFG;
	return rxData;
}

uint16_t receive_ff(){
				uint16_t rxData;
				rxData = EUSCI_B2->RXBUF;
				while(!(EUSCI_B2->IFG & EUSCI_B_IFG_RXIFG) && (rxData!=0xFF)){
					transmit(0xFF);
					rxData = EUSCI_B2->RXBUF;
				}
		        EUSCI_B2->IFG &=~ EUSCI_B_IFG_RXIFG;
		        return rxData;
}

uint16_t receive_response(){
				  uint16_t rxData;
				  rxData = EUSCI_B2->RXBUF;
				  while(!(EUSCI_B2->IFG & EUSCI_B_IFG_RXIFG) && (rxData==0xFF)){
					  transmit(0xFF);
					  rxData = EUSCI_B2->RXBUF;
				  }
	              EUSCI_B2->IFG &=~ EUSCI_B_IFG_RXIFG;
	              return rxData;
}


void sd_init(){
	uint16_t data = 0xFF;
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
			for(i=0;i<200;i++){};
			P1->OUT &= ~BIT5;
			break;
		}
    }
	  EUSCI_B2->IFG &=~ EUSCI_B_IFG_RXIFG;

	  //CMD0
	                transmit(0x0040);
	                transmit(0x0000);
	                transmit(0x0000);
	                transmit(0x0000);
	                transmit(0x0000);
	                transmit(0x0095);
	                received_data = receive_response();//this should be 0x01 in hex, put the breakpoint after this line
	 //end of CMD0
	 //now the card is in idle state
	 send_cmd8();
}

void send_cmd8(){
	uint16_t received_data1;
	uint16_t received_data;
	transmit(0x0048);
	transmit(0x0000);
	transmit(0x0000);
	transmit(0x0001);
	transmit(0x00AA);
	transmit(0x0087);
	received_data1 = receive();//this should be 0x01, put the breakpoint after this line
	received_data = receive();
	received_data = receive();
	received_data = receive_ff();//then we send 0xFF till we get 0xFF before sending a new command
	send_cmd55();
}

void send_cmd55(){ //cmd55 is sent before sending any ACMD
	uint16_t received_data;
	transmit(0x0077);
	transmit(0x0000);
	transmit(0x0000);
	transmit(0x0000);
	transmit(0x0000);
	transmit(0x0065);
	received_data = receive();//this should be 0x01, put the breakpoint after this line
	received_data = receive_ff();//then we send 0xFF till we get 0xFF before sending a new command
	send_acmd41();
}

void send_acmd41(){//acmd41 is sent
		uint16_t received_data;
		transmit(0x69);
		transmit(0x40);
		transmit(0x00);
		transmit(0x00);
		transmit(0x00);
		transmit(0x77);
		received_data = receive();//this should be 0x00, put the breakpoint after this line
		if(received_data != 0x00){
			received_data = receive();//then we send 0xFF till we get 0xFF before sending a new command
			send_cmd55();
		}
		received_data = receive_ff();
		//delay(60);
		//if your code has reached this point, then it means the SD card is initialized successfully
}

void send_cmd13(){ //cmd13 is sent to ask the card to send its status register
	uint16_t received_data;
	transmit(0x4D);
	transmit(0x00);
	transmit(0x00);
	transmit(0x00);
	transmit(0x00);
	transmit(0x00);
	received_data = receive();//this should be , put the breakpoint after this line
	received_data = receive();
	received_data = receive();
	received_data = receive_ff();
}

void send_cmd59(){
	uint16_t received_data;
	transmit(0x007B);
	transmit(0x0000);
	transmit(0x0000);
	transmit(0x0000);
	transmit(0x0000);
	transmit(0x0000);
	received_data = receive(); //response R1
	received_data = receive_ff();
}


void send_cmd58(){
	uint16_t received_data;
	transmit(0x007A);
	transmit(0x0000);
	transmit(0x0000);
	transmit(0x0000);
	transmit(0x0000);
	transmit(0x00FF);
	received_data = receive(); //ocr register
	delay(10);
	received_data = receive(); //ocr register
	//first lower 16 bits are received. Then the upper 16 bits are received.
	//In upper 16 bits, bit 30 is CCS(Card Capacity Status) and bit 31 is Card Power up status bit.
	//If the CCS bit is set, that means it is an SDHC (High Capacity) or SDXC
	//If the Card Power up status bit is set, that means the initialization has run successfully
	//This gives 0xC0 currently
	received_data = receive_ff();
}

void send_cmd10(){//to read the card CID
	uint16_t received_data;
	uint16_t cid_buffer[8];
	int i = 7, j=0;
	transmit(0x4A);
	transmit(0x00);
	transmit(0x00);
	transmit(0x00);
	transmit(0x00);
	//transmit(0xFF);
	received_data = receive();//this should be 0x00, put breakpoint after this line
	while(received_data != 0xFE){ //read until 0xFE is received - this is read token
		received_data = receive();
	}
	//get 16 bytes from CID
	delay(60);
	j = 0;
	receive_multiple_bytes(8);
	for(i=7;i<=0;i--){
		cid_buffer[i]=receiveBuffer[j];
		j++;
	}
	received_data = receive_ff();
	delay(60);
}


void send_cmd24(uint32_t address, uint8_t data){//to write a single sector
	uint16_t received_data;
	int i =0;
	uint8_t low_byte_low_add = 0x000000FF & address;
	address >>= 8;
	uint8_t low_byte_high_add = 0x000000FF & address;
	address >>= 8;
	uint8_t high_byte_low_add = 0x000000FF & address;
	address >>= 8;
	uint8_t high_byte_high_add = 0x000000FF & address;
	transmit(0x58);
	transmit(high_byte_high_add);
	transmit(high_byte_low_add);
	transmit(low_byte_high_add);
	transmit(low_byte_low_add);
	transmit(0x00);
	delay(60);
	received_data = receive_ff();
	received_data = receive();//this should be 0x00, put breakpoint after this line
	transmit(0xFF); //one byte gap
	transmit(0xFF);
	transmit(0xFE); //send data token
	for(i=0;i<512;i++){
		transmit(data); //this is the data i am writing to this sector
		data += 1;
	}
	//send two byte CRC
	transmit(0x00); //send low CRC byte
	transmit(0x00); //send high CRC byte
	received_data = receive(); /*data response byte of the format xxx0sss1. Here xxx0 aren’t used,
	bit 4 is zero, bits 3:1 (sss) hold the status code.
	: 010 = Data accepted
    : 101 = Data rejected due to CRC error
    : 110 = Data rejected due to write error and bit 0 is a one.*/
	while(received_data == 0x00){ //read while busy, wait for non zero byte to be returned
		received_data = receive();
	}
	received_data = receive_ff();
}

void readSector(uint32_t address){
		int i = 0;
		int length;
		uint16_t received_data;
		uint8_t crc_low_byte;
		uint8_t crc_high_byte;
		uint8_t read_buffer[512];
		uint8_t low_byte_low_add = 0x000000FF & address;
		address >>= 8;
		uint8_t low_byte_high_add = 0x000000FF & address;
		address >>= 8;
		uint8_t high_byte_low_add = 0x000000FF & address;
		address >>= 8;
		uint8_t high_byte_high_add = 0x000000FF & address;
		transmit(0x51);
		transmit(high_byte_high_add);
		transmit(high_byte_low_add);
		transmit(low_byte_high_add);
		transmit(low_byte_low_add);
		transmit(0xFF);
		delay(5);
		received_data = receive();//this should be 0x00, put breakpoint after this line
		while(received_data != 0xFE){ //read until 0xFE is received - this is read token
				received_data = receive();
		}
		delay(60);
		length = 512;
		receive_multiple_bytes(512);
		for(i=0;i<512;i++){
				read_buffer[i] =receiveBuffer[i];
		}
		crc_low_byte = receiveBuffer[i++];
		crc_high_byte = receiveBuffer[i++];
		received_data = receive_ff();
}

void createFile(){
	FRESULT fr;
	char * file_name = "abc.txt";
	disk_initialize(0);
	fr = f_mount(fs,"0:/",0); // Mounts the drive with do not mount now option
	fr = f_open(fp,file_name, FA_CREATE_NEW);
	f_close(fp);
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
          UCB2IE |= UCTXIE;
          UCB2IE |= UCRXIE;
          //Configuring the CS GPIO pin
              P1->DIR = BIT5;   //Sets P1.5 as an output pin
              P1->OUT = BIT5;   //Sets P1.5
              /*Turn on LCD Module*/
              lcdinit();
              //lcdputstr("test data");
              sd_init(); //initialize SD card and put it into SPI mode
              send_cmd59(); ///turn CRC option OFF
              //now the card is initialized
              //send_cmd58(); //cmd58 to read the operating conditions register ocr
              //send_cmd13()
              send_cmd10(); //cmd10 to read the card identification register cid
              //readSector(address); //cmd17 performs sector read. currently getting only 0x00
              uint32_t addr = SD_CARD_FILE_ADDRESS;
              uint8_t data = 0x41;
              //send_cmd24(address, data); //cmd24 performs sector write.
              //readSector(address); //cmd17 performs sector read.
              //readSector(addr);

            /*Turn on LCD Module*/
            lcdinit();
            //lcdWrite(0x02);
          	int i;
          	int length;
          	uint16_t received_data;
          	uint8_t crc_low_byte;
          	uint8_t crc_high_byte;
          	uint8_t read_buffer[512];
          	uint8_t low_byte_low_add = 0x000000FF & addr;
          	addr >>= 8;
          	uint8_t low_byte_high_add = 0x000000FF & addr;
          	addr >>= 8;
          	uint8_t high_byte_low_add = 0x000000FF & addr;
          	addr >>= 8;
          	uint8_t high_byte_high_add = 0x000000FF & addr;
          	transmit(0x51);
          	transmit(high_byte_high_add);
          	transmit(high_byte_low_add);
          	transmit(low_byte_high_add);
          	transmit(low_byte_low_add);
          	transmit(0x00);
          	delay(5);
          	received_data = receive();//this should be 0x00, put breakpoint after this line
          	while(received_data != 0xFE){ //read until 0xFE is received - this is read token
          		received_data = receive();
          	}
          	delay(60);
          	length = 512;
          	receive_multiple_bytes(512);
          	//get 512 bytes from sector
          	for(i=0;i<512;i++){
          		read_buffer[i] =receiveBuffer[i];
          	}
          	//Note: Values are stored as decimal values in the buffer
          	crc_low_byte = receiveBuffer[i++];
          	crc_high_byte = receiveBuffer[i++];
          	received_data = receive_ff();
          	lcdputstr(read_buffer);
            send_cmd10();
            addr = 0x01;
            readSector(addr);
            createFile();
   while(1);
    return 0;
}


