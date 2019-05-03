// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2012
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*

////////////////////////////////////////////////////////////////////////
//
//   Filename:   test_vx_data2d_grib.cc
//
//   Description:
//
//   Mod#   Date      Name           Description
//   ----   ----      ----           -----------
//
////////////////////////////////////////////////////////////////////////

using namespace std;

#include <limits.h>
#include <stdlib.h>
#include <string.h>

#include "var_info_grib.h"
#include "data2d_grib.h"
#include "data2d_grib_utils.h"

#include "vx_log.h"

////////////////////////////////////////////////////////////////////////

int main(int argc, char *argv[]) {
   VarInfo * vinfo = (VarInfo *) 0;
   MetGrib1DataFile gb_file;

   GribRecord gb_rec;
   char varinfo_str[1024];
   ConcatString gb_file_name;
   
   DataPlane plane;
   DataPlaneArray plane_array;
   int i;
   double lower, upper;

   mlog.set_verbosity_level(10);
   
   // Read command line arguments
   if(argc != 2) {
      mlog << Debug(1) << "Must supply the name of a GRIB file to process.\n";
      exit(1);
   }

   // Store the input GRIB file name
   gb_file_name << argv[1];

   // Open the GRIB file
   if(!(gb_file.open(gb_file_name))) {
      cerr << "\n\nERROR: test_vx_data2d_grib() -> "
           << "can't open GRIB file: "
           << gb_file_name << "\n\n";
      exit(1);
   }

   // Allocate VarInfoGrib object
   vinfo = new VarInfoGrib;

   // Prompt user for string to parse
   mlog << Debug(1) << "Enter GRIB1 VarInfo magic string (Q to quit): ";
   cin  >> varinfo_str;

   while(strncasecmp(varinfo_str, "q", 1) != 0) {

      mlog << Debug(1) << "\n" << sep_str << "\n\n";

      vinfo->clear();
      vinfo->set_pair("FileType", "FileType_Gb1");
      vinfo->set_magic(varinfo_str);
      mlog << Debug(1) << "\nvinfo->VarInfo::dump(cout);\n";
      vinfo->VarInfo::dump(cout);
      mlog << Debug(1) << "\nvinfo->dump(cout);\n";
      vinfo->dump(cout);

      // Read a single data plane
      mlog << Debug(1) << "\n" << sep_str << "\n\n" << "Calling data_plane() to read a single record...\n";
      gb_file.data_plane(*vinfo, plane);

      // Read a range of data planes
      mlog << Debug(1) << "\n" << sep_str << "\n\n" << "Calling data_plane_array() to read multiple records...\n";
      gb_file.data_plane_array(*vinfo, plane_array);
      mlog << Debug(1) << "Levels:\n";
      for(i=0; i<plane_array.n_planes(); i++) {
         plane_array.levels(i, lower, upper);
         mlog << Debug(1) << "[" << i+1 << "] " << lower << ", " << upper << "\n";
      }

      // Prompt user for the next string to parse
      mlog << Debug(1) << "\n" << sep_str << "\n\n" << "Enter GRIB1 VarInfo magic string (Q to quit): ";
      cin  >> varinfo_str;
   }

   // Close the GRIB file
   gb_file.close();

   // Deallocate VarInfo object
   if(vinfo) { delete vinfo; vinfo = (VarInfo *) 0; }

   return(0);
}

////////////////////////////////////////////////////////////////////////
