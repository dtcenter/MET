

////////////////////////////////////////////////////////////////////////


#ifndef  __MET_DOUBLE_ARRAY_H__
#define  __MET_DOUBLE_ARRAY_H__


////////////////////////////////////////////////////////////////////////


#include <iostream>


////////////////////////////////////////////////////////////////////////


static const int met_doublearray_alloc_inc = 100;


////////////////////////////////////////////////////////////////////////


class MetDoubleArray {

   private:

      void init_from_scratch();

      void assign(const MetDoubleArray &);

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

   public:

      MetDoubleArray();
     ~MetDoubleArray();
      MetDoubleArray(const MetDoubleArray &);
      MetDoubleArray & operator=(const MetDoubleArray &);

      void clear();

      void dump(ostream &, int depth = 0) const;

      double operator[](int) const;

      int has(double) const;

      void add(double);
      void add(const MetDoubleArray &);

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

};


////////////////////////////////////////////////////////////////////////


inline int MetDoubleArray::n_elements() const { return ( Nelements ); }

inline double MetDoubleArray::min() const { return ( Min ); }
inline double MetDoubleArray::max() const { return ( Max ); }

inline double MetDoubleArray::p10() const { return ( P10 ); }
inline double MetDoubleArray::p25() const { return ( P25 ); }
inline double MetDoubleArray::p50() const { return ( P50 ); }
inline double MetDoubleArray::p75() const { return ( P75 ); }
inline double MetDoubleArray::p90() const { return ( P90 ); }

inline double MetDoubleArray::mean()    const { return ( Mean   ); }
inline double MetDoubleArray::std_dev() const { return ( StdDev ); }


////////////////////////////////////////////////////////////////////////


#endif   /*  __MET_DOUBLE_ARRAY_H__  */


////////////////////////////////////////////////////////////////////////


