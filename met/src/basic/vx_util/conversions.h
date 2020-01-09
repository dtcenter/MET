// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2020
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*


////////////////////////////////////////////////////////////////////////

#ifndef  __CONVERSIONS_H__
#define  __CONVERSIONS_H__

////////////////////////////////////////////////////////////////////////

extern double convert_gpm_to_msl(double, double);
extern double convert_p_t_z_to_prmsl(double, double, double, double);
extern double convert_q_to_w(double);
extern double convert_p_w_to_vp(double, double);
extern double convert_vp_to_dpt(double);
extern double convert_t_to_svp(double);
extern double convert_vp_svp_to_rh(double, double);
extern double convert_p_q_t_to_rh(double, double, double);

extern double gop_by_lat(double);

////////////////////////////////////////////////////////////////////////

#endif   //  __CONVERSIONS_H__

////////////////////////////////////////////////////////////////////////
