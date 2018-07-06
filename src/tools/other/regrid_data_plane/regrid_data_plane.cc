// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2018
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*

///////////////////////////////////////////////////////////////////////
//
//   Filename:   regrid_data_plane.cc
//
//   Description:
//      This tool reads 2-dimensional fields from a gridded input file,
//      regrids them to the user-specified output grid, and writes the
//      regridded output data in NetCDF format.
//
//   Mod#   Date      Name           Description
//   ----   ----      ----           -----------
//   000    01-29-15  Halley Gotway  New
//   001    03-23-17  Halley Gotway  Change -name to an array.
//   002    06-25-17  Howard Soh     Support GOES-16
//
////////////////////////////////////////////////////////////////////////

using namespace std;

#include <cstdio>
#include <cstdlib>
#include <ctime>
#include <ctype.h>
#include <dirent.h>
#include <iostream>
#include <fstream>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include "vx_log.h"
#include "vx_data2d_factory.h"
#include "vx_data2d.h"
#include "vx_nc_util.h"
#include "vx_grid.h"
#include "vx_regrid.h"
#include "vx_util.h"
#include "vx_statistics.h"

#include "GridTemplate.h"

////////////////////////////////////////////////////////////////////////

static ConcatString program_name;

// Constants
static const InterpMthd DefaultInterpMthd = InterpMthd_Nearest;
static const int        DefaultInterpWdth = 1;
static const double     DefaultVldThresh  = 0.5;

// Variables for command line arguments
static ConcatString InputFilename;
static ConcatString OutputFilename;
static StringArray FieldSA;
static RegridInfo RGInfo;
static StringArray VarNameSA;
static int compress_level = -1;

// Output NetCDF file
static NcFile *nc_out  = (NcFile *) 0;
static NcDim  lat_dim ;
static NcDim  lon_dim ;

////////////////////////////////////////////////////////////////////////
//for GOES 16
static ConcatString coord_name;
static IntArray qc_flags;

static int  get_lat_count(NcFile *);
static int  get_lon_count(NcFile *);
static void process_data_only_file();
static void write_nc_var(const DataPlane &dp, const Grid &grid,
                     const VarInfo *vinfo, const char *vname);
static void set_coord_name(const StringArray &);
static void set_qc_flags(const StringArray &);

////////////////////////////////////////////////////////////////////////

static void process_command_line(int, char **);
static void process_data_file();
static void open_nc(const Grid &grid, const ConcatString run_cs);
static void write_nc(const DataPlane &dp, const Grid &grid,
                     const VarInfo *vinfo, const char *vname);
static void close_nc();
static void usage();
static void set_field(const StringArray &);
static void set_method(const StringArray &);
static void set_shape(const StringArray &);
static void set_width(const StringArray &);
static void set_vld_thresh(const StringArray &);
static void set_name(const StringArray &);
static void set_logfile(const StringArray &);
static void set_verbosity(const StringArray &);
static void set_compress(const StringArray &);

////////////////////////////////////////////////////////////////////////

int main(int argc, char *argv[]) {

   // Store the program name
   program_name = get_short_name(argv[0]);

   // Set handler to be called for memory allocation error
   set_new_handler(oom);

   // Process the command line arguments
   process_command_line(argc, argv);

   // Process the input data file
   if (coord_name.empty())
      process_data_file();
   else
      process_data_only_file();

   return(0);
}

////////////////////////////////////////////////////////////////////////

