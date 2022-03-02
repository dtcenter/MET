// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2022
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
// Struct to store genesis event defintion criteria
//
////////////////////////////////////////////////////////////////////////

struct GenesisEventInfo {
   ConcatString         Technique;
   vector<CycloneLevel> Category;
   SingleThresh         VMaxThresh;
   SingleThresh         MSLPThresh;

   bool                 is_genesis(const TrackPoint &) const;
   void                 clear();
};

extern GenesisEventInfo parse_conf_genesis_event_info(Dictionary *dict);

////////////////////////////////////////////////////////////////////////
//
// GenesisInfo class stores information about genesis events.
//
////////////////////////////////////////////////////////////////////////

class GenesisInfo : public TrackInfo {

   private:

      void assign(const GenesisInfo &);

      // Genesis Information
      int      GenesisIndex;
      unixtime GenesisTime;
      int      GenesisLead;
      double   Lat;
      double   Lon;
      double   DLand;

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

      void set_dland(double);
      bool set(const TrackInfo &, const GenesisEventInfo &);

         //
         //  get stuff
         //
   
      const TrackPoint *genesis() const;

      double   lat()          const;
      double   lon()          const;
      double   dland()        const;
      unixtime genesis_time() const;
      int      genesis_lead() const;
      int      genesis_fhr()  const;

         //
         //  do stuff
         //

      bool is_match(const TrackPoint &,
                    const double,
                    const int, const int) const;
      bool is_match(const GenesisInfo &,
                    const double,
                    const int, const int) const;
};

////////////////////////////////////////////////////////////////////////

inline double   GenesisInfo::lat()          const { return(Lat);         }
inline double   GenesisInfo::lon()          const { return(Lon);         }
inline double   GenesisInfo::dland()        const { return(DLand);       }
inline unixtime GenesisInfo::genesis_time() const { return(GenesisTime); }
inline int      GenesisInfo::genesis_lead() const { return(GenesisLead); }

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

      bool add(const GenesisInfo &);
      bool has(const GenesisInfo &) const;
      bool has_storm(const GenesisInfo &, int &) const;
      bool has_storm_id(const ConcatString &, int &) const;
      bool erase_storm_id(const ConcatString &);

         //
         //  get stuff
         //

      const GenesisInfo & operator[](int) const;
      int n() const;
      int n_technique() const;
};

////////////////////////////////////////////////////////////////////////

inline int GenesisInfoArray::n() const { return(Genesis.size()); }

////////////////////////////////////////////////////////////////////////

#endif   /*  __VX_GENESIS_INFO_H__  */

////////////////////////////////////////////////////////////////////////
