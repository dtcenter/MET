// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2007
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*

////////////////////////////////////////////////////////////////////////

using namespace std;

#include <iostream>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <cmath>

#include "vx_met_util/met_stats.h"
#include "vx_met_util/compute_ci.h"
#include "vx_met_util/constants.h"
#include "vx_met_util/conversions.h"
#include "vx_util/vx_util.h"
#include "vx_grib_classes/grib_strings.h"
#include "vx_wrfdata/vx_wrfdata.h"

////////////////////////////////////////////////////////////////////////
//
// Code for class GCInfo
//
////////////////////////////////////////////////////////////////////////

GCInfo::GCInfo() {
   init_from_scratch();
}

////////////////////////////////////////////////////////////////////////

GCInfo::~GCInfo() {
   clear();
}

////////////////////////////////////////////////////////////////////////

GCInfo::GCInfo(const GCInfo &c) {

   init_from_scratch();

   assign(c);
}

////////////////////////////////////////////////////////////////////////

GCInfo & GCInfo::operator=(const GCInfo &c) {

   if(this == &c) return(*this);

   assign(c);

   return(*this);
}

////////////////////////////////////////////////////////////////////////

int GCInfo::operator==(const GCInfo &c) {
   int match = 0;

   if(code     == c.code &&
      lvl_type == c.lvl_type &&
      lvl_1    == c.lvl_1 &&
      lvl_2    == c.lvl_2 &&
      vflag    == c.vflag) match = 1;

   return(match);
}


////////////////////////////////////////////////////////////////////////

void GCInfo::init_from_scratch() {

   clear();

   return;
}

////////////////////////////////////////////////////////////////////////

void GCInfo::clear() {

   abbr_str.clear();
   lvl_str.clear();
   info_str.clear();

   code     = 0;
   lvl_type = NoLevel;
   lvl_1    = 0;
   lvl_2    = 0;
   vflag    = 0;
   pflag    = 0;

   return;
}

////////////////////////////////////////////////////////////////////////

void GCInfo::assign(const GCInfo &c) {

   clear();

   abbr_str = c.abbr_str;
   lvl_str  = c.lvl_str;
   info_str = c.info_str;

   code     = c.code;

   lvl_type = c.lvl_type;
   lvl_1    = c.lvl_1;
   lvl_2    = c.lvl_2;
   vflag    = c.vflag;
   pflag    = c.pflag;

   return;
}

////////////////////////////////////////////////////////////////////////

void GCInfo::set_gcinfo(const char *c, int ptv) {
   char tmp_str[max_str_len], tmp2_str[max_str_len];
   char *ptr, *ptr2;
   int j;

   // Initialize
   clear();

   // Initialize the temp string
   strcpy(tmp_str, c);

   // Retreive the GRIB code value
   if((ptr = strtok(tmp_str, "/")) == NULL) {
      cerr << "\n\nERROR: GCInfo::set_gcinfo() -> "
           << "bad GRIB code specified \""
           << c << "\".\n\n" << flush;
      exit(1);
   }

   // Store the code value
   code = str_to_grib_code(ptr);

   // Retrieve the level value
   if((ptr = strtok(NULL, "/")) == NULL) {
      cerr << "\n\nERROR: GCInfo::set_gcinfo() -> "
           << "each GRIB code specified must be followed by an "
           << "accumulation, level, or presssure level indicator \""
           << c << "\".\n\n" << flush;
      exit(1);
   }

   // Check the level indicator type
   if(*ptr != 'A' && *ptr != 'Z' &&
      *ptr != 'P' && *ptr != 'R' &&
      *ptr != 'L') {
      cerr << "\n\nERROR: GCInfo::set_gcinfo() -> "
           << "each GRIB code specified (" << c
           << ") must be followed by level information "
           << "that begins with:\n"
           << "\t\'A\' for an accumulation interval\n"
           << "\t\'Z\' for a vertical level\n"
           << "\t\'P\' for a pressure level\n"
           << "\t\'R\' for a record number\n"
           << "\t\'L\' for a generic level\n\n"
           << flush;
      exit(1);
   }

   // Set the level type
   if(      *ptr == 'A') lvl_type = AccumLevel;
   else if (*ptr == 'Z') lvl_type = VertLevel;
   else if (*ptr == 'P') lvl_type = PresLevel;
   else if (*ptr == 'R') lvl_type = RecNumber;
   else if (*ptr == 'L') lvl_type = NoLevel;
   else                  lvl_type = NoLevel;

   // Store the level string
   set_lvl_str(ptr);

   // Advance the pointer past the 'A', 'Z', 'P', 'R', or 'L'
   ptr++;
   lvl_1 = atoi(ptr);

   // Look for a '-' and a second level indicator
   ptr2 = strchr(ptr, '-');
   if(ptr2 != NULL) {
      lvl_2 = atoi(++ptr2);
   }
   else {
      lvl_2 = lvl_1;
   }

   if(lvl_type != PresLevel && lvl_1 != lvl_2) {
      cerr << "\n\nERROR: GCInfo::set_gcinfo() -> "
           << "ranges of values are only supported for pressure "
           << "levels \"" << c << "\".\n\n" << flush;
      exit(1);
   }

   // For pressure levels, check the order of lvl_1 and lvl_2
   if(lvl_type == PresLevel) {

      // If the levels are the same, reset the lvl_str
      if(lvl_1 == lvl_2) {
         sprintf(tmp2_str, "P%i", lvl_1);
         set_lvl_str(tmp2_str);
      }
      // Switch lvl_1 and lvl_2
      else if(lvl_1 > lvl_2) {
         j = lvl_1;
         lvl_1 = lvl_2;
         lvl_2 = j;
      }
      // Reset the lvl_str to be high - low
      else {
         sprintf(tmp2_str, "P%i-%i", lvl_2, lvl_1);
         set_lvl_str(tmp2_str);
      }
   }

   // Check for "/PROB" to indicate a probability forecast
   if((ptr = strtok(NULL, "/")) != NULL) {

      if(strncasecmp(ptr, "PROB", strlen("PROB")) == 0) pflag = 1;
      else {
         cout << "WARNING: GCInfo::set_gcinfo() -> "
              << "unrecognized flag value \"" << ptr
              << "\" for GRIB code \"" << c << "\".\n" << flush;
      }
   }

   // Get the GRIB code abbreviation string
   get_grib_code_abbr(code, ptv, tmp_str);

   // For a non-zero accumulation interval, append _HH to the
   // abbreviation string
   if(lvl_type == AccumLevel && lvl_1 > 0) {
      sprintf(tmp2_str, "%s_%.2i", tmp_str, lvl_1);
      strcpy(tmp_str, tmp2_str);
   }

   // Set the abbr_str
   set_abbr_str(tmp_str);

   // Set the info_str
   sprintf(tmp_str, "%s/%s", abbr_str.text(), lvl_str.text());
   set_info_str(tmp_str);

   return;
}

////////////////////////////////////////////////////////////////////////

void GCInfo::set_abbr_str(const char *c) {

   abbr_str.clear();

   abbr_str = c;

   return;
}

////////////////////////////////////////////////////////////////////////

void GCInfo::set_lvl_str(const char *c) {

   lvl_str.clear();

   lvl_str = c;

   return;
}

////////////////////////////////////////////////////////////////////////

void GCInfo::set_info_str(const char *c) {

   info_str.clear();

   info_str = c;

   return;
}

////////////////////////////////////////////////////////////////////////
//
// Code for class PairData
//
////////////////////////////////////////////////////////////////////////

PairData::PairData() {
   init_from_scratch();
}

////////////////////////////////////////////////////////////////////////

PairData::~PairData() {
   clear();
}

////////////////////////////////////////////////////////////////////////

PairData::PairData(const PairData &pd) {

   init_from_scratch();

   assign(pd);
}

////////////////////////////////////////////////////////////////////////

PairData & PairData::operator=(const PairData &pd) {

   if(this == &pd) return(*this);

   assign(pd);

   return(*this);
}

////////////////////////////////////////////////////////////////////////

void PairData::init_from_scratch() {

   mask_wd_ptr = (WrfData *) 0;

   clear();

   return;
}

////////////////////////////////////////////////////////////////////////

void PairData::clear() {

   msg_typ.clear();
   mask_name.clear();

   mask_wd_ptr = (WrfData *) 0;  // Not allocated

   interp_mthd = im_na;
   interp_wdth = bad_data_int;

   f_na.clear();
   c_na.clear();
   o_na.clear();

   lat_na.clear();
   lon_na.clear();
   lvl_na.clear();
   elv_na.clear();

   n_pair  = 0;

   return;
}

////////////////////////////////////////////////////////////////////////

void PairData::assign(const PairData &pd) {
   int i;

   clear();

   set_mask_name(pd.mask_name);
   set_mask_wd_ptr(pd.mask_wd_ptr);

   set_msg_typ(pd.msg_typ);

   set_interp_mthd(pd.interp_mthd);
   set_interp_wdth(pd.interp_wdth);

   for(i=0; i<pd.n_pair; i++) {
      add_pair(pd.lat_na[i], pd.lon_na[i],
               pd.lvl_na[i], pd.elv_na[i],
               pd.f_na[i], pd.c_na[i], pd.o_na[i]);
   }

   return;
}

////////////////////////////////////////////////////////////////////////

void PairData::set_mask_name(const char *c) {

   mask_name = c;

   return;
}

////////////////////////////////////////////////////////////////////////

void PairData::set_mask_wd_ptr(WrfData *wd_ptr) {

   mask_wd_ptr = wd_ptr;

   return;
}

////////////////////////////////////////////////////////////////////////

void PairData::set_msg_typ(const char *c) {

   msg_typ = c;

   return;
}

////////////////////////////////////////////////////////////////////////

void PairData::set_interp_mthd(const char *str) {

   interp_mthd = string_to_interpmthd(str);

   return;
}

////////////////////////////////////////////////////////////////////////

void PairData::set_interp_mthd(InterpMthd m) {

   interp_mthd = m;

   return;
}

////////////////////////////////////////////////////////////////////////

void PairData::set_interp_wdth(int n) {

   interp_wdth = n;

   return;
}

////////////////////////////////////////////////////////////////////////

void PairData::add_pair(double lat, double lon,
                        double lvl, double elv,
                        double f,   double c,   double o) {
   lat_na.add(lat);
   lon_na.add(lon);
   lvl_na.add(lvl);
   elv_na.add(elv);

   f_na.add(f);
   c_na.add(c);
   o_na.add(o);

   // Increment the number of pairs
   n_pair += 1;

   return;
}

////////////////////////////////////////////////////////////////////////
//
// Code for class GCPairData
//
////////////////////////////////////////////////////////////////////////

GCPairData::GCPairData() {
   init_from_scratch();
}

////////////////////////////////////////////////////////////////////////

GCPairData::~GCPairData() {
   clear();
}

////////////////////////////////////////////////////////////////////////

GCPairData::GCPairData(const GCPairData &gc_pd) {

   init_from_scratch();

   assign(gc_pd);
}

////////////////////////////////////////////////////////////////////////

GCPairData & GCPairData::operator=(const GCPairData &gc_pd) {

   if(this == &gc_pd) return(*this);

   assign(gc_pd);

   return(*this);
}

////////////////////////////////////////////////////////////////////////

void GCPairData::init_from_scratch() {

   fcst_wd_ptr  = (WrfData **) 0;
   climo_wd_ptr = (WrfData **) 0;
   pd           = (PairData ***) 0;

   clear();

   return;
}

////////////////////////////////////////////////////////////////////////

void GCPairData::clear() {
   int i, j, k;

   fcst_gci.clear();
   obs_gci.clear();

   beg_ut        = (unixtime) 0;
   end_ut        = (unixtime) 0;

   interp_thresh = 0;
   n_fcst        = 0;
   n_climo       = 0;
   n_msg_typ     = 0;
   n_mask        = 0;
   n_interp      = 0;

   fcst_prs_lvl.clear();
   climo_prs_lvl.clear();

   for(i=0; i<n_fcst; i++)  fcst_wd_ptr[i]  = (WrfData *) 0;
   for(i=0; i<n_climo; i++) climo_wd_ptr[i] = (WrfData *) 0;

   fcst_wd_ptr  = (WrfData **) 0;
   climo_wd_ptr = (WrfData **) 0;

   for(i=0; i<n_msg_typ; i++) {
      for(j=0; j<n_mask; j++) {
         for(k=0; k<n_interp; k++) {
            pd[i][j][k].clear();
         }
      }
   }

   return;
}

////////////////////////////////////////////////////////////////////////

