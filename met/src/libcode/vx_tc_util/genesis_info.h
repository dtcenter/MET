// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2020
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

class GenesisInfo {

   private:

      void assign(const GenesisInfo &);

      bool IsSet;

      // Pointer to TrackInfo
      const TrackInfo *Track; // not allocated
      int   GenIndex;

      // Genesis and model information
      ConcatString Technique;
      unixtime     GenesisTime;
      unixtime     InitTime;
      int          LeadTime;

      // Genesis Location
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
      bool set(const TrackInfo *,
               const GenesisEventInfo *);

         //
         //  get stuff
         //
   
      const TrackInfo *track()        const;
      double           lat()          const;
      double           lon()          const;
      double           dland()        const;
      unixtime         genesis_time() const;
      unixtime         init()         const;
      int              init_hour()    const;
      int              lead_time()    const;

      ConcatString     storm_id()     const;
      ConcatString     basin()        const;
      ConcatString     cyclone()      const;
      ConcatString     storm_name()   const;
      ConcatString     technique()    const;
      unixtime         valid_min()    const;
      unixtime         valid_max()    const;
      int              duration()     const;

         //
         //  do stuff
         //

      bool is_match(const TrackPoint &,
                    const double) const;
};

////////////////////////////////////////////////////////////////////////

inline const TrackInfo *GenesisInfo::track()            const { return(Track);                        }
inline double           GenesisInfo::lat()              const { return(Lat);                          }
inline double           GenesisInfo::lon()              const { return(Lon);                          }
inline double           GenesisInfo::dland()            const { return(DLand);                        }
inline unixtime         GenesisInfo::genesis_time()     const { return(GenesisTime);                  }
inline unixtime         GenesisInfo::init()             const { return(InitTime);                     }
inline int              GenesisInfo::init_hour()        const { return(unix_to_sec_of_day(InitTime)); }
inline int              GenesisInfo::lead_time()        const { return(LeadTime);                     }

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
