

////////////////////////////////////////////////////////////////////////


#ifndef  __SEEPS_H__
#define  __SEEPS_H__


////////////////////////////////////////////////////////////////////////


#include <map>

#include "concat_string.h"


////////////////////////////////////////////////////////////////////////

#define SEEPS_MONTH         12
#define SEEPS_MATRIX_SIZE   9

#define SAMPLE_STATION_ID   11035

////////////////////////////////////////////////////////////////////////

static const char *MET_ENV_SEEPS_CLIMO_NAME = "MET_SEEPS_CLIMO_NAME";
static const char *MET_ENV_SEEPS_CLIMO_GRID_NAME = "MET_SEEPS_CLIMO_GRID_NAME";

static const char *dim_name_nstn      = "nstn";

static const char *var_name_p1_00     = "p1_00";
static const char *var_name_p2_00     = "p2_00";
static const char *var_name_t1_00     = "t1_00";
static const char *var_name_t2_00     = "t2_00";
static const char *var_name_p1_12     = "p1_12";
static const char *var_name_p2_12     = "p2_12";
static const char *var_name_t1_12     = "t1_12";
static const char *var_name_t2_12     = "t2_12";
static const char *var_name_matrix_00 = "matrix_00";
static const char *var_name_matrix_12 = "matrix_12";
static const char *var_name_s12_00    = "s12_00";
static const char *var_name_s13_00    = "s13_00";
static const char *var_name_s21_00    = "s21_00";
static const char *var_name_s23_00    = "s23_00";
static const char *var_name_s31_00    = "s31_00";
static const char *var_name_s32_00    = "s32_00";
static const char *var_name_s12_12    = "s12_12";
static const char *var_name_s13_12    = "s13_12";
static const char *var_name_s21_12    = "s21_12";
static const char *var_name_s23_12    = "s23_12";
static const char *var_name_s31_12    = "s31_12";
static const char *var_name_s32_12    = "s32_12";

//density_radius = 0.75 degrees (83km; this is described as “the smallest possible
// value that ensures approximately equal representation of all subregions of Europe”.)
static double density_radius = 0.75;

////////////////////////////////////////////////////////////////////////

struct SeepsScore { // For SEEPS_MPR
   int   obs_cat;   // i = obs category 0,1,2
   int   fcst_cat;  // j = model category 0,1,2
   int   s_idx;     // index for 3 by 3 matrix as 1 dimensional (fcst_cat*3)+obs_cat
   float p1;
   float p2;
   float t1;
   float t2;
   float score;
};

////////////////////////////////////////////////////////////////////////

struct SeepsAggScore {  // For SEEPS

   void clear();
   
   int   n_obs;
   int   c12;
   int   c13;
   int   c21;
   int   c23;
   int   c31;
   int   c32;
   float s12;
   float s13;
   float s21;
   float s23;
   float s31;
   float s32;
   float pv1;   // marginal probabilities of the observed values
   float pv2;
   float pv3;
   float pf1;   // marginal probabilities of the forecast values
   float pf2;
   float pf3;
   float mean_fcst;
   float mean_obs;
   float score;
   float weighted_score;
};

////////////////////////////////////////////////////////////////////////


struct SeepsRecord {
   int   sid;
   int   month;    // 1 to 12, not 0 to 11
   float lat;
   float lon;
   float elv;
   float p1;
   float p2;
   float t1;
   float t2;
   float scores[SEEPS_MATRIX_SIZE];
};

////////////////////////////////////////////////////////////////////////

struct SeepsClimoRecord {
   int   sid;
   float lat;
   float lon;
   float elv;
   float p1[SEEPS_MONTH];
   float p2[SEEPS_MONTH];
   float t1[SEEPS_MONTH];
   float t2[SEEPS_MONTH];
   float scores[SEEPS_MONTH][SEEPS_MATRIX_SIZE];
};

////////////////////////////////////////////////////////////////////////

class SeepsClimo {

   private:

      bool seeps_ready;
      int nstn;
      std::map<int,SeepsClimoRecord *> seeps_score_00_map;
      std::map<int,SeepsClimoRecord *> seeps_score_12_map;

      SeepsClimoRecord *create_climo_record(int sid, float lat, float lon, float elv,
                                            float *p1, float *p2, float *t1, float *t2, 
                                            float *scores);
      ConcatString get_seeps_climo_filename();
      void print_record(SeepsClimoRecord *record, bool with_header=false);
      void read_records(ConcatString filename);

   public:

      SeepsClimo();
     ~SeepsClimo();

      void clear();
      SeepsRecord *get_record(int sid, int month, int hour=0);
      float get_score(int sid, float p_fcst, float p_obs, int month, int hour=0);
      SeepsScore *get_seeps_score(int sid, float p_fcst, float p_obs, int month, int hour=0);
      void print_all();
      void print_record(SeepsRecord *record, bool with_header=false);

      //
      //
      //

      SeepsRecord get_seeps_record(int sid) const;

};

////////////////////////////////////////////////////////////////////////

class SeepsClimoGrid {

   private:

      int   month;
      int   hour;
      int   nx;
      int   ny;
      float *p1_buf;
      float *p2_buf;
      float *t1_buf;
      float *t2_buf;
      float *s12_buf;
      float *s13_buf;
      float *s21_buf;
      float *s23_buf;
      float *s31_buf;
      float *s32_buf;

      bool seeps_ready;
      //int nstn;
      //std::map<int,SeepsClimoRecord *> seeps_score_00_map;
      //std::map<int,SeepsClimoRecord *> seeps_score_12_map;
      //
      //SeepsClimoRecord *create_climo_record(int sid, float lat, float lon, float elv,
      //                                      float *p1, float *p2, float *t1, float *t2,
      //                                      float *scores);
      ConcatString get_seeps_climo_filename();
      //void print_record(SeepsClimoRecord *record, bool with_header=false);
      void read_seeps_scores(ConcatString filename);

   public:

      SeepsClimoGrid(int month, int hour);
     ~SeepsClimoGrid();

      void clear();
      SeepsScore *get_record(int ix, int iy, float p_fcst, float p_obs);
      float get_score(int offset, int obs_cat, int fcst_cat);
      float get_score(int ix, int iy, float p_fcst, float p_obs);
      void print_all();
      //void print_record(bool with_header=false);

      //
      //
      //

};


////////////////////////////////////////////////////////////////////////

extern SeepsClimo *get_seeps_climo();
extern SeepsClimoGrid *get_seeps_climo_grid(int month, int hour=0);

extern void release_seeps_climo();
extern void release_seeps_climo_grid();

////////////////////////////////////////////////////////////////////////


#endif   /*  __SEEPS_H__  */


////////////////////////////////////////////////////////////////////////

