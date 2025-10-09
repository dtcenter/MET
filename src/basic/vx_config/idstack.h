////////////////////////////////////////////////////////////////////////
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2025
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
#include <vector>

#include "indent.h"

////////////////////////////////////////////////////////////////////////

class IdentifierQueue {

   private:

      void init_from_scratch();

      void assign(const IdentifierQueue &);

      std::vector<std::string> i;

   public:

      IdentifierQueue();
     ~IdentifierQueue();
      IdentifierQueue(const IdentifierQueue &);
      IdentifierQueue & operator=(const IdentifierQueue &);

      void push(const std::string &);

      std::string pop();

      int n_elements() const;

      void clear();

};

////////////////////////////////////////////////////////////////////////

inline int IdentifierQueue::n_elements() const { return (int) i.size(); }

////////////////////////////////////////////////////////////////////////

class IdentifierArray {

   private:

      void init_from_scratch(); 

      void assign(const IdentifierArray &);

      void extend(int);

      std::vector<std::string> i;

   public:

      IdentifierArray();
     ~IdentifierArray();
      IdentifierArray(const IdentifierArray &);
      IdentifierArray & operator=(const IdentifierArray &);

      int n_elements() const;

      void clear();

      void add(const char *);

      const std::string & operator[](int) const;

      void add(const std::string &);

      void dump(std::ostream &, int depth = 0) const;

      bool has(const char *) const;

      bool has(const char *, int & index) const;

};

////////////////////////////////////////////////////////////////////////

inline int IdentifierArray::n_elements() const { return (int) i.size(); }

////////////////////////////////////////////////////////////////////////

#endif   /*  __ID_STACK_H__  */

////////////////////////////////////////////////////////////////////////

