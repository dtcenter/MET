

////////////////////////////////////////////////////////////////////////


#ifndef  __AFWA_PIXEL_TIME_FILE_H__
#define  __AFWA_PIXEL_TIME_FILE_H__


////////////////////////////////////////////////////////////////////////


#include <iostream>

#include "vx_cal.h"
#include "vx_util.h"
#include "grid.h"

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

      virtual bool read(const char * filename);

};


////////////////////////////////////////////////////////////////////////


inline int AfwaPixelTimeFile::operator()(int x, int y) const { return ( pixel_age_sec(x, y) ); }


////////////////////////////////////////////////////////////////////////


#endif   /*  __AFWA_PIXEL_TIME_FILE_H__  */


////////////////////////////////////////////////////////////////////////


