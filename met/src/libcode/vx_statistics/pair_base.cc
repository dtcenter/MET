// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2016
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*

////////////////////////////////////////////////////////////////////////

using namespace std;

#include <cstdio>
#include <iostream>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <cmath>

#include "pair_base.h"

#include "vx_util.h"
#include "vx_grid.h"
#include "vx_data2d.h"
#include "vx_data2d_grib.h"
#include "vx_math.h"
#include "vx_log.h"

////////////////////////////////////////////////////////////////////////
//
// Code for class PairBase
//
////////////////////////////////////////////////////////////////////////

PairBase::PairBase() {
   init_from_scratch();
}

////////////////////////////////////////////////////////////////////////

PairBase::~PairBase() {
   clear();
}

////////////////////////////////////////////////////////////////////////

void PairBase::init_from_scratch() {

   clear();

   return;
}

////////////////////////////////////////////////////////////////////////

void PairBase::clear() {

   msg_typ.clear();
   mask_name.clear();

   mask_dp_ptr  = (DataPlane *)   0;  // Not allocated
   mask_sid_ptr = (StringArray *) 0;  // Not allocated

   interp_mthd = InterpMthd_None;
   interp_dpth = bad_data_int;

   sid_sa.clear();
   lat_na.clear();
   lon_na.clear();
   x_na.clear();
   y_na.clear();
   vld_ta.clear();
   lvl_na.clear();
   elv_na.clear();
   o_na.clear();
   o_qc_sa.clear();
   cmn_na.clear();
   csd_na.clear();
   wgt_na.clear();

   n_obs = 0;

   fcst_ut = 0;

   check_unique = false;
   check_single = false;

   map_unique.clear();
   map_single.clear();

   return;
}

////////////////////////////////////////////////////////////////////////

void PairBase::set_mask_name(const char *c) {

   mask_name = c;

   return;
}

////////////////////////////////////////////////////////////////////////

void PairBase::set_mask_dp_ptr(DataPlane *dp_ptr) {

   mask_dp_ptr = dp_ptr;

   return;
}

////////////////////////////////////////////////////////////////////////

void PairBase::set_mask_sid_ptr(StringArray *sid_ptr) {

   mask_sid_ptr = sid_ptr;

   return;
}

////////////////////////////////////////////////////////////////////////

void PairBase::set_msg_typ(const char *c) {

   msg_typ = c;

   return;
}

////////////////////////////////////////////////////////////////////////

void PairBase::set_interp_mthd(const char *str) {

   interp_mthd = string_to_interpmthd(str);

   return;
}

////////////////////////////////////////////////////////////////////////

void PairBase::set_interp_mthd(InterpMthd m) {

   interp_mthd = m;

   return;
}

////////////////////////////////////////////////////////////////////////

void PairBase::set_interp_dpth(int n) {

   interp_dpth = n;

   return;
}

////////////////////////////////////////////////////////////////////////

void PairBase::set_fcst_ut(unixtime ut){

   fcst_ut = ut;

   return;
}

////////////////////////////////////////////////////////////////////////

void PairBase::set_check_unique(bool check){

   check_unique = check;

   return;
}

////////////////////////////////////////////////////////////////////////

void PairBase::set_check_single(bool check){

   check_single = check;

   return;
}

////////////////////////////////////////////////////////////////////////

int PairBase::has_obs_rec(const char *sid,
                          double lat, double lon,
                          double x, double y,
                          double lvl, double elv, int &i_obs) {
   int i, status = 0;

   //
   // Check for an existing record of this observation
   //
   for(i=0, i_obs=-1; i<n_obs; i++) {

      if(strcmp(sid_sa[i], sid) == 0 &&
         is_eq(lat_na[i], lat) &&
         is_eq(lon_na[i], lon) &&
         is_eq(lvl_na[i], lvl) &&
         is_eq(elv_na[i], elv)) {
         status = 1;
         i_obs = i;
         break;
      }
   } // end for

   return(status);
}

////////////////////////////////////////////////////////////////////////