void GCPairData::assign(const GCPairData &gc_pd) {
   int i, j, k;

   clear();

   set_fcst_gci(gc_pd.fcst_gci);
   set_obs_gci(gc_pd.obs_gci);

   beg_ut  = gc_pd.beg_ut;
   end_ut  = gc_pd.end_ut;

   interp_thresh = gc_pd.interp_thresh;

   set_n_fcst(gc_pd.n_fcst);
   for(i=0; i<gc_pd.n_fcst; i++) {
      set_fcst_prs_lvl(i, fcst_prs_lvl[i]);
      set_fcst_wd_ptr(i, fcst_wd_ptr[i]);
   }

   set_n_climo(gc_pd.n_climo);
   for(i=0; i<gc_pd.n_climo; i++) {
      set_climo_prs_lvl(i, climo_prs_lvl[i]);
      set_climo_wd_ptr(i, climo_wd_ptr[i]);
   }

   set_pd_size(gc_pd.n_msg_typ, gc_pd.n_mask, gc_pd.n_interp);

   for(i=0; i<gc_pd.n_msg_typ; i++) {
      for(j=0; j<gc_pd.n_mask; j++) {
         for(k=0; k<gc_pd.n_interp; k++) {

            pd[i][j][k] = gc_pd.pd[i][j][k];
         }
      }
   }

   return;
}

////////////////////////////////////////////////////////////////////////

void GCPairData::set_fcst_gci(const GCInfo &gci) {

   fcst_gci = gci;

   return;
}

////////////////////////////////////////////////////////////////////////

void GCPairData::set_obs_gci(const GCInfo &gci) {

   obs_gci = gci;

   return;
}

////////////////////////////////////////////////////////////////////////

void GCPairData::set_interp_thresh(double t) {

   interp_thresh = t;

   return;
}

////////////////////////////////////////////////////////////////////////

void GCPairData::set_n_fcst(int n) {
   int i;

   n_fcst = n;

   fcst_wd_ptr  = new WrfData * [n_fcst];

   for(i=0; i<n_fcst; i++) {
      fcst_prs_lvl.add(bad_data_double);
      fcst_wd_ptr[i] = (WrfData *) 0;
   }

   return;
}

////////////////////////////////////////////////////////////////////////

void GCPairData::set_fcst_prs_lvl(int i, double lvl) {

   fcst_prs_lvl.set(i, lvl);

   return;
}

////////////////////////////////////////////////////////////////////////

void GCPairData::set_fcst_wd_ptr(int i, WrfData *wd_ptr) {

   fcst_wd_ptr[i] = wd_ptr;

   return;
}

////////////////////////////////////////////////////////////////////////

void GCPairData::set_n_climo(int n) {
   int i;

   n_climo = n;

   climo_wd_ptr  = new WrfData * [n_climo];

   for(i=0; i<n_climo; i++) {
      climo_prs_lvl.add(bad_data_double);
      climo_wd_ptr[i] = (WrfData *) 0;
   }

   return;
}

////////////////////////////////////////////////////////////////////////

void GCPairData::set_climo_prs_lvl(int i, double lvl) {

   climo_prs_lvl.set(i, lvl);

   return;
}

////////////////////////////////////////////////////////////////////////

void GCPairData::set_climo_wd_ptr(int i, WrfData *wd_ptr) {

   climo_wd_ptr[i] = wd_ptr;

   return;
}

////////////////////////////////////////////////////////////////////////

void GCPairData::set_beg_ut(const unixtime ut) {

   beg_ut = ut;

   return;
}

////////////////////////////////////////////////////////////////////////

void GCPairData::set_end_ut(const unixtime ut) {

   end_ut = ut;

   return;
}

////////////////////////////////////////////////////////////////////////

void GCPairData::set_pd_size(int types, int masks, int interps) {
   int i, j;

   // Store the dimensions for the PairData array
   n_msg_typ = types;
   n_mask    = masks;
   n_interp  = interps;

   // Allocate space for the PairData array
   pd = new PairData ** [n_msg_typ];

   for(i=0; i<n_msg_typ; i++) {
      pd[i] = new PairData * [n_mask];

      for(j=0; j<n_mask; j++) {
         pd[i][j] = new PairData [n_interp];
      }
   }

   return;
}

////////////////////////////////////////////////////////////////////////

void GCPairData::set_msg_typ(int i_msg_typ, const char *name) {
   int i, j;

   for(i=0; i<n_mask; i++) {
      for(j=0; j<n_interp; j++) {
         pd[i_msg_typ][i][j].set_msg_typ(name);
      }
   }

   return;
}

////////////////////////////////////////////////////////////////////////

void GCPairData::set_mask_wd(int i_mask, const char *name,
                             WrfData *wd_ptr) {
   int i, j;

   for(i=0; i<n_msg_typ; i++) {
      for(j=0; j<n_interp; j++) {
         pd[i][i_mask][j].set_mask_name(name);
         pd[i][i_mask][j].set_mask_wd_ptr(wd_ptr);
      }
   }

   return;
}

////////////////////////////////////////////////////////////////////////

void GCPairData::set_interp(int i_interp, const char *interp_mthd_str,
                            int wdth) {
   int i, j;

   for(i=0; i<n_msg_typ; i++) {
      for(j=0; j<n_mask; j++) {
         pd[i][j][i_interp].set_interp_mthd(interp_mthd_str);
         pd[i][j][i_interp].set_interp_wdth(wdth);
      }
   }

   return;
}

////////////////////////////////////////////////////////////////////////

void GCPairData::set_interp(int i_interp, InterpMthd mthd, int wdth) {
   int i, j;

   for(i=0; i<n_msg_typ; i++) {
      for(j=0; j<n_mask; j++) {
         pd[i][j][i_interp].set_interp_mthd(mthd);
         pd[i][j][i_interp].set_interp_wdth(wdth);
      }
   }

   return;
}

////////////////////////////////////////////////////////////////////////

void GCPairData::add_obs(float *hdr_arr,     char *hdr_typ_str,
                         char  *hdr_sid_str, unixtime hdr_ut,
                         float *obs_arr,     Grid &gr) {
   int i, j, k, x, y;
   double hdr_lat, hdr_lon;
   double obs_x, obs_y, obs_lvl, obs_elv;
   double fcst_v, climo_v, obs_v;
   int fcst_lvl_below, fcst_lvl_above;
   int climo_lvl_below, climo_lvl_above;

   // Check whether the GRIB code for the observation matches
   // the specified code
   if(obs_gci.code != nint(obs_arr[1])) return;

   // Check whether the observation time falls within the valid time
   // window
   if(hdr_ut < beg_ut || hdr_ut > end_ut) return;

   hdr_lat = hdr_arr[0];
   hdr_lon = hdr_arr[1];

   obs_lvl = obs_arr[2];
   obs_elv = obs_arr[3];
   obs_v   = obs_arr[4];

   // Check whether the observation value contains valid data
   if(is_bad_data(obs_v)) return;

   // Convert the lat/lon value to x/y
   gr.latlon_to_xy(hdr_lat, -1.0*hdr_lon, obs_x, obs_y);
   x = nint(obs_x);
   y = nint(obs_y);

   // Check if the observation's lat/lon is on the grid
   if(x < 0 || x >= gr.nx() ||
      y < 0 || y >= gr.ny()) return;

   // If looking for observations on pressure levels, check whether
   // this observation falls within the specified range
   if(obs_gci.lvl_type == PresLevel &&
      (obs_lvl < obs_gci.lvl_1 || obs_lvl > obs_gci.lvl_2)
     ) return;

   // If looking for observations at a vertical level check if the
   // header type is APDSFC or SFCSHP
   if(obs_gci.lvl_type == VertLevel &&
      strstr(onlysf_msg_typ_str, hdr_typ_str) == NULL) return;

   // FUTURE WORK: add the ability to match observations at vertical
   // levels other than the surface

   // If looking for observations with an accumulation interval,
   // check whether this accumulation interval matches
   if(obs_gci.lvl_type == AccumLevel &&
      !is_eq(obs_lvl, obs_gci.lvl_1)) return;

   // Find the forecast and climatology pressure levels above and below
   // the observation point
   if(fcst_gci.lvl_type == PresLevel) {
      find_prs_lvl(1, obs_lvl, fcst_lvl_below, fcst_lvl_above);
      find_prs_lvl(0, obs_lvl, climo_lvl_below, climo_lvl_above);
   }
   // Set the forecast and climatology levels above and below
   // the observation point to zero since we're looking at a single
   // level
   else {
      fcst_lvl_below  = fcst_lvl_above  = 0;
      climo_lvl_below = climo_lvl_above = 0;
   }

   // Look through all of the PairData objects to see if the
   // observation should be added

   // Check the message types
   for(i=0; i<n_msg_typ; i++) {

      //
      // Check for a matching PrepBufr message type
      //

      // Handle ANYAIR
      if(strcmp(anyair_str, pd[i][0][0].msg_typ) == 0) {
         if(strstr(anyair_msg_typ_str, hdr_typ_str) == NULL ) continue;
      }

      // Handle ANYSFC
      else if(strcmp(anysfc_str, pd[i][0][0].msg_typ) == 0) {
         if(strstr(anysfc_msg_typ_str, hdr_typ_str) == NULL) continue;
      }

      // Handle ONLYSF
      else if(strcmp(onlysf_str, pd[i][0][0].msg_typ) == 0) {
         if(strstr(onlysf_msg_typ_str, hdr_typ_str) == NULL) continue;
      }

      // Handle all other message types
      else {
         if(strcmp(hdr_typ_str, pd[i][0][0].msg_typ) != 0) continue;
      }

      // Check the masking areas and points
      for(j=0; j<n_mask; j++) {

         // Check for the obs falling within the masking region
         if(pd[i][j][0].mask_wd_ptr != (WrfData *) 0) {
            if(!pd[i][j][0].mask_wd_ptr->s_is_on(x, y)) continue;
         }
         // Otherwise, check for the obs Station ID matching the
         // masking SID
         else {
            if(strcmp(hdr_sid_str, pd[i][j][0].mask_name) != 0)
               continue;
         }

         // Compute the interpolated values
         for(k=0; k<n_interp; k++) {

            // Compute the interpolated forecast value
            fcst_v = compute_interp(1, obs_x, obs_y, k,
                        obs_lvl, fcst_lvl_below, fcst_lvl_above);

            if(is_bad_data(fcst_v)) continue;

            // Compute the interpolated climotological value
            climo_v = compute_interp(0, obs_x, obs_y, k,
                         obs_lvl, climo_lvl_below, climo_lvl_above);

            // Add the forecast, climatological, and observation data
            pd[i][j][k].add_pair(hdr_lat, hdr_lon,
                                 obs_lvl, obs_elv,
                                 fcst_v, climo_v, obs_v);

         } // end for k
      } // end for j
   } // end for i

   return;
}

////////////////////////////////////////////////////////////////////////

void GCPairData::find_prs_lvl(int fcst_flag, double prs,
                              int &i_below, int &i_above) {
   int i, n;
   NumArray *prs_lvl;
   double dist, dist_below, dist_above;

   // Check for the forecast or climo fields
   if(fcst_flag) { n = n_fcst;  prs_lvl = &fcst_prs_lvl;  }
   else          { n = n_climo; prs_lvl = &climo_prs_lvl; }

   if(n==0) {
      i_below = i_above = bad_data_int;
      return;
   }

   // Find the closest pressure levels above and below the observation
   dist_below = dist_above = 1.0e30;
   for(i=0; i<n; i++) {

      dist = prs - (*prs_lvl)[i];

      // Check for the closest level below.
      // Levels below contain higher values of pressure.
      if(dist <= 0 && abs((long double) dist) < dist_below) {
         dist_below = abs((long double) dist);
         i_below = i;
      }

      // Check for the closest level above.
      // Levels above contain lower values of pressure.
      if(dist >= 0 && abs((long double) dist) < dist_above) {
         dist_above = abs((long double) dist);
         i_above = i;
      }
   }

   // Check if the observation is above the forecast range
   if     (is_eq(dist_below, 1.0e30) && !is_eq(dist_above, 1.0e30)) {

      // Set the index below to the index above and perform
      // no vertical interpolation
      i_below = i_above;
   }
   // Check if the observation is below the forecast range
   else if(!is_eq(dist_below, 1.0e30) && is_eq(dist_above, 1.0e30)) {

      // Set the index above to the index below and perform
      // no vertical interpolation
      i_above = i_below;
   }
   // Check if an error occurred
   else if(is_eq(dist_below, 1.0e30) && is_eq(dist_above, 1.0e30)) {

      cerr << "\n\nERROR: GCPairData::find_prs_lvl() -> "
           << "could not find a level above and/or below the "
           << "observation pressure level of " << prs << " hp\n\n"
           << flush;
      exit(1);
   }

   return;
}

////////////////////////////////////////////////////////////////////////

int GCPairData::get_n_pair() {
   int n, i, j, k;

   for(i=0, n=0; i<n_msg_typ; i++) {
      for(j=0; j<n_mask; j++) {
         for(k=0; k<n_interp; k++) {

            n += pd[i][j][k].n_pair;
         }
      }
   }

   return(n);
}

