/************************************************************************/
/* FAN 0.99A4                                                           */
/* Configuration structure.                                             */
/************************************************************************/
#ifndef __FAN_CFG.H
#define __FAN_CFG.H

/* Limits */
#define MAXMSGAREAS     1024
#define MAXFILEAREAS    1024

typedef struct
{
   char     areaid [36];                        // Area ID name

   char     from [36];                          // From/to/subj to use when
   char     to [36];                            // posting messages in this
   char     subj [72];                          // area
   char     path [120];                         // Path to area

   NETADDR  addr;                               // Addr/origin line to use
   char     origin [120];
   int      Format;                             // 0 = Squish, other = JAM
} MessageArea;

typedef struct
{
   char           tgpath [120];                    // Path to Telegard
   MessageArea   *MessageAreas [MAXMSGAREAS];      // Areas to post in
   unsigned       FileAreas [MAXFILEAREAS];        // Areas to scan

   char           logfile [120];                   // Log file name
   word     MaxLength;                          // Approx max length of message
   char     header [120];                       // Header & footer
   char     footer [120];
   int      highascii;                          // Allow HIGH ascii
   int      IncludeStats;                       // Include file stats?

} Config;

/* Prototypes */
void InitConfig (void);
void DeallocateConfigMemory (void);
int  ParseConfig (void);
int  ConfigError (char *str, long line);
int  FileAreaCompare (const void *p1, const void *p2);

#endif
