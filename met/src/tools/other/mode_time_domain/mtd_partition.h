// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2019
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*



////////////////////////////////////////////////////////////////////////


#ifndef  __MTD_PARTITION_H__
#define  __MTD_PARTITION_H__


////////////////////////////////////////////////////////////////////////


#include <iostream>

#include "concat_string.h"


////////////////////////////////////////////////////////////////////////


static const int eq_alloc_inc = 128;

extern int n_eq_max;


////////////////////////////////////////////////////////////////////////


class EquivalenceClass {

      friend class Mtd_Partition;

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

      void dump(ostream &, int = 0) const;

         //
         //  set stuff
         //

         //
         //  get stuff
         //

      bool has(int) const;

      int element(int) const;

      int n_max() const;

      int n_elements() const;

         //
         //  do stuff
         //

      void add_no_repeat(int);

};


////////////////////////////////////////////////////////////////////////


inline int EquivalenceClass::n_elements() const { return ( Nelements ); }


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


static const int mtd_partition_alloc_inc = 50;


////////////////////////////////////////////////////////////////////////


class Mtd_Partition {   //  disjoint unions of equivalence classes

   private:

      void init_from_scratch();

      void assign(const Mtd_Partition &);

      void extend(int);

      ConcatString specialized_dump_string(const int Nf, const int No) const;


      EquivalenceClass ** C;   //  allocated

      int Nelements;

      int Nalloc;

   public:

      Mtd_Partition();
     ~Mtd_Partition();
      Mtd_Partition(const Mtd_Partition &);
      Mtd_Partition & operator=(const Mtd_Partition &);

      void clear();

      void dump(ostream &, int = 0) const;

      void specialized_dump(ostream &, const int Nf, const int No) const;
      void specialized_dump(const int, const int Nf, const int No) const;   //  dump to mlog with the given verbosity

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

      const EquivalenceClass * operator()(int) const;

         //
         //  do stuff
         //

      void merge_classes (int, int);
      void merge_values  (int, int);

      void add_no_repeat(int);


};


////////////////////////////////////////////////////////////////////////


inline int Mtd_Partition::n_elements() const { return ( Nelements ); }

inline bool Mtd_Partition::has(int k) const

{

int j;
EquivalenceClass ** c = C;

for (j=0; j<Nelements; ++j, ++c)  {

   if ( (*c)->has(k) )  return ( true );

}


return ( false );

}


////////////////////////////////////////////////////////////////////////


#endif   //  __MTD_PARTITION_H__


////////////////////////////////////////////////////////////////////////



