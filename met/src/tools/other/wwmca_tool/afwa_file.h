// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2015
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*


////////////////////////////////////////////////////////////////////////


#ifndef  __AFWA_DATA_FILE_H__
#define  __AFWA_DATA_FILE_H__


////////////////////////////////////////////////////////////////////////


#include <iostream>

#include "vx_cal.h"
#include "vx_util.h"
#include "vx_grid.h"


////////////////////////////////////////////////////////////////////////


static const int afwa_nx = 1024;
static const int afwa_ny = 1024;


////////////////////////////////////////////////////////////////////////


class AfwaDataFile {

   protected:

      void init_from_scratch();

      void assign(const AfwaDataFile &);

      int two_to_one(int, int) const;


      const Grid * grid;   //  allocated

      ConcatString Filename;

      char Hemisphere;  //  'N' or 'S'

      unixtime Valid;

   public:

      AfwaDataFile();
      virtual ~AfwaDataFile();
      AfwaDataFile(const AfwaDataFile &);
      AfwaDataFile & operator=(const AfwaDataFile &);

      void clear();

      bool xy_is_ok(int x, int y) const;

      //
      // inline member functions
      //

      int nx() const;
      int ny() const;

      unixtime valid() const;

      char hemisphere() const;

      //
      // pure virtual member functions
      //

      virtual bool read(const char * filename, const char hemisphere) = 0;

      virtual int operator()(int x, int y) const = 0;

};


////////////////////////////////////////////////////////////////////////


inline int AfwaDataFile::nx() const { return ( afwa_nx ); }
inline int AfwaDataFile::ny() const { return ( afwa_ny ); }

inline unixtime AfwaDataFile::valid      () const { return ( Valid ); }
inline char     AfwaDataFile::hemisphere () const { return ( Hemisphere ); }


////////////////////////////////////////////////////////////////////////


#endif   /*  __AFWA_DATA_FILE_H__  */


////////////////////////////////////////////////////////////////////////


