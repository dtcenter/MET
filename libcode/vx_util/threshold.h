// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2007
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
////////////////////////////////////////////////////////////////////////

#ifndef  __VERIF_THRESHOLD_H__
#define  __VERIF_THRESHOLD_H__

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

////////////////////////////////////////////////////////////////////////
//
// Class to store a threshold value and the corresponding threshold
// type
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
      SingleThresh & operator=(const SingleThresh &);
      int operator==(const SingleThresh &);

      double     thresh; // Threshold value
      ThreshType type;   // Threshold type

      void       clear();

      void       set(double, ThreshType);
      void       set(const char *);

      double     get_thresh() const;
      ThreshType get_type() const;
      void       get_str(char *) const;
      void       get_str(char *, int) const;
      void       get_abbr_str(char *) const;
      void       get_abbr_str(char *, int) const;

      int        check(double);
};

////////////////////////////////////////////////////////////////////////

inline double     SingleThresh::get_thresh() const { return(thresh); }
inline ThreshType SingleThresh::get_type()   const { return(type);   }

////////////////////////////////////////////////////////////////////////

extern int check_threshold(double, double, int);

////////////////////////////////////////////////////////////////////////

#endif   //  __VERIF_THRESHOLD_H__

////////////////////////////////////////////////////////////////////////
