// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2019
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*

////////////////////////////////////////////////////////////////////////

using namespace std;

#include <cstdio>
#include <string.h>

#include "tc_hdr_columns.h"

////////////////////////////////////////////////////////////////////////
//
//  Code for class TcHdrColumns
//
////////////////////////////////////////////////////////////////////////

TcHdrColumns::TcHdrColumns() {

   init_from_scratch();
}

////////////////////////////////////////////////////////////////////////

TcHdrColumns::~TcHdrColumns() {

   clear();
}

////////////////////////////////////////////////////////////////////////

void TcHdrColumns::init_from_scratch() {

   clear();

   return;
}

////////////////////////////////////////////////////////////////////////

void TcHdrColumns::clear() {

   ADeckModel.clear();
   BDeckModel.clear();
   Desc.clear();
   StormId.clear();
   Basin.clear();
   Cyclone.clear();
   StormName.clear();
   InitTime = (unixtime) 0;
   LeadTime = 0;
   ValidTime = (unixtime) 0;
   InitMask.clear();
   ValidMask.clear();
   LineType.clear();

   return;
}

////////////////////////////////////////////////////////////////////////
