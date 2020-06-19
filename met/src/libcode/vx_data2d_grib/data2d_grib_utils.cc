// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2020
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
#include <cmath>
#include <vx_data2d.h>

#include "data2d_grib_utils.h"
#include "angles.h"
#include "is_bad_data.h"
#include "vx_log.h"
#include "vx_math.h"


////////////////////////////////////////////////////////////////////////

static bool is_prelim_match( VarInfoGrib &, const GribRecord &);

////////////////////////////////////////////////////////////////////////

bool is_prelim_match( VarInfoGrib & vinfo, const GribRecord & g)

{

   int j, k, bms_flag, accum, lower, upper;
   int ptv, center, subcenter;
   int vinfo_ptv, vinfo_center, vinfo_subcenter;
   int ens_application, ens_type, ens_number, vinfo_ens_type;
   int vinfo_ens_number;
   unixtime ut, init_ut, valid_ut;

   int p_code, code_for_lookup= vinfo.field_rec ();
   double p_thresh_lo, p_thresh_hi;

   Section1_Header *pds = (Section1_Header *) g.pds;

   ConcatString field_name = vinfo.name();

   //clean up the code - exept for code 33 and 34 (UGRID and VGRID) and name WIND when we derive wind speed and direction
   bool is_lookup_for_wind_components = (vinfo.code () == ugrd_grib_code || vinfo.code () == vgrd_grib_code) && field_name == "WIND";
   if ( is_lookup_for_wind_components)
   {
      //set field_name to null to force lookup by code
      field_name.clear ();
      code_for_lookup = vinfo.code ();
   }
   vinfo.set_code (bad_data_int);
   vinfo.units ().clear ();
   vinfo.long_name ().clear ();

   //
   //  check ptv
   //

   ptv = (int) (pds->ptv);
   center = pds->center_id;
   subcenter = pds->sub_center;
   ens_application = pds -> ens_application;
   ens_number = pds -> ens_number;
   ens_type = pds -> ens_type;

   vinfo_ptv = vinfo.ptv();
   vinfo_center = vinfo.center();
   vinfo_subcenter = vinfo.subcenter();

   // If not specified, use the parameters from the current GRIB record
   if(is_bad_data(vinfo_ptv))       vinfo_ptv       = ptv;
   if(is_bad_data(vinfo_center))    vinfo_center    = center;
   if(is_bad_data(vinfo_subcenter)) vinfo_subcenter = subcenter;

   vinfo_ens_type      = ens_type;
   vinfo_ens_number    = ens_number;

   bool isEnsMatch = true;
   // compare with ens config parameters only if this record is NCEP ensemble
   if( ens_application == 1 && center == 7 && subcenter == 2 ){
      ConcatString vinfo_ens = vinfo.ens();
      if( !vinfo_ens.empty() ){
         if( vinfo_ens == conf_key_grib_ens_hi_res_ctl ){
            vinfo_ens_type = 1;
            vinfo_ens_number = 1;
         } else if( vinfo_ens == conf_key_grib_ens_low_res_ctl){
            vinfo_ens_type = 1;
            vinfo_ens_number = 2;
         } else {
            char sign = vinfo_ens.text()[0];
            if( sign == '+') {
               vinfo_ens_type = 3;
            } else if ( sign == '-' ) {
               vinfo_ens_type = 2;
            }
            char* ens_number_str  = new char[vinfo_ens.length() ];
            strncpy(ens_number_str, vinfo_ens.text()+1, (size_t) vinfo_ens.length());
            ens_number_str[vinfo_ens.length()-1] = (char) 0;
            //  if the  string is numeric
            if( check_reg_exp("^[0-9]*$", ens_number_str) ) vinfo_ens_number= atoi(ens_number_str);
            delete[] ens_number_str;
         }
         // if one of the parameters was not set - error
         if( is_bad_data(vinfo_ens_number) || is_bad_data(vinfo_ens_type) ){
            mlog << Error << "\nis_prelim_match() -> "
                 << "unrecognized GRIB_ens value '" << vinfo_ens
                 << "' should be '" << conf_key_grib_ens_hi_res_ctl
                 << "' or '" << conf_key_grib_ens_low_res_ctl
                 << "' or '+/-' followed by the number "  << "\n\n";
            exit(1);
         }
      }
      isEnsMatch = ( vinfo_ens_type == ens_type && vinfo_ens_number == ens_number );

   }

   // Check for matching parameters
   if ( vinfo_ptv       != ptv       ||
        vinfo_center    != center    ||
        vinfo_subcenter != subcenter ||
        !isEnsMatch )  return ( false );

   // if p_flag is 'on' (probability field) and the request name is set - get the real name of the field from the begining of a request name
   if( vinfo.p_flag () && !vinfo.req_name().empty())
   {
      field_name=vinfo.req_name().split ("(")[0];
   }

   // if it is one of APCP names - (APCP_Z0) - use 'APCP' only
   if ( check_reg_exp("^APCP_[0-9]*$", field_name.c_str()) )  field_name = "APCP";

   Grib1TableEntry tab;
   int tab_match = -1;

   // if the name is specified, use it
   if( !field_name.empty() ){

      //  look up the name in the grib tables
      if( !GribTable.lookup_grib1(field_name.c_str(), vinfo_ptv, code_for_lookup, vinfo_center, vinfo_subcenter, tab, tab_match) )
      {
         //  if did not find with params from the header - try default
         if( !GribTable.lookup_grib1(field_name.c_str(), default_grib1_ptv, code_for_lookup, default_grib1_center, default_grib1_subcenter, tab, tab_match) )
         {
            //  if the lookup still fails, then it's not a match
            return ( false );
         }

      }

   }

   //  if the field name is not specified, look for and use indexes
   else {

      //  if either the field name or the indices are specified, bail
      if( bad_data_int == vinfo_ptv || bad_data_int == code_for_lookup ){
         mlog << Error << "\nis_prelim_match() -> "
              << "either name or GRIB1_ptv and GRIB1_code must be "
              << "specified in field information\n\n";
         exit(1);
      }

      //  use the specified indexes to look up the field name
      if( !GribTable.lookup_grib1(code_for_lookup, vinfo_ptv, vinfo_center, vinfo_subcenter,tab) ){
         //if did not find with params from the header - try default
         if( !GribTable.lookup_grib1(code_for_lookup, default_grib1_ptv, default_grib1_center, default_grib1_subcenter, tab) )
         {
            mlog << Error << "\nis_prelim_match() -> "
                 << "no parameter found with matching GRIB1_ptv ("
                 << vinfo_ptv << ") " << "GRIB1_code ("
                 << vinfo.field_rec() << "). Use the MET_GRIB_TABLES "
                 << "environment variable to define custom GRIB tables.\n\n";
            exit(1);
         }
      }
   }
   vinfo.set_code      ( tab.code         );
   vinfo.set_units     ( tab.units.c_str());
   vinfo.set_long_name ( tab.full_name.c_str()    );

   //
   //  test the level type number, if specified
   //

if ( !is_bad_data(vinfo.level().type_num()) &&
     vinfo.level().type_num() != (int) pds->type ){

   mlog << Debug(4)
        << "For GRIB record number " << g.rec_num
        << ", the requested level type (" << vinfo.level().type_num()
        << ") does not match the current level type ("
        << (int) pds->type << ").\n";

   return ( false );
}

   //
   //  store lower and upper level values
   //

lower = nint ( vinfo.level().lower() );
upper = nint ( vinfo.level().upper() );

   //
   //  check for a specific record number, ensuring that
   //  the GRIB codes match
   //

if ( vinfo.level().type() == LevelType_RecNumber )  {

  if ( lower == g.rec_num &&
       vinfo.code() == g.gribcode() )  return ( true );
  else                                 return ( false );

}

   //
   //  gribcode
   //

if ( vinfo.code() != g.gribcode() )  return ( false );

   //
   //  parse timing info from pds
   //

read_pds(g, bms_flag, init_ut, valid_ut, accum);

   //
   //  init time
   //

ut = vinfo.init();

if ( ut != 0 && ut != init_ut )  return ( false );

   //
   //  valid time
   //

ut = vinfo.valid();

if ( ut != 0 && ut != valid_ut )  return ( false );

   //
   //  lead time
   //

k = vinfo.lead();

if ( k >= 0 && k != ( valid_ut - init_ut) )  return ( false );

   //
   //  accumulation interval
   //

if ( vinfo.level().type() == LevelType_Accum ) {

      //
      //  time range indicator of 4 for accumulation
      //

   if ( pds->tri != 4 )  return ( false );

      //
      //  check that the accumulation seconds match the limits
      //

   if ( lower != accum || upper != accum )  return ( false );
}

   //
   //  time range indicator
   //

if ( !is_bad_data(vinfo.tri()) && pds->tri != vinfo.tri() )  return ( false );

   //
   //  probability info
   //

if ( vinfo.p_flag() &&
     vinfo.p_code() > 0 &&
     ( !is_bad_data ( vinfo.p_thresh_lo().get_value() ) ||
       !is_bad_data ( vinfo.p_thresh_hi().get_value() ) ) ) {

      //
      //  parse probability info from pds
      //

   read_pds_prob ( g, p_code, p_thresh_lo, p_thresh_hi );

      //
      //  probabilistic gribcode
      //

   if ( vinfo.p_code() != p_code )  return ( false );

      //
      //  lower probabiility threshold
      //

   if ( !is_bad_data ( p_thresh_lo ) &&
        !is_eq ( vinfo.p_thresh_lo().get_value(), p_thresh_lo, loose_tol ) )  return ( false );

      //
      //  upper probability threshold
      //

   if ( !is_bad_data ( p_thresh_hi ) &&
        !is_eq ( vinfo.p_thresh_hi().get_value(), p_thresh_hi, loose_tol ) )  return ( false );

}

   //
   //  find the level information for this record
   //

for ( j=0; j<n_grib_level_list; ++j ) {
   if ( pds->type == grib_level_list[j].level ) break;
}
if ( j == n_grib_level_list )  return ( false );

   //
   //  check that the record level type is consistent with the
   //  requested level type
   //

if ( vinfo.level().type() == LevelType_Pres &&
     grib_level_list[j].type != 3 )  return ( false );

if ( vinfo.level().type() == LevelType_Vert &&
     grib_level_list[j].type != 2 )  return ( false );

   //
   //  done
   //

return ( true );

}

