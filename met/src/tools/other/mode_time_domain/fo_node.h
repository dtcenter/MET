// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2020
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*



////////////////////////////////////////////////////////////////////////


#ifndef  __MTD_NODE_H__
#define  __MTD_NODE_H__


////////////////////////////////////////////////////////////////////////


#include <iostream>


////////////////////////////////////////////////////////////////////////


class FO_Node {

   private:

      void init_from_scratch();

      void assign(const FO_Node &);


      int Number;

      bool IsFcst;

      bool IsVisited;

      bool HasEdge;


   public:

      FO_Node();
     ~FO_Node();
      FO_Node(const FO_Node &);
      FO_Node & operator=(const FO_Node &);


      void clear();

      void dump(ostream &, int) const;

         //
         //  set stuff
         //

      void set_fcst();
      void set_obs();

      void set_number(int);

      void set_visited  (bool = true);
      void set_has_edge (bool = true);

         //
         //  get stuff
         //

      bool is_fcst() const;
      bool is_obs() const;

      bool is_visited() const;
      bool has_edge() const;

      int number() const;

         //
         //  do stuff
         //



};


////////////////////////////////////////////////////////////////////////


inline bool FO_Node::is_fcst() const { return (   IsFcst ); }
inline bool FO_Node::is_obs() const  { return ( ! IsFcst ); }

inline bool FO_Node::is_visited() const  { return ( IsVisited ); }

inline bool FO_Node::has_edge() const  { return ( HasEdge ); }

inline int FO_Node::number() const  { return ( Number ); }

inline void FO_Node::set_fcst() { IsFcst = true; }
inline void FO_Node::set_obs()  { IsFcst = false; }


////////////////////////////////////////////////////////////////////////


#endif   /*  __MTD_NODE_H__  */


////////////////////////////////////////////////////////////////////////


