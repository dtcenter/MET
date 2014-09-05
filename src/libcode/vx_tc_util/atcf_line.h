// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2014
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

static const int   MinATCFElements = 18;
static const char  BestTrackStr[]  = "BEST";
static const char  OperTrackStr[]  = "CARQ";

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

class ATCFLine {

      friend bool operator>>(istream &, ATCFLine &);

   private:

      void init_from_scratch();

      void assign(const ATCFLine &);

      ConcatString Line;
      
      ConcatString Basin;

      ConcatString CycloneNumber;

      unixtime WarningTime;

      int TechniqueNumber;   // or minutes for best track

      ConcatString Technique;

      int ForecastPeriod;   // forecast hours, -24 to 240

      int LatTenths;   // + north, - south

      int LonTenths;   // + east, - west

      int Vmax;   // knots

      int MSLP;   // millibars

      CycloneLevel Level;

      int WindIntensity;   // knots

      QuadrantType Quadrant;
      
      int Radius1;  // nautical miles
      int Radius2;  // nautical miles
      int Radius3;  // nautical miles
      int Radius4;  // nautical miles

      int IsobarPressure;   // millibars

      int IsobarRadius;     // nautical miles

      int MaxWindRadius;    // nautical miles

      int Gusts;   // knots

      int EyeDiameter;   // nautical miles

      SubregionCode SubRegion;

      int MaxSeas;   // feet

      ConcatString Initials;   // forecaster's initials

      int StormDirection;   // degrees

      int StormSpeed;       // knots

      ConcatString StormName;

      SystemsDepth Depth;

      int WaveHeight;   // feet

      QuadrantType SeasCode;

      int SeasRadius1;   // nautical miles
      int SeasRadius2;   // nautical miles
      int SeasRadius3;   // nautical miles
      int SeasRadius4;   // nautical miles

   public:

      ATCFLine();
     ~ATCFLine();
      ATCFLine(const ATCFLine &);
      ATCFLine & operator= (const ATCFLine &);
      bool        operator==(const ATCFLine &);

      void clear();

      void dump(ostream &, int = 0) const;

         //
         // set stuff
         //

      void set_technique(const ConcatString &);

         //
         // get stuff
         //

      ConcatString line() const;

      ConcatString basin() const;

      ConcatString cyclone_number() const;

      unixtime warning_time() const;

      int technique_number() const;   // or minutes for best track

      int forecast_period() const;

      unixtime valid() const;   // WarningTime + ForecastPeriod

      int lead() const;   // seconds

      ConcatString technique() const;

      double lat() const;    // degrees, + north, - south

      double lon() const;   // degrees, + west, - east

      int v_max() const;

      int mslp() const;

      CycloneLevel level() const;

      int wind_intensity() const;

      QuadrantType quadrant() const;
      
      int radius1() const;
      int radius2() const;
      int radius3() const;
      int radius4() const;

      int isobar_pressure() const;

      int isobar_radius() const;

      int max_wind_radius() const;

      int gusts() const;

      int eye_diameter() const;

      SubregionCode subregion() const;

      int max_seas() const;

      ConcatString initials() const;

      int storm_direction() const;

      int storm_speed() const;

      ConcatString storm_name() const;

      SystemsDepth depth() const;

      int wave_height() const;

      QuadrantType seas_code() const;

      int seas_radius1() const;
      int seas_radius2() const;
      int seas_radius3() const;
      int seas_radius4() const;

         //
         // do stuff
         //

};

////////////////////////////////////////////////////////////////////////

inline void           ATCFLine::set_technique(const ConcatString &s) { Technique = s; }

inline ConcatString   ATCFLine::line()             const { return(Line);            }
inline ConcatString   ATCFLine::basin()            const { return(Basin);           }
inline ConcatString   ATCFLine::cyclone_number()   const { return(CycloneNumber);   }
inline unixtime       ATCFLine::warning_time()     const { return(WarningTime);     }
inline int            ATCFLine::technique_number() const { return(TechniqueNumber); }
inline int            ATCFLine::forecast_period()  const { return(ForecastPeriod);  }
inline ConcatString   ATCFLine::technique()        const { return(Technique);       }
inline double         ATCFLine::lat()              const { return(0.1*LatTenths);   }
inline double         ATCFLine::lon()              const { return(0.1*LonTenths);   }
inline int            ATCFLine::v_max()            const { return(Vmax);            }
inline int            ATCFLine::mslp()             const { return(MSLP);            }
inline CycloneLevel   ATCFLine::level()            const { return(Level);           }
inline int            ATCFLine::wind_intensity()   const { return(WindIntensity);   }
inline QuadrantType   ATCFLine::quadrant()         const { return(Quadrant);        }
inline int            ATCFLine::radius1()          const { return(Radius1);         }
inline int            ATCFLine::radius2()          const { return(Radius2);         }
inline int            ATCFLine::radius3()          const { return(Radius3);         }
inline int            ATCFLine::radius4()          const { return(Radius4);         }
inline int            ATCFLine::isobar_pressure()  const { return(IsobarPressure);  }
inline int            ATCFLine::isobar_radius()    const { return(IsobarRadius);    }
inline int            ATCFLine::max_wind_radius()  const { return(MaxWindRadius);   }
inline int            ATCFLine::gusts()            const { return(Gusts);           }
inline int            ATCFLine::eye_diameter()     const { return(EyeDiameter);     }
inline SubregionCode  ATCFLine::subregion()        const { return(SubRegion);       }
inline int            ATCFLine::max_seas()         const { return(MaxSeas);         }
inline ConcatString   ATCFLine::initials()         const { return(Initials);        }
inline int            ATCFLine::storm_direction()  const { return(StormDirection);  }
inline int            ATCFLine::storm_speed()      const { return(StormSpeed);      }
inline ConcatString   ATCFLine::storm_name()       const { return(StormName);       }
inline SystemsDepth   ATCFLine::depth()            const { return(Depth);           }
inline int            ATCFLine::wave_height()      const { return(WaveHeight);      }
inline QuadrantType   ATCFLine::seas_code()        const { return(SeasCode);        }
inline int            ATCFLine::seas_radius1()     const { return(SeasRadius1);     }
inline int            ATCFLine::seas_radius2()     const { return(SeasRadius2);     }
inline int            ATCFLine::seas_radius3()     const { return(SeasRadius3);     }
inline int            ATCFLine::seas_radius4()     const { return(SeasRadius4);     }

////////////////////////////////////////////////////////////////////////

extern bool operator>>(istream &, ATCFLine &);

extern CycloneLevel   wind_speed_to_cyclone_level(int);
  
////////////////////////////////////////////////////////////////////////

#endif   /*  __VX_ATCF_LINE_H__  */

////////////////////////////////////////////////////////////////////////
