

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
constexpr char var_name_odfl_00[]   = "odfl_00";
constexpr char var_name_odfh_00[]   = "odfh_00";
constexpr char var_name_olfd_00[]   = "olfd_00";
constexpr char var_name_olfh_00[]   = "olfh_00";
constexpr char var_name_ohfd_00[]   = "ohfd_00";
constexpr char var_name_ohfl_00[]   = "ohfl_00";
constexpr char var_name_odfl_12[]   = "odfl_12";
constexpr char var_name_odfh_12[]   = "odfh_12";
constexpr char var_name_olfd_12[]   = "olfd_12";
constexpr char var_name_olfh_12[]   = "olfh_12";
constexpr char var_name_ohfd_12[]   = "ohfd_12";
constexpr char var_name_ohfl_12[]   = "ohfl_12";
constexpr char def_seeps_point_filename[] =
   "MET_BASE/climo/seeps/PPT24_seepsweights.nc";
constexpr char def_seeps_grid_filename[] =
   "MET_BASE/climo/seeps/PPT24_seepsweights_grid_v12.0.nc";

//density_radius = 0.75 degrees (83km; this is described as “the smallest possible
// value that ensures approximately equal representation of all subregions of Europe”.)
constexpr double density_radius = 0.75;
const double density_radius_rad = density_radius * rad_per_deg;

////////////////////////////////////////////////////////////////////////

struct SeepsScore { // For SEEPS_MPR
   int   obs_cat;   // i = obs category 0,1,2 (dry, light, heavy)
   int   fcst_cat;  // j = model category 0,1,2 (dry, light, heavy)
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

   int    n_obs;
   int    c_odfl;
   int    c_odfh;
   int    c_olfd;
   int    c_olfh;
   int    c_ohfd;
   int    c_ohfl;
   double s_odfl;
   double s_odfh;
   double s_olfd;
   double s_olfh;
   double s_ohfd;
   double s_ohfl;
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
      virtual void read_seeps_climo_grid(ConcatString filename) {};
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
      void read_seeps_climo_grid(ConcatString filename) override;

   public:

      SeepsClimo(ConcatString seeps_climo_name);
     ~SeepsClimo();

      SeepsRecord *get_record(int sid, int month, int hour);
      double get_seeps_category(int sid, double p_fcst, double p_obs,
                                int month, int hour);
      SeepsScore *get_seeps_score(int sid, double p_fcst, double p_obs,
                                  int month, int hour);

      void print_all();
      void print_record(SeepsRecord *record, bool with_header=false);

      //
      //
      //

};

////////////////////////////////////////////////////////////////////////

class SeepsClimoGrid : public SeepsClimoBase {

   private:

      int   month;
      int   hour; // not implemented
      int   nx;
      int   ny;
      double *p1_buf;
      double *p2_buf;
      double *t1_buf;
      double *t2_buf;
      double *s_odfl_buf;
      double *s_odfh_buf;
      double *s_olfd_buf;
      double *s_olfh_buf;
      double *s_ohfd_buf;
      double *s_ohfl_buf;

      void init_from_scratch();

   protected:
      void clear() override;
      ConcatString get_env_climo_name() override { return MET_ENV_SEEPS_GRID_CLIMO_NAME; };
      char *get_def_climo_name() override { return (char *)def_seeps_grid_filename; };
      void read_seeps_climo_grid(ConcatString filename) override;

   public:

      SeepsClimoGrid(int month, int hour, ConcatString seeps_climo_name);
     ~SeepsClimoGrid();

      SeepsScore *get_record(int ix, int iy, double p_fcst, double p_obs);
      double get_seeps_score(int offset, int obs_cat, int fcst_cat);
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