void process_command_line(int argc, char **argv) {
   CommandLine cline;

   // Set default regridding options
   RGInfo.enable     = true;
   RGInfo.field      = FieldType_None;
   RGInfo.method     = DefaultInterpMthd;
   RGInfo.width      = DefaultInterpWdth;
   RGInfo.vld_thresh = DefaultVldThresh;
   RGInfo.shape      = GridTemplateFactory::GridTemplate_Square;

   // Check for zero arguments
   if(argc == 1) usage();

   // Parse the command line into tokens
   cline.set(argc, argv);

   // Set the usage function
   cline.set_usage(usage);

   // Add the options function calls
   cline.add(set_field,      "-field",      1);
   cline.add(set_method,     "-method",     1);
   cline.add(set_shape,      "-shape",      1);
   cline.add(set_width,      "-width",      1);
   cline.add(set_vld_thresh, "-vld_thresh", 1);
   cline.add(set_name,       "-name",       1);
   cline.add(set_logfile,    "-log",        1);
   cline.add(set_verbosity,  "-v",          1);
   cline.add(set_compress,   "-compress",   1);
   cline.add(set_coord_name, "-coord",      1);
   cline.add(set_qc_flags,   "-qc",         1);

   // Parse the command line
   cline.parse();

   // Check for error. There should be three arguments left:
   // - input filename
   // - destination grid,
   // - output filename
   if(cline.n() != 3) usage();

   // Store the filenames and config string.
   InputFilename  = cline[0];
   RGInfo.name    = cline[1];
   OutputFilename = cline[2];

   // Check for at least one configuration string
   if(FieldSA.n_elements() < 1) {
      mlog << Error << "\nprocess_command_line() -> "
           << "The -field option must be used at least once!\n\n";
      usage();
   }

   // Check that the number of output names and fields match
   if(VarNameSA.n_elements() > 0 &&
      VarNameSA.n_elements() != FieldSA.n_elements()) {
      mlog << Error << "\nprocess_command_line() -> "
           << "When the -name option is used, the number of entries ("
           << VarNameSA.n_elements() << ") must match the number of "
           << "-field entries (" << FieldSA.n_elements() << ")!\n\n";
      usage();
   }

   RGInfo.validate();

   return;
}

////////////////////////////////////////////////////////////////////////

void process_data_file() {
   DataPlane fr_dp, to_dp;
   Grid fr_grid, to_grid;
   GrdFileType ftype;
   double dmin, dmax;
   ConcatString run_cs, vname;

   // Initialize configuration object
   MetConfig config;
   config.read(replace_path(config_const_filename));
   config.read_string(FieldSA[0]);

   // Note: The command line argument MUST processed before this
   if (compress_level < 0) compress_level = config.nc_compression();

   // Get the gridded file type from config string, if present
   ftype = parse_conf_file_type(&config);

   // Read the input data file
   Met2dDataFileFactory m_factory;
   Met2dDataFile *fr_mtddf = (Met2dDataFile *) 0;

   // Determine the "from" grid
   mlog << Debug(1)  << "Reading data file: " << InputFilename << "\n";
   fr_mtddf = m_factory.new_met_2d_data_file(InputFilename, ftype);

   if(!fr_mtddf) {
      mlog << Error << "\nprocess_data_file() -> "
           << "\"" << InputFilename << "\" not a valid data file\n\n";
      exit(1);
   }
   fr_grid = fr_mtddf->grid();
   mlog << Debug(2) << "Input grid: " << fr_grid.serialize() << "\n";

   // Determine the "to" grid
   to_grid = parse_vx_grid(RGInfo, &fr_grid, &fr_grid);
   mlog << Debug(2) << "Output grid: " << to_grid.serialize() << "\n";

   GridTemplateFactory gtf;
   mlog << Debug(2) << "Interpolation options: "
        << "method = " << interpmthd_to_string(RGInfo.method)
        << ", width = " << RGInfo.width
        << ", shape = " << gtf.enum2String(RGInfo.shape)
        << ", vld_thresh = " << RGInfo.vld_thresh << "\n";

   // Build the run command string
   run_cs << "Regrid from " << fr_grid.serialize() << " to " << to_grid.serialize();

   // Setup the VarInfo request object
   VarInfoFactory v_factory;
   VarInfo *vinfo;
   vinfo = v_factory.new_var_info(fr_mtddf->file_type());

   if(!vinfo) {
      mlog << Error << "\nprocess_data_file() -> "
           << "unable to determine file type of \"" << InputFilename
           << "\"\n\n";
      exit(1);
   }

   // Open the output file
   open_nc(to_grid, run_cs);

   // Loop through the requested fields
   for(int i=0; i<FieldSA.n_elements(); i++) {

      // Initialize
      vinfo->clear();

      // Populate the VarInfo object using the config string
      config.read_string(FieldSA[i]);
      vinfo->set_dict(config);

      // Get the data plane from the file for this VarInfo object
      if(!fr_mtddf->data_plane(*vinfo, fr_dp)) {
         mlog << Error << "\nprocess_data_file() -> trouble getting field \""
              << FieldSA[i] << "\" from file \"" << InputFilename << "\"\n\n";
         exit(1);
      }

      // Regrid the data plane
      to_dp = met_regrid(fr_dp, fr_grid, to_grid, RGInfo);

      // List range of data values
      if(mlog.verbosity_level() >= 2) {
         fr_dp.data_range(dmin, dmax);
         mlog << Debug(2)
              << "Range of input data (" << FieldSA[i] << ") is "
              << dmin << " to " << dmax << ".\n";
         to_dp.data_range(dmin, dmax);
         mlog << Debug(2)
              << "Range of regridded data (" << FieldSA[i] << ") is "
              << dmin << " to " << dmax << ".\n";
      }

      // Select output variable name
      if(VarNameSA.n_elements() == 0) {
         vname << cs_erase << vinfo->name();
         if(vinfo->level().type() != LevelType_Accum &&
            ftype != FileType_NcMet &&
            ftype != FileType_General_Netcdf &&
            ftype != FileType_NcPinterp &&
            ftype != FileType_NcCF) {
            vname << "_" << vinfo->level_name();
         }
      }
      else {
         vname = VarNameSA[i];
      }

      // Write the regridded data
      write_nc(to_dp, to_grid, vinfo, vname);

   } // end for i

   // Close the output file
   close_nc();

   // Clean up
   if(fr_mtddf) { delete fr_mtddf; fr_mtddf = (Met2dDataFile *) 0; }
   if(vinfo)    { delete vinfo;    vinfo    = (VarInfo *)       0; }

   return;
}

