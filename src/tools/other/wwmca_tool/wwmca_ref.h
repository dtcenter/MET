// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2019
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

#include "af_file.h"
#include "af_cp_file.h"
#include "af_pt_file.h"

#include "vx_config.h"
#include "vx_grid.h"
#include "interp_util.h"


////////////////////////////////////////////////////////////////////////


enum GridHemisphere {

   north_hemisphere,
   south_hemisphere,

   both_hemispheres,

   no_hemisphere

};


////////////////////////////////////////////////////////////////////////


typedef void InterpFunction (const DataPlane & fat, DataPlane & out, int w, double);


////////////////////////////////////////////////////////////////////////


class WwmcaRegridder {

   private:

      WwmcaRegridder(const WwmcaRegridder &);
      WwmcaRegridder & operator=(const WwmcaRegridder &);

      void init_from_scratch();

      void get_interpolated_data(DataPlane &) const;

      void find_grid_hemisphere();

      void get_grid();

      void do_single_hemi(DataPlane &, const Grid * From,
                                       const AFCloudPctFile * cloud,
                                       const AFPixelTimeFile * pixel) const;

      void do_both_hemi(DataPlane &) const;


      InterpMthd Method;   //  interpolation

      int Width;           //  interpolation

      double Fraction;     //  fraction of good data needed for interpolation

      bool WritePixelAge;  //  write pixel age instead of cloud data

      InterpFunction * interp_func;


      StringArray grid_strings;

      GridHemisphere Hemi;

      const Grid * NHgrid;           //  allocated
      const Grid * SHgrid;           //  allocated

      const AFCloudPctFile * cp_nh;  //  allocated
      const AFCloudPctFile * cp_sh;  //  allocated

      const AFPixelTimeFile * pt_nh; //  allocated
      const AFPixelTimeFile * pt_sh; //  allocated

      const Grid * ToGrid;           //  allocated

      MetConfig * Config;            //  not allocated

      ConcatString ConfigFilename;


   public:

      WwmcaRegridder();
     ~WwmcaRegridder();

      void clear();

      void dump(ostream &, int = 0) const;

      void set_cp_nh_file(const char *);
      void set_cp_sh_file(const char *);

      void set_pt_nh_file(const char *, bool);
      void set_pt_sh_file(const char *, bool);

      void set_config(MetConfig &, const char * filename);


      GridHemisphere hemi() const;

      void do_output(const char * output_filename);


};


////////////////////////////////////////////////////////////////////////


inline GridHemisphere WwmcaRegridder::hemi() const { return ( Hemi ); }


////////////////////////////////////////////////////////////////////////


extern void dp_interp_min     (const DataPlane & fat, DataPlane & out, int w, double t);
extern void dp_interp_max     (const DataPlane & fat, DataPlane & out, int w, double t);
extern void dp_interp_uw_mean (const DataPlane & fat, DataPlane & out, int w, double t);


////////////////////////////////////////////////////////////////////////


#endif   /*  __WWMCA_REGRIDDER_H__  */


////////////////////////////////////////////////////////////////////////


