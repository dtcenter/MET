// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2020
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*

////////////////////////////////////////////////////////////////////////
//
//   Filename:   pbtime.cc
//
//   Description:
//
//   Mod#   Date      Name           Description
//   ----   ----      ----           -----------
//
////////////////////////////////////////////////////////////////////////

using namespace std;

#include <cstdio>
#include <cstdlib>
#include <ctime>
#include <ctype.h>
#include <dirent.h>
#include <iostream>
#include <fstream>
#include <math.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include "vx_log.h"
#include "vx_pb_util.h"
#include "vx_cal.h"
#include "concat_string.h"
#include "temp_file.h"

////////////////////////////////////////////////////////////////////////

//
// Constants
//

static const char *program_name = "pbtime";
static int file_unit            = 11;

////////////////////////////////////////////////////////////////////////

extern "C" {
   void openpb_(const char *, int *);
   void ireadns_(int *, char *, int *);
}

////////////////////////////////////////////////////////////////////////

int main(int argc, char *argv[]) {
   int i_date;
   ConcatString pb_file, blk_file;
   unixtime ut;
   char hdr_typ[512], time_str[512];

   if(argc != 2) {
      cout << "Usage: " << program_name << " pbfile\n"
           << "\twhere \"pbfile\" is the PrepBufr file whose center "
           << "time should be dumped (required).\n"
           << flush;
      exit(1);
   }

   // Store the PREPBUFR file
   pb_file << argv[1];

   // Build the temporary block file name
   blk_file = make_temp_file_name("/tmp/tmp_pbtime_blk", NULL);

   // Block the PrepBufr file and open it for reading.
   pblock(pb_file.c_str(), blk_file.c_str(), block);

   // Open the blocked temp PrepBufr file for reading
   openpb_(blk_file.c_str(), &file_unit);

   // Read the first PrepBufr message
   ireadns_(&file_unit, hdr_typ, &i_date);

   // Format and dump out the time string
   snprintf(time_str, sizeof(time_str), "%.10i", i_date);
   ut = yyyymmddhh_to_unix(time_str);
   cout << unix_to_yyyymmdd_hhmmss(ut) << "\n" << flush;

   // Delete the temporary blocked file
   remove_temp_file(blk_file);

   return(0);
}

////////////////////////////////////////////////////////////////////////
