// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2024
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*


////////////////////////////////////////////////////////////////////////


#include <iostream>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <cmath>
#include <time.h>

#include <netcdf>

#include "file_exists.h"

#include "string_fxns.h"
#include "vx_log.h"
#include "nc_utils.h"
#include "seeps.h"

using namespace std;
using namespace netCDF;


////////////////////////////////////////////////////////////////////////

bool standalone_debug_seeps = false;

static SeepsClimo *seeps_climo = nullptr;
static std::map<int,SeepsClimoGrid *> seeps_climo_grid_map_00;

static const char *var_name_sid       = "sid";
static const char *var_name_lat       = "lat";
static const char *var_name_lon       = "lon";
static const char *var_name_elv       = "elv";

static const char *dim_name_lat       = "latitude";
static const char *dim_name_lon       = "longitude";

double weighted_average(double, double, double, double);

////////////////////////////////////////////////////////////////////////

SeepsClimo *get_seeps_climo(ConcatString seeps_point_climo_name) {
   if (! seeps_climo) seeps_climo = new SeepsClimo(seeps_point_climo_name);
   return seeps_climo;
}

////////////////////////////////////////////////////////////////////////

void release_seeps_climo() {
   if (seeps_climo) { delete seeps_climo; seeps_climo = nullptr; }
}

////////////////////////////////////////////////////////////////////////

SeepsClimoGrid *get_seeps_climo_grid(int month, ConcatString seeps_grid_climo_name, int hour) {

   if (seeps_climo_grid_map_00.count(month) == 0) {
      seeps_climo_grid_map_00[month] = nullptr;
      seeps_climo_grid_map_00[month] = new SeepsClimoGrid(month, hour, seeps_grid_climo_name);
   }

   return seeps_climo_grid_map_00[month];
}

////////////////////////////////////////////////////////////////////////

void release_seeps_climo_grid(int month, int hour) {

   if (seeps_climo_grid_map_00.count(month) > 0) {
      delete seeps_climo_grid_map_00[month];
      seeps_climo_grid_map_00[month] = nullptr;
      seeps_climo_grid_map_00.erase(month);
   }
}

////////////////////////////////////////////////////////////////////////

double weighted_average(double v1, double w1, double v2, double w2) {
   return(is_bad_data(v1) || is_bad_data(v2) ?
          bad_data_double :
          v1 * w1 + v2 * w2);
}

////////////////////////////////////////////////////////////////////////

void SeepsAggScore::clear() {

   n_obs = 0;
   c12 = c13 = c21 = c23 = c31 = c32 = 0;
   s12 = s13 = s21 = s23 = s31 = s32 = 0.;
   pv1 = pv2 = pv3 = 0.;
   pf1 = pf2 = pf3 = 0.;
   mean_fcst = mean_obs = bad_data_double;
   weighted_score = score = bad_data_double;

}

////////////////////////////////////////////////////////////////////////

SeepsAggScore & SeepsAggScore::operator+=(const SeepsAggScore &c) {

   // Check for degenerate case
   if(n_obs == 0 && c.n_obs == 0) return *this;

   // Compute weights
   double w1 = (double)   n_obs / (n_obs + c.n_obs);
   double w2 = (double) c.n_obs / (n_obs + c.n_obs);

   // Increment number of obs
   n_obs += c.n_obs;

   // Increment counts
   c12 += c.c12;
   c13 += c.c13;
   c21 += c.c21;
   c23 += c.c23;
   c31 += c.c31;
   c32 += c.c32;

   // Compute weighted averages
   s12 = weighted_average(s12, w1, c.s12, w2);
   s13 = weighted_average(s13, w1, c.s13, w2);
   s21 = weighted_average(s21, w1, c.s21, w2);
   s23 = weighted_average(s23, w1, c.s23, w2);
   s31 = weighted_average(s31, w1, c.s31, w2);
   s32 = weighted_average(s32, w1, c.s32, w2);

   pv1 = weighted_average(pv1, w1, c.pv1, w2);
   pv2 = weighted_average(pv2, w1, c.pv2, w2);
   pv3 = weighted_average(pv3, w1, c.pv3, w2);

   pf1 = weighted_average(pf1, w1, c.pf1, w2);
   pf2 = weighted_average(pf2, w1, c.pf2, w2);
   pf3 = weighted_average(pf3, w1, c.pf3, w2);

   mean_fcst = weighted_average(mean_fcst, w1, c.mean_fcst, w2);
   mean_obs  = weighted_average(mean_obs,  w1, c.mean_obs,  w2);

   score          = weighted_average(score,          w1, c.score,          w2);
   weighted_score = weighted_average(weighted_score, w1, c.weighted_score, w2);

   return *this;
}

////////////////////////////////////////////////////////////////////////

