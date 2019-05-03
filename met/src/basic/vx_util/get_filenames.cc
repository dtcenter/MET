// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2016
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

#include "vx_log.h"

#include "get_filenames.h"
#include "string_fxns.h"


////////////////////////////////////////////////////////////////////////


StringArray get_filenames(const StringArray & search_dirs, const StringArray & suffix)

{

int j;
const int N = search_dirs.n_elements();
StringArray a, b;
struct stat sbuf;


for (j=0; j<N; ++j)  {

   if ( stat(search_dirs[j], &sbuf) < 0 )  {

      mlog << Warning << "\nget_filenames() -> "
           << "can't stat \"" << search_dirs[j] << "\"\n\n";

      continue;

   }

   if ( S_ISDIR(sbuf.st_mode) )  {

      b = get_filenames_from_dir(search_dirs[j], suffix);

      a.add(b);

      b.clear();

   } else if ( S_ISREG(sbuf.st_mode) )  {

      //
      //  process any explicitly specified regular files
      //  regardless of suffix
      //
 
      a.add(search_dirs[j]);

   }

}


return ( a );

}


////////////////////////////////////////////////////////////////////////


StringArray get_filenames_from_dir(const char * directory_path, const StringArray & suffix)

{

DIR * directory = (DIR *) 0;
struct dirent * entry = (struct dirent *) 0;
StringArray a, b;
char entry_path[PATH_MAX];
ConcatString cur_suffix;
struct stat sbuf;


directory = opendir(directory_path);

if ( !directory )  {

   mlog << Error << "\nget_filenames_from_dir() -> "
        << "can't open directory path \"" << directory_path << "\"\n\n";

   exit ( 1 );

}

while ( (entry = readdir(directory)) != NULL )  {

   if ( strcmp(entry->d_name, "." ) == 0 )  continue;
   if ( strcmp(entry->d_name, "..") == 0 )  continue;

   sprintf(entry_path, "%s/%s", directory_path, entry->d_name);

   if ( stat(entry_path, &sbuf) < 0 )  {

      mlog << Error << "\nget_filenames_from_dir() -> "
           << "can't stat \"" << entry_path << "\"\n\n";

      exit ( 1 );

   }

   if ( S_ISDIR(sbuf.st_mode) )  {

      b = get_filenames_from_dir(entry_path, suffix);

      a.add(b);

      b.clear();

   } else if ( S_ISREG(sbuf.st_mode) )  {

      // check if the file name ends with one of the suffixes by applying a regular expression
      for( int i=0; i<= suffix.n_elements()-1; i++ ){
         char** mat = NULL;
         char regex[strlen(suffix[i]) +1];
         strcpy (regex,suffix[i]);
         strcat (regex,"$");
         if( 1 == regex_apply(regex, 1, entry_path, mat) ){
            a.add(entry_path);
            break;
         }

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


StringArray parse_ascii_file_list(const char * path)

{

ifstream f_in;
StringArray a;
char file_name[PATH_MAX];
   
   //
   //  Open the input ascii file
   //

f_in.open(path);
if(!f_in) {
   mlog << Error << "\nparse_ascii_file_list() -> "
        << "can't open the ASCII file list \"" << path
        << "\" for reading\n\n";
   exit(1);
}

   //
   //  Read and store the file names
   //

while(f_in >> file_name) a.add(file_name);

   //
   //  done
   //

f_in.close();

return(a);

}


////////////////////////////////////////////////////////////////////////
