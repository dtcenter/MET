// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2016
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*



////////////////////////////////////////////////////////////////////////


#ifndef  __STRING_ARRAY_H__
#define  __STRING_ARRAY_H__


////////////////////////////////////////////////////////////////////////


#include <iostream>


////////////////////////////////////////////////////////////////////////


class StringArray {

   public:


      void init_from_scratch();

      void assign(const StringArray &);


      char ** s;

      int  Nelements;

      int  Nalloc;

      int  MaxLength;

      bool IgnoreCase;


   public:

      StringArray();
     ~StringArray();
      StringArray(const StringArray &);
      StringArray & operator=(const StringArray &);

      void clear();

      void extend(int);

      void dump(ostream &, int depth = 0) const;

      const char * operator[](int) const;

      void set_ignore_case(const bool);

      void add(const char *);

      void add(const StringArray &);

      void add_css(const char *);

      void set(int i, const char *);

      void insert(int i, const char *);

      int n_elements() const;
      int n() const;   //  same thing

      int max_length() const;

      int length(int) const;

      int has(const char *) const;

      int has(const char *, int & index) const;

         //
         //  parse delimited strings
         //

      void parse_wsss(const char *);

      void parse_css(const char *);

      void parse_delim(const char *, const char *delim);

         //
         //  for use when parsing command-line switches
         //

      void shift_down(int pos, int shift);

      int has_option(int & index) const;

         //
         //  sort in increasing lex order  (uses qsort)
         //

      void sort();

};


////////////////////////////////////////////////////////////////////////


inline int StringArray::n_elements() const { return ( Nelements ); }

inline int StringArray::n         () const { return ( Nelements ); }

inline int StringArray::max_length() const { return ( MaxLength ); }


////////////////////////////////////////////////////////////////////////


#endif   /*  __STRING_ARRAY_H__  */


////////////////////////////////////////////////////////////////////////


