// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2014
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


extern void sort(double *array, int n);


extern double percentile(const double *ordered_array, int n, double t);


extern int rank(const double *array, double *rank, int n);


///////////////////////////////////////////////////////////////////////////////


#endif   //  __PERCENTILE_H__


///////////////////////////////////////////////////////////////////////////////

