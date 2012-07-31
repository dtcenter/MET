// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2012
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*

////////////////////////////////////////////////////////////////////////

#ifndef  __VX_TRACK_POINT_H__
#define  __VX_TRACK_POINT_H__

////////////////////////////////////////////////////////////////////////

#include <iostream>

#include "vx_cal.h"
#include "vx_util.h"

#include "track_line.h"

////////////////////////////////////////////////////////////////////////

static const int NQuadInfoValues = 4;

// Define the wind intensity levels to be handled
static const int WindIntensity[] = { 34, 50, 64 };
static const int NWinds = sizeof(WindIntensity)/sizeof(*WindIntensity);

////////////////////////////////////////////////////////////////////////

class QuadInfo {
   private:

      void init_from_scratch();

      void assign(const QuadInfo &);

      int          Intensity;
      QuadrantType Quadrant;
      int          Value[NQuadInfoValues];

   public:

      QuadInfo();
     ~QuadInfo();
      QuadInfo(const QuadInfo &);
      QuadInfo & operator=(const QuadInfo &);
      QuadInfo & operator+=(const QuadInfo &);

      void clear();

      void         dump(ostream &, int = 0)  const;
      ConcatString serialize()               const;
      ConcatString serialize_r(int, int = 0) const;

         //
         //  set stuff
         //

      void set_wind(const TrackLine &);
      void set_seas(const TrackLine &);

      void set_intensity(int);
      void set_quadrant(QuadrantType);
      void set_value(int, int);

         //
         //  get stuff
         //

      int    operator[](int) const;
      int          intensity()     const;
      QuadrantType quadrant()      const;

         //
         //  do stuff
         //

      bool has_wind(const TrackLine &)      const;
      bool has_seas(const TrackLine &)      const;

      bool is_match_wind(const TrackLine &) const;
      bool is_match_seas(const TrackLine &) const;

};

////////////////////////////////////////////////////////////////////////

inline void QuadInfo::set_intensity(int i)         { Intensity = i; }
inline void QuadInfo::set_quadrant(QuadrantType t) { Quadrant = t;  }

inline int          QuadInfo::intensity() const { return(Intensity); }
inline QuadrantType QuadInfo::quadrant()  const { return(Quadrant);  }

////////////////////////////////////////////////////////////////////////
//
// TrackPoint class stores the data for a single (lat,lon) track point.
//
////////////////////////////////////////////////////////////////////////

class TrackPoint {

   private:

      void init_from_scratch();
      void assign(const TrackPoint &);

      bool          IsSet;
      
      // Timing information
      unixtime      ValidTime;
      int           LeadTime;   //  seconds
 
      // Location
      double        Lat;        //  degrees, + north, - south
      double        Lon;        //  degrees, + west, - east

      // Intensity
      int           Vmax;       //  knots
      int           MSLP;       //  millibars
      CycloneLevel  Level;
      WatchWarnType WatchWarn; // watch/warning status

      // Wind Radii
      QuadInfo      Wind[NWinds];

   public:

      TrackPoint();
     ~TrackPoint();
      TrackPoint(const TrackPoint &);
      TrackPoint & operator=(const TrackPoint &);
      TrackPoint & operator+=(const TrackPoint &);      

      void clear();

      void         dump(ostream &, int = 0)  const;
      ConcatString serialize()               const;
      ConcatString serialize_r(int, int = 0) const;

         //
         //  set stuff
         //

      void initialize(const TrackLine &);

      void set_valid(const unixtime);
      void set_lead(const int);
      void set_lat(const double);
      void set_lon(const double);
      void set_v_max(const int);
      void set_mslp(const int);
      void set_level(CycloneLevel);
      void set_watch_warn(WatchWarnType);
      void set_watch_warn(WatchWarnType, unixtime);

         //
         //  get stuff
         //
         
      const QuadInfo & operator[](int) const;
      unixtime         valid()         const;
      int              lead()          const;
      double           lat()           const;
      double           lon()           const;
      int              v_max()         const;
      int              mslp()          const;
      CycloneLevel     level()         const;
      WatchWarnType    watch_warn()    const;

         //
         //  do stuff
         //

      void set_wind(int, const QuadInfo &);
      bool set(const TrackLine &);
      bool has(const TrackLine &)      const;
      bool is_match(const TrackLine &) const;

};

////////////////////////////////////////////////////////////////////////

inline void TrackPoint::set_valid(const unixtime u)    { ValidTime = u; }
inline void TrackPoint::set_lead(const int s)          { LeadTime = s;  }
inline void TrackPoint::set_lat(const double l)        { Lat = l;       }
inline void TrackPoint::set_lon(const double l)        { Lon = l;       }
inline void TrackPoint::set_v_max(const int v)         { Vmax = v;      }
inline void TrackPoint::set_mslp(const int v)          { MSLP = v;      }
inline void TrackPoint::set_level(CycloneLevel l)      { Level = l;     }
inline void TrackPoint::set_watch_warn(WatchWarnType t){ WatchWarn = t; }

inline unixtime      TrackPoint::valid()      const { return(ValidTime); }
inline int           TrackPoint::lead()       const { return(LeadTime);  }
inline double        TrackPoint::lat()        const { return(Lat);       }
inline double        TrackPoint::lon()        const { return(Lon);       }
inline int           TrackPoint::v_max()      const { return(Vmax);      }
inline int           TrackPoint::mslp()       const { return(MSLP);      }
inline CycloneLevel  TrackPoint::level()      const { return(Level);     }
inline WatchWarnType TrackPoint::watch_warn() const { return(WatchWarn); }

////////////////////////////////////////////////////////////////////////

#endif   /*  __VX_TRACK_POINT_H__  */

////////////////////////////////////////////////////////////////////////
