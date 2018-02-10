#include <stdint.h>
#ifndef SPI_H
#define SPI_H
#endif

void sd_init();
void readSector(uint32_t addr);
void send_cmd24(uint32_t addr, uint8_t data);

