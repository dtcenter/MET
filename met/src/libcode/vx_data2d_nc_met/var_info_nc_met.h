// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2020
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*


///////////////////////////////////////////////////////////////////////////////

#ifndef __VAR_INFO_NC_MET_H__
#define __VAR_INFO_NC_MET_H__

///////////////////////////////////////////////////////////////////////////////

#include "var_info.h"

#include "data_file_type.h"
#include "long_array.h"
#include "math_constants.h"
#include "nc_constants.h"

///////////////////////////////////////////////////////////////////////////////

class VarInfoNcMet : public VarInfo
{
   private:

         //
         // NetCDF-specific parameters
         //

      LongArray Dimension; // Dimension values for extracting 2D field

      void init_from_scratch();
      void assign(const VarInfoNcMet &);

   public:
      VarInfoNcMet();
      ~VarInfoNcMet();
      VarInfoNcMet(const VarInfoNcMet &);
      VarInfoNcMet & operator=(const VarInfoNcMet &);

      void dump(ostream &) const;
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

inline GrdFileType       VarInfoNcMet::file_type()      const { return(FileType_NcMet);         }
inline const LongArray & VarInfoNcMet::dimension()      const { return(Dimension);              }
inline int               VarInfoNcMet::dimension(int i) const { return(Dimension[i]);           }
inline int               VarInfoNcMet::n_dimension()    const { return(Dimension.n_elements()); }

///////////////////////////////////////////////////////////////////////////////

#endif  // __VAR_INFO_NC_MET_H__

///////////////////////////////////////////////////////////////////////////////
