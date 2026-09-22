// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2026
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
#include <memory>
#include <vector>

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

      std::vector<int> E;

    public:

      EquivalenceClass();
     ~EquivalenceClass();
      EquivalenceClass(const EquivalenceClass &);
      EquivalenceClass & operator=(const EquivalenceClass &);

      void clear();

      void dump(std::ostream &, int = 0) const;

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


inline int EquivalenceClass::n_elements() const { return (int) E.size(); }


////////////////////////////////////////////////////////////////////////


inline bool EquivalenceClass::has(int k) const

{

for (int j : E)  {

   if ( j == k )  return true;

}


return false;

}


////////////////////////////////////////////////////////////////////////


extern std::ostream & operator<<(std::ostream &, const EquivalenceClass &);


////////////////////////////////////////////////////////////////////////


static const int mtd_partition_alloc_inc = 50;


////////////////////////////////////////////////////////////////////////


class Mtd_Partition {   //  disjoint unions of equivalence classes

   private:

      void init_from_scratch();

      void assign(const Mtd_Partition &);

      ConcatString specialized_dump_string(const int Nf, const int No) const;


      std::vector<std::unique_ptr<EquivalenceClass>> C;

   public:

      Mtd_Partition();
     ~Mtd_Partition();
      Mtd_Partition(const Mtd_Partition &);
      Mtd_Partition & operator=(const Mtd_Partition &);

      void clear();

      void dump(std::ostream &, int = 0) const;

      void specialized_dump(std::ostream &, const int Nf, const int No) const;
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


inline int Mtd_Partition::n_elements() const { return (int) C.size(); }

inline bool Mtd_Partition::has(int k) const

{

for (const auto & c : C)  {

   if ( c->has(k) )  return true;

}


return false;

}


////////////////////////////////////////////////////////////////////////


#endif   //  __MTD_PARTITION_H__


////////////////////////////////////////////////////////////////////////



