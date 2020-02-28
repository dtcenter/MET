// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2020
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*

////////////////////////////////////////////////////////////////////////

#ifndef  __VX_TRACK_PAIR_INFO_H__
#define  __VX_TRACK_PAIR_INFO_H__

////////////////////////////////////////////////////////////////////////

#include <iostream>

#include "track_point.h"
#include "track_info.h"
#include "tc_stat_line.h"

#include "vx_cal.h"
#include "vx_math.h"
#include "vx_util.h"
#include "vx_config.h"

////////////////////////////////////////////////////////////////////////

static const int TrackPairLineAllocInc = 10;
static const int TrackPairInfoAllocInc = 10;

////////////////////////////////////////////////////////////////////////
//
// TrackPairInfo class stores a matched ADECK and BDECK track and their
// corresponding errors.
//
////////////////////////////////////////////////////////////////////////

class TrackPairInfo {

   private:

      void init_from_scratch();
      void assign(const TrackPairInfo &);
      void extend(int, bool exact = true);

      // Number of track points
      int          NPoints;

      // ADECK and BDECK tracks
      TrackInfo    ADeck;
      TrackInfo    BDeck;

      // Distances to land
      NumArray     ADeckDLand;
      NumArray     BDeckDLand;

      // Track Error, X/Y Errors, and Along/Cross Track Errors
      NumArray     TrackErr;
      NumArray     XErr;
      NumArray     YErr;
      NumArray     AlongTrackErr;
      NumArray     CrossTrackErr;

      // TCStatLines used to construct this track
      int          NLines;
      int          NAlloc;
      TCStatLine * Line;

      // Status for whether RI/RW occurred
      NumArray     ADeckRIRW;
      NumArray     BDeckRIRW;

      // Previous intensity values
      NumArray     ADeckPrvInt;
      NumArray     BDeckPrvInt;

      // Status for whether this point should be used
      NumArray     Keep;

   public:

      TrackPairInfo();
     ~TrackPairInfo();
      TrackPairInfo(const TrackPairInfo &);
      TrackPairInfo & operator=(const TrackPairInfo &);

      void clear();

      void         dump(ostream &, int = 0)  const;
      ConcatString case_info()               const;
      ConcatString serialize()               const;
      ConcatString serialize_r(int, int = 0) const;

         //
         //  set stuff
         //

      void initialize(const TrackInfo &, const TrackInfo &);
      void initialize(const TCStatLine &);
      void set_keep(int, int);

         //
         //  get stuff
         //

      int                n_points()           const;
      const TrackInfo &  adeck()              const;
      const TrackInfo &  bdeck()              const;
      unixtime           valid(int)           const;
      double             adeck_dland(int)     const;
      double             bdeck_dland(int)     const;
      double             track_err(int)       const;
      double             x_err(int)           const;
      double             y_err(int)           const;
      double             along_track_err(int) const;
      double             cross_track_err(int) const;
      int                adeck_rirw(int)      const;
      int                bdeck_rirw(int)      const;
      int                adeck_prv_int(int)   const;
      int                bdeck_prv_int(int)   const;

      int                n_lines()            const;
      const TCStatLine * line(int i)          const;
      int                i_init()             const;

      bool               keep(int)            const;
      WatchWarnType      watch_warn(int)      const;

         //
         //  do stuff
         //

      void add(const TrackPoint &, const TrackPoint &,
               double, double, double, double, double, double, double);
      void add(const TCStatLine &);
      void add_watch_warn(const ConcatString &, WatchWarnType, unixtime);

      int  check_water_only();
      int  check_rirw(const TrackType, const int, const int,
                      const bool, const bool,
                      const SingleThresh &, const SingleThresh &);
      int  check_landfall(const int, const int);
      bool landfall_window(unixtime, unixtime) const;

      TrackPairInfo keep_subset() const;
};

////////////////////////////////////////////////////////////////////////

inline int                TrackPairInfo::n_points()             const { return(NPoints);              }
inline const TrackInfo &  TrackPairInfo::adeck()                const { return(ADeck);                }
inline const TrackInfo &  TrackPairInfo::bdeck()                const { return(BDeck);                }
inline double             TrackPairInfo::adeck_dland(int i)     const { return(ADeckDLand[i]);        }
inline double             TrackPairInfo::bdeck_dland(int i)     const { return(BDeckDLand[i]);        }
inline double             TrackPairInfo::track_err(int i)       const { return(TrackErr[i]);          }
inline double             TrackPairInfo::x_err(int i)           const { return(XErr[i]);              }
inline double             TrackPairInfo::y_err(int i)           const { return(YErr[i]);              }
inline double             TrackPairInfo::along_track_err(int i) const { return(AlongTrackErr[i]);     }
inline double             TrackPairInfo::cross_track_err(int i) const { return(CrossTrackErr[i]);     }
inline int                TrackPairInfo::adeck_rirw(int i)      const { return(nint(ADeckRIRW[i]));   }
inline int                TrackPairInfo::bdeck_rirw(int i)      const { return(nint(BDeckRIRW[i]));   }
inline int                TrackPairInfo::adeck_prv_int(int i)   const { return(nint(ADeckPrvInt[i])); }
inline int                TrackPairInfo::bdeck_prv_int(int i)   const { return(nint(BDeckPrvInt[i])); }
inline int                TrackPairInfo::n_lines()              const { return(NLines);               }
inline const TCStatLine * TrackPairInfo::line(int i)            const { return(&Line[i]);             }
inline bool               TrackPairInfo::keep(int i)            const { return(Keep[i] != 0);         }

////////////////////////////////////////////////////////////////////////
//
// TrackPairInfoArray class stores an array of TrackPairInfo objects.
//
////////////////////////////////////////////////////////////////////////

class TrackPairInfoArray {

   private:

      void init_from_scratch();
      void assign(const TrackPairInfoArray &);
      void extend(int, bool exact = true);

      TrackPairInfo *Pair;
      int            NPairs;
      int            NAlloc;

   public:

      TrackPairInfoArray();
     ~TrackPairInfoArray();
      TrackPairInfoArray(const TrackPairInfoArray &);
      TrackPairInfoArray & operator=(const TrackPairInfoArray &);

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

      const TrackPairInfo & operator[](int) const;
      int n_pairs()  const;
      int n_points() const;

         //
         //  do stuff
         //

      void add(const TrackPairInfo &);
      void add_watch_warn(const ConcatString &, WatchWarnType, unixtime);
};

////////////////////////////////////////////////////////////////////////

inline int TrackPairInfoArray::n_pairs() const { return(NPairs); }

////////////////////////////////////////////////////////////////////////

#endif   /*  __VX_TRACK_PAIR_INFO_H__  */

////////////////////////////////////////////////////////////////////////
