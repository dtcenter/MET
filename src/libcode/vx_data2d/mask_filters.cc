// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2024
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*

////////////////////////////////////////////////////////////////////////


using namespace std;

#include "mask_filters.h"


////////////////////////////////////////////////////////////////////////


   //
   //  Code for class MaskFilters
   //


////////////////////////////////////////////////////////////////////////


MaskFilters::MaskFilters():
   grid_mask(0),
   area_mask(0),
   poly_mask(0),
   sid_mask(0),
   typ_mask(0)
{
   clear();
}

////////////////////////////////////////////////////////////////////////

MaskFilters::MaskFilters(Grid *_grid_mask, MaskPlane *_area_mask, MaskPoly *_poly_mask,
                       StringArray *_sid_mask, StringArray *_typ_mask) {
   clear();
   grid_mask = _grid_mask;
   area_mask = _area_mask;
   poly_mask = _poly_mask;
   sid_mask = _sid_mask;
   typ_mask = _typ_mask;
}
              
////////////////////////////////////////////////////////////////////////

void MaskFilters::clear() {
   grid_mask_cnt = 0;
   area_mask_cnt = 0;
   poly_mask_cnt = 0;
   sid_mask_cnt = 0;
   typ_mask_cnt = 0;
}

////////////////////////////////////////////////////////////////////////

bool MaskFilters::is_filtered(double lat, double lon) {
   bool masked = false;
   // Apply the grid mask
   if(grid_mask) {
      double grid_x, grid_y;
      grid_mask->latlon_to_xy(lat, -1.0*lon, grid_x, grid_y);

      if(grid_x < 0 || grid_x >= grid_mask->nx() ||
         grid_y < 0 || grid_y >= grid_mask->ny()) {
         grid_mask_cnt++;
         masked = true;
      }

      // Apply the area mask
      if(area_mask && !masked) {
         if(!area_mask->s_is_on(nint(grid_x), nint(grid_y))) {
            area_mask_cnt++;
            masked = true;
         }
      }
   }

   // Apply the polyline mask
   if(poly_mask && !masked) {
     if(!poly_mask->latlon_is_inside_dege(lat, lon)) {
       poly_mask_cnt++;
       masked = true;
     }
   }
   
   return masked;
}

////////////////////////////////////////////////////////////////////////

bool MaskFilters::is_filtered_sid(const char *sid) {
   bool masked = false;

   // Apply the station ID mask
   if(sid_mask) {
      if(!sid_mask->has(sid)) {
         sid_mask_cnt++;
         masked = true;
      }
   }

   return masked;
}

////////////////////////////////////////////////////////////////////////

bool MaskFilters::is_filtered_typ(const char *msg_typ) {
   bool masked = false;

   // Apply the message type mask
   if(typ_mask) {
      if(!typ_mask->has(msg_typ)) {
         typ_mask_cnt++;
         masked = true;
      }
   }
   return masked;
}

////////////////////////////////////////////////////////////////////////

void MaskFilters::set_area_mask(MaskPlane *_area_mask) {
   area_mask = _area_mask;
}

////////////////////////////////////////////////////////////////////////

void MaskFilters::set_grid_mask(Grid *_grid_mask) {
   grid_mask = _grid_mask;
}

////////////////////////////////////////////////////////////////////////

void MaskFilters::set_poly_mask(MaskPoly *_poly_mask) {
   poly_mask = _poly_mask;
}

////////////////////////////////////////////////////////////////////////

void MaskFilters::set_sid_mask(StringArray *_sid_mask) {
   sid_mask = _sid_mask;
}

////////////////////////////////////////////////////////////////////////

void MaskFilters::set_typ_mask(StringArray *_typ_mask) {
   typ_mask = _typ_mask;
}

////////////////////////////////////////////////////////////////////////
