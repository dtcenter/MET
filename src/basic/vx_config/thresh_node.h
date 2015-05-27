

////////////////////////////////////////////////////////////////////////


#ifndef  __THRESH_NODE_H__
#define  __THRESH_NODE_H__


////////////////////////////////////////////////////////////////////////


#include "scanner_stuff.h"
#include "threshold.h"


////////////////////////////////////////////////////////////////////////


class ThreshNode {

   public:

      ThreshNode();
      virtual ~ThreshNode();

      virtual bool check(double) const = 0;

      virtual ThreshNode * copy() const = 0;

      virtual ThreshType type() const = 0;

};


////////////////////////////////////////////////////////////////////////


class Or_Node : public ThreshNode {

   public:

      Or_Node();
     ~Or_Node();

      bool check(double) const;

      ThreshNode * copy() const;

      ThreshType type() const;

      ThreshNode * left_child;
      ThreshNode * right_child;

};


////////////////////////////////////////////////////////////////////////


inline ThreshType Or_Node::type() const { return ( thresh_complex ); }


////////////////////////////////////////////////////////////////////////


class And_Node : public ThreshNode {

   public:

      And_Node();
     ~And_Node();

      bool check(double) const;

      ThreshType type() const;

      ThreshNode * copy() const;

      ThreshNode * left_child;
      ThreshNode * right_child;

};


////////////////////////////////////////////////////////////////////////


inline ThreshType And_Node::type() const { return ( thresh_complex ); }


////////////////////////////////////////////////////////////////////////


class Not_Node : public ThreshNode {

   public:

      Not_Node();
     ~Not_Node();

      bool check(double) const;

      ThreshType type() const;

      ThreshNode * copy() const;

      ThreshNode * child;

};


////////////////////////////////////////////////////////////////////////


inline ThreshType Not_Node::type() const { return ( thresh_complex ); }


////////////////////////////////////////////////////////////////////////


class Simple_Node : public ThreshNode {

   public:

      Simple_Node();
     ~Simple_Node();


      ThreshType type() const;

      bool check(double) const;

      ThreshNode * copy() const;

      double T;

      ThreshType op;

};


////////////////////////////////////////////////////////////////////////


#endif   /*  __THRESH_NODE_H__  */


////////////////////////////////////////////////////////////////////////


