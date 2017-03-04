/************************************************************************/
/* FAN 0.99A3                                                           */
/* Processing functions (main)                                          */
/************************************************************************/

#include <fan.h>
Set_Alert_File ("PROCESS.CPP")

int (*bbsOpenFileArea)(int areanumber) = NULL;
void (*bbsCloseFileArea)(void) = NULL;
int (*bbsReadNextFile)(FileRecordInfo *f) = NULL;
char *(*bbsGetAreaName)(void) = NULL;

// Macros for above
#define OpenFileArea(x)          (*bbsOpenFileArea)(x)
#define CloseFileArea()          (*bbsCloseFileArea)()
#define ReadNextFile(x)          (*bbsReadNextFile)(x)
#define GetAreaName()            (*bbsGetAreaName)()

char *MsgBuff=NULL;
int FinalMessage=0;
/* ------------------------------------------------------------------------*/
Statistics AreaTotals, GrandTotals;

// ÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍ
// int Process (void)
// ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
// Parameters: 
// Return:     1 if OK, 0 if error
// ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
// The main 'processing loop' of FAN.  Does NOT deal with individual files
// or BBStype-specific stuff (nor should it deal with message writing), but
// acts as the control center for the processing stage.
// ÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍ
int Process (void)
{
int CurFileArea=0;
FileRecordInfo f;
char Temp[120];
int DidSomething = 0, FirstThisArea = 0;
int c;

   /* We only support Telegard right now. */
   bbsOpenFileArea = TG_openarea;
   bbsCloseFileArea = TG_closearea;
   bbsReadNextFile = TG_getfile;
   bbsGetAreaName = TG_getareaname;

#ifdef DEBUG
   alib.log.Put ("@File area access hooks:");
   alib.log.Put ("@  (*bbsOpenFileArea)(area&)        %p", bbsOpenFileArea);
   alib.log.Put ("@  (*bbsCloseFileArea)()            %p", bbsCloseFileArea);
   alib.log.Put ("@  (*bbsReadNextFile(fileinfo&)     %p", bbsReadNextFile);
   alib.log.Put ("@  (*bbsGetAreaName)()              %p", bbsGetAreaName);
#endif
   MsgBuff = (char *)MALLOC (config->MaxLength + (config->MaxLength/2));
   if (!MsgBuff)
      NoMemory();

   memset (MsgBuff, NUL, (config->MaxLength + (config->MaxLength/2)));

   while (config->FileAreas[CurFileArea])
   {
   int areanum=config->FileAreas[CurFileArea];

      FirstThisArea=1;
      AreaTotals.Reset();

      if (!OpenFileArea(areanum))
         goto nextarea;

      textcolor (10);
      cprintf ("Processing file area %-5d %s\r\n", areanum, GetAreaName());
      clreol();

      while (ReadNextFile(&f))
      {
         if (file_in_finished_index (f))
            continue;

         /* If this file is NOT newer, don't even bother with it. */
         if (f.upload_date < LastScanDate)
            continue;

         if (!DidSomething)
         {
            ImportMsgHeader();
            DidSomething=1;
         }

         if (FirstThisArea)
         {
            memset (&Temp, NUL, sizeof Temp);
            alib.log.Put ("#Area %d (%s)", areanum, GetAreaName());
            if (config->highascii)
            {
               for (c=0; c < 79; c++)
                  Temp[c] = 'Í';
            }
            else
            {
               for (c=0; c < 79; c++)
                  Temp[c] = '=';
            }

            strcat (MsgBuff, "\r");
            strcat (MsgBuff, Temp);
            sprintf (Temp, "\rFILE AREA %d: %s\r", areanum, GetAreaName());
            strcat (MsgBuff, Temp);

            if (config->highascii)
            {
               for (c=0; c < 79; c++)
                  Temp[c] = 'Í';
            }
            else
            {
               for (c=0; c < 79; c++)
                  Temp[c] = '=';
            }
            strcat (MsgBuff, Temp);
            strcat (MsgBuff, "\r");
            FirstThisArea=0;
         }
         AddFileToIndex (f);

         AddFile (f);
         AreaTotals.AddFile();
         AreaTotals.AddBytes (f.size);
         GrandTotals.AddFile();
         GrandTotals.AddBytes (f.size);

         if (strlen(MsgBuff) >= config->MaxLength - 100)
         {
            alib.log.Put (":Message split at offset %ld", strlen(MsgBuff));
            strcat (MsgBuff, "(* Continued on next message... *)\r");
            if (!WriteMessage())
            {
               FREE (MsgBuff);
               CloseFileArea();
               return (True);
            }
            memset (MsgBuff, NUL, (config->MaxLength + (config->MaxLength/2)));
            strcpy (MsgBuff, "(* ...Continued from previous message *)\r");
         }
        
      }

nextarea:
      if (config->IncludeStats && !FirstThisArea)
      {
      char buf [35];
         
         if (config->highascii)
         {
            for (c=0; c < 79; c++)
               Temp[c] = 'Ä';
         }
         else
         {
            for (c=0; c < 79; c++)
               Temp[c] = '-';
         }
         strcat (MsgBuff, Temp);
         strcat (MsgBuff, "\rArea Totals: ");
         MakeFancySize (buf, AreaTotals.GetBytes());
         sprintf (Temp, "%s", buf);

         strcat (Temp, " in ");
         sprintf (buf, "%d", AreaTotals.GetFiles());
         strcat (Temp, buf);
         strcat (Temp, " file");
         if (AreaTotals.GetFiles() != 1)
            strcat (Temp, "s");

         strcat (Temp, "\r");
         strcat (MsgBuff, Temp);
         AreaTotals.Reset();
      }

      CloseFileArea();
      CurFileArea++;
   }

   if (DidSomething)
   {
      FinalMessage = True;
      if (!WriteMessage())
      {
         FREE (MsgBuff);
         return (True);
      }

   }

   FREE (MsgBuff);
   return (False);
}

// ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
// void ImportMsgHeader (void)
// ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
// Arguments: 
// Return:    
// ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
// Imports the reply message header.
// ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
void ImportMsgHeader (void)
{
FILE *fp;

   if (!*config->header)
      return;

   fp = _fsopen (config->header, "r", SH_DENYWR);
   if (!fp)
   {
      strcpy (MsgBuff, "SYSTEM FAILURE: Header missing\r\r");
      alib.log.Put ("!Header file \"%s\" missing", config->header);
      return;
   }
   *MsgBuff = '\0';

   while (!feof(fp))
   {
   int i;
   char szLine [120];

      *szLine = '\0';
      fgets (szLine, 120, fp);
      for (i=strlen (szLine) - 1; (szLine[i] == '\r' || szLine[i] == '\n') && i >= 0; i--);
      szLine[i+1] = '\0';
      strcat (szLine, "\r");
      strcat (MsgBuff, szLine);
   }

   fclose (fp);
   return;
}

// ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
// void ImportMsgFooter (void)
// ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
// Arguments: 
// Return:    
// ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
// Imports the reply message footer.
// ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
void ImportMsgFooter (void)
{
FILE *fp;

   if (!*config->footer)
      return;

   fp = _fsopen (config->footer, "r", SH_DENYWR);
   if (!fp)
   {
      strcat (MsgBuff, "\r\rSYSTEM FAILURE: Footer missing\r");
      alib.log.Put ("!Footer file \"%s\" missing", config->footer);
      return;
   }

   while (!feof(fp))
   {
   int i;
   char szLine [120];

      *szLine = '\0';
      fgets (szLine, 120, fp);
      for (i=strlen (szLine) - 1; (szLine[i] == '\r' || szLine[i] == '\n') && i >= 0; i--);
      szLine[i+1] = '\0';
      strcat (szLine, "\r");
      strcat (MsgBuff, szLine);
   }

   fclose (fp);
   return;
}

// ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
// void AddFile (FileRecordInfo f)
// ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
// Arguments: FileREcordInfo f      File information
// Return:    
// ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
// Adds a file to the message.
// ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
void AddFile (FileRecordInfo f)
{
char temp[128];
char temp2[128];
char FileID [90];
char *Comment;
char *formatted_comment;
extern FILE*TgFp_Desc;
int n;
   formatted_comment = (char *)MALLOC (4096);
   if (!formatted_comment)
      NoMemory();

   memset (&temp, NUL, sizeof temp);
   MakeFancySize (temp, f.size);
   textcolor (11);
   cprintf ("   New file: ");
   textcolor (14);
   cprintf ("%s", f.filename);
   textcolor (9);
   cprintf (" (%s)\r\n", temp);
   sprintf (temp2, "%s", temp);
   sprintf (temp, "%-4.4s", temp2);

   sprintf (temp2, "%-12.12s %-4.4s ", f.filename, temp);
   strcpy (FileID, temp2);
   struct tm *TMNOW;
   TMNOW = localtime (&f.file_date);
   strftime (temp2, 128, "%m/%d/%y  ", TMNOW);
   strcat (FileID, temp2);
   alib.log.Put ("-   File: %s", FileID);

   if (!f.description_length)
      strcpy (formatted_comment, "(No description available)");
   else
   {
   char *q, *p;

      Comment = (char *)MALLOC (f.description_length + 4);
      if (!Comment)
         NoMemory();
   
      fread (Comment, 1, f.description_length, TgFp_Desc);
      *(Comment + f.description_length) = '\0';

      p = formatted_comment;
      q = Comment;
      while (*q)
      {
         if (*q == 13)
         {
            if (!*(q+1))
            {
               q++;
               continue;
            }
   
            *p = '\r';
            p++;
            /* Insert 28 spaces. */
            for (n=1; n <= 28; n++)
            {
               *p = ' ';
               p++;
            }
            q++;
   
            continue;
         }
         *p = *q;
         q++;
         p++;
      }
      *p = '\0';
   }

   strcat (MsgBuff, FileID);
   strcat (MsgBuff, formatted_comment);
   strcat (MsgBuff, "\r");
   FREE (Comment);
   FREE (formatted_comment);
}

// ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
// int WriteMessage (void)
// ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
// Arguments: 
// Return:    1 if OK, 0 on error
// ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
// Prepares and writes a message ine ach area.
// ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
int WriteMessage (void)
{
int CurMsgArea=0;
long MID;
char buf [400];

   memset (&buf, '\0', sizeof buf);
   if (FinalMessage)
   {
      if (config->IncludeStats)
      {
      char Temp[120];
      int c;
   
         strcat (MsgBuff, "\rTotals: ");
         MakeFancySize (buf, GrandTotals.GetBytes());
         sprintf (Temp, "%s", buf);
      
         strcat (Temp, " in ");
         sprintf (buf, "%d", GrandTotals.GetFiles());
         strcat (Temp, buf);
         strcat (Temp, " file");
         if (GrandTotals.GetFiles() != 1)
            strcat (Temp, "s");
         strcat (Temp, ".\r");

         strcat (MsgBuff, Temp);
      }
   
      ImportMsgFooter();
   }

   while (config->MessageAreas[CurMsgArea] != NULL)
   {
   MessageArea *t = config->MessageAreas[CurMsgArea];

      if (t->Format == 0)
         WriteSquish (t);
      else
         WriteJAM (t);
//      WriteSquish (t);

      CurMsgArea++;
   }

     
   return (True);
}

// ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
// int WriteJAM (void)
// ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
// Arguments: 
// Return:    1 if OK, 0 on error
// ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
// Writes the message to a JAM area.                
// ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
int WriteJAM (MessageArea *t)
{
JAMMsgHeader hdr;
char *mbuffer;
char string [1024];
char Temp [1024];

   Alert (t == NULL);

   if (!JamOpenBase (t->path))
      if (!JamCreateBase (t->path))
      {
         alib.log.Put ("!Error opening message base (%s)!", t->path);
         textcolor (12);
         cprintf ("\r\nError opening JAM message base \"%s\"!\r\n", t->path);
         return (False);
      }

   mbuffer = MsgBuff;
   while (*mbuffer)
      mbuffer++;

   JamInitMsgWrite (&hdr);
   JamAddSubfield (JSUB_From, t->from);
   JamAddSubfield (JSUB_To, t->to);
   JamAddSubfield (JSUB_Subject, t->subj);
   sprintf (string, t->addr.point != 0 ? "%u:%u/%u.%u" : "%u:%u/%u",
                    t->addr.zone, t->addr.net, t->addr.node, t->addr.point);
   JamAddSubfield (JSUB_OAddr, string);
   JamAddSubfield (JSUB_DAddr, string);
   if (t->addr.point)
      sprintf (string, "%u:%u/%u.%u %-8lx",
                        t->addr.zone, t->addr.net, t->addr.node,
                        t->addr.point, MakeMsgID());
   else
      sprintf (string, "%u:%u/%u %-8lx",
                        t->addr.zone, t->addr.net, t->addr.node, MakeMsgID());

   JamAddSubfield (JSUB_MSGID, string);
   sprintf (string, "%s %s", PROG_SHORT, PROG_VERSION);
   JamAddSubfield (JSUB_PID, string);

   sprintf (Temp, "\r--- %s %s\r", PROG_SHORT, PROG_VERSION);
   strcat (MsgBuff, Temp);
   sprintf (Temp, " * Origin: %s (", t->origin);
   strcat (MsgBuff, Temp);
   if (t->addr.point)
      sprintf (string, "%u:%u/%u.%u)\r",
                        t->addr.zone, t->addr.net, t->addr.node,
                        t->addr.point);
   else
      sprintf (string, "%u:%u/%u)\r",
                        t->addr.zone, t->addr.net, t->addr.node);
   strcat (MsgBuff, string);

   hdr.attr1 = JMSG_LOCAL|JMSG_TYPEECHO;
   if (!JamSaveMessage (&hdr, MsgBuff))
   {
      *mbuffer = NUL;
      textcolor (12);
      cprintf ("\r\nError: Unable to write message to JAM base \"%s\"!\r\n", t->path);
      JamCloseBase();
      return (False);
   }
   *mbuffer = NUL;
   JamCloseBase();
   cprintf ("\rMessage written to JAM area %s                 \r\n", t->areaid);
   return 0;
}

// ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
// int WriteSquish (void)
// ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
// Arguments: 
// Return:    1 if OK, 0 on error
// ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
// Writes the message to a Squish area.                
// ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
int WriteSquish (MessageArea *t)
{
MAPI msgapi;
MAREA marea;
char Temp [512];
char Work [512];
dword MID;
char buf [1024];

#ifdef DEBUG
   alib.log.Put ("@Writing to Squish area");
#endif

   if (!msgapi.Init (t->addr.zone))
   {
      textcolor (12);
      cprintf ("Error initializing MsgAPI!\r\n");
      return (False);
   }

   if (!marea.open (t->path, MSGAREA_CRIFNEC, MSGTYPE_SQUISH))
   {
      msgapi.Close();
      textcolor (12);
      cprintf ("Error opening message base \"%s\"!", t->path);
      return (False);
   }

   Message msg (marea);
   char *mbuffer;

   if (!msg.Open (MOPEN_CREATE, 0L))
   {
      marea.close();
      msgapi.Close();
      textcolor (12);
      cprintf ("Error %d creating message!\r\n", msgapierr);
      return (False);
   }

   MID = MakeMsgID();
   sprintf (buf, "\x01MSGID: %d:%d/%d.%d %-8.8lx\x01PID: %s %s", t->addr.zone,
                                                t->addr.net,
                                                t->addr.node,
                                                t->addr.point,
                                                MID,
                                                PROG_SHORT, PROG_VERSION);

   strcpy (msg.header.to, t->to);
   strcpy (msg.header.from, t->from);
   strcpy (msg.header.subj, t->subj);
   msg.header.attr = MSGLOCAL;
   memcpy (&msg.header.orig, &t->addr, sizeof (NETADDR));
   memcpy (&msg.header.dest, &t->addr, sizeof (NETADDR));
   msg.header.date_written = *GetTimeStamp (time(NULL));
   msg.header.date_arrived = *GetTimeStamp (time(NULL));

   mbuffer = MsgBuff;
   while (*mbuffer)
      mbuffer++;

   sprintf (Temp, "\r--- %s %s\r", PROG_SHORT, PROG_VERSION);
   strcat (MsgBuff, Temp);

   sprintf (Temp, " * Origin: %s (", t->origin);
   if (t->addr.point)
      sprintf (Work, "%d:%d/%d.%d)\r", t->addr.zone,
                                       t->addr.net,
                                       t->addr.node,
                                       t->addr.point);
   else
      sprintf (Work, "%d:%d/%d)\r", t->addr.zone, t->addr.net, t->addr.node);
   strcat (Temp, Work);
   strcat (MsgBuff, Temp);
   
   if (!msg.Write (0, MsgBuff, buf))
   {
      msg.Close();
      marea.close();
      msgapi.Close();
      textcolor (12);
      cprintf ("Error %d writing message!\r\n", msgapierr);
      return (False);
   }

   *mbuffer = '\0';

   msg.Close();
   marea.close();
   msgapi.Close();
   textcolor (10);
   cprintf ("\rMessage written to Squish area %s                 \r\n", t->areaid);

   return 0;
}

// ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
// static struct _stamp *GetTimeStamp (time_t tt)
// ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
// Arguments: time_t tt             Time_t value to use
// Return:    Pointer to static _stamp structure.
// ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
// Converts a time_t value to a _stamp.
// ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
static struct _stamp *GetTimeStamp (time_t tt)
{
    static struct _stamp st;
    struct tm *tms;

    tms = localtime(&tt);
    st.time.ss = tms->tm_sec >> 1;
    st.time.mm = tms->tm_min;
    st.time.hh = tms->tm_hour;
    st.date.da = tms->tm_mday;
    st.date.mo = tms->tm_mon + 1;
    st.date.yr = tms->tm_year - 80;
    return (&st);
}

// ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
// unsigned long MakeMsgID (void)
// ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
// Arguments: 
// Return:    MSGID
// ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
// Creates a unique MsgID.
// ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
unsigned long MakeMsgID (void)
{
    static unsigned long old_id = 0;
    unsigned long i;
    time_t now;
    struct tm *t;

    now = time(NULL);
    t = localtime(&now);
    i = (t->tm_sec * 10L) +
        (t->tm_min * 600L) +
        (t->tm_hour * 36000L) +
        (t->tm_mday * 864000L) +
        (t->tm_mon * 26784000L) +
        (t->tm_year * 321408000L);

    if (i <= old_id)
        i = old_id + 1;

    return (old_id = i);
}

// ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
// void AddFileToIndex (FileRecordInfo f);
// ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
// Arguments: FileRecordInfo f            File record info to add
// Return:    
// ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
// Checks to see if 'f' is in the 'finished' file, and adds it if it isn't.
// ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
void AddFileToIndex (FileRecordInfo f)
{
FILE *fp;

   if (!file_in_finished_index (f))
   {
      fp = _fsopen ("FAN_DONE.IDX", "ab", SH_DENYWR);
      if (!fp)
      {
         alib.log.Put ("!Unable to add record to FAN_DONE.IDX");
         textcolor (12);
         cprintf ("\r\nError adding record to FAN_DONE.IDX\r\n");
         return;
      }

      fwrite (&f, sizeof (f), 1, fp);
      fclose (fp);
      alib.log.Put (">Record for %s added to FAN_DONE.IDX", f.filename);
      return;
   }
}

// ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
// int file_in_finished_index (FileRecordInfo f);
// ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
// Arguments: FileRecordInfo f            File record info to check
// Return:    1=there, 0=not there
// ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
// Checks to see if 'f' is in the 'finished' file
// ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
int file_in_finished_index (FileRecordInfo f)
{
FILE *fp;
FileRecordInfo x;

   fp = _fsopen ("FAN_DONE.IDX", "rb", SH_DENYWR);
   if (!fp)
   {
      alib.log.Put ("!Unable to open FAN_DONE.IDX");
      textcolor (12);
      cprintf ("\r\nError opening FAN_DONE.IDX\r\n");
      return 0;
   }

   while (!feof (fp))
   {
      fread (&x, sizeof (x), 1, fp);
      if (x.size == f.size)
         if (!strcmpi (f.filename, x.filename))
            if (x.upload_date == f.upload_date)
               if (x.file_date == f.file_date)
               {
                  alib.log.Put ("!File %s already in index!", x.filename);
                  fclose (fp);
                  return 1;
               }
   }

   fclose (fp);

   return 0;
}