bool PairBase::add_obs(const char *sid,
                       double lat, double lon, double x, double y,
                       unixtime ut, double lvl, double elv,
                       double o, const char *qc,
                       double cmn, double csd, double wgt) {

   if( check_unique ){

      //  build a uniqueness test key
      string unq_key = str_format("%.4f:%.4f:%d:%.2f:%.2f:%.4f",
            lat,       //  lat
            lon,       //  lon
            ut,        //  valid time
            lvl,       //  level
            elv,       //  elevation
            o).text(); //  obs value

      //  add the station id to the reporting map
      if( 3 <= mlog.verbosity_level() )
         map_unique_sid.insert( pair<string,string>(unq_key, sid) );

      //  if the key is already present in the map, do not add the obs
      if( 0 < map_unique.count(unq_key) ) return false;

      //  add the key to the map
      map_unique[unq_key] = n_obs;

   } else if( check_single ){

      //  build a uniqueness test key
      string sng_key = str_format("%.3f:%.3f:%.2f:%.2f",
            lat,         //  lat
            lon,         //  lon
            lvl,         //  level
            elv).text(); //  elevation

      //  add a single value reporting string to the reporting map
      if( 3 <= mlog.verbosity_level() ){
         string sng_val = str_format("%s:%d:%.4f",
               sid,       //  station id
               ut,        //  valid time
               o).text(); //  obs value
         map_single_val.insert( pair<string,string>(sng_key, sng_val) );
      }

      //  if the key is not present in the duplicate map, add it to the map
      if( 1 > map_single.count(sng_key) ){
         map_single[sng_key] = str_format("%d:%d", n_obs, labs(fcst_ut - ut));
      }

      //  if the key is present, use the one with the closest valid time to the forecast
      else {

         //  parse the single duplicate value string
         char** mat = NULL;
         string sng_val = map_single[sng_key];
         if( 3 != regex_apply("^([0-9]+):([0-9]+)$", 3, sng_val.c_str(), mat) ){
            mlog << Error << "\nPairBase::add_obs() - regex_apply failed to parse '"
                 << map_single[sng_key].c_str() << "'\n\n";
            exit(1);
         }

         int obs_idx = atoi(mat[1]);
         int ut_diff = atoi(mat[2]);
         regex_clean(mat);

         //  if the current observation is closer to the forecast valid time, use it instead
         if( ut_diff > labs(fcst_ut - ut) ){
            sid_sa.set(obs_idx, sid);
            x_na  .set(obs_idx, x);
            y_na  .set(obs_idx, y);
            vld_ta.set(obs_idx, ut);
            o_na  .set(obs_idx, o);

            map_single[sng_key] = str_format("%d:%d", obs_idx, labs(fcst_ut - ut));
         }

         return false;
      }

   }

   sid_sa.add(sid);
   lat_na.add(lat);
   lon_na.add(lon);
   x_na.add(x);
   y_na.add(y);
   vld_ta.add(ut);
   lvl_na.add(lvl);
   elv_na.add(elv);
   o_na.add(o);
   o_qc_sa.add(qc);
   cmn_na.add(cmn);
   csd_na.add(csd);
   wgt_na.add(wgt);

   // Increment the number of pairs
   n_obs += 1;

   return true;
}

////////////////////////////////////////////////////////////////////////

void PairBase::print_duplicate_report(){

   /*
    * * * *  Unique duplicates  * * * *
    */
   if( check_unique ){

      if( 3 > mlog.verbosity_level() || ! map_unique_sid.size() ) return;

      //  iterate over the keys in the unique station id map
      mlog << Debug(3) << "\nDuplicate point observations for -unique setting:\n";
      for( map<string,int>::iterator it_unique = map_unique.begin();
           it_unique != map_unique.end(); ++it_unique ){

         //  parse and format the unique key string
         char** mat = NULL;
         const char* pat = "^([^:]+):([^:]+):([^:]+):([^:]+):([^:]+):([^:]+)$";
         string key_unique_sid = (*it_unique).first;
         if( 7 != regex_apply(pat, 7, key_unique_sid.c_str(), mat) ){
            mlog << Error << "\nPairBase::print_duplicate_report() - regex_apply failed "
                 << "to parse '" << key_unique_sid.c_str() << "'\n\n";
            exit(1);
         }
         string msg_key = str_format(
                             "[lat: %s  lon: %s  valid: %s  lev: %s  elv: %s  ob_val: %s]",
                             mat[1], mat[2], unix_to_yyyymmdd_hhmmss( atoi(mat[3]) ).text(),
                             mat[4], mat[5], mat[6]).text();
         regex_clean(mat);

         string msg = "    " + msg_key + " - Station IDs: ";

         //  print the station ids for the current key
         int num_sid = 0;
         pair<multimap<string,string>::iterator, multimap<string,string>::iterator> unique_sid;
         unique_sid = map_unique_sid.equal_range(key_unique_sid);
         for( multimap<string,string>::iterator it_sid = unique_sid.first;
              it_sid != unique_sid.second; ++it_sid)
            msg += (num_sid++ ? ", " : "") + (*it_sid).second;

         if( 1 < num_sid ) mlog << Debug(3) << msg.c_str() << "\n";

      }

      mlog << Debug(3) << "\n";

   }


   /*
    * * * *  Single duplicates  * * * *
    */
   else if( check_single ){

      if( 3 > mlog.verbosity_level() || ! map_single_val.size() ) return;

      //  iterate over the keys in the unique station id map
      mlog << Debug(3) << "\nDuplicate point observations for -single setting:\n";
      for( map<string,string>::iterator it_single = map_single.begin();
            it_single != map_single.end(); ++it_single ){

         int num_val = 0;
         string key_single_val = (*it_single).first;

         //  parse the single key string
         char** mat = NULL;
         if( 5 != regex_apply("^([^:]+):([^:]+):([^:]+):([^:]+)$", 5, key_single_val.c_str(), mat) ){
            mlog << Error << "\nPairBase::print_duplicate_report() - regex_apply failed "
                 << "to parse '" << key_single_val.c_str() << "'\n\n";
            exit(1);
         }
         string msg_key = str_format("[lat: %s  lon: %s  lev: %s  elv: %s]",
                                     mat[1], mat[2], mat[3], mat[4]).text();
         regex_clean(mat);

         //  parse the single key value
         if( 3 != regex_apply("^([^:]+):([^:]+)$", 3, (*it_single).second.c_str(), mat) ){
            mlog << Error << "\nPairBase::print_duplicate_report() - regex_apply failed "
                 << "to parse '" << (*it_single).second.c_str() << "'\n\n";
            exit(1);
         }
         string msg_val = str_format("%s (HHMMSS)", sec_to_hhmmss( atoi(mat[2]) ).text()).text();
         regex_clean(mat);

         string msg = "  " + msg_key + " - used point ob with valid time offset of " + msg_val;

         //  parse and print the point obs information for the current key
         pair<multimap<string,string>::iterator, multimap<string,string>::iterator> single_val;
         single_val = map_single_val.equal_range(key_single_val);
         for( multimap<string,string>::iterator it_val = single_val.first;
              it_val != single_val.second; ++it_val, num_val++){

            if( 4 != regex_apply("^([^:]+):([^:]+):([^:]+)$", 4, (*it_val).second.c_str(), mat) ){
               mlog << Error << "\nPairBase::print_duplicate_report() - regex_apply failed "
                    << "to parse '" << (*it_single).second.c_str() << "'\n\n";
               exit(1);
            }
            string msg_ob = str_format("[sid: %6s  vld: %s  ob_val: %8s]",
                                        mat[1], unix_to_yyyymmdd_hhmmss( atoi(mat[2]) ).text(), mat[3]).text();
            regex_clean(mat);
            msg += "\n    " + msg_ob;

         }

         if( 1 < num_val ) mlog << Debug(3) << msg.c_str() << "\n\n";

      }

      mlog << Debug(3) << "\n";

   }

}

