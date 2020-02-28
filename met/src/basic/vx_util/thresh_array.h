// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2020
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
////////////////////////////////////////////////////////////////////////

#ifndef  __THRESH_ARRAY_H__
#define  __THRESH_ARRAY_H__

////////////////////////////////////////////////////////////////////////

#include <iostream>
#include <vector>

#include "concat_string.h"
#include "threshold.h"

////////////////////////////////////////////////////////////////////////

class ThreshArray {

   public:

      void init_from_scratch();
      void assign(const ThreshArray &);

      SingleThresh * t;

      int Nelements;
      int Nalloc;

   public:

      ThreshArray();
     ~ThreshArray();
      ThreshArray(const ThreshArray &);
      ThreshArray & operator=(const ThreshArray &);

      void clear();

      void extend(int, bool exact = true);

      void dump(ostream &, int depth = 0) const;

      bool operator==(const ThreshArray &) const;
      SingleThresh operator[](int) const;

      const SingleThresh * thresh() const;
      SingleThresh * buf() const;

      void add(const SingleThresh &);
      void add(const double, const ThreshType);
      void add(const char *);
      void add(const ThreshArray &);
      void add_css(const char *);

      void parse_thresh_str(const char *);

      int n_elements() const;
      int n() const;

      int has(const SingleThresh &) const;
      int has(const SingleThresh &, int & index) const;

      bool need_perc();
      void set_perc(const NumArray *, const NumArray *, const NumArray *);
      void set_perc(const NumArray *, const NumArray *, const NumArray *,
                    const ThreshArray *, const ThreshArray *);

      void multiply_by(const double);

      void get_simple_nodes(vector<Simple_Node> &);

      ConcatString get_str(const char * = thresh_default_sep,
                           int precision = thresh_default_precision) const;
      ConcatString get_abbr_str(const char * = thresh_default_sep,
                                int precision = thresh_default_precision) const;

      void check_bin_thresh() const;
      int check_bins(double) const;
      int check_bins(double, double, double) const;

      bool check_dbl(double) const;
      bool check_dbl(double, double, double) const;
};

////////////////////////////////////////////////////////////////////////

inline int                  ThreshArray::n_elements() const { return ( Nelements ); }
inline int                  ThreshArray::n()          const { return ( Nelements ); }
inline const SingleThresh * ThreshArray::thresh()     const { return ( t );         }
inline       SingleThresh * ThreshArray::buf()        const { return ( t );         }

////////////////////////////////////////////////////////////////////////

extern ThreshArray  string_to_prob_thresh    (const char *);
extern ConcatString prob_thresh_to_string    (const ThreshArray &); 
extern bool         check_prob_thresh        (const ThreshArray &, bool error_out = true);
extern ThreshArray  process_perc_thresh_bins (const ThreshArray &);
extern ThreshArray  process_rps_cdp_thresh   (const ThreshArray &);
extern ConcatString write_css                (const ThreshArray &);


////////////////////////////////////////////////////////////////////////

#endif   /*  __THRESH_ARRAY_H__  */

////////////////////////////////////////////////////////////////////////