////////////////////////////////////////////////////////////////////////

double GCPairData::compute_interp(int fcst_flag,
                                  double obs_x, double obs_y,
                                  int i_interp, double prs,
                                  int i_below, int i_above) {
   int n;
   NumArray *prs_lvl;
   WrfData **wd_ptr;
   double v, v_below, v_above, t;

   // Check for the forecast or climo fields
   if(fcst_flag) {
      n       = n_fcst;
      prs_lvl = &fcst_prs_lvl;
      wd_ptr  = fcst_wd_ptr;
   }
   else {
      n       = n_climo;
      prs_lvl = &climo_prs_lvl;
      wd_ptr  = climo_wd_ptr;
   }

   if(n==0) return(bad_data_double);

   v_below = compute_horz_interp(wd_ptr[i_below], obs_x, obs_y,
                                 pd[0][0][i_interp].interp_mthd,
                                 pd[0][0][i_interp].interp_wdth);

   if(i_below == i_above) {
      v = v_below;
   }
   else {
      v_above = compute_horz_interp(wd_ptr[i_above], obs_x, obs_y,
                                    pd[0][0][i_interp].interp_mthd,
                                    pd[0][0][i_interp].interp_wdth);

      // If verifying specific humidity, do vertical interpolation in
      // the natural log of q
      if(fcst_gci.code == spfh_grib_code &&
         obs_gci.code  == spfh_grib_code) {
         t = compute_vert_interp(log(v_below), (*prs_lvl)[i_below],
                                 log(v_above), (*prs_lvl)[i_above],
                                 prs);
         v = exp(t);
      }
      // Vertically interpolate to the observation pressure level
      else {
         v = compute_vert_interp(v_below, (*prs_lvl)[i_below],
                                 v_above, (*prs_lvl)[i_above],
                                 prs);
      }
   }

   return(v);
}

////////////////////////////////////////////////////////////////////////

double GCPairData::compute_horz_interp(WrfData *wd_ptr,
                                       double obs_x, double obs_y,
                                       int mthd, int wdth) {
   double v;
   int x_ll, y_ll;

   // The neighborhood width is odd, find the lower-left corner of
   // the neighborhood
   if(wdth%2 == 1) {
      x_ll = nint(obs_x) - (wdth - 1)/2;
      y_ll = nint(obs_y) - (wdth - 1)/2;
   }
   // The neighborhood width is even, find the lower-left corner of
   // the neighborhood
   else {
      x_ll = nint(floor(obs_x) - (wdth/2 - 1));
      y_ll = nint(floor(obs_y) - (wdth/2 - 1));
   }

   // Compute the interpolated value for the fields above and below
   switch(mthd) {

      case(im_min):     // Minimum
         v = interp_min(*wd_ptr, x_ll, y_ll, wdth, interp_thresh);
         break;

      case(im_max):     // Maximum
         v = interp_max(*wd_ptr, x_ll, y_ll, wdth, interp_thresh);
         break;

      case(im_median):  // Median
         v = interp_median(*wd_ptr, x_ll, y_ll, wdth, interp_thresh);
         break;

      case(im_uw_mean): // Unweighted Mean
         v = interp_uw_mean(*wd_ptr, x_ll, y_ll, wdth, interp_thresh);
         break;

      case(im_dw_mean): // Distance-Weighted Mean
         v = interp_dw_mean(*wd_ptr, x_ll, y_ll, wdth, obs_x, obs_y,
                            dw_mean_pow, interp_thresh);
         break;

      case(im_ls_fit):  // Least-squares fit
         v = interp_ls_fit(*wd_ptr, x_ll, y_ll, wdth, obs_x, obs_y,
                           interp_thresh);
         break;

      default:
         cerr << "\n\nERROR: GCPairData::compute_horz_interp() -> "
              << "unexpected interpolation method encountered: "
              << mthd << "\n\n" << flush;
         exit(1);
         break;
   }

   return(v);
}

////////////////////////////////////////////////////////////////////////
//
// Vertically interpolate between values "v1" and "v2" at pressure
// levels "prs1" and "prs2" to pressure level "to_prs".
//
////////////////////////////////////////////////////////////////////////

double GCPairData::compute_vert_interp(double v1, double prs1,
                                       double v2, double prs2,
                                       double to_prs) {
   double v_interp;

   if(prs1 <= 0.0 || prs2 <= 0.0 || to_prs <= 0.0) {
      cerr << "\n\nERROR: GCPairData::compute_vert_interp() -> "
           << "pressure shouldn't be <= zero!\n\n" << flush;
      exit(1);
   }

   v_interp = v1 + ((v2-v1)*log(prs1/to_prs)/log(prs1/prs2));

   return(v_interp);
}

////////////////////////////////////////////////////////////////////////
//
// Code for class CIInfo
//
////////////////////////////////////////////////////////////////////////

CIInfo::CIInfo() {
   init_from_scratch();
}

////////////////////////////////////////////////////////////////////////

CIInfo::~CIInfo() {
   clear();
}

////////////////////////////////////////////////////////////////////////

CIInfo::CIInfo(const CIInfo &c) {

   init_from_scratch();

   assign(c);
}

////////////////////////////////////////////////////////////////////////

CIInfo & CIInfo::operator=(const CIInfo &c) {

   if(this == &c) return(*this);

   assign(c);

   return(*this);
}

////////////////////////////////////////////////////////////////////////

void CIInfo::init_from_scratch() {

   v_ncl = (double *) 0;
   v_ncu = (double *) 0;

   v_bcl = (double *) 0;
   v_bcu = (double *) 0;

   clear();

   return;
}

////////////////////////////////////////////////////////////////////////

void CIInfo::clear() {

   n = 0;
   v = bad_data_double;

   if(v_ncl) { delete [] v_ncl; v_ncl = (double *) 0; }
   if(v_ncu) { delete [] v_ncu; v_ncu = (double *) 0; }

   if(v_bcl) { delete [] v_bcl; v_bcl = (double *) 0; }
   if(v_bcu) { delete [] v_bcu; v_bcu = (double *) 0; }

   return;
}

////////////////////////////////////////////////////////////////////////

void CIInfo::set_bad_data() {
   int i;

   v = bad_data_double;

   for(i=0; i<n; i++) {
      v_ncl[i] = v_ncu[i] = bad_data_double;
      v_bcl[i] = v_bcu[i] = bad_data_double;
   }

   return;
}

////////////////////////////////////////////////////////////////////////

void CIInfo::assign(const CIInfo &c) {
   int i;

   clear();

   allocate_n_alpha(c.n);

   v = c.v;

   for(i=0; i<c.n; i++) {
      v_ncl[i] = c.v_ncl[i];
      v_ncu[i] = c.v_ncu[i];
      v_bcl[i] = c.v_bcl[i];
      v_bcu[i] = c.v_bcu[i];
   }

   return;
}

////////////////////////////////////////////////////////////////////////

void CIInfo::allocate_n_alpha(int i) {
   int j;

   n = i;

   if(n > 0) {
      v_ncl = new double [n];
      v_ncu = new double [n];
      v_bcl = new double [n];
      v_bcu = new double [n];

      if(!v_ncl || !v_ncu || !v_bcl || !v_bcu) {
         cerr << "\n\nERROR: CIInfo::allocate_n_alpha() -> "
              << "Memory allocation error!\n\n" << flush;
         exit(1);
      }
   }

   // Initialize the values
   for(j=0; j<n; j++) {
      v_ncl[j] = bad_data_double;
      v_ncu[j] = bad_data_double;
      v_bcl[j] = bad_data_double;
      v_bcu[j] = bad_data_double;
   }

   return;
}

////////////////////////////////////////////////////////////////////////
//
// Code for class CTSInfo
//
////////////////////////////////////////////////////////////////////////

CTSInfo::CTSInfo() {
   init_from_scratch();
}

////////////////////////////////////////////////////////////////////////

CTSInfo::~CTSInfo() {
   clear();
}

////////////////////////////////////////////////////////////////////////

CTSInfo::CTSInfo(const CTSInfo &c) {

   init_from_scratch();

   assign(c);
}

////////////////////////////////////////////////////////////////////////

CTSInfo & CTSInfo::operator=(const CTSInfo &c) {

   if(this == &c) return(*this);

   assign(c);

   return(*this);
}

////////////////////////////////////////////////////////////////////////

void CTSInfo::init_from_scratch() {

   alpha = (double *) 0;

   clear();

   return;
}

////////////////////////////////////////////////////////////////////////

void CTSInfo::clear() {

   n_alpha = 0;
   if(alpha) { delete [] alpha; alpha = (double *) 0; }

   cts.zero_out();
   cts_fcst_thresh.clear();
   cts_obs_thresh.clear();

   baser.clear();
   fmean.clear();
   acc.clear();
   fbias.clear();
   pody.clear();
   podn.clear();
   pofd.clear();
   far.clear();
   csi.clear();
   gss.clear();
   hk.clear();
   hss.clear();
   odds.clear();

   return;
}

////////////////////////////////////////////////////////////////////////

void CTSInfo::assign(const CTSInfo &c) {
   int i;

   clear();

   cts = c.cts;
   cts_fcst_thresh = c.cts_fcst_thresh;
   cts_obs_thresh  = c.cts_obs_thresh;

   allocate_n_alpha(c.n_alpha);
   for(i=0; i<c.n_alpha; i++) { alpha[i] = c.alpha[i]; }

   baser = c.baser;
   fmean = c.fmean;
   acc = c.acc;
   fbias = c.fbias;
   pody = c.pody;
   podn = c.podn;
   pofd = c.pofd;
   far = c.far;
   csi = c.csi;
   gss = c.gss;
   hk = c.hk;
   hss = c.hss;
   odds = c.odds;

   return;
}

////////////////////////////////////////////////////////////////////////

void CTSInfo::allocate_n_alpha(int i) {

   n_alpha = i;

   if(n_alpha > 0) {

      alpha = new double [n_alpha];

      if(!alpha) {
         cerr << "\n\nERROR: CTSInfo::allocate_n() -> "
              << "Memory allocation error!\n\n" << flush;
         exit(1);
      }

      baser.allocate_n_alpha(n_alpha);
      fmean.allocate_n_alpha(n_alpha);
      acc.allocate_n_alpha(n_alpha);
      fbias.allocate_n_alpha(n_alpha);
      pody.allocate_n_alpha(n_alpha);
      podn.allocate_n_alpha(n_alpha);
      pofd.allocate_n_alpha(n_alpha);
      far.allocate_n_alpha(n_alpha);
      csi.allocate_n_alpha(n_alpha);
      gss.allocate_n_alpha(n_alpha);
      hk.allocate_n_alpha(n_alpha);
      hss.allocate_n_alpha(n_alpha);
      odds.allocate_n_alpha(n_alpha);
   }

   return;
}

////////////////////////////////////////////////////////////////////////

void CTSInfo::compute_stats() {

   baser.v = cts.oy_tp();
   fmean.v = cts.fy_tp();
   acc.v   = cts.accuracy();
   fbias.v = cts.fbias();
   pody.v  = cts.pod_yes();
   podn.v  = cts.pod_no();
   pofd.v  = cts.pofd();
   far.v   = cts.far();
   csi.v   = cts.csi();
   gss.v   = cts.gss();
   hk.v    = cts.hk();
   hss.v   = cts.hss();
   odds.v  = cts.odds();

   return;
}

////////////////////////////////////////////////////////////////////////

void CTSInfo::compute_ci() {
   int i;

   //
   // Compute confidence intervals for each alpha value specified
   //
   for(i=0; i<n_alpha; i++) {

      //
      // Compute confidence intervals for the scores based on
      // proportions
      //
      compute_proportion_ci(baser.v, cts.n(), alpha[i],
                            baser.v_ncl[i], baser.v_ncu[i]);
      compute_proportion_ci(fmean.v, cts.n(), alpha[i],
                            fmean.v_ncl[i], fmean.v_ncu[i]);
      compute_proportion_ci(acc.v, cts.n(), alpha[i],
                            acc.v_ncl[i], acc.v_ncu[i]);
      compute_proportion_ci(pody.v, cts.n(), alpha[i],
                            pody.v_ncl[i], pody.v_ncu[i]);
      compute_proportion_ci(podn.v, cts.n(), alpha[i],
                            podn.v_ncl[i], podn.v_ncu[i]);
      compute_proportion_ci(pofd.v, cts.n(), alpha[i],
                            pofd.v_ncl[i], pofd.v_ncu[i]);
      compute_proportion_ci(far.v, cts.n(), alpha[i],
                            far.v_ncl[i], far.v_ncu[i]);
      compute_proportion_ci(csi.v, cts.n(), alpha[i],
                            csi.v_ncl[i], csi.v_ncu[i]);

      //
      // Compute a confidence interval for Hanssen and Kuipers discriminant
      //
      compute_hk_ci(hk.v, alpha[i],
                    cts.fy_oy(), cts.fy_on(), cts.fn_oy(), cts.fn_on(),
                    hk.v_ncl[i], hk.v_ncu[i]);

      //
      // Compute a confidence interval for the odds ratio
      //
      compute_woolf_ci(odds.v, alpha[i],
                       cts.fy_oy(), cts.fy_on(), cts.fn_oy(), cts.fn_on(),
                       odds.v_ncl[i], odds.v_ncu[i]);
   } // end for i

   return;
}

