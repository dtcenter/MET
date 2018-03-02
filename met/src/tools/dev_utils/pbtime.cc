// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2018
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

// Constants used to interface to Fortran subroutines

// Maximum number of BUFR parameters
static const int mxr8pm         = 10;
// Maximum number of BUFR levels
static const int mxr8lv         = 255;
// Maximum number of BUFR event sequences
static const int mxr8vn         = 10;
// Maximum number of BUFR variable types
static const int mxr8vt         = 6;

// File unit number for opening the PrepBufr file
static int file_unit            = 11;

////////////////////////////////////////////////////////////////////////

// Shared memory defined in the PREPBC common block in readpb.prm
static double      hdr[mxr8pm];
static double      evns[mxr8vt][mxr8vn][mxr8lv][mxr8pm];
static int         nlev;

////////////////////////////////////////////////////////////////////////

extern "C" {
   void openpb_(const char *, int *);
   void readpb_(int *, char *, int *, int *, int *, double[mxr8pm],
                double[mxr8vt][mxr8vn][mxr8lv][mxr8pm]);
}

////////////////////////////////////////////////////////////////////////

int main(int argc, char *argv[]) {
   int i_date, i_ret;
   int mon, day, yr, hr, min, sec;
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
   blk_file = make_temp_file_name("/tmp/tmp_pbtime_blk", '\0');

   // Block the PrepBufr file and open it for reading.
   pblock(pb_file, blk_file, block);

   // Open the blocked temp PrepBufr file for reading
   openpb_(blk_file, &file_unit);

   // Read the first PrepBufr message
   readpb_(&file_unit, hdr_typ, &i_date, &i_ret, &nlev, hdr, evns);

   // Format and dump out the time string
   sprintf(time_str, "%.10i", i_date);
   ut = yyyymmddhh_to_unix(time_str);
   unix_to_mdyhms(ut, mon, day, yr, hr, min, sec);
   sprintf(time_str, "%.4i%.2i%.2i_%.2i%.2i%.2i", yr, mon, day, hr, min, sec);
   cout << time_str << "\n" << flush;

   // Delete the temporary blocked file
   remove_temp_file(blk_file);

   return(0);
}

////////////////////////////////////////////////////////////////////////
