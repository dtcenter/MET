// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2007
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*



////////////////////////////////////////////////////////////////////////


#ifndef  __DOUBLE_ARRAY_H__
#define  __DOUBLE_ARRAY_H__


////////////////////////////////////////////////////////////////////////


#include <iostream>


////////////////////////////////////////////////////////////////////////


static const int doublearray_alloc_inc = 100;


////////////////////////////////////////////////////////////////////////


class DoubleArray {

   private:

      void init_from_scratch();

      void assign(const DoubleArray &);

      void extend(int);


      double * e;

      int Nelements;

      int Nalloc;

      int StatsDone;

      double Min;
      double Max;

      double P10;
      double P25;
      double P50;
      double P75;
      double P90;

      double Mean;
      double StdDev;
      double Sum;

   public:

      DoubleArray();
     ~DoubleArray();
      DoubleArray(const DoubleArray &);
      DoubleArray & operator=(const DoubleArray &);

      void clear();

      void dump(ostream &, int depth = 0) const;

      double operator[](int) const;

      int has(double) const;

      void add(double);
      void add(const DoubleArray &);

      int n_elements() const;

      void do_stats();

      double min() const;
      double max() const;

      double p10() const;
      double p25() const;
      double p50() const;
      double p75() const;
      double p90() const;

      double mean() const;
      double std_dev() const;
      double sum() const;

};


////////////////////////////////////////////////////////////////////////


inline int DoubleArray::n_elements() const { return ( Nelements ); }

inline double DoubleArray::min() const { return ( Min ); }
inline double DoubleArray::max() const { return ( Max ); }

inline double DoubleArray::p10() const { return ( P10 ); }
inline double DoubleArray::p25() const { return ( P25 ); }
inline double DoubleArray::p50() const { return ( P50 ); }
inline double DoubleArray::p75() const { return ( P75 ); }
inline double DoubleArray::p90() const { return ( P90 ); }

inline double DoubleArray::mean()    const { return ( Mean   ); }
inline double DoubleArray::std_dev() const { return ( StdDev ); }
inline double DoubleArray::sum()     const { return ( Sum    ); }


////////////////////////////////////////////////////////////////////////


#endif   /*  __DOUBLE_ARRAY_H__  */


////////////////////////////////////////////////////////////////////////


