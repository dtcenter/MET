// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2025
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*

///////////////////////////////////////////////////////////////////////////////

#ifndef __VAR_INFO_NC_MET_RA_H__
#define __VAR_INFO_NC_MET_RA_H__

///////////////////////////////////////////////////////////////////////////////

#include "var_info.h"

#include "data_file_type.h"
#include "long_array.h"
#include "math_constants.h"
#include "nc_constants.h"

///////////////////////////////////////////////////////////////////////////////

// JHG, maybe inherit from VarInfoNcMet instead?

class VarInfoNcMetRA : public VarInfo
{
   private:

         //
         // NetCDF-specific parameters
         //

      LongArray Dimension; // Dimension values for extracting 2D field

      void init_from_scratch();
      void assign(const VarInfoNcMetRA &);

   public:
      VarInfoNcMetRA();
      ~VarInfoNcMetRA();
      VarInfoNcMetRA(const VarInfoNcMetRA &);
      VarInfoNcMetRA & operator=(const VarInfoNcMetRA &);
      virtual VarInfo *clone() const;

      void dump(std::ostream &) const;
      void clear();

         //
         // get stuff
         //

      GrdFileType file_type()             const;
      const       LongArray & dimension() const;
      int         dimension(int i)        const;
      int         n_dimension()           const;

         //
         // set stuff
         //

      void set_magic(const ConcatString &, const ConcatString &);
      void set_dict(Dictionary &s);

      void add_dimension(int dim);

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

inline GrdFileType       VarInfoNcMetRA::file_type()      const { return FileType_NcMet;         }
inline const LongArray & VarInfoNcMetRA::dimension()      const { return Dimension;              }
inline int               VarInfoNcMetRA::dimension(int i) const { return Dimension[i];           }
inline int               VarInfoNcMetRA::n_dimension()    const { return Dimension.n_elements(); }

///////////////////////////////////////////////////////////////////////////////

#endif  // __VAR_INFO_NC_MET_RA_H__

///////////////////////////////////////////////////////////////////////////////
