

////////////////////////////////////////////////////////////////////////


using namespace std;

#include <iostream>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <cmath>
#include <time.h>

#include <netcdf>
using namespace netCDF;

#include "file_exists.h"

#include "string_fxns.h"
#include "vx_log.h"
#include "nc_utils.h"
#include "seeps.h"

////////////////////////////////////////////////////////////////////////

bool standalone_debug_seeps = false;

static SeepsClimo *seeps_Climo = 0;

static const char *def_seeps_filename = "MET_BASE/climo/seeps/PPT24_seepsweights.nc";

static const char *var_name_sid       = "sid";
static const char *var_name_lat       = "lat";
static const char *var_name_lon       = "lon";
static const char *var_name_elv       = "elv";

////////////////////////////////////////////////////////////////////////

SeepsClimo *get_seeps_climo() {
   if (! seeps_Climo) seeps_Climo = new SeepsClimo();
   return seeps_Climo;
}

////////////////////////////////////////////////////////////////////////

void release_seeps_climo() {
   if (seeps_Climo) { delete seeps_Climo; seeps_Climo = 0; }
}

////////////////////////////////////////////////////////////////////////


SeepsClimo::SeepsClimo() {

   ConcatString seeps_name= get_seeps_climo_filename();
   if (file_exists(seeps_name.c_str())) {
      read_records(seeps_name);
   }
}

////////////////////////////////////////////////////////////////////////

SeepsClimo::~SeepsClimo() {
   clear();
}

////////////////////////////////////////////////////////////////////////

void SeepsClimo::clear() {
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
      int sid, float lat, float lon, float elv,
      float *p1, float *p2, float *t1, float *t2, float *scores) {
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
                            (idx+1), record->p1[idx], record->p2[idx], record->t1[idx], record->t2[idx]);
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
   SeepsRecord *record = NULL;
   const char *method_name = "SeepsClimo::get_record() -> ";

   if (seeps_ready) {
      SeepsClimoRecord *climo_record = NULL;
      map<int,SeepsClimoRecord *>::iterator it;
      if (hour < 6 || hour >= 18) {
         it = seeps_score_00_map.find(sid);
         if (it != seeps_score_00_map.end()) climo_record = it->second;
      }
      else {
         it = seeps_score_12_map.find(sid);
         if (it != seeps_score_12_map.end()) climo_record = it->second;
      }
      if (climo_record) {

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
   }
   else {
      mlog << Error << "\n" << method_name
           << "The SEEPS climo data is missing!"
           << " Turn off SEEPS and SEEPS_MPR to continue\n\n";
      exit(1);
   }
   return record;
}

////////////////////////////////////////////////////////////////////////

ConcatString SeepsClimo::get_seeps_climo_filename() {
   ConcatString seeps_filename;
   const char *method_name = "SeepsClimo::get_seeps_climo_filename() -> ";

   // Use the MET_TMP_DIR environment variable, if set.
   bool use_env = get_env(MET_ENV_SEEPS_CLIMO_NAME, seeps_filename);
   if(use_env) {
      seeps_filename = replace_path(seeps_filename);
   }
   else seeps_filename = replace_path(def_seeps_filename);

   if (seeps_ready = file_exists(seeps_filename.c_str())) {
      mlog << Debug(7) << method_name << "SEEPS climo name=\""
           << seeps_filename.c_str() << "\"\n";
   }
   else {
      ConcatString message = " ";
      if (use_env) {
         message.add("from the env. name ");
         message.add(MET_ENV_SEEPS_CLIMO_NAME);
      }
      mlog << Warning << "\n" << method_name
           << "The SEEPS climo name \"" << seeps_filename.c_str()
           << "\"" << message << " does not exist!\n\n";
   }

   return seeps_filename;
}

////////////////////////////////////////////////////////////////////////

float SeepsClimo::get_score(int sid, float p_fcst, float p_obs,
                            int month, int hour) {
   float score = (float)bad_data_double;
   SeepsRecord *record = get_record(sid, month, hour);

   if (NULL != record) {
      // Determine location in contingency table
      int ic = (p_obs>record->t1)+(p_obs>record->t2);
      int jc = (p_fcst>record->t1)+(p_fcst>record->t2);

      score = record->scores[(jc*3)+ic];
      delete record;
   }

   return score;
}

////////////////////////////////////////////////////////////////////////

