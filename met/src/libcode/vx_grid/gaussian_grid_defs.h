

// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2017
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*




////////////////////////////////////////////////////////////////////////


#ifndef  __GAUSSIAN_GRID_DEFINITIONS_H__
#define  __GAUSSIAN_GRID_DEFINITIONS_H__


////////////////////////////////////////////////////////////////////////


struct GaussianData {

   const char * name;   //  not allocated

   double Lon_Zero;     //  longitude that has x = 0

   int Nx;
   int Ny;

};


////////////////////////////////////////////////////////////////////////


#endif   /*  __GAUSSIAN_GRID_DEFINITIONS_H__  */


////////////////////////////////////////////////////////////////////////



