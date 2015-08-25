

////////////////////////////////////////////////////////////////////////


#ifndef  __3D_TXT_HEADER_H__
#define  __3D_TXT_HEADER_H__


////////////////////////////////////////////////////////////////////////


static const char * header_3d_cols [] = {


   "VERSION", 
   "MODEL", 
   "FCST_LEAD", 
   "FCST_VALID", 
   "FCST_T_DELTA", 
   "FCST_ACCUM", 
   "OBS_LEAD", 
   "OBS_VALID", 
   "OBS_T_DELTA", 
   "OBS_ACCUM", 
   "FCST_RAD", 
   "FCST_THR", 
   "OBS_RAD", 
   "OBS_THR", 
   "FCST_VAR", 
   "FCST_LEV", 
   "OBS_VAR", 
   "OBS_LEV", 

   // "OBJECT_ID", 
   // "OBJECT_CAT", 

};


static const int n_header_3d_cols = sizeof(header_3d_cols)/sizeof(*header_3d_cols);


////////////////////////////////////////////////////////////////////////


#endif   /*  __3D_TXT_HEADER_H__  */


////////////////////////////////////////////////////////////////////////