////////////////////////////////////////////////////////////////////////

Grid get_grid(ConcatString grid_name_file) {
   Grid grid;
   if(find_grid_by_name(grid_name_file, grid)) {
      mlog << Debug(3) << " Found grid from " << grid_name_file << "\n";
   }
   else {
      Met2dDataFileFactory factory;
      Met2dDataFile * datafile = (Met2dDataFile *) 0;

      // If that doesn't work, try to open a data file.
      datafile = factory.new_met_2d_data_file(replace_path(grid_name_file));

      if(!datafile) {
        mlog << Error << "\nget_grid() -> "
             << "can't open data file \"" << grid_name_file << "\"\n\n";
        exit(1);
      }

      // Store the data file's grid
      grid = datafile->grid();

      delete datafile; datafile = (Met2dDataFile *) 0;
   }
   return grid;
}

void check_lat_lon(int data_size, float  *latitudes, float  *longitudes) {
   int cnt_printed = 0;
   int cnt_missing_lat = 0;
   int cnt_missing_lon = 0;
   int cnt_bad_lat = 0;
   int cnt_bad_lon = 0;
   float min_lat=90.0;
   float max_lat=-90.0;
   float min_lon=360.0;
   float max_lon=-360.0;
   for (int idx=0; idx<data_size; idx++) {
      if (cnt_printed < 10 && !is_eq(latitudes[idx], -999.0) && !is_eq(longitudes[idx], -999.0)) {
         mlog << Debug(7) << "  index: " << idx <<  " lat: " << latitudes[idx]
                          << ", lon: " << longitudes[idx] << "\n";
         cnt_printed++;
      }
      if (is_eq(latitudes[idx], -999.0))  cnt_missing_lat++;
      else if (latitudes[idx] < -90 || latitudes[idx] > 90)  cnt_bad_lat++;
      else {
         if (min_lat > latitudes[idx]) min_lat = latitudes[idx];
         if (max_lat < latitudes[idx]) max_lat = latitudes[idx];
      }
      if (is_eq(longitudes[idx], -999.0)) cnt_missing_lon++;
      else if (longitudes[idx] < -180 || longitudes[idx] > 180) cnt_bad_lon++;
      else {
         if (min_lon > longitudes[idx]) min_lon = longitudes[idx];
         if (max_lon < longitudes[idx]) max_lon = longitudes[idx];
      }
   }
   mlog << Debug(7) << "\n MISSING lat: " << cnt_missing_lat << ", lon: " << cnt_missing_lon << "\n"
                    << "     bad lat: " << cnt_bad_lat << ", lon: " << cnt_bad_lon << "\n"
                    << "     LAT min: " << min_lat << ", max: " << max_lat << "\n"
                    << "    LONG min: " << min_lon << ", max: " << max_lon << "\n"
                    << "\n\n";

}

