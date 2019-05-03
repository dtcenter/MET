// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2016
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*

////////////////////////////////////////////////////////////////////////

#ifndef  __VX_ATCF_LINE_H__
#define  __VX_ATCF_LINE_H__

////////////////////////////////////////////////////////////////////////
//
// Based on Best Track file format information at:
//    http://www.nrlmry.navy.mil/atcf_web/docs/database/new/abrdeck.html
//
////////////////////////////////////////////////////////////////////////

#include <iostream>

#include "vx_cal.h"
#include "vx_util.h"

////////////////////////////////////////////////////////////////////////

enum WatchWarnType {
   TropicalStormWatch, // Tropical Storm Watch
   TropicalStormWarn,  // Tropical Storm Warning
   
   GaleWarn,           // Gale Warning
   StormWarn,          // Storm Warning
   
   HurricaneWatch,     // Hurricane Watch
   HurricaneWarn,      // Hurricane Warning

   NoWatchWarnType
};

extern WatchWarnType ww_max(const WatchWarnType, const WatchWarnType);
extern WatchWarnType int_to_watchwarntype(int);
extern WatchWarnType string_to_watchwarntype(const char *);
extern ConcatString  watchwarntype_to_string(const WatchWarnType);

////////////////////////////////////////////////////////////////////////

enum CycloneLevel {
   Disturbance,            // DB
   TropicalDepression,     // TD
   TropicalStorm,          // TS

   Typhoon,                // TY
   SuperTyphoon,           // ST
   TropicalCyclone,        // TC

   Hurricane,              // HU
   SubtropicalDepression,  // SD
   SubtropicalStorm,       // SS

   ExtratropicalSystem,    // EX
   Inland,                 // IN
   Dissipating,            // DS

   Low,                    // LO
   TropicalWave,           // WV
   Extrapolated,           // ET

   NoCycloneLevel          // XX
};

extern CycloneLevel string_to_cyclonelevel(const char *);
extern ConcatString cyclonelevel_to_string(const CycloneLevel);

////////////////////////////////////////////////////////////////////////

enum QuadrantType {
    FullCircle, // AAA

    N_Quadrant, // NNQ
    E_Quadrant, // EEQ
    S_Quadrant, // SSQ
    W_Quadrant, // WWQ

   NE_Quadrant, // NEQ
   SE_Quadrant, // SEQ
   SW_Quadrant, // SWQ
   NW_Quadrant, // NWQ

   NoQuadrantType
};

extern QuadrantType string_to_quadranttype(const char *);
extern ConcatString quadranttype_to_string(const QuadrantType);

////////////////////////////////////////////////////////////////////////

enum SubregionCode {
   Arabian_Sea,       // A
   Bay_of_Bengal,     // B
   Central_Pacific,   // C
   Eastern_Pacific,   // E
   Atlantic,          // L
   South_Pacific,     // P
   South_Atlantic,    // Q
   South_IO,          // S
   Western_Pacific,   // W
   NoSubregionCode
};

extern SubregionCode string_to_subregioncode(const char *);
extern ConcatString  subregioncode_to_string(const SubregionCode);

////////////////////////////////////////////////////////////////////////

enum SystemsDepth {
   DeepDepth,         // D
   MediumDepth,       // M
   ShallowDepth,      // S
   NoSystemsDepth     // X
};

extern SystemsDepth string_to_systemsdepth(const char *);
extern ConcatString systemsdepth_to_string(const SystemsDepth);

////////////////////////////////////////////////////////////////////////
//
// ATCFLine class stores the data in a single ATCF line.
//
////////////////////////////////////////////////////////////////////////

class ATCFLine : public DataLine {

   private:

      void init_from_scratch();

      void assign(const ATCFLine &);

      ConcatString Technique;
      bool         IsBestTrack;
      bool         IsOperTrack;

   public:

      ATCFLine();
     ~ATCFLine();
      ATCFLine(const ATCFLine &);
      ATCFLine & operator= (const ATCFLine &);
      bool       operator==(const ATCFLine &);

      void dump(ostream &, int depth = 0) const;

      void clear();

      int read_line(LineDataFile *);   //  virtual from base class

      int is_header() const;           //  virtual from base class

         //
         // set values
         //

      void set_technique(const ConcatString &);
      void set_best_track(const bool);
      void set_oper_track(const bool);

         //
         // retrieve column values
         //

      bool          is_best_track()    const;
      bool          is_oper_track()    const;

      ConcatString  get_item     (int) const;
      ConcatString  get_line        () const;
      
      ConcatString  basin           () const;
      ConcatString  cyclone_number  () const;
      unixtime      warning_time    () const;
      int           technique_number() const; // or minutes for best track
      ConcatString  technique       () const;

      int           forecast_period () const;
      double        lat             () const; // degrees, + north, - south
      double        lon             () const; // degrees, + west, - east
      int           v_max           () const;
      int           mslp            () const;

      unixtime      valid           () const; // WarningTime + ForecastPeriod
      int           lead            () const; // seconds
      CycloneLevel  level           () const;
      int           wind_intensity  () const;

      QuadrantType  quadrant        () const;      
      int           radius1         () const;
      int           radius2         () const;
      int           radius3         () const;
      int           radius4         () const;

      int           isobar_pressure () const;
      int           isobar_radius   () const;
      int           max_wind_radius () const;
      int           gusts           () const;
      int           eye_diameter    () const;

      SubregionCode subregion       () const;
      int           max_seas        () const;
      ConcatString  initials        () const;
      int           storm_direction () const;
      int           storm_speed     () const;

      ConcatString  storm_name      () const;
      SystemsDepth  depth           () const;
      int           wave_height     () const;

      QuadrantType  seas_code       () const;
      int           seas_radius1    () const;
      int           seas_radius2    () const;
      int           seas_radius3    () const;
      int           seas_radius4    () const;
};

////////////////////////////////////////////////////////////////////////

inline void ATCFLine::set_technique(const ConcatString &s) { Technique   = s;     }
inline void ATCFLine::set_best_track(const bool tf)        { IsBestTrack = tf;    } 
inline void ATCFLine::set_oper_track(const bool tf)        { IsOperTrack = tf;    } 
inline bool ATCFLine::is_best_track() const                { return(IsBestTrack); }
inline bool ATCFLine::is_oper_track() const                { return(IsOperTrack); }

////////////////////////////////////////////////////////////////////////

extern CycloneLevel wind_speed_to_cyclone_level(int);
  
////////////////////////////////////////////////////////////////////////

#endif   /*  __VX_ATCF_LINE_H__  */

////////////////////////////////////////////////////////////////////////
