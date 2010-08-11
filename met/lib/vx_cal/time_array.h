// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2007
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*



////////////////////////////////////////////////////////////////////////


#ifndef  __TIME_ARRAY_H__
#define  __TIME_ARRAY_H__


////////////////////////////////////////////////////////////////////////


#include <iostream>


////////////////////////////////////////////////////////////////////////


static const int time_array_alloc_inc = 1000;


////////////////////////////////////////////////////////////////////////


class TimeArray {

   private:

      void init_from_scratch();

      void assign(const TimeArray &);

      void extend(int);

      unixtime * e;

      int Nelements;

      int Nalloc;

   public:

      TimeArray();
     ~TimeArray();
      TimeArray(const TimeArray &);
      TimeArray & operator=(const TimeArray &);

      void clear();

      double operator[](int) const;

      int  has(unixtime) const;
      void add(unixtime);
      void add(const TimeArray &);

      void set(int, unixtime);

      unixtime min() const;
      unixtime max() const;

      int n_elements() const;

};


////////////////////////////////////////////////////////////////////////


inline int TimeArray::n_elements() const { return ( Nelements ); }


////////////////////////////////////////////////////////////////////////


#endif   /*  __TIME_ARRAY_H__  */


////////////////////////////////////////////////////////////////////////
