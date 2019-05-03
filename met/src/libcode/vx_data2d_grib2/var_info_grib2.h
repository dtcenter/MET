// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2016
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*


///////////////////////////////////////////////////////////////////////////////

#ifndef __VAR_INFO_GRIB2_H__
#define __VAR_INFO_GRIB2_H__

///////////////////////////////////////////////////////////////////////////////

#include <stdlib.h>
#include <string.h>
#include <map>
#include <vector>

using namespace std;

#include "vx_config.h"
#include "vx_util.h"
#include "var_info.h"
#include "data_file_type.h"

///////////////////////////////////////////////////////////////////////////////
//
//  GRIB2 keywords expected to be used in the configuration file.
//
///////////////////////////////////////////////////////////////////////////////

// GRIB version 2 keywords
static const char * const CONFIG_GRIB2_Discipline = "GRIB2_Discipline";
static const char * const CONFIG_GRIB2_MTable     = "GRIB2_MTable";
static const char * const CONFIG_GRIB2_LTable     = "GRIB2_LTable";
static const char * const CONFIG_GRIB2_Tmpl       = "GRIB2_Tmpl";
static const char * const CONFIG_GRIB2_ParmCat    = "GRIB2_ParmCat";
static const char * const CONFIG_GRIB2_Parm       = "GRIB2_Parm";
static const char * const CONFIG_GRIB2_Process    = "GRIB2_Process";
static const char * const CONFIG_GRIB2_EnsType    = "GRIB2_EnsType";
static const char * const CONFIG_GRIB2_DerType    = "GRIB2_DerType";


///////////////////////////////////////////////////////////////////////////////

class VarInfoGrib2 : public VarInfo
{
   private:

         //
         // GRIB2-specific parameters
         //   http://www.nco.ncep.noaa.gov/pmb/docs/grib2/grib2_doc.shtml
         //

      // QUESTION: How many of these are necessary for filtering input records?

      int Record;     // Record number of interest

      // Section 0
      int Discipline; // Discipline of Processed Data

      // Section 1
      int MTable;     // GRIB Master Tables Version Number
      int LTable;     // GRIB Local Tables Version Number

      // Section 4
      int Tmpl;       // Product Definition Template Number
      int ParmCat;    // Parameter Category by Product Discipline
      int Parm;       // Parameter Number by Product Discipline and Parameter Category
      int Process;    // Type of Generating Process
      int EnsType;    // Type of Ensemble Forecast
      int DerType;    // Derived Forecast

      void init_from_scratch();
      void assign(const VarInfoGrib2 &);

   public:
      VarInfoGrib2();
      ~VarInfoGrib2();
      VarInfoGrib2(const VarInfoGrib2 &);
      VarInfoGrib2 & operator=(const VarInfoGrib2 &);

      void dump(ostream &) const;
      void clear();

         //
         // get stuff
         //

      GrdFileType file_type()   const;
      int         record()      const;
      int         discipline()  const;
      int         m_table()     const;
      int         l_table()     const;
      int         tmpl()        const;
      int         parm_cat()    const;
      int         parm()        const;
      int         process()     const;
      int         ens_type()    const;
      int         der_type()    const;

         //
         // set stuff
         //

      void set_magic(const ConcatString &);
      void set_dict(Dictionary &);

      void set_record(int);
      void set_discipline(int);
      void set_m_table(int);
      void set_l_table(int);
      void set_tmpl(int);
      void set_parm_cat(int);
      void set_parm(int);
      void set_process(int);
      void set_ens_type(int);
      void set_der_type(int);

         //
         // do stuff
         //

      bool is_precipitation()     const;
      bool is_specific_humidity() const;
      bool is_u_wind()            const;
      bool is_v_wind()            const;
      bool is_wind_speed()        const;
      bool is_wind_direction()    const;

      static LevelType g2_lty_to_level_type(int lt);
      static double    g2_time_range_unit_to_sec(int ind);

};

///////////////////////////////////////////////////////////////////////////////

inline GrdFileType VarInfoGrib2::file_type()  const { return(FileType_Gb2); }
inline int         VarInfoGrib2::record()     const { return(Record);       }
inline int         VarInfoGrib2::discipline() const { return(Discipline);   }
inline int         VarInfoGrib2::m_table()    const { return(MTable);       }
inline int         VarInfoGrib2::l_table()    const { return(LTable);       }
inline int         VarInfoGrib2::tmpl()       const { return(Tmpl);         }
inline int         VarInfoGrib2::parm_cat()   const { return(ParmCat);      }
inline int         VarInfoGrib2::parm()       const { return(Parm);         }
inline int         VarInfoGrib2::process()    const { return(Process);      }
inline int         VarInfoGrib2::ens_type()   const { return(EnsType);      }
inline int         VarInfoGrib2::der_type()   const { return(DerType);      }

////////////////////////////////////////////////////////////////////////



///////////////////////////////////////////////////////////////////////////////

#endif  // __VAR_INFO_GRIB2_H__

///////////////////////////////////////////////////////////////////////////////
