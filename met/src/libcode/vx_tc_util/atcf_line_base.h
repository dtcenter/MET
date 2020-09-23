// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2020
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*

////////////////////////////////////////////////////////////////////////

#ifndef  __VX_ATCF_LINE_BASE_H__
#define  __VX_ATCF_LINE_BASE_H__

////////////////////////////////////////////////////////////////////////
//
// Based on Best Track file format information at:
//    http://www.nrlmry.navy.mil/atcf_web/docs/database/new/abrdeck.html
//
////////////////////////////////////////////////////////////////////////

#include <iostream>
#include <map>

#include "vx_cal.h"
#include "vx_util.h"

////////////////////////////////////////////////////////////////////////

enum ATCFLineType {
   ATCFLineType_Track,    // Track and intensity line type (numeric)
   ATCFLineType_GenTrack, // Genesis Track and intensity line type (numeric)
   ATCFLineType_ProbTR,   // Track probability (TR)
   ATCFLineType_ProbIN,   // Intensity probability (IN)
   ATCFLineType_ProbRIRW, // Rapid intensification probability (RI)
   ATCFLineType_ProbWD,   // Wind radii probability (WD)
   ATCFLineType_ProbPR,   // Pressure probability (PR)
   ATCFLineType_ProbGN,   // TC genesis probability (GN)
   ATCFLineType_ProbGS,   // TC genesis shape probability (GS)

   NoATCFLineType
};

extern ATCFLineType string_to_atcflinetype(const char *);
extern ConcatString atcflinetype_to_string(const ATCFLineType);

////////////////////////////////////////////////////////////////////////
//
// Base class for various ATCF line types.
//
////////////////////////////////////////////////////////////////////////

class ATCFLineBase : public DataLine {

   protected:

      void init_from_scratch();

      void assign(const ATCFLineBase &);

      const map<ConcatString,ConcatString> * BasinMap;       // not allocated
      const StringArray                    * BestTechnique;  // not allocated
      const StringArray                    * OperTechnique;  // not allocated
      const ConcatString                   * TechSuffix;     // not allocated

      ATCFLineType Type;
      ConcatString Basin;
      ConcatString Technique;
      bool         IsBestTrack;
      bool         IsOperTrack;

   public:

      ATCFLineBase();
     ~ATCFLineBase();
      ATCFLineBase(const ATCFLineBase &);
      ATCFLineBase & operator= (const ATCFLineBase &);
      bool           operator==(const ATCFLineBase &);

      void dump(ostream &, int depth = 0) const;

      void clear();

      int read_line(LineDataFile *);   //  virtual from base class

      bool is_header() const;          //  virtual from base class

         //
         // set values
         //

      void set_basin_map     (const map<ConcatString,ConcatString> *);
      void set_best_technique(const StringArray *);
      void set_oper_technique(const StringArray *);
      void set_tech_suffix   (const ConcatString *);
      void set_technique     (const ConcatString &);

         //
         // retrieve column values
         //

      ATCFLineType  type            () const;

      bool          is_best_track   () const;
      bool          is_oper_track   () const;

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
      unixtime      valid           () const; // WarningTime + ForecastPeriod
      int           valid_hour      () const;
      int           lead            () const; // seconds

      ConcatString  storm_id        () const;

};

////////////////////////////////////////////////////////////////////////

inline void ATCFLineBase::set_basin_map     (const map<ConcatString,ConcatString> *m)
                                                                    { BasinMap = m;         }
inline void ATCFLineBase::set_best_technique(const StringArray *s)  { BestTechnique = s;    }
inline void ATCFLineBase::set_oper_technique(const StringArray *s)  { OperTechnique = s;    }
inline void ATCFLineBase::set_tech_suffix   (const ConcatString *s) { TechSuffix = s;       }
inline void ATCFLineBase::set_technique     (const ConcatString &s) { Technique = s;        }
inline bool ATCFLineBase::is_best_track     () const                { return(IsBestTrack);  }
inline bool ATCFLineBase::is_oper_track     () const                { return(IsOperTrack);  }

////////////////////////////////////////////////////////////////////////

extern unixtime parse_time           (const char *);
extern double   parse_lat            (const char *);
extern double   parse_lon            (const char *);
extern int      parse_int            (const char *, const int bad_data=bad_data_int);
extern int      parse_int_check_zero (const char *);

extern ConcatString define_storm_id(unixtime, unixtime, unixtime,
                                    const ConcatString &,
                                    const ConcatString &);

////////////////////////////////////////////////////////////////////////

#endif   /*  __VX_ATCF_LINE_BASE_H__  */

////////////////////////////////////////////////////////////////////////
