// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2023
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*

////////////////////////////////////////////////////////////////////////
//
//   Filename:   tc_diag.cc
//
//   Description:
//
//   Mod#   Date      Name          Description
//   ----   ----      ----          -----------
//   000    09/27/22  Halley Gotway New
//
////////////////////////////////////////////////////////////////////////

using namespace std;

#include <cstdio>
#include <cstdlib>
#include <ctype.h>
#include <dirent.h>
#include <fstream>
#include <math.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include <netcdf>
using namespace netCDF;

#ifdef _OPENMP
  #include "omp.h"
#endif

#include "main.h"
#include "tc_diag.h"
#include "python_tc_diag.h"

#include "series_data.h"

#include "vx_grid.h"
#include "vx_regrid.h"
#include "vx_tc_util.h"
#include "vx_nc_util.h"
#include "vx_tc_nc_util.h"
#include "vx_data2d_nc_met.h"
#include "vx_util.h"
#include "vx_log.h"
#include "vx_math.h"

#include "met_file.h"

////////////////////////////////////////////////////////////////////////

static void usage();
static void process_command_line(int, char**);
static void get_file_type();

static void process_tracks(TrackInfoArray&);
static void get_atcf_files(const StringArray&,
                           const StringArray&,
                           StringArray&,
                           StringArray&);
static void process_track_files(const StringArray&,
                                const StringArray&,
                                TrackInfoArray&);
static void process_track_points(const TrackInfoArray &);
static void process_fields(const TrackInfoArray &,
                           const unixtime, int,
                           const string &,
                           const DomainInfo &);
static void process_out_files(const TrackInfoArray &);

static void merge_tmp_files(const vector<TmpFileInfo *>);
static void copy_coord_vars(NcFile *to_nc, NcFile *from_nc);
static void copy_time_vars(NcFile *to_nc, NcFile *from_nc, int);

static bool is_keeper(const ATCFLineBase *);
static void set_deck(const StringArray&);
static void set_atcf_source(const StringArray&,
                            StringArray&,
                            StringArray&);
static void set_data(const StringArray&);
static void set_config(const StringArray&);
static void set_outdir(const StringArray&);

static void setup_out_files(const TrackInfoArray &);
static ConcatString get_out_key(const TrackInfo &);
static ConcatString get_tmp_key(const TrackInfo &,
                                const TrackPoint &,
                                const string &);

static ConcatString build_tmp_file_name(const TrackInfo *,
                                        const TrackPoint *,
                                        const string &);
static ConcatString build_out_file_name(const TrackInfo *,
                                        const char *);

static void write_tc_storm(NcFile *, const char *,
                           const char *, const char *);

static void write_tc_times(NcFile *, const NcDim &,
                           const TrackInfo *,
                           const TrackPoint *);

static void compute_lat_lon(TcrmwGrid&, double *, double *);

////////////////////////////////////////////////////////////////////////

//
// TODO after the MET version 11.1.0 release:
//   - Python diagnostics:
//     - Incorporate CIRA python diagnostics scripts.
//     - Read resulting Xarray dataset in memory.
//     - Write CIRA ASCII and NetCDF diagnostics output files.
//     - Add support for $MET_PYTHON_EXE.
//   - Input data:
//     - Instead of reading DataPlanes one at a time,
//       read them all at once or perhaps in groups
//       (e.g. all pressure levels).
//     - Parellelize the processing of valid times.
//     - Add support for vortex removal. Print a WARNING if
//       the Diag Track differs from the Tech Id for the data
//       files and vortex removal has not been requested.
//   - NetCDF cylindrical coordinates output:
//     - get_var_names() returns a multimap that is sorted by
//       the order of the variable names. This reorders the vars
//       in the NetCDF cyl coord output. Would prefer that reordering
//       not happen.
//   - Consider adding support for the "regrid" dictionary to
//     control cyl coord regridding step is done.
//

int met_main(int argc, char *argv[]) {

   // Print beta status warning
   print_beta_warning("The TC-Diag tool");

   // Process command line arguments
   process_command_line(argc, argv);

   // Process the track data
   TrackInfoArray tracks;
   process_tracks(tracks);

   // Setup output files for each track
   setup_out_files(tracks);

   // Process the gridded data
   process_track_points(tracks);

   // Process the output files
   process_out_files(tracks);

   return(0);
}

////////////////////////////////////////////////////////////////////////

const string get_tool_name() {
   return "tc_diag";
}

////////////////////////////////////////////////////////////////////////

void usage() {

   cout << "\n*** Model Evaluation Tools (MET" << met_version
        << ") ***\n\n"
        << "Usage: " << program_name << "\n"
        << "\t-data domain tech_id_list [ file_1 ... file_n | data_file_list ]\n"
        << "\t-deck file\n"
        << "\t-config file\n"
        << "\t[-outdir path]\n"
        << "\t[-log file]\n"
        << "\t[-v level]\n\n"

        << "\twhere\t\"-data domain tech_id_list [ file_1 ... file_n | data_file_list ]\"\n"

        << "\t\t\tSpecifies a domain name, a comma-separated list of ATCF tech ID's,\n"
        << "\t\t\tand a list of gridded data files or an ASCII file containing\n"
        << "\t\t\ta list of files to be used.\n"
        << "\t\t\tSpecify \"-data\" once for each data source (required).\n"

        << "\t\t\"-deck source\" is the ATCF format data source "
        << "(required).\n"

        << "\t\t\"-config file\" is a TCDiagConfig file to be used "
        << "(required).\n"

        << "\t\t\"-outdir path\" overrides the default output directory "
        << "(" << out_dir << ") (optional).\n"

        << "\t\t\"-log file\" outputs log messages to the specified "
        << "file (optional).\n"

        << "\t\t\"-v level\" overrides the default level of logging ("
        << mlog.verbosity_level() << ") (optional).\n\n" << flush;

   exit(1);
}

////////////////////////////////////////////////////////////////////////

