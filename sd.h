/*All the functions in this are a result of our own development effort
 * Authors: Lakhan Shiva Kamiredddy, Dharmik Thakkar
 * */
#include "diskio.h"
void delay_ms(int delay_t);
void transmit(uint16_t txData);
uint16_t receive_response();
uint16_t receive_ff();
int receive_multiple_bytes(int len);
void changeCRCcheck();
void readCID();
void readSector(uint32_t address);
void writeSector(uint32_t address, uint8_t data);
static void power_on (void);
static int select (void);
static void power_off (void);
DSTATUS disk_initialize (
	BYTE drv		/* Physical drive number (0) */
);
DSTATUS disk_status (
	BYTE drv		/* Physical drive number (0) */
);
