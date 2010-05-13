// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2007
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*

#ifndef  __VX_PAIR_DATA_H__
#define  __VX_PAIR_DATA_H__

////////////////////////////////////////////////////////////////////////

#include "vx_util/vx_util.h"
#include "vx_data_grids/grid.h"
#include "vx_wrfdata/vx_wrfdata.h"
#include "vx_contable/vx_contable.h"

////////////////////////////////////////////////////////////////////////

static const int pair_data_alloc_jump = 1000;

////////////////////////////////////////////////////////////////////////
//
// Class to store Grib Code Information
//
////////////////////////////////////////////////////////////////////////

//
// Enumeration for level types
//
enum LevelType {

    NoLevel    = 0,
    AccumLevel = 1,
    VertLevel  = 2,
    PresLevel  = 3,
    RecNumber  = 4
};

class GCInfo {

   private:
      void init_from_scratch();
      void assign(const GCInfo &);

   public:

      GCInfo();
      ~GCInfo();
      GCInfo(const GCInfo &);
      GCInfo & operator=(const GCInfo &);
      int operator==(const GCInfo &);

      //////////////////////////////////////////////////////////////////

      ConcatString abbr_str; // GRIB code abbreviation string
      ConcatString lvl_str;  // Level defined in config file
      ConcatString info_str; // abbr_str/lvl_str

      int          code;     // GRIB code itself

      LevelType    lvl_type; // Type of level specified
      int          lvl_1;    // First level defined
      int          lvl_2;    // Second level in case of a range

      // Flag to indicate whether this field should be used in
      // vector calculations
      int          vflag;

      // Flag to indicate whether this is a probability field
      int          pflag;

      //////////////////////////////////////////////////////////////////

      void clear();

      void set_gcinfo(const char *, int);

      void set_abbr_str(const char *);
      void set_lvl_str(const char *);
      void set_info_str(const char *);
};

////////////////////////////////////////////////////////////////////////
//
// Class to store Fcst-Climo-Obs pair data
//
////////////////////////////////////////////////////////////////////////

class PairData {

   private:

      void init_from_scratch();
      void assign(const PairData &);

   public:

      PairData();
      ~PairData();
      PairData(const PairData &);
      PairData & operator=(const PairData &);

      //////////////////////////////////////////////////////////////////

      // Masking area applied to the forecast and climo fields
      ConcatString mask_name;
      WrfData     *mask_wd_ptr; // Pointer to the masking field
                                // which is not allocated

      // The verifying message type
      ConcatString msg_typ;

      // Interpolation method and width used
      InterpMthd interp_mthd;
      int        interp_wdth;

      // Forecast, climotological, and observation data pairs
      NumArray f_na;
      NumArray c_na;
      NumArray o_na;

      // Observation latitude, longitude, pressure, and elevation
      NumArray lat_na;
      NumArray lon_na;
      NumArray lvl_na;
      NumArray elv_na;
      int      n_pair;

      //////////////////////////////////////////////////////////////////

      void clear();

      void set_mask_name(const char *);
      void set_mask_wd_ptr(WrfData *);

      void set_msg_typ(const char *);

      void set_interp_mthd(const char *);
      void set_interp_mthd(InterpMthd);
      void set_interp_wdth(int);

      void add_pair(double, double, double, double,
                    double, double, double);
};

////////////////////////////////////////////////////////////////////////
//
// Class to store a variety of PairData objects for each Grib Code
//
////////////////////////////////////////////////////////////////////////

class GCPairData {

   private:

      void init_from_scratch();
      void assign(const GCPairData &);

   public:

      GCPairData();
      ~GCPairData();
      GCPairData(const GCPairData &);
      GCPairData & operator=(const GCPairData &);

      //////////////////////////////////////////////////////////////////
      //
      // The grib code information to be verified
      //
      //////////////////////////////////////////////////////////////////

