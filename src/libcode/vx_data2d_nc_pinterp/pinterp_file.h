

// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2022
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

#include <netcdf>

#include "vx_grid.h"
#include "data_plane.h"
#include "long_array.h"
#include "nc_var_info.h"


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

      void dump(std::ostream &, int = 0) const;


      netCDF::NcFile * Nc;      //  allocated

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

      netCDF::NcDim ** Dim;   //  allocated

      StringArray DimNames;

      netCDF::NcDim * Xdim;   //  not allocated
      netCDF::NcDim * Ydim;   //  not allocated
      netCDF::NcDim * Zdim;   //  not allocated
      netCDF::NcDim * Tdim;   //  not allocated

         //
         //  variables
         //

      int Nvars;

      NcVarInfo * Var;     //  allocated

      int PressureIndex;   //  index into Var array

      double hPaCF;        //  pressure to hPa conversion factor

         //
         //  Grid
         //

      Grid grid;

         //
         //  data
         //

      double data(netCDF::NcVar *, const LongArray &) const;

      bool data(netCDF::NcVar *, const LongArray &, DataPlane &, double & pressure) const;

      bool data(const char *, const LongArray &, DataPlane &,
                double & pressure, NcVarInfo *&) const;

      bool get_nc_var_info(const char *var_name, NcVarInfo *&info) const;
};


////////////////////////////////////////////////////////////////////////


#endif   /*  __MET_PINTERP_FILE_H__  */


////////////////////////////////////////////////////////////////////////


