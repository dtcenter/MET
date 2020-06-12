// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2020
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*

///////////////////////////////////////////////////////////////////////////////
//
//   Filename:   node.cc
//
//   Description:
//
//
//   Mod#   Date      Name           Description
//   ----   ----      ----           -----------
//   000    11-03-06  Halley Gotway
//
///////////////////////////////////////////////////////////////////////////////

using namespace std;

#include <iostream>
#include <unistd.h>
#include <stdlib.h>
#include <cmath>

#include "node.h"
#include "vx_log.h"
#include "vx_math.h"

///////////////////////////////////////////////////////////////////////////////
//
//  Code for class Node
//
///////////////////////////////////////////////////////////////////////////////

Node::Node() {

   child = (Node *) 0;

   sibling = (Node *) 0;

   clear();
}

///////////////////////////////////////////////////////////////////////////////

Node::~Node() {

   clear();
}

///////////////////////////////////////////////////////////////////////////////

Node::Node(const Node &c) {

   assign(c);
}

///////////////////////////////////////////////////////////////////////////////

Node & Node::operator=(const Node &c) {

   if(this == &c) return(*this);

   assign(c);

   return(*this);
}

///////////////////////////////////////////////////////////////////////////////

void Node::clear() {

   if(child) {
      child->clear();
      delete child;  child = (Node *) 0;
   }

   if(sibling) {
      sibling->clear();
      delete sibling;  sibling = (Node *) 0;
   }

   p.clear();

   return;
}

///////////////////////////////////////////////////////////////////////////////
//
// Copies the top level Node and all of its descendents (not siblings)
//
///////////////////////////////////////////////////////////////////////////////

void Node::assign(const Node &n) {

   clear();

   // Assign the top level node
   p = n.p;

   assign_tree(n.child);

   return;
}

///////////////////////////////////////////////////////////////////////////////

void Node::assign_tree(const Node *n_ptr) {

   // Assign all descendants
   while(n_ptr) {

      add_child( &(n_ptr->p) );

      child->assign_tree(n_ptr->child);

      n_ptr = n_ptr->sibling;
   }

   return;
}

///////////////////////////////////////////////////////////////////////////////

void Node::add_child(const Polyline * poly) {
   Node *n_ptr = (Node *) 0;

   // Check for first child
   if(child == NULL) {

      child = new Node;

      if(!child) {

         mlog << Error << "\nNode::add_child() -> "
              << "memory allocation error\n\n";

         exit(1);
      }

      child->p = *(poly);

      child->child = NULL;
      child->sibling = NULL;
   }
   // Existing children
   else {

      // Point to the child
      n_ptr = child;

      // Proceed to the last node
      while(n_ptr->sibling) {
         n_ptr = n_ptr->sibling;
      }

      n_ptr->sibling = new Node;

      if(!n_ptr->sibling) {

         mlog << Error << "\nNode::add_child() -> "
              << "memory allocation error\n\n";

         delete n_ptr;

         exit(1);
      }

      n_ptr->sibling->p = *(poly);

      n_ptr->sibling->child = NULL;
      n_ptr->sibling->sibling = NULL;
   }

   return;
}

///////////////////////////////////////////////////////////////////////////////

int Node::n_children() const {
   int count;
   Node *n_ptr = (Node *) 0;

   count = 0;

   // Search through all children
   n_ptr = child;

   while(n_ptr) {
      count++;

      n_ptr = n_ptr->sibling;
   }

   return(count);
}

///////////////////////////////////////////////////////////////////////////////
//
// n is the index of the child, ranging from 0 to n_children-1
//
///////////////////////////////////////////////////////////////////////////////

Node *Node::get_child(int n) const {
   int children_count, i;
   Node *n_ptr = (Node *) 0;

   if( n >= (children_count = n_children()) ) {

      mlog << Error << "\nNode::get_child(int) -> "
	   << "attempting to access child number " << n << " when only " 
	   << children_count << " exist\n\n";

      exit(1);
   }

   n_ptr = child;

   for(i=0; i<n; i++) n_ptr = n_ptr->sibling;

   return(n_ptr);
}

