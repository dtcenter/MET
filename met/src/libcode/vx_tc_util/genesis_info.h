// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2019
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
      ConcatString StormId;
      ConcatString Basin;
      ConcatString Cyclone;
      int          TechniqueNumber;
      ConcatString Technique;
      ConcatString Initials;

      // Genesis Timing
      unixtime GenTime;
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

      void initialize(const ATCFTrackLine &);

   public:

      GenesisInfo();
     ~GenesisInfo();
      GenesisInfo(const GenesisInfo &);
      GenesisInfo & operator=(const GenesisInfo &);

      void         clear();
      void         dump(ostream &, int = 0)  const;

      ConcatString serialize()               const;
      ConcatString serialize_r(int, int = 0) const;

         //
         //  set stuff
         //

      void set_storm_id();
      void set_storm_id(const ConcatString &);
      void set_basin(const ConcatString &);
      void set_cyclone(const ConcatString &);
      void set_storm_name(const ConcatString &);
      void set_technique_number(int);
      void set_technique(const ConcatString &);
      void set_initials(const ConcatString &);
      void set_init(const unixtime);

         //
         //  get stuff
         //

      bool  is_best_track()                   const;
      const ConcatString & storm_id()         const;
      const ConcatString & basin()            const;
      const ConcatString & cyclone()          const;
      const ConcatString & storm_name()       const;
      int                  technique_number() const;
      const ConcatString & technique()        const;
      const ConcatString & initials()         const;
      unixtime             init()             const;
      int                  init_hour()        const;
      unixtime             valid_min()        const;
      unixtime             valid_max()        const;
      int                  valid_dur()        const;
      unixtime             warm_core_min()    const;
      unixtime             warm_core_max()    const;
      int                  warm_core_dur()    const;
      int                  n_points()         const;

         //
         //  do stuff
         //

      bool add(const ATCFTrackLine &);

      bool is_match(const ATCFTrackLine &);

};

////////////////////////////////////////////////////////////////////////

inline bool GenesisInfo::is_best_track()                  const { return(IsBestTrack); }
inline void GenesisInfo::set_storm_id(const ConcatString &cs)   { StormId = cs;        }
inline void GenesisInfo::set_basin(const ConcatString &cs)      { Basin = cs;          }
inline void GenesisInfo::set_cyclone(const ConcatString &cs)    { Cyclone = cs;        }
inline void GenesisInfo::set_technique_number(int i)            { TechniqueNumber = i; }
inline void GenesisInfo::set_technique(const ConcatString &cs)  { Technique = cs;      }
inline void GenesisInfo::set_initials(const ConcatString &cs)   { Initials = cs;       }
inline void GenesisInfo::set_init(const unixtime u)             { InitTime = u;        }

inline const ConcatString & GenesisInfo::storm_id()         const { return(StormId);                 }
inline const ConcatString & GenesisInfo::basin()            const { return(Basin);                   }
inline const ConcatString & GenesisInfo::cyclone()          const { return(Cyclone);                 }
inline int                  GenesisInfo::technique_number() const { return(TechniqueNumber);         }
inline const ConcatString & GenesisInfo::technique()        const { return(Technique);               }
inline const ConcatString & GenesisInfo::initials()         const { return(Initials);                }
inline unixtime             GenesisInfo::init()             const { return(InitTime);                }
inline int                  GenesisInfo::init_hour()        const { return(InitTime % sec_per_hour); }
inline unixtime             GenesisInfo::valid_min()        const { return(MinValidTime);            }
inline unixtime             GenesisInfo::valid_max()        const { return(MaxValidTime);            }
inline int                  GenesisInfo::valid_dur()        const { return((MinValidTime == 0 ||
                                                                            MaxValidTime == 0 ?
                                                                            bad_data_int :
                                                                            MaxValidTime - MinValidTime)); }
inline unixtime             GenesisInfo::warm_core_min()    const { return(MinWarmCoreTime);         }
inline unixtime             GenesisInfo::warm_core_max()    const { return(MaxWarmCoreTime);         }
inline int                  GenesisInfo::warm_core_dur()    const { return((MinWarmCoreTime == 0 ||
                                                                            MaxWarmCoreTime == 0 ?
                                                                            bad_data_int :
                                                                            MaxWarmCoreTime - MinWarmCoreTime)); }
inline int                  GenesisInfo::n_points()         const { return(NPoints);                 }

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
      bool add(const TrackInfo &);

         //
         //  get stuff
         //

      const GenesisInfo & operator[](int) const;
      int n() const;

};

////////////////////////////////////////////////////////////////////////

inline int GenesisInfoArray::n() const { return(Genesis.size()); }

////////////////////////////////////////////////////////////////////////

#endif   /*  __VX_GENESIS_INFO_H__  */

////////////////////////////////////////////////////////////////////////