SeepsClimoBase::SeepsClimoBase(ConcatString seeps_climo_name) : climo_file_name{seeps_climo_name} {

   clear();
   seeps_ready = false;

}

////////////////////////////////////////////////////////////////////////

SeepsClimoBase::~SeepsClimoBase() {
   clear();
}

////////////////////////////////////////////////////////////////////////

void SeepsClimoBase::clear() {
   filtered_count = 0;
   seeps_p1_thresh.clear();
}

////////////////////////////////////////////////////////////////////////

ConcatString SeepsClimoBase::get_climo_filename() {
   ConcatString log_seeps_filename;
   ConcatString seeps_filename;
   const char *method_name = "SeepsClimoBase::get_climo_filename() -> ";

   // Use the environment variable, if set.
   ConcatString env_climo_name = get_env_climo_name();
   bool use_env = get_env(env_climo_name.c_str(), seeps_filename);
   if(!use_env) {
      seeps_filename = climo_file_name.nonempty() ? climo_file_name : get_def_climo_name();
   }
   seeps_filename = replace_path(seeps_filename);

   seeps_ready = file_exists(seeps_filename.c_str());
   if (seeps_ready) {
      mlog << Debug(7) << method_name << "SEEPS climo name=\""
           << seeps_filename.c_str() << "\"\n";
   }
   else {
      ConcatString message = "";
      ConcatString message2 = "";
      if (use_env) {
         message.add("from the environment variable ");
         message.add(env_climo_name);
         message2.add("Correct the environment variable");
      }
      else {
         message.add(climo_file_name.nonempty()
                     ? "from the configuration" : "from the default");
         message2.add("Correct the configuration");
      }
      mlog << Warning << "\n" << method_name
           << "The SEEPS climo name \"" << seeps_filename.c_str()
           << "\" " << message << " does not exist!\n"
           << message2 << " to set its location "
           << "or disable output for SEEPS and SEEPS_MPR.\n\n";
      exit(1);
   }

   return seeps_filename;
}

////////////////////////////////////////////////////////////////////////

void SeepsClimoBase::set_p1_thresh(const SingleThresh &p1_thresh) {
   seeps_p1_thresh = p1_thresh;
}


////////////////////////////////////////////////////////////////////////


SeepsClimo::SeepsClimo(ConcatString seeps_climo_name) : SeepsClimoBase{seeps_climo_name} {

   clear();
   ConcatString seeps_name = get_climo_filename();
   if (file_exists(seeps_name.c_str())) read_seeps_scores(seeps_name);

}

////////////////////////////////////////////////////////////////////////

SeepsClimo::~SeepsClimo() {
   clear();
}

////////////////////////////////////////////////////////////////////////

void SeepsClimo::clear() {
   SeepsClimoBase::clear();
   for (map<int,SeepsClimoRecord *>::iterator it=seeps_score_00_map.begin();
        it!=seeps_score_00_map.end(); ++it) {
      delete it->second;
   }

   for (map<int,SeepsClimoRecord *>::iterator it=seeps_score_12_map.begin();
        it!=seeps_score_12_map.end(); ++it) {
      delete it->second;
   }

   seeps_score_00_map.clear();
   seeps_score_12_map.clear();
};

////////////////////////////////////////////////////////////////////////

SeepsClimoRecord *SeepsClimo::create_climo_record(
      int sid, double lat, double lon, double elv,
      double *p1, double *p2, double *t1, double *t2, double *scores) {
   int offset;
   SeepsClimoRecord *record = new SeepsClimoRecord();
   
   record->sid = sid;
   record->lat = lat;
   record->lon = lon;
   record->elv = elv;
   if (standalone_debug_seeps && SAMPLE_STATION_ID == sid) {
      cout << "   sid=" << sid << ", lat=" << lat << ", lon=" << lon << ", elv=" << elv << "\n";
   }
   for (int idx=0; idx<SEEPS_MONTH; idx++) {
      record->p1[idx] = p1[idx];
      record->p2[idx] = p2[idx];
      record->t1[idx] = t1[idx];
      record->t2[idx] = t2[idx];

      if (standalone_debug_seeps && SAMPLE_STATION_ID == sid) {
         cout << str_format("\t%2d: %6.3f %6.3f  %6.3f %6.3f   ",
                            (idx+1), record->p1[idx], record->p2[idx],
                            record->t1[idx], record->t2[idx]);
      }
      for (int idx_m=0; idx_m<SEEPS_MATRIX_SIZE; idx_m++) {
         offset = idx*SEEPS_MATRIX_SIZE + idx_m;
         record->scores[idx][idx_m] = scores[offset];
         if (standalone_debug_seeps && SAMPLE_STATION_ID == sid) {
            cout << str_format(" %.3f", record->scores[idx][idx_m]);
         }
      }
      if (standalone_debug_seeps && SAMPLE_STATION_ID == sid) cout << "\n";
   }

   return record;
}

