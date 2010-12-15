

////////////////////////////////////////////////////////////////////////


#ifndef  __ECONFIG_RESULT_H__
#define  __ECONFIG_RESULT_H__


////////////////////////////////////////////////////////////////////////


#include <iostream>

#include "misc.h"


////////////////////////////////////////////////////////////////////////


enum ResultType {

   result_int,

   result_boolean,

   result_double,

   result_string, 

   no_result_type

};


////////////////////////////////////////////////////////////////////////


class Result {

   private:

      void init_from_scratch();

      void assign(const Result &);


      int Ival;

      double Dval;

      ConcatString Sval;


      ResultType Type;

   public:

      Result();
      Result(int);
      Result(bool);
      Result(double);
      Result(const char *);
     ~Result();
      Result(const Result &);
      Result & operator=(const Result &);


      void clear();

      void dump(ostream &, int depth = 0) const;

      void set_int(int);

      void set_boolean(bool);

      void set_double(double);

      void set_string(const char *);

      ResultType type() const;



      int ival() const;

      bool bval() const;

      double dval() const;

      const char * sval() const;

      operator int          () const;
      operator bool         () const;
      operator double       () const;
      operator const char * () const;

};


////////////////////////////////////////////////////////////////////////


inline ResultType Result::type() const { return ( Type ); }


////////////////////////////////////////////////////////////////////////


extern ostream & operator<<(ostream &, const Result &);


////////////////////////////////////////////////////////////////////////


#endif   //  __ECONFIG_RESULT_H__


////////////////////////////////////////////////////////////////////////





