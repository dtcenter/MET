// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2011
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

///////////////////////////////////////////////////////////////////////////////

extern void get_grib_code_list(int, int &, const GribCodeData *&);
extern ConcatString get_grib_code_list_str(int, int, int);
extern ConcatString get_grib_level_list_str(int, int);

///////////////////////////////////////////////////////////////////////////////

void get_grib_code_list(int grib_code, int ptv,
                        int &n, const GribCodeData *&ptr) {

   //
   // Search the base strings for codes less than 128
   //
   if(grib_code < 128) {
      n   = n_grib_code_list_base;
      ptr = grib_code_list_base;
   }
   //
   // Search based on the paramter table version
   //
   else {
      switch(ptv) {

         case(2):
            n   = n_grib_code_list_ptv2;
            ptr = grib_code_list_ptv2;
            break;

         case(128):
            n   = n_grib_code_list_ptv128;
            ptr = grib_code_list_ptv128;
            break;

         case(129):
            n   = n_grib_code_list_ptv129;
            ptr = grib_code_list_ptv129;
            break;

         case(130):
            n   = n_grib_code_list_ptv130;
            ptr = grib_code_list_ptv130;
            break;

         case(131):
            n   = n_grib_code_list_ptv131;
            ptr = grib_code_list_ptv131;
            break;

         case(133):
            n   = n_grib_code_list_ptv133;
            ptr = grib_code_list_ptv133;
            break;

         case(140):
            n   = n_grib_code_list_ptv140;
            ptr = grib_code_list_ptv140;
            break;

         case(141):
            n   = n_grib_code_list_ptv141;
            ptr = grib_code_list_ptv141;
            break;

         default:
            mlog << Error << "\n\n  get_grib_code_list() -> "
                 << "unsupported GRIB parameter table version number of "
                 << ptv << ".  The supported version numbers are 2, 128, "
                 << "129, 130, 131, 133, 140, 141.\n\n";
            exit(1);
            break;
      }
   }

   return;
}

///////////////////////////////////////////////////////////////////////////////

ConcatString get_grib_code_list_str(int k, int grib_code, int ptv)

{

   int i, match;
   int n_grib_code_list;
   const GribCodeData * grib_code_list = (const GribCodeData *) 0;
   ConcatString str = missing_str;

   //
   // Retrieve the GRIB code table to be used
   //
   get_grib_code_list(grib_code, ptv, n_grib_code_list, grib_code_list);

   for(i=0, match=-1; i<n_grib_code_list; i++) {

      if(grib_code == grib_code_list[i].code) { match = i; break; }

   } // end for i

   //
   // Check if we have a match
   //
   if(match >= 0) {

      switch(k) {
         case(0): // GRIB Code Name
            str = grib_code_list[match].name;
            break;

         case(1): // GRIB Code Unit
            str = grib_code_list[match].unit;
            break;

         case(2): // GRIB Code Abbreviation
            str = grib_code_list[match].abbr;
            break;

         default:
            mlog << Error << "\n\n  get_grib_code_list_str() -> "
                 << "unexpected value for k: " << k
                 << "\n\n";
            exit(1);
            break;
      }

   }

   return ( str );
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
            mlog << Error << "\n\n  get_grib_level_list_str() -> "
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

   int gc = bad_data_int;
   int i, l, is_numeric, n_grib_code_list;
   char c_upper[512];
   const GribCodeData *grib_code_list = (const GribCodeData *) 0;

   // First check if this string contains a numeric grib code.
   // Check if each character in the string is a digit.
   is_numeric = 1;
   l = strlen(c);
   for(i=0; i<l; i++) {
      if(!isdigit(c[i])) {
         is_numeric = 0;
         break;
      }
   }

   if(is_numeric) {
      gc = atoi(c);
   }
   // If not numeric, search for a matching grib code abbreviation
   else{

      // Convert the input string to all upper case ignoring anything
      // after an underscore indicating an accumulation interval
      strcpy(c_upper, c);
      l = strlen(c_upper);
      for(i=0; i<l; i++) {
         if(c_upper[i] == '_') {
            c_upper[i] = '\0';
            break;
         }
         else {
            c_upper[i] = toupper(c_upper[i]);
         }
      }

      // Search for GRIB codes from 0 to 127
      get_grib_code_list(0, ptv, n_grib_code_list, grib_code_list);

      for(i=0; i<n_grib_code_list; i++) {
         if(strcmp(c_upper, grib_code_list[i].abbr) == 0) {
            gc = grib_code_list[i].code;
            break;
         }
      }

      if(is_bad_data(gc)) {

         // Search for GRIB codes from 128 to 255
         get_grib_code_list(128, ptv, n_grib_code_list, grib_code_list);

         for(i=0; i<n_grib_code_list; i++) {
            if(strcmp(c_upper, grib_code_list[i].abbr) == 0) {
               gc = grib_code_list[i].code;
               break;
            }
         }
      }
   }

   return(gc);
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
   char *ptr, *save_ptr;

   // Parse out strings of the form:
   //    PROB
   //    PROB(lo<string<hi)
   //    PROB(string>lo)
   //    PROB(string<hi)

   strcpy(tmp_str, c);

   // Retrieve the first token containing the GRIB code info
   if((ptr = strtok_r(tmp_str, "()", &save_ptr)) == NULL) {
      mlog << Error << "\n\n  str_to_grib_code() -> "
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
   char *ptr, *save_ptr;
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
      mlog << Error << "\n\n  str_to_prob_info() -> "
           << "problems parsing the string \""
           << c << "\".\n\n";
      exit(1);
   }

   return(gc);
}

///////////////////////////////////////////////////////////////////////////////
