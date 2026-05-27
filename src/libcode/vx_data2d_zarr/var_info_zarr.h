// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2026
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*

///////////////////////////////////////////////////////////////////////////////

#ifndef __VAR_INFO_ZARR_H__
#define __VAR_INFO_ZARR_H__

///////////////////////////////////////////////////////////////////////////////

#include <stdlib.h>
#include <string.h>
#include <map>
#include <vector>

#include "vx_config.h"
#include "vx_util.h"
#include "var_info.h"
#include "data_file_type.h"

///////////////////////////////////////////////////////////////////////////////

class VarInfoZarr : public VarInfo
{
   private:

      void init_from_scratch();
      void assign(const VarInfoZarr &);

   public:
      VarInfoZarr();
      ~VarInfoZarr() override;
      VarInfoZarr(const VarInfoZarr &);
      VarInfoZarr & operator=(const VarInfoZarr &);
      VarInfo *clone() const override;

      void dump(std::ostream &) const override;
      void clear();

         //
         // get stuff
         //

      GrdFileType file_type()   const override;

         //
         // set stuff
         //

      bool set_dict(Dictionary &, bool do_exit=true) override;

         //
         // do stuff
         //

      bool is_precipitation()     const override;
      bool is_specific_humidity() const override;
      bool is_u_wind()            const override;
      bool is_v_wind()            const override;
      bool is_wind_speed()        const override;
      bool is_wind_direction()    const override;

};

///////////////////////////////////////////////////////////////////////////////

inline GrdFileType VarInfoZarr::file_type()  const { return FileType_Zarr; }

///////////////////////////////////////////////////////////////////////////////

#endif  // __VAR_INFO_ZARR_H__

///////////////////////////////////////////////////////////////////////////////
