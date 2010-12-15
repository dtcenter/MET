// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2007
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*



////////////////////////////////////////////////////////////////////////


#ifndef  __ECONFIG_RESULT_H__
#define  __ECONFIG_RESULT_H__


////////////////////////////////////////////////////////////////////////


#include <iostream>


////////////////////////////////////////////////////////////////////////


enum ResultType {

   result_int,

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

      char * Sval;   //  allocated

      ResultType Type;

   public:

      Result();
      Result(int);
      Result(double);
      Result(const char *);
     ~Result();
      Result(const Result &);
      Result & operator=(const Result &);


      void clear();

      void dump(ostream &, int depth = 0) const;

      void set_int(int);

      void set_double(double);

      void set_string(const char *);

      ResultType type() const;



      int ival() const;

      double dval() const;

      const char * sval() const;

      operator int          () const;
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