////////////////////////////////////////////////////////////////////////
//
// Code for class CNTInfo
//
////////////////////////////////////////////////////////////////////////

CNTInfo::CNTInfo() {
   init_from_scratch();
}

////////////////////////////////////////////////////////////////////////

CNTInfo::~CNTInfo() {
   clear();
}

////////////////////////////////////////////////////////////////////////

CNTInfo::CNTInfo(const CNTInfo &c) {

   init_from_scratch();

   assign(c);
}

////////////////////////////////////////////////////////////////////////

CNTInfo & CNTInfo::operator=(const CNTInfo &c) {

   if(this == &c) return(*this);

   assign(c);

   return(*this);
}

////////////////////////////////////////////////////////////////////////

void CNTInfo::init_from_scratch() {

   alpha = (double *) 0;

   clear();

   return;
}

////////////////////////////////////////////////////////////////////////

void CNTInfo::clear() {

   n = 0;
   n_alpha = 0;

   if(alpha) { delete [] alpha; alpha = (double *) 0; }

   fbar.clear();
   fstdev.clear();
   obar.clear();
   ostdev.clear();
   pr_corr.clear();
   sp_corr.clear();
   kt_corr.clear();
   me.clear();
   estdev.clear();
   mbias.clear();
   mae.clear();
   mse.clear();
   bcmse.clear();
   rmse.clear();
   e10.clear();
   e25.clear();
   e50.clear();
   e75.clear();
   e90.clear();

   n_ranks = frank_ties = orank_ties = 0;

   return;
}

////////////////////////////////////////////////////////////////////////

void CNTInfo::assign(const CNTInfo &c) {
   int i;

   clear();

   n = c.n;
   allocate_n_alpha(c.n_alpha);
   for(i=0; i<c.n; i++) { alpha[i] = c.alpha[i]; }

   fbar        = c.fbar;
   fstdev      = c.fstdev;
   obar        = c.obar;
   ostdev      = c.ostdev;
   pr_corr     = c.pr_corr;
   sp_corr     = c.sp_corr;
   kt_corr     = c.kt_corr;
   me          = c.me;
   estdev      = c.estdev;
   mbias       = c.mbias;
   mae         = c.mae;
   mse         = c.mse;
   bcmse       = c.bcmse;
   rmse        = c.rmse;
   e10         = c.e10;
   e25         = c.e25;
   e50         = c.e50;
   e75         = c.e75;
   e90         = c.e90;
   n_ranks     = c.n_ranks;
   frank_ties  = c.frank_ties;
   orank_ties  = c.orank_ties;

   return;
}

////////////////////////////////////////////////////////////////////////

void CNTInfo::allocate_n_alpha(int i) {

   n_alpha = i;

   if(n_alpha > 0) {

      alpha = new double [n_alpha];

      if(!alpha) {
         cerr << "\n\nERROR: CNTInfo::allocate_n_alpha() -> "
              << "Memory allocation error!\n\n" << flush;
         exit(1);
      }

      fbar.allocate_n_alpha(n_alpha);
      fstdev.allocate_n_alpha(n_alpha);
      obar.allocate_n_alpha(n_alpha);
      ostdev.allocate_n_alpha(n_alpha);
      pr_corr.allocate_n_alpha(n_alpha);
      sp_corr.allocate_n_alpha(n_alpha);
      kt_corr.allocate_n_alpha(n_alpha);
      me.allocate_n_alpha(n_alpha);
      estdev.allocate_n_alpha(n_alpha);
      mbias.allocate_n_alpha(n_alpha);
      mae.allocate_n_alpha(n_alpha);
      mse.allocate_n_alpha(n_alpha);
      bcmse.allocate_n_alpha(n_alpha);
      rmse.allocate_n_alpha(n_alpha);
      e10.allocate_n_alpha(n_alpha);
      e25.allocate_n_alpha(n_alpha);
      e50.allocate_n_alpha(n_alpha);
      e75.allocate_n_alpha(n_alpha);
      e90.allocate_n_alpha(n_alpha);
   }

   return;
}

////////////////////////////////////////////////////////////////////////

void CNTInfo::compute_ci() {
   int i;
   double cv_normal_l, cv_normal_u;
   double cv_chi2_l, cv_chi2_u;
   double v, cl, cu;

   //
   // Compute the confidence interval for each alpha value specified
   // In computing the confidence intervals, the spatial correlation between
   // adjacent points is ignored, and certain assumptions of normality are
   // made.  The user must decide if the computation method is appropriate
   // for the chosen field.
   //
   for(i=0; i<n_alpha; i++) {

      //
      // Check for the degenerate case
      //
      if(n <= 1) {
         fbar.v_ncl[i]    = fbar.v_ncu[i]    = bad_data_double;
         fstdev.v_ncl[i]  = fstdev.v_ncu[i]  = bad_data_double;
         obar.v_ncl[i]    = obar.v_ncu[i]    = bad_data_double;
         ostdev.v_ncl[i]  = ostdev.v_ncu[i]  = bad_data_double;
         pr_corr.v_ncl[i] = pr_corr.v_ncu[i] = bad_data_double;
         me.v_ncl[i]      = me.v_ncu[i]      = bad_data_double;
         estdev.v_ncl[i]  = estdev.v_ncu[i]  = bad_data_double;

         continue;
      }

      //
      // Compute the critical values for the Normal or Student's-T distribution
      // based on the sample size
      //
      if(n >= large_sample_threshold) {
         cv_normal_l = normal_cdf_inv(alpha[i]/2.0, 0.0, 1.0);
         cv_normal_u = normal_cdf_inv(1.0 - (alpha[i]/2.0), 0.0, 1.0);
      }
      //
      // If the number of samples is less than the large sample threshold,
      // use the T-distribution
      //
      else {
         cv_normal_l = students_t_cdf_inv(alpha[i]/2.0, n-1);
         cv_normal_u = students_t_cdf_inv(1.0 - (alpha[i]/2.0), n-1);
      }

      //
      // Compute the critical values for the Chi Squared distribution
      //
      cv_chi2_l = chi2_cdf_inv(alpha[i]/2.0, n-1);
      cv_chi2_u = chi2_cdf_inv(1.0 - (alpha[i]/2.0), n-1);

      //
      // Compute confidence interval for forecast mean
      //
      fbar.v_ncl[i] = fbar.v + cv_normal_l*fstdev.v/sqrt((double) n);
      fbar.v_ncu[i] = fbar.v + cv_normal_u*fstdev.v/sqrt((double) n);

      //
      // Compute confidence interval for forecast standard deviation,
      // assuming normality of the forecast values
      //
      v = (n-1)*fstdev.v*fstdev.v/cv_chi2_u;
      if(v < 0) fstdev.v_ncl[i] = bad_data_double;
      else      fstdev.v_ncl[i] = sqrt(v);

      v = (n-1)*fstdev.v*fstdev.v/cv_chi2_l;
      if(v < 0) fstdev.v_ncu[i] = bad_data_double;
      else      fstdev.v_ncu[i] = sqrt(v);

      //
      // Compute confidence interval for observation mean
      //
      obar.v_ncl[i] = obar.v + cv_normal_l*ostdev.v/sqrt((double) n);
      obar.v_ncu[i] = obar.v + cv_normal_u*ostdev.v/sqrt((double) n);

      //
      // Compute confidence interval for observation standard deviation
      // assuming normality of the observation values
      //
      v = (n-1)*ostdev.v*ostdev.v/cv_chi2_u;
      if(v < 0) ostdev.v_ncl[i] = bad_data_double;
      else      ostdev.v_ncl[i] = sqrt(v);

      v = (n-1)*ostdev.v*ostdev.v/cv_chi2_l;
      if(v < 0) ostdev.v_ncu[i] = bad_data_double;
      else      ostdev.v_ncu[i] = sqrt(v);

      //
      // Compute confidence interval for the correlation coefficient
      //
      if(is_bad_data(pr_corr.v) || n <= 3) {
         pr_corr.v_ncl[i] = bad_data_double;
         pr_corr.v_ncu[i] = bad_data_double;
      }
      else {
         v = 0.5*log((1 + pr_corr.v)/(1 - pr_corr.v));
         cl = v + cv_normal_l/sqrt((double) (n-3));
         cu = v + cv_normal_u/sqrt((double) (n-3));
         pr_corr.v_ncl[i] = (pow(e, 2*cl) - 1)/(pow(e, 2*cl) + 1);
         pr_corr.v_ncu[i] = (pow(e, 2*cu) - 1)/(pow(e, 2*cu) + 1);
      }

      //
      // Compute confidence interval for mean error
      //
      me.v_ncl[i] = me.v + cv_normal_l*estdev.v/sqrt((double) n);
      me.v_ncu[i] = me.v + cv_normal_u*estdev.v/sqrt((double) n);

      //
      // Compute confidence interval for the error standard deviation
      //
      v = (n-1)*estdev.v*estdev.v/cv_chi2_u;
      if(v < 0) estdev.v_ncl[i] = bad_data_double;
      else      estdev.v_ncl[i] = sqrt(v);

      v = (n-1)*estdev.v*estdev.v/cv_chi2_l;
      if(v < 0) estdev.v_ncu[i] = bad_data_double;
      else      estdev.v_ncu[i] = sqrt(v);

   } // end for i

   return;
}

////////////////////////////////////////////////////////////////////////
//
// Code for class SL1L2Info
//
////////////////////////////////////////////////////////////////////////

SL1L2Info::SL1L2Info() {
   init_from_scratch();
}

////////////////////////////////////////////////////////////////////////

SL1L2Info::~SL1L2Info() {
   clear();
}

////////////////////////////////////////////////////////////////////////

SL1L2Info::SL1L2Info(const SL1L2Info &c) {

   init_from_scratch();

   assign(c);
}

////////////////////////////////////////////////////////////////////////

SL1L2Info & SL1L2Info::operator=(const SL1L2Info &c) {

   if(this == &c) return(*this);

   assign(c);

   return(*this);
}

////////////////////////////////////////////////////////////////////////

SL1L2Info & SL1L2Info::operator+=(const SL1L2Info &c) {
   SL1L2Info s_info;

   s_info.scount  = scount + c.scount;

   if(s_info.scount > 0) {
      s_info.fbar  = (fbar*scount  + c.fbar*c.scount) /s_info.scount;
      s_info.obar  = (obar*scount  + c.obar*c.scount) /s_info.scount;
      s_info.fobar = (fobar*scount + c.fobar*c.scount)/s_info.scount;
      s_info.ffbar = (ffbar*scount + c.ffbar*c.scount)/s_info.scount;
      s_info.oobar = (oobar*scount + c.oobar*c.scount)/s_info.scount;
   }

   s_info.sacount  = sacount + c.sacount;

   if(s_info.sacount > 0) {
      s_info.fabar  = (fabar*sacount  + c.fabar*c.sacount) /s_info.sacount;
      s_info.oabar  = (oabar*sacount  + c.oabar*c.sacount) /s_info.sacount;
      s_info.foabar = (foabar*sacount + c.foabar*c.sacount)/s_info.sacount;
      s_info.ffabar = (ffabar*sacount + c.ffabar*c.sacount)/s_info.sacount;
      s_info.ooabar = (ooabar*sacount + c.ooabar*c.sacount)/s_info.sacount;
   }

   assign(s_info);

   return(*this);
}

////////////////////////////////////////////////////////////////////////

void SL1L2Info::init_from_scratch() {

   clear();

   return;
}

////////////////////////////////////////////////////////////////////////

void SL1L2Info::zero_out() {

   // SL1L2 Quantities
   fbar   = obar   = 0.0;
   fobar  = ffbar  = oobar  = 0.0;
   scount = 0;

   // SAL1L2 Quantities
   fabar  = oabar  = 0.0;
   foabar = ffabar = ooabar = 0.0;
   sacount = 0;

   return;
}

////////////////////////////////////////////////////////////////////////

void SL1L2Info::clear() {

   zero_out();

   return;
}

////////////////////////////////////////////////////////////////////////

void SL1L2Info::assign(const SL1L2Info &c) {

   clear();

   // SL1L2 Quantities
   fbar    = c.fbar;
   obar    = c.obar;
   fobar   = c.fobar;
   ffbar   = c.ffbar;
   oobar   = c.oobar;
   scount  = c.scount;

   // SAL1L2 Quantities
   fabar   = c.fabar;
   oabar   = c.oabar;
   foabar  = c.foabar;
   ffabar  = c.ffabar;
   ooabar  = c.ooabar;
   sacount = c.sacount;

   return;
}

