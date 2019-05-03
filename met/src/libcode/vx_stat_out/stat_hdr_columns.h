// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2012
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*

////////////////////////////////////////////////////////////////////////

#ifndef  __STAT_HDR_COLUMNS_H__
#define  __STAT_HDR_COLUMNS_H__

////////////////////////////////////////////////////////////////////////

#include "vx_util.h"
#include "vx_cal.h"
#include "vx_math.h"

////////////////////////////////////////////////////////////////////////

//
// Class to store data that's written to the output header columns
//

class StatHdrColumns {

   private:

      void init_from_scratch();

      // Model name
      ConcatString model;

      // Fcst lead time
      int          fcst_lead_sec;
      ConcatString fcst_lead_str;

      // Fcst beginning and ending valid times
      unixtime     fcst_valid_beg;
      ConcatString fcst_valid_beg_str;

      unixtime     fcst_valid_end;
      ConcatString fcst_valid_end_str;

      // Obs lead time
      int          obs_lead_sec;
      ConcatString obs_lead_str;

      // Obs beginning and ending valid times
      unixtime     obs_valid_beg;
      ConcatString obs_valid_beg_str;

      unixtime     obs_valid_end;
      ConcatString obs_valid_end_str;

      // Fcst variable and level
      ConcatString fcst_var;
      ConcatString fcst_lev;

      // Obs variable and level
      ConcatString obs_var;
      ConcatString obs_lev;

      // Obs message type
      ConcatString msg_typ;

      // Verification region
      ConcatString mask;

      // Interpolation method
      InterpMthd   interp_mthd;
      ConcatString interp_mthd_str;

      // Size of interpolation neighborhood
      int          interp_wdth;
      ConcatString interp_pnts_str;

      // Line type
      ConcatString line_type;

      // Fcst threshold
      SingleThresh fcst_thresh;
      ConcatString fcst_thresh_str;

      // Obs threshold
      SingleThresh obs_thresh;
      ConcatString obs_thresh_str;

      // Coverage field threshold
      SingleThresh cov_thresh;
      ConcatString cov_thresh_str;

      // Alpha Confidence value
      double       alpha;
      ConcatString alpha_str;

      // Private set functions
      void set_fcst_lead_str();
      void set_fcst_valid_beg_str();
      void set_fcst_valid_end_str();
      void set_obs_lead_str();
      void set_obs_valid_beg_str();
      void set_obs_valid_end_str();
      void set_interp_mthd_str();
      void set_interp_pnts_str();
      void set_fcst_thresh_str();
      void set_obs_thresh_str();
      void set_cov_thresh_str();

   public:

      StatHdrColumns();
     ~StatHdrColumns();

      void clear();

      void clear_fcst_thresh();
      void clear_obs_thresh();
      void clear_cov_thresh();

      // Set functions
      void set_model         (const char *);

      void set_fcst_lead_sec (const int);
      void set_fcst_valid_beg(const unixtime);
      void set_fcst_valid_end(const unixtime);

      void set_obs_lead_sec  (const int);
      void set_obs_valid_beg (const unixtime);
      void set_obs_valid_end (const unixtime);

      void set_fcst_var      (const char *);
      void set_fcst_lev      (const char *);

      void set_obs_var       (const char *);
      void set_obs_lev       (const char *);

      void set_msg_typ       (const char *);
      void set_mask          (const char *);

      void set_interp_mthd   (const InterpMthd);
      void set_interp_wdth   (const int);

      void set_line_type     (const char *);

      void set_fcst_thresh   (const SingleThresh);
      void set_fcst_thresh   (const ThreshArray);
      void set_obs_thresh    (const SingleThresh);
      void set_obs_thresh    (const ThreshArray);

      void set_cov_thresh    (const SingleThresh);
      void set_alpha         (const double);

      // Get functions
      const char *get_model             () const;

      int         get_fcst_lead_sec     () const;
      const char *get_fcst_lead_str     () const;

      unixtime    get_fcst_valid_beg    () const;
      const char *get_fcst_valid_beg_str() const;

      unixtime    get_fcst_valid_end    () const;
      const char *get_fcst_valid_end_str() const;

      int         get_obs_lead_sec      () const;
      const char *get_obs_lead_str      () const;

      unixtime    get_obs_valid_beg     () const;
      const char *get_obs_valid_beg_str () const;

