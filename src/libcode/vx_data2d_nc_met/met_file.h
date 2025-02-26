

// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2025
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*




////////////////////////////////////////////////////////////////////////


#ifndef  __MET_FILE_H__
#define  __MET_FILE_H__


////////////////////////////////////////////////////////////////////////


#include <ostream>

#include "vx_grid.h"
#include "data_plane.h"
#include "long_array.h"
#include "nc_var_info.h"


////////////////////////////////////////////////////////////////////////


static const std::string nc_met_lat_name     = "lat";
static const std::string nc_met_lon_name     = "lon";
static const std::string nc_met_range_name   = "range";
static const std::string nc_met_azimuth_name = "azimuth";


////////////////////////////////////////////////////////////////////////


class MetNcFile {

   private:

      void init_from_scratch();

      MetNcFile(const MetNcFile &);
      MetNcFile & operator=(const MetNcFile &);

   public:

      MetNcFile();
     ~MetNcFile();

      bool open(const char * filename);

      void close();

      void dump(std::ostream &, int = 0) const;

      netCDF::NcFile * Nc;    //  allocated

         //
         //  dimensions
         //

      int Ndims;

      netCDF::NcDim ** Dim;   //  allocated

      StringArray DimNames;

      netCDF::NcDim * Xdim;   //  not allocated
      netCDF::NcDim * Ydim;   //  not allocated

         //
         //  variables
         //

      int Nvars;

      NcVarInfo * Var;    //  allocated

         //
         //  Grid
         //

      Grid grid;

         //
         //  data
         //

      bool is_range_azimuth() const;

      double data(netCDF::NcVar *, const LongArray &) const;

      bool data(netCDF::NcVar *, const LongArray &, DataPlane &) const;

      bool data(const char *, const LongArray &, DataPlane &, NcVarInfo *&) const;

      NcVarInfo* find_var_name(const char * var_name) const;

};

////////////////////////////////////////////////////////////////////////

inline bool MetNcFile::is_range_azimuth() const { return grid.is_set() && grid.info().tc; }

////////////////////////////////////////////////////////////////////////

extern bool is_ncmet_range_azimuth_file(netCDF::NcFile *);

////////////////////////////////////////////////////////////////////////

#include "met_file.hpp"

////////////////////////////////////////////////////////////////////////

#endif   /*  __MET_FILE_H__  */


////////////////////////////////////////////////////////////////////////

