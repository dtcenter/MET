

////////////////////////////////////////////////////////////////////////


#ifndef  __MTD_FO_GRAPH_H__
#define  __MTD_FO_GRAPH_H__


////////////////////////////////////////////////////////////////////////


#include "fo_node.h"


////////////////////////////////////////////////////////////////////////


class FO_Graph {

   private:

      void init_from_scratch();

      void assign(const FO_Graph &);

      int Nfcst;
      int Nobs;

      int * TheGraph;   //  allocated

   public:

      FO_Graph();
     ~FO_Graph();
      FO_Graph(const FO_Graph &);
      FO_Graph & operator=(const FO_Graph &);

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

      bool fo_edge(int n_f,   int n_o) const;
      bool ff_edge(int n_f_1, int n_f_2) const;
      bool oo_edge(int n_o_1, int n_o_2) const;

         //
         //  do stuff
         //

      add_fo_edge(int n_f,   n_o);
      add_ff_edge(int n_f_1, n_f_2);
      add_oo_edge(int n_o_1, n_o_2);

      void erase_edges();


};


////////////////////////////////////////////////////////////////////////


inline int FO_Graph::n_fcst () const { return ( Nfcst ); }
inline int FO_Graph::n_obs  () const { return ( Nobs  ); }

inline int FO_Graph::n_totas () const { return ( Nfcst + Nobs  ); }


////////////////////////////////////////////////////////////////////////


extern FO_Graph union(const FO_Graph &, const FO_Graph &);


////////////////////////////////////////////////////////////////////////


#endif   /*  __MTD_FO_GRAPH_H__  */


////////////////////////////////////////////////////////////////////////


