// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2020
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*


////////////////////////////////////////////////////////////////////////


#ifndef  __MET_DATA_TWO_TO_ONE_H__
#define  __MET_DATA_TWO_TO_ONE_H__


////////////////////////////////////////////////////////////////////////


#include <iostream>
#include <unistd.h>
#include <stdlib.h>
#include <cmath>

#include "vx_log.h"


////////////////////////////////////////////////////////////////////////


class TwoOne;   //  forward declaration


////////////////////////////////////////////////////////////////////////


extern TwoOne DefaultTO;


////////////////////////////////////////////////////////////////////////


typedef int  (*TwoToOneFunction)(const int Nx, const int Ny, const int x, const int y);

typedef void (*OneToTwoFunction)(const int Nx, const int Ny, const int n, int & x, int & y);


////////////////////////////////////////////////////////////////////////


class TwoOne {

   private:

      void init_from_scratch();

      void assign(const TwoOne &);

      TwoToOneFunction TO;

      OneToTwoFunction OT;

   public:

      TwoOne();
      TwoOne(TwoToOneFunction, OneToTwoFunction);
     ~TwoOne();
      TwoOne(const TwoOne &);
      TwoOne & operator=(const TwoOne &);

      void clear();

      void set(int xdir, int ydir, int order);

      void set(TwoToOneFunction, OneToTwoFunction);

      void one_to_two (const int Nx, const int Ny, const int n, int & x, int & y) const;

      int  two_to_one (const int Nx, const int Ny, const int x, const int y) const;

};


////////////////////////////////////////////////////////////////////////


inline void TwoOne::one_to_two (const int _Nx, const int _Ny, const int _n, int & _x, int & _y) const

{

if ( !OT )  {

   mlog << Error << "\nTwoOne::one_to_two() const -> "
        << "no function set!\n\n";

   exit ( 1 );

}

OT (_Nx, _Ny, _n, _x, _y);

return;

}


////////////////////////////////////////////////////////////////////////


inline int TwoOne::two_to_one (const int _Nx, const int _Ny, const int _x, const int _y) const

{

if ( !TO )  {

   mlog << Error << "\nTwoOne::two_to_one() const -> "
        << "no function set!\n\n";

   exit ( 1 );

}

return ( TO(_Nx, _Ny, _x, _y) );

}


////////////////////////////////////////////////////////////////////////


extern int two_to_one_0_0_0(const int Nx, const int Ny, const int x, const int y);  // +x, -y, x fastest
extern int two_to_one_0_0_1(const int Nx, const int Ny, const int x, const int y);  // +x, -y, y fastest

extern int two_to_one_0_1_0(const int Nx, const int Ny, const int x, const int y);  // +x, +y, x fastest
extern int two_to_one_0_1_1(const int Nx, const int Ny, const int x, const int y);  // +x, +y, y fastest

extern int two_to_one_1_0_0(const int Nx, const int Ny, const int x, const int y);  // -x, -y, x fastest
extern int two_to_one_1_0_1(const int Nx, const int Ny, const int x, const int y);  // -x, -y, y fastest

extern int two_to_one_1_1_0(const int Nx, const int Ny, const int x, const int y);  // -x, +y, x fastest
extern int two_to_one_1_1_1(const int Nx, const int Ny, const int x, const int y);  // -x, +y, y fastest


extern void one_to_two_0_0_0(const int Nx, const int Ny, const int n, int & x, int & y);  // +x, -y, x fastest
extern void one_to_two_0_0_1(const int Nx, const int Ny, const int n, int & x, int & y);  // +x, -y, y fastest

extern void one_to_two_0_1_0(const int Nx, const int Ny, const int n, int & x, int & y);  // +x, +y, x fastest
extern void one_to_two_0_1_1(const int Nx, const int Ny, const int n, int & x, int & y);  // +x, +y, y fastest

extern void one_to_two_1_0_0(const int Nx, const int Ny, const int n, int & x, int & y);  // -x, -y, x fastest
extern void one_to_two_1_0_1(const int Nx, const int Ny, const int n, int & x, int & y);  // -x, -y, y fastest

extern void one_to_two_1_1_0(const int Nx, const int Ny, const int n, int & x, int & y);  // -x, +y, x fastest
extern void one_to_two_1_1_1(const int Nx, const int Ny, const int n, int & x, int & y);  // -x, +y, y fastest


///////////////////////////////////////////////////////////////////////////////


extern TwoToOneFunction get_two_to_one(int xdir, int ydir, int order);
extern OneToTwoFunction get_one_to_two(int xdir, int ydir, int order);


///////////////////////////////////////////////////////////////////////////////


#endif   /*  __MET_DATA_TWO_TO_ONE_H__  */


///////////////////////////////////////////////////////////////////////////////


