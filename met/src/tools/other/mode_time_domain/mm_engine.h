

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




   public:

      MM_Engine();
     ~MM_Engine();
      MM_Engine(const MM_Engine &);
      MM_Engine & operator=(const MM_Engine &);

      void clear();

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

      int n_fcst_simple()    const;
      int n_obs_simple()     const;

      int n_fcst_composite() const;
      int n_obs_composite()  const;

         //
         //  do stuff
         //

         //  Note: all object numbers are zero-based

      void do_match_merge();

      int n_composites() const;

      IntArray fcst_composite(int _comp_number) const;
      IntArray  obs_composite(int _comp_number) const;

};


////////////////////////////////////////////////////////////////////////


#endif   /*  __MTD_MATCH_MERGE_ENGINE_H__  */


////////////////////////////////////////////////////////////////////////


