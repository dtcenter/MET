// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2011
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*



////////////////////////////////////////////////////////////////////////


#ifndef  __VERIF_BOUNDINGBOX_H__
#define  __VERIF_BOUNDINGBOX_H__


////////////////////////////////////////////////////////////////////////


struct BoundingBox {

   double x_ll;
   double y_ll;

   double x_ur;
   double y_ur;

   double width;
   double height;

};


////////////////////////////////////////////////////////////////////////


extern bool bb_intersect(const BoundingBox &, const BoundingBox &);


////////////////////////////////////////////////////////////////////////


#endif   //  __VERIF_BOUNDINGBOX_H__


////////////////////////////////////////////////////////////////////////


