// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2023
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*


////////////////////////////////////////////////////////////////////////


#ifndef  __UGRID_FILE_H__
#define  __UGRID_FILE_H__


////////////////////////////////////////////////////////////////////////


#include <ostream>

#include "vx_grid.h"
#include "nc_utils.h"
#include "data_plane.h"
#include "long_array.h"
#include "nc_var_info.h"


////////////////////////////////////////////////////////////////////////


static const int UG_DIM_COUNT = 5;
static const int UG_META_VAR_COUNT = 9;


////////////////////////////////////////////////////////////////////////


class UGridFile {

   public:

      UGridFile();
     ~UGridFile();

      bool open(const char *filename);
      bool open_metadata(const char *filename);
      bool get_var_info();

      ConcatString coordinate_nc();
      void set_dataset(ConcatString _dataset_name);
      void set_map_config_file(ConcatString filename);
      void set_max_distance_km(double max_distance);

      void close();

      void dump(std::ostream &, int = 0) const;


      int getNx() const {
        return (_faceDim == nullptr) ? 0 : GET_NC_SIZE_P(_faceDim);
      }
      
      int getNy() const {
        return 1;
      }
      
      NcVarInfo *get_time_var_info() const { return _time_var_info; }
      
         //
         //  time
         //

      TimeArray ValidTime;
      NumArray raw_times;
      NumArray vlevels;

      unixtime  InitTime;
      unixtime  AccumTime;

      int       lead_time () const;   //  seconds


         //
         //  variables
         //

      int Nvars;

      NcVarInfo *Var;    //  allocated
      std::array<NcVarInfo, UG_META_VAR_COUNT>MetaVar;

         //
         //  Grid
         //

      Grid grid;
      UnstructuredData grid_data;

         //
         //  data
         //

      double getData(netCDF::NcVar *, const LongArray &) const;

      bool getData(netCDF::NcVar *, const LongArray &, DataPlane &) const;

      bool getData(const char *, const LongArray &, DataPlane &, NcVarInfo *&) const;

      netCDF::NcDim *get_vert_dim() const;
      NcVarInfo* find_by_name(const char * var_name) const;
      NcVarInfo* find_var_by_dim_name(const char *dim_name) const;
      bool find_nc_vinfo_list(const char *var_name, std::vector<NcVarInfo *> &vinfo_list) const;


   private:

      static const double DELTA_TOLERANCE;
      
      netCDF::NcFile * _ncFile;      //  allocated
      netCDF::NcFile * _ncMetaFile;  //  allocated

      ConcatString dataset_name;
      ConcatString coordinate_file;
      StringArray metadata_names;
      std::map<ConcatString,StringArray> metadata_map;
      double max_distance_km;
         //
         //  dimensions
         //

      int _numDims;

      //std::array<netCDF::NcDim *, UG_DIM_COUNT> _dims;   //  allocated

      StringArray _dimNames;

      // Pointers to the X/Y and time dimensions and the associated coordinate
      // variables.  Note that these are pointers into the _dims and Var
      // arrays so should not be deleted.

      netCDF::NcDim *_faceDim;
      netCDF::NcDim *_edgeDim;
      netCDF::NcDim *_nodeDim;
      netCDF::NcDim *_virtDim;
      netCDF::NcDim *_tDim;

      netCDF::NcVar *_latVar;
      netCDF::NcVar *_lonVar;
      NcVarInfo *_time_var_info;

      int face_count;
      //double *_lat;
      //double *_lon;

      void init_from_scratch();

      UGridFile(const UGridFile &);
      UGridFile & operator=(const UGridFile &);

      // Determine the file times from the filename

      // Read the grid information from the netCDF file and fill in the
      // grid member with that information.

      std::string find_metadata_name(std::string &key, StringArray &available_names);
      StringArray get_metadata_names(std::string &key);
      void read_config(ConcatString config_filename);
      void read_netcdf_grid();

};


////////////////////////////////////////////////////////////////////////

inline netCDF::NcDim *UGridFile::get_vert_dim() const { return _virtDim; }
inline ConcatString UGridFile::coordinate_nc() { return coordinate_file; }

////////////////////////////////////////////////////////////////////////

#endif   /*  __UGRID_FILE_H__  */


////////////////////////////////////////////////////////////////////////


