/*
 * Copyright (C) 2017, Lakhan Kamireddy, Dharmik Thakkar
 */

#include "msp.h"

/*FAT File system includes*/
#include "diskio.h"
#include "ff.h"
//#include "fatfs/ffconf.h"
#include "integer.h"

FATFS FatFs;                /* File system object for each logical drive */
FIL File[2];                /* File objects */
FRESULT fres;
BYTE Buff[64] __attribute__ ((aligned (4))) ;    /* Working buffer */
UINT s1;

#define num_text 3
#define num_mp3 3
unsigned char * text[num_text] = {"test1", "story1", "text3"};
unsigned char * mp3[num_mp3] = {"mp3_1", "mp3_2", "mp3_3"};

const TCHAR *file_path;
unsigned int cnt = 0;
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

/*Generates a delay*/
void Delayms(int delay){
    int i;
    for(i=0;i<delay; i++){
    }
}


/* generates approx 1 ms */
void delay_ms(int delay_t){
    int i=0, j=0;
    for(i=0; i<delay_t; i++){
        for(j=0; j<121; j++);
    }
}


/*Reads LCD code*/
unsigned char readlcddata(void){
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
unsigned char cmd;
P5->OUT &= ~BIT0; //RS = 0 //command
P5->OUT |=  BIT1; //RW = 1 //read
cmd = readlcddata();
while((cmd & 0x80) == 0x80){ //checking if BF is set
    cmd = readlcddata();
    P5->OUT &= ~BIT5; //Enable = 0
	};
}



/* checks for lcd busy flag */
void check_busy_flag(){
    volatile unsigned char temp;
    P5->OUT &= ~BIT0; //RS = 0 //command
    P5->OUT |=  BIT1; //RW = 1 //read
    do{
    temp = readlcddata();
    temp = temp & 0x80;
    P5->OUT &= ~BIT5; //Enable = 0
    }while(temp == 0x80);
    P5->OUT &=~  BIT1; //RW = 1 //read
}


/*
    Name: putcommand()
    Description: writes command to LCD
    Input: unsigned char
    Return value: void
*/
void putcommand(unsigned char x){
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
	P5->OUT &= ~BIT0; //RS = 0 //command
	P5->OUT &= ~BIT1; //RW = 0 //write
    //write command to lcd
    putcommand(cmd);
}

