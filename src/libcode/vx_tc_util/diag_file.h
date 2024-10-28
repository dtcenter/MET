// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2024
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*

////////////////////////////////////////////////////////////////////////

#ifndef  __DIAG_FILE_H__
#define  __DIAG_FILE_H__

////////////////////////////////////////////////////////////////////////

#include <sys/types.h>
#include <unistd.h>
#include <map>

#include "vx_config.h"
#include "vx_cal.h"
#include "data_line.h"

////////////////////////////////////////////////////////////////////////
//
// Realtime CIRA Diagnostics files:
//   - Add link to sample data
//   - Header:
//       * ATCF_ID YYYYMMDDHH *
//       * BBNN    BBNN       *
//       ATCF_ID is the technique (model) name
//       YYYYMMDDHH is the initialization time
//       BB is the 2-letter basin name
//       CC is the 2-digit cyclone number
//
// Real-time SHIPS Diagnostics files:
//   - https://ftp.nhc.noaa.gov/atcf/lsdiag
//   - Header:
//       BBCC YYMMDD HH WS LAT LON 9999 BBCCYYYY
//       BB is 2-letter basin name
//       CC is 2-digit cyclone number
//       YYMMDD HH is the initialization time
//       WS is the wind speed
//
// Developmental SHIPS Diagnostics files (not currently supported):
//   - https://rammb2.cira.colostate.edu/research/tropical-cyclones/ships
//
////////////////////////////////////////////////////////////////////////

class DiagFile : public LineDataFile {

   private:

      DiagFile(const DiagFile &);
      DiagFile & operator=(const DiagFile &);

   protected:

      void init_from_scratch();

      // Diagnostics Metadata
      DiagType     DiagSource;
      ConcatString TrackSource;
      ConcatString FieldSource;

      // Storm and model identification
      ConcatString StormId;
      ConcatString Basin;
      ConcatString Cyclone;
      StringArray  Technique;
      unixtime     InitTime;

      int          NTime;
      IntArray     LeadTime;
      NumArray     Lat;
      NumArray     Lon;

      // Diagnostic names and values
      StringArray           DiagName;
      std::vector<NumArray> DiagVal;

   public:

      DiagFile();
     ~DiagFile();

      void clear();

         //
         //  set stuff
         //

      void set_technique(const StringArray &);
      void add_technique(const std::string &);

         //
         //  get stuff
         //

      const ConcatString & storm_id()  const;
      const ConcatString & basin()     const;
      const ConcatString & cyclone()   const;
      const StringArray  & technique() const;
      const ConcatString & initials()  const;
      unixtime             init()      const;

      int                  n_time()    const;
      int                  lead(int)   const;
      unixtime             valid(int)  const;
      double               lat(int)    const;
      double               lon(int)    const;

      DiagType             diag_source()                 const;
      const ConcatString & track_source()                const;
      const ConcatString & field_source()                const;
      int                  n_diag()                      const;
      const StringArray &  diag_name()                   const;
      bool                 has_diag(const std::string &) const;
      const NumArray &     diag_val(const std::string &) const;

         //
         //  do stuff
         //

      void read          (const ConcatString &, const ConcatString &,
                          const ConcatString &, const ConcatString &,
                          const StringArray &,
                          const std::map<ConcatString,UserFunc_1Arg> *);
      void read_cira_rt  (const ConcatString &,
                          const std::map<ConcatString,UserFunc_1Arg> *);
      void read_ships_rt (const ConcatString &,
                          const std::map<ConcatString,UserFunc_1Arg> *);

};

////////////////////////////////////////////////////////////////////////

inline const ConcatString & DiagFile::storm_id()     const { return StormId;      }
inline const ConcatString & DiagFile::basin()        const { return Basin;        }
inline const ConcatString & DiagFile::cyclone()      const { return Cyclone;      }
inline const StringArray  & DiagFile::technique()    const { return Technique;    }
inline unixtime             DiagFile::init()         const { return InitTime;     }
inline int                  DiagFile::n_time()       const { return NTime;        }
inline DiagType             DiagFile::diag_source()  const { return DiagSource;   }
inline const ConcatString & DiagFile::track_source() const { return TrackSource;  }
inline const ConcatString & DiagFile::field_source() const { return FieldSource;  }
inline int                  DiagFile::n_diag()       const { return DiagName.n(); }
inline const StringArray &  DiagFile::diag_name()    const { return DiagName;     }

////////////////////////////////////////////////////////////////////////

#endif   /*  __DIAG_FILE_H__  */

////////////////////////////////////////////////////////////////////////