////////////////////////////////////////////////////////////////////////

void compute_cntinfo(const SL1L2Info &s, int aflag, CNTInfo &cnt_info) {
   double den, f, o, v;

   // Handle the count
   if(!aflag) cnt_info.n = s.scount;
   else       cnt_info.n = s.sacount;

   // Set the quantities that can't be derived from SL1L2Info to bad data
   cnt_info.sp_corr.set_bad_data();
   cnt_info.kt_corr.set_bad_data();
   cnt_info.mae.set_bad_data();
   cnt_info.e10.set_bad_data();
   cnt_info.e25.set_bad_data();
   cnt_info.e50.set_bad_data();
   cnt_info.e75.set_bad_data();
   cnt_info.e90.set_bad_data();
   cnt_info.n_ranks    = 0;
   cnt_info.frank_ties = 0;
   cnt_info.orank_ties = 0;

   // Compute forecast mean
   if(!aflag) cnt_info.fbar.v = s.fbar;
   else       cnt_info.fbar.v = s.fabar;

   // Compute forecast standard deviation
   if(!aflag) cnt_info.fstdev.v = compute_stdev(s.fbar*s.scount,
                                                s.ffbar*s.scount,
                                                s.scount);
   else       cnt_info.fstdev.v = compute_stdev(s.fabar*s.sacount,
                                                s.ffabar*s.sacount,
                                                s.sacount);

   // Compute observation mean
   if(!aflag) cnt_info.obar.v = s.obar;
   else       cnt_info.obar.v = s.oabar;

   // Compute observation standard deviation
   if(!aflag) cnt_info.ostdev.v = compute_stdev(s.obar*s.scount,
                                                s.oobar*s.scount,
                                                s.scount);
   else       cnt_info.ostdev.v = compute_stdev(s.oabar*s.sacount,
                                                s.ooabar*s.sacount,
                                                s.sacount);

   // Compute f*o mean
   if(!aflag) cnt_info.fobar = s.fobar;
   else       cnt_info.fobar = s.foabar;

   // Compute forecast squared mean
   if(!aflag) cnt_info.ffbar = s.ffbar;
   else       cnt_info.ffbar = s.ffabar;

   // Compute observation squared mean
   if(!aflag) cnt_info.oobar = s.oobar;
   else       cnt_info.oobar = s.ooabar;

   // Compute multiplicative bias
   if(is_eq(cnt_info.obar.v, 0.0)) cnt_info.mbias.v = bad_data_double;
   else                            cnt_info.mbias.v = cnt_info.fbar.v/cnt_info.obar.v;

   // Compute correlation coefficient
   v =  (cnt_info.n*cnt_info.ffbar*cnt_info.n
       - cnt_info.fbar.v*cnt_info.n*cnt_info.fbar.v*cnt_info.n)
        *
        (cnt_info.n*cnt_info.oobar*cnt_info.n
       - cnt_info.obar.v*cnt_info.n*cnt_info.obar.v*cnt_info.n);

   if(v < 0 || is_eq(v, 0.0)) {
      cnt_info.pr_corr.v = bad_data_double;
   }
   else {
      den = sqrt(v);
      cnt_info.pr_corr.v = (  (cnt_info.n*cnt_info.fobar*cnt_info.n)
                            - (cnt_info.fbar.v*cnt_info.n*cnt_info.obar.v*cnt_info.n))
                           /den;
   }

   // Check that the correlation is not bigger than 1
   if(cnt_info.pr_corr.v > 1) cnt_info.pr_corr.v = bad_data_double;

   // Compute mean error
   cnt_info.me.v = cnt_info.fbar.v - cnt_info.obar.v;

   // Compute mean squared error
   cnt_info.mse.v = cnt_info.ffbar + cnt_info.oobar - 2.0*cnt_info.fobar;

   // Compute standard deviation of the mean error
   cnt_info.estdev.v = compute_stdev(cnt_info.me.v*cnt_info.n,
                                     cnt_info.mse.v*cnt_info.n, cnt_info.n);

   // Compute bias corrected mean squared error (decomposition of MSE)
   f = cnt_info.fbar.v;
   o = cnt_info.obar.v;
   cnt_info.bcmse.v = cnt_info.mse.v - (f-o)*(f-o);

   // Compute root mean squared error
   cnt_info.rmse.v = sqrt(cnt_info.mse.v);

   //
   // Compute normal confidence intervals
   //
   cnt_info.compute_ci();

   return;
}

////////////////////////////////////////////////////////////////////////
//
// Code for class VL1L2Info
//
////////////////////////////////////////////////////////////////////////

VL1L2Info::VL1L2Info() {
   init_from_scratch();
}

////////////////////////////////////////////////////////////////////////

VL1L2Info::~VL1L2Info() {
   clear();
}

////////////////////////////////////////////////////////////////////////

VL1L2Info::VL1L2Info(const VL1L2Info &c) {

   init_from_scratch();

   assign(c);
}

////////////////////////////////////////////////////////////////////////

VL1L2Info & VL1L2Info::operator=(const VL1L2Info &c) {

   if(this == &c) return(*this);

   assign(c);

   return(*this);
}

////////////////////////////////////////////////////////////////////////

VL1L2Info & VL1L2Info::operator+=(const VL1L2Info &c) {
   VL1L2Info v_info;

   v_info.vcount  = vcount + c.vcount;

   if(v_info.vcount > 0) {
      v_info.ufbar   = (ufbar*vcount   + c.ufbar*c.vcount)  /v_info.vcount;
      v_info.vfbar   = (vfbar*vcount   + c.vfbar*c.vcount)  /v_info.vcount;
      v_info.uobar   = (uobar*vcount   + c.uobar*c.vcount)  /v_info.vcount;
      v_info.vobar   = (vobar*vcount   + c.vobar*c.vcount)  /v_info.vcount;
      v_info.uvfobar = (uvfobar*vcount + c.uvfobar*c.vcount)/v_info.vcount;
      v_info.uvffbar = (uvffbar*vcount + c.uvffbar*c.vcount)/v_info.vcount;
      v_info.uvoobar = (uvoobar*vcount + c.uvoobar*c.vcount)/v_info.vcount;
   }

   v_info.vacount  = vacount + c.vacount;

   if(v_info.vacount > 0) {
      v_info.ufabar   = (ufabar*vacount   + c.ufabar*c.vacount)  /v_info.vacount;
      v_info.vfabar   = (vfabar*vacount   + c.vfabar*c.vacount)  /v_info.vacount;
      v_info.uoabar   = (uoabar*vacount   + c.uoabar*c.vacount)  /v_info.vacount;
      v_info.voabar   = (voabar*vacount   + c.voabar*c.vacount)  /v_info.vacount;
      v_info.uvfoabar = (uvfoabar*vacount + c.uvfoabar*c.vacount)/v_info.vacount;
      v_info.uvffabar = (uvffabar*vacount + c.uvffabar*c.vacount)/v_info.vacount;
      v_info.uvooabar = (uvooabar*vacount + c.uvooabar*c.vacount)/v_info.vacount;
   }

   assign(v_info);

   return(*this);
}

////////////////////////////////////////////////////////////////////////

void VL1L2Info::init_from_scratch() {

   clear();

   return;
}

////////////////////////////////////////////////////////////////////////

void VL1L2Info::zero_out() {

   // VL1L2 Quantities
   ufbar    = vfbar    = uobar    = vobar   = 0.0;
   uvfobar  = uvffbar  = uvoobar  = 0.0;
   vcount   = 0;

   // VAL1L2 Quantities
   ufabar   = vfabar   = uoabar   = voabar  = 0.0;
   uvfoabar = uvffabar = uvooabar = 0.0;
   vacount  = 0;

   return;
}

////////////////////////////////////////////////////////////////////////

void VL1L2Info::clear() {

   // Wind speed thresholds
   wind_fcst_thresh.clear();
   wind_obs_thresh.clear();

   zero_out();

   return;
}

////////////////////////////////////////////////////////////////////////

void VL1L2Info::assign(const VL1L2Info &c) {

   clear();

   // VL1L2 Quantities
   ufbar    = c.ufbar;
   vfbar    = c.vfbar;
   uobar    = c.uobar;
   vobar    = c.vobar;
   uvfobar  = c.uvfobar;
   uvffbar  = c.uvffbar;
   uvoobar  = c.uvoobar;
   vcount   = c.vcount;

   // VAL1L2 Quantities
   ufabar   = c.ufabar;
   vfabar   = c.vfabar;
   uoabar   = c.uoabar;
   voabar   = c.voabar;
   uvfoabar = c.uvfoabar;
   uvffabar = c.uvffabar;
   uvooabar = c.uvooabar;
   vacount  = c.vacount;

   return;
}

////////////////////////////////////////////////////////////////////////
//
// Code for class NBRCTSInfo
//
////////////////////////////////////////////////////////////////////////

NBRCTSInfo::NBRCTSInfo() {
   init_from_scratch();
}

////////////////////////////////////////////////////////////////////////

NBRCTSInfo::~NBRCTSInfo() {
   clear();
}

////////////////////////////////////////////////////////////////////////

NBRCTSInfo::NBRCTSInfo(const NBRCTSInfo &c) {

   init_from_scratch();

   assign(c);
}

////////////////////////////////////////////////////////////////////////

NBRCTSInfo & NBRCTSInfo::operator=(const NBRCTSInfo &c) {

   if(this == &c) return(*this);

   assign(c);

   return(*this);
}

////////////////////////////////////////////////////////////////////////

void NBRCTSInfo::init_from_scratch() {

   clear();

   return;
}

////////////////////////////////////////////////////////////////////////

void NBRCTSInfo::clear() {

   cts_info.clear();
   nbr_wdth = bad_data_int;
   raw_fcst_thresh.clear();
   raw_obs_thresh.clear();
   frac_thresh.clear();

   return;
}

////////////////////////////////////////////////////////////////////////

void NBRCTSInfo::assign(const NBRCTSInfo &c) {

   clear();

   cts_info        = c.cts_info;
   nbr_wdth        = c.nbr_wdth;
   raw_fcst_thresh = c.raw_fcst_thresh;
   raw_obs_thresh  = c.raw_obs_thresh;
   frac_thresh     = c.frac_thresh;

   return;
}

////////////////////////////////////////////////////////////////////////

void NBRCTSInfo::allocate_n_alpha(int i) {

   cts_info.allocate_n_alpha(i);

   return;
}

////////////////////////////////////////////////////////////////////////
//
// Code for class NBRCNTInfo
//
////////////////////////////////////////////////////////////////////////

NBRCNTInfo::NBRCNTInfo() {
   init_from_scratch();
}

////////////////////////////////////////////////////////////////////////

NBRCNTInfo::~NBRCNTInfo() {
   clear();
}

////////////////////////////////////////////////////////////////////////

NBRCNTInfo::NBRCNTInfo(const NBRCNTInfo &c) {

   init_from_scratch();

   assign(c);
}

////////////////////////////////////////////////////////////////////////

NBRCNTInfo & NBRCNTInfo::operator=(const NBRCNTInfo &c) {

   if(this == &c) return(*this);

   assign(c);

   return(*this);
}

////////////////////////////////////////////////////////////////////////

void NBRCNTInfo::init_from_scratch() {

   clear();

   return;
}

////////////////////////////////////////////////////////////////////////

void NBRCNTInfo::clear() {

   fbs.clear();
   fss.clear();
   cnt_info.clear();
   nbr_wdth = bad_data_int;
   raw_fcst_thresh.clear();
   raw_obs_thresh.clear();

   return;
}

////////////////////////////////////////////////////////////////////////

void NBRCNTInfo::assign(const NBRCNTInfo &c) {

   clear();

   fbs             = c.fbs;
   fss             = c.fss;
   cnt_info        = c.cnt_info;
   nbr_wdth        = c.nbr_wdth;
   raw_fcst_thresh = c.raw_fcst_thresh;
   raw_obs_thresh  = c.raw_obs_thresh;

   return;
}

////////////////////////////////////////////////////////////////////////

void NBRCNTInfo::allocate_n_alpha(int i) {

   cnt_info.allocate_n_alpha(i);

   fbs.allocate_n_alpha(i);
   fss.allocate_n_alpha(i);

   return;
}

////////////////////////////////////////////////////////////////////////

void NBRCNTInfo::compute_stats() {
   double num, den;
   int n;

   //
   // Compute FBS
   //
   n   = cnt_info.n;
   num = cnt_info.ffbar*n + cnt_info.oobar*n - 2.0*cnt_info.fobar*n;

   if(n == 0) fbs.v = bad_data_double;
   else       fbs.v = (double) num/n;

   //
   // Compute FSS
   //
   num = fbs.v;
   den = cnt_info.ffbar + cnt_info.oobar;

   if(is_eq(den, 0.0)) fss.v = bad_data_double;
   else                fss.v = 1.0 - (num/den);

   return;
}

