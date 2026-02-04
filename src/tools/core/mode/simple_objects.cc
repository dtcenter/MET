// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2026
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*


////////////////////////////////////////////////////////////////////////

#include "simple_objects.hh"

using namespace std;

////////////////////////////////////////////////////////////////////////

SimpleObjects::SimpleObjects() :
   _dataType(ModeDataType::MvMode_Both),
   _rIndex(-1),
   _tIndex(-1)
{
}

////////////////////////////////////////////////////////////////////////

SimpleObjects::~SimpleObjects()
{
}

////////////////////////////////////////////////////////////////////////

void SimpleObjects::init(ModeDataType dataType, int rIndex, int tIndex)
{
   _dataType = dataType;
   _rIndex = rIndex;
   _tIndex = tIndex;
}             

////////////////////////////////////////////////////////////////////////

void SimpleObjects::setSuper(bool isFcst, int n_fcst_files, bool do_clusters,
                             BoolCalc &f_calc)
{
   _super = ModeSuperObject(isFcst, n_fcst_files, do_clusters,
                            _rIndex, _tIndex, _mvd, f_calc);
}

////////////////////////////////////////////////////////////////////////

void SimpleObjects::clear(void)
{
   for (auto &x : _mvd) {
      delete x;
      x = nullptr;
   }
   _mvd.clear();
}
