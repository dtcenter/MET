

////////////////////////////////////////////////////////////////////////


#ifndef  __3D_SINGLE_ATT_COLUMNS_H__
#define  __3D_SINGLE_ATT_COLUMNS_H__


////////////////////////////////////////////////////////////////////////


static const char * att_3d_single_cols [] = {


   "CENTROID_X", 
   "CENTROID_Y", 
   "CENTROID_T", 
   "CENTROID_LAT", 
   "CENTROID_LON ", 

   "X_DOT", 
   "Y_DOT", 

   "AXIS_ANG", 
   "VOLUME", 

   "START_TIME", 
   "END_TIME", 

   "INTENSITY_10", 
   "INTENSITY_25", 
   "INTENSITY_50", 
   "INTENSITY_75", 
   "INTENSITY_90", 
   "INTENSITY_50", 
   "INTENSITY_SUM", 

};


static const int n_3d_single_cols = sizeof(att_3d_single_cols)/sizeof(*att_3d_single_cols);


////////////////////////////////////////////////////////////////////////


#endif   /*  __3D_SINGLE_ATT_COLUMNS_H__  */


////////////////////////////////////////////////////////////////////////


