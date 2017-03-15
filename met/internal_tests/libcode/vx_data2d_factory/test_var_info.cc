// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2017
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*

////////////////////////////////////////////////////////////////////////
//
//   Filename:   test_var_info.cc
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
#include "var_info_grib.h"
#ifdef WITH_GRIB2
  #include "var_info_grib2.h"
#endif
#include "var_info_nc_met.h"
#include "var_info_nc_pinterp.h"

#include "vx_log.h"

////////////////////////////////////////////////////////////////////////

int main(int argc, char *argv[]) {
   VarInfoFactory factory;
   VarInfo *vi_g1   = (VarInfo *) 0; // allocated, need to delete
   VarInfo *vi_g2   = (VarInfo *) 0; // allocated, need to delete
   VarInfo *vi_nc   = (VarInfo *) 0; // allocated, need to delete
   VarInfo *vi_user = (VarInfo *) 0; // allocated, need to delete
   int type;
   ConcatString type_str;

   // Create new objects
   vi_g1   = factory.new_var_info("FileType_Gb1");
   vi_g2   = factory.new_var_info("FileType_Gb2");
   vi_nc   = factory.new_var_info("FileType_NcMet");

   // Set up a GRIB1 object
   vi_g1->set_pair(CONFIG_FileType, "FileType_Gb1");
   vi_g1->set_pair(CONFIG_Init, "20100101_23");
   vi_g1->set_pair(CONFIG_Valid, "20100102_00");
   vi_g1->set_pair(CONFIG_Lead, "1");

   vi_g1->set_pair(CONFIG_GRIB_PTV, "129");
   vi_g1->set_pair(CONFIG_GRIB_Code, "212");
   vi_g1->set_pair(CONFIG_GRIB_LvlType, "201");

   //
   //  Randy:  commented out this line because CONFIG_GRIB_PPTV
   //
   //  seems no longer to be defined in var_info_grib.h
   //

   // vi_g1->set_pair(CONFIG_GRIB_PPTV, "2");
   vi_g1->set_pair(CONFIG_GRIB_PCode, "12");

   // Set up a GRIB2 object
   vi_g2->set_pair(CONFIG_FileType, "FileType_Gb2");
   vi_g2->set_pair(CONFIG_Init, "20120101_23");
   vi_g2->set_pair(CONFIG_Valid, "20120102_00");
   vi_g2->set_pair(CONFIG_Lead, "033000");

#ifdef WITH_GRIB2
   vi_g2->set_pair(CONFIG_GRIB2_Discipline, "1");
   vi_g2->set_pair(CONFIG_GRIB2_MTable, "2");
   vi_g2->set_pair(CONFIG_GRIB2_LTable, "3");
   vi_g2->set_pair(CONFIG_GRIB2_Tmpl, "4");
   vi_g2->set_pair(CONFIG_GRIB2_ParmCat, "5");
   vi_g2->set_pair(CONFIG_GRIB2_Parm, "6");
   vi_g2->set_pair(CONFIG_GRIB2_Process, "7");
   vi_g2->set_pair(CONFIG_GRIB2_EnsType, "8");
   vi_g2->set_pair(CONFIG_GRIB2_DerType, "9");
#endif

   // Set up a NetCDF object
   vi_nc->set_pair(CONFIG_FileType, "FileType_NcMet");
   vi_nc->set_pair(CONFIG_Init, "20120101_23");
   vi_nc->set_pair(CONFIG_Valid, "20120102_00");
   vi_nc->set_pair(CONFIG_Lead, "033000");

   vi_nc->set_pair(CONFIG_NetCDF_Dimension, "1");
   vi_nc->set_pair(CONFIG_NetCDF_Dimension, "2");
   vi_nc->set_pair(CONFIG_NetCDF_Dimension, "3");
   vi_nc->set_pair(CONFIG_NetCDF_Dimension, "4");
   vi_nc->set_pair(CONFIG_NetCDF_Dimension, "5");

   // Test out GRIB1 VarInfo
   cout << "\nvi_g1->VarInfo::dump(cout);\n";
   vi_g1->VarInfo::dump(cout);
   cout << "\nvi_g1->dump(cout);\n";
   vi_g1->dump(cout);

   // Test out GRIB2 VarInfo
   cout << "\nvi_g2->VarInfo::dump(cout);\n";
   vi_g2->VarInfo::dump(cout);
   cout << "\nvi_g2->dump(cout);\n";
   vi_g2->dump(cout);

   // Test out NetCDF VarInfo
   cout << "\nvi_nc->VarInfo::dump(cout);\n";
   vi_nc->VarInfo::dump(cout);
   cout << "\nvi_nc->dump(cout);\n";
   vi_nc->dump(cout);

   // Read magic strings from the user
   char name_str[1024], level_str[1024];
   strcpy(name_str,  " ");
   strcpy(level_str, " ");

   while(strlen(name_str) > 0) {

      // Delete it if it's been allocated
      if(vi_user) { delete vi_user; vi_user = (VarInfo *) 0; }

      // Determine file type to be tested
      cout << "\n##################################################################\n\n";
      cout << "Enter file type (0 to quit, 1 for GRIB1, 2 for GRIB2, 3 for NetCDF): ";
      cin  >> type;

      // Allocate based on the requested type
      switch(type) {

         case 0:
            cout << "Quitting...\n";
            exit(0);
            break;
         case 1:
            type_str = "FileType_Gb1";
            break;
         case 2:
            type_str = "FileType_Gb2";
            break;
         case 3:
            type_str = "FileType_NcMet";
            break;

         default:
            cerr << "\n\ntest_var_info: -> "
                 << "unexpected input type of " << type << "!\n\n" << flush;
            exit(1);
      }
      vi_user = factory.new_var_info(type_str);

      // Prompt user for string to parse
      cout << "\nEnter test " << type_str << " string: ";
      cin  >> name_str >> level_str;
      if(strlen(name_str) == 0) break;

      vi_user->clear();
      vi_user->set_pair("FileType", type_str);
      vi_user->set_magic(name_str, level_str);
      cout << "\nvi_user->VarInfo::dump(cout);\n";
      vi_user->VarInfo::dump(cout);
      cout << "\nvi_user->dump(cout);\n";
      vi_user->dump(cout);
   }

//    if(vi_g1)   { delete vi_g1;   vi_g1   = (VarInfo *) 0; }
//    if(vi_g2)   { delete vi_g2;   vi_g2   = (VarInfo *) 0; }
//    if(vi_nc)   { delete vi_nc;   vi_nc   = (VarInfo *) 0; }
   if(vi_user) { delete vi_user; vi_user = (VarInfo *) 0; }

   return(0);
}

////////////////////////////////////////////////////////////////////////