///////////////////////////////////////////////////////////////////////////////

int Node::is_closed() const {
   int closed;
   Node *n_ptr = (Node *) 0;

   // Check if the current polyline is closed
   closed = p.is_closed();

   // Search through all descendants
   n_ptr = child;

   while(n_ptr) {
      closed *= n_ptr->is_closed();

      n_ptr = n_ptr->sibling;
   }

   return(closed);
}

///////////////////////////////////////////////////////////////////////////////

void Node::centroid(double &ubar, double &vbar) const {
   double sa, sum_x, sum_y;

   sum_x = sum_y = 0.0;

   sum_first_moments(sum_x, sum_y);

   sum_x /= 6.0;
   sum_y /= 6.0;

   sa = uv_signed_area();

   ubar = sum_x/sa;
   vbar = sum_y/sa;

   return;
}

///////////////////////////////////////////////////////////////////////////////

void Node::translate(double du, double dv) {
   Node *n_ptr = (Node *) 0;

   // Translate each point of the current polyline
   p.translate(du, dv);

   // Apply translation to all descendants
   n_ptr = child;

   while(n_ptr) {
      n_ptr->translate(du, dv);

      n_ptr = n_ptr->sibling;
   }

   return;
}

///////////////////////////////////////////////////////////////////////////////

double Node::angle() const {
   double a, sa, Ixx, Ixy, Iyy, x_bar, y_bar;

   if(p.n_points < 3 && n_children() == 0) {

      mlog << Error << "\nNode::angle() -> "
	   << "not enough points!\n\n";

      exit(1);
   }

   if(p.n_points == 0) {

      x_bar = y_bar = 0.0;
   }
   else {

      // Get the centroid of the current polyline
      p.centroid(x_bar, y_bar);
   }

   Ixx = Ixy = Iyy = 0.0;

   sum_second_moments(x_bar, y_bar, Ixx, Ixy, Iyy);

   sa = uv_signed_area();

   Ixx /= 12.0*sa;
   Iyy /= 12.0*sa;
   Ixy /= 24.0*sa;

   a = 0.5*deg_per_rad*atan2( 2.0*Ixy, Ixx - Iyy );

   return(a);
}

///////////////////////////////////////////////////////////////////////////////

void Node::rotate(double deg) {
   double ubar, vbar;

   centroid(ubar, vbar);

   rotate(deg, ubar, vbar);

   return;
}

///////////////////////////////////////////////////////////////////////////////

void Node::rotate(double deg, double ubar, double vbar) {
   Node *n_ptr = (Node *) 0;

   p.rotate(deg, ubar, vbar);

   // Calculate area for all descendants
   n_ptr = child;

   while(n_ptr) {
      n_ptr->rotate(deg, ubar, vbar);

      n_ptr = n_ptr->sibling;
   }

   return;
}

///////////////////////////////////////////////////////////////////////////////

double Node::uv_signed_area() const {
   double sum;
   Node *n_ptr = (Node *) 0;

   sum = p.uv_signed_area();

   // Calculate area for all descendants
   n_ptr = child;

   while(n_ptr) {
      sum += n_ptr->uv_signed_area();

      n_ptr = n_ptr->sibling;
   }

   return(sum);
}

///////////////////////////////////////////////////////////////////////////////

int Node::is_inside(double u_test, double v_test) const {
   int count;
   Node *n_ptr = (Node *) 0;

   if(p.is_inside(u_test, v_test)) {

      if(p.uv_signed_area() > 0.0) {
         count = 1;
      }
      else {
         count = -1;
      }
   }
   else {
      count = 0;
   }

   // Apply is_inside function for all descendants
   n_ptr = child;

   while(n_ptr) {

      count += n_ptr->is_inside(u_test, v_test);

      n_ptr = n_ptr->sibling;
   }

   return(count);
}

///////////////////////////////////////////////////////////////////////////////

