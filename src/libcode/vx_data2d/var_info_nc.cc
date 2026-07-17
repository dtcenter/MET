// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2026
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*

///////////////////////////////////////////////////////////////////////////////
//
//   Filename:   var_info_nc.cc
//
//   Description:
//
//   Mod#   Date      Name           Description
//   ----   ----      ----           -----------
//
///////////////////////////////////////////////////////////////////////////////

#include <map>
#include <stdlib.h>
#include <strings.h>

#include "var_info_nc.h"
#include "nc_constants.h"

using namespace std;

///////////////////////////////////////////////////////////////////////////////

static void check_dim_offset(const char *);

///////////////////////////////////////////////////////////////////////////////
//
//  Code for class VarInfoNC
//
///////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////

void VarInfoNC::clear_dimension() {
   Dimension.clear();
   Is_offset.clear();
   Dim_value.clear();
}

///////////////////////////////////////////////////////////////////////////////

void VarInfoNC::add_dimension(long dim, bool as_offset, double dim_value) {
   Dimension.add(dim);
   Is_offset.add(as_offset);
   Dim_value.add(dim_value);
   return;
}

///////////////////////////////////////////////////////////////////////////////

void VarInfoNC::parse_level(const ConcatString &level_str) {
   const char *method_name = "VarInfoNC::parse_level() -> ";

   if(level_str.empty()
       || level_str.string().find_first_of("(") == std::string::npos) {
      // Should be handled by the derived class
      clear_dimension();
      set_default_levels(level_str);
      return;
   }

   char *ptr = nullptr;
   char *ptr2 = nullptr;
   char *ptr3 = nullptr;
   char *save_ptr = nullptr;
   ConcatString tmp_str;

   // Initialize the temp string
   tmp_str = level_str;

   // Parse the level specification
   // Retreive the NetCDF level specification
   ptr = strtok_r((char*)tmp_str.c_str(), "()", &save_ptr);

   // Set the level name
   Level.set_req_name(ptr);
   Level.set_name(ptr);

   // If dimensions are specified, clear the default value
   if (strchr(ptr, ',') != nullptr) clear_dimension();

   // Parse the dimensions
   bool as_offset = true;
   while ((ptr2 = strtok_r(ptr, ",", &save_ptr)) != nullptr) {
      // Check for wildcards
      if (strchr(ptr2, '*') != nullptr) {
         add_dimension(vx_data2d_star);
      }
      else {
         as_offset = (*ptr2 != '@');
         if (!as_offset) ptr2++;

         // Check for a range of levels
         ptr3 = strchr(ptr2, '-');

         //skip negative sign of the negative value to check the range
         if (ptr3 != nullptr && ptr3 == ptr2) ptr3 = strchr((ptr2+1), '-');

         if (ptr3 != nullptr && ptr3 != ptr2) {
            // Store the dimension of the range and limits
            parse_vertical_range(ptr2, ptr3, as_offset, method_name);
         }
         // Check for a range of times
         else if ((ptr3 = strchr(ptr2, ':')) != nullptr) {
            parse_time_range(ptr2, ptr3, as_offset, method_name);
         }
         else {
            parse_single_level(ptr2, as_offset, method_name);
         }
      }

      // Set ptr to nullptr for next call to strtok
      ptr = nullptr;

   } // end while

}


///////////////////////////////////////////////////////////////////////////////

void VarInfoNC::parse_single_level(const char *lstr, bool as_offset, const char *caller) {
   // Single level
   long level = 0;
   double level_value = bad_data_double;

   if (is_datestring(lstr)) {
      unixtime unix_time = timestring_to_unix(lstr);
      level = vx_data2d_dim_by_value;
      level_value = (double) unix_time;
      as_offset = false;
   }
   else if (is_number(lstr)) {
      if (as_offset) {
         check_dim_offset(lstr);
         level = atoi(lstr);
      }
      else {
         level = vx_data2d_dim_by_value;
         level_value = atof(lstr);
      }
   }
   else {
      mlog << Error << "\n" << caller
           << "trouble parsing NetCDF dimension value \""
           << lstr << "\"!\n\n";
      exit(1);
   }
   if (as_offset) add_dimension(level, as_offset);
   else add_dimension(level, as_offset, level_value);
}

///////////////////////////////////////////////////////////////////////////////

