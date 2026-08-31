// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2026
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*


///////////////////////////////////////////////////////////////////////////////

#ifndef __VAR_INFO_NC_MET_H__
#define __VAR_INFO_NC_MET_H__

///////////////////////////////////////////////////////////////////////////////

#include "var_info_nc.h"

#include "data_file_type.h"
#include "long_array.h"
#include "math_constants.h"
#include "nc_constants.h"

///////////////////////////////////////////////////////////////////////////////

class VarInfoNcMet : public VarInfoNC
{
   private:

         //
         // NetCDF-specific parameters
         //

      void init_from_scratch();
      void assign(const VarInfoNcMet &);

   protected:
      void set_default_levels(const ConcatString &lstr) override;

   public:
      VarInfoNcMet();
      ~VarInfoNcMet() override;
      VarInfoNcMet(const VarInfoNcMet &);
      VarInfoNcMet & operator=(const VarInfoNcMet &);
      std::unique_ptr<VarInfo> clone() const override;

      void dump(std::ostream &) const override;
      void clear();

         //
         // get stuff
         //

      GrdFileType file_type()             const override;

         //
         // set stuff
         //

      void set_magic(const ConcatString &, const ConcatString &) override;
      bool set_dict(Dictionary &s, bool do_exit=true) override;

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

inline GrdFileType       VarInfoNcMet::file_type()      const { return FileType_NcMet;         }

///////////////////////////////////////////////////////////////////////////////

#endif  // __VAR_INFO_NC_MET_H__

///////////////////////////////////////////////////////////////////////////////
