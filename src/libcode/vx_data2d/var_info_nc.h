// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2026
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*

///////////////////////////////////////////////////////////////////////////////

#ifndef __VAR_INFO_NC_H__
#define __VAR_INFO_NC_H__

///////////////////////////////////////////////////////////////////////////////

#include "var_info.h"

///////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////

class VarInfoNC : public VarInfo
{
   protected:

      LongArray Dimension; // Dimension values for extracting 2D field
      BoolArray Is_offset; // boolean for Dimension value (true: offset, false: value to be an offset (false for value)
      NumArray  Dim_value; // Dimension values as float for extracting 2D field

      virtual void set_default_levels(const ConcatString &lstr) = 0;

      void clear_dimension();
      void parse_level(const ConcatString &level_str);
      void parse_single_level(char *lstr, bool as_offset, const char *caller);
      void parse_time_range(char *ptr_lower, char *ptr_upper, bool as_offset, const char *caller);
      void parse_vertical_range(char *ptr_lower, char *ptr_upper, bool as_offset, const char *caller);
      void set_magic_pre(const ConcatString &nstr, const ConcatString &lstr);
      void set_magic_post(const ConcatString &req_name, const ConcatString &level_name);

   public:

      virtual ~VarInfoNC() = default;

      const LongArray & dimension()      const;
      long              dimension(int i) const;
      const NumArray  & dim_value()      const;
      double            dim_value(int i) const;
      const BoolArray & is_offset()      const;
      bool              is_offset(int i) const;
      int               n_dimension()    const;

      void add_dimension(long dim, bool as_offset=true, double dim_value=bad_data_double);
      void set_dimension(int i_dim, long dim);

};

///////////////////////////////////////////////////////////////////////////////


inline const LongArray & VarInfoNC::dimension()      const { return Dimension;              }
inline long              VarInfoNC::dimension(int i) const { return (int)Dimension[i];      }
inline int               VarInfoNC::n_dimension()    const { return Dimension.n_elements(); }
inline const NumArray  & VarInfoNC::dim_value()      const { return Dim_value;              }
inline double            VarInfoNC::dim_value(int i) const { return Dim_value[i];           }
inline const BoolArray & VarInfoNC::is_offset()      const { return Is_offset;              }
inline bool              VarInfoNC::is_offset(int i) const { return Is_offset[i];           }


///////////////////////////////////////////////////////////////////////////////

#endif  // __VAR_INFO_NC_H__

///////////////////////////////////////////////////////////////////////////////
