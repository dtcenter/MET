// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2007
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*

///////////////////////////////////////////////////////////////////////////////

using namespace std;

#include <iostream>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <cmath>

#include "vx_met_util/vx_met_util.h"

///////////////////////////////////////////////////////////////////////////////

FileType check_magic_cookie     (const char *);
FileType check_grib_magic_cookie(const char *);
int      has_substr             (const char *, int, const char *);

///////////////////////////////////////////////////////////////////////////////
//
// Determine the file type for the file name indicated.  First, try
// case-insensitive matching of the file extension.  If no match is found,
// try matching the magic cookie.
//
///////////////////////////////////////////////////////////////////////////////

FileType get_file_type(const char *filename) {
   FileType f_type = NoFileType;
   const char *ptr = (char *) 0;
   char ext_str[max_str_len];
   int i;

   //
   // Check that the file exists
   //
   if(access(filename, F_OK) != 0) {
      cerr << "\n\nERROR: get_file_type() -> "
           << "can't access file \"" << filename
           << "\"\n\n" << flush;
      exit(1);
   }

   // Strip off the extension from the file name, if one exists, and compare
   // to the known file extensions.
   if((ptr = strrchr(filename, '.')) != NULL) {

      // Store the file extension
      strcpy(ext_str, ++ptr);

      // Convert the extension string to all lower case
      for(i=0; i<(int) strlen(ext_str); i++) {
         ext_str[i] = tolower(ext_str[i]);
      }

      // Check the GRIB file extensions
      if(f_type == NoFileType) {
         for(i=0; i<n_gb_file_ext; i++) {
            if(strcmp(ext_str, gb_file_ext[i]) == 0) {
               f_type = GbFileType;
               break;
            }
         }
      }

      // Check the NetCDF file extensions
      if(f_type == NoFileType) {
         for(i=0; i<n_nc_file_ext; i++) {
            if(strcmp(ext_str, nc_file_ext[i]) == 0) {
               f_type = NcFileType;
               break;
            }
         }
      }

      // Check the BUFR file extensions
      if(f_type == NoFileType) {
         for(i=0; i<n_bf_file_ext; i++) {
            if(strcmp(ext_str, bf_file_ext[i]) == 0) {
               f_type = BfFileType;
               break;
            }
         }
      }

   } // end if

   // If we haven't found the file type yet, check the magic cookie
   if(f_type == NoFileType) f_type = check_magic_cookie(filename);

   // Now do a better check specifically for GRIB
   if(f_type == NoFileType) f_type = check_grib_magic_cookie(filename);

   return(f_type);
}

///////////////////////////////////////////////////////////////////////////////

FileType check_magic_cookie(const char *filename) {
   FileType f_type = NoFileType;
   char buf[buf_size];

   // Open the file
   ifstream in(filename, ios::binary);
   if(!in) return(NoFileType);

   // Read first set of bytes into a buffer
   in.read(buf, sizeof(buf));

   // Compare the contents of the buffer to the magic cookies
   if     (has_substr(buf, buf_size, magic_cookie_str[0])) f_type = GbFileType;
   else if(has_substr(buf, buf_size, magic_cookie_str[1])) f_type = NcFileType;
   else if(has_substr(buf, buf_size, magic_cookie_str[2])) f_type = BfFileType;

   // Close the file
   in.close();

   return(f_type);
}

///////////////////////////////////////////////////////////////////////////////

FileType check_grib_magic_cookie(const char *filename) {
   FileType f_type = NoFileType;
   char buf[100];

   // Open the file
   ifstream in(filename);
   if(!in) return(NoFileType);

   // Read through the file looking for the GRIB magic cookie
   while(in) {

      // If not at the beginning of the file, back up 5 spaces
      if(in.tellg() > 5) in.seekg(-5, ios::cur);

      // Read the next chunk into the buffer
      in.read(buf, sizeof(buf));

      // Check the buffer for the GRIB magic cookie
      if(has_substr(buf, 100, magic_cookie_str[0])) {
         f_type = GbFileType;
         break;
      }
   }

   // Close the file
   in.close();

   return(f_type);
}

///////////////////////////////////////////////////////////////////////////////
//
// Searches the string str1 out to the specified length ignoring any NULL
// characters for an occurence of str2.  If found, return 1. If not found,
// return 0.
//
///////////////////////////////////////////////////////////////////////////////

int has_substr(const char *str1, int len, const char *str2) {
   int i, v;
   char *search_str = (char *) 0;

   // Allocate space for a copy of str1
   search_str = new char [len];

   // Make a copy of str1, replacing all NULL characters with a space
   for(i=0; i<len; i++) {
      if(str1[i] == '\0') search_str[i] = ' ';
      else                search_str[i] = str1[i];
   }

   // Call the strstr routine to check for any instances of str2 in search_str
   if(strstr(search_str, str2) == NULL) v = 0;
   else                                 v = 1;

   // Delete allocated memory
   if(search_str) { delete search_str; search_str = (char *) 0; }

   return(v);
}

///////////////////////////////////////////////////////////////////////////////
