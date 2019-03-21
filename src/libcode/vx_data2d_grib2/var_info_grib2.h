// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2019
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
      int ParmCat;    // Parameter Category by Product Discipline
      int Parm;       // Parameter Number by Product Discipline and Parameter Category
      int PDTmpl;     // Product Definition Template Number (Table 4.0)
      int Process;    // Type of Generating Process (Table 4.3)
      int EnsType;    // Type of Ensemble Forecast (Table 4.6)
      int DerType;    // Derived Forecast (Table 4.7)
      int StatType;   // Statistical Processing Type (Table 4.10)

      IntArray IPDTmplIndex; // Index into the GRIB2 ipdtmpl array
      IntArray IPDTmplVal;   // Corresponding GRIB2 ipdtmpl value

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
      int         parm_cat()    const;
      int         parm()        const;
      int         pdt()         const;
      int         process()     const;
      int         ens_type()    const;
      int         der_type()    const;
      int         stat_type()   const;

      int         n_ipdtmpl()        const;
      int         ipdtmpl_index(int) const;
      int         ipdtmpl_val(int)   const;

         //
         // set stuff
         //

      void set_magic(const ConcatString &, const ConcatString &);
      void set_dict(Dictionary &);

      void set_record(int);
      void set_discipline(int);
      void set_m_table(int);
      void set_l_table(int);
      void set_parm_cat(int);
      void set_parm(int);
      void set_pdt(int);
      void set_process(int);
      void set_ens_type(int);
      void set_der_type(int);
      void set_stat_type(int);
      void set_ipdtmpl_index(const IntArray &);
      void set_ipdtmpl_val(const IntArray &);

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
inline int         VarInfoGrib2::parm_cat()   const { return(ParmCat);      }
inline int         VarInfoGrib2::parm()       const { return(Parm);         }
inline int         VarInfoGrib2::pdt()        const { return(PDTmpl);       }
inline int         VarInfoGrib2::process()    const { return(Process);      }
inline int         VarInfoGrib2::ens_type()   const { return(EnsType);      }
inline int         VarInfoGrib2::der_type()   const { return(DerType);      }
inline int         VarInfoGrib2::stat_type()  const { return(StatType);     }
inline int         VarInfoGrib2::n_ipdtmpl()  const {
                                    return(IPDTmplIndex.n()); }
inline int         VarInfoGrib2::ipdtmpl_index(int i) const {
                                    return(IPDTmplIndex[i]); }
inline int         VarInfoGrib2::ipdtmpl_val(int i) const {
                                    return(IPDTmplVal[i]); }

///////////////////////////////////////////////////////////////////////////////



///////////////////////////////////////////////////////////////////////////////

#endif  // __VAR_INFO_GRIB2_H__

///////////////////////////////////////////////////////////////////////////////
