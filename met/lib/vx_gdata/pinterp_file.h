

// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2010
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*




////////////////////////////////////////////////////////////////////////


#ifndef  __MET_PINTERP_FILE_H__
#define  __MET_PINTERP_FILE_H__


////////////////////////////////////////////////////////////////////////


#include <ostream>

#include <netcdf.hh>

#include "vx_data_grids/grid.h"
#include "vx_wrfdata/vx_wrfdata.h"
#include "vx_util/long_array.h"

#include "vx_gdata/var_info.h"


////////////////////////////////////////////////////////////////////////


class PinterpFile {

   private:

      void init_from_scratch();

      PinterpFile(const PinterpFile &);
      PinterpFile & operator=(const PinterpFile &);


   public:

      PinterpFile();
     ~PinterpFile();

      bool open(const char * filename);

      void close();

      void dump(ostream &, int = 0) const;


      NcFile * Nc;      //  allocated

         //
         //  time
         //

      int Ntimes;

      unixtime * Time;  //  allocated

      unixtime InitTime;

      unixtime valid_time (int) const;
      int      lead_time  (int) const;   //  seconds

         //
         //  dimensions
         //

      int Ndims;

      NcDim ** Dim;   //  allocated

      StringArray DimNames;

      NcDim * Xdim;   //  not allocated
      NcDim * Ydim;   //  not allocated
      NcDim * Zdim;   //  not allocated
      NcDim * Tdim;   //  not allocated

         //
         //  variables
         //

      int Nvars;

      VarInfo * Var;    //  allocated

      int PressureIndex;   //  index into Var array

         //
         //  Grid
         //

      Grid grid;

         //
         //  data
         //

      double data(NcVar *, const LongArray &) const;

      bool data(NcVar *, const LongArray &, WrfData &, double & pressure) const;

      bool data(const char *, const LongArray &, WrfData &, double & pressure) const;

};


////////////////////////////////////////////////////////////////////////


#endif   /*  __MET_PINTERP_FILE_H__  */


////////////////////////////////////////////////////////////////////////


