// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2020
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*

////////////////////////////////////////////////////////////////////////
//
//   Filename:   mtd.cc
//
//   Description:
//     Define 3-dimensional space/time objects.
//
//   Mod#   Date      Name           Description
//   ----   ----      ----           -----------
//   000    10-15-15  Bullock        New
//   001    05-15-17  Prestopnik P.  Added regrid shape
//   002    04-18-19  Halley Gotway  Add FCST and OBS units.
//   003    04-25-19  Halley Gotway  Add percentiles to 2D output.
//
////////////////////////////////////////////////////////////////////////


static const char default_config_path          [] = "MET_BASE/config/MTDConfig_default";

static const char txt_2d_suffix                [] = "2d.txt";

static const char txt_3d_single_simple_suffix  [] = "3d_single_simple.txt";
static const char txt_3d_pair_simple_suffix    [] = "3d_pair_simple.txt";

static const char txt_3d_single_cluster_suffix [] = "3d_single_cluster.txt";
static const char txt_3d_pair_cluster_suffix   [] = "3d_pair_cluster.txt";

static const char nc_suffix                    [] = "obj.nc";

static const char default_prefix               [] = "mtd";


////////////////////////////////////////////////////////////////////////


using namespace std;

#include <iostream>
#include <unistd.h>
#include <stdlib.h>
#include <cmath>

#include "vx_data2d_factory.h"
#include "apply_mask.h"
#include "mtd_config_info.h"
#include "mtd_file.h"
#include "interest_calc.h"
#include "3d_att_single_array.h"
#include "mtd_txt_output.h"
#include "mtd_read_data.h"
#include "mm_engine.h"
#include "mtd_nc_output.h"


////////////////////////////////////////////////////////////////////////


static ConcatString program_name;

static CommandLine cline;

static ConcatString output_directory = (string)".";

static StringArray fcst_filenames;
static StringArray  obs_filenames;

static StringArray  single_filenames;

static ConcatString local_config_filename;


////////////////////////////////////////////////////////////////////////


static void usage();

static void set_fcst      (const StringArray &);
static void set_obs       (const StringArray &);
static void set_single    (const StringArray &);
static void set_config    (const StringArray &);

static void set_verbosity (const StringArray &);
static void set_logfile   (const StringArray &);
static void set_outdir    (const StringArray &);

static ConcatString make_output_prefix(const MtdConfigInfo &, unixtime start_time);

static void do_single_field(MtdConfigInfo &);


////////////////////////////////////////////////////////////////////////


int main(int argc, char * argv [])