void VarInfoNC::parse_time_range(const char *ptr_lower, char *ptr_upper,
                                 bool as_offset, const char *caller) {

   // Check if a range has already been supplied
   if (Dimension.has(range_flag)) {
      mlog << Error << "\n" << caller
           << "only one dimension can have a range for NetCDF variable \""
           << MagicStr << "\".\n\n";
      exit(1);
   }

   int increment = 0;
   // Store the dimension of the range and limits
   *ptr_upper = 0;
   ptr_upper++;

   if(*ptr_upper == '@') {
      if (as_offset) {
         mlog << Error << "\n" << caller
              << "Can not mix an offset and a value for NetCDF variable \""
              << MagicStr << "\".\n\n";
         exit(1);
      }
      else ptr_upper++;    // to support @time1:@time2
   }

   char *ptr_inc = strchr(ptr_upper, ':');
   if (ptr_inc != nullptr) {
      *ptr_inc = 0;
      ptr_inc++;
   }
   mlog << Debug(7) << caller
        << " start: " << ptr_lower << ", end: " << ptr_upper
        << ", as_offset: " << as_offset << "\n";

   bool datestring_start = is_datestring(ptr_lower);
   bool datestring_end   = is_datestring(ptr_upper);
   if (datestring_start != datestring_end) {
      mlog << Error << "\n" << caller
           << "the time value and an index/offset can not be mixed for NetCDF variable \""
           << MagicStr << "\".\n\n";
      exit(1);
   }

   // Parse the lower and upper time limits
   unixtime time_lower = 0;
   unixtime time_upper = 0;

   if (datestring_start && datestring_end) {
      as_offset = false;
      time_lower = timestring_to_unix(ptr_lower);
      time_upper = timestring_to_unix(ptr_upper);
   }
   else if (as_offset) {

      // Check for integer dimension offsets
      check_dim_offset(ptr_lower);
      check_dim_offset(ptr_upper);

      time_lower = (unixtime) atoi(ptr_lower);
      time_upper = (unixtime) atoi(ptr_upper);
   }
   else {
      time_lower = (unixtime) nint(atof(ptr_lower));
      time_upper = (unixtime) nint(atof(ptr_upper));
   }

   if (ptr_inc != nullptr) {
      if (as_offset) {
         increment = atoi(ptr_inc);
      }
      else {
         increment = is_float(ptr_inc)
                     ? nint(atof(ptr_inc))
                     : timestring_to_sec(ptr_inc);
         mlog << Debug(7) << caller
              << "increment: \"" << ptr_inc << "\" to "
              << increment << " seconds.\n";
      }
   }

   add_dimension(range_flag, as_offset);
   Level.set_lower((double) time_lower);
   Level.set_upper((double) time_upper);
   Level.set_increment(increment);

   // Assume time level type for a range of levels
   Level.set_type(LevelType_Time);
   Level.set_is_offset(as_offset);
}

///////////////////////////////////////////////////////////////////////////////

void VarInfoNC::parse_vertical_range(const char *ptr_lower, char *ptr_upper,
                                     bool as_offset, const char *caller) {
   const char *method_name = "VarInfoNC::parse_vertical_range() -> ";
  
   // Check if a range has already been supplied
   if (Dimension.has(range_flag)) {
      mlog << Error << "\n" << caller
           << "only one dimension can have a range for NetCDF variable \""
           << MagicStr << "\".\n\n";
      exit(1);
   }

   *ptr_upper = 0;
   ptr_upper++;
   mlog << Debug(4) << method_name
        << " start: " << ptr_lower << ", end: " << ptr_upper
        << ", as_offset: " << as_offset << "\n";

   add_dimension(range_flag, as_offset);
   if (*ptr_upper == '@') {
      if (as_offset) {
         mlog << Error << "\n" << caller
              << "Can not mix an offset and a value for NetCDF variable \""
              << MagicStr << "\".\n\n";
         exit(1);
      }
      ptr_upper++;    // to support @vlevel_lower-@vlevel_upper
   }
   if(as_offset) {
      // Check for integer dimension offsets
      check_dim_offset(ptr_lower);
      check_dim_offset(ptr_upper);

      int lower = atoi(ptr_lower);
      int upper = atoi(ptr_upper);
      if (lower > upper) {
         int tmp = lower;
         lower = upper;
         upper = tmp;
      }
      Level.set_lower(lower);
      Level.set_upper(upper);
   }
   else {
      float lower = atof(ptr_lower);
      float upper = atof(ptr_upper);
      if (lower > upper) {
         float tmp = lower;
         lower = upper;
         upper = tmp;
      }
      Level.set_lower(lower);
      Level.set_upper(upper);
      mlog << Debug(4) << method_name
           << " lower: " << lower << ", upper: " << upper << "\n";
   }

   // Assume pressure level type for a range of levels
   Level.set_type(LevelType_Pres);
   Level.set_is_offset(as_offset);
}

///////////////////////////////////////////////////////////////////////////////

void VarInfoNC::set_dimension(int i_dim, long dim) const {
   Dimension[i_dim] = dim;
   return;
}


///////////////////////////////////////////////////////////////////////////////

void VarInfoNC::set_magic_pre(const ConcatString &nstr, const ConcatString &lstr) {

   // Store the magic string
   VarInfo::set_magic(nstr, lstr);

   // Set the requested name and default output name
   set_req_name(nstr.c_str());
   set_name(nstr);

}

      
///////////////////////////////////////////////////////////////////////////////

void VarInfoNC::set_magic_post(const ConcatString &req_name, const ConcatString &level_name) {
   ConcatString tmp_str;
   
   // Check for "/PROB" to indicate a probability forecast
   if (strstr(MagicStr.c_str(), "/PROB") != nullptr) PFlag = true;

   // Set the long name
   tmp_str.format("%s(%s)", req_name.text(), level_name.text());
   set_long_name(tmp_str.c_str());

   // Set the units
   set_units(na_str);

}


///////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
//
// Begin miscellaneous utility functions
//
///////////////////////////////////////////////////////////////////////////////

static void check_dim_offset(const char *ptr) {

   if(!is_eq(atof(ptr), (double) atoi(ptr))) {
      mlog << Warning << "\ncheck_dim_offset() -> "
           << "Found non-integer NetCDF dimension index ("
           << ptr << " != " << atoi(ptr) << ").\n"
           << "Did you intend to use \"@" << ptr
           << "\" to specify the value for that dimension instead?\n\n";
   }

   return;
}

////////////////////////////////////////////////////////////////////////
