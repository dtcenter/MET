

////////////////////////////////////////////////////////////////////////


#ifndef  __PARTITION_H__
#define  __PARTITION_H__


////////////////////////////////////////////////////////////////////////


#include <iostream>


////////////////////////////////////////////////////////////////////////


static const int eq_alloc_inc = 128;

extern int n_eq_max;


////////////////////////////////////////////////////////////////////////


class EquivalenceClass {

      friend class Partition;

   private:

      void init_from_scratch();

      void assign(const EquivalenceClass &);

      void extend(int);


      int * E;   //  allocated

      int Nelements;

      int Nalloc;

    public:

      EquivalenceClass();
     ~EquivalenceClass();
      EquivalenceClass(const EquivalenceClass &);
      EquivalenceClass & operator=(const EquivalenceClass &);

      void clear();

         //
         //  set stuff
         //

         //
         //  get stuff
         //

      bool has(int) const;

      int element(int) const;

      int n_max() const;

         //
         //  do stuff
         //

      void add_no_repeat(int);

};


////////////////////////////////////////////////////////////////////////


inline bool EquivalenceClass::has(int k) const

{

int j;
int * e = E;

for (j=0; j<Nelements; ++j, ++e)  {

   if ( *e == k )  return ( true );

}


return ( false );

}


////////////////////////////////////////////////////////////////////////


extern ostream & operator<<(ostream &, const EquivalenceClass &);


////////////////////////////////////////////////////////////////////////


static const int partition_alloc_inc = 50;


////////////////////////////////////////////////////////////////////////


class Partition {   //  disjoint unions of equivalence classes

   private:

      void init_from_scratch();

      void assign(const Partition &);

      void extend(int);


      EquivalenceClass ** C;   //  allocated

      int Nelements;

      int Nalloc;

   public:

      Partition();
     ~Partition();
      Partition(const Partition &);
      Partition & operator=(const Partition &);

      void clear();

         //
         //  set stuff
         //

         //
         //  get stuff
         //

      bool has(int) const;

      bool has(int index, int value) const;

      int which_class(int) const;

      int n_elements() const;

         //
         //  do stuff
         //

      void merge_classes (int, int);
      void merge_values  (int, int);

      void add_no_repeat(int);


};


////////////////////////////////////////////////////////////////////////


inline int Partition::n_elements() const { return ( Nelements ); }

inline bool Partition::has(int k) const

{

int j;
EquivalenceClass ** c = C;

for (j=0; j<Nelements; ++j, ++c)  {

   if ( (*c)->has(k) )  return ( true );

}


return ( false );

}


////////////////////////////////////////////////////////////////////////


#endif   //  __PARTITION_H__


////////////////////////////////////////////////////////////////////////



