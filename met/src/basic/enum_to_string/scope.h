// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2019
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*


////////////////////////////////////////////////////////////////////////


#ifndef  __ENUM_SCOPE_STACK_H__
#define  __ENUM_SCOPE_STACK_H__


////////////////////////////////////////////////////////////////////////


#include <iostream>


////////////////////////////////////////////////////////////////////////


class ScopeStackElement {

   private:

      const char * Name;   //  if any

      int Level;   //  bracket depth

      void assign(const ScopeStackElement &);

   public:

      ScopeStackElement();
     ~ScopeStackElement();
      ScopeStackElement(const ScopeStackElement &);
      ScopeStackElement & operator=(const ScopeStackElement &);


      void set_name(const char *);

      void set_level(int);

      const char * name() const;

      int level() const;

      void clear();

};


////////////////////////////////////////////////////////////////////////


inline int ScopeStackElement::level() const { return ( Level ); }

inline const char * ScopeStackElement::name() const { return ( Name ); }


////////////////////////////////////////////////////////////////////////


extern ostream & operator<<(ostream &, const ScopeStackElement &);


////////////////////////////////////////////////////////////////////////


static const int max_scope_stack_depth = 10;


////////////////////////////////////////////////////////////////////////


class ScopeStack {

   private:

      ScopeStackElement s[max_scope_stack_depth];

      int N;

      void assign(const ScopeStack &);

   public:

      ScopeStack();
     ~ScopeStack();
      ScopeStack(const ScopeStack &);
      ScopeStack & operator=(const ScopeStack &);

      void pop();
      void push(const ScopeStackElement &);

      void level_push(const ScopeStackElement &);

      void clear_to_level(int);

      void clear();

      int n_elements() const;

      const ScopeStackElement & peek(int pos) const;

};


////////////////////////////////////////////////////////////////////////


inline int ScopeStack::n_elements() const { return ( N ); }


////////////////////////////////////////////////////////////////////////


extern ostream & operator<<(ostream &, const ScopeStack &);


////////////////////////////////////////////////////////////////////////


#endif   /*  __ENUM_SCOPE_STACK_H__  */


////////////////////////////////////////////////////////////////////////



