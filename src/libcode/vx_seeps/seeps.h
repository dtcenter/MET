

////////////////////////////////////////////////////////////////////////


#ifndef  __SEEPS_H__
#define  __SEEPS_H__


////////////////////////////////////////////////////////////////////////


#include <map>

#include "concat_string.h"


////////////////////////////////////////////////////////////////////////

constexpr int SEEPS_MONTH       = 12;
constexpr int SEEPS_MATRIX_SIZE = 9;

constexpr int SAMPLE_STATION_ID = 11035;

////////////////////////////////////////////////////////////////////////

constexpr char MET_ENV_SEEPS_POINT_CLIMO_NAME[] = "MET_SEEPS_POINT_CLIMO_NAME";
constexpr char MET_ENV_SEEPS_GRID_CLIMO_NAME[]  = "MET_SEEPS_GRID_CLIMO_NAME";

constexpr char dim_name_nstn[]      = "nstn";

constexpr char var_name_p1_00[]     = "p1_00";
constexpr char var_name_p2_00[]     = "p2_00";
constexpr char var_name_t1_00[]     = "t1_00";
constexpr char var_name_t2_00[]     = "t2_00";
constexpr char var_name_p1_12[]     = "p1_12";
constexpr char var_name_p2_12[]     = "p2_12";
constexpr char var_name_t1_12[]     = "t1_12";
constexpr char var_name_t2_12[]     = "t2_12";
constexpr char var_name_matrix_00[] = "matrix_00";
constexpr char var_name_matrix_12[] = "matrix_12";
constexpr char var_name_s12_00[]    = "s12_00";
constexpr char var_name_s13_00[]    = "s13_00";
constexpr char var_name_s21_00[]    = "s21_00";
constexpr char var_name_s23_00[]    = "s23_00";
constexpr char var_name_s31_00[]    = "s31_00";
constexpr char var_name_s32_00[]    = "s32_00";
constexpr char var_name_s12_12[]    = "s12_12";
constexpr char var_name_s13_12[]    = "s13_12";
constexpr char var_name_s21_12[]    = "s21_12";
constexpr char var_name_s23_12[]    = "s23_12";
constexpr char var_name_s31_12[]    = "s31_12";
constexpr char var_name_s32_12[]    = "s32_12";

constexpr char def_seeps_point_filename[] =
   "MET_BASE/climo/seeps/PPT24_seepsweights.nc";
constexpr char def_seeps_grid_filename[] =
   "MET_BASE/climo/seeps/PPT24_seepsweights_grid.nc";

//density_radius = 0.75 degrees (83km; this is described as “the smallest possible
// value that ensures approximately equal representation of all subregions of Europe”.)
constexpr double density_radius = 0.75;
const double density_radius_rad = density_radius * rad_per_deg;

////////////////////////////////////////////////////////////////////////

struct SeepsScore { // For SEEPS_MPR
   int   obs_cat;   // i = obs category 0,1,2
   int   fcst_cat;  // j = model category 0,1,2
   int   s_idx;     // index for 3 by 3 matrix as 1 dimensional (fcst_cat*3)+obs_cat
   double p1;
   double p2;
   double t1;
   double t2;
   double score;
};

////////////////////////////////////////////////////////////////////////

struct SeepsAggScore {  // For SEEPS
   void clear();
   SeepsAggScore & operator+=(const SeepsAggScore &);

   int   n_obs;
   int   c12;
   int   c13;
   int   c21;
   int   c23;
   int   c31;
   int   c32;
   double s12;
   double s13;
   double s21;
   double s23;
   double s31;
   double s32;
   double pv1;   // marginal probabilities of the observed values
   double pv2;
   double pv3;
   double pf1;   // marginal probabilities of the forecast values
   double pf2;
   double pf3;
   double mean_fcst;
   double mean_obs;
   double score;
   double weighted_score;
};

////////////////////////////////////////////////////////////////////////


struct SeepsRecord {
   int   sid;
   int   month;    // 1 to 12, not 0 to 11
   double lat;
   double lon;
   double elv;
   double p1;
   double p2;
   double t1;
   double t2;
   double scores[SEEPS_MATRIX_SIZE];
};

////////////////////////////////////////////////////////////////////////

struct SeepsClimoRecord {
   int   sid;
   double lat;
   double lon;
   double elv;
   double p1[SEEPS_MONTH];
   double p2[SEEPS_MONTH];
   double t1[SEEPS_MONTH];
   double t2[SEEPS_MONTH];
   double scores[SEEPS_MONTH][SEEPS_MATRIX_SIZE];
};

