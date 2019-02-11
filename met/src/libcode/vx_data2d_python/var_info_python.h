// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2019
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*


///////////////////////////////////////////////////////////////////////////////

#ifndef __VAR_INFO_PYTHON_H__
#define __VAR_INFO_PYTHON_H__

///////////////////////////////////////////////////////////////////////////////

#include "var_info.h"
#include "vx_config.h"

#include "data_file_type.h"

///////////////////////////////////////////////////////////////////////////////


class VarInfoPython : public VarInfo

{

   private:

      GrdFileType Type;

      void init_from_scratch();
      void assign(const VarInfoPython &);

   public:
      VarInfoPython();
      ~VarInfoPython();
      VarInfoPython(const VarInfoPython &);
      VarInfoPython & operator=(const VarInfoPython &);

      void dump(ostream &) const;
      void clear();

         //
         // get stuff
         //

      GrdFileType file_type() const;

         //
         // set stuff
         //

      void set_file_type(const GrdFileType);
      void set_magic(const ConcatString &, const ConcatString &);
      void set_dict(Dictionary &);

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

inline GrdFileType VarInfoPython::file_type() const { return(Type); }

inline bool VarInfoPython::is_precipitation()     const { return ( false ); }
inline bool VarInfoPython::is_specific_humidity() const { return ( false ); }
inline bool VarInfoPython::is_u_wind()            const { return ( false ); }
inline bool VarInfoPython::is_v_wind()            const { return ( false ); }
inline bool VarInfoPython::is_wind_speed()        const { return ( false ); }
inline bool VarInfoPython::is_wind_direction()    const { return ( false ); }

///////////////////////////////////////////////////////////////////////////////

#endif  // __VAR_INFO_PYTHON_H__

///////////////////////////////////////////////////////////////////////////////