////////////////////////////////////////////////////////////////////////

SeepsRecord *SeepsClimo::get_record(int sid, int month, int hour) {
   SeepsRecord *record = nullptr;
   const char *method_name = "SeepsClimo::get_record() -> ";

   if (is_seeps_ready()) {
      SeepsClimoRecord *climo_record = nullptr;
      map<int,SeepsClimoRecord *>::iterator it;
      if (hour < 6 || hour >= 18) {
         it = seeps_score_00_map.find(sid);
         if (it != seeps_score_00_map.end()) climo_record = it->second;
      }
      else {
         it = seeps_score_12_map.find(sid);
         if (it != seeps_score_12_map.end()) climo_record = it->second;
      }
      if (nullptr != climo_record) {
         double p1 = climo_record->p1[month-1];
         if (check_seeps_p1_thresh(p1)) {
            record = new SeepsRecord;
            record->sid = climo_record->sid;
            record->lat = climo_record->lat;
            record->lon = climo_record->lon;
            record->elv = climo_record->elv;
            record->month = month;
            record->p1 = climo_record->p1[month-1];
            record->p2 = climo_record->p2[month-1];
            record->t1 = climo_record->t1[month-1];
            record->t2 = climo_record->t2[month-1];
            for (int idx=0; idx<SEEPS_MATRIX_SIZE; idx++) {
               record->scores[idx] = climo_record->scores[month-1][idx];
            }
         }
         else if (!is_eq(p1, bad_data_double)) {
            increase_filtered_count();
            mlog << Debug(7) << method_name << " filtered by threshold p1="
                 << climo_record->p1[month-1] <<"\n";
         }
      }
   }
   else {
      mlog << Error << "\n" << method_name
           << "The SEEPS point climo data is missing!\n"
           << "Set the " << MET_ENV_SEEPS_POINT_CLIMO_NAME
           << " environment variable to define its location "
           << "or disable output for SEEPS and SEEPS_MPR.\n\n";
      exit(1);
   }

   return record;
}

////////////////////////////////////////////////////////////////////////

double SeepsClimo::get_score(int sid, double p_fcst, double p_obs,
                             int month, int hour) {
   double score = bad_data_double;
   SeepsRecord *record = get_record(sid, month, hour);

   if (nullptr != record) {
      // Determine location in contingency table
      int ic = (p_obs>record->t1)+(p_obs>record->t2);
      int jc = (p_fcst>record->t1)+(p_fcst>record->t2);

      score = record->scores[(jc*3)+ic];
      delete record;
   }

   return score;
}

////////////////////////////////////////////////////////////////////////

SeepsScore *SeepsClimo::get_seeps_score(int sid, double p_fcst, double p_obs,
                                        int month, int hour) {
   SeepsScore *score = nullptr;
   SeepsRecord *record = get_record(sid, month, hour);

   if (nullptr != record) {
      score = new SeepsScore();
      score->p1 = record->p1;
      score->p2 = record->p2;
      score->t1 = record->t1;
      score->t2 = record->t2;

      score->obs_cat = (p_obs>record->t1)+(p_obs>record->t2);
      score->fcst_cat = (p_fcst>record->t1)+(p_fcst>record->t2);
      score->s_idx = (score->fcst_cat*3)+score->obs_cat;
      score->score = record->scores[score->s_idx];
      delete record;
   }

   return score;
}


////////////////////////////////////////////////////////////////////////

void SeepsClimo::print_all() {
   const char *method_name = "SeepsClimo::print_all() -> ";

   cout << "\n";
   cout << "===============  00Z  ===============\n";
   cout << "  sid\tlat\tlon\telv\n";
   cout << "\tmonth\tp1\tp2\tt1\tt2\tscores (3 by 3 matrix)\n";
   for (map<int,SeepsClimoRecord *>::iterator it=seeps_score_00_map.begin();
        it!=seeps_score_00_map.end(); ++it) {
      print_record(it->second);
   }

   cout << "\n";
   cout << "===============  12Z  ===============\n";
   cout << "  sid\tlat\tlon\telv\n";
   cout << "\tmonth\tp1\tp2\tt1\tt2\tscores (3 by 3 matrix)\n";
   for (map<int,SeepsClimoRecord *>::iterator it=seeps_score_12_map.begin();
        it!=seeps_score_12_map.end(); ++it) {
      print_record(it->second);
   }

}

////////////////////////////////////////////////////////////////////////