void get_grid_mapping(const ConcatString coord_name,
      Grid to_grid, IntArray *cellMapping, int &from_lat_count, int &from_lon_count) {
   static const char *method_name = "get_grid_mapping()";
   DataPlane from_dp;
   DataPlane to_dp;

   // Determine the "from" grid
   mlog << Debug(2)  << method_name << " Reading coord file: " << coord_name << "\n";
   NcFile *_nc = open_ncfile(coord_name);
   int to_lat_count = to_grid.ny();
   int to_lon_count = to_grid.nx();

   from_lat_count = 1500;
   from_lon_count = 2500;
   if (!IS_INVALID_NC_P(_nc)) {
      from_lat_count = get_lat_count(_nc);
      from_lon_count = get_lon_count(_nc);
   }
   int data_size  = from_lat_count * from_lon_count;
   mlog << Debug(4) << method_name << " data_size (ny*nx): " << data_size
        << " = " << from_lat_count << " * " << from_lon_count << "\n"
        << "                   target grid (nx,ny)="
        << to_lon_count << "," << to_lat_count << "\n";

   from_dp.set_size(from_lon_count, from_lat_count);
   to_dp.set_size(to_lon_count, to_lat_count);

   if (data_size > 0) {
      double x, y;
      float  lat, lon;
      int    idx_x, idx_y;
      int    coord_offset, to_offset;
      int    count_in_grid;
      float  *latitudes  = new float[data_size];
      float  *longitudes = new float[data_size];

      memset(latitudes,  0, data_size*sizeof(float));
      memset(longitudes, 0, data_size*sizeof(float));

      if (!IS_INVALID_NC_P(_nc)) {
         NcVar var_lat = get_nc_var(_nc, "latitude");
         NcVar var_lon = get_nc_var(_nc, "longitude");
         if (!IS_INVALID_NC(var_lat) && !IS_INVALID_NC(var_lon)) {
            get_nc_data(&var_lat, latitudes);
            get_nc_data(&var_lon, longitudes);
         }
      }
      else {
         bool result;
         FILE *pFile = fopen ( coord_name, "rb" );
         result = fread (latitudes,sizeof(latitudes[0]),data_size,pFile);
         result = fread (longitudes,sizeof(longitudes[0]),data_size,pFile);
         fclose (pFile);
      }
      check_lat_lon(data_size, latitudes, longitudes);

      count_in_grid = 0;

      //Following the logic at DataPlane::two_to_one(int x, int y) n = y*Nx + x;
      for (int xIdx=0; xIdx<from_lat_count; xIdx++) {
         int lat_offset = from_lon_count * xIdx;
         for (int yIdx=0; yIdx<from_lon_count; yIdx++) {
            int offset = lat_offset + yIdx;
            coord_offset = from_dp.two_to_one(yIdx, xIdx);
            lat = latitudes[coord_offset];
            lon = longitudes[coord_offset];
            to_grid.latlon_to_xy(lat, -1.0*lon, x, y);
            idx_x = nint(x);
            idx_y = nint(y);

            if (0 <= idx_x && idx_x < to_lon_count && 0 <= idx_y && idx_y < to_lat_count) {
               to_offset = to_dp.two_to_one(idx_x, idx_y);
               cellMapping[to_offset].add(coord_offset);
               count_in_grid++;
            }
         }
      }
      mlog << Debug(3) << method_name << " within grid: " << count_in_grid
           << " out of " << data_size << " (" << count_in_grid*100/data_size << "%)\n";
      delete [] latitudes;
      delete [] longitudes;
   }
   if(_nc) {
      delete _nc;
   }
}