      GCInfo fcst_gci;         // Forecast field
      GCInfo obs_gci;          // Observation field

      double interp_thresh;    // Threshold between 0 and 1 used when
                               // interpolating the forecasts to the
                               // observation location.

      //////////////////////////////////////////////////////////////////
      //
      // Forecast and climatological fields falling between the
      // requested levels.  Store the number of fields, the pressure
      // level corresponding to each field, and pointers to the data.
      //
      //////////////////////////////////////////////////////////////////

      int      n_fcst;         // Number of fcst fields
      NumArray fcst_lvl;       // Array of vertical levels
      WrfData  **fcst_wd_ptr;  // Array of pointers to the fcst data

      int      n_climo;        // Number of climo fields
      NumArray climo_lvl;      // Array of vertical levels
      WrfData  **climo_wd_ptr; // Array of pointers to the climo data

      //////////////////////////////////////////////////////////////////

      unixtime beg_ut;         // Beginning of valid time window
      unixtime end_ut;         // End of valid time window

      //////////////////////////////////////////////////////////////////

      int      n_msg_typ;      // Number of verifying message types

      int      n_mask;         // Total number of masking regions
                               // of masking WrfData fields or SIDs

      int      n_interp;       // Number of interpolation techniques

      //////////////////////////////////////////////////////////////////

      PairData ***pd;          // 3-Dim Array of PairData objects
                               // as [n_msg_typ][n_mask][n_interp]

      //////////////////////////////////////////////////////////////////

      void clear();

      void set_fcst_gci(const GCInfo &);
      void set_obs_gci(const GCInfo &);
      void set_interp_thresh(double);

      void set_n_fcst(int);
      void set_fcst_lvl(int, double);
      void set_fcst_wd_ptr(int, WrfData *);

      void set_n_climo(int);
      void set_climo_lvl(int, double);
      void set_climo_wd_ptr(int, WrfData *);

      void set_beg_ut(const unixtime);
      void set_end_ut(const unixtime);

      // Should call set_pd_size prior to setting up the msg_typ, mask
      // or interpolation stuff
      void set_pd_size(int, int, int);

      void set_msg_typ(int, const char *);

      void set_mask_wd(int, const char *, WrfData *);

      void set_interp(int, const char *, int);
      void set_interp(int, InterpMthd, int);

      void add_obs(float *, char *, char *, unixtime, float *, Grid &);
      void find_vert_lvl(int, double, int &, int &);

      int  get_n_pair();

      double compute_interp(int, double, double, int, double, int, int);
      double compute_horz_interp(WrfData *, double, double, int, int);
      double compute_vert_pinterp(double, double, double, double, double);
      double compute_vert_zinterp(double, double, double, double, double);
};

////////////////////////////////////////////////////////////////////////
//
// Class to store Ensemble-Observation pair data
//
////////////////////////////////////////////////////////////////////////

class EnsPairData {

   private:

      void init_from_scratch();
      void assign(const EnsPairData &);

   public:

      EnsPairData();
      ~EnsPairData();
      EnsPairData(const EnsPairData &);
      EnsPairData & operator=(const EnsPairData &);

      //////////////////////////////////////////////////////////////////

      // Masking area applied to the forecast and climo fields
      ConcatString mask_name;
      WrfData     *mask_wd_ptr; // Pointer to the masking field
                                // which is not allocated

      // The verifying message type
      ConcatString msg_typ;

      // Interpolation method and width used
      InterpMthd interp_mthd;
      int        interp_wdth;

      // Ensemble, observation, and rank data pairs
      NumArray  o_na; // Observation values [n_pair]
      NumArray *e_na; // Ensemble values [n_pair][n_ens]
      NumArray  v_na; // Number of valid ensemble values [n_pair]
      NumArray  r_na; // Observation ranks [n_pair]

      // Observation latitude, longitude, pressure, and elevation
      StringArray sid_sa;
      NumArray    lat_na;
      NumArray    lon_na;
      NumArray      x_na;
      NumArray      y_na;
      NumArray    lvl_na;
      NumArray    elv_na;
      int         n_pair;