////////////////////////////////////////////////////////////////////////
//
// Code for class ISCInfo
//
////////////////////////////////////////////////////////////////////////

ISCInfo::ISCInfo() {
   init_from_scratch();
}

////////////////////////////////////////////////////////////////////////

ISCInfo::~ISCInfo() {
   clear();
}

////////////////////////////////////////////////////////////////////////

ISCInfo::ISCInfo(const ISCInfo &c) {

   init_from_scratch();

   assign(c);
}

////////////////////////////////////////////////////////////////////////

ISCInfo & ISCInfo::operator=(const ISCInfo &c) {

   if(this == &c) return(*this);

   assign(c);

   return(*this);
}

////////////////////////////////////////////////////////////////////////

void ISCInfo::init_from_scratch() {

   mse_scale = (double *) 0;
   isc_scale = (double *) 0;
   fen_scale = (double *) 0;
   oen_scale = (double *) 0;

   clear();

   return;
}

////////////////////////////////////////////////////////////////////////

void ISCInfo::clear() {

   cts.zero_out();
   total = 0;

   mse   = isc   = bad_data_double;
   fen   = oen   = bad_data_double;
   baser = fbias = bad_data_double;

   tile_dim = bad_data_int;
   tile_xll = bad_data_int;
   tile_yll = bad_data_int;

   n_scale  = 0;

   if(mse_scale) { delete [] mse_scale; mse_scale = (double *) 0; }
   if(isc_scale) { delete [] mse_scale; mse_scale = (double *) 0; }
   if(fen_scale) { delete [] mse_scale; mse_scale = (double *) 0; }
   if(oen_scale) { delete [] mse_scale; mse_scale = (double *) 0; }

   return;
}

////////////////////////////////////////////////////////////////////////

void ISCInfo::zero_out() {
   int i;

   cts.zero_out();
   total = 0;

   mse   = isc   = 0.0;
   fen   = oen   = 0.0;
   baser = fbias = 0.0;

   tile_dim = bad_data_int;
   tile_xll = bad_data_int;
   tile_yll = bad_data_int;


   if(n_scale > 0) {
      for(i=0; i<=n_scale; i++) {
         mse_scale[i] = 0.0;
         isc_scale[i] = 0.0;
         fen_scale[i] = 0.0;
         oen_scale[i] = 0.0;
      }
   }

   return;
}

////////////////////////////////////////////////////////////////////////

void ISCInfo::assign(const ISCInfo &c) {
   int i;

   clear();

   cts = c.cts;

   cts_thresh = c.cts_thresh;

   mse   = c.mse;
   isc   = c.isc;
   fen   = c.fen;
   oen   = c.oen;
   baser = c.baser;
   fbias = c.fbias;
   total = c.total;

   tile_dim = c.tile_dim;
   tile_xll = c.tile_xll;
   tile_yll = c.tile_yll;

   allocate_n_scale(c.n_scale);

   for(i=0; i<n_scale; i++) {
      mse_scale[i] = c.mse_scale[i];
      isc_scale[i] = c.isc_scale[i];
      fen_scale[i] = c.fen_scale[i];
      oen_scale[i] = c.oen_scale[i];
   }

   return;
}

////////////////////////////////////////////////////////////////////////

void ISCInfo::allocate_n_scale(int i) {
   int j;

   n_scale = i;

   if(n_scale > 0) {
      mse_scale = new double [n_scale+1];
      isc_scale = new double [n_scale+1];
      fen_scale = new double [n_scale+1];
      oen_scale = new double [n_scale+1];

      if(!mse_scale || !isc_scale || !fen_scale || !oen_scale) {
         cerr << "\n\nERROR: ISCInfo::allocate_n_scale() -> "
              << "Memory allocation error!\n\n" << flush;
         exit(1);
      }
   }

   // Initialize the values
   for(j=0; j<=n_scale; j++) {
      mse_scale[j] = bad_data_double;
      isc_scale[j] = bad_data_double;
      fen_scale[j] = bad_data_double;
      oen_scale[j] = bad_data_double;
   }

   return;
}

////////////////////////////////////////////////////////////////////////

void ISCInfo::compute_isc() {
   double den;
   int i;

   // Get the Total, Base Rate, and Frequency Bias
   total = cts.n();
   fbias = cts.fbias();
   baser = cts.baser();

   // Compute the denominator for ISC
   den = fbias*baser*(1.0 - baser) + baser*(1.0 - fbias*baser);

   // Compute ISC for the whole field
   if(is_bad_data(fbias) ||
      is_bad_data(baser) ||
      is_bad_data(mse)   ||
      is_eq(den, 0.0)) isc = bad_data_double;
   else                isc = 1.0 - mse/den;

   // Compute the Intensity-Scale score for each scale
   den /= (n_scale+1);
   for(i=0; i<=n_scale; i++) {

      if(is_bad_data(fbias)        ||
         is_bad_data(baser)        ||
         is_bad_data(mse_scale[i]) ||
         is_eq(den, 0.0)) isc_scale[i] = bad_data_double;
      else                isc_scale[i] = 1.0 - mse_scale[i]/den;
   } // end for i

   return;
}

////////////////////////////////////////////////////////////////////////

void ISCInfo::compute_isc(int i) {
   double den;

   // Get the Total, Base Rate, and Frequency Bias
   total = cts.n();
   fbias = cts.fbias();
   baser = cts.baser();

   // Compute the denominator for ISC
   den = fbias*baser*(1.0 - baser) + baser*(1.0 - fbias*baser);

   // Compute ISC for the whole field
   if(i < 0) {
      if(is_bad_data(fbias) ||
         is_bad_data(baser) ||
         is_bad_data(mse)   ||
         is_eq(den, 0.0)) isc = bad_data_double;
      else                isc = 1.0 - mse/den;
   }

   // Compute the Intensity-Scale score for each scale
   else {
      den /= (n_scale+1);

      if(is_bad_data(fbias)        ||
         is_bad_data(baser)        ||
         is_bad_data(mse_scale[i]) ||
         is_eq(den, 0.0)) isc_scale[i] = bad_data_double;
      else                isc_scale[i] = 1.0 - mse_scale[i]/den;
   }

   return;
}

////////////////////////////////////////////////////////////////////////
//
// Code for class PCTInfo
//
////////////////////////////////////////////////////////////////////////

PCTInfo::PCTInfo() {
   init_from_scratch();
}

////////////////////////////////////////////////////////////////////////

PCTInfo::~PCTInfo() {
   clear();
}

////////////////////////////////////////////////////////////////////////

PCTInfo::PCTInfo(const PCTInfo &c) {

   init_from_scratch();

   assign(c);
}

////////////////////////////////////////////////////////////////////////

PCTInfo & PCTInfo::operator=(const PCTInfo &c) {

   if(this == &c) return(*this);

   assign(c);

   return(*this);
}

////////////////////////////////////////////////////////////////////////

void PCTInfo::init_from_scratch() {

   alpha = (double *) 0;

   clear();

   return;
}

////////////////////////////////////////////////////////////////////////

void PCTInfo::clear() {

   n_alpha = 0;
   if(alpha) { delete [] alpha; alpha = (double *) 0; }

   pct.zero_out();
   pct_fcst_thresh.clear();
   pct_obs_thresh.clear();

   brier.clear();

   return;
}

////////////////////////////////////////////////////////////////////////

void PCTInfo::assign(const PCTInfo &c) {
   int i;

   clear();

   pct = c.pct;
   pct_fcst_thresh = c.pct_fcst_thresh;
   pct_obs_thresh  = c.pct_obs_thresh;

   allocate_n_alpha(c.n_alpha);
   for(i=0; i<c.n_alpha; i++) { alpha[i] = c.alpha[i]; }

   brier = c.brier;

   return;
}

////////////////////////////////////////////////////////////////////////

void PCTInfo::allocate_n_alpha(int i) {

   n_alpha = i;

   if(n_alpha > 0) {

      alpha = new double [n_alpha];

      if(!alpha) {
         cerr << "\n\nERROR: PCTInfo::allocate_n() -> "
              << "Memory allocation error!\n\n" << flush;
         exit(1);
      }

      brier.allocate_n_alpha(n_alpha);
   }

   return;
}

////////////////////////////////////////////////////////////////////////

void PCTInfo::compute_stats() {

   brier.v = pct.brier_score();

   return;
}

////////////////////////////////////////////////////////////////////////

void PCTInfo::compute_ci() {
   int i;
   double halfwidth;

   //
   // Compute confidence intervals for each alpha value specified
   //
   for(i=0; i<n_alpha; i++) {

      halfwidth = pct.brier_ci_halfwidth(alpha[i]);
      brier.v_ncl[i] = brier.v - halfwidth;
      brier.v_ncu[i] = brier.v + halfwidth;
   } // end for i

   return;
}

////////////////////////////////////////////////////////////////////////
//
// Begin code for misc functions
//
////////////////////////////////////////////////////////////////////////

int parse_message_type(const char *msg_typ_str, char **&msg_typ_arr) {
   char tmp_str[max_str_len];
   char *c = (char *) 0;
   int n, i;

   // Compute the number of tokens in the string based on " "
   n = num_tokens(msg_typ_str, " ");

   // Check for no tokens in string
   if(n == 0) return(0);

   // Allocate space for the list of tokens
   msg_typ_arr = new char * [n];

   // Initialize the temp string for use in tokenizing
   strcpy(tmp_str, msg_typ_str);

   // Tokenize the string and store the double values
   c = strtok(tmp_str, " ");
   msg_typ_arr[0] = new char [strlen(c)+1];
   strcpy(msg_typ_arr[0], c);

   // Parse remaining tokens
   for(i=1; i<n; i++) {
      c = strtok(0, " ");
      msg_typ_arr[i] = new char [strlen(c)+1];
      strcpy(msg_typ_arr[i], c);
   }

   return(n);
}

////////////////////////////////////////////////////////////////////////

int parse_dbl_list(const char *dbl_str, double *&dbl_arr) {
   char tmp_str[max_str_len];
   char *c = (char *) 0;
   int n, i;

   // Compute the number of tokens in the string based on " "
   n = num_tokens(dbl_str, " ");

   // Check for no tokens in string
   if(n == 0) return(0);

   // Allocate space for the list of tokens
   dbl_arr = new double [n];

   // Initialize the temp string for use in tokenizing
   strcpy(tmp_str, dbl_str);

   // Tokenize the string and store the double values
   c = strtok(tmp_str, " ");
   dbl_arr[0] = atof(c);

   // Parse remaining tokens
   for(i=1; i<n; i++) dbl_arr[i] = atof(strtok(0, " "));

   return(n);
}

////////////////////////////////////////////////////////////////////////

int parse_int_list(const char *int_str, int *&int_arr) {
   char tmp_str[max_str_len];
   char *c = (char *) 0;
   int n, i;

   // Compute the number of tokens in the string based on " "
   n = num_tokens(int_str, " ");

   // Check for no tokens in string
   if(n == 0) return(0);

   // Allocate space for the list of tokens
   int_arr = new int [n];

   // Initialize the temp string for use in tokenizing
   strcpy(tmp_str, int_str);

   // Tokenize the string and store the integer values
   c = strtok(tmp_str, " ");
   int_arr[0] = nint(atof(c));

   // Parse remaining tokens
   for(i=1; i<n; i++) int_arr[i] = nint(atof(strtok(0, " ")));

   return(n);
}

////////////////////////////////////////////////////////////////////////

int max_int(const int *v_int, int n) {
   int i, v_max;

   if(n <= 0) return(0);

   v_max = v_int[0];
   for(i=1; i<n; i++) if(v_int[i] > v_max) v_max = v_int[i];

   return(v_max);
}

////////////////////////////////////////////////////////////////////////

int min_int(const int *v_int, int n) {
   int i, v_min;

   if(n <= 0) return(0);

   v_min = v_int[0];
   for(i=1; i<n; i++) if(v_int[i] < v_min) v_min = v_int[i];

   return(v_min);
}

////////////////////////////////////////////////////////////////////////

void dbl_to_str(double v, char *v_str) {

   dbl_to_str(v, v_str, default_precision);

   return;
}

////////////////////////////////////////////////////////////////////////

void dbl_to_str(double v, char *v_str, int precision) {
   char fmt_str[32];

   sprintf(fmt_str, "%s%i%s", "%.", precision, "f");

   if(is_bad_data(v)) sprintf(v_str, "%i", bad_data_int);
   else               sprintf(v_str, fmt_str, v);

   return;
}

////////////////////////////////////////////////////////////////////////

