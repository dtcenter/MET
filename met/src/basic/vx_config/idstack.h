////////////////////////////////////////////////////////////////////////


// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2020
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*


////////////////////////////////////////////////////////////////////////


#ifndef  __ID_STACK_H__
#define  __ID_STACK_H__


////////////////////////////////////////////////////////////////////////


#include <string>

#include "indent.h"

using namespace std;

////////////////////////////////////////////////////////////////////////


static const int max_id_size = 128;


////////////////////////////////////////////////////////////////////////


class Identifier {

   private:

      void init_from_scratch();

      void assign(const Identifier &);

   public:

      Identifier();
     ~Identifier();
      Identifier(const Identifier &);
      Identifier & operator=(const Identifier &);

      string name;

      void clear();

      void set(const char *);

      void dump(ostream &, int depth = 0) const;

};


////////////////////////////////////////////////////////////////////////


static const int max_id_queue_size = 30;


////////////////////////////////////////////////////////////////////////


class IdentifierQueue {

   private:

      void init_from_scratch();

      void assign(const IdentifierQueue &);

      int Nelements;

      Identifier * i[max_id_queue_size];

   public:

      IdentifierQueue();
     ~IdentifierQueue();
      IdentifierQueue(const IdentifierQueue &);
      IdentifierQueue & operator=(const IdentifierQueue &);

      void push(const Identifier &);

      Identifier pop();

      int n_elements() const;

      void clear();

};


////////////////////////////////////////////////////////////////////////


inline int IdentifierQueue::n_elements() const { return ( Nelements ); }


////////////////////////////////////////////////////////////////////////


class IdentifierArray {

   private:

      void init_from_scratch();

      void assign(const IdentifierArray &);

      void extend(int);


      int Nelements;

      int Nalloc;

      Identifier * i;

   public:

      IdentifierArray();
     ~IdentifierArray();
      IdentifierArray(const IdentifierArray &);
      IdentifierArray & operator=(const IdentifierArray &);

      int n_elements() const;

      void clear();

      void add(const char *);

      const Identifier & operator[](int) const;

      void add(const Identifier &);

      void dump(ostream &, int depth = 0) const;

      bool has(const char *) const;

      bool has(const char *, int & index) const;

};


////////////////////////////////////////////////////////////////////////


inline int IdentifierArray::n_elements() const { return ( Nelements ); }


////////////////////////////////////////////////////////////////////////


#endif   /*  __ID_STACK_H__  */


////////////////////////////////////////////////////////////////////////