void process_data_only_file() {
   DataPlane to_dp;
   Grid fr_grid, to_grid;
   GrdFileType ftype;
   double dmin, dmax;
   ConcatString run_cs, vname;
   static const char *method_name = "process_data_only_file() ";

   // Initialize configuration object
   MetConfig config;
   config.read(replace_path(config_const_filename));

   // Note: The command line argument MUST processed before this
   if (compress_level < 0) compress_level = config.nc_compression();

   // Get the gridded file type from config string, if present
   ftype = parse_conf_file_type(&config);

   // Read the input data file

   // Determine the "to" grid
   to_grid = get_grid(RGInfo.name);

   int to_lat_count = to_grid.ny();
   int to_lon_count = to_grid.nx();
   int from_lat_count, from_lon_count, from_data_size;
   IntArray *cellMapping = new IntArray[(to_lat_count * to_lon_count)];
   get_grid_mapping(coord_name, to_grid, cellMapping, from_lat_count, from_lon_count);
   from_data_size = from_lat_count * from_lon_count;

   GridTemplateFactory gtf;
   mlog << Debug(2) << "Interpolation options: "
        << "method = " << interpmthd_to_string(RGInfo.method)
        //<< ", width = " << RGInfo.width
        //<< ", shape = " << gtf.enum2String(RGInfo.shape)
        << ", vld_thresh = " << RGInfo.vld_thresh << "\n";

   // Build the run command string
   run_cs << "Regrid from " << coord_name << " to " << to_grid.serialize();

   // Setup the VarInfo request object
   VarInfoFactory v_factory;
   VarInfo *vinfo = (VarInfo *)0;
   vinfo = v_factory.new_var_info(FileType_NcCF);

   // Open the output file
   open_nc(to_grid, run_cs);

   bool all_attrs = false;
   NcFile   *_nc_in = open_ncfile(InputFilename);
   ncbyte  *qc_data = new ncbyte[from_data_size];
   float *from_data = new float[from_data_size];

   memset(qc_data, -99, from_data_size*sizeof(ncbyte)); // -99 is arbitrary number as invliad QC value

   to_dp.set_size(to_lon_count, to_lat_count);

   int qc_filtered_count;
   NcVar var_qc;
   ConcatString qc_var_name;

   // Loop through the requested fields
   bool has_qc_flags = (qc_flags.n_elements() > 0);
   for(int i=0; i<FieldSA.n_elements(); i++) {

      // Regrid the data plane
      int offset;
      int to_offset;
      int valid_count;
      int censored_count;
      int missing_count = 0;
      int non_missing_count = 0;
      float data_value;
      float from_min_value =  10e10;
      float from_max_value = -10e10;
      float qc_min_value =  10e10;
      float qc_max_value = -10e10;
      IntArray cellArray;
      NumArray dataArray;

      // Initialize
      vinfo->clear();

      // Populate the VarInfo object using the config string
      config.read_string(FieldSA[i]);
      vinfo->set_dict(config);

      //AOD:ancillary_variables = "DQF" ; byte DQF(y, x) ;
      bool has_qc_var = false;
      NcVar var_data = get_nc_var(_nc_in, vinfo->name());
      if (get_att_value_string(&var_data, "ancillary_variables", qc_var_name)) {
         var_qc = get_nc_var(_nc_in, qc_var_name);
         get_nc_data(&var_qc, qc_data);
         has_qc_var = true;
         mlog << Debug(3) << method_name << "found QC var: " << qc_var_name << ".\n";
      }

      get_nc_data(&var_data, (float *)from_data);

      censored_count = 0;
      qc_filtered_count = 0;
      to_dp.erase();
      to_dp.set_constant(bad_data_double);

      for (int xIdx=0; xIdx<to_lon_count; xIdx++) {
         for (int yIdx=0; yIdx<to_lat_count; yIdx++) {
            offset = to_dp.two_to_one(xIdx,yIdx);
            cellArray = cellMapping[offset];
            if (0 < cellArray.n_elements()) {
               valid_count = 0;
               dataArray.clear();
               for (int dIdx=0; dIdx<cellArray.n_elements(); dIdx++) {
                  data_value = from_data[cellArray[dIdx]];
                  if (is_eq(data_value, bad_data_float)) {
                     missing_count++;
                     continue;
                  }

                  if(mlog.verbosity_level() >= 4) {
                     if (from_min_value > data_value) from_min_value = data_value;
                     if (from_max_value < data_value) from_max_value = data_value;
                  }

                  //Filter by QC flag
                  if (!has_qc_var || !has_qc_flags
                       || qc_flags.has(qc_data[cellArray[dIdx]])) {
                     for(int i=0; i<vinfo->censor_thresh().n_elements(); i++) {
                        // Break out after the first match.
                        if(vinfo->censor_thresh()[i].check(data_value)) {
                           //float saved_value = data_value;
                           data_value = vinfo->censor_val()[i];
                           censored_count++;
                           //cout << "   DEBUG org value: " << saved_value << " to " << data_value << "\n";
                           break;
                        }
                     }
                     dataArray.add(data_value);
                     if (mlog.verbosity_level() >= 4) {
                        if (qc_min_value > data_value) qc_min_value = data_value;
                        if (qc_max_value < data_value) qc_max_value = data_value;
                     }
                  }
                  else {
                     qc_filtered_count++;
                  }
                  valid_count++;
               }
               if (0 < dataArray.n_elements()) {
                  int data_count = dataArray.n_elements();
                  float new_value;
                  if      (RGInfo.method == InterpMthd_Min) new_value = dataArray.min();
                  else if (RGInfo.method == InterpMthd_Max) new_value = dataArray.max();
                  else if (RGInfo.method == InterpMthd_Median) {
                     cellArray.sort_increasing();
                     new_value = dataArray[data_count/2];
                     if (0 == data_count % 2) new_value = (new_value + dataArray[(data_count/2)+1])/2;
                  }
                  else {
                     float sum_value = dataArray.sum();
                     new_value = dataArray.sum() / data_count;
                  }
                  to_dp.set(new_value, xIdx, yIdx);
                  if(mlog.verbosity_level() > 7) {
                     if (300 < dataArray.n_elements()) {
                        cout << " data from " << dataArray.n_elements() << ". max: "
                             << dataArray.max() << ", min: " << dataArray.min()
                             << " mean: " << dataArray.sum()/dataArray.n_elements() << "\n";
                     }
                  }
               }
            }
         }
      }
      mlog << Debug(4) << method_name << " Count: missing: "
           << missing_count << ", non_missing: " << non_missing_count
           << " value range: [" << from_min_value << " - " << from_max_value
           << "] QCed: [" << qc_min_value << " - " << qc_max_value << "]\n";

      // List range of data values
      if(mlog.verbosity_level() >= 2) {
         to_dp.data_range(dmin, dmax);

         char censored_info[50];
         if (censored_count > 0) {
            sprintf (censored_info, " censored count: %d", censored_count);
         }
         else sprintf (censored_info, "");
         mlog << Debug(2)
              << "Range of regridded data (" << vinfo->name() << ") is "
              << dmin << " to " << dmax << ".\n";
         mlog << Debug(2)
              << "filtered by QC: " << qc_filtered_count
              << ", has_qc_var: " << has_qc_var << censored_info << ".\n";
      }

      // Select output variable name
      if(VarNameSA.n_elements() == 0) {
         vname << cs_erase << vinfo->name();
      }
      else {
         vname = VarNameSA[i];
      }

      // Write the regridded data
      write_nc_var(to_dp, to_grid, vinfo, vname);
      NcVar to_var = get_nc_var(nc_out, vname);
      copy_nc_atts(&var_data, &to_var, all_attrs);

   } // end for i

   if (from_data) {
      delete[] from_data;
      from_data = (float *)0;
   }
   if (cellMapping) {
      delete[] cellMapping;
      cellMapping = (IntArray *)0;
   }

   NcVar from_var;
   multimap<string,NcVar> mapVar = GET_NC_VARS_P(_nc_in);
   for (multimap<string,NcVar>::iterator itVar = mapVar.begin();
         itVar != mapVar.end(); ++itVar) {
      if ((*itVar).first == "t" || string::npos != (*itVar).first.find("time")) {
         from_var = (*itVar).second;
         NcVar *to_var = copy_nc_var(nc_out, &from_var);
      }
   }

   copy_nc_atts(_nc_in, nc_out, all_attrs);

   // Close the output file
   close_nc();

   // Clean up
   if(vinfo)    { delete vinfo;    vinfo    = (VarInfo *)       0; }
   if(_nc_in)   { delete _nc_in;   _nc_in   = (NcFile *)        0; }

   return;
}

