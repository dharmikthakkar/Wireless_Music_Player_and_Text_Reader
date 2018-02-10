/*
 * lcd.c header file declarations
 * Authors: Lakhan Shiva Kamireddy, Dharmik Thakkar
 * All the functions in this header file are a result of our own development effort
 * */
static void delay_us(int delay);
void delay_ms(int delay_t);
void Delayms(int delay);
char readlcddata(void);
void lcdBusyWait();
void lcdclear();
void putcommand(unsigned char x);
void commandWrite(unsigned char cmd);
void putdata(unsigned char x);
void dataWrite(unsigned char Data);
unsigned char dataRead(void);
void lcdputch(char ch);
void lcdscrolldown(void);
void lcdgotoaddr(unsigned char addr);
void lcdputstr(char *str, unsigned char addr);
void lcdinit();
void lcdmenu();
void lcdpopulatefiles(unsigned int option);
int lcdreadtextfile(unsigned int option);
void custom_char_play(unsigned char lcd_a);
void custom_char_pause(unsigned char lcd_a);
void custom_char_prev(unsigned char lcd_a);
void custom_char_next(unsigned char lcd_a);
void custom_char_shuffle_on(unsigned char lcd_a);
void custom_char_shuffle_off(unsigned char lcd_a);
void custom_char_loop(unsigned char lcd_a);
void custom_char_loop_off(unsigned char lcd_a);
