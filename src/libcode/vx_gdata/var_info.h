

// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2010
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*




////////////////////////////////////////////////////////////////////////


#ifndef  __PINTERP_VAR_INFO_H__
#define  __PINTERP_VAR_INFO_H__


////////////////////////////////////////////////////////////////////////


#include <netcdf.hh>

#include "vx_util.h"


////////////////////////////////////////////////////////////////////////


class VarInfo {

   private:

      void init_from_scratch();

      void assign(const VarInfo &);

   public:

      VarInfo();
     ~VarInfo();
      VarInfo(const VarInfo &);
      VarInfo & operator=(const VarInfo &);

      void clear();

      void dump(ostream &, int = 0) const;


      NcVar * var;   //  not allocated

      ConcatString name;

      ConcatString level;   // nul if N/A

      ConcatString units;   // nul if N/A

      int AccumTime;        // seconds

      int Ndims;

      NcDim ** Dims; //  allocated

      int x_slot;    //   starting from zero
      int y_slot;    //
      int z_slot;    //   -1 if not defined
      int t_slot;    //

};


////////////////////////////////////////////////////////////////////////


#endif   /*  __PINTERP_VAR_INFO_H__  */


////////////////////////////////////////////////////////////////////////


