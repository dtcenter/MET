// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2020
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

      unixtime * e;

      int Nelements;

      int Nalloc;

      bool Sorted;

   public:

      TimeArray();
     ~TimeArray();
      TimeArray(const TimeArray &);
      TimeArray & operator=(const TimeArray &);

      void clear();

      void erase();

      void extend(int, bool exact = true);

      void dump(ostream &, int depth = 0) const;

      unixtime operator[](int) const;

      int  has(unixtime) const;
      int  index(unixtime) const;
      void add(unixtime);
      void add(const TimeArray &);
      void add_css(const char *);

      void set(int, unixtime);

      void sort_array();

      void equal_dt(TimeArray &beg, TimeArray &end) const;

      TimeArray subset(int, int) const;

      unixtime min() const;
      unixtime max() const;

      int n_elements() const;
      int n() const;

};


////////////////////////////////////////////////////////////////////////


inline int TimeArray::n_elements() const { return ( Nelements ); }
inline int TimeArray::n()          const { return ( Nelements ); }


////////////////////////////////////////////////////////////////////////


#endif   /*  __TIME_ARRAY_H__  */


////////////////////////////////////////////////////////////////////////
