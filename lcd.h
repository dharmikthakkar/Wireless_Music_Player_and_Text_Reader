static void delay_us(int delay);
void Delayms(int delay);
char readlcddata(void);
void lcdBusyWait();
void putcommand(unsigned char x);
void commandWrite(unsigned char cmd);
void putdata(unsigned char x);
void dataWrite(unsigned char Data);
unsigned char dataRead(void);
void lcdputch(char ch);
void lcdgotoaddr(unsigned char addr);
void lcdputstr(char *str);
void lcdinit();
void lcdmenu();
void lcdpopulatefiles(unsigned int option);
