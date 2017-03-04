/************************************************************************/
/* FAN 0.99A4                                                           */
/* Statistics functions                                                 */
/************************************************************************/
#ifndef __FAN_STAT.H
#define __FAN_STAT.H

class Statistics
{
protected:
   long  files, bytes;

public:
   Statistics() {Reset();}
   void  Reset (void) {files=0L; bytes=0L;}
   long  GetFiles (void) {return files;}
   long  GetBytes (void) {return bytes;}
   void  AddFile (void) {files++;}
   void  AddBytes (long amount){bytes+=amount;}
};

#endif
