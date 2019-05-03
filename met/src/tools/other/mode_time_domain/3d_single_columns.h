// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2019
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*



////////////////////////////////////////////////////////////////////////


#ifndef  __3D_SINGLE_ATT_COLUMNS_H__
#define  __3D_SINGLE_ATT_COLUMNS_H__


////////////////////////////////////////////////////////////////////////


static const char * att_3d_single_cols [] = {


   "OBJECT_ID",

   "OBJECT_CAT",

   "CENTROID_X",
   "CENTROID_Y",
   "CENTROID_T",
   "CENTROID_LAT",
   "CENTROID_LON",

   "X_DOT",
   "Y_DOT",

   "AXIS_ANG",
   "VOLUME",

   "START_TIME",
   "END_TIME",

   "CDIST_TRAVELLED",

   "INTENSITY_10",
   "INTENSITY_25",
   "INTENSITY_50",
   "INTENSITY_75",
   "INTENSITY_90",

};


static const int n_3d_single_cols = sizeof(att_3d_single_cols)/sizeof(*att_3d_single_cols);


////////////////////////////////////////////////////////////////////////


#endif   /*  __3D_SINGLE_ATT_COLUMNS_H__  */


////////////////////////////////////////////////////////////////////////