////////////////////////////////////////////////////////////////////////

void open_nc(const Grid &grid, ConcatString run_cs) {

   // Create output file
   nc_out = open_ncfile(OutputFilename, true);

   if(IS_INVALID_NC_P(nc_out)) {
      mlog << Error << "\nopen_nc() -> "
           << "trouble opening output NetCDF file \""
           << OutputFilename << "\"\n\n";
      exit(1);
   }

   // Add global attributes
   write_netcdf_global(nc_out, OutputFilename, program_name);

   // Add the run command
   add_att(nc_out, "RunCommand", run_cs);

   // Add the projection information
   write_netcdf_proj(nc_out, grid);

   // Define Dimensions
   lat_dim = add_dim(nc_out, "lat", (long) grid.ny());
   lon_dim = add_dim(nc_out, "lon", (long) grid.nx());

   // Add the lat/lon variables
   write_netcdf_latlon(nc_out, &lat_dim, &lon_dim, grid);

   return;
}

////////////////////////////////////////////////////////////////////////

void write_nc_data(const DataPlane &dp, const Grid &grid, NcVar *data_var) {

   // Allocate memory to store data values for each grid point
   float *data = new float [grid.nx()*grid.ny()];

   // Store the data
   int grid_nx = grid.nx();
   int grid_ny = grid.ny();
   for(int x=0; x<grid_nx; x++) {
      for(int y=0; y<grid_ny; y++) {
         int n = DefaultTO.two_to_one(grid_nx, grid_ny, x, y);
         data[n] = (float) dp(x, y);
      } // end for y
   } // end for x

   // Write out the data
   if(!put_nc_data_with_dims(data_var, &data[0], grid.ny(), grid.nx())) {
      mlog << Error << "\nwrite_nc_data() -> "
           << "error writing data to the output file.\n\n";
      exit(1);
   }

   // Clean up
   if(data) { delete [] data;  data = (float *)  0; }

   return;
}

