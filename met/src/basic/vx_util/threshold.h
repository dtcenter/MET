// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2014
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
////////////////////////////////////////////////////////////////////////

#ifndef  __THRESHOLD_H__
#define  __THRESHOLD_H__

////////////////////////////////////////////////////////////////////////

#include <iostream>

#include "concat_string.h"

////////////////////////////////////////////////////////////////////////

//
// Enumeration to indicate the different ways of thresholding a field
//
enum ThreshType {
   thresh_na = 0,
   thresh_lt = 1,
   thresh_le = 2,
   thresh_eq = 3,
   thresh_ne = 4,
   thresh_gt = 5,
   thresh_ge = 6
};

static const int n_thresh_type = 7;
static const char * const thresh_type_str[n_thresh_type] = {
   "na", "<", "<=", "=", "!=", ">", ">="
};
static const char * const thresh_abbr_str[n_thresh_type] = {
   "na", "lt", "le", "eq", "ne", "gt", "ge"
};

static const int thresh_default_precision = 3;

////////////////////////////////////////////////////////////////////////
//
// Class to store a threshold value and type
//
////////////////////////////////////////////////////////////////////////

class SingleThresh {

   private:
      void init_from_scratch();
      void assign(const SingleThresh &);

   public:

      SingleThresh();
      ~SingleThresh();
      SingleThresh(const SingleThresh &);
      SingleThresh(const char *);      
      SingleThresh & operator=(const SingleThresh &);

      void dump(ostream &, int = 0) const;

      bool operator==(const SingleThresh &) const;

      double       thresh; // Threshold value
      ThreshType   type;   // Threshold type

      void         clear();

      void         set(double, ThreshType);
      void         set(const char *);

      double       get_thresh() const;
      ThreshType   get_type() const;
      ConcatString get_str(int precision = thresh_default_precision) const;
      void         get_str(char *, int precision = thresh_default_precision) const;
      ConcatString get_abbr_str(int precision = thresh_default_precision) const;
      void         get_abbr_str(char *, int precision = thresh_default_precision) const;

      bool         check(double) const;
};

////////////////////////////////////////////////////////////////////////

inline double     SingleThresh::get_thresh() const { return(thresh); }
inline ThreshType SingleThresh::get_type()   const { return(type);   }

////////////////////////////////////////////////////////////////////////

extern bool check_threshold(double, double, int);

////////////////////////////////////////////////////////////////////////

#endif   //  __THRESHOLD_H__

////////////////////////////////////////////////////////////////////////