{

program_name = get_short_name(argv[0]);

cline.set(argc, argv);

cline.set_usage(usage);

cline.add(set_fcst,      "-fcst",   -1);
cline.add(set_obs,       "-obs",    -1);
cline.add(set_single,    "-single", -1);
cline.add(set_config,    "-config",  1);
cline.add(set_verbosity, "-v",       1);
cline.add(set_logfile,   "-log",     1);
cline.add(set_outdir,    "-outdir",  1);

cline.parse();

if ( cline.n() != 0 )  usage();

if ( single_filenames.n() == 0 )  {

   if ( fcst_filenames.n() == 0 )  usage();
   if (  obs_filenames.n() == 0 )  usage();

}

if ( output_directory.empty() )  output_directory = ".";

   //
   //  read the config file
   //

MtdConfigInfo config;
ConcatString default_config_filename;
ConcatString path;

if ( local_config_filename.empty() )  {

   mlog << Error
        << "\n  " << program_name << ": must specify a configuration "
        << "file using the \"-config\" command line option.\n\n";

   exit ( 1 );

}

default_config_filename = replace_path(default_config_path);

config.read_config(default_config_filename.c_str(), local_config_filename.c_str());



   //
   //  determine the input file types
   //    - check the config file for the file_type
   //    - check the first data file
   //


   //
   //  if we're doing a single field, handle this separately
   //

if ( single_filenames.n() > 0 )  {

   GrdFileType stype;

   stype = parse_conf_file_type(config.conf.lookup_dictionary(conf_key_fcst));   //  use the "fcst" dictionary

   single_filenames = parse_file_list(single_filenames);

   if ( stype == FileType_None ) stype = grd_file_type(single_filenames[0].c_str());

   config.process_config(stype, stype);

   do_single_field(config);

   return ( 0 );

}

   //
   //  parse the forecast and observation file lists
   //

fcst_filenames = parse_file_list(fcst_filenames);
obs_filenames  = parse_file_list(obs_filenames);

GrdFileType ftype, otype;

ftype = parse_conf_file_type(config.conf.lookup_dictionary(conf_key_fcst));
otype = parse_conf_file_type(config.conf.lookup_dictionary(conf_key_obs));

if ( ftype == FileType_None ) ftype = grd_file_type(fcst_filenames[0].c_str());
if ( otype == FileType_None ) otype = grd_file_type(obs_filenames[0].c_str());

config.process_config(ftype, otype);

const double ti_thresh = config.total_interest_thresh;


int j, k;
MtdFloatFile fcst_raw, obs_raw;
MtdFloatFile fcst_conv, obs_conv;
MtdIntFile fcst_mask, obs_mask;
MtdIntFile fcst_obj, obs_obj;
MM_Engine engine;


   //
   //  read the data files
   //

mtd_read_data(config, *(config.fcst_info), fcst_filenames, fcst_raw);

mtd_read_data(config, *(config.obs_info),   obs_filenames,  obs_raw);

if ( fcst_raw.nt() != obs_raw.nt() )  {

   mlog << Error
        << "\n  " << program_name << ": forecast and observation must have the same number of times ("
        << fcst_raw.nt() << " != " << obs_raw.nt() << ")!\n\n";

   exit ( 1 );

}

if ( fcst_raw.delta_t() != obs_raw.delta_t() )  {

   mlog << Error
        << "\n  " << program_name << ": forecast time difference is different than observation time difference ("
        << fcst_raw.delta_t() << " seconds != " << obs_raw.delta_t() << " seconds)!\n\n";

   exit ( 1 );

}

config.delta_t_seconds = fcst_raw.delta_t();

   //
   //  regrid, if necessary
   //

mlog << Debug(2) << "regridding, if needed ...\n";

const Grid to_grid = parse_vx_grid(config.fcst_info->regrid(),
                                   fcst_raw.grid_p(), obs_raw.grid_p());

fcst_raw.regrid(to_grid, config.fcst_info->regrid());
 obs_raw.regrid(to_grid, config.obs_info->regrid());

   //
   //  make the output file prefix
   //

ConcatString prefix;
// int year, month, day, hour, minute, second;
// char junk[256];

prefix = make_output_prefix(config, obs_raw.start_valid_time());


   //
   //  set up the total interest calculator
   //

engine.calc.add(config.space_centroid_dist_wt, config.space_centroid_dist_if, &PairAtt3D::SpaceCentroidDist);
engine.calc.add(config.time_centroid_delta_wt, config.time_centroid_delta_if, &PairAtt3D::TimeCentroidDelta);
engine.calc.add(config.speed_delta_wt,         config.speed_delta_if,         &PairAtt3D::SpeedDelta);
engine.calc.add(config.direction_diff_wt,      config.direction_diff_if,      &PairAtt3D::DirectionDifference);
engine.calc.add(config.volume_ratio_wt,        config.volume_ratio_if,        &PairAtt3D::VolumeRatio);
engine.calc.add(config.axis_angle_diff_wt,     config.axis_angle_diff_if,     &PairAtt3D::AxisDiff);
engine.calc.add(config.start_time_delta_wt,    config.start_time_delta_if,    &PairAtt3D::StartTimeDelta);
engine.calc.add(config.end_time_delta_wt,      config.end_time_delta_if,      &PairAtt3D::EndTimeDelta);

   //
   //  make sure that not all of the weights are zero
   //

engine.calc.check();

   //
   //  convolve
   //

 obs_conv =  obs_raw.convolve(config.obs_conv_radius, config.obs_conv_time_beg, config.obs_conv_time_end);
fcst_conv = fcst_raw.convolve(config.fcst_conv_radius, config.fcst_conv_time_beg, config.fcst_conv_time_end);

   //
   //  threshold
   //

fcst_mask = fcst_conv.threshold(config.fcst_conv_thresh);
 obs_mask =  obs_conv.threshold(config.obs_conv_thresh);

   //
   //  number the objects
   //

fcst_obj = fcst_mask;
 obs_obj =  obs_mask;

mlog << Debug(2) << "Splitting fcst object field\n";
   fcst_obj.split();
mlog << Debug(2) << "Done splitting fcst\n";
mlog << Debug(2) << "Splitting obs object field\n";
    obs_obj.split();
mlog << Debug(2) << "Done splitting obs\n";

   //
   //  toss small objects
   //

fcst_obj.toss_small_objects(config.min_volume);
 obs_obj.toss_small_objects(config.min_volume);

const bool have_pairs =    (fcst_obj.n_objects() != 0)
                        && ( obs_obj.n_objects() != 0);

mlog << Debug(5) << "have_pairs = " << bool_to_string(have_pairs) << '\n';

   //
   //  set up the match/merge engine
   //

if ( have_pairs )  engine.set_size(fcst_obj.n_objects(), obs_obj.n_objects());

   //
   //  get single attributes
   //

SingleAtt3D att_3;
SingleAtt3DArray fcst_single_att, obs_single_att;
Object mask;

mlog << Debug(2)
     << "Calculating 3D fcst single attributes\n";

for (j=0; j<(fcst_obj.n_objects()); ++j)  {

   mask = fcst_obj.select(j + 1);   //  1-based

   att_3 = calc_3d_single_atts(mask, fcst_raw, config.model.c_str(), config.inten_perc_value);

   att_3.set_object_number(j + 1);   //  1-based

   att_3.set_fcst();

   att_3.set_simple();

   fcst_single_att.add(att_3);

}



mlog << Debug(2)
     << "Calculating 3D obs single attributes\n";

for (j=0; j<(obs_obj.n_objects()); ++j)  {

   mask = obs_obj.select(j + 1);   //  1-based

   att_3 = calc_3d_single_atts(mask, obs_raw, config.model.c_str(), config.inten_perc_value);

   att_3.set_object_number(j + 1);   //  1-based

   att_3.set_obs();

   att_3.set_simple();

   obs_single_att.add(att_3);

}



   //
   //  get simple pair attributes
   //

PairAtt3DArray pa_simple;
PairAtt3D p;
MtdIntFile fo, oo;

if ( have_pairs )  {

   // mlog << Debug(5) << "\n  Calculating pair attributes ... (Nf = "
   //      << (fcst_obj.n_objects()) << ", No = "
   //      << (obs_obj.n_objects())  << ")\n\n";

   for (j=0; j<(fcst_obj.n_objects()); ++j)  {

      fo = fcst_obj.select(j + 1);

      for (k=0; k<(obs_obj.n_objects()); ++k)  {

         oo = obs_obj.select(k + 1);

         p = calc_3d_pair_atts(fo, oo, fcst_single_att[j], obs_single_att[k]);

         p.set_total_interest(engine.calc(p));

         p.set_simple();

         // mlog << Debug(5) << "   (F_" << j << ", O_" << k << ")   "
         //      << p.total_interest() << '\n';

         pa_simple.add(p);

      }

   }

}   //  if have_pairs



   //
   //  calculate 2d simple attributes
   //

int t;
SingleAtt2DArray fcst_simple_att_2d, obs_simple_att_2d;
SingleAtt2D att_2;
MtdIntFile mask_2d;
DataPlane raw_2d;

   //   fcst simple objects

mlog << Debug(2) << "Calculating 2D simple fcst attributes\n";

for (j=0; j<(fcst_obj.n_objects()); ++j)  {

   att_3 = fcst_single_att[j];

   for (t=(att_3.tmin()); t<=(att_3.tmax()); ++t)  {

      mask_2d = fcst_obj.const_t_mask(t, j + 1);   //  1-based

      fcst_raw.get_data_plane(t, raw_2d);

      att_2 = calc_2d_single_atts(mask_2d, raw_2d, j + 1, config.inten_perc_value);

      att_2.set_fcst();

      att_2.set_valid_time(fcst_obj.start_valid_time() + t*(fcst_obj.delta_t()));

      att_2.set_lead_time(fcst_obj.lead_time(t));

      att_2.set_object_number(j + 1);   //  1-based

      att_2.set_time_index(t);

      att_2.set_is_simple();

      fcst_simple_att_2d.add(att_2);

   }   //  for k

}   //  for j

   //   obs simple objects

mlog << Debug(2) << "Calculating 2D simple obs attributes\n";

for (j=0; j<(obs_obj.n_objects()); ++j)  {

   att_3 = obs_single_att[j];

   for (t=(att_3.tmin()); t<=(att_3.tmax()); ++t)  {

      mask_2d = obs_obj.const_t_mask(t, j + 1);   //  1-based

      obs_raw.get_data_plane(t, raw_2d);

      att_2 = calc_2d_single_atts(mask_2d, raw_2d, j + 1, config.inten_perc_value);

      att_2.set_obs();

      att_2.set_valid_time(obs_obj.start_valid_time() + t*(obs_obj.delta_t()));

      att_2.set_lead_time(obs_obj.lead_time(t));

      att_2.set_object_number(j + 1);   //  1-based

      att_2.set_time_index(t);

      att_2.set_is_simple();

      obs_simple_att_2d.add(att_2);

   }   //  for k

}   //  for j





   //
   //  create graph
   //

IntArray a;
int n_clusters = 0;

if ( have_pairs )  {

   for (j=0; j<(pa_simple.n()); ++j)  {

      if ( pa_simple.total_interest(j) < ti_thresh )  continue;

      engine.graph.set_fo_edge(pa_simple.fcst_obj_number(j) - 1, pa_simple.obs_obj_number(j) - 1);

   }   //  for j

   engine.graph.dump_as_table(6);

   engine.do_match_merge();

   n_clusters = engine.n_composites();

   engine.partition_dump(6);

   mlog << Debug(2) << "N clusters = " << n_clusters << '\n';

   if ( mlog.verbosity_level() > 5 )  {

      for (j=0; j<n_clusters; ++j)  {

         // mlog << Debug(5) << "Fcst objects in composite " << j << ":  ";

         a = engine.fcst_composite(j);

         // a.dump_one_line(cout);

         // mlog << Debug(5) << "Obs  objects in composite " << j << ":  ";

         a = engine.obs_composite(j);

         // a.dump_one_line(cout);

         // mlog << Debug(5) << '\n';

      }

   }   //  if

   // mlog << Debug(2) << "N composites = " << e.n_composites() << "\n";

}   //  if have_pairs

   //
   //  get single cluster attributes
   //

SingleAtt3DArray fcst_cluster_att, obs_cluster_att;

if ( have_pairs )  {

   mlog << Debug(2)
        << "Calculating 3D fcst cluster attributes\n";

   for (j=0; j<n_clusters; ++j)  {

      a = engine.fcst_composite(j);   //  0-based

      a.increment(1);

      mask = fcst_obj.select_cluster(a);   //  1-based

      att_3 = calc_3d_single_atts(mask, fcst_raw, config.model.c_str(), config.inten_perc_value);

      att_3.set_object_number(j + 1);   //  1-based

      att_3.set_fcst();

      att_3.set_cluster();

      fcst_cluster_att.add(att_3);

   }

   // if ( mlog.verbosity_level() > 5 )  fcst_cluster_att.dump(cout);

   mlog << Debug(2)
        << "Calculating 3D obs cluster attributes\n";

   for (j=0; j<n_clusters; ++j)  {

      a = engine.obs_composite(j);   //  0-based

      a.increment(1);

      mask = obs_obj.select_cluster(a);   //  1-based

      att_3 = calc_3d_single_atts(mask, obs_raw, config.model.c_str(), config.inten_perc_value);

      // if ( att.Xvelocity > 20.0 )  mask.write("w.nc");

      att_3.set_object_number(j + 1);   //  1-based

      att_3.set_obs();

      att_3.set_cluster();

      obs_cluster_att.add(att_3);

   }

   // obs_cluster_att.dump(cout);

}   //  if have_pairs

   //
   //  get cluster pair attributes
   //

PairAtt3DArray pa_cluster;
IntArray b;

if ( have_pairs )  {

   mlog << Debug(2)
        << "Calculating 3D cluster pair attributes\n";

   for (j=0; j<n_clusters; ++j)  {

      a = engine.fcst_composite(j);   //  0-based

      a.increment(1);

      fo = fcst_obj.select_cluster(a);   //  1-based

      for (k=0; k<n_clusters; ++k)  {

         b = engine.obs_composite(k);   //  0-based

         b.increment(1);

         oo = obs_obj.select_cluster(b);   //  1-based

         p = calc_3d_pair_atts(fo, oo, fcst_cluster_att[j], obs_cluster_att[k]);

         p.set_cluster();

         // p.set_total_interest(e.calc(p));
         p.set_total_interest(-1.0);

         // mlog << Debug(5) << "   (F_" << j << ", O_" << k << ")   "
         //      << p.total_interest() << '\n';

         pa_cluster.add(p);

      }

   }

}   //  if have_pairs

   //
   //  calculate 2d cluster attributes
   //

SingleAtt2DArray fcst_cluster_att_2d, obs_cluster_att_2d;


if ( have_pairs )  {

   mlog << Debug(2) << "Calculating 2D cluster fcst attributes\n";

   for (j=0; j<n_clusters; ++j)  {

      att_3 = fcst_cluster_att[j];

      a = engine.fcst_composite(j);   //  0-based

      a.increment(1);

      mask = fcst_obj.select_cluster(a);   //  1-based

      for (t=(att_3.tmin()); t<=(att_3.tmax()); ++t)  {

         // mask_2d = mask.const_t_mask(t, j + 1);   //  1-based
         mask_2d = mask.const_t_mask(t, 1);   //  1-based

         // cout << "j = " << j << ",   vol = " << mask_2d.object_volume(0) << '\n';

         fcst_raw.get_data_plane(t, raw_2d);

         att_2 = calc_2d_single_atts(mask_2d, raw_2d, j + 1, config.inten_perc_value);

         att_2.set_fcst();

         att_2.set_valid_time(fcst_obj.start_valid_time() + t*(fcst_obj.delta_t()));

         att_2.set_lead_time(fcst_obj.lead_time(t));

         att_2.set_cluster_number (j + 1);   //  1-based

         att_2.set_time_index(t);

         att_2.set_is_cluster();

         fcst_cluster_att_2d.add(att_2);

      }   //  for t

   }   //  for j

       ///////////////////////////////////

   mlog << Debug(2) << "Calculating 2D cluster obs attributes\n";

   for (j=0; j<n_clusters; ++j)  {

      att_3 = obs_cluster_att[j];

      a = engine.obs_composite(j);   //  0-based

      a.increment(1);

      mask = obs_obj.select_cluster(a);   //  1-based

      for (t=(att_3.tmin()); t<=(att_3.tmax()); ++t)  {

         // mask_2d = mask.const_t_mask(t, j + 1);   //  1-based
         mask_2d = mask.const_t_mask(t, 1);   //  1-based

         obs_raw.get_data_plane(t, raw_2d);

         att_2 = calc_2d_single_atts(mask_2d, raw_2d, j + 1, config.inten_perc_value);

         att_2.set_obs();

         att_2.set_valid_time(obs_obj.start_valid_time() + t*(obs_obj.delta_t()));

         att_2.set_lead_time(obs_obj.lead_time(t));

         att_2.set_cluster_number (j + 1);   //  1-based

         att_2.set_time_index(t);

         att_2.set_is_cluster();

         obs_cluster_att_2d.add(att_2);

      }   //  for t

   }   //  for j




}   //  if have pairs

   //
   //  patch the cluster ids
   //

if ( have_pairs )  {

   fcst_single_att.patch_cluster_numbers(engine);
    obs_single_att.patch_cluster_numbers(engine);

   fcst_cluster_att.patch_cluster_numbers(engine);
    obs_cluster_att.patch_cluster_numbers(engine);

   fcst_simple_att_2d.patch_cluster_numbers(engine);
    obs_simple_att_2d.patch_cluster_numbers(engine);

   pa_simple.patch_cluster_numbers(engine);
   pa_cluster.patch_cluster_numbers(engine);

}

   //
   //  write 2d attributes for each simple & cluster object for each time slice
   //

path << cs_erase
     << output_directory << '/'
     << prefix << '_' << txt_2d_suffix;

mlog << Debug(2)
     << "Creating 2D constant-time slice attributes file: \""
     << path << "\"\n";

do_2d_txt_output(fcst_raw, obs_raw, 
                 fcst_simple_att_2d,  obs_simple_att_2d,
                 fcst_cluster_att_2d, obs_cluster_att_2d, config, path.c_str());

   //
   //  write simple single attributes
   //

path << cs_erase
     << output_directory << '/'
     << prefix << '_' << txt_3d_single_simple_suffix;

mlog << Debug(2)
     << "Creating 3D single simple attributes file: \""
     << path << "\"\n";

do_3d_single_txt_output(fcst_single_att, obs_single_att, config, path.c_str());

   //
   //  write simple pair attributes
   //

if ( have_pairs )  {

   path << cs_erase
        << output_directory << '/'
        << prefix << '_' << txt_3d_pair_simple_suffix;

   mlog << Debug(2)
        << "Creating 3D pair simple attributes file: \""
        << path << "\"\n";

   do_3d_pair_txt_output(pa_simple, config, false, path.c_str());

}

   //
   //  write cluster single attributes
   //

if ( have_pairs )  {

   path << cs_erase
        << output_directory << '/'
        << prefix << '_' << txt_3d_single_cluster_suffix;

   mlog << Debug(2)
        << "Creating 3D cluster single attributes file: \""
        << path << "\"\n";

   do_3d_single_txt_output(fcst_cluster_att, obs_cluster_att, config, path.c_str());

}

   //
   //  write cluster pair attributes
   //

if ( have_pairs )  {

   path << cs_erase
        << output_directory << '/'
        << prefix << '_' << txt_3d_pair_cluster_suffix;

   mlog << Debug(2)
        << "Creating 3D cluster pair attributes file: \""
        << path << "\"\n";

   do_3d_pair_txt_output(pa_cluster, config, true, path.c_str());

}

   //
   //  netcdf output
   //

path << cs_erase
     << output_directory << '/'
     << prefix << '_' << nc_suffix;

mlog << Debug(2)
     << "Creating NetCDF file: \""
     << path << "\"\n";


do_mtd_nc_output(config.nc_info, engine, fcst_raw, obs_raw, fcst_obj, obs_obj, config, path.c_str());


   //
   //  done
   //

return ( 0 );

}


