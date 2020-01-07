// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2020
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*


////////////////////////////////////////////////////////////////////////


#ifndef  __MAXIMUM_INTERPOLATOR_H__
#define  __MAXIMUM_INTERPOLATOR_H__


////////////////////////////////////////////////////////////////////////


#include "interp_base.h"


////////////////////////////////////////////////////////////////////////


class Max_Interp : public Interpolator {

   private:

      void init_from_scratch();

      void assign(const Max_Interp &);

   public:

      Max_Interp();
     ~Max_Interp();

      Max_Interp(const Max_Interp &);
      Max_Interp & operator=(const Max_Interp &);

      void clear();

         //
         //  from the base class
         //

      InterpolationValue operator()(double x, double y) const;

      Interpolator * copy() const;

      void dump(ostream &, int = 0) const;

};


////////////////////////////////////////////////////////////////////////


#endif   /*  __MAXIMUM_INTERPOLATOR_H__  */


////////////////////////////////////////////////////////////////////////


