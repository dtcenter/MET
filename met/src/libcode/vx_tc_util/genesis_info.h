// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2020
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*

////////////////////////////////////////////////////////////////////////

#ifndef  __VX_GENESIS_INFO_H__
#define  __VX_GENESIS_INFO_H__

////////////////////////////////////////////////////////////////////////

#include <iostream>
#include <vector>

#include "vx_cal.h"
#include "vx_math.h"
#include "vx_util.h"

#include "atcf_track_line.h"
#include "track_info.h"

////////////////////////////////////////////////////////////////////////
//
// GenesisInfo class stores information about genesis events.
//
////////////////////////////////////////////////////////////////////////

class GenesisInfo {

   private:

      void assign(const GenesisInfo &);

      // Storm and model identification

      bool         IsSet;
      bool         IsBestTrack;
      bool         IsOperTrack;
      bool         IsAnlyTrack;

      ConcatString StormId;
      ConcatString Basin;
      ConcatString Cyclone;
      ConcatString StormName;
      int          TechniqueNumber;
      ConcatString Technique;
      ConcatString Initials;

      // Genesis Timing
      unixtime GenesisTime;
      unixtime InitTime;
      int      LeadTime;

      // Genesis Location
      double   Lat;
      double   Lon;
      double   DLand;

      // Track Summary
      int      NPoints;
      unixtime MinValidTime;
      unixtime MaxValidTime;
      unixtime MinWarmCoreTime;
      unixtime MaxWarmCoreTime;

   public:

      GenesisInfo();
     ~GenesisInfo();
      GenesisInfo(const GenesisInfo &);
      GenesisInfo & operator=(const GenesisInfo &);
      bool          operator==(const GenesisInfo &) const;
      bool          is_storm(const GenesisInfo &) const;

      void         clear();
      void         dump(ostream &, int = 0)  const;

      ConcatString serialize()               const;
      ConcatString serialize_r(int, int = 0) const;

         //
         //  set stuff
         //

      void set_storm_id();
      void set_dland(double);
      bool set(const TrackInfo &, int);

         //
         //  get stuff
         //

      bool is_best_track() const;
      bool is_oper_track() const;
      bool is_anly_track() const;

      const ConcatString & storm_id()         const;
      const ConcatString & basin()            const;
      const ConcatString & cyclone()          const;
      const ConcatString & storm_name()       const;
      int                  technique_number() const;
      const ConcatString & technique()        const;
      const ConcatString & initials()         const;
      double               lat()              const;
      double               lon()              const;
      double               dland()            const;
      unixtime             genesis_time()     const;
      unixtime             init()             const;
      int                  init_hour()        const;
      int                  lead_time()        const;
      unixtime             valid_min()        const;
      unixtime             valid_max()        const;
      int                  valid_dur()        const;
      int                  n_points()         const;
      unixtime             warm_core_min()    const;
      unixtime             warm_core_max()    const;
      int                  warm_core_dur()    const;

         //
         //  do stuff
         //

      bool is_match(const GenesisInfo &,
                    const double, const int, const int,
                    double &, int &) const;
};

////////////////////////////////////////////////////////////////////////

inline bool                 GenesisInfo::is_best_track()    const { return(IsBestTrack);                  }
inline bool                 GenesisInfo::is_oper_track()    const { return(IsOperTrack);                  }
inline bool                 GenesisInfo::is_anly_track()    const { return(IsAnlyTrack);                  }
inline const ConcatString & GenesisInfo::storm_id()         const { return(StormId);                      }
inline const ConcatString & GenesisInfo::basin()            const { return(Basin);                        }
inline const ConcatString & GenesisInfo::cyclone()          const { return(Cyclone);                      }
inline const ConcatString & GenesisInfo::storm_name()       const { return(StormName);                    }
inline int                  GenesisInfo::technique_number() const { return(TechniqueNumber);              }
inline const ConcatString & GenesisInfo::technique()        const { return(Technique);                    }
inline const ConcatString & GenesisInfo::initials()         const { return(Initials);                     }
inline double               GenesisInfo::lat()              const { return(Lat);                          }
inline double               GenesisInfo::lon()              const { return(Lon);                          }
inline double               GenesisInfo::dland()            const { return(DLand);                        }
inline unixtime             GenesisInfo::genesis_time()     const { return(GenesisTime);                  }
inline unixtime             GenesisInfo::init()             const { return(InitTime);                     }
inline int                  GenesisInfo::init_hour()        const { return(unix_to_sec_of_day(InitTime)); }
inline int                  GenesisInfo::lead_time()        const { return(LeadTime);                     }
inline int                  GenesisInfo::n_points()         const { return(NPoints);                      }
inline unixtime             GenesisInfo::valid_min()        const { return(MinValidTime);                 }
inline unixtime             GenesisInfo::valid_max()        const { return(MaxValidTime);                 }
inline int                  GenesisInfo::valid_dur()        const { return((MinValidTime == 0 || MaxValidTime == 0 ?
                                                                            bad_data_int : MaxValidTime - MinValidTime)); }
inline unixtime             GenesisInfo::warm_core_min()    const { return(MinWarmCoreTime);              }
inline unixtime             GenesisInfo::warm_core_max()    const { return(MaxWarmCoreTime);              }
inline int                  GenesisInfo::warm_core_dur()    const { return((MinWarmCoreTime == 0 || MaxWarmCoreTime == 0 ?
                                                                            bad_data_int : MaxWarmCoreTime - MinWarmCoreTime)); }

////////////////////////////////////////////////////////////////////////
//
// GenesisInfoArray class stores an array of GenesisInfo objects.
//
////////////////////////////////////////////////////////////////////////

class GenesisInfoArray {

   private:

      void init_from_scratch();
      void assign(const GenesisInfoArray &);

      vector<GenesisInfo> Genesis;

   public:

      GenesisInfoArray();
     ~GenesisInfoArray();
      GenesisInfoArray(const GenesisInfoArray &);
      GenesisInfoArray & operator=(const GenesisInfoArray &);

      void         clear();
      void         dump(ostream &, int = 0) const;

      ConcatString serialize()              const;
      ConcatString serialize_r(int = 0)     const;

         //
         //  set stuff
         //

      void add(const GenesisInfo &);
      bool add(const TrackInfo &, int);
      bool has(const GenesisInfo &);
      bool has_storm(const GenesisInfo &);
      void set_dland(int, double);

         //
         //  get stuff
         //

      const GenesisInfo & operator[](int) const;
      int n() const;
      int n_technique() const;

         //
         //  do stuff
         //

      int find_match(const GenesisInfo &,
                     const double, const int, const int) const;

};

////////////////////////////////////////////////////////////////////////

inline int GenesisInfoArray::n() const { return(Genesis.size()); }

////////////////////////////////////////////////////////////////////////

#endif   /*  __VX_GENESIS_INFO_H__  */

////////////////////////////////////////////////////////////////////////
