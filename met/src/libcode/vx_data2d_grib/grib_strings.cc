// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2019
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*

using namespace std;

///////////////////////////////////////////////////////////////////////////////

#include <cctype>
#include <cstdio>
#include <iostream>
#include <sstream>
#include <unistd.h>
#include <cstdlib>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <cmath>

#include "grib_classes.h"
#include "grib_strings.h"
#include "vx_math.h"
#include "vx_util.h"
#include "vx_log.h"
#include "vx_data2d.h"

///////////////////////////////////////////////////////////////////////////////

ConcatString get_grib_code_list_str(int k, int grib_code, int ptv)

{

   //  look up the name in the grib tables
   Grib1TableEntry tab;
   if( !GribTable.lookup_grib1(grib_code, ptv, tab) ){
      mlog << Error << "\nget_grib_code_list_str() - unrecognized GRIB1 code "
           << grib_code << " and/or table version " << ptv << "\n\n";
      exit(1);
   }

   //  return the requested field
   switch(k) {
      case 0:  return tab.full_name;      // GRIB Code Name
      case 1:  return tab.units;          // GRIB Code Unit
      case 2:  return tab.parm_name;      // GRIB Code Abbreviation
      default:
         mlog << Error << "\nget_grib_code_list_str() - unexpected value for k: "
              << k << "\n\n";
         exit(1);
   }

}

///////////////////////////////////////////////////////////////////////////////

ConcatString get_grib_code_name(int grib_code, int ptv)

{

   ConcatString str = get_grib_code_list_str(0, grib_code, ptv);

   return ( str );
}

///////////////////////////////////////////////////////////////////////////////

ConcatString get_grib_code_unit(int grib_code, int ptv)

{

   ConcatString str = get_grib_code_list_str(1, grib_code, ptv);

   return ( str );

}

///////////////////////////////////////////////////////////////////////////////

ConcatString get_grib_code_abbr(int grib_code, int ptv)

{

   ConcatString str = get_grib_code_list_str(2, grib_code, ptv);

   return ( str );

}

///////////////////////////////////////////////////////////////////////////////

ConcatString get_grib_level_list_str(int k, int grib_level)

{
   int i, match;
   ConcatString str = missing_str;

   for(i=0, match=-1; i<n_grib_level_list; i++) {

      if(grib_level == grib_level_list[i].level) { match = i; break; }

   } // end for i

   //
   // Check if we have a match
   //
   if(match > 0) {

      switch(k) {
         case(0): // GRIB Level Name
            str = grib_level_list[match].name;
            break;

         case(1): // GRIB Level Abbreviation
            str = grib_level_list[match].abbr;
            break;

         default:
            mlog << Error << "\nget_grib_level_list_str() -> "
                 << "unexpected value for k: " << k
                 << "\n\n";
            exit(1);
            break;

      }   //  switch

   }   //  if

   return ( str );

}

///////////////////////////////////////////////////////////////////////////////

ConcatString get_grib_level_name(int grib_level)

{

ConcatString str = get_grib_level_list_str(0, grib_level);

return ( str );

}

///////////////////////////////////////////////////////////////////////////////

ConcatString get_grib_level_abbr(int grib_level)

{

ConcatString str = get_grib_level_list_str(1, grib_level);

return ( str );

}

///////////////////////////////////////////////////////////////////////////////

ConcatString get_grib_level_str(int grib_level, unsigned char *level_info)

{

   int i, match;
   ConcatString str = missing_str;

   for(i=0, match=-1; i<n_grib_level_list; i++) {

      if(grib_level == grib_level_list[i].level) { match = i; break; }

   }

        if(match < 0)                        str = missing_str;
   else if(grib_level_list[match].flag == 0) str = grib_level_list[match].abbr;
   else if(grib_level_list[match].flag == 1) {

      str << cs_erase << grib_level_list[match].abbr << '_' << char2_to_int(level_info);

   } else if(grib_level_list[match].flag == 2) {

      str << cs_erase
          << grib_level_list[match].abbr << '_'
          << ((int) level_info[0])       << '_'
          << ((int) level_info[1]);

   }

   return ( str );
}

///////////////////////////////////////////////////////////////////////////////

int str_to_grib_code(const char *c)

{

   int gc = bad_data_int;

   //
   // Search all of the parameter table versions in order for a matching
   // GRIB code
   //
   if(is_bad_data(gc)) gc = str_to_grib_code(c, 2);
   if(is_bad_data(gc)) gc = str_to_grib_code(c, 128);
   if(is_bad_data(gc)) gc = str_to_grib_code(c, 129);
   if(is_bad_data(gc)) gc = str_to_grib_code(c, 130);
   if(is_bad_data(gc)) gc = str_to_grib_code(c, 131);
   if(is_bad_data(gc)) gc = str_to_grib_code(c, 133);
   if(is_bad_data(gc)) gc = str_to_grib_code(c, 140);
   if(is_bad_data(gc)) gc = str_to_grib_code(c, 141);

   return(gc);
}

///////////////////////////////////////////////////////////////////////////////

int str_to_grib_code(const char *c, int ptv)

