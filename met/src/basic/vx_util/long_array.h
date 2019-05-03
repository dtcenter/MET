

// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2019
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*




////////////////////////////////////////////////////////////////////////


#ifndef  __MET_LONG_ARRAY_H__
#define  __MET_LONG_ARRAY_H__


////////////////////////////////////////////////////////////////////////


#include <iostream>


////////////////////////////////////////////////////////////////////////


class LongArray {

   private:

      void init_from_scratch();

      void assign(const LongArray &);

      void extend(int);


      int Nelements;

      int Nalloc;

      int AllocInc;

      long * e;


   public:

      LongArray();
     ~LongArray();
      LongArray(const LongArray &);
      LongArray & operator=(const LongArray &);

      void clear();

      void erase();

      void dump(ostream &, int = 0) const;

      void set_alloc_inc(int = 0);   //  0 means default value (10)

      int n_elements() const;

      int has(const long) const;

      void add(const long &);
      void add(const LongArray &);

      long & operator[](int) const;

      operator long * () const;

};


////////////////////////////////////////////////////////////////////////


inline int LongArray::n_elements() const { return ( Nelements ); }

inline LongArray::operator long * () const { return ( e ); }

inline void LongArray::erase() { Nelements = 0;  return; }


////////////////////////////////////////////////////////////////////////


#endif   /*  __MET_LONG_ARRAY_H__  */


////////////////////////////////////////////////////////////////////////


