// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2023
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*


////////////////////////////////////////////////////////////////////////


#ifndef __MASK_FILTERS_H__
#define __MASK_FILTERS_H__


////////////////////////////////////////////////////////////////////////


#include "grid_base.h"


////////////////////////////////////////////////////////////////////////

class MaskFilters {

protected:

   int grid_mask_cnt;
   int area_mask_cnt;
   int poly_mask_cnt;
   int typ_mask_cnt;
   int sid_mask_cnt;

   Grid *grid_mask;
   MaskPlane *area_mask;
   MaskPoly *poly_mask;
   StringArray *sid_mask;   // station IDs to be excuded
   StringArray *typ_mask;   // message types to be excuded

public:

   MaskFilters();
   MaskFilters(Grid *grid_mask, MaskPlane *area_mask,
               MaskPoly *poly_mask, StringArray *sid_mask, StringArray *typ_mask);

   void clear();

   int get_area_mask_cnt();
   int get_grid_mask_cnt();
   int get_poly_mask_cnt();
   int get_sid_mask_cnt();
   int get_typ_mask_cnt();

   bool is_filtered(double lat, double lon);
   bool is_filtered_sid(const char *sid);
   bool is_filtered_typ(const char *msg_typ);
   
   void set_area_mask(MaskPlane *_area_mask);
   void set_grid_mask(Grid *_grid_mask);
   void set_poly_mask(MaskPoly *_poly_mask);
   void set_sid_mask(StringArray *_sid_mask);
   void set_typ_mask(StringArray *_typ_mask);

};

////////////////////////////////////////////////////////////////////////

inline int MaskFilters::get_area_mask_cnt() { return area_mask_cnt; };
inline int MaskFilters::get_grid_mask_cnt() { return grid_mask_cnt; };
inline int MaskFilters::get_poly_mask_cnt() { return poly_mask_cnt; };
inline int MaskFilters::get_sid_mask_cnt()  { return sid_mask_cnt; };
inline int MaskFilters::get_typ_mask_cnt()  { return typ_mask_cnt; };


////////////////////////////////////////////////////////////////////////


#endif   /*  __MASK_FILTERS_H__  */


////////////////////////////////////////////////////////////////////////
