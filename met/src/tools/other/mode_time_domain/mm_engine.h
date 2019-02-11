// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2019
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*



////////////////////////////////////////////////////////////////////////


#ifndef  __MTD_MATCH_MERGE_ENGINE_H__
#define  __MTD_MATCH_MERGE_ENGINE_H__


////////////////////////////////////////////////////////////////////////


#include "mtd_partition.h"
#include "interest_calc.h"
#include "fo_graph.h"

#include "int_array.h"


////////////////////////////////////////////////////////////////////////


class MM_Engine {

   protected:

      void init_from_scratch();

      void assign(const MM_Engine &);

      int N_Composites;
      // int N_Obs_Composites;

      int * comp_to_eq;   //  allocated, 0-based, both ways


   public:

      MM_Engine();
     ~MM_Engine();
      MM_Engine(const MM_Engine &);
      MM_Engine & operator=(const MM_Engine &);

      void clear();

      void partition_dump(ostream &) const;
      void partition_dump(const int) const;   //  dump to mlog with the given verbosity

      InterestCalculator calc;

      Mtd_Partition part;

      FO_Graph graph;

         //
         //  set stuff
         //

      void set_size(const int _n_fcst, const int _n_obs);

         //
         //  get stuff
         //

      int n_fcst_simples()    const;
      int n_obs_simples()     const;

      int n_composites() const;

          //
          //  return the index of the composite (ie, equivalence class) that
          //    has the given fcst or obs object number
          //

      int composite_with_fcst (const int) const;   //  0-based (both input and output)
      int composite_with_obs  (const int) const;   //  0-based (both input and output)

         //
         //  do stuff
         //

         //  Note: all object numbers are 0-based

      void do_match_merge();

      int map_fcst_id_to_composite (const int id) const;      //  0-based
      int map_obs_id_to_composite  (const int id) const;      //  0-based

      IntArray fcst_composite(int _composite_number) const;   //  0-based
      IntArray  obs_composite(int _composite_number) const;   //  0-based

};


////////////////////////////////////////////////////////////////////////


inline int MM_Engine::n_fcst_simples     () const { return ( graph.n_fcst () ); }
inline int MM_Engine::n_obs_simples      () const { return ( graph.n_obs  () ); }

inline int MM_Engine::n_composites       () const { return ( N_Composites ); }


////////////////////////////////////////////////////////////////////////


#endif   /*  __MTD_MATCH_MERGE_ENGINE_H__  */


////////////////////////////////////////////////////////////////////////


