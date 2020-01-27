// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2020
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*


///////////////////////////////////////////////////////////////////////////////

#ifndef __VAR_INFO_NC_CF_H__
#define __VAR_INFO_NC_CF_H__

///////////////////////////////////////////////////////////////////////////////

#include "var_info.h"

#include "data_file_type.h"
#include "long_array.h"
#include "math_constants.h"
#include "nc_constants.h"

///////////////////////////////////////////////////////////////////////////////

class VarInfoNcCF : public VarInfo
{
   private:

      //
      // NetCDF-specific parameters
      //

      LongArray Dimension; // Dimension values for extracting 2D field

      void init_from_scratch();
      void assign(const VarInfoNcCF &);

   public:
      VarInfoNcCF();
      ~VarInfoNcCF();
      VarInfoNcCF(const VarInfoNcCF &);
      VarInfoNcCF & operator=(const VarInfoNcCF &);

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

inline GrdFileType       VarInfoNcCF::file_type()      const { return(FileType_NcCF);         }
inline const LongArray & VarInfoNcCF::dimension()      const { return(Dimension);              }
inline int               VarInfoNcCF::dimension(int i) const { return(Dimension[i]);           }
inline int               VarInfoNcCF::n_dimension()    const { return(Dimension.n_elements()); }

///////////////////////////////////////////////////////////////////////////////

#endif  // __VAR_INFO_NC_CF_H__

///////////////////////////////////////////////////////////////////////////////