void process_command_line(int argc, char **argv) {
   CommandLine cline;
   ConcatString default_config_file;

   // Default output directory
   out_dir = replace_path(default_out_dir);

   // Default output prefix
   out_prefix = replace_path(default_out_prefix);

   // Print usage statement for no arguments
   if(argc <= 1) usage();

   // Parse command line into tokens
   cline.set(argc, argv);

   // Set usage function
   cline.set_usage(usage);

   // Add function calls for arguments
   cline.add(set_data,   "-data",   -1);
   cline.add(set_deck,   "-deck",   -1);
   cline.add(set_config, "-config",  1);
   cline.add(set_outdir, "-outdir",  1);

   // Parse command line
   cline.parse();

   // Check for required arguments
   if(data_opt_map.size() == 0 ||
      deck_source.n()     == 0 ||
      config_file.empty()) {
      mlog << Error << "\nThe \"-data\", \"-deck\", and \"-config\" "
           << "command line arguments are required!\n\n";
      usage();
   }

   // Create default config file name
   default_config_file = replace_path(default_config_filename);

   // List config files
   mlog << Debug(1)
        << "Config File Default: " << default_config_file << "\n"
        << "Config File User: " << config_file << "\n";

   // Read config files
   conf_info.read_config(default_config_file.c_str(), config_file.c_str());

   // Get data file type from input files
   get_file_type();

   // Process the configuration
   conf_info.process_config(file_type, data_opt_map);

   return;
}

////////////////////////////////////////////////////////////////////////

void get_file_type() {
   Met2dDataFileFactory mtddf_factory;
   Met2dDataFile *mtddf = (Met2dDataFile *) 0;
   int i;

   // Build one long list of input data files
   StringArray file_list;
   map<string,DataOptInfo>::iterator it;
   for(it = data_opt_map.begin(); it != data_opt_map.end(); it++) {
      file_list.add(it->second.data_files);
   }

   // Get data file type from config
   GrdFileType conf_file_type =
      parse_conf_file_type(conf_info.conf.lookup_dictionary(conf_key_data));

   // Find the first file that actually exists
   for(i=0; i<file_list.n(); i++) {
      if(file_exists(file_list[i].c_str()) ||
         is_python_grdfiletype(conf_file_type)) break;
   }

   // Check for no valid files
   if(i == file_list.n()) {
      mlog << Error << "\nget_file_type() -> "
           << "No valid data files found.\n\n";
      exit(1);
   }

   // Read first valid file
   if(!(mtddf = mtddf_factory.new_met_2d_data_file(
                   file_list[i].c_str(), conf_file_type))) {
       mlog << Error << "\nget_file_type() -> "
            << "Trouble reading data file \""
            << file_list[i] << "\"\n\n";
       exit(1);
   }

   // Store the actual file type
   file_type = mtddf->file_type();

   // Clean up
   if(mtddf) { delete mtddf; mtddf = (Met2dDataFile *) 0; }

   return;
}

////////////////////////////////////////////////////////////////////////

void process_tracks(TrackInfoArray& tracks) {
   StringArray files, files_model_suffix;
   TimeArray init_ta;
   int i;

   // Get list of track files
   get_atcf_files(deck_source, deck_model_suffix,
                  files, files_model_suffix);

   mlog << Debug(2) << "Processing " << files.n()
        << " track data file(s).\n";

   process_track_files(files, files_model_suffix, tracks);

   // Get list of unique track initialization times
   for(i=0; i<tracks.n(); i++) {
      if(!init_ta.has(tracks[i].init())) init_ta.add(tracks[i].init());
   }

   // Check for a single track initialization time
   if(init_ta.n() > 1) {
      mlog << Error << "\nprocess_tracks() -> "
           << "set the \"init_inc\" config option to select one of the "
           << init_ta.n() << " track initialization times between "
           << unix_to_yyyymmddhh(init_ta.min()) << " and "
           << unix_to_yyyymmddhh(init_ta.max()) << ".\n\n";
      exit(1);
   }
   // Store the initialization time
   else {
      init_ut = init_ta[0];
   }

   return;
}

////////////////////////////////////////////////////////////////////////
//
// Automated Tropical cyclone Forecasting System
// https://www.nrlmry.navy.mil/atcf_web/docs/ATCF-FAQ.html
//
////////////////////////////////////////////////////////////////////////

void get_atcf_files(const StringArray& source,
                    const StringArray& model_suffix,
                    StringArray& files,
                    StringArray& files_model_suffix) {

   StringArray cur_source, cur_files;

   if(source.n() != model_suffix.n()) {
      mlog << Error << "\nget_atcf_files() -> "
           << "the source and suffix arrays must be equal length!\n\n";
      exit(1);
   }

   // Initialize
   files.clear();
   files_model_suffix.clear();

   // Build list of files from all sources
   for(int i = 0; i < source.n(); i++) {
      cur_source.clear();
      cur_source.add(source[i]);
      cur_files = get_filenames(cur_source, NULL, atcf_suffix);

      for(int j = 0; j < cur_files.n(); j++) {
         files.add(cur_files[j]);
         files_model_suffix.add(model_suffix[i]);
      }
   }

   return;
}

////////////////////////////////////////////////////////////////////////

void process_track_files(const StringArray& files,
                         const StringArray& model_suffix,
                         TrackInfoArray& tracks) {
   int i, cur_read, cur_add, tot_read, tot_add;
   LineDataFile f;
   ConcatString cs;
   ATCFTrackLine line;

   // Initialize
   tracks.clear();

   // Initialize counts
   tot_read = tot_add = 0;

   // Process input ATCF files
   for(i=0; i<files.n(); i++) {

       mlog << Debug(1) << "Reading track file: " << files[i] << "\n";

       // Open current file
       if(!f.open(files[i].c_str())) {
          mlog << Error << "\nprocess_track_files() -> "
              << "unable to open file \"" << files[i] << "\"\n\n";
          exit(1);
       }

       // Initialize counts
       cur_read = cur_add = 0;

       // Read each line
       while(f >> line) {

          // Increment line counts
          cur_read++;
          tot_read++;

          // Add model suffix, if specified
          if(model_suffix[i].length() > 0) {
             cs << cs_erase << line.technique() << model_suffix[i];
             line.set_technique(cs);
          }

          // Check the keep status
          if(!is_keeper(&line)) continue;

          // Attempt to add current line to TrackInfoArray
          if(tracks.add(line, true, false)) {
             cur_add++;
             tot_add++;
          }
       }

       // Dump out current number of lines
       mlog << Debug(4)
           << "[File " << i + 1 << " of " << files.n()
           << "] Used " << cur_add << " of " << cur_read
           << " lines read from file \n\"" << files[i] << "\"\n";

       // Close current file
       f.close();

   } // End loop over files

   // Check for no matching tracks
   if(tracks.n() == 0) {
       mlog << Error << "\nprocess_track_files() -> "
           << "no tracks retained! Adjust the config file "
           << "filtering options to select a single track.\n\n";
       exit(1);
   }

   return;
}

