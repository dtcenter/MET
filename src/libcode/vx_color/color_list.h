// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2019
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*



////////////////////////////////////////////////////////////////////////


#ifndef  __COLOR_LIST_H__
#define  __COLOR_LIST_H__


////////////////////////////////////////////////////////////////////////


#include <iostream>

#include "color_parser.h"
#include "concat_string.h"

////////////////////////////////////////////////////////////////////////


class ClistEntry {

   private:

      void init_from_scratch();

      void assign(const ClistEntry &);

      Dcolor D;

      ConcatString Name;

   public:

      ClistEntry();
     ~ClistEntry();
      ClistEntry(const ClistEntry &);
      ClistEntry & operator=(const ClistEntry &);

      void clear();

      void dump(ostream &, int depth = 0) const;

      void set_name(const std::string);

      void set_color(const Dcolor &);

      Dcolor dc() const;

      const char * name() const;

};


////////////////////////////////////////////////////////////////////////


inline Dcolor ClistEntry::dc() const { return ( D ); }

inline const char * ClistEntry::name() const { return ( Name.c_str() ); }


////////////////////////////////////////////////////////////////////////


static const int colorlist_alloc_inc = 50;


////////////////////////////////////////////////////////////////////////


class ColorList {

   private:

      void init_from_scratch();

      void assign(const ColorList &);

      void extend(int);

      int Nelements;

      int Nalloc;

      ClistEntry * e;

   public:

      ColorList();
     ~ColorList();
      ColorList(const ColorList &);
      ColorList & operator=(const ColorList &);

      void clear();

      void dump(ostream &, int depth = 0) const;

      int n_elements() const;

      ClistEntry operator[](int) const;

      void add(const ClistEntry &);

      int has_name(const std::string, int & index);

};


////////////////////////////////////////////////////////////////////////


inline int ColorList::n_elements() const { return ( Nelements ); }


////////////////////////////////////////////////////////////////////////


#endif   //  __COLOR_LIST_H__


////////////////////////////////////////////////////////////////////////