////////////////////////////////////////////////////////////////////////

bool is_exact_match( VarInfoGrib & vinfo, const GribRecord & g)

{

int lower, upper, grib_lower, grib_upper, grib_type_num;

   //
   //  check common logic
   //

if ( !is_prelim_match(vinfo, g) ) return ( false );

   //
   //  store requested lower and upper limits
   //

lower = nint ( vinfo.level().lower() );
upper = nint ( vinfo.level().upper() );

   //
   //  get the GRIB record's level values
   //

read_pds_level(g, grib_lower, grib_upper, grib_type_num);

   //
   //  check the level type number, if specified
   //

if ( !is_bad_data(vinfo.level().type_num()) &&
     vinfo.level().type_num() != grib_type_num )  {

   mlog << Debug(4)
        << "For GRIB record number " << g.rec_num
        << ", the requested level type (" << vinfo.level().type_num()
        << ") does not match the current level type ("
        << grib_type_num << ").\n";

   return ( false );

}

   //
   //  for non-accumulation intervals and specific record numbers,
   //  check if the levels match
   //

if ( ( vinfo.level().type() != LevelType_Accum     ) &&
     ( vinfo.level().type() != LevelType_RecNumber ) &&
     ( grib_lower != lower || grib_upper != upper ) )  {

   mlog << Debug(4)
        << "For GRIB record number " << g.rec_num
        << ", the requested level values (" << lower << " and " << upper
        << ") do not match the current level values "
        << "(" << grib_lower << " and " << grib_upper << ").\n";

   return ( false );

}

   //
   //  done
   //

return ( true );

}