{

   //  if the input string is numeric, interpret as a grib code
   if( check_reg_exp("[0-9]+", c) ) return atoi(c);

   //  look up the name in the grib tables
   int n_matches;
   Grib1TableEntry tab;
   if( !GribTable.lookup_grib1(c, ptv, bad_data_int,
                               default_grib1_center, default_grib1_subcenter,
                               tab, n_matches) )
      return bad_data_int;

   return tab.code;

}

///////////////////////////////////////////////////////////////////////////////

int str_to_grib_code(const char *c, int &pcode, double &pthresh_lo, double &pthresh_hi)

{

   int gc = bad_data_int;

   //
   // Search all of the parameter table versions in order for a matching
   // GRIB code
   //
   if(is_bad_data(gc)) gc = str_to_grib_code(c, pcode, pthresh_lo, pthresh_hi, 2);
   if(is_bad_data(gc)) gc = str_to_grib_code(c, pcode, pthresh_lo, pthresh_hi, 128);
   if(is_bad_data(gc)) gc = str_to_grib_code(c, pcode, pthresh_lo, pthresh_hi, 129);
   if(is_bad_data(gc)) gc = str_to_grib_code(c, pcode, pthresh_lo, pthresh_hi, 130);
   if(is_bad_data(gc)) gc = str_to_grib_code(c, pcode, pthresh_lo, pthresh_hi, 131);
   if(is_bad_data(gc)) gc = str_to_grib_code(c, pcode, pthresh_lo, pthresh_hi, 133);
   if(is_bad_data(gc)) gc = str_to_grib_code(c, pcode, pthresh_lo, pthresh_hi, 140);
   if(is_bad_data(gc)) gc = str_to_grib_code(c, pcode, pthresh_lo, pthresh_hi, 141);

   return(gc);
}

///////////////////////////////////////////////////////////////////////////////

int str_to_grib_code(const char *c, int &pcode,
                     double &pthresh_lo, double &pthresh_hi, int ptv) {
   int gc = bad_data_int;
   char tmp_str[512];
   char *ptr = (char *) 0, *save_ptr = (char *) 0;

   // Parse out strings of the form:
   //    PROB
   //    PROB(lo<string<hi)
   //    PROB(string>lo)
   //    PROB(string<hi)

   strcpy(tmp_str, c);

   // Retrieve the first token containing the GRIB code info
   if((ptr = strtok_r(tmp_str, "()", &save_ptr)) == NULL) {
      mlog << Error << "\nstr_to_grib_code() -> "
           << "problems parsing the string \""
           << c << "\".\n\n";
      exit(1);
   }

   // Get the GRIB code
   gc = str_to_grib_code(ptr, ptv);

   // Check for probability information
   if((ptr = strtok_r(NULL, "()", &save_ptr)) != NULL) {
      pcode = str_to_prob_info(ptr, pthresh_lo, pthresh_hi, ptv);
   }
   // No probability information specified
   else {
     pcode = bad_data_int;
     pthresh_lo = pthresh_hi = bad_data_double;
   }

   return(gc);
}

///////////////////////////////////////////////////////////////////////////////

int str_to_prob_info(const char *c, double &pthresh_lo, double &pthresh_hi,
                     int ptv) {
   int gc = bad_data_int, i, n_lt, n_gt;
   char tmp_str[512];
   char *ptr = (char *) 0, *save_ptr = (char *) 0;
   SingleThresh st;

   // Parse out strings of the form:
   //    lo<string<hi
   //    string>lo
   //    string<hi

   // Initialize
   pthresh_lo = pthresh_hi = bad_data_double;

   // Count the number of '<' or '>' characters
   for(i=0, n_lt=0, n_gt=0; i<(int)strlen(c); i++) {
      if(c[i] == '<') n_lt++;
      if(c[i] == '>') n_gt++;
   }
   strcpy(tmp_str, c);

   // Single inequality
   if(n_lt + n_gt == 1) {

      // Parse the GRIB code
      ptr = strtok_r(tmp_str, "<>", &save_ptr);
      gc  = str_to_grib_code(ptr, ptv);

      // Parse the threshold
      ptr = strtok_r(NULL, "<>", &save_ptr);
      if(n_lt > 0) pthresh_hi = atof(ptr);
      else         pthresh_lo = atof(ptr);
   }
   // Double inequality
   else if(n_lt + n_gt == 2) {

      // Parse the first threshold
      ptr = strtok_r(tmp_str, "<>", &save_ptr);
      if(n_lt > 0) pthresh_lo = atof(ptr);
      else         pthresh_hi = atof(ptr);

      // Parse the GRIB code
      ptr = strtok_r(NULL, "<>", &save_ptr);
      gc  = str_to_grib_code(ptr, ptv);

      // Parse the second threshold
      ptr = strtok_r(NULL, "<>", &save_ptr);
      if(n_lt > 0) pthresh_hi = atof(ptr);
      else         pthresh_lo = atof(ptr);
   }
   else {
      mlog << Error << "\nstr_to_prob_info() -> "
           << "problems parsing the string \""
           << c << "\".\n\n";
      exit(1);
   }

   return(gc);
}

///////////////////////////////////////////////////////////////////////////////