void SeepsClimo::print_record(SeepsClimoRecord *record, bool with_header) {
   if (with_header) {
      cout << "  sid\tlat\tlon\telv\n";
      cout << "\tmonth\tp1\tp2\tt1\tt2\tscores (3 by 3 matrix)\n";
   }
   cout << "  " << record->sid << "\t" << record->lat << "\t" << record->lon
        << "\t" << record->elv << "\n";
   for (int idx=0; idx<SEEPS_MONTH; idx++) {
      cout << "\t" << (idx+1) << "\t" << record->p1[idx] << "\t" << record->p2[idx]
           << "\t" << record->t1[idx] << "\t" << record->t2[idx];
      for (int idx2=0; idx2<SEEPS_MATRIX_SIZE; idx2++) {
         cout << "\t" << record->scores[idx][idx2];
      }
      cout << "\n";
   }
}

////////////////////////////////////////////////////////////////////////

void SeepsClimo::print_record(SeepsRecord *record, bool with_header) {
   if (with_header) cout << "  sid\tlat\tlon\telv\tp1\tp2\tt1\tt2\tscores (3 by 3 matrix)\n";

   cout << "  " << record->sid << "\t" << record->lat << "\t" << record->lon
        << "\t" << record->elv << "\t" << record->p1 << "\t" << record->p2
        << "\t" << record->t1 << "\t" << record->t2;
   for (int idx=0; idx<SEEPS_MATRIX_SIZE; idx++) {
      cout << "\t" << record->scores[idx];
   }
   cout << " for month " << record->month << "\n";
}

////////////////////////////////////////////////////////////////////////