////////////////////////////////////////////////////////////////////////


bool is_range_match( VarInfoGrib & vinfo, const GribRecord & g)

{

int lower, upper, grib_lower, grib_upper, grib_type_num;

   //
   //  check common logic
   //

if ( !is_prelim_match(vinfo, g) ) return ( false );

   //
   //  store requested lower and upper limits
   //

lower = nint ( vinfo.level().lower() );
upper = nint ( vinfo.level().upper() );

   //
   //  get the GRIB record's level values
   //

read_pds_level(g, grib_lower, grib_upper, grib_type_num);

   //
   //  check the level type number, if specified
   //

if ( !is_bad_data(vinfo.level().type_num()) &&
     vinfo.level().type_num() != grib_type_num )  {

   mlog << Debug(4)
        << "For GRIB record number " << g.rec_num
        << ", the requested level type (" << vinfo.level().type_num()
        << ") does not match the current level type ("
        << grib_type_num << ").\n";

   return ( false );
}
   //
   //  for non-accumulation intervals and specific record number,
   //  check if the GRIB levels fall within the requested range
   //

if ( ( vinfo.level().type() != LevelType_Accum     ) &&
     ( vinfo.level().type() != LevelType_RecNumber ) &&
     ( grib_lower < lower || grib_upper > upper ) )  {

   mlog << Debug(4)
        << "For GRIB record number " << g.rec_num
        << ", the requested level values (" << lower << " and " << upper
        << ") do not fall within the current range of level values "
        << "(" << grib_lower << " and " << grib_upper << ").\n";

   return ( false );
}

   //
   //  done
   //

return ( true );

}


