

////////////////////////////////////////////////////////////////////////


#ifndef  __MTD_NODE_H__
#define  __MTD_NODE_H__


////////////////////////////////////////////////////////////////////////


class FO_Node {

   private:

      void init_from_scratch();

      void assign(const FO_Node &);


      int Number;

      bool IsFcst;

      bool IsVisited;


   public:

      FO_Node();
     ~FO_Node();
      FO_Node(const FO_Node &);
      FO_Node & operator=(const FO_Node &);


      void clear();

         //
         //  set stuff
         //

      void set_fcst();
      void set_obs();

      void set_number(int);

      void set_visited(bool = true);

         //
         //  get stuff
         //

      bool is_fcst() const;
      bool is_obs() const;

      bool is_visited() const;

      int number() const;

         //
         //  do stuff
         //



};


////////////////////////////////////////////////////////////////////////


inline bool FO_Node::is_fcst() const { return (   IsFcst ); }
inline bool FO_Node::is_obs() const  { return ( ! IsFcst ); }

inline bool FO_Node::is_visited() const  { return ( IsVisited ); }

inline int FO_Node::number() const  { return ( Number ); }

inline void FO_Node::set_fcst() { IsFcst = true; }
inline void FO_Node::set_obs()  { IsFcst = false; }


////////////////////////////////////////////////////////////////////////


#endif   /*  __MTD_NODE_H__  */


////////////////////////////////////////////////////////////////////////


