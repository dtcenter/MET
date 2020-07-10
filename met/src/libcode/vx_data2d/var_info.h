// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2020
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*

///////////////////////////////////////////////////////////////////////////////

#ifndef __VAR_INFO_H__
#define __VAR_INFO_H__

///////////////////////////////////////////////////////////////////////////////

#include "concat_string.h"
#include "level_info.h"
#include "threshold.h"
#include "vx_cal.h"
#include "vx_config.h"
#include "vx_grid.h"

#include "data_file_type.h"

///////////////////////////////////////////////////////////////////////////////

class VarInfo
{
   protected:

      ConcatString  MagicStr;  // Requested magic string
      Dictionary    Dict;      // Requested fcst/obs dictionary of fields

      ConcatString  ReqName;   // Requested parameter name
      ConcatString  Name;      // Name of parameter
      ConcatString  Units;     // Units for parameter
      LevelInfo     Level;     // Level information
      ConcatString  LongName;  // Description string
      ConcatString  Ensemble;  // Ensemble information

      bool          PFlag;     // Flag for probability
      ConcatString  PName;     // Name for probability
      ConcatString  PUnits;    // Unit for probability
      SingleThresh  PThreshLo; // Lower probability threshold
      SingleThresh  PThreshHi; // Upper probability threshold
      bool          PAsScalar; // Flag to process probabilities as scalars

      int           UVIndex;   // Index for u/v vector wind

      unixtime      Init;      // Initialization time in unixtime
      unixtime      Valid;     // Valid time in unixtime
      int           Lead;      // Lead time in seconds

      ThreshArray   CensorThresh; // Censoring thesholds
      NumArray      CensorVal;    // and replacement values

      int           nBins;     // Number of pdf bins
      NumArray      Range;     // Range of pdf bins

      RegridInfo    Regrid;    // Regridding logic

      // Options to override metadata
      ConcatString  SetAttrName;
      ConcatString  SetAttrUnits;
      ConcatString  SetAttrLevel;
      ConcatString  SetAttrLongName;

      Grid          SetAttrGrid;

      unixtime      SetAttrInit;
      unixtime      SetAttrValid;
      int           SetAttrLead;
      int           SetAttrAccum;

      int           SetAttrIsPrecipitation;
      int           SetAttrIsSpecificHumidity;
      int           SetAttrIsUWind;
      int           SetAttrIsVWind;
      int           SetAttrIsGridRelative;
      int           SetAttrIsWindSpeed;
      int           SetAttrIsWindDirection;
      int           SetAttrIsProb;

      void init_from_scratch();
      void assign(const VarInfo &);

   public:

      VarInfo();
      virtual ~VarInfo();
      VarInfo(const VarInfo &);
      VarInfo & operator=(const VarInfo &);

      // Conversion function
      UserFunc_1Arg ConvertFx;

      void clear();

      virtual void dump(ostream &) const;

         //
         // get stuff
         //

      virtual GrdFileType  file_type() const = 0;

      ConcatString magic_str()      const;
      ConcatString req_name()       const;
      ConcatString name()           const;
      ConcatString units()          const;
      LevelInfo    level()          const;
      ConcatString req_level_name() const;
      ConcatString level_name()     const;
      ConcatString long_name()      const;
      ConcatString ens()            const;

      bool         p_flag()         const;
      ConcatString p_name()         const;
      ConcatString p_units()        const;
      SingleThresh p_thresh_lo()    const;
      SingleThresh p_thresh_hi()    const;
      bool         p_as_scalar()    const;

      int          uv_index()       const;

      unixtime     init()           const;
      unixtime     valid()          const;
      int          lead()           const;

      ThreshArray  censor_thresh()  const;
      NumArray     censor_val()     const;

      int          n_bins()         const;
      NumArray     range()          const;

      RegridInfo   regrid()         const;

      ConcatString name_attr()      const;
      ConcatString units_attr()     const;
      ConcatString level_attr()     const;
      ConcatString long_name_attr() const;
      ConcatString ensemble_attr()  const;

      Grid         grid_attr()      const;
      unixtime     init_attr()      const;
      unixtime     valid_attr()     const;
      int          lead_attr()      const;
      int          accum_attr()     const;

         //
         // set stuff
         //

      virtual void set_magic(const ConcatString &, const ConcatString &);
      virtual void set_dict(Dictionary &);
      virtual void add_grib_code(Dictionary &);

