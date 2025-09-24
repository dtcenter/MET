// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2025
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*

///////////////////////////////////////////////////////////////////////////////

#ifndef  __FIELD_TYPE_H__
#define  __FIELD_TYPE_H__

///////////////////////////////////////////////////////////////////////////////

//
// Enumeration for field type configuration parameters
//

enum class FieldType {
   None, // Default
   Fcst, // Apply to forecast field
   Obs,  // Apply to observation field
   Both  // Apply to both forecast and observation field
};

///////////////////////////////////////////////////////////////////////////////

#endif   //  __FIELD_TYPE_H__

///////////////////////////////////////////////////////////////////////////////
