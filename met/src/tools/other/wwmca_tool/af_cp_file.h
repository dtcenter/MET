// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2020
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*


////////////////////////////////////////////////////////////////////////


#ifndef  __AF_CLOUD_PCT_FILE_H__
#define  __AF_CLOUD_PCT_FILE_H__


////////////////////////////////////////////////////////////////////////


#include <iostream>

#include "vx_cal.h"
#include "vx_util.h"
#include "vx_grid.h"

#include "af_file.h"


////////////////////////////////////////////////////////////////////////


class AFCloudPctFile : public AFDataFile {

   private:

      void init_from_scratch();

      void assign(const AFCloudPctFile &);

      unsigned char * Buf;

   public:

      AFCloudPctFile();
      virtual ~AFCloudPctFile();
      AFCloudPctFile(const AFCloudPctFile &);
      AFCloudPctFile & operator=(const AFCloudPctFile &);

      void clear();

      int cloud_pct(int x, int y) const;

      virtual int operator()(int x, int y) const;

      virtual bool read(const char * filename, const char hemisphere);

};


////////////////////////////////////////////////////////////////////////


inline int AFCloudPctFile::operator()(int x, int y) const { return ( cloud_pct(x, y) ); }


////////////////////////////////////////////////////////////////////////


#endif   /*  __AF_CLOUD_PCT_FILE_H__  */


////////////////////////////////////////////////////////////////////////


