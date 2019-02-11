// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2019
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*



////////////////////////////////////////////////////////////////////////


#ifndef  __TIME_SERIES_H__
#define  __TIME_SERIES_H__


////////////////////////////////////////////////////////////////////////


#include <iostream>

#include "vx_cal.h"


////////////////////////////////////////////////////////////////////////


class TimeSeries {

   private:

      void init_from_scratch();

      void assign(const TimeSeries &);


      unixtime TimeStart;

      int TimeDelta;   //  seconds

      double * Value;

      int Nelements;

   public:

      TimeSeries();
     ~TimeSeries();
      TimeSeries(const TimeSeries &);
      TimeSeries & operator=(const TimeSeries &);

      void clear();

      void dump(ostream &, int depth = 0) const;

      void set(unixtime _start, int _delta, int _n);

      unixtime time_start() const;

      int time_delta() const;   //  seconds

      int n_elements() const;

      double operator()(int index)  const;   // get values

      void put(double, int index);

};


////////////////////////////////////////////////////////////////////////


inline unixtime TimeSeries::time_start() const { return ( TimeStart ); }
inline int      TimeSeries::time_delta() const { return ( TimeDelta ); }

inline int      TimeSeries::n_elements() const { return ( Nelements ); }


////////////////////////////////////////////////////////////////////////


#endif   /*  __TIME_SERIES_H__  */


////////////////////////////////////////////////////////////////////////


