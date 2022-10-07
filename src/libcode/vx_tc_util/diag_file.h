// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2022
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

#include "vx_cal.h"
#include "data_line.h"

////////////////////////////////////////////////////////////////////////

enum DiagFileType {
   DiagFileType_None, // Default
   LSDiagFileType,    // Large Scale Diagnostics
   TCDiagFileType     // Tropical Cyclone Diagnostics
};

////////////////////////////////////////////////////////////////////////
//
// LSDIAG files:
//   - https://ftp.nhc.noaa.gov/atcf/lsdiag
//   - Header:
//       BBCC YYMMDD HH WS LAT LON 9999 BBCCYYYY
//       BB is 2-letter basin name
//       CC is 2-digit cyclone number
//       YYMMDD HH is the initialization time
//       WS is the wind speed
//
// TCDIAG files:
//   - Add link to sample data
//   - Header:
//       * ATCF_ID YYYYMMDDHH *
//       * BBNN    BBNN       *
//       ATCF_ID is the technique (model) name
//       YYYYMMDDHH is the initialization time
//       BB is the 2-letter basin name
//       CC is the 2-digit cyclone number
//
////////////////////////////////////////////////////////////////////////

class DiagFile : public LineDataFile {

   private:

      DiagFile(const DiagFile &);
      DiagFile & operator=(const DiagFile &);

   protected:

      void init_from_scratch();

      // Diagnostics file type
      DiagFileType FileType;

      // Storm and model identification
      ConcatString StormId;
      ConcatString Basin;
      ConcatString Cyclone;
      ConcatString Technique;
      unixtime     InitTime;

      int          NTime;
      IntArray     LeadTime;
      NumArray     Lat;
      NumArray     Lon;

   public:

      DiagFile();
     ~DiagFile();

         //
         //  set stuff
         //

         //
         //  get stuff
         //

      const ConcatString & storm_id()  const;
      const ConcatString & basin()     const;
      const ConcatString & cyclone()   const;
      const ConcatString & technique() const;
      const ConcatString & initials()  const;
      unixtime             init()      const;

      int                  n_time()    const;
      int                  lead(int)   const;
      unixtime             valid(int)  const;
      double               lat(int)    const;
      double               lon(int)    const;

         //
         //  do stuff
         //

      bool open_tcdiag(const std::string &, const std::string &);
      bool open_lsdiag(const std::string &, const std::string &);

};

////////////////////////////////////////////////////////////////////////

inline const ConcatString & DiagFile::storm_id()  const { return(StormId);   }
inline const ConcatString & DiagFile::basin()     const { return(Basin);     }
inline const ConcatString & DiagFile::cyclone()   const { return(Cyclone);   }
inline const ConcatString & DiagFile::technique() const { return(Technique); }
inline unixtime             DiagFile::init()      const { return(InitTime);  }
inline int                  DiagFile::n_time()    const { return(NTime);     }

////////////////////////////////////////////////////////////////////////

#endif   /*  __DIAG_FILE_H__  */

////////////////////////////////////////////////////////////////////////
