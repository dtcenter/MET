// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2026
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*


////////////////////////////////////////////////////////////////////////


#ifndef  __INTERPOLATOR_H__
#define  __INTERPOLATOR_H__


////////////////////////////////////////////////////////////////////////


#include <iostream>

#include "vx_math.h"
#include "vx_util.h"
#include <vector>


////////////////////////////////////////////////////////////////////////


class InterpolationValue {

   private:

      void init_from_scratch();

      void assign(const InterpolationValue &);

   public:

      InterpolationValue();
     ~InterpolationValue();
      InterpolationValue(const InterpolationValue &);
      InterpolationValue & operator=(const InterpolationValue &);

      void clear();

      void set_good (double);
      void set_bad  ();

      double value;

      bool ok;   //  value good or bad

};


////////////////////////////////////////////////////////////////////////


extern std::ostream & operator<<(std::ostream &, const InterpolationValue &);


#endif   /*  __INTERPOLATOR_H__  */


////////////////////////////////////////////////////////////////////////


