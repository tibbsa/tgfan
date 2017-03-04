/************************************************************************/
/* FAN 0.99A4                                                           */
/* Main header file                                                     */
/************************************************************************/
#ifndef __FAN_H
#define __FAN_H

#include <errno.h>
#include <alib.h>
//#include "e:\source\resource\bbs\jam\api\jammb.h"
#include "fan_cfg.h"
#include "fan_proc.h"
#include "fan_data.h"
#include "fan_sdt.h"
#include "fan_stat.h"

// ������������������������������������������������������������������������
// PROGRAM NAME / VERSION INFORMATION
// ������������������������������������������������������������������������
#define PROG         "Telegard File Announcer"
#define PROG_SHORT   "TgFan"
#define PROG_VERSION "1.0"
#define COPYRIGHT    "1998 by Anthony Tibbs"
//#define DEV_DISCLAIM " BETA"

// ������������������������������������������������������������������������
// FUNCTION PROTOTYPES                  
// ������������������������������������������������������������������������
int main (int argc, char **argv);
void Banner (void);
int CommandLine (int argc, char **argv);
void NoMemory (void);
void MakeFancySize (char *buffer, long amount);
int  MultiNodeCheck (void);
void MultiNodeSet (void);
void MultiNodeClear (void);

#endif