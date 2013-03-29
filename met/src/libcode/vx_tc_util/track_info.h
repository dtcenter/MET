// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2012
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*

////////////////////////////////////////////////////////////////////////

#ifndef  __VX_TRACK_INFO_H__
#define  __VX_TRACK_INFO_H__

////////////////////////////////////////////////////////////////////////

#include <iostream>

#include "vx_cal.h"
#include "vx_math.h"
#include "vx_util.h"

#include "atcf_line.h"
#include "track_point.h"

////////////////////////////////////////////////////////////////////////

static const int TrackInfoAllocInc      = 100;
static const int TrackInfoArrayAllocInc = 10;
static const int MaxBestTrackTimeInc    = 24 * sec_per_hour;

////////////////////////////////////////////////////////////////////////
//
// TrackInfo class stores multiple ATCF lines that are grouped together
// into a tropical cyclone track.
//
////////////////////////////////////////////////////////////////////////

class TrackInfo {

   private:

      void init_from_scratch();
      void assign(const TrackInfo &);
      void extend(int);
      
      bool         IsSet;

      // Storm and model identification
      ConcatString StormId;
      ConcatString Basin;
      ConcatString Cyclone;
      ConcatString StormName;
      int          TechniqueNumber;
      ConcatString Technique;

      // Forecaster Initials
      ConcatString Initials;

      // Timing information
      unixtime     InitTime;
      unixtime     MinValidTime;
      unixtime     MaxValidTime;

      // TrackPoints                    
      TrackPoint  *Point;
      int          NPoints;
      int          NAlloc;

   public:

      TrackInfo();
     ~TrackInfo();
      TrackInfo(const TrackInfo &);
      TrackInfo & operator=(const TrackInfo &);

      void clear();
      void clear_points();

      void         dump(ostream &, int = 0)  const;
      ConcatString serialize()               const;
      ConcatString serialize_r(int, int = 0) const;

         //
         //  set stuff
         //

      void initialize(const ATCFLine &);

      void set_storm_id();
      void set_basin(const char *);
      void set_cyclone(const char *);
      void set_storm_name(const char *);
      void set_technique_number(int);
      void set_technique(const char *);
      void set_initials(const char *);
      void set_init(const unixtime);
      void set_valid_min(const unixtime);
      void set_valid_max(const unixtime);
      void set_point(int, const TrackPoint &);
      
         //
         //  get stuff
         //

      int lead_index(int)       const;
      int valid_index(unixtime) const;

      const TrackPoint &   operator[](int)    const;
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
      int                  valid_inc()        const;
      int                  n_points()         const;

         //
         //  do stuff
         //

      void add(const TrackPoint &);
      bool add(const ATCFLine &);
      void add_watch_warn(const ConcatString &, WatchWarnType, unixtime);
      
      bool has(const ATCFLine &) const;
      
      bool is_match(const ATCFLine &) const;
      bool is_match(const TrackInfo &) const;
      
      bool is_interp() const;

};

////////////////////////////////////////////////////////////////////////

inline void TrackInfo::set_basin(const char *s)        { Basin = s;           };
inline void TrackInfo::set_cyclone(const char *s)      { Cyclone = s;         };
inline void TrackInfo::set_storm_name(const char *s)   { StormName = s;       };
inline void TrackInfo::set_technique_number(int i)     { TechniqueNumber = i; };
inline void TrackInfo::set_technique(const char *s)    { Technique = s;       };
inline void TrackInfo::set_initials(const char *s)     { Initials = s;        };
inline void TrackInfo::set_init(const unixtime u)      { InitTime = u;        };
inline void TrackInfo::set_valid_min(const unixtime u) { MinValidTime = u;    };
inline void TrackInfo::set_valid_max(const unixtime u) { MaxValidTime = u;    };

inline const ConcatString & TrackInfo::storm_id()         const { return(StormId);                 }
inline const ConcatString & TrackInfo::basin()            const { return(Basin);                   }
inline const ConcatString & TrackInfo::cyclone()          const { return(Cyclone);                 }
inline const ConcatString & TrackInfo::storm_name()       const { return(StormName);               }
inline int                  TrackInfo::technique_number() const { return(TechniqueNumber);         }
inline const ConcatString & TrackInfo::technique()        const { return(Technique);               }
inline const ConcatString & TrackInfo::initials()         const { return(Initials);                }
inline unixtime             TrackInfo::init()             const { return(InitTime);                }
inline int                  TrackInfo::init_hour()        const { return(InitTime % sec_per_hour); }
inline unixtime             TrackInfo::valid_min()        const { return(MinValidTime);            }
inline unixtime             TrackInfo::valid_max()        const { return(MaxValidTime);            }
inline int                  TrackInfo::n_points()         const { return(NPoints);                 }

////////////////////////////////////////////////////////////////////////
//
// TrackInfoArray class stores an array of TrackInfo objects.
//
////////////////////////////////////////////////////////////////////////

class TrackInfoArray {

   friend TrackInfo consensus(const TrackInfoArray &, const ConcatString &, int);

   private:

      void init_from_scratch();
      void assign(const TrackInfoArray &);
      void extend(int);

      TrackInfo     *Track;
      int            NTracks;
      int            NAlloc;

   public:

      TrackInfoArray();
     ~TrackInfoArray();
      TrackInfoArray(const TrackInfoArray &);
      TrackInfoArray & operator=(const TrackInfoArray &);

      void clear();

      void         dump(ostream &, int = 0) const;
      ConcatString serialize()              const;
      ConcatString serialize_r(int = 0)     const;

         //
         //  set stuff
         //

         //
         //  get stuff
         //

      const TrackInfo & operator[](int) const;
      int n_tracks() const;

         //
         //  do stuff
         //

      void add(const TrackInfo &);
      void set(int, const TrackInfo &);
      bool add(const ATCFLine &, bool = true);
      bool has(const ATCFLine &) const;

};

////////////////////////////////////////////////////////////////////////

inline int TrackInfoArray::n_tracks() const { return(NTracks); }

////////////////////////////////////////////////////////////////////////

extern TrackInfo consensus(const TrackInfoArray &, const ConcatString &, int);
extern bool has_storm_id(const StringArray &, const ConcatString &basin,
                         const ConcatString &cyclone, unixtime init);

////////////////////////////////////////////////////////////////////////

#endif   /*  __VX_TRACK_INFO_H__  */

////////////////////////////////////////////////////////////////////////