double compute_stdev(double sum, double sum_sq, int n) {
   double sigma, v;

   if(n <= 1) {
      sigma = bad_data_double;
   }
   else {

      v = (sum_sq - sum*sum/(double) n)/((double) (n - 1));

      if(v < 0) sigma = bad_data_double;
      else      sigma = sqrt(v);
   }

   return(sigma);
}

///////////////////////////////////////////////////////////////////////////////
//
// Convert a field of data into its corresponding field of ranks of that data.
// Return the number of valid data points that were ranked and keep track
// of the number of rank ties.
//
///////////////////////////////////////////////////////////////////////////////

int compute_rank(const WrfData &wd, WrfData &wd_rank, double *data_rank, int &ties) {
   int x, y, n, i;
   double *data;
   int *data_loc;

   // Arrays to store the raw data values to be ranked, their locations,
   // and their computed ranks.  The ranks are stored as doubles since
   // they can be set to 0.5 in the case of ties.
   data      = new double [wd.get_nx()*wd.get_ny()];
   data_loc  = new int    [wd.get_nx()*wd.get_ny()];

   // Search the input field for valid data and keep track of its location
   n = 0;
   for(x=0; x<wd.get_nx(); x++) {
      for(y=0; y<wd.get_ny(); y++) {

         if(!wd.is_bad_xy(x, y)) {
            data[n] = wd.get_xy_double(x, y);
            data_loc[n] = wd.two_to_one(x, y);
            n++;
         }
      }
   }

   // Compute the rank of the data and store the ranks in the data_rank array
   // Keep track of the number of ties in the ranks
   ties = rank(data, data_rank, n);

   // Set up the wd_rank object
   wd_rank.set_size(wd.get_nx(), wd.get_ny());
   wd_rank.set_m((double) n/wrfdata_int_data_max);
   wd_rank.set_b(0.0);

   // Assign the ranks to the wd_rank field
   for(i=0; i<n; i++) {
      wd_rank.one_to_two(data_loc[i], x, y);
      wd_rank.put_xy_double(data_rank[i], x, y);
   }

   // Deallocate memory
   if(data)      { delete [] data;      data = (double *) 0; }
   if(data_loc)  { delete [] data_loc;  data_loc = (int *) 0; }

   return(n);
}

////////////////////////////////////////////////////////////////////////
//
// Assume that the input f_na and o_na contain only valid data.
//
////////////////////////////////////////////////////////////////////////

void compute_cntinfo(const NumArray &f_na, const NumArray &o_na,
                     const NumArray &i_na,
                     int fcst_gc, int obs_gc,
                     int cnt_flag, int rank_flag, int normal_ci_flag,
                     CNTInfo &cnt_info) {
   int i, j, n;
   double f, o, v, f_sum, o_sum, ff_sum, oo_sum, fo_sum;
   double err, err_sum, abs_err_sum, err_sq_sum, den;
   NumArray err_na;

   //
   // Check that the forecast and observation arrays of the same length
   //
   if(f_na.n_elements() != o_na.n_elements() ||
      f_na.n_elements() == 0) {
      cerr << "\n\nERROR: compute_cntinfo() -> "
           << "the forecast and observation arrays must have the same "
           << "non-zero length!\n\n" << flush;
      throw(1);
   }

   //
   // Loop over the length of the index array
   //
   n = i_na.n_elements();

   //
   // Compute the continuous statistics from the fcst and obs arrays
   //
   f_sum = o_sum = ff_sum = oo_sum = fo_sum = 0.0;
   err_sum = abs_err_sum = err_sq_sum = 0.0;
   for(i=0; i<n; i++) {

      //
      // Get the index to be used from the index num array
      //
      j = nint(i_na[i]);

      f = f_na[j];
      o = o_na[j];

      //
      // Compute the error
      //
      err = f-o;
      err_na.add(err);

      f_sum       += f;
      o_sum       += o;
      ff_sum      += f*f;
      oo_sum      += o*o;
      fo_sum      += f*o;
      err_sum     += err;
      abs_err_sum += abs((long double) err);
      err_sq_sum  += err*err;
   } // end for i

   //
   // Store the sample size
   //
   cnt_info.n = n;

   //
   // Compute forecast mean and standard deviation
   //
   cnt_info.fbar.v   = f_sum/n;
   cnt_info.fstdev.v = compute_stdev(f_sum, ff_sum, n);

   //
   // Compute observation mean and standard deviation
   //
   cnt_info.obar.v   = o_sum/n;
   cnt_info.ostdev.v = compute_stdev(o_sum, oo_sum, n);

   //
   // Compute f*o, f*f, and o*o means
   //
   cnt_info.fobar = fo_sum/n;
   cnt_info.ffbar = ff_sum/n;
   cnt_info.oobar = oo_sum/n;

   //
   // If the cnt_flag is not set return after computing the partial sums
   //
   if(!cnt_flag) return;

   //
   // Compute multiplicative bias
   //
   if(is_eq(cnt_info.obar.v, 0.0))
      cnt_info.mbias.v = bad_data_double;
   else
      cnt_info.mbias.v = cnt_info.fbar.v/cnt_info.obar.v;

   //
   // Compute Pearson correlation coefficient
   //
   v = (n*ff_sum - f_sum*f_sum)*(n*oo_sum - o_sum*o_sum);
   if(v < 0 || is_eq(v, 0.0)) {
      cnt_info.pr_corr.v = bad_data_double;
   }
   else {
      den = sqrt(v);
      cnt_info.pr_corr.v = ((n*fo_sum) - (f_sum*o_sum))/den;
   }

   //
   // Compute percentiles of the error
   //
   err_na.sort_array();
   cnt_info.e10.v = err_na.percentile_array(0.10);
   cnt_info.e25.v = err_na.percentile_array(0.25);
   cnt_info.e50.v = err_na.percentile_array(0.50);
   cnt_info.e75.v = err_na.percentile_array(0.75);
   cnt_info.e90.v = err_na.percentile_array(0.90);

   //
   // Compute mean error and standard deviation of the mean error
   //
   cnt_info.me.v     = err_sum/n;
   cnt_info.estdev.v = compute_stdev(err_sum, err_sq_sum, n);

   //
   // Compute mean absolute error
   //
   cnt_info.mae.v = abs_err_sum/n;

   //
   // Compute mean squared error
   //
   cnt_info.mse.v = err_sq_sum/n;

   //
   // Compute bias corrected mean squared error (decomposition of MSE)
   //
   f = cnt_info.fbar.v;
   o = cnt_info.obar.v;
   cnt_info.bcmse.v = cnt_info.mse.v - (f-o)*(f-o);

   //
   // Compute root mean squared error
   //
   cnt_info.rmse.v = sqrt(err_sq_sum/n);

   //
   // Only compute the Kendall Tau and Spearman's Rank corrleation
   // coefficients if the rank_flag is set.
   //
   if(rank_flag) {
      int concordant, discordant, extra_f, extra_o;
      int n_zero_zero, n_f_rank, n_o_rank, n_f_rank_ties, n_o_rank_ties;
      NumArray f_na2, o_na2, f_na_rank, o_na_rank;

      //
      // If verifying precipitation, mask out the (0, 0) cases.
      //
      if(is_precip_code(fcst_gc) && is_precip_code(obs_gc)) {

         for(i=0, n_zero_zero=0; i<n; i++) {

            //
            // Get the index to be used from the index num array
            //
            j = nint(i_na[i]);

            f = f_na[j];
            o = o_na[j];

            //
            // Only copy them over if f > 0 or o > 0
            //
            if(f > 0.0001 || o > 0.0001) {
               f_na2.add(f);
               o_na2.add(o);
            }
            else {
               n_zero_zero++;
            }
         } // end for i
      }
      //
      // Copy over the elements using the indices provided without
      // masking out the (0, 0) cases
      //
      else {

         for(i=0; i<n; i++) {
            j = nint(i_na[i]);
            f_na2.add(f_na[j]);
            o_na2.add(o_na[j]);
         }
         n_zero_zero = 0;
      }

      //
      // Compute ranks of the remaining raw data values
      // in the fcst and obs arrays
      //
      f_na_rank = f_na2;
      o_na_rank = o_na2;
      n_f_rank  = f_na_rank.rank_array(n_f_rank_ties);
      n_o_rank  = o_na_rank.rank_array(n_o_rank_ties);

      if(n_f_rank != n_o_rank) {

         cerr << "\n\nERROR: compute_cntinfo() -> "
              << "n_f_rank does not equal n_o_rank!\n\n"
              << flush;
         throw(1);
      }
      else {
         n = n_f_rank;
      }

      //
      // Store the number of ranks and ties
      //
      cnt_info.n_ranks    = n;
      cnt_info.frank_ties = n_f_rank_ties;
      cnt_info.orank_ties = n_o_rank_ties;

      //
      // Compute sums for the ranks for use in computing Spearman's
      // Rank correlation coefficient
      //
      f_sum = o_sum = ff_sum = oo_sum = fo_sum = 0.0;
      for(i=0; i<n_f_rank; i++) {

         f = f_na_rank[i];
         o = o_na_rank[i];

         f_sum  += f;
         o_sum  += o;
         ff_sum += f*f;
         oo_sum += o*o;
         fo_sum += f*o;
      } // end for i

      //
      // Compute Spearman's Rank correlation coefficient
      //
      v = (n*ff_sum - f_sum*f_sum)*(n*oo_sum - o_sum*o_sum);
      if(v < 0 || is_eq(v, 0.0)) {
         cnt_info.sp_corr.v = bad_data_double;
      }
      else {
         den = sqrt(v);
         cnt_info.sp_corr.v = ((n*fo_sum) - (f_sum*o_sum))/den;
      }

      //
      // Compute Kendall Tau Rank correlation coefficient:
      // For each pair of ranked data points (fi, oi), compare it to all other pairs
      // of ranked data points (fj, oj) where j > i.  If the relative ordering of the
      // ranks of the f's is the same as the relative ordering of the ranks of the o's,
      // count the comparison as concordant.  If the previous is not the case, count
      // the comparison as discordant.  If there is a tie between the o's, count the
      // comparison as extra_f.  A tie between the f's counts as an extra_o.  If there
      // is a tie in both the f's and o's, don't count the comparison as anything.
      //
      concordant = discordant = extra_f = extra_o = 0;
      for(i=0; i<n; i++) {
         for(j=i+1; j<n; j++) {

            //
            // Check for agreement in the relative ordering of ranks
            //
            if(      (f_na_rank[i] > f_na_rank[j] && o_na_rank[i] > o_na_rank[j]) ||
                     (f_na_rank[i] < f_na_rank[j] && o_na_rank[i] < o_na_rank[j]) ) concordant++;
            //
            // Check for disagreement in the relative ordering of ranks
            //
            else if( (f_na_rank[i] > f_na_rank[j] && o_na_rank[i] < o_na_rank[j]) ||
                     (f_na_rank[i] < f_na_rank[j] && o_na_rank[i] > o_na_rank[j]) ) discordant++;
            //
            // Check for ties in the forecast rank
            //
            else if(is_eq(f_na_rank[i], f_na_rank[j]) && !is_eq(o_na_rank[i], o_na_rank[j])) extra_o++;
            //
            // Check for ties in the observation rank
            //
            else if(!is_eq(f_na_rank[i], f_na_rank[j]) && is_eq(o_na_rank[i], o_na_rank[j])) extra_f++;
         }
      }
      den = sqrt((double) concordant+discordant+extra_f)*
            sqrt((double) concordant+discordant+extra_o);
      if(is_eq(den, 0.0)) cnt_info.kt_corr.v = bad_data_double;
      else                cnt_info.kt_corr.v = (concordant - discordant)/den;
   } // end if rank_flag

   //
   // Compute normal confidence intervals if the normal_ci_flag
   // is set
   //
   if(normal_ci_flag) cnt_info.compute_ci();

   return;
}

////////////////////////////////////////////////////////////////////////
//
// Compute the CNTInfo object from the pairs of data but remove the
// i-th data point.
//
////////////////////////////////////////////////////////////////////////

void compute_i_cntinfo(const NumArray &f_na, const NumArray &o_na,
                       int fcst_gc, int obs_gc, int skip,
                       int rank_flag, int normal_ci_flag,
                       CNTInfo &cnt_info) {
   int i, n, count;
   NumArray f_na_i, o_na_i, i_na_i;

   //
   // Check that the forecast and observation arrays of the same length
   //
   if(f_na.n_elements() != o_na.n_elements()) {
      cerr << "\n\nERROR: compute_i_cntinfo() -> "
           << "the forecast and observation arrays must have the same "
           << "length!\n\n" << flush;
      throw(1);
   }
   else {
      n = f_na.n_elements();
   }

   if(skip < 0 || skip > n) {
      cerr << "\n\nERROR: compute_i_cntinfo() -> "
           << "the skip index (" << skip << ") is out of bounds!\n\n"
           << flush;
      throw(1);
   }

   //
   // Copy over the forecast, observation, and index values except
   // for the one to be skipped
   //
   for(i=0, count=0; i<n; i++) {
      if(i == skip) continue;
      f_na_i.add(f_na[i]);
      o_na_i.add(o_na[i]);
      i_na_i.add(count);
      count++;
   }

   compute_cntinfo(f_na_i, o_na_i, i_na_i, fcst_gc, obs_gc,
                   1, rank_flag, normal_ci_flag, cnt_info);

   return;
}