////////////////////////////////////////////////////////////////////////
//
// Check if the ATCFLineBase should be kept
//
////////////////////////////////////////////////////////////////////////

bool is_keeper(const ATCFLineBase * line) {
   bool keep = true;
   ConcatString cs;

   // Check model
   if(conf_info.model.n() > 0 &&
      !conf_info.model.has(line->technique())) {
     cs << "model " << line->technique() << " not in " << write_css(conf_info.model);
     keep = false;
   }

   // Check storm id
   else if(conf_info.storm_id.nonempty() &&
         conf_info.storm_id != line->storm_id()) {
     cs << "storm_id " << line->storm_id() << " != " << conf_info.storm_id;
     keep = false;
   }

   // Check basin
   else if(conf_info.basin.nonempty() &&
           conf_info.basin != line->basin()) {
     cs << "basin " << line->basin() << " != " << conf_info.basin;
     keep = false;
   }

   // Check cyclone
   else if(conf_info.cyclone.nonempty() &&
         conf_info.cyclone != line->cyclone_number()) {
     cs << "cyclone " << line->cyclone_number() << " != " << conf_info.cyclone;
     keep = false;
   }

   // Check initialization time
   else if(conf_info.init_inc != (unixtime) 0 &&
         conf_info.init_inc != line->warning_time()) {
     cs << "init_inc " << unix_to_yyyymmddhh(line->warning_time())
       << " != " << unix_to_yyyymmddhh(conf_info.init_inc);
     keep = false;
   }

   // Check valid time
   else if((conf_info.valid_beg > 0 &&
          conf_info.valid_beg > line->valid())   ||
         (conf_info.valid_end > 0 &&
          conf_info.valid_end < line->valid())   ||
         (conf_info.valid_inc.n() > 0 &&
         !conf_info.valid_inc.has(line->valid())) ||
         (conf_info.valid_exc.n() > 0 &&
          conf_info.valid_exc.has(line->valid()))) {
      cs << "valid_time " << unix_to_yyyymmddhh(line->valid());
      keep = false;
   }

   // Check valid hour
   else if(conf_info.valid_hour.n() > 0 &&
         !conf_info.valid_hour.has(line->valid_hour())) {
      cs << "valid_hour " << line->valid_hour();
      keep = false;
   }

   // Check lead time
   else if(conf_info.lead_time.n() > 0 &&
        !conf_info.lead_time.has(line->lead())){
     cs << "lead_time " << sec_to_hhmmss(line->lead());
     keep = false;
   }

   if(!keep) {
     mlog << Debug(4) << "Skipping track line for " << cs << ":\n"
         << line->get_line() << "\n";
   }

   // Return the keep status
   return(keep);
}

////////////////////////////////////////////////////////////////////////

void set_deck(const StringArray& a) {
   set_atcf_source(a, deck_source, deck_model_suffix);
}

////////////////////////////////////////////////////////////////////////

void set_atcf_source(const StringArray& a,
                     StringArray& source,
                     StringArray& model_suffix) {
   StringArray sa;
   ConcatString cs, suffix;

   // Check for optional suffix sub-argument
   for(int i = 0; i < a.n(); i++) {
      if(a[i] == "suffix") {
         cs = a[i];
         sa = cs.split("=");
         if(sa.n() != 2) {
            mlog << Error << "\nset_atcf_source() -> "
                 << "the model suffix must be specified as "
                 << "\"suffix=string\".\n\n";
         }
         else {
            suffix = sa[1];
         }
      }
   }

   // Parse remaining sources
   for(int i = 0; i < a.n(); i++) {
      if( a[i] == "suffix" ) continue;
      source.add(a[i]);
      model_suffix.add(suffix);
   }

   return;
}

////////////////////////////////////////////////////////////////////////

void set_data(const StringArray& a) {

   // Check for enough arguments
   // e.g. -data parent GFSO,AEMN gfs_file_list
   if(a.n() < 3) {
      mlog << Error << "\nset_data() -> "
           << "each \"-data\" command line option must specify a domain name, "
           << "a comma-separated list of ATCF tech ID's, and the corresponding "
           << "gridded data files.\n\n";
      usage();
   }

   // Store current -data options
   DataOptInfo info;

   // First argument is the domain name
   string domain = a[0];

   // Second argument is a comma-separated list of tech ID's
   info.tech_ids.parse_css(a[1]);

   // Remaining arguments are gridded data files or file lists
   StringArray sa;
   for(int i=2; i<a.n(); i++) sa.add(a[i]);
   info.data_files = parse_file_list(sa);

   // Sanity check that input files exist
   for(int i=0; i<info.data_files.n(); i++) {
      if(!file_exists(info.data_files[i].c_str())) {
         mlog << Warning << "\nset_data() -> "
              << "File does not exist: " << info.data_files[i] << "\n\n";
      }
   }

   // Store the data in the map
   if(data_opt_map.count(domain) == 0) data_opt_map[domain]  = info;
   else                                data_opt_map[domain] += info;

   return;
}

////////////////////////////////////////////////////////////////////////

void set_config(const StringArray& a) {
   config_file = a[0];
}

////////////////////////////////////////////////////////////////////////

void set_outdir(const StringArray& a) {
   out_dir = a[0];
}

////////////////////////////////////////////////////////////////////////