void SeepsClimo::read_seeps_scores(ConcatString filename) {
   clock_t clock_time = clock();
   const char *method_name = "SeepsClimo::read_records() -> ";

   try {
      double p1_00_buf[SEEPS_MONTH];
      double p2_00_buf[SEEPS_MONTH];
      double t1_00_buf[SEEPS_MONTH];
      double t2_00_buf[SEEPS_MONTH];
      double p1_12_buf[SEEPS_MONTH];
      double p2_12_buf[SEEPS_MONTH];
      double t1_12_buf[SEEPS_MONTH];
      double t2_12_buf[SEEPS_MONTH];
      double matrix_00_buf[SEEPS_MONTH*SEEPS_MATRIX_SIZE];
      double matrix_12_buf[SEEPS_MONTH*SEEPS_MATRIX_SIZE];
      NcFile *nc_file = open_ncfile(filename.c_str());

      clear();

      // dimensions: month = 12 ; nstn = 5293 ; nmatrix = 9 ;
      get_dim(nc_file, dim_name_nstn, nstn, true);
      mlog << Debug(6) << method_name << "dimensions nstn = " << nstn << "\n";
      if (standalone_debug_seeps) cout << "dimensions nstn = " << nstn << "\n";

      vector<int   > sid_array(nstn);
      vector<double> lat_array(nstn);
      vector<double> lon_array(nstn);
      vector<double> elv_array(nstn);
      vector<double> p1_00_array(nstn*SEEPS_MONTH);
      vector<double> p2_00_array(nstn*SEEPS_MONTH);
      vector<double> t1_00_array(nstn*SEEPS_MONTH);
      vector<double> t2_00_array(nstn*SEEPS_MONTH);
      vector<double> p1_12_array(nstn*SEEPS_MONTH);
      vector<double> p2_12_array(nstn*SEEPS_MONTH);
      vector<double> t1_12_array(nstn*SEEPS_MONTH);
      vector<double> t2_12_array(nstn*SEEPS_MONTH);
      vector<double> matrix_00_array(nstn*SEEPS_MONTH*SEEPS_MATRIX_SIZE);
      vector<double> matrix_12_array(nstn*SEEPS_MONTH*SEEPS_MATRIX_SIZE);

      NcVar var_sid       = get_nc_var(nc_file, var_name_sid);
      NcVar var_lat       = get_nc_var(nc_file, var_name_lat);
      NcVar var_lon       = get_nc_var(nc_file, var_name_lon);
      NcVar var_elv       = get_nc_var(nc_file, var_name_elv);
      NcVar var_p1_00     = get_nc_var(nc_file, var_name_p1_00);
      NcVar var_p2_00     = get_nc_var(nc_file, var_name_p2_00);
      NcVar var_t1_00     = get_nc_var(nc_file, var_name_t1_00);
      NcVar var_t2_00     = get_nc_var(nc_file, var_name_t2_00);
      NcVar var_p1_12     = get_nc_var(nc_file, var_name_p1_12);
      NcVar var_p2_12     = get_nc_var(nc_file, var_name_p2_12);
      NcVar var_t1_12     = get_nc_var(nc_file, var_name_t1_12);
      NcVar var_t2_12     = get_nc_var(nc_file, var_name_t2_12);
      NcVar var_matrix_00 = get_nc_var(nc_file, var_name_matrix_00);
      NcVar var_matrix_12 = get_nc_var(nc_file, var_name_matrix_12);

      if (IS_INVALID_NC(var_sid) || !get_nc_data(&var_sid, sid_array)) {
         mlog << Error << "\n" << method_name
              << "Did not get sid\n\n";
         exit(1);
      }
      if (IS_INVALID_NC(var_lat) || !get_nc_data(&var_lat, lat_array)) {
         mlog << Error << "\n" << method_name
              << "Did not get lat\n\n";
         exit(1);
      }
      if (IS_INVALID_NC(var_lon) || !get_nc_data(&var_lon, lon_array)) {
         mlog << Error << "\n" << method_name
              << "Did not get lon\n\n";
         exit(1);
      }
      if (IS_INVALID_NC(var_elv) || !get_nc_data(&var_elv, elv_array)) {
         mlog << Error << "\n" << method_name
              << "Did not get elv\n\n";
         exit(1);
      }
      if (IS_INVALID_NC(var_p1_00) || !get_nc_data(&var_p1_00, p1_00_array)) {
         mlog << Error << "\n" << method_name
              << "Did not get p1_00\n\n";
         exit(1);
      }
      if (IS_INVALID_NC(var_p2_00) || !get_nc_data(&var_p2_00, p2_00_array)) {
         mlog << Error << "\n" << method_name
              << "Did not get p2_00\n\n";
         exit(1);
      }
      if (IS_INVALID_NC(var_t1_00) || !get_nc_data(&var_t1_00, t1_00_array)) {
         mlog << Error << "\n" << method_name
              << "Did not get t1_00\n\n";
         exit(1);
      }
      if (IS_INVALID_NC(var_t2_00) || !get_nc_data(&var_t2_00, t2_00_array)) {
         mlog << Error << "\n" << method_name
              << "Did not get t2_00\n\n";
         exit(1);
      }
      if (IS_INVALID_NC(var_p1_12) || !get_nc_data(&var_p1_12, p1_12_array)) {
         mlog << Error << "\n" << method_name
              << "Did not get p1_12\n\n";
         exit(1);
      }
      if (IS_INVALID_NC(var_p2_12) || !get_nc_data(&var_p2_12, p2_12_array)) {
         mlog << Error << "\n" << method_name
              << "Did not get p2_12\n\n";
         exit(1);
      }
      if (IS_INVALID_NC(var_t1_12) || !get_nc_data(&var_t1_12, t1_12_array)) {
         mlog << Error << "\n" << method_name
              << "Did not get t1_12\n\n";
         exit(1);
      }
      if (IS_INVALID_NC(var_t2_12) || !get_nc_data(&var_t2_12, t2_12_array)) {
         mlog << Error << "\n" << method_name
              << "Did not get t2_12\n\n";
         exit(1);
      }
      if (IS_INVALID_NC(var_matrix_00) || !get_nc_data(&var_matrix_00, matrix_00_array)) {
         mlog << Error << "\n" << method_name
              << "Did not get matrix_00\n\n";
         exit(1);
      }
      if (IS_INVALID_NC(var_matrix_12) || !get_nc_data(&var_matrix_12, matrix_12_array)) {
         mlog << Error << "\n" << method_name
              << "Did not get matrix_12\n\n";
         exit(1);
      }

      SeepsClimoRecord *rec_00;
      SeepsClimoRecord *rec_12;
      for (int idx=0; idx<nstn; idx++) {
         int sid = sid_array[idx];
         int start_offset = idx * SEEPS_MONTH;
         for (int idx2=0; idx2<SEEPS_MONTH; idx2++) {
            p1_00_buf[idx2] = p1_00_array[start_offset + idx2];
            p2_00_buf[idx2] = p2_00_array[start_offset + idx2];
            t1_00_buf[idx2] = t1_00_array[start_offset + idx2];
            t2_00_buf[idx2] = t2_00_array[start_offset + idx2];
            p1_12_buf[idx2] = p1_12_array[start_offset + idx2];
            p2_12_buf[idx2] = p2_12_array[start_offset + idx2];
            t1_12_buf[idx2] = t1_12_array[start_offset + idx2];
            t2_12_buf[idx2] = t2_12_array[start_offset + idx2];
            int offset_to = idx2 * SEEPS_MATRIX_SIZE;
            int offset_from = (start_offset + idx2) * SEEPS_MATRIX_SIZE;
            for (int idx3=0; idx3<SEEPS_MATRIX_SIZE; idx3++) {
               matrix_00_buf[offset_to+idx3] = matrix_00_array[offset_from+idx3];
               matrix_12_buf[offset_to+idx3] = matrix_12_array[offset_from+idx3];
            }
         }
         rec_00 = create_climo_record(sid, lat_array[idx], lon_array[idx], elv_array[idx],
                                      p1_00_buf, p2_00_buf, t1_00_buf, t2_00_buf, matrix_00_buf);
         rec_12 = create_climo_record(sid, lat_array[idx], lon_array[idx], elv_array[idx],
                                      p1_12_buf, p2_12_buf, t1_12_buf, t2_12_buf, matrix_12_buf);
         seeps_score_00_map[sid] = rec_00;
         seeps_score_12_map[sid] = rec_12;
      }

      nc_file->close();

      float duration = (float)(clock() - clock_time)/CLOCKS_PER_SEC;
      mlog << Debug(6) << method_name << "took " << duration << " seconds\n";
      if (standalone_debug_seeps) cout << method_name << "took " << duration  << " seconds\n";
   }
   catch(int i_err) {

      set_seeps_ready(false);
      mlog << Error << "\n" << method_name
           << "encountered an error on reading " << filename << ". ecception_code="  << i_err << "\n\n";

      exit(i_err);
   } // end catch block

}

