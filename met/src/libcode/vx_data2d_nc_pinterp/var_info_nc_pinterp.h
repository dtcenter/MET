// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2020
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*


///////////////////////////////////////////////////////////////////////////////

#ifndef __VAR_INFO_NC_PINTERP_H__
#define __VAR_INFO_NC_PINTERP_H__

///////////////////////////////////////////////////////////////////////////////

#include "var_info.h"

#include "data_file_type.h"
#include "long_array.h"
#include "math_constants.h"
#include "nc_constants.h"

///////////////////////////////////////////////////////////////////////////////

//
// List of Pinterp precipitation variable names
// Taken from the WRF version 3.2 Registry.EM file
//

static const char *pinterp_precipitation_names[] = {
   "TS_RAINC",    // Cumulus precip
   "TS_RAINNC",   // Grid-scale precip
   "RAINC",       // ACCUMULATED TOTAL CUMULUS PRECIPITATION, mm
   "RAINNC",      // ACCUMULATED TOTAL GRID SCALE PRECIPITATION, mm
   "PRATEC",      // PRECIP RATE FROM CUMULUS SCHEME, mm s-1
   "RAINCV",      // TIME-STEP CUMULUS PRECIPITATION, mm
   "RAINNCV",     // TIME-STEP NONCONVECTIVE PRECIPITATION, mm
   "RAINBL",      // PBL TIME-STEP TOTAL PRECIPITATION, mm
   "APR_GR",      // PRECIP FROM CLOSURE OLD_GRELL, mm hour-1
   "APR_W",       // PRECIP FROM CLOSURE W, mm hour-1
   "APR_MC",      // PRECIP FROM CLOSURE KRISH MV, mm hour-1
   "APR_ST",      // PRECIP FROM CLOSURE STABILITY, mm hour-1
   "APR_AS",      // PRECIP FROM CLOSURE AS-TYPE, mm hour-1
   "APR_CAPMA",   // PRECIP FROM MAX CAP, mm hour-1
   "APR_CAPME",   // PRECIP FROM MEAN CAP, mm hour-1
   "APR_CAPMI",   // PRECIP FROM MIN CAP, mm hour-1
   "PR_ENS",      // PRECIP RATE PDF IN GRELL CUMULUS SCHEME, mb hour-1
   "CLDEFI",      // precipitation efficiency in BMJ SCHEME
   "sr",          // fraction of frozen precipitation
   "prec_acc_c",  // ACCUMULATED CUMULUS PRECIPITATION OVER prec_acc_dt PERIODS OF TIME, mm
   "prec_acc_nc", // ACCUMULATED GRID SCALE PRECIPITATION OVER prec_acc_dt PERIODS OF TIME, mm
   "prec_acc_dt", // bucket reset time interval between outputs for cumulus or grid scale precipitation, minutes
   "prec_acc_opt" // option to output precip in a time window
};

//
// Number of Pinterp precipitation variable names
//

static const int n_pinterp_precipitation_names =
                     sizeof(pinterp_precipitation_names)/
                    sizeof(*pinterp_precipitation_names);

///////////////////////////////////////////////////////////////////////////////

//
// List of Pinterp specific humidity variable names
// Taken from the WRF version 3.2 Registry.EM file
//

static const char *pinterp_specific_humidity_names[] = {
   "SPECHUMD",   // Specific humidity, kg kg-1
   "QLEV_URB3D", // SPECIFIC HUMIDITY, dimensionless
   "QZ0",        // SPECIFIC HUMIDITY AT ZNT, kg kg-1
   "QSFC",       // SPECIFIC HUMIDITY AT LOWER BOUNDARY, kg kg-1
   "QSHLTR",     // SHELTER SPECIFIC HUMIDITY FROM MYJ, kg kg-1
   "Q10"         // 10-M SPECIFIC HUMIDITY FROM MYJ, kg kg-1
};

//
// Number of Pinterp specific humidity variable names
//

static const int n_pinterp_specific_humidity_names =
                     sizeof(pinterp_specific_humidity_names)/
                    sizeof(*pinterp_specific_humidity_names);

///////////////////////////////////////////////////////////////////////////////

//
// List of U-component of wind variable names
// Taken from the WRF version 3.2 Registry.EM file
//

