// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2019
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*

////////////////////////////////////////////////////////////////////////

#ifndef  __STAT_HDR_COLUMNS_H__
#define  __STAT_HDR_COLUMNS_H__

////////////////////////////////////////////////////////////////////////

#include "vx_config.h"
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

      // Description
      ConcatString desc;

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

      // Fcst variable, units, and level
      ConcatString fcst_var;
      ConcatString fcst_units;
      ConcatString fcst_lev;

      // Obs variable, units, and level
      ConcatString obs_var;
      ConcatString obs_units;
      ConcatString obs_lev;

      // Obs type
      ConcatString obtype;

      // Verification region
      ConcatString mask;

      // Interpolation method and size
      ConcatString interp_mthd;
      int          interp_pnts;
      ConcatString interp_pnts_str;

      // Line type
      ConcatString line_type;

      // Fcst and obs thresholds
      ThreshArray fcst_thresh;
      ThreshArray obs_thresh;
      SetLogic    thresh_logic;

      // Coverage field threshold
      ThreshArray cov_thresh;

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

   public:

      StatHdrColumns();
     ~StatHdrColumns();

      void clear();

      // Set functions
      void set_model         (const char *);
      void set_desc          (const char *);

      void set_fcst_lead_sec (const int);
      void set_fcst_valid_beg(const unixtime);
      void set_fcst_valid_end(const unixtime);

      void set_obs_lead_sec  (const int);
      void set_obs_valid_beg (const unixtime);
      void set_obs_valid_end (const unixtime);

      void set_fcst_var      (const ConcatString);
      void set_fcst_units    (const ConcatString);
      void set_fcst_lev      (const char *);

      void set_obs_var       (const ConcatString);
      void set_obs_units     (const ConcatString);
      void set_obs_lev       (const char *);

      void set_obtype        (const char *);
      void set_mask          (const char *);

      void set_interp_mthd   (ConcatString s,
                              GridTemplateFactory::GridTemplates shape = GridTemplateFactory::GridTemplate_None);
      void set_interp_mthd   (const InterpMthd m,
                              GridTemplateFactory::GridTemplates shape = GridTemplateFactory::GridTemplate_None);
      void set_interp_pnts   (const int);
      void set_interp_wdth   (const int);

      void set_line_type     (const char *);

      void set_fcst_thresh   (const SingleThresh);
      void set_fcst_thresh   (const ThreshArray);
      void set_obs_thresh    (const SingleThresh);
      void set_obs_thresh    (const ThreshArray);
      void set_thresh_logic  (const SetLogic);

      void set_cov_thresh    (const SingleThresh);
      void set_cov_thresh    (const ThreshArray);
      void set_alpha         (const double);

      // Get functions
      ConcatString get_model             () const;
      ConcatString get_desc              () const;

      int          get_fcst_lead_sec     () const;
      ConcatString get_fcst_lead_str     () const;

      unixtime     get_fcst_valid_beg    () const;
      ConcatString get_fcst_valid_beg_str() const;

      unixtime     get_fcst_valid_end    () const;
      ConcatString get_fcst_valid_end_str() const;

      int          get_obs_lead_sec      () const;
      ConcatString get_obs_lead_str      () const;

      unixtime     get_obs_valid_beg     () const;
      ConcatString get_obs_valid_beg_str () const;

      unixtime     get_obs_valid_end     () const;
      ConcatString get_obs_valid_end_str () const;

      ConcatString get_fcst_var          () const;
      ConcatString get_fcst_units        () const;
      ConcatString get_fcst_lev          () const;

      ConcatString get_obs_var           () const;
      ConcatString get_obs_units         () const;
      ConcatString get_obs_lev           () const;

      ConcatString get_obtype            () const;
      ConcatString get_mask              () const;

      ConcatString get_interp_mthd       () const;
      int          get_interp_pnts       () const;
      ConcatString get_interp_pnts_str   () const;

      ConcatString get_line_type         () const;

      ThreshArray  get_fcst_thresh       () const;
      ConcatString get_fcst_thresh_str   () const;

      ThreshArray  get_obs_thresh        () const;
      ConcatString get_obs_thresh_str    () const;

      SetLogic     get_thresh_logic      () const;

      ThreshArray  get_cov_thresh        () const;
      ConcatString get_cov_thresh_str    () const;

      double       get_alpha             () const;
};