////////////////////////////////////////////////////////////////////////

SeepsClimoGrid::SeepsClimoGrid(int month, int hour, ConcatString seeps_climo_name)
   : month{month}, hour{hour}, SeepsClimoBase{seeps_climo_name}
{
   init_from_scratch();
   ConcatString seeps_name = get_climo_filename();
   if (file_exists(seeps_name.c_str())) read_seeps_scores(seeps_name);
}

////////////////////////////////////////////////////////////////////////

SeepsClimoGrid::~SeepsClimoGrid() {
   clear();
}

////////////////////////////////////////////////////////////////////////

void SeepsClimoGrid::init_from_scratch() {
   clear();
}

////////////////////////////////////////////////////////////////////////

void SeepsClimoGrid::clear() {
   SeepsClimoBase::clear();
   p1_buf.clear();
   p2_buf.clear();
   t1_buf.clear();
   t2_buf.clear();
   s12_buf.clear();
   s13_buf.clear();
   s21_buf.clear();
   s23_buf.clear();
   s31_buf.clear();
   s32_buf.clear();
};

////////////////////////////////////////////////////////////////////////

SeepsScore *SeepsClimoGrid::get_record(int ix, int iy,
                                       double p_fcst, double p_obs) {
   SeepsScore *seeps_record = nullptr;
   const char *method_name = "SeepsClimoGrid::get_record() -> ";
   if (!is_eq(p_fcst, -9999.0) && !is_eq(p_obs, -9999.0)) {
      int offset = iy * nx + ix;
      double p1 = p1_buf[offset];

      if (check_seeps_p1_thresh(p1)) {
         // Determine location in contingency table
         int ic = (p_obs>t1_buf[offset])+(p_obs>t2_buf[offset]);
         int jc = (p_fcst>t1_buf[offset])+(p_fcst>t2_buf[offset]);
         double score = get_score(offset, ic, jc);

         seeps_record = new SeepsScore();
         seeps_record->obs_cat = ic;
         seeps_record->fcst_cat = jc;
         seeps_record->s_idx = (jc*3)+ic;
         seeps_record->p1 = p1_buf[offset];
         seeps_record->p2 = p2_buf[offset];
         seeps_record->t1 = t1_buf[offset];
         seeps_record->t2 = t2_buf[offset];
         seeps_record->score = score;
      }
      else if (~is_eq(p1, bad_data_double)) {
         increase_filtered_count();
         mlog << Debug(7) << method_name << " filtered by threshold p1=" << p1_buf[offset] <<"\n";
      }
   }

   return seeps_record;
}

////////////////////////////////////////////////////////////////////////

double SeepsClimoGrid::get_score(int offset, int obs_cat, int fcst_cat) {
   double score = bad_data_double;

   if (offset >= (nx * ny)) {
      mlog << Error << "\nSeepsClimoGrid::get_score() --> offset (" << offset
           << " is too big (" << (nx*ny) << ")\n";
      return score;
   }

   if (obs_cat == 0) {
      if (fcst_cat == 1) score = s12_buf[offset];
      else if (fcst_cat == 2) score = s13_buf[offset];
      else score = 0.;
   }
   else if (obs_cat == 1) {
      if (fcst_cat == 0) score = s21_buf[offset];
      else if (fcst_cat == 2) score = s23_buf[offset];
      else score = 0.;
   }
   else {
      if (fcst_cat == 0) score = s31_buf[offset];
      else if (fcst_cat == 1) score = s32_buf[offset];
      else score = 0.;
   }

   return score;
}

////////////////////////////////////////////////////////////////////////

