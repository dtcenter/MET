// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2013
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
#include "filename_suffix.h"


////////////////////////////////////////////////////////////////////////


StringArray get_filenames(const StringArray & search_dirs, const StringArray & suffix)

{

int j;
const int N = search_dirs.n_elements();
StringArray a, b;
ConcatString cur_suffix;
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

      cur_suffix = filename_suffix(search_dirs[j]);
     
      if ( suffix.has(cur_suffix) )  a.add(search_dirs[j]);

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

      cur_suffix = filename_suffix(entry_path);

      if ( suffix.has(cur_suffix) )  a.add(entry_path);

   }

}   //  while

   //
   //  done
   //

closedir(directory);  directory = (DIR *) 0;

return ( a );

}


////////////////////////////////////////////////////////////////////////

