

////////////////////////////////////////////////////////////////////////


#ifndef  __MET_INT_ARRAY_H__
#define  __MET_INT_ARRAY_H__


////////////////////////////////////////////////////////////////////////


#include <iostream>


////////////////////////////////////////////////////////////////////////


static const int met_intarray_alloc_inc = 20;


////////////////////////////////////////////////////////////////////////


class MetIntArray {

   private:

      void init_from_scratch();

      void assign(const MetIntArray &);



      int * e;

      int Nelements;

      int Nalloc;

   public:

      MetIntArray();
     ~MetIntArray();
      MetIntArray(const MetIntArray &);
      MetIntArray & operator=(const MetIntArray &);

      void clear();

      void dump(ostream &, int depth = 0) const;

      void extend(int);


      void set_size(int);

      int operator[](int) const;

      int has(int) const;
      int has(int, int & index) const;

      void add(int);
      void add(const MetIntArray &);

      void add_no_repeat(int);
      void add_no_repeat(const MetIntArray &);

      void put(int index, int value);

      int n_elements() const;

      void sort_increasing();

};


////////////////////////////////////////////////////////////////////////


inline int MetIntArray::n_elements() const { return ( Nelements ); }


////////////////////////////////////////////////////////////////////////


#endif   /*  __MET_INT_ARRAY_H__  */


////////////////////////////////////////////////////////////////////////


