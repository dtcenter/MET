// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2019
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*


//////////////////////////////////////////////////////////////////


#ifndef __NAV_H__
#define __NAV_H__


//////////////////////////////////////////////////////////////////

extern double            gc_dist(double lat1, double lon1, double lat2, double lon2);
extern double         gc_bearing(double lat1, double lon1, double lat2, double lon2);

extern double            rl_dist(double lat1, double lon1, double lat2, double lon2);
extern double         rl_bearing(double lat1, double lon1, double lat2, double lon2);

extern void          gc_point_v1(double lat1, double lon1, double lat2, double lon2, 
                                 double dist, double &lat, double &lon);

extern void          gc_point_v2(double lat1, double lon1, double bear, double dist, 
                                 double &lat, double &lon);

extern void          rl_point_v1(double lat1, double lon1, double lat2, double lon2, 
                                 double dist, double &lat, double &lon);

extern void          rl_point_v2(double lat1, double lon1, double bear, double dist, 
                                 double &lat, double &lon);

extern double        haversine(double a);
extern double       ahaversine(double t);
extern double        meridional_parts(double a);

extern double  gc_dist_to_line(double lat1, double lon1,
                               double lat2, double lon2,
                               double lat3, double lon3);

//////////////////////////////////////////////////////////////////


#endif  //  __NAV_H__


//////////////////////////////////////////////////////////////////