void write_nc(const DataPlane &dp, const Grid &grid,
              const VarInfo *vinfo, const char *vname) {

   int deflate_level = compress_level;
   if (deflate_level < 0) deflate_level = 0;

   NcVar data_var = add_var(nc_out, (string)vname, ncFloat,
                            lat_dim, lon_dim, deflate_level);
   add_att(&data_var, "name", (string)vname);
   add_att(&data_var, "long_name", (string)vinfo->long_name());
   add_att(&data_var, "level", (string)vinfo->level_name());
   add_att(&data_var, "units", (string)vinfo->units());
   add_att(&data_var, "_FillValue", bad_data_float);
   write_netcdf_var_times(&data_var, dp);

   write_nc_data(dp, grid, &data_var);

   return;
}

void write_nc_var(const DataPlane &dp, const Grid &grid,
              const VarInfo *vinfo, const char *vname) {

   int deflate_level = compress_level;
   if (deflate_level < 0) deflate_level = 0;

   NcVar data_var = add_var(nc_out, (string)vname, ncFloat,
                            lat_dim, lon_dim, deflate_level);

   add_att(&data_var, "_FillValue", bad_data_float);
   write_netcdf_var_times(&data_var, dp);

   write_nc_data(dp, grid, &data_var);

   return;
}

////////////////////////////////////////////////////////////////////////

int get_lat_count(NcFile *_nc) {
   int lat_count = 0;
   NcDim dim_lat = get_nc_dim(_nc, "lat");
   if(!IS_INVALID_NC(dim_lat)) lat_count= get_dim_size(&dim_lat);
   return lat_count;
}

int get_lon_count(NcFile *_nc) {
   int lon_count = 0;
   NcDim dim_lon = get_nc_dim(_nc, "lon");
   if(!IS_INVALID_NC(dim_lon)) lon_count= get_dim_size(&dim_lon);
   return lon_count;
}

////////////////////////////////////////////////////////////////////////

void close_nc() {

   // Clean up
   if(nc_out) {
      delete nc_out; nc_out = (NcFile *) 0;
   }

   // List the output file
   mlog << Debug(1)
        << "Writing output file: " << OutputFilename << "\n";

   return;
}

////////////////////////////////////////////////////////////////////////

