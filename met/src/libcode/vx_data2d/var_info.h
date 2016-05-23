// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2015
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

#include "data_file_type.h"

///////////////////////////////////////////////////////////////////////////////

class VarInfo
{
   protected:

      ConcatString MagicStr;  // Requested magic string
      Dictionary   Dict;      // Requested fcst/obs dictionary of fields

      ConcatString ReqName;   // Requested parameter name
      ConcatString Name;      // Name of parameter
      ConcatString Units;     // Units for parameter
      LevelInfo    Level;     // Level information
      ConcatString LongName;  // Description string

      bool         PFlag;     // Flag for probability
      ConcatString PName;     // Name for probability
      ConcatString PUnits;    // Unit for probability
      SingleThresh PThreshLo; // Lower probability threshold
      SingleThresh PThreshHi; // Upper probability threshold

      bool         VFlag;     // Flag for vector winds

      unixtime     Init;      // Initialization time in unixtime
      unixtime     Valid;     // Valid time in unixtime
      int          Lead;      // Lead time in seconds

      void init_from_scratch();
      void assign(const VarInfo &);

   public:

      VarInfo();
      virtual ~VarInfo();
      VarInfo(const VarInfo &);
      VarInfo & operator=(const VarInfo &);

      void clear();

      virtual void dump(ostream &) const;
      virtual void add_grib_code (Dictionary &);

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

      bool         p_flag()         const;
      ConcatString p_name()         const;
      ConcatString p_units()        const;
      SingleThresh p_thresh_lo()    const;
      SingleThresh p_thresh_hi()    const;

      bool         v_flag()         const;

      unixtime     init()           const;
      unixtime     valid()          const;
      int          lead()           const;

         //
         // set stuff
         //

      virtual void set_magic(const ConcatString &);
      virtual void set_dict(Dictionary &);

      void set_req_name(const char *);
      void set_name(const char *);
      void set_units(const char *);
      void set_level_info(const LevelInfo &);
      void set_req_level_name(const char *);
      void set_level_name(const char *);
      void set_long_name(const char *);

      void set_p_flag(bool);
      void set_p_name(const char *);
      void set_p_units(const char *);
      void set_p_thresh_lo(const SingleThresh &);
      void set_p_thresh_hi(const SingleThresh &);

      void set_v_flag(bool);
      
      void set_init(unixtime);
      void set_valid(unixtime);
      void set_lead(int);

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
};

///////////////////////////////////////////////////////////////////////////////

inline ConcatString VarInfo::magic_str()      const { return(MagicStr);         }
inline ConcatString VarInfo::req_name()       const { return(ReqName);          }
inline ConcatString VarInfo::name()           const { return(Name);             }
inline ConcatString VarInfo::units()          const { return(Units);            }
inline LevelInfo    VarInfo::level()          const { return(Level);            }
inline ConcatString VarInfo::req_level_name() const { return(Level.req_name()); }
inline ConcatString VarInfo::level_name()     const { return(Level.name());     }
inline ConcatString VarInfo::long_name()      const { return(LongName);         }

inline bool         VarInfo::p_flag()         const { return(PFlag);            }
inline ConcatString VarInfo::p_name()         const { return(PName);            }
inline ConcatString VarInfo::p_units()        const { return(PUnits);           }
inline SingleThresh VarInfo::p_thresh_lo()    const { return(PThreshLo);        }
inline SingleThresh VarInfo::p_thresh_hi()    const { return(PThreshHi);        }

inline bool         VarInfo::v_flag()         const { return(VFlag);            }

inline unixtime     VarInfo::init()           const { return(Init);             }
inline unixtime     VarInfo::valid()          const { return(Valid);            }
inline int          VarInfo::lead()           const { return(Lead);             }

///////////////////////////////////////////////////////////////////////////////

#endif  // __VAR_INFO_H__

///////////////////////////////////////////////////////////////////////////////
