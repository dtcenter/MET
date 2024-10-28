// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2024
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*



////////////////////////////////////////////////////////////////////////


#ifndef  __NUM_ARRAY_H__
#define  __NUM_ARRAY_H__


////////////////////////////////////////////////////////////////////////


#include <iostream>

#include "concat_string.h"
#include "is_bad_data.h"


////////////////////////////////////////////////////////////////////////


class NumArray {

   private:

      void init_from_scratch();

      void assign(const NumArray &);

      std::vector<double> e;

      bool Sorted;

   public:

      NumArray();
     ~NumArray();
      NumArray(const NumArray &);
      NumArray & operator=(const NumArray &);
      bool operator==(const NumArray &) const;

      void clear();

      void erase();

      void extend(int);

      void dump(std::ostream &, int depth = 0) const;

      double operator[](int) const;

      const std::vector<double> & vals() const;
      double * buf();
      
      int has(int, bool forward=true)    const;
      int has(double, bool forward=true) const;

      bool is_const(double) const;

      void add(int);
      void add(double);
      void add(const NumArray &);
      void add_seq(int, int);
      void add_const(double, int);
      void add_css(const char *);
      void add_css_sec(const char *);

      void set(int);
      void set(double);
      void set(int, int);
      void set(int, double);
      void set_const(double, int);

      // Increment value
      void inc(int, int);
      void inc(int, double);

      void   sort_array(bool increasing=true);
      void   reorder(const NumArray &);
      int    rank_array(int &);
      double percentile_array(double);
      double compute_percentile(double, bool) const;
      double iqr();
      void   compute_mean_variance(double &, double &) const;
      void   compute_mean_stdev(double &, double &) const;
      double sum() const;
      double mode() const;
      double min() const;
      double max() const;
      double range() const;

      ConcatString serialize() const;
      ConcatString summarize() const;

      int n_elements() const;
      int n() const;         //  same as n_elements()
      int n_valid() const;

      NumArray subset(int, int) const;

      NumArray subset(const NumArray &) const;

      double mean() const;
      double mean_sqrt() const;
      double mean_fisher() const;
      double mean_abs_diff() const;
      double wmean_abs_diff() const;
      
      double variance(int skip_index = bad_data_int) const;
      double stdev(int skip_index = bad_data_int) const;

      double wmean(const NumArray &) const;
      double wmean_sqrt(const NumArray &) const;
      double wmean_fisher(const NumArray &) const;

};


////////////////////////////////////////////////////////////////////////


inline int            NumArray::n_elements()         const { return e.size();   }
inline int            NumArray::n()                  const { return e.size();   }
inline const std::vector<double> &
                      NumArray::vals()               const { return e;          }
inline       double * NumArray::buf()                      { return e.data();   }
inline void           NumArray::inc(int i, int v)          { e[i] += v; return; }
inline void           NumArray::inc(int i, double v)       { e[i] += v; return; }


////////////////////////////////////////////////////////////////////////


extern ConcatString write_css       (const NumArray &);
extern ConcatString write_css_hhmmss(const NumArray &);


////////////////////////////////////////////////////////////////////////


#endif   /*  __NUM_ARRAY_H__  */


////////////////////////////////////////////////////////////////////////