////////////////////////////////////////////////////////////////////////

inline ConcatString StatHdrColumns::get_model             () const { return(model.contents(na_str));              }
inline ConcatString StatHdrColumns::get_desc              () const { return(desc.contents(na_str));               }

inline int          StatHdrColumns::get_fcst_lead_sec     () const { return(fcst_lead_sec);                       }
inline ConcatString StatHdrColumns::get_fcst_lead_str     () const { return(fcst_lead_str.contents(na_str));      }

inline unixtime     StatHdrColumns::get_fcst_valid_beg    () const { return(fcst_valid_beg);                      }
inline ConcatString StatHdrColumns::get_fcst_valid_beg_str() const { return(fcst_valid_beg_str.contents(na_str)); }

inline unixtime     StatHdrColumns::get_fcst_valid_end    () const { return(fcst_valid_end);                      }
inline ConcatString StatHdrColumns::get_fcst_valid_end_str() const { return(fcst_valid_end_str.contents(na_str)); }

inline int          StatHdrColumns::get_obs_lead_sec      () const { return(obs_lead_sec);                        }
inline ConcatString StatHdrColumns::get_obs_lead_str      () const { return(obs_lead_str.contents(na_str));       }

inline unixtime     StatHdrColumns::get_obs_valid_beg     () const { return(obs_valid_beg);                       }
inline ConcatString StatHdrColumns::get_obs_valid_beg_str () const { return(obs_valid_beg_str.contents(na_str));  }

inline unixtime     StatHdrColumns::get_obs_valid_end     () const { return(obs_valid_end);                       }
inline ConcatString StatHdrColumns::get_obs_valid_end_str () const { return(obs_valid_end_str.contents(na_str));  }

inline ConcatString StatHdrColumns::get_fcst_var          () const { return(fcst_var.contents(na_str));           }
inline ConcatString StatHdrColumns::get_fcst_units        () const { return(fcst_units.contents(na_str));           }
inline ConcatString StatHdrColumns::get_fcst_lev          () const { return(fcst_lev.contents(na_str));           }

inline ConcatString StatHdrColumns::get_obs_var           () const { return(obs_var.contents(na_str));            }
inline ConcatString StatHdrColumns::get_obs_units         () const { return(obs_units.contents(na_str));            }
inline ConcatString StatHdrColumns::get_obs_lev           () const { return(obs_lev.contents(na_str));            }

inline ConcatString StatHdrColumns::get_obtype            () const { return(obtype.contents(na_str));             }
inline ConcatString StatHdrColumns::get_mask              () const { return(mask.contents(na_str));               }

inline ConcatString StatHdrColumns::get_interp_mthd       () const { return(interp_mthd.contents(na_str));        }
inline int          StatHdrColumns::get_interp_pnts       () const { return(interp_pnts);                         }
inline ConcatString StatHdrColumns::get_interp_pnts_str   () const { return(interp_pnts_str.contents(na_str));    }

inline ConcatString StatHdrColumns::get_line_type         () const { return(line_type.contents(na_str));          }

inline ThreshArray  StatHdrColumns::get_fcst_thresh       () const { return(fcst_thresh);                         }

inline ThreshArray  StatHdrColumns::get_obs_thresh        () const { return(obs_thresh);                          }
inline ConcatString StatHdrColumns::get_obs_thresh_str    () const { return(obs_thresh.get_str());                }

inline SetLogic     StatHdrColumns::get_thresh_logic      () const { return(thresh_logic);                        }

inline ThreshArray  StatHdrColumns::get_cov_thresh        () const { return(cov_thresh);                          }
inline ConcatString StatHdrColumns::get_cov_thresh_str    () const { return(prob_thresh_to_string(cov_thresh));   }

inline double       StatHdrColumns::get_alpha             () const { return(alpha);                               }

////////////////////////////////////////////////////////////////////////

#endif   /*  __STAT_HDR_COLUMNS_H__  */

////////////////////////////////////////////////////////////////////////
