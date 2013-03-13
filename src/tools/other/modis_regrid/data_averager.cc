

////////////////////////////////////////////////////////////////////////


using namespace std;

#include <iostream>
#include <unistd.h>
#include <stdlib.h>
#include <cmath>

#include "data_averager.h"

#include "vx_math.h"


////////////////////////////////////////////////////////////////////////


   //
   //  Code for class DataAverager
   //


////////////////////////////////////////////////////////////////////////


DataAverager::DataAverager()

{

init_from_scratch();

}


////////////////////////////////////////////////////////////////////////


DataAverager::~DataAverager()

{

clear();

}


////////////////////////////////////////////////////////////////////////


void DataAverager::init_from_scratch()

{

grid = 0;

Counts = 0;

Sum = 0;

clear();

return;

}


////////////////////////////////////////////////////////////////////////


void DataAverager::clear()

{

grid = 0;

if ( Sum )  { delete [] Sum;   Sum = 0; }

if ( Counts )  { delete [] Counts;   Counts = 0; }

Nx = Ny = 0;


return;

}


////////////////////////////////////////////////////////////////////////


void DataAverager::set(const Grid & _grid)

{

int j;

clear();

grid = &_grid;

Nx = grid->nx();
Ny = grid->ny();

const int nxy = Nx*Ny;

Sum = new double [nxy];

Counts = new int [nxy];

for (j=0; j<nxy; ++j)  {

   Counts[j] = 0;

   Sum[j] = 0.0;

}

   //
   //  done
   //

return;

}


////////////////////////////////////////////////////////////////////////


void DataAverager::put(double value, int x, int y)

{

if ( (x < 0) || (x >= Nx) || (y < 0) || (y >= Ny) )  return;

const int n = two_to_one(x, y);

Sum[n] += value;

Counts[n] += 1;

return;

}


////////////////////////////////////////////////////////////////////////


void DataAverager::put(double value, double lat, double lon)

{

double dx, dy;


grid->latlon_to_xy(lat, lon, dx, dy);

const int x = nint(dx);
const int y = nint(dy);

put(value, x, y);


return;

}


////////////////////////////////////////////////////////////////////////


double DataAverager::ave(int x, int y) const

{

const int n = two_to_one(x, y);

const int c = Counts[n];

if ( c == 0 )  return ( 0.0 );

return ( (Sum[n])/c );

}


////////////////////////////////////////////////////////////////////////


