// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2007
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*



////////////////////////////////////////////////////////////////////////


#ifndef  __ANALYSIS_TOOL_BOXPLOT_H__
#define  __ANALYSIS_TOOL_BOXPLOT_H__


////////////////////////////////////////////////////////////////////////


#include <iostream>


////////////////////////////////////////////////////////////////////////


class Box {

   private:

      void init_from_scratch();

      void assign(const Box &);

      double P05;
      double P25;
      double P50;
      double P75;
      double P95;

      int N;

      int N_outliers;

      double * Outlier;

      int is_outlier(double) const;

   public:

      Box();
     ~Box();
      Box(const Box &);
      Box & operator=(const Box &);

      void clear();

      void dump(ostream & out, int depth = 0) const;


      void set_unsorted(const double *, int);

      void set_sorted(const double *, int);


      double p05() const;
      double p25() const;
      double p50() const;
      double p75() const;
      double p95() const;

      int n() const;

      double iqr() const;   //  interquartile range

      int n_outliers() const;

      double outlier(int) const;

};


////////////////////////////////////////////////////////////////////////


inline double Box::p05() const { return ( P05 ); }
inline double Box::p25() const { return ( P25 ); }
inline double Box::p50() const { return ( P50 ); }
inline double Box::p75() const { return ( P75 ); }
inline double Box::p95() const { return ( P95 ); }

inline double Box::iqr() const { return ( P75 - P25 ); }

inline int    Box::n  () const { return ( N ); }

inline int    Box::n_outliers () const { return ( N_outliers ); }

inline int    Box::is_outlier(double x) const { return ( (x < P05) || (x > P95) ); }


////////////////////////////////////////////////////////////////////////


#endif   /*  __ANALYSIS_TOOL_BOXPLOT_H__  */


////////////////////////////////////////////////////////////////////////




