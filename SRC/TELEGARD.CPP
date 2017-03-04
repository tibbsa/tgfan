/************************************************************************/
/* FAN 0.99A3                                                           */
/* Telegard support functions                                           */
/************************************************************************/

#include <fan.h>
Set_Alert_File ("HOOKS/TELEGARD.CPP")

FILE        *TgFp_Data=NULL;                    /* *.FA? */
FILE        *TgFp_Desc=NULL;                    /* *.FD? */
configrec   TgInfo;                             /* To get data path */
int         HaveTgInfo=0;
farearec    TgFileAreaInfo;

// ออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออ
// int TG_openarea (int areanumber)
// ฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤ
// Parameters: int areanumber       Area # to open data files for
// Return:     1 if OK, 0 if error
// ฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤ
// Opens a teleagrd file area.  (Uses config->tgpath\\data)
// ออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออ
int TG_openarea (int areanumber)
{
char tmp[120];

   if (!HaveTgInfo)
   {
   static int Tried=0;

      if (!Tried)
      {
         /* We have to find out where FAREAS.DAT is. */
   #ifdef DEBUG
         alib.log.Put ("@Grabbing CONFIG.TG information");
   #endif
         if (!tgReadConfig (config->tgpath, &TgInfo))
         {
            textcolor (12);
            cprintf ("Error reading %s\\CONFIG.TG!\r\n", config->tgpath);
            Tried=1;
   
            return (False);
         }
         HaveTgInfo=1;
      }
      else
         return (False);
   }
   sprintf (tmp, "%sFAREAS.DAT", TgInfo.datapath);
   if (!TgFp_Data)
   {
      TgFp_Data = _fsopen (tmp, "rb", SH_DENYWR);
      if (!TgFp_Data)
      {
         alib.log.Put ("!File area data file \"%s\" can't be opened!", tmp);
         textcolor (12);
         cprintf ("Can't open file area data file \"%s\"\r\n", tmp);
         return (False);
      }
   }

   if (filelength(fileno(TgFp_Data)) < (sizeof(farearec) * areanumber))
   {
      alib.log.Put ("!File area %d doesn't exist!", areanumber);
#ifdef DEBUG
      alib.log.Put (">  (filelength check failed)");
#endif
      textcolor (12);
      cprintf ("File area %d doesn't exist!\r\n", areanumber);
      fclose (TgFp_Data);
      return (False);
   }

   fseek (TgFp_Data, sizeof (farearec) * areanumber, SEEK_SET);
   if (fread(&TgFileAreaInfo, 1, sizeof (farearec), TgFp_Data) != sizeof (farearec))
   {
      if (feof(TgFp_Data))
      {
         alib.log.Put ("!File area %d doesn't exist!", areanumber);
   #ifdef DEBUG
         alib.log.Put (">  (fread & feof check failed)");
   #endif
         textcolor (12);
         cprintf ("File area %d doesn't exist!\r\n", areanumber);
         fclose (TgFp_Data);
         return (False);
      }

      alib.log.Put ("!Error reading FAREAS.DAT!");
      textcolor (12);
      cprintf ("Erro reading \"%s\"!\r\n", tmp);
      fclose (TgFp_Data);
      return (False);
   }

   P2Cconvert (TgFileAreaInfo.desc);
   P2Cconvert (TgFileAreaInfo.filename);
   P2Cconvert (TgFileAreaInfo.path);
   P2Cconvert (TgFileAreaInfo.filepath);

   fclose (TgFp_Data);

   /* -------------------------------------------------------------------- */
   /* NOW, let's actually open the FILE AREA.                              */
   /* -------------------------------------------------------------------- */
   kill_filename_trail_slash (TgFileAreaInfo.filepath);
   sprintf (tmp, "%s\\%s.fa", TgFileAreaInfo.filepath, TgFileAreaInfo.filename);
   TgFp_Data = _fsopen (tmp, "rb", SH_DENYNO);
   if (!TgFp_Data)
   {
      alib.log.Put ("!File area index \"%s\" not found!", tmp);
      textcolor (12);
      cprintf ("File area index \"%s\" not found!\r\n", tmp);
      return (False);
   }

   sprintf (tmp, "%s\\%s.fad", TgFileAreaInfo.filepath, TgFileAreaInfo.filename);
   TgFp_Desc = _fsopen (tmp, "rb", SH_DENYNO);
   if (!TgFp_Desc)
   {
      alib.log.Put ("!File description database \"%s\" not found!", tmp);
      textcolor (12);
      cprintf ("File description database \"%s\" not found!\r\n", tmp);
      return (False);
   }

   return (True);
}

// ออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออ
// void TG_closearea (void)
// ฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤ
// Parameters: 
// Return:     
// ฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤ
// Closes all file handles/memory for a telegard file aarea.
// ออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออ
void TG_closearea (void)
{
   if (TgFp_Data)
   {
      fclose (TgFp_Data);
      TgFp_Data=NULL;
   }
   if (TgFp_Desc)
   {
      fclose (TgFp_Desc);
      TgFp_Desc=NULL;
   }
}

// ออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออ
// int TG_getfile (FileRecordInfo *f)
// ฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤ
// Parameters: FileRecordInfo *f    Pointer to struct to fill out
// Return:     1=OK, 0=Error
// ฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤ
// Reads the next file into memory.
// ออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออ
int TG_getfile (FileRecordInfo *f)
{
fbrec frec;
char *p;
char *q;
int tmp;
char vfn [15];

   Alert (TgFp_Data == NULL);
   Alert (TgFp_Desc == NULL);

TryAgain:
   fread (&frec, 1, sizeof frec, TgFp_Data);
   if (feof(TgFp_Data))
      return (False);
   P2Cconvert (frec.filename);

   /* If this file is offline or unvalidated, skip it. */
   if (frec.filestatus.offline || (!frec.filestatus.validated) || (!frec.filestatus.available))
   {
      alib.log.Put ("!File %s is offline", frec.filename);
      goto TryAgain;
   }


   p = f->filename;
   q = frec.filename;
   for (tmp=0; tmp < 8; tmp++)
   {
      *p = *q;
      p++;
      q++;
   }
   *p = '\0';
   strtrim (f->filename);
   p=f->filename;
   while (*p)
      p++;

   while (*q)
   {
      *p = *q;
      q++;
      p++;
   }
   *p = '\0';
   strtrim (f->filename);

   f->description_length = frec.desclength;
   f->size = frec.size;
   f->upload_date = frec.uldate;
   f->file_date = frec.filedate;

   fseek (TgFp_Desc, frec.descofs, SEEK_SET);
   fread (&vfn, 13, 1, TgFp_Desc);
   P2Cconvert (vfn);
   if (strcmpi(vfn, frec.filename) != 0)
   {
      alib.log.Put ("!Description on %s invalid", frec.filename);
      f->description_length = 0;
   }

time_t tsp;
struct tm *tup;

   tup = gmtime (&frec.uldate);
   f->upload_date = mktime (tup);
   tup = gmtime (&frec.uldate);
   f->upload_date = mktime (tup);

#ifdef DEBUG
char sb[120];

   timezone = 0;
   daylight = 0;
   tsp = time(NULL);
   
   timezone = 0;
   daylight = 0;
   strcpy (sb, ctime (&tsp));
   p = sb;
   while (*p)
      p++;
   p--;
   *p = '\0';

   alib.log.Put ("@--- ChkIsNew() AFTER mktime()-------------------");
   alib.log.Put ("@Current time_t: 0x%lx (%s / %ld)", tsp, sb, tsp);
   tsp = f->upload_date;
   timezone = 0;
   daylight = 0;
   strcpy (sb, ctime (&tsp));
   p = sb;
   while (*p)
      p++;
   p--;
   *p = '\0';

   alib.log.Put ("@Upload time_t : 0x%lx (%s / %ld)", tsp, sb, tsp);
   tsp = LastScanDate;
   timezone = 0;
   daylight = 0;
   strcpy (sb, ctime (&tsp));
   p = sb;
   while (*p)
      p++;
   p--;
   *p = '\0';

   alib.log.Put ("@Last scan date: 0x%lx (%s / %ld)", tsp, sb, tsp);
#endif

   return (True);
}

// ออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออ
// char *TG_getareaname (void)
// ฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤ
// Parameters: 
// Return:     Area name
// ฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤ
// Returns the name of the file area.
// ออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออ
char *TG_getareaname (void)
{
   return (TgFileAreaInfo.desc);
}