      void set_req_name(const char *);
      void set_name(const char *);
      void set_name(const string);
      void set_units(const char *);
      void set_level_info(const LevelInfo &);
      void set_req_level_name(const char *);
      void set_level_name(const char *);
      void set_long_name(const char *);
      void set_ens(const char *);

      void set_p_flag(bool);
      void set_p_name(const char *);
      void set_p_units(const char *);
      void set_p_thresh_lo(const SingleThresh &);
      void set_p_thresh_hi(const SingleThresh &);
      void set_p_as_scalar(bool);

      void set_uv_index(int);

      void set_init(unixtime);
      void set_valid(unixtime);
      void set_lead(int);

      void set_censor_thresh(const ThreshArray &);
      void set_censor_val(const NumArray &);

      void set_n_bins(const int &);
      void set_range(const NumArray &);

      void set_regrid(const RegridInfo &);

      void set_level_info_grib(Dictionary & dict);
      void set_prob_info_grib(ConcatString prob_name,
                              double thresh_lo, double thresh_hi);

         //
         // do stuff
         //

      virtual bool is_precipitation()     const = 0;
      virtual bool is_specific_humidity() const = 0;
      virtual bool is_u_wind()            const = 0;
      virtual bool is_v_wind()            const = 0;
      virtual bool is_wind_speed()        const = 0;
      virtual bool is_wind_direction()    const = 0;
              bool is_prob();
};

///////////////////////////////////////////////////////////////////////////////

inline void VarInfo::add_grib_code(Dictionary &d)   { return;                   }

inline ConcatString VarInfo::magic_str()      const { return(MagicStr);         }
inline ConcatString VarInfo::req_name()       const { return(ReqName);          }
inline ConcatString VarInfo::name()           const { return(Name);             }
inline ConcatString VarInfo::units()          const { return(Units);            }
inline LevelInfo    VarInfo::level()          const { return(Level);            }
inline ConcatString VarInfo::req_level_name() const { return(Level.req_name()); }
inline ConcatString VarInfo::level_name()     const { return(Level.name());     }
inline ConcatString VarInfo::long_name()      const { return(LongName);         }
inline ConcatString VarInfo::ens()            const { return(Ensemble);         }

inline bool         VarInfo::p_flag()         const { return(PFlag);            }
inline ConcatString VarInfo::p_name()         const { return(PName);            }
inline ConcatString VarInfo::p_units()        const { return(PUnits);           }
inline SingleThresh VarInfo::p_thresh_lo()    const { return(PThreshLo);        }
inline SingleThresh VarInfo::p_thresh_hi()    const { return(PThreshHi);        }
inline bool         VarInfo::p_as_scalar()    const { return(PAsScalar);        }

inline int          VarInfo::uv_index()       const { return(UVIndex);          }

inline unixtime     VarInfo::init()           const { return(Init);             }
inline unixtime     VarInfo::valid()          const { return(Valid);            }
inline int          VarInfo::lead()           const { return(Lead);             }

inline ThreshArray  VarInfo::censor_thresh()  const { return(CensorThresh);     }
inline NumArray     VarInfo::censor_val()     const { return(CensorVal);        }

inline int          VarInfo::n_bins()         const { return(nBins);            }
inline NumArray     VarInfo::range()          const { return(Range);            }

inline RegridInfo   VarInfo::regrid()         const { return(Regrid);           }

inline ConcatString VarInfo::name_attr()      const { return(SetAttrName.empty()     ? name()       : SetAttrName);     }
inline ConcatString VarInfo::units_attr()     const { return(SetAttrUnits.empty()    ? units()      : SetAttrUnits);    }
inline ConcatString VarInfo::level_attr()     const { return(SetAttrLevel.empty()    ? level_name() : SetAttrLevel);    }
inline ConcatString VarInfo::long_name_attr() const { return(SetAttrLongName.empty() ? long_name()  : SetAttrLongName); }

inline Grid         VarInfo::grid_attr()      const { return(SetAttrGrid);     }

inline unixtime     VarInfo::init_attr()      const { return(SetAttrInit);     }
inline unixtime     VarInfo::valid_attr()     const { return(SetAttrValid);    }
inline int          VarInfo::lead_attr()      const { return(SetAttrLead);     }
inline int          VarInfo::accum_attr()     const { return(SetAttrAccum);    }

///////////////////////////////////////////////////////////////////////////////

#endif  // __VAR_INFO_H__

///////////////////////////////////////////////////////////////////////////////
