/************************************************************************/
/* FAN 0.99A3                                                           */
/* Data module (data only)                                              */
/************************************************************************/

#include <fan.h>


time_t  LastScanDate=0L;                     // Date/time FAN was last run
Config *config=NULL;                         // Configuration information
char    ConfigFilename [120]="FAN.CFG";
int     ResetDateFlag = False;               // Just resetting to 0?

