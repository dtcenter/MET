// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2017
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*



////////////////////////////////////////////////////////////////////////


#ifndef  __2D_ATT_COLUMNS_H__
#define  __2D_ATT_COLUMNS_H__


////////////////////////////////////////////////////////////////////////


static const char * att_2d_cols [] = {


   "OBJ_ID",       

   "CLUSTER_ID",       

   "TIME_INDEX",       

   "AREA",       

   "CENTROID_X",       
   "CENTROID_Y",       
   "CENTROID_LAT",       
   "CENTROID_LON",       

   "AXIS_ANG",       

};


static const int n_2d_cols = sizeof(att_2d_cols)/sizeof(*att_2d_cols);


////////////////////////////////////////////////////////////////////////


#endif   /*  __2D_ATT_COLUMNS_H__  */


////////////////////////////////////////////////////////////////////////


