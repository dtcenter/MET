

// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2019
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*




////////////////////////////////////////////////////////////////////////


#ifndef  __NC_VAR_INFO_H__
#define  __NC_VAR_INFO_H__


////////////////////////////////////////////////////////////////////////


#include <netcdf>
using namespace netCDF;

#include "vx_util.h"

extern unixtime  get_att_value_unixtime(const NcAtt *);

////////////////////////////////////////////////////////////////////////


class NcVarInfo {

   private:

      void init_from_scratch();

      void assign(const NcVarInfo &);

   public:

      NcVarInfo();
     ~NcVarInfo();
      NcVarInfo(const NcVarInfo &);
      NcVarInfo & operator=(const NcVarInfo &);

      void clear();

      void dump(ostream &, int = 0) const;


      NcVar * var;   //  not allocated

      ConcatString name;

      ConcatString name_att;

      ConcatString long_name_att;
      
      ConcatString level_att;

      ConcatString units_att;

      int AccumTime; // seconds

      int Ndims;

      NcDim ** Dims; //  allocated

      int x_slot;    //   starting from zero
      int y_slot;    //
      int z_slot;    //   -1 if not defined
      int t_slot;    //

};

////////////////////////////////////////////////////////////////////////



extern bool get_att_str(const NcVarInfo &, const ConcatString, ConcatString &);

extern bool get_att_int(const NcVarInfo &, const ConcatString, int &);

   //  unixtimes could be ints or strings

extern bool get_att_unixtime(const NcVarInfo &, const ConcatString, unixtime &);


////////////////////////////////////////////////////////////////////////


#endif   /*  __NC_VAR_INFO_H__  */


////////////////////////////////////////////////////////////////////////