////////////////////////////////////////////////////////////////////////

void PairBase::add_obs(double x, double y, double o,
                       double cmn, double csd, double wgt) {

   sid_sa.add(na_str);
   lat_na.add(bad_data_double);
   lon_na.add(bad_data_double);
   x_na.add(x);
   y_na.add(y);
   vld_ta.add(bad_data_int);
   lvl_na.add(bad_data_double);
   elv_na.add(bad_data_double);
   o_na.add(o);
   o_qc_sa.add(na_str);
   cmn_na.add(cmn);
   csd_na.add(csd);
   wgt_na.add(wgt);

   // Increment the number of observations
   n_obs += 1;

   return;
}

////////////////////////////////////////////////////////////////////////

void PairBase::set_obs(int i_obs, const char *sid,
                       double lat, double lon, double x, double y,
                       unixtime ut, double lvl, double elv,
                       double o, const char *qc,
                       double cmn, double csd, double wgt) {

   if(i_obs < 0 || i_obs >= n_obs) {
      mlog << Error << "\nPairBase::set_obs() -> "
           << "range check error: " << i_obs << " not in (0, "
           << n_obs << ").\n\n"
          ;
      exit(1);
   }

   sid_sa.set(i_obs, sid);
   lat_na.set(i_obs, lat);
   lon_na.set(i_obs, lon);
   x_na.set(i_obs, x);
   y_na.set(i_obs, y);
   vld_ta.set(i_obs, ut);
   lvl_na.set(i_obs, lvl);
   elv_na.set(i_obs, elv);
   o_na.set(i_obs, o);
   o_qc_sa.set(i_obs, qc);
   cmn_na.set(i_obs, cmn);
   csd_na.set(i_obs, csd);
   wgt_na.set(i_obs, wgt);

   return;
}

////////////////////////////////////////////////////////////////////////

void PairBase::set_obs(int i_obs, double x, double y,
                       double o, double cmn, double csd, double wgt) {

   if(i_obs < 0 || i_obs >= n_obs) {
      mlog << Error << "\nPairBase::set_obs() -> "
           << "range check error: " << i_obs << " not in (0, "
           << n_obs << ").\n\n"
          ;
      exit(1);
   }

   sid_sa.set(i_obs, na_str);
   lat_na.set(i_obs, bad_data_double);
   lon_na.set(i_obs, bad_data_double);
   x_na.set(i_obs, x);
   y_na.set(i_obs, y);
   vld_ta.set(i_obs, bad_data_int);
   lvl_na.set(i_obs, bad_data_double);
   elv_na.set(i_obs, bad_data_double);
   o_na.set(i_obs, o);
   o_qc_sa.set(i_obs, na_str);
   cmn_na.set(i_obs, cmn);
   csd_na.set(i_obs, csd);
   wgt_na.set(i_obs, wgt);

   return;
}

////////////////////////////////////////////////////////////////////////
