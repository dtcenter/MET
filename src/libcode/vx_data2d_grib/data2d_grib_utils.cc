// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2015
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

#include "data2d_grib_utils.h"
#include "angles.h"
#include "is_bad_data.h"
#include "vx_log.h"
#include "vx_math.h"


////////////////////////////////////////////////////////////////////////

extern bool is_prelim_match(const VarInfoGrib &, const GribRecord &);

////////////////////////////////////////////////////////////////////////

bool is_prelim_match(const VarInfoGrib & vinfo, const GribRecord & g)

{

int j, k, bms_flag, accum, lower, upper;
unixtime ut, init_ut, valid_ut;

int p_code;
double p_thresh_lo, p_thresh_hi;

Section1_Header *pds = (Section1_Header *) g.pds;

   //
   //  check ptv
   //

k = (int) (pds->ptv);

if ( vinfo.ptv() != k )  return ( false );

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
      //  time range indicator
      //

   if ( pds->tri != 4 )  return ( false );

      //
      //  check that the accumulation seconds match the limits
      //

   if ( lower != accum || upper != accum )  return ( false );
}

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
      //  lower probility threshold
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

bool is_exact_match(const VarInfoGrib & vinfo, const GribRecord & g)

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
     vinfo.level().type_num() != grib_type_num )  return ( false );

   //
   //  for non-accumulation intervals and specific record numbers,
   //  check if the levels match
   //

if ( ( vinfo.level().type() != LevelType_Accum     ) &&
     ( vinfo.level().type() != LevelType_RecNumber ) &&
     ( grib_lower != lower || grib_upper != upper ) )  return ( false );

   //
   //  done
   //

return ( true );

}


////////////////////////////////////////////////////////////////////////


bool is_range_match(const VarInfoGrib & vinfo, const GribRecord & g)

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
     vinfo.level().type_num() != grib_type_num )  return ( false );

   //
   //  for non-accumulation intervals and specific record number,
   //  check if the GRIB levels fall within the requested range
   //

if ( ( vinfo.level().type() != LevelType_Accum     ) &&
     ( vinfo.level().type() != LevelType_RecNumber ) &&
     ( grib_lower < lower || grib_upper > upper ) )  return ( false );

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
   double sec_per_fcst_unit;
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