int Node::is_polyline_point(double u_test, double v_test) const {
   int poly_point;
   Node *n_ptr = (Node *) 0;

   poly_point = p.is_polyline_point(u_test, v_test);

   // Check if it's contained in the Polyline of descendants
   n_ptr = child;

   while(n_ptr) {

      if(n_ptr->is_polyline_point(u_test, v_test)) poly_point += 1;

      n_ptr = n_ptr->sibling;
   }

   return(poly_point);
}

///////////////////////////////////////////////////////////////////////////////

void Node::bounding_box(Box &bb) const {
   Node *n_ptr = (Node *) 0;

   p.bounding_box(bb);

   // Search for min and max for all descendents
   n_ptr = child;

   while(n_ptr) {

      n_ptr->bounding_box(bb);

      n_ptr = n_ptr->sibling;
   }

   // Calculate width and height
   // bb.width = bb.x_ur - bb.x_ll;
   // bb.height = bb.y_ur - bb.y_ll;

   return;
}

///////////////////////////////////////////////////////////////////////////////

void Node::sum_first_moments(double &sum_x, double &sum_y) const {
   Node *n_ptr = (Node *) 0;

   p.sum_first_moments(sum_x, sum_y);

   // Sum the first moments for all descendants
   n_ptr = child;

   while(n_ptr) {
      n_ptr->sum_first_moments(sum_x, sum_y);

      n_ptr = n_ptr->sibling;
   }

   return;
}

////////////////////////////////////////////////////////////////////////////////

void Node::sum_second_moments(double x_bar, double y_bar,
                              double &Ixx, double &Ixy, double &Iyy) const {
   Node *n_ptr = (Node *) 0;

   p.sum_second_moments(x_bar, y_bar, Ixx, Ixy, Iyy);

   // Sum the second moments for all descendants
   n_ptr = child;

   while(n_ptr) {
      n_ptr->sum_second_moments(x_bar, y_bar, Ixx, Ixy, Iyy);

      n_ptr = n_ptr->sibling;
   }

   return;
}

///////////////////////////////////////////////////////////////////////////////
//
// Code for misc functions
//
///////////////////////////////////////////////////////////////////////////////

double node_dist(const Node &a, const Node &b) {
   double min_dist, dist;
   int i_a, i_b, num_a, num_b;
   Node *a_ptr = (Node *) 0, *b_ptr = (Node *) 0;

   num_a = a.n_children();
   num_b = b.n_children();

   if(num_a == 0 || num_b == 0) {
      mlog << Error << "\ndouble node_dist(Node &, Node &) -> "
           << "encountered empty node tree\n\n";

      exit(1);
   }

   min_dist = -1.0;

   for(i_a=0; i_a<num_a; i_a++) {

      a_ptr = a.get_child(i_a);

      for(i_b=0; i_b<num_b; i_b++) {

         b_ptr = b.get_child(i_b);

         dist = polyline_dist(a_ptr->p, b_ptr->p);

         if(is_eq(min_dist, -1.0)) min_dist = dist;
         else if(dist < min_dist)  min_dist = dist;

      } // end for i_b
   } // end for i_a

   return(min_dist);
}

///////////////////////////////////////////////////////////////////////////////

double node_polyline_dist(const Node &a, const Polyline &b) {
   double min_dist, dist;
   int i_a, num_a;
   Node *a_ptr = (Node *) 0;

   num_a = a.n_children();

   if(num_a == 0) {
      mlog << Error << "\ndouble node_polyline_dist(Node &, Node &) -> "
           << "encountered empty node tree\n\n";

      exit(1);
   }

   min_dist = -1.0;

   for(i_a=0; i_a<num_a; i_a++) {

      a_ptr = a.get_child(i_a);

      dist = polyline_dist(a_ptr->p, b);

      if(is_eq(min_dist, -1.0)) min_dist = dist;
      else if(dist < min_dist)  min_dist = dist;

   } // end for i_a

   return(min_dist);
}

///////////////////////////////////////////////////////////////////////////////
