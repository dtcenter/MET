// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2019
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*

////////////////////////////////////////////////////////////////////////
//
//   Filename:   test_factory.cc
//
//   Description:
//
//   Mod#   Date      Name           Description
//   ----   ----      ----           -----------
//
////////////////////////////////////////////////////////////////////////

using namespace std;

#include <stdlib.h>
#include <string.h>

#include "var_info_factory.h"
#include "data2d_factory.h"
#include "data_class.h"

#include "vx_log.h"

////////////////////////////////////////////////////////////////////////

int main(int argc, char *argv[]) {

   // Factory objects
   Met2dDataFileFactory mtddf_factory;
   VarInfoFactory       vinfo_factory;

   // Pointers for the current objects
   Met2dDataFile * mtddf_cur = (Met2dDataFile *) 0;
   VarInfo       * vinfo_cur = (VarInfo       *) 0;

   // DataPlane objects for storing the data
   DataPlane plane;
   DataPlaneArray plane_array;
   int i, n_lvl;
   double lower, upper, min, max;

   // Input parameters
   ConcatString in_file_name;
   std::string name_str, level_str;

   // Set verbosity level very high
   mlog.set_verbosity_level(10);

   // Read command line arguments
   if(argc != 2) {
      mlog << Error << "\nMust supply the name of a gridded data file to process.\n\n";
      exit(1);
   }

   // Store the input data file name
   in_file_name << argv[1];

   // Create an instance of this file type
   mtddf_cur = mtddf_factory.new_met_2d_data_file(in_file_name.c_str());
   if(mtddf_cur == (Met2dDataFile *) 0) {
      mlog << "\n\n  test_factory() -> "
           << "trouble reading input file \"" << in_file_name << "\"\n\n";
      exit(1);
   }

   // Dump the contents
   mlog << Debug(1) << "\nCALLING: mtddf_cur->Met2dDataFile::dump(cout);\n";
   mtddf_cur->Met2dDataFile::dump(cout);
   mlog << Debug(1) << "\nCALLING: mtddf_cur->dump(cout);\n";
   mtddf_cur->dump(cout);

   // Create a VarInfo object based on the gridded data file type
   vinfo_cur = vinfo_factory.new_var_info(mtddf_cur->file_type());

   mlog << Debug(1) << "\nCALLING: vinfo_cur->VarInfo::dump(cout);\n";
   vinfo_cur->VarInfo::dump(cout);

   mlog << Debug(1) << "\nCALLING: vinfo_cur->dump(cout);\n";
   vinfo_cur->dump(cout);

   // Prompt user for a magic string
   mlog << Debug(1) << "\n" << sep_str << "\n" << sep_str << "\n" << sep_str << "\n\n"
        << "Enter VarInfo name and level (Q to quit): ";
   cin  >> name_str >> level_str;

   // Process the user's magic strings
   while(strncasecmp(name_str.c_str(), "q", 1) != 0) {

      // Set up the VarInfo object
      vinfo_cur->clear();
      vinfo_cur->set_magic(name_str, level_str);

      // Dump the contents
      mlog << Debug(1) << "\nCALLING: vinfo_cur->VarInfo::dump(cout);\n";
      vinfo_cur->VarInfo::dump(cout);
      mlog << Debug(1) << "\nCALLING: vinfo_cur->dump(cout);\n";
      vinfo_cur->dump(cout);

      // Read a single data plane
      mlog << Debug(1) << "\n" << sep_str << "\n\n"
           << "CALLING: data_plane() to read a single record...\n";
      mtddf_cur->data_plane(*vinfo_cur, plane);

      // Dump information about the data plane
      mlog << Debug(1) << "\nCALLING: plane.dump(cout);\n";
      plane.dump(cout);
      plane.data_range(min, max);
      mlog << Debug(1) << "Data range = [" << min << ", " << max << "]\n";

      // Dump the contents
      mlog << Debug(1) << "\nCALLING: vinfo_cur->VarInfo::dump(cout);\n";
      vinfo_cur->VarInfo::dump(cout);
      mlog << Debug(1) << "\nCALLING: vinfo_cur->dump(cout);\n";
      vinfo_cur->dump(cout);

      // Read a range of data planes
      mlog << Debug(1) << "\n" << sep_str << "\n\n"
           << "CALLING: data_plane_array() to read multiple records...\n";
      n_lvl = mtddf_cur->data_plane_array(*vinfo_cur, plane_array);

      // Dump information about the data planes
      for(i=0; i<n_lvl; i++) {
         plane_array.levels(i, lower, upper);
         plane_array[i].data_range(min, max);
         mlog << Debug(1)
              << "[" << i+1
              << "] Level = [" << lower << ", " << upper
              << "], Range = [" << min << ", " << max << "]\n";
         plane_array[i].dump(cout);
      }

      // Dump the contents
      mlog << Debug(1) << "\nCALLING: vinfo_cur->VarInfo::dump(cout);\n";
      vinfo_cur->VarInfo::dump(cout);
      mlog << Debug(1) << "\nCALLING: vinfo_cur->dump(cout);\n";
      vinfo_cur->dump(cout);

      // Prompt user for a magic string
      mlog << Debug(1) << "\n" << sep_str << "\n" << sep_str << "\n" << sep_str << "\n\n"
           << "Enter VarInfo name and level string (Q to quit): ";
      cin  >> name_str >> level_str;

   } // end while loop

   // Clean up
   if(mtddf_cur) { delete mtddf_cur; mtddf_cur = (Met2dDataFile *) 0; }
   if(vinfo_cur) { delete vinfo_cur; vinfo_cur = (VarInfo       *) 0; }

   return(0);
}

////////////////////////////////////////////////////////////////////////
