// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2024
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

#include "parse_file_list.h"
#include "data2d_factory_utils.h"


////////////////////////////////////////////////////////////////////////


static const char python_str    [] = "python";
static const char file_list_str [] = "file_list";


////////////////////////////////////////////////////////////////////////


StringArray parse_file_list(const StringArray& in_list)

{

StringArray out_list;

   //
   //  check for an empty input list
   //

if ( in_list.n() == 0 )  {

   mlog << Error << "\nparse_file_list() -> "
        << "empty list!\n\n";

   exit(1);

}

   //
   //  assume a list of length one is an ascii file list
   //

if ( in_list.n() == 1 )  {

   out_list = parse_ascii_file_list(in_list[0].c_str());

}

   //
   //  if out_list is still empty, return the input file list
   //

if ( out_list.n() == 0 )  out_list = in_list;


return out_list;

}


////////////////////////////////////////////////////////////////////////


StringArray parse_ascii_file_list(const char * path)

{

ifstream f_in;
StringArray a;
GrdFileType file_type;
std::string file_name;
ConcatString list_str(file_list_str);
bool check_files_exist = true;

   //
   //  If the input is not a regular file, return an empty list
   //

if ( !is_regular_file(path) )  return a;

   //
   //  If the input is a known gridded data file, return an empty list
   //

if ( (file_type = grd_file_type(path)) != FileType_None )  {

   mlog << Debug(5) << "parse_ascii_file_list() -> "
        << "File \"" << path << "\" of type "
        << grdfiletype_to_string(file_type)
        << " is not an ASCII file list.\n";

   return a;
}

   //
   //  Open the input ascii file
   //

met_open(f_in, path);

if ( !f_in )  {

   mlog << Error << "\nparse_ascii_file_list() -> "
        << "can't open the ASCII file list \"" << path
        << "\" for reading\n\n";

   exit(1);

}

   //
   //  Read and store the file names
   //

int n_missing = 0;

const int max_missing = 10;

while(f_in >> file_name)  {

   //
   //  If the first entry is file_list, do not check
   //  that the following entries actually exist as
   //  files on the system
   //

   if( a.n() == 0 && list_str.comparecase(file_name.c_str()) == 0 ) {

      check_files_exist = false;

      continue;
   }

   //
   //  Add current string to the list of files
   //

   a.add(file_name.c_str());

   //
   //  Keep track of the number of missing files.
   //

   if ( check_files_exist ) {

      if ( !is_regular_file(file_name.c_str()) )  n_missing++;

      if ( n_missing >= max_missing )  break;

   }
}

//
//  Check for too many missing files:
//  - A small number of files that are all missing
//  - A large number of files with a least 10 missing
//

if ( check_files_exist )  {

   if ( ( a.n() <  max_missing && n_missing == a.n()       ) ||
        ( a.n() >= max_missing && n_missing >= max_missing ) )  { 

      mlog << Debug(5) << "parse_ascii_file_list() -> "
           << "File \"" << path << "\" is not an ASCII file list "
           << "since there are too many missing files.\n";

      a.clear();
   }
}

f_in.close();

return a;

}


////////////////////////////////////////////////////////////////////////