////////////////////////////////////////////////////////////////////////


bool get_data_plane(const GribRecord & r, DataPlane & plane)

{

int j;
int x, y;
int count;
double value;
const int nx = r.nx;
const int ny = r.ny;
const int nxy = nx*ny;
const bool has_bms = r.bms_flag;
unixtime init_ut, valid_ut;
int bms_flag, accum;


plane.set_size(nx, ny);

count = 0;

for (j=0; j<nxy; ++j)  {

   r.TO.one_to_two(nx, ny, j, x, y);

   if ( has_bms )  {

      if ( r.bms_bit(j) )  value = r.data_value(count++);
      else                 value = bad_data_double;

   } else {

      value = r.data_value(j);

   }

   plane.set(value, x, y);

}   //  for j

   //
   //  store the times
   //

read_pds(r, bms_flag, init_ut, valid_ut, accum);

plane.set_init  ( init_ut );
plane.set_valid ( valid_ut );
plane.set_lead  ( (int) (valid_ut - init_ut) );
plane.set_accum ( accum );

   //
   //  done
   //

return ( true );

}


////////////////////////////////////////////////////////////////////////


void read_pds(const GribRecord &r, int &bms_flag,
              unixtime &init_ut, unixtime &valid_ut, int &accum) {
   double sec_per_fcst_unit = 0.0;
   unsigned char pp1[2];
   Section1_Header *pds = (Section1_Header *) 0;

   pds = (Section1_Header *) r.pds;

   //
   // Check PDS for flag for the presence of a GDS and BMS section
   //
   if(!(pds->flag & 128)) {
      mlog << Error << "\nread_pds() -> "
           << "No Grid Description Section present in the "
           << "grib record.\n\n";
      exit(1);
   }
   if(pds->flag & 64) bms_flag = true;

   //
   // Check PDS for the initialization time
   //
   init_ut = mdyhms_to_unix(pds->month, pds->day,
                            pds->year + (pds->century - 1)*100,
                            pds->hour, pds->minute, 0);

   //
   // Check PDS for time units
   //
   switch((int) pds->fcst_unit) {

      case 0: // minute
         sec_per_fcst_unit = sec_per_minute;
         break;

      case 1: // hour
         sec_per_fcst_unit = sec_per_hour;
         break;

      case 2: // day
         sec_per_fcst_unit = sec_per_day;
         break;

      case 3: // month
         sec_per_fcst_unit = sec_per_day*30.0;
         break;

      case 4: // year
         sec_per_fcst_unit = sec_per_day*365.0;
         break;

      case 5: // decade
         sec_per_fcst_unit = sec_per_day*365.0*10.0;
         break;

      case 6: // normal (30 years)
         sec_per_fcst_unit = sec_per_day*365.0*30.0;
         break;

      case 7: // century
         sec_per_fcst_unit = sec_per_day*365.0*100.0;
         break;

      case 10: // 3 hours
         sec_per_fcst_unit = sec_per_hour*3.0;
         break;

      case 11: // 6 hours
         sec_per_fcst_unit = sec_per_hour*6.0;
         break;

      case 12: // 12 hours
         sec_per_fcst_unit = sec_per_hour*12.0;
         break;

      case 13: // 15 minutes
         sec_per_fcst_unit = sec_per_minute*15.0;
         break;

      case 14: // 30 minutes
         sec_per_fcst_unit = sec_per_minute*30.0;
         break;

      case 254: // second
         sec_per_fcst_unit = 1.0;
         break;

      default:
         mlog << Error << "\nread_pds() -> "
              << "unexpected time unit of "
              << (int) pds->fcst_unit << ".\n\n";
         exit(1);
         break;
   }

   //
   // Set the valid and accumulation times based on the
   // contents of the time range indicator
   //
   switch((int) pds->tri) {

      case 0: // Valid time = init + p1
         valid_ut = (unixtime) (init_ut + pds->p1*sec_per_fcst_unit);
         accum = 0;
         break;

      case 1: // Valid time = init
         valid_ut = init_ut;
         accum = 0;
         break;

      case 2: // Valid time between init + p1 and init + p2
         valid_ut = (unixtime) (init_ut + pds->p2*sec_per_fcst_unit);
         accum = 0;
         break;

      case 3: // Average
         valid_ut = (unixtime) (init_ut + pds->p2*sec_per_fcst_unit);
         accum = 0;
         break;

      case 4: // Accumulation
         valid_ut = (unixtime) (init_ut + pds->p2*sec_per_fcst_unit);
         accum = nint((pds->p2 - pds->p1)*sec_per_fcst_unit);
         break;

      case 5: // Difference: product valid at init + p2
         valid_ut = (unixtime) (init_ut + pds->p2*sec_per_fcst_unit);
         accum = 0;
         break;

      case 6: // Average: reference time - P1 to reference time - P2
         valid_ut = (unixtime) (init_ut - pds->p2*sec_per_fcst_unit);
         accum = 0;
         break;

      case 7: // Average: reference time - P1 to reference time + P2
         valid_ut = (unixtime) (init_ut + pds->p2*sec_per_fcst_unit);
         accum = 0;
         break;

      case 10: // P1 occupies octets 19 and 20: product valid at init + p1
          pp1[0] = pds->p1;
          pp1[1] = pds->p2;
          valid_ut = (unixtime) (init_ut + char2_to_int(pp1)*sec_per_fcst_unit);
          accum = 0;
          break;

      case 51: // Climatological Mean Value
          valid_ut = init_ut;
          accum = 0;
          break;

      case 136: // Climatological Standard Deviation
          valid_ut = init_ut;
          accum = 0;
          break;

      default:
         mlog << Error << "\nread_pds() -> "
              << "unexpected time range indicator of "
              << (int) pds->tri << ".\n\n";
         exit(1);
         break;
   }

   return;
}

