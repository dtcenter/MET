// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2019
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*



////////////////////////////////////////////////////////////////////////


#ifndef  __AFM_TOKEN_H__
#define  __AFM_TOKEN_H__


////////////////////////////////////////////////////////////////////////


#include <iostream>

#include "afm_token_types.h"
#include "afm_keywords.h"
#include "concat_string.h"

////////////////////////////////////////////////////////////////////////


class AfmToken {

   private:

      void init_from_scratch();

      void assign(const AfmToken &);

   public:

      AfmToken();
     ~AfmToken();
      AfmToken(const AfmToken &);
      AfmToken & operator=(const AfmToken &);

      void clear();

      void dump(ostream &, int depth = 0) const;

      //      void set_string(const char *);
      void set_string(const ConcatString);

      int line_number;
      int column;

      double as_double() const;

         //
         //  token type
         //

      AfmTokenType type;

         //
         //  token values
         //

      AfmKeyword keyword;

      int i;

      double d;

      //      char * s;
      ConcatString s;

};


////////////////////////////////////////////////////////////////////////


#endif   //  __AFM_TOKEN_H__


////////////////////////////////////////////////////////////////////////


