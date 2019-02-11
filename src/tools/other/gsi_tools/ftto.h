// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2019
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*



////////////////////////////////////////////////////////////////////////


#ifndef  __FORTRAN_TWO_TO_ONE_H__
#define  __FORTRAN_TWO_TO_ONE_H__


////////////////////////////////////////////////////////////////////////


inline int fortran_two_to_one(const int N1, const int v1, const int v2) { return ( v2*N1 + v1 ); }


////////////////////////////////////////////////////////////////////////


#endif   /*  __FORTRAN_TWO_TO_ONE_H__  */


////////////////////////////////////////////////////////////////////////