void usage() {

   GridTemplateFactory gtf;
   cout << "\n*** Model Evaluation Tools (MET" << met_version
        << ") ***\n\n"

        << "Usage: " << program_name << "\n"
        << "\tinput_filename\n"
        << "\tto_grid\n"
        << "\toutput_filename\n"
        << "\t-field string\n"
        << "\t[-coord filename]\n"
        << "\t[-qc flags]\n"
        << "\t[-method type]\n"
        << "\t[-width n]\n"
        << "\t[-shape type]\n"
        << "\t[-vld_thresh n]\n"
        << "\t[-name list]\n"
        << "\t[-log file]\n"
        << "\t[-v level]\n"
        << "\t[-compress level]\n\n"

        << "\twhere\t\"input_filename\" is the gridded data file to be "
        << "read (required).\n"

        << "\t\t\"to_grid\" defines the output grid as a named grid, the "
        << "path to a gridded data file, or an explicit grid "
        << "specification string (required).\n"

        << "\t\t\"output_filename\" is the output NetCDF file to be "
        << "written (required).\n"

        << "\t\t\"-field string\" may be used multiple times to define "
        << "the data to be regridded (required).\n"

        << "\t\t\"-coord filename\" specifies the lat/lon grid mapping information (optional).\n"
        << "\t\t\"-qc flags\" specifies a comma-separated list of QC flags, for example \"0,1\" (optional).\n"
        << "\t\t\tOnly applied if the -coord argument is given and the QC variable exists.\n"

        << "\t\t\"-method type\" overrides the default regridding "
        << "method (" << interpmthd_to_string(RGInfo.method)
        << ") (optional).\n"

        << "\t\t\"-width n\" overrides the default regridding "
        << "width (" << RGInfo.width << ") (optional).\n"

        << "\t\t\"-shape type\" overrides the default interpolation shape ("
        << gtf.enum2String(RGInfo.shape) << ") "
        <<  "(optional).\n"

        << "\t\t\"-vld_thresh n\" overrides the default required "
        << "ratio of valid data for regridding (" << RGInfo.vld_thresh
        << ") (optional).\n"

        << "\t\t\"-name list\" specifies a comma-separated list of "
        << "output variable names for each field specified (optional).\n"

        << "\t\t\"-log file\" outputs log messages to the specified "
        << "file (optional).\n"

        << "\t\t\"-v level\" overrides the default level of logging ("
        << mlog.verbosity_level() << ") (optional).\n"

        << "\t\t\"-compress level\" overrides the compression level of NetCDF variable (optional).\n\n" << flush;

   exit(1);
}

////////////////////////////////////////////////////////////////////////

void set_field(const StringArray &a) {
   FieldSA.add(a[0]);
}

////////////////////////////////////////////////////////////////////////

void set_method(const StringArray &a) {
   RGInfo.method = string_to_interpmthd(a[0]);
}

////////////////////////////////////////////////////////////////////////

void set_width(const StringArray &a) {
   RGInfo.width = atoi(a[0]);
}


////////////////////////////////////////////////////////////////////////

void set_shape(const StringArray &a) {
	GridTemplateFactory gtf;
	RGInfo.shape = gtf.string2Enum(a[0]);
}

////////////////////////////////////////////////////////////////////////

void set_vld_thresh(const StringArray &a) {
   RGInfo.vld_thresh = atof(a[0]);
   if(RGInfo.vld_thresh > 1 || RGInfo.vld_thresh < 0) {
      mlog << Error << "\nset_vld_thresh() -> "
           << "-vld_thresh may only be set between 0 and 1: "
           << RGInfo.vld_thresh << "\n\n";
      exit(1);
   }
}

////////////////////////////////////////////////////////////////////////

void set_name(const StringArray & a) {
   VarNameSA.add_css(a[0]);
}

////////////////////////////////////////////////////////////////////////

void set_logfile(const StringArray &a) {
   ConcatString filename;

   filename = a[0];

   mlog.open_log_file(filename);
}

////////////////////////////////////////////////////////////////////////

void set_verbosity(const StringArray &a) {
   mlog.set_verbosity_level(atoi(a[0]));
}

////////////////////////////////////////////////////////////////////////

void set_compress(const StringArray & a) {
   compress_level = atoi(a[0]);
}

////////////////////////////////////////////////////////////////////////

void set_coord_name(const StringArray & a) {
   coord_name = a[0];
   if (RGInfo.method == DefaultInterpMthd) {
      RGInfo.width  = 0;
      RGInfo.method = InterpMthd_UW_Mean;
   }
}

////////////////////////////////////////////////////////////////////////

void set_qc_flags(const StringArray & a) {
   int qc_flag;
   StringArray sa;

   sa.parse_css(a[0]);
   for (int idx=0; idx<sa.n_elements(); idx++) {
      qc_flag = atoi(sa[idx]);
      if ( !qc_flags.has(qc_flag) ) qc_flags.add(qc_flag);
   }
}