////////////////////////////////////////////////////////////////////////


void usage()

{

ConcatString tab;

tab.set_repeat(' ', 10);

cout << "\n*** Model Evaluation Tools (MET" << met_version
     << ") ***\n\n"

     << "Usage: " << program_name << "\n"

     << tab << "-fcst    file_1 ... file_n | file_list\n"
     << tab << "-obs     file_1 ... file_n | file_list\n"
     << tab << "-single  file_1 ... file_n | file_list\n"
     << tab << "-config  config_file\n"
     << tab << "[-outdir path]\n"
     << tab << "[-log    file]\n"
     << tab << "[-v      level]\n\n"

     << "\twhere\t\"-fcst file_1 ... file_n\" are the gridded "
     << "forecast files to be used (required).\n"

     << "\t\t\"-fcst fcst_file_list\" is an ASCII file containing "
     << "a list of gridded forecast files to be used (required).\n"

     << "\t\t\"-obs  file_1 ... file_n\" are the gridded "
     << "observation files to be used (required).\n"

     << "\t\t\"-obs  obs_file_list\" is an ASCII file containing "
     << "a list of gridded observation files to be used (required).\n"

     << "\t\t\"-single\" instead of \"-fcst\" and \"-obs\" to run on "
     << "a single field (optional).\n"

     << "\t\t\"-config file\" is an MTDConfig file containing the "
     << "desired configuration settings (required).\n"

     << "\t\t\"-outdir path\" overrides the default output directory ("
     << output_directory << ") (optional).\n"

     << "\t\t\"-log file\" outputs log messages to the specified "
     << "file (optional).\n"

     << "\t\t\"-v level\" overrides the default level of logging ("
     << mlog.verbosity_level() << ") (optional).\n\n" << flush;

exit ( 1 );

}


