// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2019
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*



////////////////////////////////////////////////////////////////////////


#ifndef  __3D_PAIR_ATT_COLUMNS_H__
#define  __3D_PAIR_ATT_COLUMNS_H__


////////////////////////////////////////////////////////////////////////


static const char * att_3d_pair_cols [] = {

   "OBJECT_ID", 

   "OBJECT_CAT", 

   "SPACE_CENTROID_DIST", 
   "TIME_CENTROID_DELTA", 

   "AXIS_DIFF", 

   "SPEED_DELTA", 
   "DIRECTION_DIFF", 

   "VOLUME_RATIO", 

   "START_TIME_DELTA", 
   "END_TIME_DELTA", 

   "INTERSECTION_VOLUME", 

   "DURATION_DIFF", 

   "INTEREST", 

};


static const int n_att_3d_pair_cols = sizeof(att_3d_pair_cols)/sizeof(*att_3d_pair_cols);


////////////////////////////////////////////////////////////////////////


#endif   /*  __3D_PAIR_ATT_COLUMNS_H__  */


////////////////////////////////////////////////////////////////////////