void setup_out_files(const TrackInfoArray &tracks) {
   OutFileInfo out_info;
   int i, j;

   // Setup output files for each track
   for(i=0; i<tracks.n(); i++) {

      // Build the map key
      ConcatString key = get_out_key(tracks[i]);

      // Check for duplicates
      if(out_file_map.count(key) > 0) {
         mlog << Error << "\nsetup_out_files()-> "
              << "found multiple tracks for key \""
              << key << "\"!\n\n";
         exit(1);
      }

      // Add new map entry
      out_file_map[key] = out_info;

      mlog << Debug(3) << "Preparing output files for "
           << key << " track.\n";

      // Store the track
      out_file_map[key].trk_ptr = &tracks[i];

      // NetCDF diagnostics output
      if(conf_info.nc_diag_flag) {
         out_file_map[key].nc_diag_file =
            build_out_file_name(out_file_map[key].trk_ptr, "_diag.nc");
         out_file_map[key].nc_diag_out =
            out_file_map[key].setup_nc_file(out_file_map[key].nc_diag_file);
      }

      // CIRA diagnostics output
      if(conf_info.cira_diag_flag) {
         out_file_map[key].cira_diag_file =
            build_out_file_name(out_file_map[key].trk_ptr, "_diag.txt");
         out_file_map[key].cira_diag_out = new ofstream;
         out_file_map[key].cira_diag_out->open(out_file_map[key].cira_diag_file);

         if(!(*out_file_map[key].cira_diag_out)) {
            mlog << Error << "\nsetup_out_files()-> "
                 << "can't open the output file \""
                 << out_file_map[key].cira_diag_file
                 << "\" for writing!\n\n";
            exit(1);
         }

         // Fixed width
         out_file_map[key].cira_diag_out->setf(ios::fixed);
      }
   } // end for i

   return;
}

////////////////////////////////////////////////////////////////////////

ConcatString get_out_key(const TrackInfo &track) {
   ConcatString cs;

   cs << track.storm_id() << "_"
      << track.technique() << "_"
      << unix_to_yyyymmddhh(track.init());

   return(cs);
}

////////////////////////////////////////////////////////////////////////

ConcatString get_tmp_key(const TrackInfo &track,
                         const TrackPoint &point,
                         const string &domain) {
   ConcatString cs;

   cs << track.storm_id() << "_"
      << track.technique() << "_"
      << unix_to_yyyymmddhh(track.init()) << "_f"
      << point.lead() /sec_per_hour << "_"
      << domain;

   return(cs);
}

////////////////////////////////////////////////////////////////////////

ConcatString build_tmp_file_name(const TrackInfo *trk_ptr,
                                 const TrackPoint *pnt_ptr,
                                 const string &domain) {
   ConcatString cs;

   // Build the temp file name with the program name,
   // track/timing information, and domain name

   cs << conf_info.tmp_dir
      << "/tmp_" << program_name << "_"
      << get_tmp_key(*trk_ptr, *pnt_ptr, domain);

   return(make_temp_file_name(cs.text(), ".nc"));
}

////////////////////////////////////////////////////////////////////////

ConcatString build_out_file_name(const TrackInfo *trk_ptr,
                                 const char *suffix) {
   ConcatString cs;

   // Build the output file name
   cs << out_dir << "/" << program_name;

   // Append the output prefix, if defined
   if(conf_info.output_prefix.nonempty()) {
      cs << "_" << conf_info.output_prefix;
   }

   // Append the track information
   cs << "_" << get_out_key(*trk_ptr);

   // Append the suffix
   cs << suffix;

   return(cs);
}

////////////////////////////////////////////////////////////////////////

void write_tc_storm(NcFile *nc_out, const char *storm_id,
                    const char *model, const char *domain) {

   // Add the storm id
   if(storm_id) {
      NcVar sid_var = nc_out->addVar("storm_id", ncString);
      sid_var.putVar(&storm_id);
   }

   // Add the model
   if(model) {
      NcVar mdl_var = nc_out->addVar("model", ncString);
      mdl_var.putVar(&model);
   }

   // Add the domain name
   if(domain) {
      NcVar dmn_var = nc_out->addVar("domain", ncString);
      dmn_var.putVar(&domain);
   }

   return;
}

////////////////////////////////////////////////////////////////////////

void write_tc_times(NcFile *nc_out, const NcDim &vld_dim,
                    const TrackInfo *trk_ptr,
                    const TrackPoint *pnt_ptr) {

   // Check pointer
   if(!trk_ptr) {
      mlog << Error << "\nwrite_tc_times() -> "
           << "null track pointer!\n\n";
      exit(1);
   }

   NcVar init_str_var, init_ut_var;
   NcVar vld_str_var, vld_ut_var;
   NcVar lead_str_var, lead_sec_var;

   // Define and write the track initialization time
   def_tc_init_time(nc_out, init_str_var, init_ut_var);
   write_tc_init_time(nc_out, init_str_var, init_ut_var,
                      trk_ptr->init());

   // Define valid and lead times
   def_tc_valid_time(nc_out, vld_dim, vld_str_var, vld_ut_var);
   def_tc_lead_time(nc_out, vld_dim, lead_str_var, lead_sec_var);

   // Write valid and lead times for a single point
   if(pnt_ptr) {
      write_tc_valid_time(nc_out, 0, vld_str_var, vld_ut_var,
                          pnt_ptr->valid());
      write_tc_lead_time(nc_out, 0, lead_str_var, lead_sec_var,
                         pnt_ptr->lead());
   }
   // Write valid and lead times for all track points
   else {
      for(int i_pnt=0; i_pnt<trk_ptr->n_points(); i_pnt++) {
         write_tc_valid_time(nc_out, i_pnt, vld_str_var, vld_ut_var,
                             (*trk_ptr)[i_pnt].valid());
         write_tc_lead_time(nc_out, i_pnt, lead_str_var, lead_sec_var,
                            (*trk_ptr)[i_pnt].lead());
      }
   }

   return;
}

////////////////////////////////////////////////////////////////////////

void compute_lat_lon(TcrmwGrid& grid,
                     double *lat_arr, double *lon_arr) {

   // Compute lat and lon coordinate arrays
   for(int ir=0; ir<grid.range_n(); ir++) {
      for(int ia=0; ia<grid.azimuth_n(); ia++) {
         double lat, lon;
         int i = ir * grid.azimuth_n() + ia;
         grid.range_azi_to_latlon(
            ir * grid.range_delta_km(),
            ia * grid.azimuth_delta_deg(),
            lat, lon);
         lat_arr[i] =  lat;
         lon_arr[i] = -lon; // degrees east to west
      }
   }

   return;
}

////////////////////////////////////////////////////////////////////////

