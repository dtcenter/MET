// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2020
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*


////////////////////////////////////////////////////////////////////////


#ifndef  __CONCAT_STRING_H__
#define  __CONCAT_STRING_H__


////////////////////////////////////////////////////////////////////////


#include <iostream>
#include <sstream>
#include <string>
#include <stdarg.h>

#include "string_array.h"
#include "indent.h"


////////////////////////////////////////////////////////////////////////


   //
   //  minimum and default allocation increment for the ConcatString class
   //


static const int min_cs_alloc_inc     =  32;

static const int default_cs_alloc_inc = 128;

static const int max_str_len          = 512;


////////////////////////////////////////////////////////////////////////


static const int concat_string_default_precision =  5;

static const int concat_string_max_precision     = 12;


////////////////////////////////////////////////////////////////////////


enum CSInlineCommand {

   cs_erase,
   cs_clear

};


////////////////////////////////////////////////////////////////////////


class ConcatString {

   private:

      void init_from_scratch();

      void assign(const ConcatString &);

      void extend(int);

      int AllocInc;

      int Precision;

      char FloatFormat[16];

      std::string *s;

   public:

      ConcatString();
      ConcatString(int _alloc_inc);
     ~ConcatString();
      ConcatString(const ConcatString &);
      ConcatString(const std::string &);
      ConcatString(const char *);
      ConcatString & operator=(const ConcatString &);
      ConcatString & operator=(const std::string &);
      ConcatString & operator=(const char *);
      bool operator==(const ConcatString &) const;
      bool operator==(const char *) const;


      void clear();

         //
         //  set stuff
         //

      void set_alloc_inc(int);

      void set_precision(int);

         //
         //  get stuff
         //

      const char * text() const;

      const char * c_str() const;

      const std::string & string() const;

      const std::string contents(const char *str = 0) const;   //  returns str or "(nul)" if the string is empty

      int length() const;   //  not including trailing nul

      int precision() const;

      const char * float_format() const;

      int alloc_inc() const;

      bool empty() const;
      bool nonempty() const;

      char char_at(const int idx) const;  //  returns nul if outside range

      char operator [] (const int) const;  //  calls exit() if outside range

         //
         //  do stuff
         //

      void add(const char);

      void add(const ConcatString &);

      void add(const std::string &);

      void add(const char *);

      void chomp();   //  removes possible trailing newline

      void chomp(const char);   //  removes trailing char, if possible

      void chomp(const char *);   //  removes trailing suffix, if possible

      operator const std::string () const;

      bool startswith(const char *) const;
      bool   endswith(const char *) const;

      void ws_strip();   //  strip leading and trailing whitespace, if any

      void strip_cpp_comment();   //  strip trailing C++ comment, if any

      void strip_paren();   //  strip contents of trailing parenthesis, if any

      StringArray split(const char * delim) const;

      ConcatString dirname() const;

      ConcatString basename() const;

      void set_repeat(char, int count);

      void erase();

      void elim_trailing_whitespace();

      int format(const char *format, ...);

      bool read_line(std::istream &);   //  read a line from the input stream

      void replace_char(int i, char c);
      //  replace all occurences of target with replacement
      //  if "replacement" is an environment variable, use it's value
      void replace(const char * target, const char * replacement, bool check_env = true);

      void set_upper();
      void set_lower();

      int find(int c);
      int compare(size_t pos, size_t len, std::string str);
      int comparecase(size_t pos, size_t len, std::string str);
      int comparecase(const char *);

};


////////////////////////////////////////////////////////////////////////


inline const char * ConcatString::text()          const { return ( s ? s->c_str() : 0); }
inline const char * ConcatString::c_str()         const { return ( s ? s->c_str() : 0); }
inline const std::string & ConcatString::string() const { return ( *s ); }

inline int          ConcatString::length()       const { return ( (int) (s->length()) ); }

inline int          ConcatString::precision()    const { return ( Precision ); }

inline const char * ConcatString::float_format() const { return ( FloatFormat ); }

inline int          ConcatString::alloc_inc()    const { return ( AllocInc ); }

inline bool         ConcatString::empty()        const { return ( s ?  s->empty() : true ); }
inline bool         ConcatString::nonempty()     const { return ( s ? !s->empty() : false ); }

inline              ConcatString::operator const std::string () const { return ( s ? *s : 0 ); }


////////////////////////////////////////////////////////////////////////


extern ConcatString to_upper(const ConcatString &);

extern ConcatString to_lower(const ConcatString &);


   //
   //  writing a ConcatString to an ostream
   //


extern std::ostream & operator<<(std::ostream &, const ConcatString &);


////////////////////////////////////////////////////////////////////////


   //
   //  writing things into a ConcatString
   //


extern ConcatString & operator<< (ConcatString &, const char);
extern ConcatString & operator<< (ConcatString &, const char *);
extern ConcatString & operator<< (ConcatString &, const std::string&);
extern ConcatString & operator<< (ConcatString &, const ConcatString &);
extern ConcatString & operator<< (ConcatString &, int);
extern ConcatString & operator<< (ConcatString &, unsigned int);
extern ConcatString & operator<< (ConcatString &, long long);
extern ConcatString & operator<< (ConcatString &, double);

extern ConcatString & operator<< (ConcatString &, CSInlineCommand);
extern ConcatString & operator<< (ConcatString &, const Indent &);


   //
   //  string comparison functions
   //


extern bool operator == (const char *, const ConcatString &);

extern bool operator != (const ConcatString &, const ConcatString &);
extern bool operator != (const ConcatString &, const char *);
extern bool operator != (const char *, const ConcatString &);

extern bool operator <= (const ConcatString &, const ConcatString &);
extern bool operator <= (const ConcatString &, const char *);
extern bool operator <= (const char *, const ConcatString &);

extern bool operator >= (const ConcatString &, const ConcatString &);
extern bool operator >= (const ConcatString &, const char *);
extern bool operator >= (const char *, const ConcatString &);

extern bool operator <  (const ConcatString &, const ConcatString &);
extern bool operator <  (const ConcatString &, const char *);
extern bool operator <  (const char *, const ConcatString &);

extern bool operator >  (const ConcatString &, const ConcatString &);
extern bool operator >  (const ConcatString &, const char *);
extern bool operator >  (const char *, const ConcatString &);

extern ConcatString write_css(const StringArray &);

extern bool get_env(const char *, ConcatString &);


////////////////////////////////////////////////////////////////////////


#endif   //  __CONCAT_STRING_H__


////////////////////////////////////////////////////////////////////////


