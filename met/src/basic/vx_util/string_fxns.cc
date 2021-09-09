// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2021
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
#include <libgen.h>
#include <limits.h>
#include <cmath>
#include <regex.h>

#include "util_constants.h"
#include "string_fxns.h"
#include "num_array.h"
#include "vx_cal.h"
#include "vx_log.h"


const bool ENHANCE_STR_APIS = false;

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

const char * short_name = (const char *) 0;

if ( path ) {
   int j;

   j = m_strlen(path) - 1;

   while ( (j >= 0) && (path[j] != '/') )  --j;

   ++j;

   short_name = path + j;

}

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
   ptr = str + m_strlen(str) - 1;

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
   ptr = str + m_strlen(str) - 1;

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
   const char *method_name = "num_tokens() -> ";

   //
   // Check for an empty string
   //
   if(!test_str) return(0);
   
   int buf_len = m_strlen(test_str);
   if(buf_len <= 0) return(0);

   //
   // Initialize the temp string for use in tokenizing
   //
   temp_str = m_strcpy2(test_str, method_name);
   if (temp_str) {

      //
      // Compute the number of tokens in the string
      //
      //c = strtok(temp_str1.c_str(), separator);
      c = strtok(temp_str, separator);

      //
      // Check for an empty string
      //
      if(!c) { delete [] temp_str;  temp_str = 0;  return(0); }
      else   n = 1;

      //
      // Parse remaining tokens
      //
      //
      while((c = strtok(0, separator)) != NULL) n++;

   }

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
      if(strncasecmp(str, prefix_list[i], m_strlen(prefix_list[i])) == 0) {
         status = true;
         break;
      }
   }

   return(status);
}

////////////////////////////////////////////////////////////////////////

int regex_apply(const char* pat, int num_mat, const char* str, char** &mat)
{
   const char *method_name = "regex_apply() ";
   //  compile the regex pattern
   int rc = 0, num_act = 0, num_pmat = ( 0 == num_mat ? 1 : num_mat );
   regex_t *re = new regex_t;
   regmatch_t pmatch[num_pmat];
   if(0 != (rc = regcomp(re, pat, REG_EXTENDED))){
      regfree(re);
      if( re ) { delete re; re = 0; }
      mlog << Error << "\n" << method_name << "- regcomp() error: " << rc << "\n\n";
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
            m_strcpy(mat[i], str_dat.substr(pmatch[i].rm_so, mat_len).data(),
                     method_name, "mat[i]");
         }
         mat[num_act] = NULL;

      }
   } else {
      mat = NULL;
   }

   regfree(re);
   if( re ) { delete re; re = 0; }
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
   str.replace(pos, m_strlen(old), repl);

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
   int i = 0;
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

int m_strlen(const char *str) {
   int str_len = 0;
   if (str) str_len = strlen(str);  // or use sizeof str;

   return str_len;
}

////////////////////////////////////////////////////////////////////////
// to_string is allocated.

void m_strcpy(char *to_str, const char *from_str, const char *method_name,
              const char *extra_msg) {

   if (ENHANCE_STR_APIS) {
      int str_len = sizeof to_str;
      m_strncpy(to_str, from_str, str_len, method_name, extra_msg);
   }
   else strcpy(to_str, from_str);

}

////////////////////////////////////////////////////////////////////////
// to_string is not allocated. Allocate to_string and return the to_string after copying

char *m_strcpy2(const char *from_str, const char *method_name, const char *extra_msg) {
   char *to_str = (char *) 0;
   if (from_str) {
      int str_len = m_strlen(from_str);

      to_str = new char[str_len + 1];

      if(!to_str) {
         mlog << Error << "\n" << method_name
              << "memory allocation error (m_strcpy)"
              << (extra_msg == 0 ? "" : extra_msg) << "\n\n";
         exit(1);
      }

      m_strcpy(to_str, from_str, method_name, extra_msg);
   }
   else {
      mlog << Error << "\n" << method_name 
           << " Do not copy the string because a from_string is NULL. " 
           << (extra_msg == 0 ? "" : extra_msg) << "\n\n";
   }

   return to_str;
}

////////////////////////////////////////////////////////////////////////

void m_strncpy(char *to_str, const char *from_str, const int buf_len,
               const char *method_name, const char *extra_msg) {
   if (!from_str){
      mlog << Warning << "\n" << method_name 
           << " Do not copy the string because a from_string is NULL. " 
           << (extra_msg == 0 ? "" : extra_msg) << "\n\n";
   }
   else if (!to_str){
      mlog << Warning << "\n" << method_name 
           << " Do not copy the string because a to_string is NULL. " 
           << (extra_msg == 0 ? "" : extra_msg) << "\n\n";
   }
   else {   // (from_str && to_str)
      int str_len = m_strlen(from_str);
      if (str_len > buf_len) str_len = buf_len;

      memset(to_str, 0, str_len);
      if (ENHANCE_STR_APIS) {
         string temp_str = from_str;
         temp_str.copy(to_str, str_len);
         to_str[str_len] = 0;

         // Kludge: The sizeof from_str is 8 when the filenames come from a python script
         if (strcmp(from_str, to_str)) {
            str_len = m_strlen(from_str);
            if (str_len > buf_len) str_len = buf_len;
            temp_str.copy(to_str, str_len);
            to_str[str_len] = 0;
         }
      }
      else {
         strncpy(to_str, from_str, str_len);
         to_str[str_len] = 0;
      }

      if (strcmp(from_str, to_str)) {
         mlog << Warning << "\n" << method_name
              << " truncated a string " << (extra_msg == 0 ? "" : extra_msg)
              << " from \"" << from_str << "\" to \"" << to_str << "\"\n\n";
      }
   }

}


////////////////////////////////////////////////////////////////////////
