// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2019
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*



////////////////////////////////////////////////////////////////////////


using namespace std;

#include <iostream>
#include <fstream>
#include <unistd.h>
#include <cstdlib>
#include <string.h>

#include "vx_log.h"
#include "uc_queue.h"


////////////////////////////////////////////////////////////////////////


   //
   //  Code for class UCQueue
   //


////////////////////////////////////////////////////////////////////////


UCQueue::UCQueue()

{

NElements = 0;

}


////////////////////////////////////////////////////////////////////////


UCQueue::~UCQueue()

{

clear();

}


////////////////////////////////////////////////////////////////////////


UCQueue::UCQueue(const UCQueue &Q)

{

assign(Q);

}


////////////////////////////////////////////////////////////////////////


UCQueue & UCQueue::operator=(const UCQueue &Q)

{

if ( this == &Q )  return ( *this );

assign(Q);

return ( *this );

}


////////////////////////////////////////////////////////////////////////


void UCQueue::assign(const UCQueue &Q)

{

clear();

if ( Q.NElements == 0 )  return;

memcpy(data, Q.data, ucqueue_size);

NElements = Q.NElements;

return;

}


////////////////////////////////////////////////////////////////////////


void UCQueue::clear()

{

NElements = 0;

return;

}


////////////////////////////////////////////////////////////////////////


int UCQueue::is_empty() const

{

return ( (NElements == 0) ? 1 : 0 );

}


////////////////////////////////////////////////////////////////////////


int UCQueue::n_elements() const

{

return ( NElements );

}


////////////////////////////////////////////////////////////////////////


int UCQueue::last_char() const

{

if ( NElements == 0 )  return ( -1 );

return ( (int) (data[0]) );

}


////////////////////////////////////////////////////////////////////////


int UCQueue::run_count() const

{

return ( calc_run_count() );

}


////////////////////////////////////////////////////////////////////////


void UCQueue::enqueue(unsigned char u)

{

if ( NElements >= ucqueue_size )  {

   mlog << Error << "\nUCQueue::enqueue() -> queue full!\n\n";

   exit ( 1 );

}

int j;

for (j=NElements; j>=1; --j)  {

   data[j] = data[j - 1];

}

data[0] = u;

++NElements;

return;

}


////////////////////////////////////////////////////////////////////////


unsigned char UCQueue::dequeue()

{

if ( NElements == 0 )  {

   mlog << Error << "\nUCQueue::dequeue() -> queue empty!\n\n";

   exit ( 1 );

}

unsigned char u;

u = data[NElements - 1];

--NElements;

return ( u );

}


////////////////////////////////////////////////////////////////////////


int UCQueue::calc_run_count() const

{

int j, count;

if ( NElements <= 1 )  return ( NElements );

count = 1;

for (j=1; j<NElements; ++j)  {

   if ( data[j] != data[0] )  break;

   ++count;

}


return ( count );

}


////////////////////////////////////////////////////////////////////////