static const char *pinterp_u_wind_names[] = {
   "UU",         // x-wind component, m s-1
   "TS_U",       // Surface wind U-component, earth-relative
   "UZ0"         // U WIND COMPONENT AT ZNT, m s-1
};

//
// Number of Pinterp U-component of wind variable names
//

static const int n_pinterp_u_wind_names =
                     sizeof(pinterp_u_wind_names)/
                    sizeof(*pinterp_u_wind_names);

///////////////////////////////////////////////////////////////////////////////

//
// List of V-component of wind variable names
// Taken from the WRF version 3.2 Registry.EM file
//

static const char *pinterp_v_wind_names[] = {
   "VV",         // y-wind component, m s-1
   "TS_V",       // Surface wind V-component, earth-relative
   "VZ0"         // V WIND COMPONENT AT ZNT, m s-1
};

//
// Number of Pinterp V-component of wind variable names
//

static const int n_pinterp_v_wind_names =
                     sizeof(pinterp_v_wind_names)/
                    sizeof(*pinterp_v_wind_names);

///////////////////////////////////////////////////////////////////////////////

//
// List of wind variable names that should be rotated from grid-relative
// to earth-relative.  MET is not able to read winds from Pinterp files since
// they are defined on a staggered grid.  If the code is enhanced to do so,
// the data in these variables should be rotated from grid-relative to
// earth-relative prior to verifying.
// Taken from the WRF version 3.2 Registry.EM file
//

static const char *pinterp_grid_relative_names[] = {
   "UU",         // x-wind component, m s-1
   "UZ0",        // U WIND COMPONENT AT ZNT, m s-1
   "VV",         // y-wind component, m s-1
   "VZ0"         // V WIND COMPONENT AT ZNT, m s-1
};

//
// Number of Pinterp grid relative variable names
//

static const int n_pinterp_grid_relative_names =
                     sizeof(pinterp_grid_relative_names)/
                    sizeof(*pinterp_grid_relative_names);

///////////////////////////////////////////////////////////////////////////////

//
// List of wind speed variable names
// Taken from the WRF version 3.2 Registry.EM file
//

static const char *pinterp_wind_speed_names[] = {
   "WSPD"        // Wind speed, m s-1
};

//
// Number of Pinterp wind speed variable names
//

static const int n_pinterp_wind_speed_names =
                     sizeof(pinterp_wind_speed_names)/
                    sizeof(*pinterp_wind_speed_names);

///////////////////////////////////////////////////////////////////////////////

class VarInfoNcPinterp : public VarInfo
{
   private:

         //
         // NetCDF-specific parameters
         //

      LongArray Dimension; // Dimension values for extracting 2D field

      void init_from_scratch();
      void assign(const VarInfoNcPinterp &);

   public:
      VarInfoNcPinterp();
      ~VarInfoNcPinterp();
      VarInfoNcPinterp(const VarInfoNcPinterp &);
      VarInfoNcPinterp & operator=(const VarInfoNcPinterp &);

      void dump(ostream &) const;
      void clear();

         //
         // get stuff
         //

      GrdFileType file_type() const;
      const LongArray & dimension() const;
      int dimension(int i) const;
      int n_dimension() const;

         //
         // set stuff
         //

      void set_magic(const ConcatString &, const ConcatString &);
      void set_dict(Dictionary &);

      void add_dimension(int dim);
      void set_dimension(int i_dim, int dim);

         //
         // do stuff
         //

      bool is_precipitation()     const;
      bool is_specific_humidity() const;
      bool is_u_wind()            const;
      bool is_v_wind()            const;
      bool is_wind_speed()        const;
      bool is_wind_direction()    const;
      bool is_grid_relative()     const;
};

///////////////////////////////////////////////////////////////////////////////

inline GrdFileType       VarInfoNcPinterp::file_type()      const { return(FileType_NcPinterp);     }
inline const LongArray & VarInfoNcPinterp::dimension()      const { return(Dimension);              }
inline int               VarInfoNcPinterp::dimension(int i) const { return(Dimension[i]);           }
inline int               VarInfoNcPinterp::n_dimension()    const { return(Dimension.n_elements()); }

///////////////////////////////////////////////////////////////////////////////

#endif  // __VAR_INFO_NC_PINTERP_H__

///////////////////////////////////////////////////////////////////////////////
