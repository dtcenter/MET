// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2020
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
#include <ostream>
#include <vector>
#include <algorithm>

////////////////////////////////////////////////////////////////////////


class StringArray {

   public:


      void init_from_scratch();

      void assign(const StringArray &);

      std::vector<std::string> s;

      int  Nalloc;

      int  MaxLength;

      bool IgnoreCase;


   public:

      StringArray();
     ~StringArray();
      StringArray(const StringArray &);
      StringArray & operator=(const StringArray &);
      bool operator==(const StringArray &) const;

      void clear();

      void dump(std::ostream &, int depth = 0) const;

      const std::string operator[](int) const;

      void set_ignore_case(const bool);

      void add(const std::string text);
      
      void add(const StringArray &);

      void add_css(const std::string);

      void set(int i, const std::string);

      void insert(int i, const char *);

      int n_elements() const;
      int n() const;   //  same thing

      int max_length() const;

      int length(int) const;

      bool has(const std::string, bool forward=true) const;

      bool has(const std::string, int & index, bool forward=true) const;

         //
         //  parse delimited strings
         //

      void parse_wsss(const std::string);

      void parse_css(const std::string);

      void parse_delim(const std::string, const char *delim);

         //
         //  for use when parsing command-line switches
         //

      void shift_down(int pos, int shift);

      bool has_option(int & index) const;

         //
         //  for use with a list of regular expressions
         //

      bool reg_exp_match(const char *) const;

         //
         //  sort in increasing lex order  (uses qsort)
         //

      void sort();

         //
         //  return a unique subset of strings
         //

      StringArray uniq();

};


////////////////////////////////////////////////////////////////////////


inline int StringArray::n_elements() const { return ( (int) (s.size()) ); }

inline int StringArray::n         () const { return ( s.size() ); }

inline int StringArray::max_length() const { return ( MaxLength ); }


////////////////////////////////////////////////////////////////////////


extern bool check_reg_exp(const char *, const char *);


////////////////////////////////////////////////////////////////////////


#endif   /*  __STRING_ARRAY_H__  */


////////////////////////////////////////////////////////////////////////


