// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2011
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*



////////////////////////////////////////////////////////////////////////


using namespace std;

#include <iostream>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <cmath>

#include "indent.h"
#include "bitarray.h"


////////////////////////////////////////////////////////////////////////


   //
   //  Code for class BitArray
   //


////////////////////////////////////////////////////////////////////////


BitArray::BitArray()

{

init_from_scratch();

}


////////////////////////////////////////////////////////////////////////


BitArray::~BitArray()

{

clear();

}


////////////////////////////////////////////////////////////////////////


BitArray::BitArray(const BitArray & b)

{

init_from_scratch();

assign(b);

}


////////////////////////////////////////////////////////////////////////


BitArray & BitArray::operator=(const BitArray & b)

{

if ( this == &b )  return ( * this );

assign(b);

return ( * this );

}


////////////////////////////////////////////////////////////////////////


void BitArray::init_from_scratch()

{

u = (unsigned char *) 0;

Nbits = Nalloc = 0;


return;

}


////////////////////////////////////////////////////////////////////////


void BitArray::clear()

{

if ( u )  { delete [] u;  u = (unsigned char *) 0; }

Nbits = Nalloc = 0;

return;

}


////////////////////////////////////////////////////////////////////////


void BitArray::assign(const BitArray & b)

{

clear();

if ( !(b.u) )  return;

set_size(b.Nbits);

memcpy(u, b.u, Nalloc);

   //
   //  done
   //

return;

}


////////////////////////////////////////////////////////////////////////


void BitArray::dump(ostream & out, int depth) const

{

Indent prefix(depth);


out << prefix << "Nbits  = " << Nbits  << "\n";
out << prefix << "Nalloc = " << Nalloc << "\n";

if ( u )  {

   int j, k;
   unsigned char mask;

   if ( Nalloc < 100 )  {

      for (j=0; j<Nalloc; ++j)  {

         out << prefix << "u[" << j << "] = ";

         for (k=0; k<8; ++k)  {

            mask = (unsigned char) (1 << (7 - k));

            if ( (u[j]) & mask )  out.put('1');
            else                  out.put('0');

            if ( k == 3 )  out.put(' ');

         }   //  for k

         out.put('\n');

      }   //  for j

   }   //  if Nalloc

}   //  if u

   //
   //  done
   //

out.flush();

return;

}


////////////////////////////////////////////////////////////////////////


void BitArray::set_size(int n)

{

if ( n <= 0 )  {

   cerr << "\n\n  void BitArray::set_size() -> bad size ... " << n << "\n\n";

   exit ( 1 );

}

clear();

Nbits = n;

Nalloc = (Nbits + 7)/8;

u = new unsigned char [Nalloc];

if ( !u )  {

   cerr << "\n\n  BitArray::set_size(int) -> memory allocation error\n\n";

   exit ( 1 );

}

memset(u, 0, Nalloc);

   //
   //  done
   //

return;

}


////////////////////////////////////////////////////////////////////////


void BitArray::set_all_zeroes()

{

if ( !u )  return;

memset(u, 0, Nalloc);


return;

}


////////////////////////////////////////////////////////////////////////


void BitArray::set_all_ones()

{

if ( !u )  return;

memset(u, 255, Nalloc);

   //
   //  want to keep the "extra" bits in the last byte set to zero
   //    otherwise the count of the "on" bits might
   //    get screwed up
   //

int j;

u[Nalloc - 1] = (unsigned char) 0;

for (j=0; j<(Nbits%8); ++j)  {

   set_bit(8*(Nbits/8) + j, 1);

}



return;

}


////////////////////////////////////////////////////////////////////////


void BitArray::set_bit(int bitno, int onoff)

{

if ( (bitno < 0) || (bitno >= Nbits) )  {

   cerr << "\n\n  BitArray::set_bit() -> bad bit number ... " << bitno << "\n\n";

   exit ( 1 );

}

int byteno;
unsigned char mask;

byteno = bitno/8;

mask = (unsigned char) (1 << (7 - bitno%8));

if ( onoff )  u[byteno] |= mask;
else          u[byteno] &= ~mask;

return;

}


////////////////////////////////////////////////////////////////////////


int BitArray::operator[](int bitno) const

{

if ( (bitno < 0) || (bitno >= Nbits) )  {

   cerr << "\n\n  BitArray::operator[](int) const -> bad bit number ... " << bitno << "\n\n";

   exit ( 1 );

}

int byteno;
unsigned char mask;

byteno = bitno/8;

mask = (unsigned char) (1 << (7 - bitno%8));

if ( (u[byteno] ) & mask )  return ( 1 );

return ( 0 );

}


////////////////////////////////////////////////////////////////////////


int BitArray::n_bits_on() const

{

   //
   //  this algorithm depends on the last unused bits
   //    in the last byte being zero
   //

if ( !u )  return ( 0 );

int j, count;

count = 0;

if ( Nalloc < 256 )  {

   for (j=0; j<Nalloc; ++j)  {

      count += bob(u[j]);

   }

   return ( count );

}

   //
   //  general case
   //

int bobcount[256];

for (j=0; j<256; ++j)  {

   bobcount[j] = bob((unsigned char) j);

}

count = 0;

for (j=0; j<Nalloc; ++j)  {

   count += bobcount[u[j]];

}


   //
   //  done
   //

return ( count );

}


////////////////////////////////////////////////////////////////////////


   //
   //  Code for misc functions
   //


////////////////////////////////////////////////////////////////////////


int bob(const unsigned char u)

{

int j, count;
unsigned char mask;


count = 0;

for (j=0; j<8; ++j)  {

   mask = (unsigned char) (1 << j);

   if ( u & mask )  ++count;

}


return ( count );

}


////////////////////////////////////////////////////////////////////////