////////////////////////////////////////////////////////////////////////

void compute_ctsinfo(const NumArray &f_na, const NumArray &o_na,
                     const NumArray &i_na,
                     int cts_flag, int normal_ci_flag,
                     CTSInfo &cts_info) {
   int i, j, n;
   double f, o;
   SingleThresh ft, ot;

   //
   // Check that the forecast and observation arrays of the same length
   //
   if(f_na.n_elements() != o_na.n_elements()) {
      cerr << "\n\nERROR: compute_ctsinfo() -> "
           << "the forecast and observation arrays must have the same "
           << "length!\n\n" << flush;
      throw(1);
   }

   //
   // Loop over the length of the index array
   //
   n = i_na.n_elements();

   //
   // Reset the CTS object
   //
   cts_info.cts.zero_out();

   //
   // Get the threshold value to be applied
   //
   ft = cts_info.cts_fcst_thresh;
   ot = cts_info.cts_obs_thresh;

   //
   // Loop through the pair data and fill in the contingency table
   //
   for(i=0; i<n; i++) {

      //
      // Get the index to be used from the index num array
      //
      j = nint(i_na[i]);

      f = f_na[j];
      o = o_na[j];

      //
      // Update the contingency table counts for this pair
      //
      if     ( ft.check(f) &&  ot.check(o)) cts_info.cts.inc_fy_oy();
      else if( ft.check(f) && !ot.check(o)) cts_info.cts.inc_fy_on();
      else if(!ft.check(f) &&  ot.check(o)) cts_info.cts.inc_fn_oy();
      else if(!ft.check(f) && !ot.check(o)) cts_info.cts.inc_fn_on();
   } // end for i

   //
   // Only compute the categorical stats if reqeusted
   //
   if(cts_flag) {

      cts_info.compute_stats();

      //
      // Only compute the normal confidence intervals if requested
      //
      if(normal_ci_flag) cts_info.compute_ci();
   }

   return;
}

////////////////////////////////////////////////////////////////////////

void compute_i_ctsinfo(const NumArray &f_na, const NumArray &o_na,
                       int skip,
                       int normal_ci_flag,
                       CTSInfo &cts_info) {
   int i, n, count;
   NumArray f_na_i, o_na_i, i_na_i;

   //
   // Check that the forecast and observation arrays of the same length
   //
   if(f_na.n_elements() != o_na.n_elements()) {
      cerr << "\n\nERROR: compute_i_ctsinfo() -> "
           << "the forecast and observation arrays must have the same "
           << "length!\n\n" << flush;
      throw(1);
   }
   else {
      n = f_na.n_elements();
   }

   if(skip < 0 || skip > n) {
      cerr << "\n\nERROR: compute_i_ctsinfo() -> "
           << "the skip index (" << skip << ") is out of bounds!\n\n"
           << flush;
      throw(1);
   }

   //
   // Copy over the forecast, observation, and index values except
   // for the one to be skipped
   //
   for(i=0, count=0; i<n; i++) {
      if(i == skip) continue;
      f_na_i.add(f_na[i]);
      o_na_i.add(o_na[i]);
      i_na_i.add(count);
      count++;
   }

   compute_ctsinfo(f_na_i, o_na_i, i_na_i,
                   1, normal_ci_flag, cts_info);

   return;
}

////////////////////////////////////////////////////////////////////////

void compute_pctinfo(const NumArray &f_na, const NumArray &o_na,
                     int pstd_flag, PCTInfo &pct_info) {
   int i, n_thresh, n_pair;
   double *p_thresh = (double *) 0;
   SingleThresh ot;

   //
   // Check that the forecast and observation arrays of the same length
   //
   if(f_na.n_elements() != o_na.n_elements()) {
      cerr << "\n\nERROR: compute_pctinfo() -> "
           << "the forecast and observation arrays must have the same "
           << "length!\n\n" << flush;
      throw(1);
   }
   n_pair = f_na.n_elements();

   //
   // Store the thresholds as an array of doubles
   //
   n_thresh = pct_info.pct_fcst_thresh.n_elements();
   p_thresh = new double [n_thresh];

   for(i=0; i<n_thresh; i++)
      p_thresh[i] = pct_info.pct_fcst_thresh[i].thresh;

   //
   // Set up the Nx2ContingencyTable
   //
   pct_info.pct.clear();
   pct_info.pct.set_size(n_thresh-1);
   pct_info.pct.set_thresholds(p_thresh);

   //
   // Get the observation threshold value to be applied
   //
   ot = pct_info.pct_obs_thresh;

   //
   // Loop through the pair data and fill in the contingency table
   //
   for(i=0; i<n_pair; i++) {

      //
      // Check the observation thresholds and increment accordingly
      //
      if(ot.check(o_na[i])) pct_info.pct.inc_event(f_na[i]);
      else                  pct_info.pct.inc_nonevent(f_na[i]);
   } // end for i

   //
   // Only compute the probabilistic stats if reqeusted
   //
   if(pstd_flag) {
      pct_info.compute_stats();
      pct_info.compute_ci();
   }

   //
   // Deallocate memory
   //
   if(p_thresh) { delete [] p_thresh; p_thresh = (double *) 0; }

   return;
}

////////////////////////////////////////////////////////////////////////
//
// Compute the following partial sums info for use in computing
// FBS and FSS:
//    n, ffbar, oobar, fobar
//
////////////////////////////////////////////////////////////////////////

void compute_nbrcntinfo(const NumArray &f_na, const NumArray &o_na,
                        const NumArray &i_na,
                        NBRCNTInfo &nbrcnt_info, int nbrcnt_flag) {
   int i, j, n;
   double f, o, ff_sum, oo_sum, fo_sum;

   //
   // Check that the forecast and observation arrays of the same length
   //
   if(f_na.n_elements() != o_na.n_elements() ||
      f_na.n_elements() == 0) {
      cerr << "\n\nERROR: compute_nbrcntinfo() -> "
           << "the forecast and observation arrays must have the same "
           << "non-zero length!\n\n" << flush;
      throw(1);
   }

   //
   // Loop over the length of the index array
   //
   n = i_na.n_elements();

   //
   // Compute the continuous statistics from the fcst and obs arrays
   //
   ff_sum = oo_sum = fo_sum = 0.0;
   for(i=0; i<n; i++) {

      //
      // Get the index to be used from the index num array
      //
      j = nint(i_na[i]);

      f = f_na[j];
      o = o_na[j];

      ff_sum += f*f;
      oo_sum += o*o;
      fo_sum += f*o;
   } // end for i

   //
   // Store the sample size
   //
   nbrcnt_info.cnt_info.n = n;

   //
   // Compute the f*o, f*f, and o*o means
   //
   nbrcnt_info.cnt_info.fobar = fo_sum/n;
   nbrcnt_info.cnt_info.ffbar = ff_sum/n;
   nbrcnt_info.cnt_info.oobar = oo_sum/n;

   //
   // Only compute FBS and FSS if requested
   //
   if(nbrcnt_flag) {
      nbrcnt_info.compute_stats();
   }

   return;
}

////////////////////////////////////////////////////////////////////////

void compute_i_nbrcntinfo(const NumArray &f_na, const NumArray &o_na,
                          int skip,
                          NBRCNTInfo &nbrcnt_info) {
   int i, n, count;
   NumArray f_na_i, o_na_i, i_na_i;

   //
   // Check that the forecast and observation arrays of the same length
   //
   if(f_na.n_elements() != o_na.n_elements()) {
      cerr << "\n\nERROR: compute_i_nbrcntinfo() -> "
           << "the forecast and observation arrays must have the same "
           << "length!\n\n" << flush;
      throw(1);
   }
   else {
      n = f_na.n_elements();
   }

   if(skip < 0 || skip > n) {
      cerr << "\n\nERROR: compute_i_nbrcntinfo() -> "
           << "the skip index (" << skip << ") is out of bounds!\n\n"
           << flush;
      throw(1);
   }

   //
   // Copy over the forecast, observation, and index values except
   // for the one to be skipped
   //
   for(i=0, count=0; i<n; i++) {
      if(i == skip) continue;
      f_na_i.add(f_na[i]);
      o_na_i.add(o_na[i]);
      i_na_i.add(count);
      count++;
   }

   compute_nbrcntinfo(f_na_i, o_na_i, i_na_i, nbrcnt_info, 1);

   return;
}

////////////////////////////////////////////////////////////////////////

void compute_mean_stdev(const NumArray &v_na, const NumArray &i_na,
                        int normal_ci_flag, double alpha,
                        CIInfo &mean_ci, CIInfo &stdev_ci) {
   int i, j, n;
   double v, sum, sum_sq;
   double cv_normal_l, cv_normal_u;

   //
   // Loop over the length of the index array
   //
   n = i_na.n_elements();

   //
   // Loop over the values provided
   //
   sum = sum_sq = 0.0;
   for(i=0; i<n; i++) {

      //
      // Get the index to be used from the index num array
      //
      j = nint(i_na[i]);

      v = v_na[j];

      sum    += v;
      sum_sq += v*v;
   } // end for i

   //
   // Compute the mean
   //
   if(n == 0) mean_ci.v = bad_data_double;
   else       mean_ci.v = sum/n;

   //
   // Compute the standard deviation
   //
   if(n <= 1) {
      stdev_ci.v = bad_data_double;
   }
   else {

      v = (sum_sq - sum*sum/(double) n)/((double) (n - 1));

      if(v < 0) stdev_ci.v = bad_data_double;
      else      stdev_ci.v = sqrt(v);
   }

   //
   // Compute the normal confidence interval for the mean
   // if the normal_ci_flag is set
   //
   if(normal_ci_flag) {

      //
      // Check for the degenerate case
      //
      if(n <= 1) {
         mean_ci.v_ncl[0]  = mean_ci.v_ncu[0]   = bad_data_double;
      }
      else {

         //
         // Compute the critical values for the Normal or Student's-T
         // distribution based on the sample size
         //
         if(n >= large_sample_threshold) {
            cv_normal_l = normal_cdf_inv(alpha/2.0, 0.0, 1.0);
            cv_normal_u = normal_cdf_inv(1.0 - (alpha/2.0), 0.0, 1.0);
         }
         //
         // If the number of samples is less than the large sample threshold,
         // use the T-distribution
         //
         else {
            cv_normal_l = students_t_cdf_inv(alpha/2.0, n-1);
            cv_normal_u = students_t_cdf_inv(1.0 - (alpha/2.0), n-1);
         }

         //
         // Compute confidence interval for the mean
         //
         mean_ci.v_ncl[0] = mean_ci.v +
                            cv_normal_l*stdev_ci.v/sqrt((double) n);
         mean_ci.v_ncu[0] = mean_ci.v +
                            cv_normal_u*stdev_ci.v/sqrt((double) n);
      } // end else
   }

   return;
}

////////////////////////////////////////////////////////////////////////

void compute_i_mean_stdev(const NumArray &v_na,
                          int normal_ci_flag, double alpha, int skip,
                          CIInfo &mean_ci, CIInfo &stdev_ci) {
   int i, n, count;
   NumArray v_na_i, i_na_i;

   n = v_na.n_elements();

   if(skip < 0 || skip > n) {
      cerr << "\n\nERROR: compute_i_mean_stdev() -> "
           << "the skip index (" << skip << ") is out of bounds!\n\n"
           << flush;
      exit(1);
   }

   //
   // Copy over the array and index values except for the one to
   // be skipped
   //
   for(i=0, count=0; i<n; i++) {
      if(i == skip) continue;
      v_na_i.add(v_na[i]);
      i_na_i.add(count);
      count++;
   }

   compute_mean_stdev(v_na_i, i_na_i, normal_ci_flag, alpha,
                      mean_ci, stdev_ci);

   return;
}

////////////////////////////////////////////////////////////////////////

int is_precip_code(int gc) {
   int r;

   //
   // Check whether this grib code is a precipitation type:
   //    - precipitation rate
   //    - thunderstorm probability
   //    - total precipitation
   //    - large scale precipitation
   //    - convective precipitation
   //
   if(gc == prate_grib_code || gc == tstm_grib_code  ||
      gc == apcp_grib_code  || gc == ncpcp_grib_code ||
      gc == acpcp_grib_code) r = 1;
   else                      r = 0;

   return(r);
}

////////////////////////////////////////////////////////////////////////

