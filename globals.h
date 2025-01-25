#include <sys/types.h>
#include <libetc.h>
#include <libgte.h>
#include <libgpu.h>

#ifndef GLOBALS_H
#define GLOBALS_H

#define OT_LEN 8192
#define PRIMBUFF_LEN 131072 // 128 kb

void EmptyOT(u_short currbuff);

void SetOTAt(u_short currbuff, u_int i, u_long value);
u_long* GetOTAt(u_short currbuff, u_int i);

void IncrementNextPrim(u_int size);
void SetNextPrim(char* value);
void ResetNextPrim(u_short currbuff); 
char* GetNextPrim(void);
 
#endif