void process_track_points(const TrackInfoArray& tracks) {
   int i, j, k, i_pnt, n_pts;
   TmpFileInfo tmp_info;
   map<string,DomainInfo>::iterator dom_it;

   // Build list of unique valid times
   TimeArray valid_ta;
   for(i=0,n_pts=0; i<tracks.n(); i++) {
      n_pts += tracks[i].n_points();
      for(j=0; j<tracks[i].n_points(); j++) {
         if(!valid_ta.has(tracks[i][j].valid())) {
            valid_ta.add(tracks[i][j].valid());
         }
      }
   }

   // Sort the valid times
   valid_ta.sort_array();

   mlog << Debug(2) << "Processing " << tracks.n()
        << " tracks consisting of " << n_pts
        << " points with " << valid_ta.n()
        << " unique valid times from "
        << unix_to_yyyymmddhh(valid_ta.min())
        << " to " << unix_to_yyyymmddhh(valid_ta.max())
        << ".\n";

   // Create temporary files for each TrackPoint/Domain

   // Loop over the unique valid times
   for(i=0; i<valid_ta.n(); i++) {

      // Loop over the domains to be processed
      for(j=0; j<conf_info.domain_info.size(); j++) {

         mlog << Debug(3) << "Processing valid time "
              << unix_to_yyyymmddhh(valid_ta[i]) << " \""
              << conf_info.domain_info[j].domain
              << "\" domain.\n";

         // Setup a temp file for this valid time in each track
         for(k=0; k<tracks.n(); k++) {

            // Find the track point for this valid time
            if((i_pnt = tracks[k].valid_index(valid_ta[i])) < 0) continue;

            // Build the map key
            ConcatString tmp_key = get_tmp_key(tracks[k],
                                               tracks[k][i_pnt],
                                               conf_info.domain_info[j].domain);

            // Check for duplicates
            if(tmp_file_map.count(tmp_key) > 0) {
               mlog << Error << "\nprocess_track_points()-> "
                    << "found multiple temp file entries for key \""
                    << tmp_key << "\"!\n\n";
               exit(1);
            }

            // Add new map entry
            tmp_file_map[tmp_key] = tmp_info;

            // Setup a temp file for the current point
            tmp_file_map[tmp_key].open(&tracks[k],
                                       &tracks[k][i_pnt],
                                       conf_info.domain_info[j],
                                       conf_info.pressure_levels);

         } // end for k
      } // end for j
   } // end for i

// TODO: Work on this parallel code

//#pragma omp parallel default(none)                      \
//   shared(mlog, conf_info, tracks, valid_ta)            \
//   private(i, dom_it)
//   {

      // Parallel: Loop over the unique valid times
//#pragma omp for schedule (static)
//#pragma omp parallel for
      for(i=0; i<valid_ta.n(); i++) {

         // Parallel: Loop over the domains to be processed
         for(j=0; j<conf_info.domain_info.size(); j++) {

            // Process the gridded data for the current
            // domain and valid time
            process_fields(tracks, valid_ta[i], i,
                           conf_info.domain_info[j].domain,
                           conf_info.domain_info[j]);

         } // end for j
      } // end for i
//   } // End of omp parallel

   return;
}

////////////////////////////////////////////////////////////////////////

void process_fields(const TrackInfoArray &tracks,
                    const unixtime vld_ut, int i_vld,
                    const string &domain,
                    const DomainInfo &di) {
   int i, j, i_pnt;
   DataPlane data_dp;
   Grid grid_dp;
   VarInfoFactory vi_factory;
   VarInfo *vi = (VarInfo *) 0;
   StringArray tmp_key_sa, fields_missing;

   // TODO: Consider adding vortex removal logic here
   // Read in the full set of fields required for vortex removal
   // Add flag to configure which fields are used for vortex removal

   // TODO: Add logic to support one_time_per_file_flag set to true.

   // Loop over the VarInfo fields to be processed
   for(i=0; i<di.var_info_ptr.size(); i++) {

      // Make a local VarInfo copy to store the valid time
      vi = vi_factory.new_copy(di.var_info_ptr[i]);
      vi->set_valid(vld_ut);

      // Find data for this track point
      bool status = get_series_entry(i_vld, vi,
                       di.data_files, file_type,
                       data_dp, grid_dp,
                       false, false);

      // Keep track of missing fields
      if(!status) {
         fields_missing.add(vi->magic_str());

         // Store the requested valid time
         data_dp.set_valid(vld_ut);
      }

      // Do coordinate transformation for each track point
      for(j=0; j<tracks.n(); j++) {

         // Find the track point for this valid time
         if((i_pnt = tracks[j].valid_index(vld_ut)) < 0) continue;

         // Build the map key
         ConcatString tmp_key = get_tmp_key(tracks[j],
                                            tracks[j][i_pnt],
                                            domain);

         // Store unique keys
         if(!tmp_key_sa.has(tmp_key)) tmp_key_sa.add(tmp_key);

         // TODO: Consider adding vortex removal logic here
         // Assume that it applies to each track point location independently.
         // Need to load multiple fields for the vortex removal logic.
         // Perhaps do 2 passes... process the vortex removal first?

         // Compute and write the cylindrical coordinate data
         tmp_file_map[tmp_key].write_nc_data(vi, data_dp, grid_dp);

      } // end for j

      // Deallocate memory
      delete vi;
      vi = (VarInfo *) 0;

   } // end for i

   // Print warning for missing fields
   if(fields_missing.n() > 0) {
      mlog << Warning << "For the "
           << domain << " domain, "
           << sec_to_hhmmss(vld_ut - init_ut) << " lead time, and "
           << unix_to_yyyymmdd_hhmmss(vld_ut) << " valid time, "
           << fields_missing.n() << " of " << di.var_info_ptr.size()
           << " requested fields missing:\n"
           << write_css(fields_missing) << "\n";
   }

   // Loop over the current set of temp files
   for(i=0; i<tmp_key_sa.n(); i++) {

      // Run the python diagnostic scripts
      for(j=0; j<di.diag_script.n(); j++) {

         python_tc_diag(di.diag_script[j].c_str(),
            tmp_file_map[tmp_key_sa[i]].tmp_file,
            tmp_file_map[tmp_key_sa[i]].diag_map);

      } // end for j
   } // end for i

   return;
}

////////////////////////////////////////////////////////////////////////