/*
    Name: putdata()
    Description: puts data to LCD
    Input: unsigned char
    Return value: void
*/
void putdata(unsigned char x){
	//Using port 4 to put the word to LCD
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
void lcdputch(unsigned char ch){
    lcdBusyWait();
    P5->OUT |= BIT0; //RS = 1 //data
    P5->OUT &= ~BIT1; //RW = 0 //write
    delay_ms(2);
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


/* goto CGRAM address */
void lcdgotocgramaddr(unsigned char addr){
    addr = (addr<<3) | 0x40;
   // check_busy_flag();
    delay_ms(20);
    P4->DIR = BIT0 | BIT1 | BIT2 | BIT3 | BIT4 | BIT5 | BIT6 | BIT7;
    P4->OUT = addr;
    P5->OUT &= ~BIT0; //RS = 0
    P5->OUT &= ~BIT1; //RW = 0
    P5->OUT |= BIT5; //Enable = 1
    P5->OUT &= ~BIT5; //Enable = 0
    Delayms(20);
    Delayms(200);



}



/* Put custom character on LCD */
void lcdputchcustom(unsigned char cc){
    delay_ms(20);
    P4->DIR = BIT0 | BIT1 | BIT2 | BIT3 | BIT4 | BIT5 | BIT6 | BIT7;
    P4->OUT = cc;
    P5->OUT |= BIT0; //RS = 0
    P5->OUT &= ~BIT1; //RW = 0
    delay_ms(2);
    P5->OUT |= BIT5; //Enable = 1
        Delayms(20);
        P5->OUT &= ~BIT5; //Enable = 0
    Delayms(200);
}


/*
    Name: lcdputstr()
    Description: Clears the LCD display and prints on the screen
    Input: char *str
    Return Value: void
*/
void lcdputstr(char *str, unsigned char addr) {
    unsigned int i = 0;
    int lineNumber = 1;
    unsigned char c;
    lcdBusyWait();
    P5->OUT |= BIT0; //RS = 1 //data
    P5->OUT &= ~BIT1; //RW = 0 //write
    c = *str;
    if(addr == 0){
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
    else{
    	lcdgotoaddr(addr);
    	while(c){
    	        if(i % 16 == 0){
    	            if(i!=0)
    	                lineNumber++;
    	            if(i % 64 == 0){
    	                lineNumber = 1;
    	            }
    	            if(lineNumber == 1){
    	            }
    	            else if(lineNumber == 2){
    	            }
    	            else if(lineNumber == 3){
    	            }
    	            else if(lineNumber == 4){
    	                lineNumber = 1;
    	            }
    	        }
    	        i++;
    	        lcdputch(c);
    	        c=*(str+i);
    	    }
    }
}

/*
    Name: lcdclear()
    Description: Clears the LCD display
    Input: void
    Return Value: void
*/
void lcdclear(){
    lcdBusyWait();
    //lcd clear command
    commandWrite(0x01);
}

/*
    Name: lcdmenu()
    Description: Displays menu for our application
    Input: void
    Return Value: void
*/
void lcdmenu(){
	//Puts the lcd menu which displays A. Music B. Text files C. Exit
	lcdclear();
	lcdgotoaddr(0x00);
	lcdputstr("A.Music", 0x00);
	lcdgotoaddr(0x40);
	lcdputstr("B.Text", 0x40);
	lcdgotoaddr(0x10);
	lcdputstr("C.Exit", 0x10);
}


/*
    Name: lcdscrolldown()
    Description: Scrolls down
    Input: void
    Return Value: void
*/
void lcdscrolldown(void){
		 fres = f_read(&File[0], Buff,  60, &s1);
	 	 cnt = cnt + s1;
	 	 if(fres || s1 == 0) return;
	 	 	 lcdclear();
	 		 lcdputstr(Buff, 0x00); //Displays the file on the LCD
}

/*
    Name: lcdreadtextfile()
    Description: Displays the text file on the lcd
    Input: option for text file
    Return Value: int
*/
int lcdreadtextfile(unsigned int option){
	if(option >= num_text){
		return -1; //There are only num_text number of files
	}
	if(option == 0 ){
		file_path = "test1.txt";
	}
	else if(option == 1){
		file_path = "story1.txt";
	}
	 fres = f_mount(&FatFs, "", 1);
	 fres = f_open(&File[0], file_path, FA_READ);
	 fres = f_read(&File[0], Buff,  60, &s1);
	 lcdclear();
	 lcdputstr(Buff, 0x00); //Displays the file on the LCD
	 return 0;
}


/*
    Name: lcdpopulatefiles()
    Description: Populates file names on LCD
    Input: option
    Return Value: void
*/
void lcdpopulatefiles(unsigned int option){
	unsigned char addr1 =  0x08;
	unsigned char addr2 =  0x48;
	unsigned char addr3 =  0x18;
	unsigned int i=0;
	//populate the LCD filenames on the LCD
	if(option == 1){
		//if the option is 1, then populate music files on the LCD
		lcdgotoaddr(addr1);
		lcdputstr(text[i],addr1);
		i++;
		lcdgotoaddr(addr2);
		lcdputstr(text[i],addr2);
		i++;
		lcdgotoaddr(addr3);
		lcdputstr(text[i],addr3);
	}
	else if(option == 2){
		//if the option is 2, then populate music files on the LCD
		lcdgotoaddr(addr1);
		lcdputstr(mp3[i],addr1);
		i++;
		lcdgotoaddr(addr2);
		lcdputstr(mp3[i],addr2);
		i++;
		lcdgotoaddr(addr3);
		lcdputstr(mp3[i],addr3);
	}
}


/*
    Name: lcdinit()
    Description: initializes the LCD
    Input: void
    Return Value: void
*/
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




/* Create custom character */

void lcdcreatechar(unsigned char ccode, unsigned char row_vals[], unsigned char lcd_addr){
    unsigned char i;
    lcdgotocgramaddr(ccode);
    for(i=0; i<8; i++){
        lcdputchcustom(row_vals[i]);
        delay_ms(2);
    }

    lcdgotoaddr(lcd_addr);
    delay_ms(20);
    lcdputchcustom(ccode);
    lcdgotoaddr(0x00);
}


/* Create custom character for play*/
void custom_char_play(unsigned char lcd_a){
    unsigned char char_code;
    unsigned char cgram_data[8];
    char_code = 0;
    cgram_data[0] = 0b11011;
    cgram_data[1] = 0b11011;
    cgram_data[2] = 0b11011;
    cgram_data[3] = 0b11011;
    cgram_data[4] = 0b11011;
    cgram_data[5] = 0b11011;
    cgram_data[6] = 0b11011;
    cgram_data[7] = 0b11011;
    lcdgotoaddr(lcd_a);
    lcdcreatechar(char_code, cgram_data, lcd_a);
}


/* Create custom character for pause*/
void custom_char_pause(unsigned char lcd_a){
    unsigned char char_code;
    unsigned char cgram_data[8];
    char_code = 1;
    cgram_data[0] = 0b10000;
    cgram_data[1] = 0b11100;
    cgram_data[2] = 0b11110;
    cgram_data[3] = 0b11111;
    cgram_data[4] = 0b11110;
    cgram_data[5] = 0b11100;
    cgram_data[6] = 0b11000;
    cgram_data[7] = 0b10000;
    lcdgotoaddr(lcd_a);
    lcdcreatechar(char_code, cgram_data, lcd_a);
}

/* Create custom character for next*/
void custom_char_next(unsigned char lcd_a){
    unsigned char char_code;
    unsigned char cgram_data[8];
    char_code = 2;
    cgram_data[0] = 0b10001;
    cgram_data[1] = 0b11101;
    cgram_data[2] = 0b11101;
    cgram_data[3] = 0b11111;
    cgram_data[4] = 0b11111;
    cgram_data[5] = 0b11101;
    cgram_data[6] = 0b11001;
    cgram_data[7] = 0b10001;
    lcdgotoaddr(lcd_a);
    lcdcreatechar(char_code, cgram_data, lcd_a);
}

/* Create custom character for previous*/
void custom_char_prev(unsigned char lcd_a){
    unsigned char char_code;
    unsigned char cgram_data[8];
    char_code = 3;
    cgram_data[0] = 0b10001;
    cgram_data[1] = 0b10111;
    cgram_data[2] = 0b10111;
    cgram_data[3] = 0b11111;
    cgram_data[4] = 0b11111;
    cgram_data[5] = 0b10111;
    cgram_data[6] = 0b10011;
    cgram_data[7] = 0b10001;
    lcdgotoaddr(lcd_a);
    lcdcreatechar(char_code, cgram_data, lcd_a);
}

/* Create custom character for shuffle ON*/
void custom_char_shuffle_on(unsigned char lcd_a){
    unsigned char char_code;
    unsigned char cgram_data[8];
    char_code = 4;
    cgram_data[0] = 0b00000;
    cgram_data[1] = 0b11011;
    cgram_data[2] = 0b01010;
    cgram_data[3] = 0b00100;
    cgram_data[4] = 0b01010;
    cgram_data[5] = 0b11011;
    cgram_data[6] = 0b00000;
    cgram_data[7] = 0b11111;
    lcdgotoaddr(lcd_a);
    lcdcreatechar(char_code, cgram_data, lcd_a);
}

/* Create custom character for OFF*/
void custom_char_shuffle_off(unsigned char lcd_a){
    unsigned char char_code;
    unsigned char cgram_data[8];
    char_code = 5;
    cgram_data[0] = 0b11111;
    cgram_data[1] = 0b00000;
    cgram_data[2] = 0b00000;
    cgram_data[3] = 0b00000;
    cgram_data[4] = 0b00000;
    cgram_data[5] = 0b00000;
    cgram_data[6] = 0b00000;
    cgram_data[7] = 0b11111;
    lcdgotoaddr(lcd_a);
    lcdcreatechar(char_code, cgram_data, lcd_a);
}

/* Create custom character for loop*/
void custom_char_loop(unsigned char lcd_a){
    unsigned char char_code;
    unsigned char cgram_data[8];
    char_code = 6;
    cgram_data[0] = 0b00010;
    cgram_data[1] = 0b11111;
    cgram_data[2] = 0b10010;
    cgram_data[3] = 0b10001;
    cgram_data[4] = 0b10001;
    cgram_data[5] = 0b01001;
    cgram_data[6] = 0b11111;
    cgram_data[7] = 0b01000;
    lcdgotoaddr(lcd_a);
    lcdcreatechar(char_code, cgram_data, lcd_a);
}

/* Create custom character for loop OFF*/
void custom_char_loop_off(unsigned char lcd_a){
    unsigned char char_code;
    unsigned char cgram_data[8];
    char_code = 7;
    cgram_data[0] = 0b00010;
    cgram_data[1] = 0b11111;
    cgram_data[2] = 0b00010;
    cgram_data[3] = 0b00000;
    cgram_data[4] = 0b00000;
    cgram_data[5] = 0b00010;
    cgram_data[6] = 0b11111;
    cgram_data[7] = 0b00010;
    lcdgotoaddr(lcd_a);
    lcdcreatechar(char_code, cgram_data, lcd_a);
}
