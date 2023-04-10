// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2023
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*



////////////////////////////////////////////////////////////////////////


#ifndef  __AFM_LINE_H__
#define  __AFM_LINE_H__


////////////////////////////////////////////////////////////////////////


#include <iostream>

#include "afm_token.h"
#include "concat_string.h"

////////////////////////////////////////////////////////////////////////


class AfmLine {

   friend std::istream & operator>>(std::istream &, AfmLine &);

   private:

      void init_from_scratch();

      void assign(const AfmLine &);

      ConcatString Line;

      StringArray LineItems;

      unsigned int item_index;

      int LineLength;


   public:

      AfmLine();
     ~AfmLine();
      AfmLine(const AfmLine &);
      AfmLine & operator=(const AfmLine &);

      int line_number;

      void clear();

      void split_line();

      AfmToken nexttoken();

      AfmToken rest_as_string();

      int is_ok() const;

};


////////////////////////////////////////////////////////////////////////


extern std::istream & operator>>(std::istream &, AfmLine &);


////////////////////////////////////////////////////////////////////////


#endif   //  __AFM_LINE_H__


////////////////////////////////////////////////////////////////////////


