

////////////////////////////////////////////////////////////////////////


#ifndef  __AVERAGE_INTERPOLATOR_H__
#define  __AVERAGE_INTERPOLATOR_H__


////////////////////////////////////////////////////////////////////////


#include "interp_base.h"


////////////////////////////////////////////////////////////////////////


class Ave_Interp : public Interpolator {

   private:

      void init_from_scratch();

      void assign(const Ave_Interp &);

   public:

      Ave_Interp();
     ~Ave_Interp();

      Ave_Interp(const Ave_Interp &);
      Ave_Interp & operator=(const Ave_Interp &);

      void clear();

         //
         //  from the base class
         //

      InterpolationValue operator()(double x, double y) const;

      Interpolator * copy() const;

      void dump(ostream &, int = 0) const;

};


////////////////////////////////////////////////////////////////////////


#endif   /*  __AVERAGE_INTERPOLATOR_H__  */


////////////////////////////////////////////////////////////////////////


