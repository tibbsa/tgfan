/************************************************************************/
/* FAN 0.99A3                                                           */
/************************************************************************/

#include <fan.h>
Set_Alert_File ("FAN.CPP")

extern unsigned _stklen = 16384U;
int ForceSaveScanDate=False;                   /* Force scan date save. */

// ออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออ
// int main (int argc, char **argv)        
// ฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤ
// Parameters: 
// Return:     0 = OK, 1 = Error
// ฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤ
// No comment necassary.
// ออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออ
int main (int argc, char **argv)
{
char errorlevel=0;
   tzset();
   timezone = 0;
   daylight = 0;

   Banner();
   if (!CommandLine (argc, argv))
   {
      errorlevel = 1;
      goto Cleanup;
   }

   if (MultiNodeCheck() == 1)                   /* 1 == In Use */
   {
      textattr (12);
      cprintf ("Error: TgFan is already running on another node!\r\n");
      errorlevel = 2;
      goto Cleanup;
   }
   MultiNodeSet();

   /* Parse the configuration file */
   if (!ParseConfig())
   {
      errorlevel = 1;
      goto Cleanup;
   }

#ifdef DEBUG
   if (!alib.log.Open (config->logfile, "", "FAN ", True))
#else
   if (!alib.log.Open (config->logfile, "", "FAN ", False))
#endif
   {
      textcolor (12);
      cprintf ("Error opening log file \"%s\"\r\n", config->logfile);
      errorlevel = 1;
      goto Cleanup;
   }
   alib.log.Put ("-");
   alib.log.Put ("+%s begin (mem=%ldk)", PROG_SHORT, (coreleft()/1024L));
#ifdef DEV_DISCLAIM
   alib.log.Put ("!THIS IS A%s RELEASE OF %s!", DEV_DISCLAIM, PROG_SHORT);
   alib.log.Put ("!USE OF THIS SOFTWARE IS AT YOUR OWN RISK!");
#endif

   LoadScanDate ("TGFAN.LSD");

   errorlevel = Process();
   if (errorlevel == 0)
      SaveScanDate ("TGFAN.LSD");

Cleanup:
   MultiNodeClear();
   if (ForceSaveScanDate)
      SaveScanDate ("TGFAN.LSD");

   DeallocateConfigMemory();
   alib.log.Put ("+%s end", PROG_SHORT, (coreleft()/1024L));
   alib.log.Close();
   return errorlevel;
}

// ออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออ
// void Banner (void)
// ฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤ
// Parameters: 
// Return:     
// ฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤ
// Displays the program name/copyright banner/disclaimer message.
// ออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออ
void Banner (void)
{
   textcolor (14);
   cprintf ("%s, Version %s\r\n", PROG, PROG_VERSION);
   cprintf ("Copyright (C) %s\r\n", COPYRIGHT);
   textcolor (7);
}

// ออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออ
// int CommandLine (int argc, char **argv)
// ฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤ
// Parameters: (argc/argv)       Arguments passed from main()
// Return:     0=Error, 1=OK
// ฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤ
// Parses any command line arguments that may have been passed to FAN.
// ออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออ
int CommandLine (int argc, char **argv)
{
int count;
char *p;

   Alert (argv == NULL);
   for (count=1; count < argc; count++)
   {
      p = argv [count];
      if (*p != '/' && *p != '-')
      {
         textcolor (12);
         cprintf ("Command line error: Switches must start with '/' or '-'!\r\n");
         goto ShowHelp;
      }

      p++;

      switch (toupper(*p))
      {
         case 'H':
         case '?':   goto ShowHelp;
         case 'C':   p++;
                     if (!*p || isspace(*p))
                     {
                        textcolor (12);
                        cprintf ("Command line error: Config filename not specified\r\n");
                        goto ShowHelp;
                     }
                     strcpy (ConfigFilename, p);
                     break;

         case 'R':   textcolor (15);
                     ForceSaveScanDate = True;
                     cprintf ("Last scan date reset to today!\r\n");
                     textcolor (7);
                     return (False);

         case '0':   textcolor (15);
                     ForceSaveScanDate = True;
                     ResetDateFlag = True;
                     cprintf ("Last scan date reset to Jan 1, 1970!\r\n");
                     textcolor (7);
                     return (False);

         default :   textcolor (12);
                     cprintf ("Unknown switch: %s\r\n", argv [count]);
                     goto ShowHelp;
      }
   }

   return (True);

ShowHelp:
   textcolor (7);
   cprintf ("Usage:  FAN [switches]\r\n");
   cprintf ("        -? or -H       Command line help (this screen)\r\n");
   cprintf ("        -C<file>       Specifies alternate configuration file <file>\r\n");
   cprintf ("        -R             Reset last scan date (set to today)\r\n");
   cprintf ("        -0             (Debugging) Reset LSD to Jan 1, 1970\r\n");

   return (False);
}

// ออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออ
// void NoMemory (void)
// ฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤ
// Parameters: 
// Return:     
// ฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤ
// This is called when FAN runs out of memory, to display an error message,
// and exit as GRACEFULLY as possible.
// ออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออ
void NoMemory (void)
{
   textcolor (12);
   cprintf ("\r\nOUT OF MEMORY! - attempting graceful exit\r\n");
   alib.log.Put ("!OUT OF MEMORY");
   alib.log.Put ("!Emergancy exit, last scan date NOT updated");
   alib.log.Put ("+%s end (mem=%ldk)", PROG_SHORT, (coreleft()/1024L));
   alib.log.Close();
   DeallocateConfigMemory();
   exit (255);
}

// ฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤ
// void MakeFancySize (char *buffer, long amount);
// ฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤ
// Arguments: char *buffer          Pointer to buffer to use
//            long amount           Number to convert
// Return:    
// ฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤ
// Makes a fancy file size (1.5k, 957k, 2.9M, etc.) out of a number
// ฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤ
void MakeFancySize (char *buffer, long amount)
{
long n;
char x[25];

   if (amount > 1000000L)
   {
   long c=0L;

      n = (amount / 1000000L);
      if (n < 10)
         c = amount % 1000000L;

      sprintf (buffer, "%ld", n);
      if (c)
      {
         sprintf (x, ".%.1ld", c);
         if (x[2] >= '5')
         {
            if (x[1] < '9')
               x[1] ++;
         }
         x[2] = '\0';
         strcat (buffer, x);
      }
      strcat (buffer, "M");
      return;
   }

   if (amount >= 1024L)
   {
   long c=0L;

      n = amount/1024L;
      sprintf (buffer, "%ld", n);
      if (n < 10)
         c = amount % 1024L;
      if (c)
      {
         sprintf (x, ".%.1ld", c);
         if (x[2] >= '5')
         {
            if (x[1] < '9')
               x[1] ++;
         }
         x[2] = '\0';
         strcat (buffer, x);
      }
      strcat (buffer, "k");
      return;
   }
   sprintf (buffer, "%ldb", amount);
}

// ออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออ
// void MultiNodeSet (void)
// ฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤ
// Parameters: 
// Return:     
// ฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤ
// Creates the file named 'TGFAN.USE' which signals other nodes NOT to run.
// ออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออ
void MultiNodeSet (void)
{
FILE *f;

   f = fopen ("TGFAN.USE", "w");
   if (!f)
   {
      textattr (12);
      cprintf ("Warning: Unable to set multi-node lock (err=%d)!\r\n", errno);
      return;
   }
   fclose (f);
   textattr (10);
   cprintf ("Multinode lock set\r\n");
}

// ออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออ
// int MultiNodeCheck (void)
// ฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤ
// Parameters: 
// Return:     1=In use, 0=Clear
// ฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤ
// Checks to see if 'TGFAN.USE' exists.  
// ออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออ
int MultiNodeCheck (void)
{
int result;
struct ffblk f;

   result = findfirst ("TGFAN.USE", &f, 0);
   if (!result)
      return 1;
   else
      return 0;
}

// ออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออ
// void MultiNodeClear (void)
// ฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤ
// Parameters: 
// Return:     
// ฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤ
// Clears TGFAN.USE, if it exists.
// ออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออ
void MultiNodeClear (void)
{
   if (MultiNodeCheck())
   {
      if (unlink("TGFAN.USE") == -1)
      {
         textattr (12);
         cprintf ("Confused: Can't clear multinode lock?\r\n");
         return;
      }
   }
}
