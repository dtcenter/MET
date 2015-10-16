// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2015
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
#include "num_array.h"
#include "vx_cal.h"
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

      mlog << Error << "\ncheck_met_version() -> "
           << "The version number listed in the config file ("
           << check_version << ") is not compatible with the current "
           << "version of the code (" << met_version << ").\n\n";
      exit(1);
   }

   return;
}


////////////////////////////////////////////////////////////////////////


bool less_than_met_version(const char *str1, const char *str2) {

   // Assuming version number format "Vn.n"
   double v1 = atof(str1 + 1);
   double v2 = atof(str2 + 1); 

   return(v1 < v2);
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
   char *ptr = (char *) 0;

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
   char *ptr = (char *) 0;

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


int num_tokens(const char *test_str, const char *separator)

{
   int n;
   char *temp_str = (char *) 0;
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
      mlog << Error << "\nnum_tokens() -> "
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
// Does replace(met_base_str, MET_BASE) on the output string, first
// checking the MET_BASE environment variable.
//
////////////////////////////////////////////////////////////////////////

ConcatString replace_path(const char * path) {
   ConcatString s, met_base_val;
   char *ptr;

   // Initialize
   s = path;

   // Use the MET_BASE environment variable, if set.
   // Otherwise, use the compile-time value.
   if((ptr = getenv(met_base_str)) != NULL) met_base_val = ptr;
   else                                     met_base_val = MET_BASE;

   s.replace(met_base_str, met_base_val);

   return(s);
}

////////////////////////////////////////////////////////////////////////

int regex_apply(const char* pat, int num_mat, const char* str, char** &mat)
{
   //  compile the regex pattern
   int rc = 0, num_act = 0, num_pmat = ( 0 == num_mat ? 1 : num_mat );
   regex_t *re = new regex_t;
   regmatch_t pmatch[num_pmat];
   if( 0 != (rc = regcomp(re, pat, REG_EXTENDED)) ){
      mlog << Error << "\napply_regex - regcomp() error: " << rc << "\n\n";
      exit(1);
   }

   //  apply the pattern to the input string
   rc = regexec(re, str, num_pmat, pmatch, 0);

   //  if the match succeeded, build the data for return
   if( 0 == rc ){

      //  if no captures were requested, return the match status
      if( 1 > num_mat ){
         num_act = num_pmat;
      }

      //  otherwise, build a list of captured strings
      else {

         //  count the actual number of matches
         for(int i=0; i < num_mat; i++){ if( -1 != pmatch[i].rm_so ) num_act++; }

         //  store the matched strings in a null-terminated list
         string str_dat = str;
         mat = new char*[num_act + 1];
         for(int i=0; i < num_act; i++){
            int mat_len = pmatch[i].rm_eo - pmatch[i].rm_so;
            mat[i] = new char[mat_len + 1];
            strcpy(mat[i], str_dat.substr(pmatch[i].rm_so, mat_len).data());
         }
         mat[num_act] = NULL;

      }
   } else {
      mat = NULL;
   }

   regfree(re);
   return num_act;
}

////////////////////////////////////////////////////////////////////////

void regex_clean(char** &mat)
{
   if( !mat ) return;
   for(int i=0; mat[i] != NULL; i++) delete [] mat[i];
   delete [] mat;
   mat = NULL;
}

////////////////////////////////////////////////////////////////////////

ConcatString str_replace(const char* data, const char* old, const char* repl){
   string str = data;
   size_t pos = str.find( old );
   if( string::npos == pos ) return "";
   str.replace(pos, strlen(old), repl);

   ConcatString ret = str.data();
   return ret;
}

////////////////////////////////////////////////////////////////////////

ConcatString str_replace_all(const char* data, const char* old, const char* repl){
   string str = data;
   while( string::npos != str.find(old) ) str = str_replace(str.c_str(), old, repl);

   ConcatString ret = str.c_str();
   return ret;
}

////////////////////////////////////////////////////////////////////////

ConcatString str_format(const char *fmt, ...){
   va_list vl;
   va_start(vl, fmt);
   char buf[max_str_len];
   int status = vsprintf(buf, fmt, vl);
   va_end(vl);

   if( status >= max_str_len - 1 ){
      mlog << Error << "\nstr_format() - overwrote buffer\n\n";
      exit(1);
   }

   ConcatString ret = buf;
   return ret;
}

////////////////////////////////////////////////////////////////////////

ConcatString str_trim(const char *str){
   string dat = str;
   while( ' ' == dat.at(0) )              dat.replace(0,              1, "");
   while( ' ' == dat.at(dat.size() - 1) ) dat.replace(dat.size() - 1, 1, "");

   ConcatString ret = dat.c_str();
   return ret;
}
