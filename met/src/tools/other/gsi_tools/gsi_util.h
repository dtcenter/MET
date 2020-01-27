// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2020
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*

////////////////////////////////////////////////////////////////////////

#ifndef  __GSI_UTIL_H__
#define  __GSI_UTIL_H__

////////////////////////////////////////////////////////////////////////

using namespace std;

#include "vx_stat_out.h"

////////////////////////////////////////////////////////////////////////

// Constants
static const char   default_model[]       = "GSI";
static const char   default_desc[]        = "NA";
static const int    default_lead          = 0;
static const char   default_lev[]         = "NA";
static const char   default_obtype[]      = "NA";
static const char   default_vx_mask[]     = "NA";
static const char   default_interp_mthd[] = "NA";
static const int    default_interp_wdth   = 0;
static const char   default_thresh[]      = "NA";
static const double default_alpha         = bad_data_double;

static const int    bad_setup_qc          = -999;
static const char   key_sep[]             = ":";

static const char   conv_id_str[]         = "conv";

static const char  *micro_id_str [] = {
   "amsua", "amsub", "mhs",
   "msu",   "hsb",   "ssmi",
   "ssmis", "amsre", "atms"
};
static const int n_micro_id_str = sizeof(micro_id_str)/sizeof(*micro_id_str);

////////////////////////////////////////////////////////////////////////

struct ConvData {
   ConcatString var, obtype, sid;
   double lat, lon, prs, elv;
   unixtime fcst_ut, obs_ut;
   double obs, obs_v;
   double guess, guess_v;
   IntArray obs_qc;
   int hgt, prep_use, anly_use, setup_qc;
   double err_in, err_adj, err_fin, qc_wght;
   int n_use;
};

////////////////////////////////////////////////////////////////////////

struct RadData {
   ConcatString var;
   double lat, lon, elv;
   unixtime fcst_ut, obs_ut;
   double obs;
   double guess;
   IntArray obs_qc;
   int use, scan_pos;
   double sat_znth, sat_azmth;
   double sun_znth, sun_azmth, sun_glnt;
   double frac_wtr, frac_lnd, frac_ice, frac_snw;
   double sfc_twtr, sfc_tlnd, sfc_tice, sfc_tsnw;
   double tsoil, soilm;
   int land_type;
   double frac_veg, snw_dpth, sfc_wind;
   double frac_cld, ctop_prs;
   double tfnd, twarm, tcool, tzfnd;
   double obs_err, fcst_nobc, sfc_emis, stability;
   double prs_max_wgt;
   int n_use;
};

////////////////////////////////////////////////////////////////////////

static bool not_has_FCST_VALID_BEG = true;
static bool not_has_FCST_VALID_END = true;
static bool not_has_OBS_VALID_BEG  = true;
static bool not_has_OBS_VALID_END  = true;
static bool not_has_FCST_VAR       = true;
static bool not_has_OBS_VAR        = true;
static bool not_has_OBTYPE         = true;

////////////////////////////////////////////////////////////////////////

ConvData parse_conv_data(const ConvRecord &r, const int i);
RadData  parse_rad_data (const RadRecord  &r, const int i,
                         const int chval, const int use);

ConcatString get_conv_key(const ConvData &d);
ConcatString get_rad_key (const RadData &d);

int key_to_integer(const char * key);

////////////////////////////////////////////////////////////////////////

bool is_conv(const char *);
bool is_micro(const char *);
bool is_retr(const char *);

////////////////////////////////////////////////////////////////////////

void setup_header(StatHdrColumns &shc,
                  const StringArray &name, const StringArray &value,
                  const char *line_type);
void setup_table(AsciiTable &at);

////////////////////////////////////////////////////////////////////////

#endif   /*  __GSI_UTIL_H__  */

////////////////////////////////////////////////////////////////////////
