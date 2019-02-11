// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2019
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*


//////////////////////////////////////////////////////////////////


#ifndef  __ANGLES_H__
#define  __ANGLES_H__


//////////////////////////////////////////////////////////////////


extern double rescale_lon(double);

extern double rescale_deg(double, double, double);

extern double angle_between(double, double);

extern double angle_difference(double, double, double, double);


extern double convert_u_v_to_wdir(double, double);

extern double convert_u_v_to_wind(double, double);

extern void   convert_u_v_to_unit(double, double, double &, double &);

extern void   convert_u_v_to_unit(double &, double &);


//////////////////////////////////////////////////////////////////


#endif   /*  __ANGLES_H__  */


//////////////////////////////////////////////////////////////////


