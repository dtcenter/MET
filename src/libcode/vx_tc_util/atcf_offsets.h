// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2023
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
//

static const int BasinOffset           = 0;
static const int CycloneNumberOffset   = 1;
static const int WarningTimeOffset     = 2;
static const int TechniqueNumberOffset = 3;
static const int TechniqueOffset       = 4;

static const int ForecastPeriodOffset  = 5;
static const int LatTenthsOffset       = 6;
static const int LonTenthsOffset       = 7;

//
// Offsets specific to the ADECK and BDECK track lines
//   http://www.nrlmry.navy.mil/atcf_web/docs/database/new/abrdeck.html
// Offsets for columns common to ATCFLineType_Track and ATCFLineType_GenTrack
//

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

//
// Offsets for columns specific to the ATCFLineType_Track
//

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

static const int UserDefinedOffset     = 35;
static const int WarmCoreOffset        = 39;

static const char ThermoParams_Str[]   = "THERMO PARARMS";

//
// Offsets for columns specific to the ATCFLineType_GenTrack
// Reference: https://dtcenter.org/HurrWRF/users/docs/users_guide/HWRF-UG-2018.pdf
//

static const int GenStormIdOffset        = 2;

static const int GenParameterBOffset     = 21;
static const int GenThermWindLowerOffset = 22;
static const int GenThermWindUpperOffset = 23;
static const int GenWarmCoreOffset       = 24;
static const int GenStormDirectionOffset = 25;

static const int GenStormSpeedOffset     = 26;
static const int GenMean850VortOffset    = 27;
static const int GenMax850VortOffset     = 28;
static const int GenMean700VortOffset    = 29;
static const int GenMax700VortOffset     = 30;

//
// Offsets specific to the EDECK probability lines
//   http://www.nrlmry.navy.mil/atcf_web/docs/database/new/edeck.txt
//

static const int ProbOffset            = 8;  // probability of event (0-100)
static const int ProbItemOffset        = 9;  // RI: intensity change, GN: time window

//
// Offsets specific to ATCF EDECK RI line type
//

static const int ProbRIRWValueOffset     = 10; // final intensity
static const int ProbRIRWInitialsOffset  = 11; // forecaster initials
static const int ProbRIRWBegOffset       = 12; // RIRW start time
static const int ProbRIRWEndOffset       = 13; // RIRW stop time

//
// Offsets specific to ATCF EDECK GN line type
//

static const int ProbGenInitialsOffset   = 10; // forecaster initials
static const int ProbGenOrDisOffset      = 11; // genesis or dissipation:
                                               //   invest, genFcst, genesis,
                                               //   disFcst, dissipate
static const int ProbGenTimeOffset       = 12; // forecast genesis time

//
// Minimum number of required elements
//

static const int MinATCFTrackElements    = 8;
static const int MinATCFGenTrackElements = 9;
static const int MinATCFProbRIRWElements = 14;
static const int MinATCFProbGNElements   = 13;

////////////////////////////////////////////////////////////////////////

#endif   /*  __ATCF_OFFSETS_H__  */

////////////////////////////////////////////////////////////////////////
