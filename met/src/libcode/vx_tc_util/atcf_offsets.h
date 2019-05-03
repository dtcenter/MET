// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2016
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*

////////////////////////////////////////////////////////////////////////

#ifndef  __ATCF_OFFSETS_H__
#define  __ATCF_OFFSETS_H__

////////////////////////////////////////////////////////////////////////

//
// ATCF Column Offsets
//   http://www.nrlmry.navy.mil/atcf_web/docs/database/new/abrdeck.html
//

static const int BasinOffset           = 0;
static const int CycloneNumberOffset   = 1;
static const int WarningTimeOffset     = 2;
static const int TechniqueNumberOffset = 3;
static const int TechniqueOffset       = 4;

static const int ForecastPeriodOffset  = 5;
static const int LatTenthsOffset       = 6;
static const int LonTenthsOffset       = 7;
static const int VMaxOffset            = 8;
static const int MSLPOffset            = 9;

static const int LevelOffset           = 10;
static const int WindIntensityOffset   = 11;
static const int QuadrantOffset        = 12;
static const int Radius1Offset         = 13;
static const int Radius2Offset         = 14;

static const int Radius3Offset         = 15;
static const int Radius4Offset         = 16;
static const int IsobarPressureOffset  = 17;
static const int IsobarRadiusOffset    = 18;
static const int MaxWindRadiusOffset   = 19;

static const int GustsOffset           = 20;
static const int EyeDiameterOffset     = 21;
static const int SubRegionOffset       = 22;
static const int MaxSeasOffset         = 23;
static const int InitialsOffset        = 24;

static const int StormDirectionOffset  = 25;
static const int StormSpeedOffset      = 26;
static const int StormNameOffset       = 27;
static const int DepthOffset           = 28;
static const int WaveHeightOffset      = 29;

static const int SeasCodeOffset        = 30;
static const int SeasRadius1Offset     = 31;
static const int SeasRadius2Offset     = 32;
static const int SeasRadius3Offset     = 33;
static const int SeasRadius4Offset     = 34;

static const int MinATCFElements       = 8;

////////////////////////////////////////////////////////////////////////

#endif   /*  __ATCF_OFFSETS_H__  */

////////////////////////////////////////////////////////////////////////
