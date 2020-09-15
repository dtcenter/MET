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
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <dirent.h>
#include <errno.h>
#include <cstdio>
#include <limits.h>
#include <cmath>

#include "vx_cal.h"
#include "vx_log.h"

#include "get_filenames.h"
#include "string_fxns.h"


////////////////////////////////////////////////////////////////////////


static const char python_str    [] = "python";
static const char file_list_str [] = "file_list";


////////////////////////////////////////////////////////////////////////


StringArray get_filenames(const StringArray & search_dir_list,
                          const char * prefix, const char * suffix,
                          bool check_regular)

{

int j;
const int N = search_dir_list.n_elements();
StringArray a;
ConcatString cs, py_str(python_str);

for (j=0; j<N; ++j)  {

   // Check for python embedding commands
   if ( py_str.comparecase(search_dir_list[j].c_str()) == 0 ) {

      cs << cs_erase << python_str;
      j++;

      // Append the python script and any arguments
      while ( j < N )  {

         // Look for the next python command
         if ( py_str.comparecase(search_dir_list[j].c_str()) == 0 )  {
            j--;
            break;
         }
         cs << " " << search_dir_list[j];
         j++;
      }

      // Add full python command to the list
      a.add(cs);

   }
   else {

      a.add(get_filenames(string(search_dir_list[j]), prefix, suffix, check_regular));

   }

}

return ( a );

}

////////////////////////////////////////////////////////////////////////


StringArray get_filenames(const ConcatString & search_dir,
                          const char * prefix, const char * suffix,
                          bool check_regular)

{

StringArray a, b;
struct stat sbuf;

 if ( stat(search_dir.c_str(), &sbuf) < 0 )  {

   mlog << Warning << "\nget_filenames() -> "
        << "can't stat \"" << search_dir << "\"\n\n";

   return ( a );

}

if ( S_ISDIR(sbuf.st_mode) )  {

   //
   //  process directory
   //

   b = get_filenames_from_dir(search_dir.c_str(), prefix, suffix);

   a.add(b);

   b.clear();

} else if ( S_ISREG(sbuf.st_mode) )  {

   //
   //  process regular files and, if requested, enforce that the
   //  prefix and suffix match
   //

   if ( check_regular ) {

      //
      //  get the file name
      //

     const char * ptr = strrchr(search_dir.c_str(), '/');
     if ( !ptr )  ptr = search_dir.c_str();
      else         ++ptr;

      if ( check_prefix_suffix(ptr, prefix, suffix) )  {
         a.add(search_dir);
      }

   } else  {

      a.add(search_dir);

   }
}

return ( a );

}


////////////////////////////////////////////////////////////////////////


StringArray get_filenames_from_dir(const char * directory_path,
                                   const char * prefix,
                                   const char * suffix)

{

DIR * directory = (DIR *) 0;
struct dirent * entry = (struct dirent *) 0;
StringArray a, b;
char entry_path[PATH_MAX];
ConcatString regex;
struct stat sbuf;


directory = met_opendir(directory_path);

if ( !directory )  {

   mlog << Error << "\nget_filenames_from_dir() -> "
        << "can't open directory path \"" << directory_path << "\"\n\n";

   exit ( 1 );

}

while ( (entry = readdir(directory)) != NULL )  {

   if ( strcmp(entry->d_name, "." ) == 0 )  continue;
   if ( strcmp(entry->d_name, "..") == 0 )  continue;

   snprintf(entry_path, sizeof(entry_path), "%s/%s", directory_path, entry->d_name);

   if ( stat(entry_path, &sbuf) < 0 )  {

      mlog << Error << "\nget_filenames_from_dir() -> "
           << "can't stat \"" << entry_path << "\"\n\n";

      exit ( 1 );

   }

   if ( S_ISDIR(sbuf.st_mode) )  {

      b = get_filenames_from_dir(entry_path, prefix, suffix);

      a.add(b);

      b.clear();

   } else if ( S_ISREG(sbuf.st_mode) )  {

      if ( check_prefix_suffix(entry->d_name, prefix, suffix) )  {
         a.add(entry_path);
      }

   }

}   //  while

   //
   //  done
   //

closedir(directory);  directory = (DIR *) 0;

return ( a );

}


////////////////////////////////////////////////////////////////////////


bool check_prefix_suffix(const char * path,
                         const char * prefix, const char * suffix)

{

ConcatString regex;
bool keep = true;

   //
   //  check the prefix
   //

if ( keep && prefix ) {

   regex << cs_erase << "^" << prefix;

   keep = check_reg_exp(regex.c_str(), path);

}

   //
   //  check the suffix
   //

if ( keep && suffix ) {

   regex << cs_erase << suffix << "$";

   keep = check_reg_exp(regex.c_str(), path);

}

return(keep);

}


////////////////////////////////////////////////////////////////////////
