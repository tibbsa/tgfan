/************************************************************************/
/* FAN 0.99A4                                                           */
/* Processing prototypes/definitions                                    */
/************************************************************************/
#ifndef __FAN_PROC.H
#define __FAN_PROC.H

typedef struct
{
   word     description_length;
   char     *description_buffer;
   char     filename [13];
   long     size;
   time_t   upload_date;
   time_t   file_date;
} FileRecordInfo;

int Process (void);
void AddFile (FileRecordInfo f);
void ImportMsgFooter (void);
void ImportMsgHeader (void);
int WriteMessage (void);
static struct _stamp *GetTimeStamp (time_t tt);
unsigned long MakeMsgID (void);
int WriteSquish (MessageArea *t);
int WriteJAM (MessageArea *t);

void AddFileToIndex (FileRecordInfo f);
int file_in_finished_index (FileRecordInfo f);

int TG_openarea (int areanumber);
void TG_closearea (void);
int TG_getfile (FileRecordInfo *f);
char *TG_getareaname (void);


#endif
