

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


