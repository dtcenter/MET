

// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2020
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*




////////////////////////////////////////////////////////////////////////


#ifndef  __NCCF_FILE_H__
#define  __NCCF_FILE_H__


////////////////////////////////////////////////////////////////////////


#include <ostream>

#include <netcdf>
using namespace netCDF;

#include "vx_grid.h"
#include "nc_utils.h"
#include "data_plane.h"
#include "long_array.h"
#include "nc_var_info.h"


////////////////////////////////////////////////////////////////////////


static const char nccf_lat_var_name [] = "lat";
static const char nccf_lon_var_name [] = "lon";


////////////////////////////////////////////////////////////////////////


class NcCfFile {

   public:

      NcCfFile();
     ~NcCfFile();

      bool open(const char * filename);

      void close();

      void dump(ostream &, int = 0) const;


      int getNx() const
      {
        if (_xDim == 0)
          return 0;

        return GET_NC_SIZE_P(_xDim);
      }
      
      int getNy() const
      {
        if (_yDim == 0)
          return 0;

        return GET_NC_SIZE_P(_yDim);
      }
      
      NcVarInfo *get_time_var_info() const { return _time_var_info; }
      
         //
         //  time
         //

      TimeArray ValidTime;

      unixtime  InitTime;

      int       lead_time () const;   //  seconds


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

      double getData(NcVar *, const LongArray &) const;

      bool getData(NcVar *, const LongArray &, DataPlane &) const;

      bool getData(const char *, const LongArray &, DataPlane &, NcVarInfo *&) const;

      NcVarInfo* find_var_name(const char * var_name) const;

   private:

      static const double DELTA_TOLERANCE;
      
      NcFile * _ncFile;      //  allocated

         //
         //  dimensions
         //

      int _numDims;

      NcDim ** _dims;   //  allocated

      StringArray _dimNames;

      // Pointers to the X/Y and time dimensions and the associated coordinate
      // variables.  Note that these are pointers into the _dims and Var
      // arrays so should not be deleted.

      NcDim *_xDim;
      NcDim *_yDim;
      NcDim *_tDim;

      NcVar *_xCoordVar;
      NcVar *_yCoordVar;
      NcVarInfo *_time_var_info;
      
      void init_from_scratch();

      NcCfFile(const NcCfFile &);
      NcCfFile & operator=(const NcCfFile &);

      // Determine the file times from the filename

      unixtime get_valid_time_from_file_path(const string &filepath) const;
      unixtime get_init_time_from_file_path(const string &filepath) const;
      unixtime get_time_from_TRMM_3B42_3hourly_filename(const string &filename) const;
      unixtime get_time_from_TRMM_3B42_daily_filename(const string &filename) const;


      // Read the grid information from the netCDF file and fill in the
      // grid member with that information.

      void read_netcdf_grid();
      void get_grid_from_grid_mapping(const NcVarAtt *grid_mapping_att);
      
      void get_grid_mapping_albers_conical_equal_area(const NcVar *grid_mapping_var);
      void get_grid_mapping_azimuthal_equidistant(const NcVar *grid_mapping_var);
      void get_grid_mapping_lambert_azimuthal_equal_area(const NcVar *grid_mapping_var);
      void get_grid_mapping_lambert_conformal_conic(const NcVar *grid_mapping_var);
      void get_grid_mapping_lambert_cylindrical_equal_area(const NcVar *grid_mapping_var);
      void get_grid_mapping_latitude_longitude(const NcVar *grid_mapping_var);
      void get_grid_mapping_mercator(const NcVar *grid_mapping_var);
      void get_grid_mapping_orthographic(const NcVar *grid_mapping_var);
      void get_grid_mapping_polar_stereographic(const NcVar *grid_mapping_var);
      void get_grid_mapping_rotated_latitude_longitude(const NcVar *grid_mapping_var);
      void get_grid_mapping_stereographic(const NcVar *grid_mapping_var);
      void get_grid_mapping_transverse_mercator(const NcVar *grid_mapping_var);
      void get_grid_mapping_vertical_perspective(const NcVar *grid_mapping_var);
      void get_grid_mapping_geostationary(const NcVar *grid_mapping_var);
      
      bool get_grid_from_coordinates(const NcVar *data_var);
      bool get_grid_from_dimensions();

};


////////////////////////////////////////////////////////////////////////


extern void parse_cf_time_string(const char *, unixtime &ref_ut, int &sec_per_unit);


////////////////////////////////////////////////////////////////////////


#endif   /*  __NCCF_FILE_H__  */


////////////////////////////////////////////////////////////////////////


