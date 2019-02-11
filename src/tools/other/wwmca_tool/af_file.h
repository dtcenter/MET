// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2019
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*


////////////////////////////////////////////////////////////////////////


#ifndef  __AF_DATA_FILE_H__
#define  __AF_DATA_FILE_H__


////////////////////////////////////////////////////////////////////////


#include <iostream>

#include "vx_cal.h"
#include "vx_util.h"
#include "vx_grid.h"


////////////////////////////////////////////////////////////////////////


static const int af_nx = 1024;
static const int af_ny = 1024;


////////////////////////////////////////////////////////////////////////


class AFDataFile {

   protected:

      void init_from_scratch();

      void assign(const AFDataFile &);

      int two_to_one(int, int) const;


      const Grid * grid;   //  allocated

      ConcatString Filename;

      char Hemisphere;  //  'N' or 'S'

      unixtime Init;

      unixtime Valid;

   public:

      AFDataFile();
      virtual ~AFDataFile();
      AFDataFile(const AFDataFile &);
      AFDataFile & operator=(const AFDataFile &);

      void clear();

      bool xy_is_ok(int x, int y) const;

      //
      // inline member functions
      //

      int nx() const;
      int ny() const;

      unixtime init() const;

      unixtime valid() const;

      char hemisphere() const;

      virtual bool read(const char * filename, const char hemisphere);

      //
      // pure virtual member functions
      //

      virtual int operator()(int x, int y) const = 0;

};


////////////////////////////////////////////////////////////////////////


inline int AFDataFile::nx() const { return ( af_nx ); }
inline int AFDataFile::ny() const { return ( af_ny ); }

inline unixtime AFDataFile::init       () const { return ( Init ); }
inline unixtime AFDataFile::valid      () const { return ( Valid ); }
inline char     AFDataFile::hemisphere () const { return ( Hemisphere ); }


////////////////////////////////////////////////////////////////////////


extern void parse_af_filename(const char * filename, char & Hemisphere,
                              unixtime & Valid, unixtime & Init);


////////////////////////////////////////////////////////////////////////


#endif   /*  __AF_DATA_FILE_H__  */


////////////////////////////////////////////////////////////////////////


