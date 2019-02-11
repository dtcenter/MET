// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2019
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*



////////////////////////////////////////////////////////////////////////


#ifndef  __MTD_FO_GRAPH_H__
#define  __MTD_FO_GRAPH_H__


////////////////////////////////////////////////////////////////////////


#include "fo_node.h"


////////////////////////////////////////////////////////////////////////


// typedef int GraphNodeType;


////////////////////////////////////////////////////////////////////////


class FO_Graph {

   private:

      void init_from_scratch();

      void assign(const FO_Graph &);

      int two_to_one(int, int) const;

      int N_fcst;
      int N_obs;

      int N_total;

      int N_nodes;   //  N_total*N_total
                     //
                     //  Note that this is the number of nodes,
                     //  not the number of bytes

      FO_Node * TheGraph;   //  allocated

      void do_dump_table(AsciiTable &) const;

   public:

      FO_Graph();
     ~FO_Graph();
      FO_Graph(const FO_Graph &);
      FO_Graph & operator=(const FO_Graph &);

      void clear();

         //
         //  set stuff
         //

      void set_size(int n_fcst, int n_obs);

         //
         //  get stuff
         //

      int n_fcst() const;
      int n_obs()  const;

      int n_total()  const;

      int n_nodes()  const;

      bool has_fo_edge(int n_f,   int n_o)   const;   //  0-based
      bool has_ff_edge(int n_f_1, int n_f_2) const;   //  0-based
      bool has_oo_edge(int n_o_1, int n_o_2) const;   //  0-based

      int f_index(int f_num) const;
      int o_index(int o_num) const;

         //
         //  do stuff
         //

      void set_fo_edge(int n_f,   int n_o);     //  0-based
      void set_ff_edge(int n_f_1, int n_f_2);   //  0-based
      void set_oo_edge(int n_o_1, int n_o_2);   //  0-based

      void erase_edges();

      void dump_as_table(ostream &) const;
      void dump_as_table(const int) const;   //  dumps to mlog with the given verbosity level


};


////////////////////////////////////////////////////////////////////////


inline int FO_Graph::n_fcst () const { return ( N_fcst ); }
inline int FO_Graph::n_obs  () const { return ( N_obs  ); }

inline int FO_Graph::n_nodes () const { return ( N_nodes ); }

inline int FO_Graph::n_total () const { return ( N_fcst + N_obs  ); }

inline int FO_Graph::two_to_one(int _a, int _b) const { return ( _a*N_total + _b ); }


////////////////////////////////////////////////////////////////////////


extern FO_Graph do_union(const FO_Graph &, const FO_Graph &);


////////////////////////////////////////////////////////////////////////


#endif   /*  __MTD_FO_GRAPH_H__  */


////////////////////////////////////////////////////////////////////////