////////////////////////////////////////////////////////////////////////


void set_fcst (const StringArray & a)

{

fcst_filenames = a;

return;

}


////////////////////////////////////////////////////////////////////////


void set_obs  (const StringArray & a)

{

obs_filenames = a;

return;

}


////////////////////////////////////////////////////////////////////////


void set_single  (const StringArray & a)

{

single_filenames = a;

return;

}


////////////////////////////////////////////////////////////////////////


void set_config  (const StringArray & a)

{

local_config_filename = a[0];

return;

}


////////////////////////////////////////////////////////////////////////


void set_verbosity  (const StringArray & a)

{

int k = atoi(a[0].c_str());

mlog.set_verbosity_level(k);

return;

}


////////////////////////////////////////////////////////////////////////


void set_logfile  (const StringArray & a)

{

ConcatString filename;

filename = a[0];

mlog.open_log_file(filename);

return;

}


////////////////////////////////////////////////////////////////////////


void set_outdir  (const StringArray & a)

{

output_directory = a[0];

return;

}


////////////////////////////////////////////////////////////////////////


ConcatString make_output_prefix(const MtdConfigInfo & config, unixtime start_time)

{

ConcatString prefix;
int year, month, day, hour, minute, second;
char junk[256];


prefix << cs_erase << default_prefix << '_';

if ( config.output_prefix.nonempty() )  prefix << config.output_prefix << '_';

unix_to_mdyhms(start_time, month, day, year, hour, minute, second);

snprintf(junk, sizeof(junk), "%04d%02d%02d_%02d%02d%02dV", year, month, day, hour, minute, second);

prefix << junk;



return ( prefix );

}