double SeepsClimoGrid::get_score(int ix, int iy, double p_fcst, double p_obs) {
   double score = bad_data_double;

   if (!is_eq(p_fcst, -9999.0) && !is_eq(p_obs, -9999.0)) {
      int offset = iy * nx + ix;
      // Determine location in contingency table
      int ic = (p_obs>t1_buf[offset])+(p_obs>t2_buf[offset]);
      int jc = (p_fcst>t1_buf[offset])+(p_fcst>t2_buf[offset]);
      score = get_score(offset, ic, jc);
   }

   return score;
}

////////////////////////////////////////////////////////////////////////

void SeepsClimoGrid::read_seeps_scores(ConcatString filename) {
   clock_t clock_time = clock();
   const char *method_name = "SeepsClimoGrid::read_seeps_scores() -> ";

   try {
      NcFile *nc_file = open_ncfile(filename.c_str());

      // dimensions: month = 12;
      if (!has_dim(nc_file, dim_name_lat) || !has_dim(nc_file, dim_name_lon)) {
         mlog << Error << "\n" << method_name
              << "\"" << filename << "\" is not valid SEEPS climo file\n\n";
         //exit(1);
      }

      get_dim(nc_file, dim_name_lat, ny, true);
      get_dim(nc_file, dim_name_lon, nx, true);
      mlog << Debug(6) << method_name << "dimensions lon = " << nx << " lat = " << ny
           << " month=" << month << "\n";
      if (standalone_debug_seeps) cout << "dimensions lon = " << nx << " lat = " << ny
                                       << " month=" << month << "\n";;

      p1_buf.resize(nx*ny, bad_data_double);
      p2_buf.resize(nx*ny, bad_data_double);
      t1_buf.resize(nx*ny, bad_data_double);
      t2_buf.resize(nx*ny, bad_data_double);
      s12_buf.resize(nx*ny, bad_data_double);
      s13_buf.resize(nx*ny, bad_data_double);
      s21_buf.resize(nx*ny, bad_data_double);
      s23_buf.resize(nx*ny, bad_data_double);
      s31_buf.resize(nx*ny, bad_data_double);
      s32_buf.resize(nx*ny, bad_data_double);

      LongArray curs;   // = { month-1, 0, 0 };
      LongArray dims;   // = { 1, ny, nx };
      NcVar var_p1_00  = get_nc_var(nc_file, var_name_p1_00);
      NcVar var_p2_00  = get_nc_var(nc_file, var_name_p2_00);
      NcVar var_t1_00  = get_nc_var(nc_file, var_name_t1_00);
      NcVar var_t2_00  = get_nc_var(nc_file, var_name_t2_00);
      NcVar var_s12_00 = get_nc_var(nc_file, var_name_s12_00);
      NcVar var_s13_00 = get_nc_var(nc_file, var_name_s13_00);
      NcVar var_s21_00 = get_nc_var(nc_file, var_name_s21_00);
      NcVar var_s23_00 = get_nc_var(nc_file, var_name_s23_00);
      NcVar var_s31_00 = get_nc_var(nc_file, var_name_s31_00);
      NcVar var_s32_00 = get_nc_var(nc_file, var_name_s32_00);

      curs.add(month-1);
      curs.add(0);
      curs.add(0);
      dims.add(1);
      dims.add(ny);
      dims.add(nx);

      if (IS_INVALID_NC(var_p1_00) || !get_nc_data(&var_p1_00, p1_buf, dims, curs)) {
         mlog << Error << "\n" << method_name
              << "Did not get p1_00\n\n";
         exit(1);
      }
      if (IS_INVALID_NC(var_p2_00) || !get_nc_data(&var_p2_00, p2_buf, dims, curs)) {
         mlog << Error << "\n" << method_name
              << "Did not get p2_00\n\n";
         exit(1);
      }
      if (IS_INVALID_NC(var_t1_00) || !get_nc_data(&var_t1_00, t1_buf, dims, curs)) {
         mlog << Error << "\n" << method_name
              << "Did not get t1_00\n\n";
         exit(1);
      }
      if (IS_INVALID_NC(var_t2_00) || !get_nc_data(&var_t2_00, t2_buf, dims, curs)) {
         mlog << Error << "\n" << method_name
              << "Did not get t2_00\n\n";
         exit(1);
      }
      if (IS_INVALID_NC(var_s12_00) || !get_nc_data(&var_s12_00, s12_buf, dims, curs)) {
         mlog << Error << "\n" << method_name
              << "Did not get s12_00\n\n";
         exit(1);
      }
      if (IS_INVALID_NC(var_s13_00) || !get_nc_data(&var_s12_00, s13_buf, dims, curs)) {
         mlog << Error << "\n" << method_name
              << "Did not get s13_00\n\n";
         exit(1);
      }
      if (IS_INVALID_NC(var_s21_00) || !get_nc_data(&var_s21_00, s21_buf, dims, curs)) {
         mlog << Error << "\n" << method_name
              << "Did not get s21_00\n\n";
         exit(1);
      }
      if (IS_INVALID_NC(var_s23_00) || !get_nc_data(&var_s23_00, s23_buf, dims, curs)) {
         mlog << Error << "\n" << method_name
              << "Did not get s23_00\n\n";
         exit(1);
      }
      if (IS_INVALID_NC(var_s31_00) || !get_nc_data(&var_s31_00, s31_buf, dims, curs)) {
         mlog << Error << "\n" << method_name
              << "Did not get s31_00\n\n";
         exit(1);
      }
      if (IS_INVALID_NC(var_s32_00) || !get_nc_data(&var_s32_00, s32_buf, dims, curs)) {
         mlog << Error << "\n" << method_name
              << "Did not get s32_00\n\n";
         exit(1);
      }
      nc_file->close();

      float duration = (float)(clock() - clock_time)/CLOCKS_PER_SEC;
      mlog << Debug(6) << method_name << "took " << duration << " seconds\n";
      if (standalone_debug_seeps) cout << method_name << "took " << duration  << " seconds\n";
   }
   catch(...) {

      set_seeps_ready(false);
      mlog << Error << "\n" << method_name
           << "encountered an error on reading " << filename << ".\n\n";

      exit(-1);
   } // end catch block

}

