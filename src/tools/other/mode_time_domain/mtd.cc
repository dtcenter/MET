

////////////////////////////////////////////////////////////////////////


static const char default_config_path [] = "MET_BASE/config/MTDConfig_default";


////////////////////////////////////////////////////////////////////////


using namespace std;

#include <iostream>
#include <unistd.h>
#include <stdlib.h>
#include <cmath>

#include "mtd_config_info.h"
#include "mtd_file.h"
#include "interest_calc.h"
#include "3d_att_single_array.h"
#include "mtd_txt_output.h"
#include "mtd_read_data.h"


////////////////////////////////////////////////////////////////////////


static ConcatString program_name;

static CommandLine cline;

static StringArray fcst_filenames;
static StringArray  obs_filenames;

static ConcatString local_config_filename;


////////////////////////////////////////////////////////////////////////


static void usage();

static void set_fcst   (const StringArray &);
static void set_obs    (const StringArray &);
static void set_config (const StringArray &);

static void set_verbosity (const StringArray &);
static void set_logfile   (const StringArray &);


////////////////////////////////////////////////////////////////////////


int main(int argc, char * argv [])

{

program_name = get_short_name(argv[0]);

cline.set(argc, argv);

cline.set_usage(usage);

cline.add(set_fcst,      "-fcst",   -1);
cline.add(set_obs,       "-obs",    -1);
cline.add(set_config,    "-config",  1);
cline.add(set_verbosity, "-v",       1);
cline.add(set_logfile,   "-log",     1);

cline.parse();

cout << "\n\n  Verbosity = " << mlog.verbosity_level() << "\n\n";

if ( cline.n() != 0 )  usage();   //  should only be config file left on command line

// fcst_filenames.dump(cout);

   //
   //  read the config file
   //

MtdConfigInfo config;
ConcatString default_config_filename;
ConcatString path;


default_config_filename = replace_path(default_config_path);

cout << "\n\n  Default config file = \"" << default_config_filename << "\"\n\n" << flush;


config.read_config(default_config_filename, local_config_filename);

config.process_config(FileType_NcMet, FileType_NcMet);

// exit ( 1 );

int j, k;
MtdFloatFile fcst_raw, obs_raw;
MtdFloatFile fcst_conv, obs_conv;
MtdIntFile fcst_mask, obs_mask;
MtdIntFile fcst_obj, obs_obj;
InterestCalculator calc;

   //
   //  read the data files
   //

// config.fcst_info->dump(cout);

mtd_read_data(config, *(config.fcst_info), fcst_filenames, fcst_raw);
mtd_read_data(config, *(config.obs_info),   obs_filenames,  obs_raw);

// exit ( 1 );

/*
if ( ! fcst_raw.read(fcst_filename) )  {

   cerr << "\n\n  " << program_name << ": unable to read fcst file \"" << fcst_filename << "\"\n\n";

   exit ( 1 );

}

if ( ! obs_raw.read(obs_filename) )  {

   cerr << "\n\n  " << program_name << ": unable to read obs file \"" << obs_filename << "\"\n\n";

   exit ( 1 );

}
*/

calc.add(config.space_centroid_dist_wt, config.space_centroid_dist_if, &PairAtt3D::SpaceCentroidDist);
calc.add(config.time_centroid_delta_wt, config.time_centroid_delta_if, &PairAtt3D::TimeCentroidDelta);
calc.add(config.speed_delta_wt,         config.speed_delta_if,         &PairAtt3D::SpeedDelta);
calc.add(config.direction_diff_wt,      config.direction_diff_if,      &PairAtt3D::DirectionDifference);
calc.add(config.volume_ratio_wt,        config.volume_ratio_if,        &PairAtt3D::VolumeRatio);
calc.add(config.axis_angle_diff_wt,     config.axis_angle_diff_if,     &PairAtt3D::AxisDiff);
calc.add(config.start_time_delta_wt,    config.start_time_delta_if,    &PairAtt3D::StartTimeDelta);
calc.add(config.end_time_delta_wt,      config.end_time_delta_if,      &PairAtt3D::EndTimeDelta);

calc.check();



cout << "\n  fcst conv radius = " << (config.fcst_conv_radius) << "\n";
cout << "\n   obs conv radius = " << (config.obs_conv_radius) << "\n";


fcst_conv = fcst_raw.convolve(config.fcst_conv_radius);
 obs_conv =  obs_raw.convolve(config.obs_conv_radius);

fcst_mask = fcst_conv.threshold(config.fcst_conv_thresh);
 obs_mask =  obs_conv.threshold(config.obs_conv_thresh);

fcst_obj = fcst_mask;
 obs_obj =  obs_mask;

cout << "Start split\n" << flush;
fcst_obj.split();
cout << "mid split\n" << flush;
 obs_obj.split();
cout << "End split\n" << flush;


fcst_conv.write("fcst_conv.nc");
 obs_conv.write("obs_conv.nc");

fcst_mask.write("fcst_mask.nc");
 obs_mask.write("obs_mask.nc");

fcst_obj.write("fcst_obj_notoss.nc");
 obs_obj.write("obs_obj_notoss.nc");

fcst_obj.toss_small_objects(config.min_volume);
 obs_obj.toss_small_objects(config.min_volume);

fcst_obj.write("fcst_obj_toss.nc");
 obs_obj.write("obs_obj_toss.nc");

cout << "\n\n  fcst threshold:\n";
config.fcst_conv_thresh.dump(cout);

cout << "\n\n  obs threshold:\n";
config.obs_conv_thresh.dump(cout);


cout << "\n\n   n_header_3d_cols = " << n_header_3d_cols << '\n';
cout << "\n\n   n_3d_single_cols = " << n_3d_single_cols << '\n';

   //
   // get single attributes
   //

SingleAtt3D att;
SingleAtt3DArray fcst_att, obs_att;
// MtdIntFile s;

cout << "calculating fcst atts\n" << flush;

for (j=0; j<(fcst_obj.n_objects()); ++j)  {

   att = calc_3d_single_atts(fcst_obj, fcst_raw, config.model, j);

   att.set_fcst();

   att.set_simple();

   fcst_att.add(att);

}

cout << "calculating obs atts\n" << flush;

for (j=0; j<(obs_obj.n_objects()); ++j)  {

   att = calc_3d_single_atts(obs_obj, obs_raw, config.model, j);

   att.set_obs();

   att.set_simple();

   obs_att.add(att);

}

// s = obs_obj.select(1);
// 
// s.write("obs_1_select.nc");

// cout << "\n\n  Obs single atts:\n\n";

// obs_att.dump(cout);

do_3d_single_txt_output(fcst_att, obs_att, config, "a.txt");

PairAtt3DArray a;
PairAtt3D p;
MtdIntFile fo, oo;

cout << "\n\n  calculating pair atts\n\n";

for (j=0; j<(fcst_obj.n_objects()); ++j)  {

   fo = fcst_obj.select(j + 1);

   for (k=0; k<(obs_obj.n_objects()); ++k)  {

      oo = obs_obj.select(k + 1);

      cout << "   (" << j << ", " << k << ")\n" << flush;

      if ( (j == 4) && (k == 4) )  {

         cout << "stop\n";

      }

      p = calc_3d_pair_atts(fo, oo, fcst_att[j], obs_att[k]);

      p.set_total_interest(calc(p));

      a.add(p);

   }

}


a.dump(cout);





   //
   //  done
   //

return ( 0 );

}


////////////////////////////////////////////////////////////////////////


void usage()

{

ConcatString tab;

tab.set_repeat(' ', 10 + program_name.length());

cout << "Usage: " << program_name << '\n';

cout << tab << "-fcst   file_list\n";
cout << tab << "-obs    file_list\n";
cout << tab << "-config config_file\n";
cout << tab << "[ -log  file ]\n";
cout << tab << "[ -v    level ]\n";


   //
   //  done
   //

cout << "\n\n";

exit ( 1 );

return;

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


void set_config  (const StringArray & a)

{

local_config_filename = a[0];

return;

}


////////////////////////////////////////////////////////////////////////


void set_verbosity  (const StringArray & a)

{

int k = atoi(a[0]);

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





