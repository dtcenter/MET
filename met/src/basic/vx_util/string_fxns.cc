// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2019
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
   // Check if the major version numbers match.
   //
   ConcatString check_major(parse_version_major(check_version));
   ConcatString met_major(parse_version_major(met_version));
   return(check_major == met_major);
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


ConcatString parse_version(const char * version, const int ndots) {
   ConcatString cs;
   string s = version;
   int i, n;

   //
   // Parse the version string to the requested depth.
   //
   for(i=n=0; i<s.length(); i++) {
      if(s[i] == '.') n++;
      if(n == ndots) break;
   }

   cs = s.substr(0, i);

   return(cs);
}


////////////////////////////////////////////////////////////////////////


ConcatString parse_version_major(const char * version) {
   return(parse_version(version, 1));
}


////////////////////////////////////////////////////////////////////////


ConcatString parse_version_major_minor(const char * version) {
   return(parse_version(version, 2));
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
   if(!c) { delete [] temp_str;  temp_str = 0;  return(0); }
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

int regex_apply(const char* pat, int num_mat, const char* str, char** &mat)
{
   //  compile the regex pattern
   int rc = 0, num_act = 0, num_pmat = ( 0 == num_mat ? 1 : num_mat );
   regex_t *re = new regex_t;
   // regex_t * re = (regex_t *) malloc(sizeof(regex_t));
   regmatch_t pmatch[num_pmat];
   if( 0 != (rc = regcomp(re, pat, REG_EXTENDED)) ){
      regfree(re);  re = 0;
      mlog << Error << "\napply_regex - regcomp() error: " << rc << "\n\n";
      if ( re )  { delete re;  re = 0; }
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
   if ( re )  { delete re;  re = 0; }
   re = 0;
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
   ConcatString ret;
   if(!data) return(ret);

   string str = data;
   size_t pos = str.find( old );
   if( string::npos == pos ) return ret;
   str.replace(pos, strlen(old), repl);

   ret = str;
   return ret;
}

////////////////////////////////////////////////////////////////////////

ConcatString str_replace_all(const char* data, const char* old, const char* repl){
   ConcatString ret;
   if(!data) return(ret);

   string str = data;
   while( string::npos != str.find(old) ) str = str_replace(str.c_str(), old, repl);

   ret = str.c_str();
   return ret;
}

////////////////////////////////////////////////////////////////////////

ConcatString str_format(const char *fmt, ...){
   va_list vl;
   va_start(vl, fmt);
   char buf[max_str_len];
   int status = vsnprintf(buf, sizeof(buf), fmt, vl);
   va_end(vl);

   if( status >= max_str_len - 1 ){
      mlog << Error << "\nstr_format() - overwrote buffer\n\n";
      exit(1);
   }

   ConcatString ret = string(buf);
   return ret;
}

////////////////////////////////////////////////////////////////////////

ConcatString str_trim(const ConcatString str){
   string dat = str;
   while( ' ' == dat.at(0) )              dat.replace(0,              1, "");
   while( ' ' == dat.at(dat.size() - 1) ) dat.replace(dat.size() - 1, 1, "");

   ConcatString ret = dat;
   return ret;
}

////////////////////////////////////////////////////////////////////////

int parse_thresh_index(const char *col_name) {
   int i;
   const char *ptr = (const char *) 0;

   if((ptr = strrchr(col_name, '_')) != NULL) i = atoi(++ptr);
   else {
      mlog << Error << "\nparse_thresh_index() -> "
           << "unexpected column name specified: \""
           << col_name << "\"\n\n";
      exit(1);
   }

   return(i);
}

////////////////////////////////////////////////////////////////////////
