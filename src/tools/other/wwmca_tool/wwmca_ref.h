

   // *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
   // ** Copyright UCAR (c) 1992 - 2012
   // ** University Corporation for Atmospheric Research (UCAR)
   // ** National Center for Atmospheric Research (NCAR)
   // ** Research Applications Lab (RAL)
   // ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
   // *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*




////////////////////////////////////////////////////////////////////////


#ifndef  __WWMCA_REGRIDDER_H__
#define  __WWMCA_REGRIDDER_H__


////////////////////////////////////////////////////////////////////////


#include <iostream>

#include <netcdf.hh>

#include "afwa_file.h"
#include "afwa_cp_file.h"
#include "afwa_pt_file.h"
#include "interp_base.h"
#include "wwmca_regrid_Conf.h"

#include "vx_grid.h"


////////////////////////////////////////////////////////////////////////


enum GridHemisphere {

   north_hemisphere,
   south_hemisphere,

   both_hemispheres,

   no_hemisphere

};


////////////////////////////////////////////////////////////////////////


class WwmcaRegridder {

   private:

      WwmcaRegridder(const WwmcaRegridder &);
      WwmcaRegridder & operator=(const WwmcaRegridder &);

      void init_from_scratch();


      InterpolationValue get_interpolated_value(int to_x, int to_y) const;

      void find_grid_hemisphere();

      void get_interpolator();

      void get_grid();

      void parse_lambert_grid();
      void parse_latlon_grid();
      void parse_stereographic_grid();
      void parse_mercator_grid();


      InterpolationValue do_single_hemi(int to_x, int to_y, const Grid * From,
                                        const AfwaCloudPctFile & cloud,
                                        const AfwaPixelTimeFile & pixel) const;

      InterpolationValue do_both_hemi(int to_x, int to_y) const;


      GridInfo ginfo;


      StringArray grid_strings;

      GridHemisphere Hemi;

      const Grid * NHgrid;           //  allocated
      const Grid * SHgrid;           //  allocated

      const AfwaCloudPctFile * cp_nh;   //  allocated
      const AfwaCloudPctFile * cp_sh;   //  allocated

      const AfwaPixelTimeFile * pt_nh;   //  allocated
      const AfwaPixelTimeFile * pt_sh;   //  allocated

      const Grid * ToGrid;           //  allocated

      Interpolator * interp;         //  allocated

      wwmca_regrid_Conf * Config;          //  not allocated

      ConcatString ConfigFilename;


   public:

      WwmcaRegridder();
     ~WwmcaRegridder();

      void clear();

      void dump(ostream &, int = 0) const;

      void set_cp_nh_file(const char *);
      void set_cp_sh_file(const char *);

      void set_pt_nh_file(const char *);
      void set_pt_sh_file(const char *);

      void set_config(wwmca_regrid_Conf &, const char * filename);


      GridHemisphere hemi() const;

      void do_output(const char * output_filename);

         //
         //  get interpolated value for "to" grid point (x, y)
         //

      InterpolationValue operator()(int x, int y) const;

};


////////////////////////////////////////////////////////////////////////


inline GridHemisphere WwmcaRegridder::hemi() const { return ( Hemi ); }


////////////////////////////////////////////////////////////////////////


#endif   /*  __WWMCA_REGRIDDER_H__  */


////////////////////////////////////////////////////////////////////////


