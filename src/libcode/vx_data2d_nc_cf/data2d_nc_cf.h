// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2026
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*


////////////////////////////////////////////////////////////////////////


#ifndef  __MET_VX_DATA_2D_NC_CF_H__
#define  __MET_VX_DATA_2D_NC_CF_H__


////////////////////////////////////////////////////////////////////////


#include "data_plane.h"
#include "data_class.h"
#include "var_info_nc_cf.h"
#include "nc_cf_file.h"
#include "two_to_one.h"


////////////////////////////////////////////////////////////////////////


class MetNcCFDataFile : public Met2dDataFile {

   private:

      MetNcCFDataFile(const MetNcCFDataFile &);
      MetNcCFDataFile & operator=(const MetNcCFDataFile &);

      int add_data_planes_by_time(VarInfo &vinfo, const LevelInfo &level,
                                  DataPlaneArray &plane_array);
      int add_data_planes_by_z(VarInfo &vinfo, const LevelInfo &level,
                               DataPlaneArray &plane_array);
      LongArray collect_time_offsets(VarInfo &vinfo);
      LongArray collect_z_offsets(VarInfo &vinfo);

      long convert_generic_to_offset(double value, const std::string& dim_name, std::vector<double> values);
      long convert_time_to_offset(double time_value) const;
      long convert_z_to_offset(double z_value, const NcVarInfo* data_var);
      bool data_plane(VarInfo &, DataPlane &, const LongArray &dimension);
      void error_message(const bool is_dim_time, const int error_code,
                         const double _lower, const double _upper,
                         const long _value, const ConcatString &var_name,
                         const std::string &method_name) const;
      NcVarInfo *find_first_data_var();
      long find_generic_offset(VarInfo& vinfo, const NcVarInfo* data_var, int index);
      long find_time_offset(VarInfo &vinfo, const NcVarInfo *data_var);
      long find_z_offset(VarInfo &vinfo, const NcVarInfo *data_var);
      NcVarInfo *get_data_var(VarInfo &vinfo);
      std::string get_dim_name(const NcVarInfo* data_var, int index) const;
      std::string get_z_dim_name(const NcVarInfo *data_var) const;
      void nccf_init_from_scratch();

      //
      //  NetCDF file
      //

      NcCfFile *_file;          // allocated
      long cur_time_index;      // current time index to get the data plane (for array of data_plane)
      long cur_z_index;         // current vlevel index to get the data plane (for array of data_plane)

   public:

      MetNcCFDataFile();
     ~MetNcCFDataFile();

      virtual int nx() const
      {
         if (_file == nullptr) return 0;

         return _file->getNx();
      }


      virtual int ny() const
      {
         if (_file == nullptr) return 0;

         return _file->getNy();
      }


      //
      //  set stuff
      //

      //
      //  get stuff
      //

      GrdFileType file_type() const;

      //  retrieve the first matching data plane

      bool data_plane(VarInfo &, DataPlane &);

      //  retrieve all matching data planes

      int data_plane_array(VarInfo &, DataPlaneArray &);

      //  retrieve the index of the first matching record

      int index(VarInfo &);

      //
      //  do stuff
      //

      bool open  (const char * filename);

      void close ();

      void dump(std::ostream &, int = 0) const;

      Grid build_grid_from_lat_lon_vars(netCDF::NcVar *lat_var, netCDF::NcVar *lon_var,
                                        const long lat_counts, const long lon_counts);
};


////////////////////////////////////////////////////////////////////////


inline GrdFileType MetNcCFDataFile::file_type () const { return FileType_NcCF; }


////////////////////////////////////////////////////////////////////////


#endif   /*  __MET_VX_DATA_2D_NC_CF_H__  */


////////////////////////////////////////////////////////////////////////


