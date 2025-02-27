// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2023
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*

////////////////////////////////////////////////////////////////////////

#ifndef  __MET_FILE_HPP__
#define  __MET_FILE_HPP__

////////////////////////////////////////////////////////////////////////

template <typename T>

void copy_nc_data_as_double(double *to_array, const T *from_array,
                            const int x_slot, const int y_slot,
                            const int nx, const int ny,
                            double missing_value, double fill_value) {
   int offset = 0;

   if (x_slot > y_slot) {
      for (int y=0; y<ny; ++y) {
         int start_offset = y * nx;
         for (int x=0; x<nx; ++x) {
            auto value = (double) from_array[x + start_offset];
            if(is_eq(value, missing_value) || is_eq(value, fill_value))
               value = bad_data_double;
            to_array[offset++] = value;
         }
      }
   }
   else {
      for (int x=0; x<nx; ++x) {
         int start_offset = x * ny;
         for (int y=0; y<ny; ++y) {
            auto value = (double) from_array[y + start_offset];
            if(is_eq(value, missing_value) || is_eq(value, fill_value))
               value = bad_data_double;
            to_array[offset++] = value;
         }
      }
   }
}

////////////////////////////////////////////////////////////////////////

#endif   /*  __MET_FILE_HPP__  */

////////////////////////////////////////////////////////////////////////