////////////////////////////////////////////////////////////////////////

void SeepsClimoGrid::print_all() {
   const char *method_name = "SeepsClimoGrid::print_all() -> ";
   if (standalone_debug_seeps) {
      int offset;
      offset = 0;
      cout << method_name << " p1_buf[" << offset << "] = " <<  p1_buf[offset]  << "\n";
      cout << method_name << " p2_buf[" << offset << "] = " <<  p2_buf[offset]  << "\n";
      cout << method_name << " t1_buf[" << offset << "] = " <<  t1_buf[offset]  << "\n";
      cout << method_name << " t2_buf[" << offset << "] = " <<  t2_buf[offset]  << "\n";
      cout << method_name << "s12_buf[" << offset << "] = " << s12_buf[offset]  << "\n";
      cout << method_name << "s13_buf[" << offset << "] = " << s13_buf[offset]  << "\n";
      cout << method_name << "s21_buf[" << offset << "] = " << s21_buf[offset]  << "\n";
      cout << method_name << "s23_buf[" << offset << "] = " << s23_buf[offset]  << "\n";
      cout << method_name << "s31_buf[" << offset << "] = " << s31_buf[offset]  << "\n";
      cout << method_name << "s32_buf[" << offset << "] = " << s32_buf[offset]  << "\n";

      offset = 400;
      cout << method_name << " p1_buf[" << offset << "] = " <<  p1_buf[offset]  << "\n";
      cout << method_name << " p2_buf[" << offset << "] = " <<  p2_buf[offset]  << "\n";
      cout << method_name << " t1_buf[" << offset << "] = " <<  t1_buf[offset]  << "\n";
      cout << method_name << " t2_buf[" << offset << "] = " <<  t2_buf[offset]  << "\n";
      cout << method_name << "s12_buf[" << offset << "] = " << s12_buf[offset]  << "\n";
      cout << method_name << "s13_buf[" << offset << "] = " << s13_buf[offset]  << "\n";
      cout << method_name << "s21_buf[" << offset << "] = " << s21_buf[offset]  << "\n";
      cout << method_name << "s23_buf[" << offset << "] = " << s23_buf[offset]  << "\n";
      cout << method_name << "s31_buf[" << offset << "] = " << s31_buf[offset]  << "\n";
      cout << method_name << "s32_buf[" << offset << "] = " << s32_buf[offset]  << "\n";

      offset = (nx*ny) - 1;
      cout << method_name << " p1_buf[" << offset << "] = " <<  p1_buf[offset]  << "\n";
      cout << method_name << " p2_buf[" << offset << "] = " <<  p2_buf[offset]  << "\n";
      cout << method_name << " t1_buf[" << offset << "] = " <<  t1_buf[offset]  << "\n";
      cout << method_name << " t2_buf[" << offset << "] = " <<  t2_buf[offset]  << "\n";
      cout << method_name << "s12_buf[" << offset << "] = " << s12_buf[offset]  << "\n";
      cout << method_name << "s13_buf[" << offset << "] = " << s13_buf[offset]  << "\n";
      cout << method_name << "s21_buf[" << offset << "] = " << s21_buf[offset]  << "\n";
      cout << method_name << "s23_buf[" << offset << "] = " << s23_buf[offset]  << "\n";
      cout << method_name << "s31_buf[" << offset << "] = " << s31_buf[offset]  << "\n";
      cout << method_name << "s32_buf[" << offset << "] = " << s32_buf[offset]  << "\n";
   }
}

////////////////////////////////////////////////////////////////////////