void process_out_files(const TrackInfoArray& tracks) {
   const char *method_name = "process_out_files()";
   vector<TmpFileInfo *> domain_tmp_file_list;

   // Loop over tracks
   for(int i_trk=0; i_trk<tracks.n(); i_trk++) {

      // Output file key
      ConcatString out_key = get_out_key(tracks[i_trk]);

      // Error check
      if(out_file_map.count(out_key) == 0) {
         mlog << Error << "\n" << method_name << " -> "
              << "no output file map entry found for key \""
              << out_key << "\"!\n\n";
         exit(1);
      }

      // Loop over domains
      for(int i_dom=0; i_dom<conf_info.domain_info.size(); i_dom++) {

         // Initialize list of domain-specific temp files
         domain_tmp_file_list.clear();

         // Loop over track points
         for(int i_pnt=0; i_pnt<tracks[i_trk].n_points(); i_pnt++) {

            // Temp file key
            ConcatString tmp_key = get_tmp_key(tracks[i_trk],
                                               tracks[i_trk][i_pnt],
                                               conf_info.domain_info[i_dom].domain);

            // Error check
            if(tmp_file_map.count(tmp_key) == 0) {
                mlog << Error << "\n" << method_name << " -> "
                     << "no temporary file map entry found for key \""
                     << tmp_key << "\"!\n\n";
                exit(1);
            }

            // Update list of domain-specific temp files
            domain_tmp_file_list.push_back(&tmp_file_map[tmp_key]);

            // Store the diagnostics for each track point
            out_file_map[out_key].add_diag_map(tmp_file_map[tmp_key].diag_map, i_pnt);

         } // end for i_pnt

         // Write NetCDF range-azimuth output
         if(conf_info.nc_rng_azi_flag) {
            merge_tmp_files(domain_tmp_file_list);
         }

      } // end for i_dom

      // Write NetCDF diagnostics output
      if(conf_info.nc_diag_flag) {
         out_file_map[out_key].write_nc_diag();
      }

      // Finish the output for this track
      out_file_map[out_key].clear();

   } // end for i_trk

   return;
}

////////////////////////////////////////////////////////////////////////

void merge_tmp_files(const vector<TmpFileInfo *> tmp_files) {
   NcFile *nc_out = (NcFile *) 0;

   // Loop over temp files
   for(int i_tmp=0; i_tmp<tmp_files.size(); i_tmp++) {

      // Create the output NetCDF file
      if(!nc_out) {
         ConcatString suffix_cs, file_name;
         suffix_cs << "_cyl_grid_"
                   << tmp_files[i_tmp]->domain << ".nc";
         file_name = build_out_file_name(
                        tmp_files[i_tmp]->trk_ptr,
                        suffix_cs.c_str());

         mlog << Debug(1) << "Writing output file: "
              << file_name << "\n";

         nc_out = open_ncfile(file_name.c_str(), true);

         if(IS_INVALID_NC_P(nc_out)) {
            mlog << Error << "\nmerge_tmp_files() -> "
                 << "trouble opening output NetCDF file "
                 << file_name << "\n\n";
            exit(1);
         }

         // Add global attributes
         write_netcdf_global(nc_out, file_name.c_str(), program_name);

         // Write track info
         write_tc_storm(nc_out,
                        tmp_files[i_tmp]->trk_ptr->storm_id().c_str(),
                        tmp_files[i_tmp]->trk_ptr->technique().c_str(),
                        nullptr);

         // Write the track lines
         write_tc_track_lines(nc_out,
                              *(tmp_files[i_tmp]->trk_ptr));

         // Define the time dimension
         NcDim vld_dim = add_dim(nc_out, "time",
                                 tmp_files[i_tmp]->trk_ptr->n_points());

         // Write timing info for the entire track
         write_tc_times(nc_out, vld_dim,
                        tmp_files[i_tmp]->trk_ptr, nullptr);

         // Copy coordinate variables
         copy_coord_vars(nc_out, tmp_files[i_tmp]->tmp_out);

      } // end if !nc_out

      // Copy time variables
      copy_time_vars(nc_out, tmp_files[i_tmp]->tmp_out, i_tmp);

   } // end for i_tmp

   return;
}

////////////////////////////////////////////////////////////////////////

void copy_coord_vars(NcFile *to_nc, NcFile *from_nc) {

   // Get the input variable names
   StringArray var_names;
   get_var_names(from_nc, &var_names);

   // Loop over the variables
   for(int i=0; i<var_names.n(); i++) {

      // Skip non-coordinate variables
      if(!has_dim(from_nc, var_names[i].c_str())) continue;

      // Get the current coordinate variable
      NcVar from_var = get_var(from_nc, var_names[i].c_str());
      NcVar *to_var = copy_nc_var(to_nc, &from_var);
   }

   return;
}


////////////////////////////////////////////////////////////////////////

void copy_time_vars(NcFile *to_nc, NcFile *from_nc, int i_time) {
   int i, j;

   // Get the input variable names
   StringArray var_names;
   get_var_names(from_nc, &var_names);

   // Loop over the variables
   for(i=0; i<var_names.n(); i++) {

      // Current source variable
      NcVar from_var = get_var(from_nc, var_names[i].c_str());

      // Get dimensions
      StringArray dim_names;
      get_dim_names(&from_var, &dim_names);

      // Skip time variable names and variables without the time dimension
      if(var_names[i].find("time") != string::npos ||
         !dim_names.has("time")) {
         continue;
      }

      // Add new variable, if needed
      if(!has_var(to_nc, var_names[i].c_str())) {
         vector<NcDim> dims;
         for(j=0; j<dim_names.n(); j++) {
            dims.push_back(get_nc_dim(to_nc, dim_names[j]));
         }
         NcVar new_var = to_nc->addVar(var_names[i], ncDouble, dims);
         copy_nc_atts(&from_var, &new_var);
      }

      // Write data for the current time
      NcVar to_var = get_var(to_nc, var_names[i].c_str());

      vector<size_t> offsets;
      vector<size_t> counts;

      int buf_size = 1;

      for(j=0; j<dim_names.n(); j++) {
         if(dim_names[j] == "time") {
            offsets.push_back(i_time);
            counts.push_back(1);
         }
         else {
            offsets.push_back(0);
            NcDim cur_dim = get_nc_dim(to_nc, dim_names[j]);
            int dim_size = get_dim_size(&cur_dim);
            buf_size *= dim_size;
            counts.push_back(dim_size);
         }
      }

      // Allocate buffer
      double *buf = new double[buf_size];

      // Copy the data for this time slice
      get_nc_data(&from_var, buf);
      to_var.putVar(offsets, counts, buf);

      // Cleanup
      if(buf) { delete[] buf; buf = (double *) 0; }

   } // end for i

   return;
}

////////////////////////////////////////////////////////////////////////
//
//  Code for class OutFileInfo
//
////////////////////////////////////////////////////////////////////////

