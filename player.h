/*
 * Authors: Lakhan Shiva Kamireddy, Dharmik Thakkar
 * */
/* All the functions in this are a result of our own development efforts,
 * We are not using some functions at all in our project
 * They are:
 * void SaveUIState(void)
 * void RestoreUIState(void)
 * int GetUICommand(void)
 * int VSTestHandleFile(const char *fileName, int record)
 * These function names are same as generic library function names
/*
*/
#ifndef PLAYER_RECORDER_H
#define PLAYER_RECORDER_H

#include "vs1063_uc.h"

int VSTestInitHardware(void);
int VSTestInitSoftware(void);
int VSTestHandleFile(const char *fileName, int record);

void WriteSci(u_int8 addr, u_int16 data);
u_int16 ReadSci(u_int8 addr);
int WriteSdi(const u_int8 *data, u_int8 bytes);
void WriteSpiByteSDI(u_int8 data);
void SaveUIState(void);
void RestoreUIState(void);
int GetUICommand(void);

#endif
