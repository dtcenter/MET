// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2019
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*



////////////////////////////////////////////////////////////////////////


#ifndef  __INDENT_H__
#define  __INDENT_H__


////////////////////////////////////////////////////////////////////////


#include <iostream>


////////////////////////////////////////////////////////////////////////


static const int   default_indent_delta = 5;


static const char  default_on_char      = '|';
static const char  default_off_char     = ' ';


////////////////////////////////////////////////////////////////////////


class Indent {

   private:

      void init_from_scratch();

      void assign(const Indent &);

   public:

      Indent();
      Indent(int);   //  depth
     ~Indent();
      Indent(const Indent &);
      Indent & operator=(const Indent &);

      void clear();

      int depth;

      int delta;

      char on_char;
      char off_char;

};


////////////////////////////////////////////////////////////////////////


extern std::ostream & operator<<(std::ostream &, const Indent &);


////////////////////////////////////////////////////////////////////////


#endif   //  __INDENT_H__


////////////////////////////////////////////////////////////////////////


