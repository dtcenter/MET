// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2015
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*


////////////////////////////////////////////////////////////////////////


#ifndef  __AFWA_CLOUD_PCT_FILE_H__
#define  __AFWA_CLOUD_PCT_FILE_H__


////////////////////////////////////////////////////////////////////////


#include <iostream>

#include "vx_cal.h"
#include "vx_util.h"
#include "vx_grid.h"

#include "afwa_file.h"


////////////////////////////////////////////////////////////////////////


class AfwaCloudPctFile : public AfwaDataFile {

   private:

      void init_from_scratch();

      void assign(const AfwaCloudPctFile &);

      unsigned char * Buf;

   public:

      AfwaCloudPctFile();
      virtual ~AfwaCloudPctFile();
      AfwaCloudPctFile(const AfwaCloudPctFile &);
      AfwaCloudPctFile & operator=(const AfwaCloudPctFile &);

      void clear();

      int cloud_pct(int x, int y) const;

      virtual int operator()(int x, int y) const;

      virtual bool read(const char * filename, const char hemisphere);

};


////////////////////////////////////////////////////////////////////////


inline int AfwaCloudPctFile::operator()(int x, int y) const { return ( cloud_pct(x, y) ); }


////////////////////////////////////////////////////////////////////////


extern bool is_afwa_cloud_pct_filename(const char * filename, char & Hemisphere, unixtime & Valid);


////////////////////////////////////////////////////////////////////////


#endif   /*  __AFWA_CLOUD_PCT_FILE_H__  */


////////////////////////////////////////////////////////////////////////