SeepsScore *SeepsClimo::get_seeps_score(int sid, float p_fcst,
                                        float p_obs, int month, int hour) {
   SeepsScore *score = NULL;
   SeepsRecord *record = get_record(sid, month, hour);

   if (NULL != record) {
      score = new SeepsScore();
      score->p1 = record->p1;
      score->p2 = record->p1;
      score->t1 = record->t1;
      score->t2 = record->t2;

      score->obs_cat = (p_obs>record->t1)+(p_obs>record->t2);
      score->model_cat = (p_fcst>record->t1)+(p_fcst>record->t2);
      score->score = record->scores[(score->model_cat*3)+score->obs_cat];
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

void SeepsClimo::read_records(ConcatString filename) {
   clock_t clock_time = clock();
   float p1_00_buf[SEEPS_MONTH];
   float p2_00_buf[SEEPS_MONTH];
   float t1_00_buf[SEEPS_MONTH];
   float t2_00_buf[SEEPS_MONTH];
   float p1_12_buf[SEEPS_MONTH];
   float p2_12_buf[SEEPS_MONTH];
   float t1_12_buf[SEEPS_MONTH];
   float t2_12_buf[SEEPS_MONTH];
   float matrix_00_buf[SEEPS_MONTH*SEEPS_MATRIX_SIZE];
   float matrix_12_buf[SEEPS_MONTH*SEEPS_MATRIX_SIZE];
   NcFile *nc_file = open_ncfile(filename.c_str());
   const char *method_name = "SeepsClimo::read_records() -> ";

   // dimensions: month = 12 ; nstn = 5293 ; nmatrix = 9 ;
   get_dim(nc_file, dim_name_nstn, nstn, true);
   mlog << Debug(3) << method_name << "dimension nstn = " << nstn << "\n";
   if (standalone_debug_seeps) cout << "dimension nstn = " << nstn << "\n";

   int   *sid_array = new int[nstn];
   float *lat_array = new float[nstn];
   float *lon_array = new float[nstn];
   float *elv_array = new float[nstn];
   float *p1_00_array = new float[nstn*SEEPS_MONTH];
   float *p2_00_array = new float[nstn*SEEPS_MONTH];
   float *t1_00_array = new float[nstn*SEEPS_MONTH];
   float *t2_00_array = new float[nstn*SEEPS_MONTH];
   float *p1_12_array = new float[nstn*SEEPS_MONTH];
   float *p2_12_array = new float[nstn*SEEPS_MONTH];
   float *t1_12_array = new float[nstn*SEEPS_MONTH];
   float *t2_12_array = new float[nstn*SEEPS_MONTH];
   float *matrix_00_array = new float[nstn*SEEPS_MONTH*SEEPS_MATRIX_SIZE];
   float *matrix_12_array = new float[nstn*SEEPS_MONTH*SEEPS_MATRIX_SIZE];
   
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

   if (sid_array) delete [] sid_array;
   if (lat_array) delete [] lat_array;
   if (lon_array) delete [] lon_array;
   if (elv_array) delete [] elv_array;
   if (p1_00_array) delete [] p1_00_array;
   if (p2_00_array) delete [] p2_00_array;
   if (t1_00_array) delete [] t1_00_array;
   if (t2_00_array) delete [] t2_00_array;
   if (p1_12_array) delete [] p1_12_array;
   if (p2_12_array) delete [] p2_12_array;
   if (t1_12_array) delete [] t1_12_array;
   if (t2_12_array) delete [] t2_12_array;
   if (matrix_00_array) delete [] matrix_00_array;
   if (matrix_12_array) delete [] matrix_12_array;

   nc_file->close();

   float duration = (float)(clock() - clock_time)/CLOCKS_PER_SEC;
   mlog << Debug(4) << method_name << "took " << duration << " seconds\n";
   if (standalone_debug_seeps) cout << method_name << "took " << duration  << " seconds\n";
   
}

////////////////////////////////////////////////////////////////////////

void SeepsAggScore::init() {

   n_obs = 0;
   c12 = 0;
   c13 = 0;
   c21 = 0;
   c23 = 0;
   c31 = 0;
   c32 = 0;
   s12 = 0.;
   s13 = 0.;
   s21 = 0.;
   s23 = 0.;
   s31 = 0.;
   s32 = 0.;
   pv1 = 0.;
   pv2 = 0.;
   pv3 = 0.;
   pf1 = 0.;
   pf2 = 0.;
   pf3 = 0.;
   mean_fcst = bad_data_double;
   mean_obs = bad_data_double;
   score = bad_data_double;

}

////////////////////////////////////////////////////////////////////////
