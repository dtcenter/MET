

////////////////////////////////////////////////////////////////////////


   //
   //  Warning:
   //
   //     This file is machine generated.
   //
   //     Do not edit by hand.
   //
   //
   //     Created by enum_to_string from file "affine.h"
   //
   //     on July 7, 2010   8:41 am MDT
   //


////////////////////////////////////////////////////////////////////////


#ifndef  __VIEWGRAVITY_TO_STRING_H__
#define  __VIEWGRAVITY_TO_STRING_H__


////////////////////////////////////////////////////////////////////////


#include "vx_math/affine.h"


////////////////////////////////////////////////////////////////////////


extern void viewgravity_to_string(const ViewGravity, char * out);


extern bool string_to_viewgravity(const char *, ViewGravity &);


////////////////////////////////////////////////////////////////////////


   //
   //  minimum string length needed to hold output values from
   //
   //    the above function ... includes trailing nul
   //


static const int max_enum_viewgravity_len = 23;


////////////////////////////////////////////////////////////////////////


static const int n_enum_viewgravitys = 10;


////////////////////////////////////////////////////////////////////////


static const ViewGravity enum_viewgravity_array [10] = {

   view_center_gravity,       //  # 0
   view_north_gravity,        //  # 1
   view_south_gravity,        //  # 2
   view_east_gravity,         //  # 3
   view_west_gravity,         //  # 4

   view_northwest_gravity,    //  # 5
   view_northeast_gravity,    //  # 6
   view_southwest_gravity,    //  # 7
   view_southeast_gravity,    //  # 8
   no_view_gravity            //  # 9

};


////////////////////////////////////////////////////////////////////////


#endif   /*  __VIEWGRAVITY_TO_STRING_H__  */


////////////////////////////////////////////////////////////////////////


