

////////////////////////////////////////////////////////////////////////


#ifndef  __MINIMUM_INTERPOLATOR_H__
#define  __MINIMUM_INTERPOLATOR_H__


////////////////////////////////////////////////////////////////////////


#include "interp_base.h"


////////////////////////////////////////////////////////////////////////


class Min_Interp : public Interpolator {

   private:

      void init_from_scratch();

      void assign(const Min_Interp &);

   public:

      Min_Interp();
     ~Min_Interp();

      Min_Interp(const Min_Interp &);
      Min_Interp & operator=(const Min_Interp &);

      void clear();

         //
         //  from the base class
         //

      InterpolationValue operator()(double x, double y) const;

      Interpolator * copy() const;

      void dump(ostream &, int = 0) const;

};


////////////////////////////////////////////////////////////////////////


#endif   /*  __MINIMUM_INTERPOLATOR_H__  */


////////////////////////////////////////////////////////////////////////


