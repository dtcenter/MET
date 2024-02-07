// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2024
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*

////////////////////////////////////////////////////////////////////////

#include <cstdio>
#include <iostream>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>

#include "temp_file.h"
#include "file_exists.h"
#include "vx_log.h"

using namespace std;

////////////////////////////////////////////////////////////////////////

ConcatString make_temp_file_name(const char *prefix, const char *suffix) {
   int i, pid;
   ConcatString s;
   const int max_tries = 1000;

   //
   // Retrieve the current process id
   //
   pid = (int) getpid();

   i = -1;

   do {
      i++;

      if(i > max_tries) {
         mlog << Error << "\nmake_temp_file_name() -> "
              << "failed to make temporary file name:\n"
              << s << "\n\n";
         exit(1);
      }

      //
      // Initialize
      //
      s.erase();

      //
      // Build the file name
      //
      if(prefix) s << prefix << '_';
      s << pid << '_' << i;
      if(suffix) s << '_' << suffix;

   } while(file_exists(s.c_str()));

   return s;
}

////////////////////////////////////////////////////////////////////////

void remove_temp_file(const ConcatString file_name) {
   int errno;

   //
   // Attempt to remove the file and print out any error message
   //
   const char *keep_temp = getenv("MET_KEEP_TEMP_FILE");
   if (nullptr != keep_temp
       && (0 == strcmp(keep_temp, "true") || 0 == strcmp(keep_temp, "yes"))) {
      mlog << Debug(2) << "The temporary file ("
           << file_name << ") was not deleted. Please remove it manually\n\n";
      return;
   }

   if((errno = remove(file_name.c_str())) != 0) {
      mlog << Error << "\nremove_temp_file() -> "
           << "can't delete temporary file: \""
           << file_name << "\" ("
           << strerror(errno) << ")\n\n";
      exit(1);
   }

   return;
}

////////////////////////////////////////////////////////////////////////