////////////////////////////////////////////////////////////////////////

class SeepsClimoBase {

   private:
      bool seeps_ready;
      int filtered_count;
      SingleThresh seeps_p1_thresh;      // Range of SEEPS p1 (probability of being dry)
      ConcatString climo_file_name;

   protected:

      bool is_seeps_ready() { return seeps_ready; };
      void increase_filtered_count() { filtered_count++; };
      bool check_seeps_p1_thresh(double p1) { return seeps_p1_thresh.check(p1); };
      ConcatString get_climo_filename();

      virtual void clear();
      virtual ConcatString get_env_climo_name() { return "not defined"; };
      virtual char *get_def_climo_name() { return nullptr; };
      virtual void read_seeps_scores(ConcatString filename) {};
      void set_seeps_ready(bool _seeps_ready) { seeps_ready = _seeps_ready; };

   public:

      SeepsClimoBase(ConcatString seeps_climo_name);
      virtual ~SeepsClimoBase();
      void set_p1_thresh(const SingleThresh &p1_thresh);
      int get_filtered_count() const;

};

////////////////////////////////////////////////////////////////////////

class SeepsClimo : public SeepsClimoBase {

   private:

      int nstn;
      std::map<int,SeepsClimoRecord *> seeps_score_00_map;
      std::map<int,SeepsClimoRecord *> seeps_score_12_map;

      SeepsClimoRecord *create_climo_record(int sid, double lat, double lon, double elv,
                                            double *p1, double *p2, double *t1, double *t2, 
                                            double *scores);
      void print_record(SeepsClimoRecord *record, bool with_header=false);
      void read_records(ConcatString filename);

   protected:
      void clear() override;
      ConcatString get_env_climo_name() override { return MET_ENV_SEEPS_POINT_CLIMO_NAME; };
      char *get_def_climo_name() override { return (char *)def_seeps_point_filename; };
      void read_seeps_scores(ConcatString filename) override;

   public:

      SeepsClimo(ConcatString seeps_climo_name);
     ~SeepsClimo();

      SeepsRecord *get_record(int sid, int month, int hour);
      double get_score(int sid, double p_fcst, double p_obs, int month, int hour);
      SeepsScore *get_seeps_score(int sid, double p_fcst, double p_obs, int month, int hour);
      void print_all();
      void print_record(SeepsRecord *record, bool with_header=false);

      //
      //
      //

      SeepsRecord get_seeps_record(int sid) const;

};

////////////////////////////////////////////////////////////////////////

class SeepsClimoGrid : public SeepsClimoBase {

   private:

      int   month;
      int   hour; // not implemented
      int   nx;
      int   ny;
      std::vector<double> p1_buf;
      std::vector<double> p2_buf;
      std::vector<double> t1_buf;
      std::vector<double> t2_buf;
      std::vector<double> s12_buf;
      std::vector<double> s13_buf;
      std::vector<double> s21_buf;
      std::vector<double> s23_buf;
      std::vector<double> s31_buf;
      std::vector<double> s32_buf;

      void init_from_scratch();

   protected:
      void clear() override;
      ConcatString get_env_climo_name() override { return MET_ENV_SEEPS_GRID_CLIMO_NAME; };
      char *get_def_climo_name() override { return (char *)def_seeps_grid_filename; };
      void read_seeps_scores(ConcatString filename) override;

   public:

      SeepsClimoGrid(int month, int hour, ConcatString seeps_climo_name);
     ~SeepsClimoGrid();

      SeepsScore *get_record(int ix, int iy, double p_fcst, double p_obs);
      double get_score(int offset, int obs_cat, int fcst_cat);
      double get_score(int ix, int iy, double p_fcst, double p_obs);
      void print_all();

      //
      //
      //

};


////////////////////////////////////////////////////////////////////////

inline int SeepsClimoBase::get_filtered_count() const { return filtered_count; }

////////////////////////////////////////////////////////////////////////

extern SeepsClimo *get_seeps_climo(ConcatString seeps_point_climo_name);
extern SeepsClimoGrid *get_seeps_climo_grid(int month, ConcatString seeps_grid_climo_name, int hour=0);

extern void release_seeps_climo();
extern void release_seeps_climo_grid();

////////////////////////////////////////////////////////////////////////


#endif   /*  __SEEPS_H__  */


////////////////////////////////////////////////////////////////////////

