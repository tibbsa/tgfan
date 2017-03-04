/************************************************************************/
/* FAN 0.99A3                                                           */
/* "Last Scan Date Pointer" Update/Load Functions                       */
/************************************************************************/

#include <fan.h>
Set_Alert_File ("SCANDATE.CPP")

// ออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออ
// void LoadScanDate (char *filename);
// ฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤ
// Parameters: char *filename       .EXE file to load scandate from (argv[0])
// Return:     
// ฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤ
// Loads the LSD value from the end of the file `filename' (normally, this
// should be FAN.EXE)
// ออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออ
void LoadScanDate (char *filename)
{
FILE *fp;
long Ident;

   Alert (filename == NULL);
   Alert (*filename == '\0');

   fp = _fsopen (filename, "rb", SH_DENYWR);
   if (!fp)
   {
      LastScanDate = 0L;
      textcolor (12);
      cprintf ("WARNING: Can't open file \"%s\" - can't get \"last scan date\"!\r\n", filename);
      alib.log.Put ("!Can't open \"%s\" to read last scan date", filename);
      textcolor (7);
      return;
   }

   fread (&Ident, 1, sizeof Ident, fp);
   if (Ident != SCANDATE_ID)
   {
      textcolor (12);
      cprintf ("Last scan date is not present\r\n");
      textcolor (7);
      alib.log.Put ("!Last scan date not present");
      fclose (fp);
      return;
   }
   fread (&LastScanDate, 1, sizeof LastScanDate, fp);
   fclose (fp);
}

// ออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออ
// void SaveScanDate (char *filename);
// ฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤ
// Parameters: char *filename       .EXE file to save scandate to (argv[0])
// Return:     
// ฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤ
// Saves the LSD value to the end of the file `filename' (normally, this
// should be FAN.EXE)
// ออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออ
void SaveScanDate (char *filename)
{
FILE *fp;
long Ident;

   Alert (filename == NULL);
   Alert (*filename == '\0');
   if (!ResetDateFlag)
      LastScanDate = time (NULL);
   else
      LastScanDate = 0L;

   fp = _fsopen (filename, "wb", SH_DENYNO);
   if (!fp)
   {
      textcolor (12);
      cprintf ("WARNING: Can't open file \"%s\" - can't set \"last scan date\"!\r\n", filename);
      alib.log.Put ("!Can't open \"%s\" to write last scan date", filename);
      textcolor (7);
      return;
   }
   Ident = SCANDATE_ID;

   fwrite (&Ident, 1, sizeof Ident, fp);
   fwrite (&LastScanDate, 1, sizeof LastScanDate, fp);
   fclose (fp);
}
