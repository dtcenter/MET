// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2019
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*


///////////////////////////////////////////////////////////////////////////////

#ifndef __VAR_INFO_GRIB_H__
#define __VAR_INFO_GRIB_H__

///////////////////////////////////////////////////////////////////////////////

#include "var_info.h"
#include "vx_config.h"

#include "data_file_type.h"

///////////////////////////////////////////////////////////////////////////////
//
//  GRIB1 keywords expected to be used in the configuration file.
//
///////////////////////////////////////////////////////////////////////////////

// GRIB version 1 keywords
static const char * const CONFIG_GRIB_PTV     = "GRIB_PTV";
static const char * const CONFIG_GRIB_Code    = "GRIB_Code";
static const char * const CONFIG_GRIB_LvlType = "GRIB_Level";
static const char * const CONFIG_GRIB_PCode   = "GRIB_Prob_Code";

///////////////////////////////////////////////////////////////////////////////



///////////////////////////////////////////////////////////////////////////////

class VarInfoGrib : public VarInfo
{
   private:

         //
         // GRIB1-specific parameters
         //   http://www.nco.ncep.noaa.gov/pmb/docs/on388
         //

      int PTV;      // Parameter table version number
      int Code;     // Parameter number from 0 to 255
      int LvlType;  // Level type from 0 to 201

      int PCode;    // Parameter number for probability from 0 to 255

      int Center;    // Identification of center
      int Subcenter; // Identification of subsenter

      int FieldRec;


      void init_from_scratch();
      void assign(const VarInfoGrib &);

   public:
      VarInfoGrib();
      ~VarInfoGrib();
      VarInfoGrib(const VarInfoGrib &);
      VarInfoGrib & operator=(const VarInfoGrib &);

      void dump(ostream &) const;
      void clear();

         //
         // get stuff
         //

      GrdFileType file_type() const;
      int         ptv()       const;
      int         code()      const;
      int         lvl_type()  const;
      int         p_code()    const;
      int         center()    const;
      int         subcenter()    const;
      int         field_rec()    const;

         //
         // set stuff
         //

      void set_magic(const ConcatString &, const ConcatString &);
      void set_dict(Dictionary &);
      void add_grib_code(Dictionary &);

      void set_ptv(int);
      void set_code(int);
      void set_lvl_type(int);
      void set_p_code(int);
      void set_center(int);
      void set_subcenter(int);
      void set_field_rec(int);

         //
         // do stuff
         //

      bool is_precipitation()     const;
      bool is_specific_humidity() const;
      bool is_u_wind()            const;
      bool is_v_wind()            const;
      bool is_wind_speed()        const;
      bool is_wind_direction()    const;
};

///////////////////////////////////////////////////////////////////////////////

inline GrdFileType VarInfoGrib::file_type() const { return(FileType_Gb1); }
inline int         VarInfoGrib::ptv()       const { return(PTV);          }
inline int         VarInfoGrib::code()      const { return(Code);         }
inline int         VarInfoGrib::lvl_type()  const { return(LvlType);      }
inline int         VarInfoGrib::p_code()    const { return(PCode);        }
inline int         VarInfoGrib::center()    const { return(Center);        }
inline int         VarInfoGrib::subcenter()    const { return(Subcenter);        }
inline int         VarInfoGrib::field_rec()    const { return(FieldRec);        }

///////////////////////////////////////////////////////////////////////////////

#endif  // __VAR_INFO_GRIB_H__

///////////////////////////////////////////////////////////////////////////////
