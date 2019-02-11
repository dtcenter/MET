// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2019
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*


////////////////////////////////////////////////////////////////////////


#ifndef  __NEAREST_NEIGHBOR_INTERPOLATOR_H__
#define  __NEAREST_NEIGHBOR_INTERPOLATOR_H__


////////////////////////////////////////////////////////////////////////


#include "interp_base.h"


////////////////////////////////////////////////////////////////////////


class Nearest_Interp : public Interpolator {

   private:

      void init_from_scratch();

      void assign(const Nearest_Interp &);

   public:

      Nearest_Interp();
     ~Nearest_Interp();

      Nearest_Interp(const Nearest_Interp &);
      Nearest_Interp & operator=(const Nearest_Interp &);

      void clear();

         //
         //  from the base class
         //

      InterpolationValue operator()(double x, double y) const;

      Interpolator * copy() const;

      void dump(ostream &, int = 0) const;

};


////////////////////////////////////////////////////////////////////////


#endif   /*  __NEAREST_NEIGHBOR_INTERPOLATOR_H__  */


////////////////////////////////////////////////////////////////////////


