// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2019
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*


////////////////////////////////////////////////////////////////////////


#ifndef  __DATA_AVERAGER_H__
#define  __DATA_AVERAGER_H__


////////////////////////////////////////////////////////////////////////


#include "vx_grid.h"


////////////////////////////////////////////////////////////////////////


class DataAverager {

   private:

      DataAverager(const DataAverager &);
      DataAverager & operator=(const DataAverager &);

      void init_from_scratch();

      void clear();

   public:

      int two_to_one(int x, int y) const;

      int Nx;
      int Ny;

      double * Sum;   //  allocated

      int * Counts;   //  allocated

      bool * DataOk;   //  allocated

      const Grid * grid;   //  not allocated

   public:

      DataAverager();
     ~DataAverager();

      void set(const Grid &);

      void put(double value, int x, int y);

      void put          (double value, double lat, double lon);

      int count(int x, int y) const;

      double sum(int x, int y) const;

      double ave(int x, int y) const;

      bool ok(int x, int y) const;

};


////////////////////////////////////////////////////////////////////////


inline int DataAverager::two_to_one (int x, int y) const { return ( y*Nx + x ); }

inline int    DataAverager::count(int x, int y) const { return ( Counts[two_to_one(x, y)] ); }
inline double DataAverager::sum  (int x, int y) const { return (    Sum[two_to_one(x, y)] ); }
inline bool   DataAverager::ok   (int x, int y) const { return ( DataOk[two_to_one(x, y)] ); }


////////////////////////////////////////////////////////////////////////


#endif   /*  __DATA_AVERAGER_H__  */


////////////////////////////////////////////////////////////////////////




