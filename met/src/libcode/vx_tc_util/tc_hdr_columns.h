// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2020
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*

////////////////////////////////////////////////////////////////////////

#ifndef  __TC_HDR_COLUMNS_H__
#define  __TC_HDR_COLUMNS_H__

////////////////////////////////////////////////////////////////////////

#include "vx_util.h"
#include "vx_cal.h"
#include "vx_math.h"

////////////////////////////////////////////////////////////////////////

//
// Class to store data that's written to the output header columns
//

class TcHdrColumns {

   private:

      void init_from_scratch();

      // Model names
      ConcatString ADeckModel, BDeckModel;

      // User-defined description
      ConcatString Desc;

      // Basin and cyclone names
      ConcatString StormId, Basin, Cyclone, StormName;

      // Timing info
      unixtime     InitTime, ValidTime;
      int          LeadTime;

      // Masking regions
      ConcatString InitMask, ValidMask;

      // Line type
      ConcatString LineType;

   public:

      TcHdrColumns();
     ~TcHdrColumns();

      void clear();

      // Set functions
      void set_adeck_model (const ConcatString &);
      void set_bdeck_model (const ConcatString &);
      void set_desc        (const ConcatString &);
      void set_storm_id    (const ConcatString &);
      void set_basin       (const ConcatString &);
      void set_cyclone     (const ConcatString &);
      void set_storm_name  (const ConcatString &);
      void set_init        (const unixtime);
      void set_lead        (const int);
      void set_valid       (const unixtime);
      void set_init_mask   (const ConcatString &);
      void set_valid_mask  (const ConcatString &);
      void set_line_type   (const ConcatString &);

      // Get functions
      ConcatString adeck_model () const;
      ConcatString bdeck_model () const;
      ConcatString desc        () const;
      ConcatString storm_id    () const;
      ConcatString basin       () const;
      ConcatString cyclone     () const;
      ConcatString storm_name  () const;
      int          lead        () const;
      unixtime     init        () const;
      int          init_hour   () const;
      unixtime     valid       () const;
      int          valid_hour  () const;
      ConcatString init_mask   () const;
      ConcatString valid_mask  () const;

      ConcatString line_type   () const;
};

////////////////////////////////////////////////////////////////////////

inline void TcHdrColumns::set_adeck_model (const ConcatString &s) { ADeckModel = s; }
inline void TcHdrColumns::set_bdeck_model (const ConcatString &s) { BDeckModel = s; }
inline void TcHdrColumns::set_desc        (const ConcatString &s) { Desc = s;       }
inline void TcHdrColumns::set_storm_id    (const ConcatString &s) { StormId = s;    }
inline void TcHdrColumns::set_basin       (const ConcatString &s) { Basin = s;      }
inline void TcHdrColumns::set_cyclone     (const ConcatString &s) { Cyclone = s;    }
inline void TcHdrColumns::set_storm_name  (const ConcatString &s) { StormName = s;  }
inline void TcHdrColumns::set_lead        (const int s)           { LeadTime = s;   }
inline void TcHdrColumns::set_init        (const unixtime u)      { InitTime = u;   }
inline void TcHdrColumns::set_valid       (const unixtime u)      { ValidTime = u;  }
inline void TcHdrColumns::set_init_mask   (const ConcatString &s) { InitMask = s;   }
inline void TcHdrColumns::set_valid_mask  (const ConcatString &s) { ValidMask = s;  }
inline void TcHdrColumns::set_line_type   (const ConcatString &s) { LineType = s;   }

inline ConcatString TcHdrColumns::adeck_model () const { return(ADeckModel);                    }
inline ConcatString TcHdrColumns::bdeck_model () const { return(BDeckModel);                    }
inline ConcatString TcHdrColumns::desc        () const { return(Desc);                          }
inline ConcatString TcHdrColumns::storm_id    () const { return(StormId);                       }
inline ConcatString TcHdrColumns::basin       () const { return(Basin);                         }
inline ConcatString TcHdrColumns::cyclone     () const { return(Cyclone);                       }
inline ConcatString TcHdrColumns::storm_name  () const { return(StormName);                     }
inline int          TcHdrColumns::lead        () const { return(LeadTime);                      }
inline unixtime     TcHdrColumns::init        () const { return(InitTime);                      }
inline int          TcHdrColumns::init_hour   () const { return(unix_to_sec_of_day(InitTime));  }
inline unixtime     TcHdrColumns::valid       () const { return(ValidTime);                     }
inline int          TcHdrColumns::valid_hour  () const { return(unix_to_sec_of_day(ValidTime)); }
inline ConcatString TcHdrColumns::init_mask   () const { return(InitMask);                      }
inline ConcatString TcHdrColumns::valid_mask  () const { return(ValidMask);                     }
inline ConcatString TcHdrColumns::line_type   () const { return(LineType);                      }

////////////////////////////////////////////////////////////////////////

#endif   /*  __TC_HDR_COLUMNS_H__  */

////////////////////////////////////////////////////////////////////////