OutFileInfo::OutFileInfo() {
   init_from_scratch();
}

////////////////////////////////////////////////////////////////////////

OutFileInfo::~OutFileInfo() {
   clear();
}

////////////////////////////////////////////////////////////////////////

void OutFileInfo::init_from_scratch() {

   // Initialize track pointer
   trk_ptr = (TrackInfo *) 0;

   // Initialize output file stream pointers
   nc_diag_out   = (NcFile *) 0;
   cira_diag_out = (ofstream *) 0;

   clear();

   return;
}

////////////////////////////////////////////////////////////////////////

void OutFileInfo::clear() {

   trk_ptr = (TrackInfo *) 0;

   // Clear the diagnostics map
   diag_map.clear();

   // Write NetCDF diagnostics file
   if(nc_diag_out) {

      mlog << Debug(1) << "Writing output file: "
           << nc_diag_file << "\n";

      // Close the output file
      nc_diag_out->close();
      delete nc_diag_out;
      nc_diag_out = (NcFile *) 0;
   }
   nc_diag_file.clear();

   // Write CIRA diagnostics file
   if(cira_diag_out) {

      mlog << Debug(1) << "Writing output file: "
           << cira_diag_file << "\n";

      // Write the output
      *cira_diag_out << cira_diag_at;

      // Close the output file
      cira_diag_out->close();
      delete cira_diag_out;
      cira_diag_out = (ofstream *) 0;
   }
   cira_diag_file.clear();
   cira_diag_at.clear();

   return;
}

////////////////////////////////////////////////////////////////////////

NcFile *OutFileInfo::setup_nc_file(const string &out_file) {

   if(!trk_ptr) return(nullptr);

   // Open the output NetCDF file
   NcFile *nc_out = open_ncfile(out_file.c_str(), true);

   if(IS_INVALID_NC_P(nc_out)) {
      mlog << Error << "\nOutFileInfo::setup_nc_file() -> "
           << "trouble opening output NetCDF file "
           << out_file << "\n\n";
      exit(1);
   }

   // Add global attributes
   write_netcdf_global(nc_out, out_file.c_str(), program_name);

   // Define dimension
   vld_dim = add_dim(nc_out, "time",
                     trk_ptr->n_points());

   // Write track info
   write_tc_storm(nc_out,
                  trk_ptr->storm_id().c_str(),
                  trk_ptr->technique().c_str(),
                  nullptr);

   // Write timing info for the entire track
   write_tc_times(nc_out, vld_dim,
                  trk_ptr, nullptr);

   return(nc_out);
}

////////////////////////////////////////////////////////////////////////

void OutFileInfo::add_diag_map(const map<string,double> &tmp_diag_map,
                               int i_pnt) {

   // Track pointer must be set
   if(!trk_ptr) {
      mlog << Error << "\nOutFileInfo::add_diag_map() -> "
           << "track pointer not set!\n\n";
      exit(1);
   }

   // Check the range
   if(i_pnt < 0 || i_pnt >= trk_ptr->n_points()) {
      mlog << Error << "\nOutFileInfo::add_diag_map() -> "
           << "track point index (" << i_pnt
           << ") range check error!\n\n";
      exit(1);
   }

   // Loop over the input diagnostics map
   map<string,double>::const_iterator it;
   for(it = tmp_diag_map.begin(); it != tmp_diag_map.end(); it++) {

      // Add new diagnostics array entry, if needed
      if(diag_map.count(it->first) == 0) {
         NumArray empty_na;
         empty_na.set_const(bad_data_double,
                            trk_ptr->n_points());
         diag_map[it->first] = empty_na;
      }

      // Store the diagnostic value for the track point
      diag_map[it->first].set(i_pnt, it->second);
   }

   return;
}

////////////////////////////////////////////////////////////////////////

void OutFileInfo::write_nc_diag() {

   // Setup dimensions
   vector<NcDim> dims;
   dims.push_back(vld_dim);

   vector<size_t> offsets;
   offsets.push_back(0);
   
   vector<size_t> counts;
   counts.push_back(get_dim_size(&vld_dim));

   // Write the diagnostics for each lead time
   map<string,NumArray>::iterator it;
   for(it = diag_map.begin(); it != diag_map.end(); it++) {
      NcVar diag_var = nc_diag_out->addVar(it->first, ncDouble, dims);
      add_att(&diag_var, fill_value_att_name, bad_data_double);
      diag_var.putVar(offsets, counts, it->second.buf());
   }

   return;
}

////////////////////////////////////////////////////////////////////////
//
//  Code for class TmpFileInfo
//
////////////////////////////////////////////////////////////////////////

TmpFileInfo::TmpFileInfo() {
   init_from_scratch();
}

////////////////////////////////////////////////////////////////////////

TmpFileInfo::~TmpFileInfo() {
   clear();
}

////////////////////////////////////////////////////////////////////////

void TmpFileInfo::init_from_scratch() {

   // Initialize pointers
   trk_ptr = (TrackInfo *) 0;
   pnt_ptr = (TrackPoint *) 0;
   tmp_out = (NcFile *) 0;

   clear();

   return;
}

////////////////////////////////////////////////////////////////////////

void TmpFileInfo::open(const TrackInfo *t_ptr,
                       const TrackPoint *p_ptr,
                       const DomainInfo &di,
                       const set<double> &prs_lev) {

   // Set pointers
   trk_ptr = t_ptr;
   pnt_ptr = p_ptr;
   domain  = di.domain;

   // Open the temp file
   tmp_file = build_tmp_file_name(trk_ptr, pnt_ptr, domain);

   mlog << Debug(3) << "Creating temp file: " << tmp_file << "\n";

   setup_nc_file(di, prs_lev);

   return;
}

////////////////////////////////////////////////////////////////////////

void TmpFileInfo::close() {

   // Write NetCDF temp file
   if(tmp_out) {

      mlog << Debug(3) << "Writing temp file: "
           << tmp_file << "\n";

      delete tmp_out;
      tmp_out = (NcFile *) 0;
   }

   return;
}

////////////////////////////////////////////////////////////////////////

void TmpFileInfo::clear() {

   trk_ptr = (TrackInfo *) 0;
   pnt_ptr = (TrackPoint *) 0;

   diag_map.clear();

   grid_out.clear();
   ra_grid.clear();

   pressure_levels.clear();

   domain.clear();

   // Delete the temp file
   if(tmp_out) {

      remove_temp_file(tmp_file);

      tmp_out = (NcFile *) 0;
   }
   tmp_file.clear();

   return;
}