      unixtime    get_obs_valid_end     () const;
      const char *get_obs_valid_end_str () const;

      const char *get_fcst_var          () const;
      const char *get_fcst_lev          () const;

      const char *get_obs_var           () const;
      const char *get_obs_lev           () const;

      const char *get_msg_typ           () const;
      const char *get_mask              () const;

      InterpMthd  get_interp_mthd       () const;
      const char *get_interp_mthd_str   () const;
      int         get_interp_wdth       () const;
      const char *get_interp_pnts_str   () const;

      const char *get_line_type         () const;

      SingleThresh get_fcst_thresh      () const;
      const char  *get_fcst_thresh_str  () const;

      SingleThresh get_obs_thresh       () const;
      const char  *get_obs_thresh_str   () const;

      SingleThresh get_cov_thresh       () const;
      const char  *get_cov_thresh_str   () const;
      double       get_alpha            () const;
};

////////////////////////////////////////////////////////////////////////

inline const char *StatHdrColumns::get_model             () const { return(model);               }

inline int         StatHdrColumns::get_fcst_lead_sec     () const { return(fcst_lead_sec);       }
inline const char *StatHdrColumns::get_fcst_lead_str     () const { return(fcst_lead_str);       }

inline unixtime    StatHdrColumns::get_fcst_valid_beg    () const { return(fcst_valid_beg);      }
inline const char *StatHdrColumns::get_fcst_valid_beg_str() const { return(fcst_valid_beg_str);  }

inline unixtime    StatHdrColumns::get_fcst_valid_end    () const { return(fcst_valid_end);      }
inline const char *StatHdrColumns::get_fcst_valid_end_str() const { return(fcst_valid_end_str);  }

inline int         StatHdrColumns::get_obs_lead_sec      () const { return(obs_lead_sec);        }
inline const char *StatHdrColumns::get_obs_lead_str      () const { return(obs_lead_str);        }

inline unixtime    StatHdrColumns::get_obs_valid_beg     () const { return(obs_valid_beg);       }
inline const char *StatHdrColumns::get_obs_valid_beg_str () const { return(obs_valid_beg_str);   }

inline unixtime    StatHdrColumns::get_obs_valid_end     () const { return(obs_valid_end);       }
inline const char *StatHdrColumns::get_obs_valid_end_str () const { return(obs_valid_end_str);   }

inline const char *StatHdrColumns::get_fcst_var          () const { return(fcst_var);            }
inline const char *StatHdrColumns::get_fcst_lev          () const { return(fcst_lev);            }

inline const char *StatHdrColumns::get_obs_var           () const { return(obs_var);             }
inline const char *StatHdrColumns::get_obs_lev           () const { return(obs_lev);             }

inline const char *StatHdrColumns::get_msg_typ           () const { return(msg_typ);             }
inline const char *StatHdrColumns::get_mask              () const { return(mask);                }

inline InterpMthd  StatHdrColumns::get_interp_mthd       () const { return(interp_mthd);         }
inline const char *StatHdrColumns::get_interp_mthd_str   () const { return(interp_mthd_str);     }
inline int         StatHdrColumns::get_interp_wdth       () const { return(interp_wdth);         }
inline const char *StatHdrColumns::get_interp_pnts_str   () const { return(interp_pnts_str);     }

inline const char *StatHdrColumns::get_line_type         () const { return(line_type);           }

inline SingleThresh StatHdrColumns::get_fcst_thresh    () const { return(fcst_thresh);     }
inline const char  *StatHdrColumns::get_fcst_thresh_str() const { return(fcst_thresh_str); }

inline SingleThresh StatHdrColumns::get_obs_thresh     () const { return(obs_thresh);      }
inline const char  *StatHdrColumns::get_obs_thresh_str () const { return(obs_thresh_str);  }

inline SingleThresh StatHdrColumns::get_cov_thresh     () const { return(cov_thresh);      }
inline const char  *StatHdrColumns::get_cov_thresh_str () const { return(cov_thresh_str);  }
inline double       StatHdrColumns::get_alpha          () const { return(alpha);           }

////////////////////////////////////////////////////////////////////////

#endif   /*  __STAT_HDR_COLUMNS_H__  */

////////////////////////////////////////////////////////////////////////
