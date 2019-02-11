// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2019
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*



///////////////////////////////////////////////////////////////////////////////


#ifndef  __PERCENTILE_H__
#define  __PERCENTILE_H__


///////////////////////////////////////////////////////////////////////////////

//
// Structure used in computing the rank of an array of data
//
struct RankInfo {

   int index;
   
   const double *data;

};

///////////////////////////////////////////////////////////////////////////////


extern void sort(double * array, const int n);

extern double percentile(const double * ordered_array, const int n, const double t);



extern void sort_f(float * array, const int n);

extern float percentile_f(const float * ordered_array, const int n, const double t);



extern int do_rank(const double *array, double *rank, int n);


///////////////////////////////////////////////////////////////////////////////


#endif   //  __PERCENTILE_H__


///////////////////////////////////////////////////////////////////////////////

