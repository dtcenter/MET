

// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2025
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*




////////////////////////////////////////////////////////////////////////


#ifndef  __MET_WRF_FILE_H__
#define  __MET_WRF_FILE_H__


////////////////////////////////////////////////////////////////////////


#include <ostream>

#include <netcdf>

#include "vx_grid.h"
#include "data_plane.h"
#include "long_array.h"
#include "nc_var_info.h"


////////////////////////////////////////////////////////////////////////


class WrfFile {

   private:

      void init_from_scratch();

      WrfFile(const WrfFile &);
      WrfFile & operator=(const WrfFile &);

      static bool parse_dims_for_var(const std::string& var_name, NcVarInfo* var, std::string& z_name);
      void handle_pressure(const NcVarInfo* var, const std::string& z_name, NcVarInfo*& P,
                           bool& time_in_pressure, double& pressure_unit_conversion) const;
      static bool check_star_position_and_count(const LongArray& a, const NcVarInfo* var);
      void setup_dataplane(netCDF::NcVar* v, const LongArray& a, DataPlane& plane, int dim_count, const NcVarInfo* var) const;
      void dump_dims(std::ostream& out, int j, std::string& c) const;
      static bool parse_dim_x(const std::string& var_name, NcVarInfo* var, const std::string& c, int k);
      static void parse_dim_y(NcVarInfo* var, const std::string& c, int k);
      static void parse_z_dim(NcVarInfo* var, std::string& z_name, const std::string& c, int k);

   public:

      WrfFile();
     ~WrfFile();

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

      netCDF::NcDim * Tdim;   //  not allocated

         //
         //  variables
         //

      int Nvars;

      NcVarInfo * Var;     //  allocated

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


#endif   /*  __MET_WRF_FILE_H__  */


////////////////////////////////////////////////////////////////////////


