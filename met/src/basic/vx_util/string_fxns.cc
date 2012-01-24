// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2012
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
#include <regex.h>

#include "util_constants.h"
#include "string_fxns.h"
#include "vx_log.h"


////////////////////////////////////////////////////////////////////////


bool match_met_version(const char * check_version) {
   bool match = false;

   //
   // Check if the check version matches the first significant digit
   // of the MET version.
   //
   if(strncasecmp(check_version, met_version, strlen("Vn.") ) == 0)
      match = true;

   return(match);
}


////////////////////////////////////////////////////////////////////////


void check_met_version(const char * check_version) {

   //
   // Error if the check version does not match the MET version.
   //
   if(!(match_met_version(check_version))) {

      mlog << Error << "\n  check_met_version() -> "
           << "The version number listed in the config file ("
           << check_version << ") is not compatible with the current "
           << "version of the code (" << met_version << ").\n\n";
      exit(1);
   }

   return;
}


////////////////////////////////////////////////////////////////////////


const char * get_short_name(const char * path)

{

if ( !path )  return ( (const char *) 0 );

int j;
const char * short_name = (const char *) 0;


j = strlen(path) - 1;

while ( (j >= 0) && (path[j] != '/') )  --j;

++j;

short_name = path + j;



return ( short_name );

}


////////////////////////////////////////////////////////////////////////


void append_char(char *str, const char c)

{
   char *ptr;

   //
   // If the specified characater does not already exist at the
   // end of the string, add one.
   //
   ptr = str + strlen(str) - 1;

   if(*ptr != c) {
      *(++ptr) = c;
      *(++ptr) = 0;
   }
   else {
      *(++ptr) = 0;
   }

   return;
}


////////////////////////////////////////////////////////////////////////


void strip_char(char *str, const char c)

{
   char *ptr;

   //
   // If the specified character exists at the end of the string,
   // remove it.
   //
   ptr = str + strlen(str) - 1;

   if(*ptr == c) {
      *(ptr) = 0;
   }

   return;
}


////////////////////////////////////////////////////////////////////////


bool check_reg_exp(const char *reg_exp_str, const char *test_str)

{
   bool valid = false;
   regex_t buffer;
   regex_t *preg = &buffer;

   if( regcomp(preg, reg_exp_str, REG_EXTENDED*REG_NOSUB) != 0 ) {
      mlog << Error << "\n  \tcheck_reg_exp(char *, char *) -> "
           << "regcomp error for \""
           << reg_exp_str << "\" and \"" << test_str << "\"\n\n";

      exit(1);
   }

   if( regexec(preg, test_str, 0, 0, 0) == 0 ) { valid = true; }

   //
   // Free allocated memory.
   //
   regfree( preg );

   return(valid);
}


////////////////////////////////////////////////////////////////////////


int num_tokens(const char *test_str, const char *separator)

{
   int n;
   char *temp_str;
   char *c = (char *) 0;

   //
   // Check for an empty string
   //
   if(strlen(test_str) <= 0) return(0);

   //
   // Initialize the temp string for use in tokenizing
   //
   temp_str = new char[strlen(test_str) + 1];

   if(!temp_str) {
      mlog << Error << "\n  num_tokens() -> "
           << "memory allocation error\n\n";
      exit(1);
   }
   strcpy(temp_str, test_str);

   //
   // Compute the number of tokens in the string
   //
   c = strtok(temp_str, separator);

   //
   // Check for an empty string
   //
   if(!c) return(0);
   else   n = 1;

   //
   // Parse remaining tokens
   //
   while((c = strtok(0, separator)) != NULL) n++;

   if(temp_str) { delete [] temp_str; temp_str = (char *) 0; }

   return(n);
}


////////////////////////////////////////////////////////////////////////
//
// Search through list of prefixes provided to see if the string begins
// with any of them.  Return true for a match.
//
////////////////////////////////////////////////////////////////////////

bool has_prefix(const char **prefix_list, int n_prefix,
                const char *str) {
   int i;
   bool status = false;

   //
   // Check to see the string begins with any of the prefixes, using
   // case-insensitive matching.
   //
   for(i=0; i<n_prefix; i++) {
      if(strncasecmp(str, prefix_list[i], strlen(prefix_list[i])) == 0) {
         status = true;
         break;
      }
   }

   return(status);
}

////////////////////////////////////////////////////////////////////////

   //
   //  Does replace(met_base_str, MET_BASE) on the output string
   //

ConcatString replace_path(const char * path)

{

ConcatString s;

s = path;

s.replace(met_base_str, MET_BASE);

return ( s );

}


////////////////////////////////////////////////////////////////////////
