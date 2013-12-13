

////////////////////////////////////////////////////////////////////////


   //
   //  Warning:  This file is machine generated
   //
   //            Do not edit by hand
   //
   //
   //  Created by arraygen on December 12, 2013   11:56 am   MST
   //


////////////////////////////////////////////////////////////////////////


#ifndef  __MODE_LINE_ARRAY_H__
#define  __MODE_LINE_ARRAY_H__


////////////////////////////////////////////////////////////////////////


#include <iostream>

#include "mode_line.h"


////////////////////////////////////////////////////////////////////////


class ModeLineArray {

   private:

      void init_from_scratch();

      void assign(const ModeLineArray &);

      void extend(int);


      int Nelements;

      int Nalloc;

      int AllocInc;

      ModeLine ** e;


   public:

      ModeLineArray();
     ~ModeLineArray();
      ModeLineArray(const ModeLineArray &);
      ModeLineArray & operator=(const ModeLineArray &);

      void clear();

      void dump(ostream &, int = 0) const;

      void set_alloc_inc(int = 0);   //  0 means default value (50)

      int n_elements() const;

      void add(const ModeLine &);
      void add(const ModeLineArray &);

      ModeLine operator[](int) const;

};


////////////////////////////////////////////////////////////////////////


inline int ModeLineArray::n_elements() const { return ( Nelements ); }


////////////////////////////////////////////////////////////////////////


#endif   /*  __MODE_LINE_ARRAY_H__  */


////////////////////////////////////////////////////////////////////////