      //////////////////////////////////////////////////////////////////

      void clear();

      void set_mask_name(const char *);
      void set_mask_wd_ptr(WrfData *);

      void set_msg_typ(const char *);

      void set_interp_mthd(const char *);
      void set_interp_mthd(InterpMthd);
      void set_interp_wdth(int);

      void add_obs(const char *, double, double, double, double, 
                   double, double, double);
      void add_obs(int, int, double);
      void add_ens(int, double);
      void set_size();

      void compute_rank(int);
      void compute_rhist(int, NumArray &);
};

////////////////////////////////////////////////////////////////////////
//
// Class to store a variety of EnsPairData objects for each field
//
////////////////////////////////////////////////////////////////////////

class GCEnsPairData {

   private:

      void init_from_scratch();
      void assign(const GCEnsPairData &);

   public:

      GCEnsPairData();
      ~GCEnsPairData();
      GCEnsPairData(const GCEnsPairData &);
      GCEnsPairData & operator=(const GCEnsPairData &);

      //////////////////////////////////////////////////////////////////
      //
      // The grib code information to be verified
      //
      //////////////////////////////////////////////////////////////////

      GCInfo fcst_gci;         // Forecast field
      GCInfo obs_gci;          // Observation field

      double interp_thresh;    // Threshold between 0 and 1 used when
                               // interpolating the forecasts to the
                               // observation location.

      //////////////////////////////////////////////////////////////////
      //
      // Forecast fields falling between the requested levels.  Store
      // the number of fields, the pressure level corresponding to each
      // field, and pointers to the data.
      //
      //////////////////////////////////////////////////////////////////

      int      n_fcst;         // Number of fcst fields
      NumArray fcst_lvl;       // Array of vertical levels
      WrfData  **fcst_wd_ptr;  // Array of pointers to the fcst data

      //////////////////////////////////////////////////////////////////

      unixtime beg_ut;         // Beginning of valid time window
      unixtime end_ut;         // End of valid time window

      //////////////////////////////////////////////////////////////////

      int      n_msg_typ;      // Number of verifying message types

      int      n_mask;         // Total number of masking regions
                               // of masking WrfData fields or SIDs

      int      n_interp;       // Number of interpolation techniques

      //////////////////////////////////////////////////////////////////

      EnsPairData ***pd;       // 3-Dim Array of PairData objects
                               // as [n_msg_typ][n_mask][n_interp]

      //////////////////////////////////////////////////////////////////

      void clear();

      void set_fcst_gci(const GCInfo &);
      void set_obs_gci(const GCInfo &);
      void set_interp_thresh(double);

      void set_n_fcst(int);
      void set_fcst_lvl(int, double);
      void set_fcst_wd_ptr(int, WrfData *);

      void set_beg_ut(const unixtime);
      void set_end_ut(const unixtime);

      // Should call set_pd_size prior to setting up the msg_typ, mask
      // or interpolation stuff
      void set_pd_size(int, int, int);

      void set_ens_size();

      void set_msg_typ(int, const char *);

      void set_mask_wd(int, const char *, WrfData *);

      void set_interp(int, const char *, int);
      void set_interp(int, InterpMthd, int);

      void add_obs(float *, const char *, const char *, unixtime, float *, Grid &);
      void add_ens();
      void add_miss();

      void find_vert_lvl(double, int &, int &);

      int  get_n_pair();

      double compute_interp(double, double, int, double, int, int);
      double compute_horz_interp(WrfData *, double, double, int, int);
      double compute_vert_pinterp(double, double, double, double, double);
      double compute_vert_zinterp(double, double, double, double, double);
};

////////////////////////////////////////////////////////////////////////

#endif   // __VX_PAIR_DATA_H__

////////////////////////////////////////////////////////////////////////
