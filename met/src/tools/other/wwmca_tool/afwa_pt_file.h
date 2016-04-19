// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2015
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*


////////////////////////////////////////////////////////////////////////


#ifndef  __AFWA_PIXEL_TIME_FILE_H__
#define  __AFWA_PIXEL_TIME_FILE_H__


////////////////////////////////////////////////////////////////////////


#include <iostream>

#include "vx_cal.h"
#include "vx_util.h"
#include "vx_grid.h"

#include "afwa_file.h"


////////////////////////////////////////////////////////////////////////


class AfwaPixelTimeFile : public AfwaDataFile {

   private:

      void init_from_scratch();

      void assign(const AfwaPixelTimeFile &);

      unsigned char * Buf;

   public:

      AfwaPixelTimeFile();
      virtual ~AfwaPixelTimeFile();
      AfwaPixelTimeFile(const AfwaPixelTimeFile &);
      AfwaPixelTimeFile & operator=(const AfwaPixelTimeFile &);

      void clear();

      int pixel_age_sec(int x, int y) const;

      virtual int operator()(int x, int y) const;

      virtual bool read(const char * filename, const char hemisphere);

};


////////////////////////////////////////////////////////////////////////


inline int AfwaPixelTimeFile::operator()(int x, int y) const { return ( pixel_age_sec(x, y) ); }


////////////////////////////////////////////////////////////////////////


extern bool is_afwa_pixel_time_filename(const char * filename, char & Hemisphere, unixtime & Valid);


////////////////////////////////////////////////////////////////////////


#endif   /*  __AFWA_PIXEL_TIME_FILE_H__  */


////////////////////////////////////////////////////////////////////////


