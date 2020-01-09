// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2020
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*


////////////////////////////////////////////////////////////////////////


#ifndef  __AF_PIXEL_TIME_FILE_H__
#define  __AF_PIXEL_TIME_FILE_H__


////////////////////////////////////////////////////////////////////////


#include <iostream>

#include "vx_cal.h"
#include "vx_util.h"
#include "vx_grid.h"

#include "af_file.h"


////////////////////////////////////////////////////////////////////////


class AFPixelTimeFile : public AFDataFile {

   private:

      void init_from_scratch();

      void assign(const AFPixelTimeFile &);

      unsigned char * Buf;

      bool SwapEndian;

   public:

      AFPixelTimeFile();
      virtual ~AFPixelTimeFile();
      AFPixelTimeFile(const AFPixelTimeFile &);
      AFPixelTimeFile & operator=(const AFPixelTimeFile &);

      void clear();

      void set_swap_endian(bool b);

      int pixel_age_sec(int x, int y) const;

      virtual int operator()(int x, int y) const;

      virtual bool read(const char * filename, const char hemisphere);

};


////////////////////////////////////////////////////////////////////////


inline void AFPixelTimeFile::set_swap_endian(bool b)        { SwapEndian = b; return;         }
inline int  AFPixelTimeFile::operator()(int x, int y) const { return ( pixel_age_sec(x, y) ); }


////////////////////////////////////////////////////////////////////////


#endif   /*  __AF_PIXEL_TIME_FILE_H__  */


////////////////////////////////////////////////////////////////////////


