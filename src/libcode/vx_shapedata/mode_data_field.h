// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2024
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*


////////////////////////////////////////////////////////////////////////


#ifndef  __MODE_DATA_FIELD_H__
#define  __MODE_DATA_FIELD_H__


////////////////////////////////////////////////////////////////////////


#include <iostream>


////////////////////////////////////////////////////////////////////////


class ModeDataField {

   private:

      void init_from_scratch();

      void assign(const ModeDataField &);

   public:

      ModeDataField();
     ~ModeDataField();
      ModeDataField(const ModeDataField &);
      ModeDataField & operator=(const ModeDataField &);

      void clear();

      void dump(std::ostream &, int depth = 0) const;

};


////////////////////////////////////////////////////////////////////////


#endif   /*  __MODE_DATA_FIELD_H__  */


////////////////////////////////////////////////////////////////////////