////////////////////////////////////////////////////////////////////////


void do_single_field(MtdConfigInfo & config)

{

MtdFloatFile raw, conv;
MtdIntFile mask, obj;
ConcatString prefix;
ConcatString path;


   //
   //  read the data files
   //

mtd_read_data(config, *(config.fcst_info), single_filenames, raw);

   //
   //  copy forecast name/units/level to observation
   //

config.obs_info->set_name(config.fcst_info->name_attr().text());

config.obs_info->set_units(config.fcst_info->units_attr().text());

config.obs_info->set_level_name(config.fcst_info->level_attr().text());

config.delta_t_seconds = raw.delta_t();

   //
   //  regrid, if necessary
   //

mlog << Debug(2) << "regridding, if needed ...\n";

const Grid to_grid = parse_vx_grid(config.fcst_info->regrid(),
                                   raw.grid_p(), raw.grid_p());

raw.regrid(to_grid, config.fcst_info->regrid());

   //
   //  make the output file prefix
   //

prefix = make_output_prefix(config, raw.start_valid_time());

   //
   //  convolve
   //

conv =  raw.convolve(config.fcst_conv_radius, config.fcst_conv_time_beg, config.fcst_conv_time_end);

   //
   //  threshold
   //

mask = conv.threshold(config.fcst_conv_thresh);

   //
   //  number the objects
   //

obj = mask;

mlog << Debug(2) << "Splitting object field\n";
   obj.split();
mlog << Debug(2) << "Done splitting\n";

   //
   //  toss small objects
   //

obj.toss_small_objects(config.min_volume);

int j;
SingleAtt3D att_3;
SingleAtt3DArray single_att;
Object select_mask;

mlog << Debug(2)
     << "Calculating 3D fcst single attributes\n";

for (j=0; j<(obj.n_objects()); ++j)  {

   select_mask = obj.select(j + 1);   //  1-based

   att_3 = calc_3d_single_atts(select_mask, raw, config.model.c_str(), config.inten_perc_value);

   att_3.set_object_number(j + 1);   //  1-based

   att_3.set_fcst();

   att_3.set_simple();

   single_att.add(att_3);

}

   //
   //  calculate 2d attributes
   //

int t;
SingleAtt2DArray att_2d;
SingleAtt2D att_2;
MtdIntFile mask_2d;
DataPlane raw_2d;


mlog << Debug(2) << "Calculating 2D fcst attributes\n";

for (j=0; j<(obj.n_objects()); ++j)  {

   att_3 = single_att[j];

   for (t=(att_3.tmin()); t<=(att_3.tmax()); ++t)  {

      mask_2d = obj.const_t_mask(t, j + 1);   //  1-based

      raw.get_data_plane(t, raw_2d);

      att_2 = calc_2d_single_atts(mask_2d, raw_2d, j + 1, config.inten_perc_value);

      att_2.set_fcst();

      att_2.set_valid_time(obj.start_valid_time() + t*(obj.delta_t()));

      att_2.set_lead_time(obj.lead_time(t));

      att_2.set_object_number(j + 1);   //  1-based

      att_2.set_time_index(t);

      att_2d.add(att_2);

   }   //  for k

}   //  for j


   //
   //  write 2d attributes for each simple object for each time slice
   //

path << cs_erase
     << output_directory << '/'
     << prefix << '_' << txt_2d_suffix;

mlog << Debug(2)
     << "Creating 2D constant-time slice attributes file: \""
     << path << "\"\n";

do_2d_txt_output(raw, raw, 
                 att_2d, config, path.c_str());

   //
   //  write simple single attributes
   //

path << cs_erase
     << output_directory << '/'
     << prefix << '_' << txt_3d_single_simple_suffix;

mlog << Debug(2)
     << "Creating 3D single simple attributes file: \""
     << path << "\"\n";

do_3d_single_txt_output(single_att, config, path.c_str());

   //
   //  netcdf output
   //

path << cs_erase
     << output_directory << '/'
     << prefix << '_' << nc_suffix;

mlog << Debug(2)
     << "Creating NetCDF file: \""
     << path << "\"\n";


do_mtd_nc_output(config.nc_info, raw, obj, config, path.c_str());



   //
   //  done
   //

return;

}


////////////////////////////////////////////////////////////////////////

