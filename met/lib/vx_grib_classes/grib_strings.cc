// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2007
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*

using namespace std;

///////////////////////////////////////////////////////////////////////////////

#include <cctype>
#include <iostream>
#include <sstream>
#include <unistd.h>
#include <cstdlib>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <cmath>

#include "vx_grib_classes/grib_classes.h"
#include "vx_grib_classes/grib_strings.h"

///////////////////////////////////////////////////////////////////////////////

extern void get_grib_code_list(int, int &, const GribCodeData *&);
extern void get_grib_code_list_str(int, int, int, char *);
extern void get_grib_level_list_str(int, int, char *);

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
            cerr << "\n\nERRROR: get_grib_code_list() -> "
                 << "unsupported GRIB parameter table version number of "
                 << ptv << ".  The supported version numbers are 2, 128, "
                 << "129, 130, 131, 133, 140, 141.\n\n" << flush;
            exit(1);
            break;
      }
   }

   return;
}

///////////////////////////////////////////////////////////////////////////////

void get_grib_code_list_str(int k, int grib_code, int ptv, char *str) {
   int i, match;
   int n_grib_code_list;
   const GribCodeData *grib_code_list;

   //
   // Retrieve the GRIB code table to be used
   //
   get_grib_code_list(grib_code, ptv, n_grib_code_list, grib_code_list);

   for(i=0, match=-1; i<n_grib_code_list; i++) {

      if(grib_code == grib_code_list[i].code) {
         match = i;
         break;
      }
   } // end for i

   //
   // Check if we have a match
   //
   if(match > 0) {

      switch(k) {
         case(0): // GRIB Code Name
            strcpy(str, grib_code_list[match].name);
            break;

         case(1): // GRIB Code Unit
            strcpy(str, grib_code_list[match].unit);
            break;

         case(2): // GRIB Code Abbreviation
            strcpy(str, grib_code_list[match].abbr);
            break;

         default:
            cerr << "\n\nERRROR: get_grib_code_list_str() -> "
                 << "unexpected value for k: " << k
                 << "\n\n" << flush;
            exit(1);
            break;
      }
   }
   else{
      strcpy(str, missing_str);
   }

   return;
}

///////////////////////////////////////////////////////////////////////////////

void get_grib_code_name(int grib_code, int ptv, char *str) {

   get_grib_code_list_str(0, grib_code, ptv, str);

   return;
}

///////////////////////////////////////////////////////////////////////////////

void get_grib_code_unit(int grib_code, int ptv, char *str) {

   get_grib_code_list_str(1, grib_code, ptv, str);

   return;
}

///////////////////////////////////////////////////////////////////////////////

void get_grib_code_abbr(int grib_code, int ptv, char *str) {

   get_grib_code_list_str(2, grib_code, ptv, str);

   return;
}

///////////////////////////////////////////////////////////////////////////////

void get_grib_level_list_str(int k, int grib_level, char *str) {
   int i, match;

   for(i=0, match=-1; i<n_grib_level_list; i++) {

      if(grib_level == grib_level_list[i].level) {
         match = i;
         break;
      }
   } // end for i

   //
   // Check if we have a match
   //
   if(match > 0) {

      switch(k) {
         case(0): // GRIB Level Name
            strcpy(str, grib_level_list[match].name);
            break;

         case(1): // GRIB Level Abbreviation
            strcpy(str, grib_level_list[match].abbr);
            break;

         default:
            cerr << "\n\nERRROR: get_grib_level_list_str() -> "
                 << "unexpected value for k: " << k
                 << "\n\n" << flush;
            exit(1);
            break;
      }
   }
   else{
      strcpy(str, missing_str);
   }

   return;
}

///////////////////////////////////////////////////////////////////////////////

void get_grib_level_name(int grib_level, char *str) {

   get_grib_level_list_str(0, grib_level, str);

   return;
}

///////////////////////////////////////////////////////////////////////////////

void get_grib_level_abbr(int grib_level, char *str) {

   get_grib_level_list_str(1, grib_level, str);

   return;
}

///////////////////////////////////////////////////////////////////////////////

void get_grib_level_str(int grib_level, unsigned char *level_info,
                        char *str) {
   int i, match;

   for(i=0, match=-1; i<n_grib_level_list; i++) {

      if(grib_level == grib_level_list[i].level) {
         match = i;
         break;
      }
   } // end for i

   if(match < 0) {
      strcpy(str, missing_str);
   }
   else if(grib_level_list[match].flag == 0) {
      strcpy(str, grib_level_list[match].abbr);
   }
   else if(grib_level_list[match].flag == 1) {
      sprintf(str, "%s_%i", grib_level_list[match].abbr,
              char2_to_int(level_info));
   }
   else if(grib_level_list[match].flag == 2) {
      sprintf(str, "%s_%i_%i", grib_level_list[match].abbr,
              (int) level_info[0], (int) level_info[1]);
   }
   else {
      strcpy(str, missing_str);
   }

   return;
}

///////////////////////////////////////////////////////////////////////////////

int str_to_grib_code(const char *c, int ptv) {
   int gc = -1;
   int i, l, is_numeric, n_grib_code_list;
   char c_upper[512];
   const GribCodeData *grib_code_list;

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

      if(gc == -1) {

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

int str_to_grib_code(const char *c) {
   int gc = -1;

   //
   // Search all of the parameter table versions in order for a matching
   // GRIB code
   //
   if(gc == -1) gc = str_to_grib_code(c, 2);
   if(gc == -1) gc = str_to_grib_code(c, 128);
   if(gc == -1) gc = str_to_grib_code(c, 129);
   if(gc == -1) gc = str_to_grib_code(c, 130);
   if(gc == -1) gc = str_to_grib_code(c, 131);
   if(gc == -1) gc = str_to_grib_code(c, 133);
   if(gc == -1) gc = str_to_grib_code(c, 140);
   if(gc == -1) gc = str_to_grib_code(c, 141);

   // Check if a matching GRIB code was found
   if(gc == -1) {
      cerr << "\n\nERROR: str_to_grib_code() -> "
           << "can't find GRIB code corresponding to string \""
           << c << "\".\n\n" << flush;
      exit(1);
   }

   return(gc);
}

///////////////////////////////////////////////////////////////////////////////