////////////////////////////////////////////////////////////////////////

void TmpFileInfo::setup_nc_file(const DomainInfo &di,
                                const set<double> &prs_lev) {

   // Open the output NetCDF file
   tmp_out = open_ncfile(tmp_file.c_str(), true);

   if(IS_INVALID_NC_P(tmp_out)) {
      mlog << Error << "\nTmpFileInfo::setup_nc_file() -> "
           << "trouble opening output NetCDF file "
           << tmp_file << "\n\n";
      exit(1);
   }

   // Add global attributes
   write_netcdf_global(tmp_out, tmp_file.c_str(), program_name);

   // Define latitude and longitude arrays
   TcrmwData d = di.data;
   int nra = d.range_n * d.azimuth_n;
   double *lat_arr = new double[nra];
   double *lon_arr = new double[nra];

   // Set grid center
   d.lat_center   =      pnt_ptr->lat();
   d.lon_center   = -1.0*pnt_ptr->lon(); // degrees east to west
   d.range_max_km = di.delta_range_km * d.range_n;

   // Instantiate the grid
   grid_out.set(d);
   ra_grid.set_from_data(d);

   mlog << Debug(3)
        << "Defining cylindrical coordinates for (Lat, Lon) = ("
        << pnt_ptr->lat() << ", " << pnt_ptr->lon() << "), Range = "
        << ra_grid.range_n() << " every " << ra_grid.range_delta_km()
        << ra_grid.range_n() << " every " << ra_grid.range_delta_km()
        << "km, Azimuth = " << ra_grid.azimuth_n() << "\n";

   // Write track info
   write_tc_storm(tmp_out,
                  trk_ptr->storm_id().c_str(),
                  trk_ptr->technique().c_str(),
                  di.domain.c_str());

   // Write the track lines
   write_tc_track_lines(tmp_out, *trk_ptr);

   // Define dimensions
   trk_dim = add_dim(tmp_out, "track_point",
                     trk_ptr->n_points());
   vld_dim = add_dim(tmp_out, "time", 1);
   rng_dim = add_dim(tmp_out, "range",
                     (long) ra_grid.range_n());
   azi_dim = add_dim(tmp_out, "azimuth",
                     (long) ra_grid.azimuth_n());

   // Write the track locations
   write_tc_track_lat_lon(tmp_out, trk_dim, *trk_ptr);

   // Write timing info for this TrackPoint
   write_tc_times(tmp_out, vld_dim, trk_ptr, pnt_ptr);

   // Define range and azimuth coordinate variables
   def_tc_range_azimuth(tmp_out,
                        rng_dim, azi_dim,
                        ra_grid, bad_data_double);

   // Pressure dimension and values (same for all temp files)
   pressure_levels = prs_lev;
   if(pressure_levels.size() > 0) {
      prs_dim = add_dim(tmp_out, "pressure",
                        (long) pressure_levels.size());
      def_tc_pressure(tmp_out, prs_dim, pressure_levels);
   }

   // Define latitude and longitude
   NcVar lat_var, lon_var;
   def_tc_lat_lon(tmp_out, vld_dim, rng_dim, azi_dim,
                  lat_var, lon_var);

   // Compute lat and lon coordinate arrays
   compute_lat_lon(ra_grid, lat_arr, lon_arr);

   // Write coordinate arrays
   write_tc_data(tmp_out, ra_grid, 0, lat_var, lat_arr);
   write_tc_data(tmp_out, ra_grid, 0, lon_var, lon_arr);

   // Write track point values
   write_tc_track_point(tmp_out, vld_dim, *pnt_ptr);

   // Clean up
   if(lat_arr) { delete[] lat_arr; lat_arr = (double *) 0; }
   if(lon_arr) { delete[] lon_arr; lon_arr = (double *) 0; }

   return;
}

////////////////////////////////////////////////////////////////////////

void TmpFileInfo::write_nc_data(const VarInfo *vi, const DataPlane &dp_in,
                                const Grid &grid_in) {
   DataPlane dp_out;
   RegridInfo ri;

   // Use default regridding options
   ri.method     = InterpMthd_Nearest;
   ri.width      = 1;
   ri.vld_thresh = 1.0;
   ri.shape      = GridTemplateFactory::GridTemplate_Square;

   // Do the cylindrical coordinate transformation
   if(dp_in.nxy() > 0) {
      dp_out = met_regrid(dp_in, grid_in, grid_out, ri);
   }
   // Handle empty input fields
   else {
      dp_out.set_valid(dp_in.valid());
      dp_out.set_size(ra_grid.range_n(), ra_grid.azimuth_n(),
                      bad_data_double);
   }

   // Logic for pressure level data
   bool is_prs = (vi->level().type() == LevelType_Pres);

   // Setup dimensions
   vector<NcDim> dims;
   dims.push_back(vld_dim);
   if(is_prs) dims.push_back(prs_dim);
   dims.push_back(rng_dim);
   dims.push_back(azi_dim);

   // Create the output variable name
   ConcatString var_name;
   var_name << vi->name_attr();
   if(!is_prs) {
      var_name << "_" << vi->level_attr();
   }

   // Add new variable, if needed
   if(!has_var(tmp_out, var_name.c_str())) {
      NcVar new_var = tmp_out->addVar(var_name, ncDouble, dims);
      add_att(&new_var, long_name_att_name, vi->long_name_attr());
      add_att(&new_var, units_att_name, vi->units_attr());
      add_att(&new_var, fill_value_att_name, bad_data_double);
   }

   // Get the current variable
   NcVar cur_var = get_var(tmp_out, var_name.c_str());

   // Write pressure level data
   if(is_prs) {

      // Find pressure level index
      int i_level = pressure_levels.size() - 1;
      for(set<double>::iterator it = pressure_levels.begin();
          it != pressure_levels.end(); ++it, --i_level) {
         if(is_eq(vi->level().lower(), *it)) break;
      }

      write_tc_pressure_level_data(tmp_out, ra_grid,
         0, i_level, cur_var, dp_out.data());
   }
   // Write single level data
   else {
      write_tc_data_rev(tmp_out, ra_grid,
         0, cur_var, dp_out.data());
   }

   return;
}

////////////////////////////////////////////////////////////////////////
