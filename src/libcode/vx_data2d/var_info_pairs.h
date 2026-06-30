// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2026
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*


///////////////////////////////////////////////////////////////////////////////

#ifndef __VAR_INFO_PAIRS_H__
#define __VAR_INFO_PAIRS_H__

///////////////////////////////////////////////////////////////////////////////

#include "var_info.h"
#include "vx_config.h"

#include "data_file_type.h"

///////////////////////////////////////////////////////////////////////////////

class VarInfoPairs : public VarInfo {

   private:

      void init_from_scratch();
      void assign(const VarInfoPairs &);

   public:
      VarInfoPairs();
      ~VarInfoPairs();
      VarInfoPairs(const VarInfoPairs &);
      VarInfoPairs & operator=(const VarInfoPairs &);
      std::unique_ptr<VarInfo> clone() const override;

      void dump(std::ostream &) const;
      void clear();

         //
         // get stuff
         //

      GrdFileType file_type() const;

         //
         // set stuff
         //

      bool set_dict(Dictionary &, bool do_exit=true) override;

         //
         // do stuff
         //
};

///////////////////////////////////////////////////////////////////////////////

inline GrdFileType VarInfoPairs::file_type() const { return FileType_Pairs; }

///////////////////////////////////////////////////////////////////////////////

#endif  // __VAR_INFO_PAIRS_H__

///////////////////////////////////////////////////////////////////////////////
