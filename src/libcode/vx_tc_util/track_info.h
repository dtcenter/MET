// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2022
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
#include "nav.h"

#include "atcf_track_line.h"
#include "track_point.h"
#include "diag_file.h"

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

   protected:

      void init_from_scratch();
      void assign(const TrackInfo &);
      void extend(int, bool exact = true);

      bool         IsSet;
      bool         IsBestTrack;
      bool         IsOperTrack;

      bool         CheckAnly;
      bool         IsAnlyTrack;

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
      unixtime     MinWarmCore;
      unixtime     MaxWarmCore;

      // Diagnostic names
      StringArray DiagName;

      // TrackPoints
      TrackPoint  *Point;
      int          NPoints;
      int          NAlloc;

      // Input ATCF Track Lines
      StringArray  TrackLines;

   public:

      TrackInfo();
     ~TrackInfo();
      TrackInfo(const TrackInfo &);
      TrackInfo & operator=(const TrackInfo &);

      void clear();
      void clear_points();

      void         dump(std::ostream &, int = 0)  const;
      ConcatString serialize()               const;
      ConcatString serialize_r(int, int = 0) const;

         //
         //  set stuff
         //

      void initialize(const ATCFTrackLine &, bool check_anly);

      void set_storm_id();
      void set_storm_id(const char *);
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
      void set_diag_name(const StringArray &);

         //
         //  get stuff
         //

      bool is_best_track() const;
      bool is_oper_track() const;
      bool is_anly_track() const;

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
      int                  duration()         const;
      unixtime             warm_core_min()    const;
      unixtime             warm_core_max()    const;
      int                  warm_core_dur()    const;
      int                  valid_inc()        const;
      int                  n_points()         const;
      int                  n_diag()           const;
      const StringArray &  diag_name()        const;
      const char *         diag_name(int)     const;

      StringArray          track_lines()      const;

         //
         //  do stuff
         //

      void add(const TrackPoint &);
      bool add(const ATCFTrackLine &, bool check_dup = false, bool check_anly = false);
      void add_watch_warn(const ConcatString &, WatchWarnType, unixtime);
      bool add_diag_data(DiagFile &, const StringArray &);
      void add_diag_value(int, double);

      bool has(const ATCFTrackLine &) const;

      bool is_match(const ATCFTrackLine &);
      bool is_match(const TrackInfo &) const;

      bool is_interp() const;

};

////////////////////////////////////////////////////////////////////////

inline bool TrackInfo::is_best_track() const               { return(IsBestTrack); }
inline bool TrackInfo::is_oper_track() const               { return(IsOperTrack); }
inline bool TrackInfo::is_anly_track() const               { return(IsAnlyTrack); }
inline void TrackInfo::set_storm_id(const char *s)         { StormId = s;         }
inline void TrackInfo::set_basin(const char *s)            { Basin = s;           }
inline void TrackInfo::set_cyclone(const char *s)          { Cyclone = s;         }
inline void TrackInfo::set_storm_name(const char *s)       { StormName = s;       }
inline void TrackInfo::set_technique_number(int i)         { TechniqueNumber = i; }
inline void TrackInfo::set_technique(const char *s)        { Technique = s;       }
inline void TrackInfo::set_initials(const char *s)         { Initials = s;        }
inline void TrackInfo::set_init(const unixtime u)          { InitTime = u;        }
inline void TrackInfo::set_valid_min(const unixtime u)     { MinValidTime = u;    }
inline void TrackInfo::set_valid_max(const unixtime u)     { MaxValidTime = u;    }
inline void TrackInfo::set_diag_name(const StringArray &s) { DiagName = s;        }

inline const ConcatString & TrackInfo::storm_id()         const { return(StormId);                      }
inline const ConcatString & TrackInfo::basin()            const { return(Basin);                        }
inline const ConcatString & TrackInfo::cyclone()          const { return(Cyclone);                      }
inline const ConcatString & TrackInfo::storm_name()       const { return(StormName);                    }
inline int                  TrackInfo::technique_number() const { return(TechniqueNumber);              }
inline const ConcatString & TrackInfo::technique()        const { return(Technique);                    }
inline const ConcatString & TrackInfo::initials()         const { return(Initials);                     }
inline unixtime             TrackInfo::init()             const { return(InitTime);                     }
inline int                  TrackInfo::init_hour()        const { return(unix_to_sec_of_day(InitTime)); }
inline unixtime             TrackInfo::valid_min()        const { return(MinValidTime);                 }
inline unixtime             TrackInfo::valid_max()        const { return(MaxValidTime);                 }
inline unixtime             TrackInfo::warm_core_min()    const { return(MinWarmCore);                  }
inline unixtime             TrackInfo::warm_core_max()    const { return(MaxWarmCore);                  }
inline int                  TrackInfo::n_points()         const { return(NPoints);                      }
inline int                  TrackInfo::n_diag()           const { return(DiagName.n());                 }
inline const StringArray &  TrackInfo::diag_name()        const { return(DiagName);                     }
inline StringArray          TrackInfo::track_lines()      const { return(TrackLines);                   }

////////////////////////////////////////////////////////////////////////
//
// TrackInfoArray class stores an array of TrackInfo objects.
//
////////////////////////////////////////////////////////////////////////

class TrackInfoArray {

   friend TrackInfo consensus(const TrackInfoArray &, const ConcatString &,
                              int, const StringArray &);

   private:

      void init_from_scratch();
      void assign(const TrackInfoArray &);

      std::vector<TrackInfo> Track;

   public:

      TrackInfoArray();
     ~TrackInfoArray();
      TrackInfoArray(const TrackInfoArray &);
      TrackInfoArray & operator=(const TrackInfoArray &);

      void clear();

      void         dump(std::ostream &, int = 0) const;
      ConcatString serialize()              const;
      ConcatString serialize_r(int = 0)     const;

         //
         //  set stuff
         //

      void add(const TrackInfo &);
      void set(int, const TrackInfo &);
      bool add(const ATCFTrackLine &, bool check_dup = false, bool check_anly = false);
      bool has(const ATCFTrackLine &) const;
      bool erase_storm_id(const ConcatString &);
      int  add_diag_data(DiagFile &, const StringArray &);

         //
         //  get stuff
         //

      const TrackInfo & operator[](int) const;
      int n() const;

};

////////////////////////////////////////////////////////////////////////

inline int TrackInfoArray::n() const { return(Track.size()); }

////////////////////////////////////////////////////////////////////////

extern TrackInfo consensus(const TrackInfoArray &, const ConcatString &, int, const StringArray &);
extern void compute_gc_dist_stdev(const double lat, const double lon,
                                  const NumArray &lats, const NumArray &lons,
                                  double &spread, double &mean);
extern bool has_storm_id(const StringArray &, const ConcatString &basin,
                         const ConcatString &cyclone, unixtime init);
extern void latlon_to_xytk_err(double alat, double alon,
                               double blat, double blon,
                               double &x_err, double &y_err, double &tk_err);

////////////////////////////////////////////////////////////////////////

#endif   /*  __VX_TRACK_INFO_H__  */

////////////////////////////////////////////////////////////////////////
