/************************************************************************/
/* FAN 0.99A3                                                           */
/* Configuration parsing/control                                        */
/************************************************************************/

#include <fan.h>
Set_Alert_File ("CONFIG.CPP")

// ออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออ
// void InitConfig (void)
// ฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤ
// Parameters: 
// Return:     
// ฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤ
// Allocates memory for the configuration, and sets default values.
// ออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออ
void InitConfig (void)
{
   config = (Config *)MALLOC (sizeof (Config));
   if (!config)
      NoMemory ();
   strcpy (config->tgpath, "C:\\TG");
   memset (&config->MessageAreas, NUL, sizeof config->MessageAreas);
   memset (&config->FileAreas, NUL, sizeof config->FileAreas);
   strcpy (config->logfile, "FAN.LOG");
   memset (&config->header, NUL, sizeof config->header);
   memset (&config->footer, NUL, sizeof config->footer);
   config->IncludeStats = True;
}

// ออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออ
// void DeallocateConfigMemory (void)
// ฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤ
// Parameters: 
// Return:     
// ฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤ
// Deallocates any memory allocated for the configuration.
// ออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออ
void DeallocateConfigMemory (void)
{
   if (!config)
      return;

   int x=0;
   while (config->MessageAreas[x])
   {
      FREE (config->MessageAreas[x]);
      config->MessageAreas[x] = NULL;
      x++;
   }

   FREE (config);
   config=NULL;
}

// ออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออ
// int ParseConfig (void)
// ฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤ
// Parameters: 
// Return:     1 if OK, 0 if error
// ฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤ
// Parses the config file named by "ConfigFilename[]"
// ออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออ
int ParseConfig (void)
{
FILE *fp;
char line [250];
char *p;
long linenum=0L;
int InMsgDef=0;
int CurMessageArea=0;
int CurFileArea=0;
MessageArea *t;
int pathset=0;

   InitConfig();

   if (!fexist (ConfigFilename))
   {
      textcolor (12);
      cprintf ("Config file \"%s\" does not exist!\r\n", ConfigFilename);
      return (False);
   }

   fp = _fsopen (ConfigFilename, "r", SH_DENYWR);
   if (!fp)
   {
      textcolor (12);
      cprintf ("Config file \"%s\" could not be opened!\r\n", ConfigFilename);
      return (False);
   }

   textcolor (15);
   cprintf ("Reading configuration from \"%s\"\r\n", ConfigFilename);
   textcolor (7);

   while (!feof(fp) && !ConfigError(NULL, 0L) && (fgets(line, 250, fp) != NULL))
   {
      linenum++;

      strtrim (line);
      p=line;
      if (*p == ';' || *p == '\r' || *p == '\n' || !*p)
         continue;

      p = strtok (line, " \t\r\n;");
      if (!p || *p == '\r' || *p == '\n' || *p == ';' || !*p)
         continue;


      if (!strcmpi(p, "Log"))
      {
         p = strtok (NULL, " \r\n\t");
         if (!p)
            ConfigError ("Log file not specified", linenum);
         else
         {
            strcpy (config->logfile, p);
            strtrim (config->logfile);
         }
         continue;
      }

      if (!strcmpi(p, "Telegard"))
      {
         p = strtok (NULL, " \r\n");
         if (!p)
            ConfigError ("Path not specified", linenum);
         else
         {
            strtrim (p);
            strcpy (config->tgpath, p);
         }
         continue;
      }

      if (!strcmpi(p, "NoTotals"))
      {
         config->IncludeStats = False;
         continue;
      }

      if (!strcmpi(p, "MaxSize"))
      {
         p = strtok (NULL, " \r\n");
         if (!p)
            ConfigError ("Size not specified", linenum);
         else
         {
            strtrim (p);
            config->MaxLength = atoi (p);
         }
         continue;
      }
         

      if (!strcmpi(p, "FileArea"))
      {
         p = strtok (NULL, " \t\r\n");
         while (p)
         {
         int start, end;
            start = end = 0;

            if (sscanf (p, "%d-%d", &start, &end) < 1)
            {
               ConfigError ("Invalid file area number specified", linenum);
               break;
            }
            else
            {
               if (end == 0)
               {
                  if (CurFileArea==1024)
                  {
                     ConfigError ("TgFan currently supports only up to 1,024 file areas!", linenum);
                     continue;
                  }
                  config->FileAreas[CurFileArea] = start;
                  CurFileArea++;
               }
               else
               {
               int c;

                  for (c=start; c <= end; c++)
                  {
                     if (CurFileArea==1024)
                        break;
                     config->FileAreas[CurFileArea] = c;
                     CurFileArea++;
                  }
               }
               p = strtok (NULL, " \t\r\n");
            }
         }
         continue;
      }
      if (!strcmpi(p, "AllowHighAscii"))
      {
         config->highascii = True;
         continue;
      }
      if (!strcmpi(p, "Header"))
      {
         p = strtok (NULL, " \r\n");
         if (!p)
            ConfigError ("Header filename not specified", linenum);
         else
         {
            strtrim (p);
            strcpy (config->header, p);
         }
         continue;
      }
      if (!strcmpi(p, "Footer"))
      {
         p = strtok (NULL, " \r\n");
         if (!p)
            ConfigError ("Footer filename not specified", linenum);
         else
         {
            strtrim (p);
            strcpy (config->footer, p);
         }
         continue;
      }

      if (!strcmpi(p, "MaxSize"))
      {
         p = strtok (NULL, " \t\r\n");
         if (!p)
            ConfigError ("Size not specified", linenum);
         else
         {
            strtrim (p);
            config->MaxLength = atoi (p);
            if (config->MaxLength > 30000)
               ConfigError ("Message size too large (maximum is 30,000 bytes)", linenum);
         }
         continue;
      }
      if (!strcmpi(p, "MsgArea"))
      {
         p = strtok (NULL, "\r\n");
         if (!p)
            ConfigError ("Message area ID not specified", linenum);
         else
         {
            config->MessageAreas[CurMessageArea] = (MessageArea *)MALLOC (sizeof (MessageArea));
            if (!config->MessageAreas[CurMessageArea])
               NoMemory();
            strtrim (p);

            t = config->MessageAreas[CurMessageArea];

            strcpy (t->areaid, p);
            sprintf (t->from, "%s %s", PROG_SHORT, PROG_VERSION);
            strcpy (t->to, "All");
            strcpy (t->subj, "New Files!");
            strcpy (t->path, "");
            t->addr.zone = t->addr.net = t->addr.node = t->addr.point = 0;
            strcpy (t->origin, "");
            t->Format = 0;
            InMsgDef=1;
            pathset=0;
         }
         continue;
      }
      if (InMsgDef)
      {
         if (!strcmpi(p, "Format"))
         {
            p = strtok (NULL, " \t\r\n");
            if (!p)
               ConfigError ("Format not specified", linenum);
            else
            {
               if (!strcmpi(p, "squish"))
                  t->Format = 0;
               else if (!strcmpi(p, "jam"))
                  t->Format = 1;
               else
                  ConfigError ("Unknown format specified", linenum);
            }
            continue;
         }

         if (!strcmpi(p, "From"))
         {
            p = strtok (NULL, "\r\n");
            if (!p)
               ConfigError ("From: name not specified", linenum);
            else
            {
               strtrim (p);
               strcpy (t->from, p);
            }
            continue;
         }
         if (!strcmpi(p, "To"))
         {
            p = strtok (NULL, "\r\n");
            if (!p)
               ConfigError ("To: name not specified", linenum);
            else
            {
               strtrim (p);
               strcpy (t->to, p);
            }
            continue;
         }
         if (!strcmpi(p, "Subj"))
         {
            p = strtok (NULL, "\r\n");
            if (!p)
               ConfigError ("Subject line not specified", linenum);
            else
            {
               strtrim (p);
               strcpy (t->subj, p);
            }
            continue;
         }
         if (!strcmpi(p, "Path"))
         {
            pathset=1;
            p = strtok (NULL, " \r\n");
            if (!p)
               ConfigError ("Path not specified", linenum);
            else
            {
               strtrim (p);
               strcpy (t->path, p);
            }
            continue;
         }
         if (!strcmpi(p, "Origin"))
         {
            p = strtok (NULL, " \t");
            if (!p)
               ConfigError ("Origin address not specified", linenum);
            else
            {
               strtrim (p);
               parsenetaddr (&t->addr, p);
               if (t->addr.zone == 0)
                  ConfigError ("Invalid address specified", linenum);
               else
               {
                  p = strtok (NULL, "\r\n");
                  if (!p)
                     ConfigError ("System name/site not specified", linenum);
                  else
                  {
                     strtrim (p);
                     strcpy (t->origin, p);
                  }
               }
            }
            continue;
         }

         if (!strcmpi(p, "End"))
         {
            if (!t->addr.zone)
            {
               textcolor (12);
               cprintf ("\r\nConfiguration error on msgarea %s:\r\n", t->areaid);
               cprintf ("   Origin address/line not set!\r\n");
               InMsgDef=0;
               ConfigError ("xxx", 0L);
               continue;
            }
            if (!pathset)
            {
               textcolor (12);
               cprintf ("\r\nConfiguration error on msgarea %s:\r\n", t->areaid);
               cprintf ("   Message base path not set!\r\n");
               InMsgDef=0;
               
               ConfigError ("xxx", 0L);
               continue;
            }

            CurMessageArea++;
            InMsgDef=0;
            continue;
         }
      }

      char Keyword [100];
      sprintf (Keyword, "Unknown keyword: %s", p);
      ConfigError (Keyword, linenum);
   }

   if (InMsgDef)
   {
      textcolor (12);
      cprintf ("ERROR: MsgArea definition for \"%s\" has no \"end\" statement!\r\n", t->areaid);
      textcolor (7);
      fclose (fp);
      return (False);
   }

   fclose (fp);

   /* Sort the file area numbers. */
   if (!ConfigError(NULL, 0L))
   {
      if (!CurFileArea)
      {
         textcolor (12);
         cprintf ("Error: No file areas have been declared!\r\n");
         textcolor (7);
         return (False);
      }
   }

   qsort (&config->FileAreas, CurFileArea, sizeof (int), FileAreaCompare);

   if (ConfigError(NULL, 0L))
      return (False);
   else
      return (True);
}

// ออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออ
// int FileAreaCompare (const void *p1, const void *p2)
// ฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤ
// Parameters: const void *p1/p2       Things to compare
// Return:     {see Qsort definition}
// ฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤ
// Used in the "qsort <fileareas>" command above.
// ออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออ
int FileAreaCompare (const void *p1, const void *p2)
{
int x, y;

   memcpy (&x, p1, sizeof (int));
   memcpy (&y, p2, sizeof (int));

   if (x < y)
      return -1;
   else if (x == y)
      return 0;
   else 
      return 1;
}
   
// ออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออ
// int ConfigError (char *str, long line)
// ฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤ
// Parameters: char *str         Error "reason" to display
//             long line         Line number error occured on
// Return:     Normally 0, but see notes
// ฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤ
// Displays a "error on line xxx in xxx: <str>" message in red.
// IF str==NULL, returns "1" if an error has occured, or "0" if one has not.
// ออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออ
int ConfigError (char *str, long line)
{
static int ErrorOccured=False;
struct text_info r;

   if (!str)
      return (ErrorOccured);

   if (!strcmpi(str, "xxx"))
   {
      ErrorOccured=True;
      return 0;
   }
   
   gettextinfo (&r);
   textcolor (12);
   cprintf ("Error on line %ld of %s: %s\r\n", line, ConfigFilename, str);
   textcolor (r.attribute);
   ErrorOccured=True;
   return 0;
}