////////////////////////////////////////////////////////////////////////

void read_pds_prob(const GribRecord &r, int &p_code,
                   double &p_thresh_lo, double &p_thresh_hi) {
   int len;
   double t1, t2;

   Section1_Header *pds = (Section1_Header *) r.pds;

   // Initialize
   p_code = 0;
   p_thresh_lo = p_thresh_hi = bad_data_double;

   // Get the PDS length
   len = char3_to_int(pds->length);
   if(len < 60)  return;

   // Store the probability GRIB code
   p_code = r.pds[45];

   // Store the thresholds
   t1 = char4_to_dbl(&r.pds[47]);
   t2 = char4_to_dbl(&r.pds[51]);

   // Check the probability type
   if(     r.pds[46] == 1) p_thresh_hi = t1;
   else if(r.pds[46] == 2) p_thresh_lo = t2;
   else if(r.pds[46] == 3) {
      p_thresh_lo = t1;
      p_thresh_hi = t2;
   }

   return;
}

////////////////////////////////////////////////////////////////////////


void read_pds_level(const GribRecord & g, int &lower, int &upper, int &type)

{
int j;

Section1_Header *pds = (Section1_Header *) g.pds;

   //
   //  find the level information for this record
   //

type = (int) pds->type;

for ( j=0; j<n_grib_level_list; ++j ) {
   if ( type == grib_level_list[j].level ) break;
}

   //
   //  check that a level type was found
   //

if ( j == n_grib_level_list )  {
   mlog << Error << "\nread_pds_level() -> "
        << " can't find the level type for GRIB record!\n\n";
   exit(1);
}

   //
   //  compute level values based on level list flag
   //

if ( grib_level_list[j].flag == 0 || grib_level_list[j].flag == 1 )  {
   lower = char2_to_int(pds->level_info);
   upper = lower;
} else {
   lower = (int) pds->level_info[0];
   upper = (int) pds->level_info[1];
}

   //
   //  done
   //

return;

}


////////////////////////////////////////////////////////////////////////

