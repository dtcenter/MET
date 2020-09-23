// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2020
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

#include "atcf_track_line.h"

////////////////////////////////////////////////////////////////////////

// Define the wind intensity levels to be handled
static const int WindIntensity[] = { 34, 50, 64 };
static const int NWinds = sizeof(WindIntensity)/sizeof(*WindIntensity);

////////////////////////////////////////////////////////////////////////

class QuadInfo {
   private:

      void init_from_scratch();

      void assign(const QuadInfo &);

      int    Intensity;
      double ALVal;
      double NEVal;
      double SEVal;
      double SWVal;
      double NWVal;

   public:

      QuadInfo();
     ~QuadInfo();
      QuadInfo(const QuadInfo &);
      QuadInfo & operator=(const QuadInfo &);
      QuadInfo & operator+=(const QuadInfo &);
      bool       operator==(const QuadInfo &) const;

      void clear();

      void         dump(ostream &, int = 0)  const;
      ConcatString serialize()               const;
      ConcatString serialize_r(int, int = 0) const;

         //
         //  set stuff
         //

      void set_wind(const ATCFTrackLine &);
      void set_seas(const ATCFTrackLine &);
      void set_quad_vals(QuadrantType, int, int, int, int);

      void set_intensity(int);
      void set_al_val(double);
      void set_ne_val(double);
      void set_se_val(double);
      void set_sw_val(double);
      void set_nw_val(double);

         //
         //  get stuff
         //

      int    intensity() const;
      double al_val()    const;
      double ne_val()    const;
      double se_val()    const;
      double sw_val()    const;
      double nw_val()    const;

         //
         //  do stuff
         //

      bool is_match_wind(const ATCFTrackLine &) const;
      bool is_match_seas(const ATCFTrackLine &) const;

};

////////////////////////////////////////////////////////////////////////

inline void QuadInfo::set_intensity(int i) { Intensity = i; }
inline void QuadInfo::set_al_val(double v) { ALVal = v;     }
inline void QuadInfo::set_ne_val(double v) { NEVal = v;     }
inline void QuadInfo::set_se_val(double v) { SEVal = v;     }
inline void QuadInfo::set_sw_val(double v) { SWVal = v;     }
inline void QuadInfo::set_nw_val(double v) { NWVal = v;     }

inline int    QuadInfo::intensity() const { return(Intensity); }
inline double QuadInfo::al_val()    const { return(ALVal);     }
inline double QuadInfo::ne_val()    const { return(NEVal);     }
inline double QuadInfo::se_val()    const { return(SEVal);     }
inline double QuadInfo::sw_val()    const { return(SWVal);     }
inline double QuadInfo::nw_val()    const { return(NWVal);     }

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
      double        Vmax;       //  knots
      double        MSLP;       //  millibars
      CycloneLevel  Level;

      // Pressure of the last closed isobar (900 - 1050 mb)
      double        RadP;

      // Radius of last closed isobar (nm)
      double        RRP;

      // Radius of maximum winds (nm)
      double        MRD;

      // Gusts (kts)
      double        Gusts;

      // Diameter of eye (nm)
      double        Eye;

      // Direction and speed
      double        Direction;
      double        Speed;

      // System Depth
      SystemsDepth  Depth;

      // Watch/Warning status
      WatchWarnType WatchWarn;

      // Warm Core status
      bool WarmCore;

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

      void initialize(const ATCFTrackLine &);

      void set_valid(const unixtime);
      void set_lead(const int);
      void set_lat(const double);
      void set_lon(const double);
      void set_v_max(const double);
      void set_mslp(const double);
      void set_level(CycloneLevel);
      void set_radp(const double);
      void set_rrp(const double);
      void set_mrd(const double);
      void set_gusts(const double);
      void set_eye(const double);
      void set_direction(const double);
      void set_speed(const double);
      void set_depth(SystemsDepth);
      void set_warm_core(bool);
      void set_watch_warn(WatchWarnType);
      void set_watch_warn(WatchWarnType, unixtime);

         //
         //  get stuff
         //

      const QuadInfo & operator[](int) const;
      unixtime         valid()         const;
      int              valid_hour()    const;
      int              lead()          const;
      double           lat()           const;
      double           lon()           const;
      double           v_max()         const;
      double           mslp()          const;
      CycloneLevel     level()         const;
      double           radp()          const;
      double           rrp()           const;
      double           mrd()           const;
      double           gusts()         const;
      double           eye()           const;
      double           direction()     const;
      double           speed()         const;
      SystemsDepth     depth()         const;
      bool             warm_core()     const;
      WatchWarnType    watch_warn()    const;

         //
         //  do stuff
         //

      void set_wind(int, const QuadInfo &);
      bool set(const ATCFTrackLine &);
      bool is_match(const ATCFTrackLine &) const;

};

////////////////////////////////////////////////////////////////////////

inline void TrackPoint::set_valid(const unixtime t)     { ValidTime = t; }
inline void TrackPoint::set_lead(const int s)           { LeadTime  = s; }
inline void TrackPoint::set_lat(const double v)         { Lat       = v; }
inline void TrackPoint::set_lon(const double v)         { Lon       = v; }
inline void TrackPoint::set_v_max(const double v)       { Vmax      = v; }
inline void TrackPoint::set_mslp(const double v)        { MSLP      = v; }
inline void TrackPoint::set_level(CycloneLevel t)       { Level     = t; }
inline void TrackPoint::set_radp(const double v)        { RadP      = v; }
inline void TrackPoint::set_rrp(const double v)         { RRP       = v; }
inline void TrackPoint::set_mrd(const double v)         { MRD       = v; }
inline void TrackPoint::set_gusts(const double v)       { Gusts     = v; }
inline void TrackPoint::set_eye(const double v)         { Eye       = v; }
inline void TrackPoint::set_direction(const double v)   { Direction = v; }
inline void TrackPoint::set_speed(const double v)       { Speed     = v; }
inline void TrackPoint::set_depth(SystemsDepth t)       { Depth     = t; }
inline void TrackPoint::set_warm_core(bool v)           { WarmCore  = v; }
inline void TrackPoint::set_watch_warn(WatchWarnType t) { WatchWarn = t; }

inline unixtime      TrackPoint::valid()      const { return(ValidTime); }
inline int           TrackPoint::valid_hour() const { return(unix_to_sec_of_day(ValidTime)); }
inline int           TrackPoint::lead()       const { return(LeadTime);  }
inline double        TrackPoint::lat()        const { return(Lat);       }
inline double        TrackPoint::lon()        const { return(Lon);       }
inline double        TrackPoint::v_max()      const { return(Vmax);      }
inline double        TrackPoint::mslp()       const { return(MSLP);      }
inline CycloneLevel  TrackPoint::level()      const { return(Level);     }
inline double        TrackPoint::radp()       const { return(RadP);      }
inline double        TrackPoint::rrp()        const { return(RRP);       }
inline double        TrackPoint::mrd()        const { return(MRD);       }
inline double        TrackPoint::gusts()      const { return(Gusts);     }
inline double        TrackPoint::eye()        const { return(Eye);       }
inline double        TrackPoint::direction()  const { return(Direction); }
inline double        TrackPoint::speed()      const { return(Speed);     }
inline SystemsDepth  TrackPoint::depth()      const { return(Depth);     }
inline bool          TrackPoint::warm_core()  const { return(WarmCore);  }
inline WatchWarnType TrackPoint::watch_warn() const { return(WatchWarn); }

////////////////////////////////////////////////////////////////////////

#endif   /*  __VX_TRACK_POINT_H__  */

////////////////////////////////////////////////////////////////////////
