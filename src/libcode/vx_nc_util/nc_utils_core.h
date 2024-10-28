// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2024
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*

////////////////////////////////////////////////////////////////////////

#ifndef  __NC_UTILS_CORE_H__
#define  __NC_UTILS_CORE_H__

////////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////////

#ifndef ncbyte
typedef signed char ncbyte; /* from ncvalues.h */
#endif   /*  ncbyte  */

////////////////////////////////////////////////////////////////////////

#define IS_VALID_NC(ncObj)          (!ncObj.isNull())
#define IS_VALID_NC_P(ncObjPtr)     ((ncObjPtr != nullptr && !ncObjPtr->isNull()))

#define IS_INVALID_NC(ncObj)        ncObj.isNull()
#define IS_INVALID_NC_P(ncObjPtr)   (ncObjPtr == nullptr || ncObjPtr->isNull())

#define GET_NC_NAME(ncObj)          ncObj.getName()
#define GET_NC_NAME_P(ncObjPtr)     ncObjPtr->getName()

#define GET_NC_SIZE(ncObj)          ncObj.getSize()
#define GET_NC_SIZE_P(ncObjPtr)     ncObjPtr->getSize()

#define GET_SAFE_NC_NAME(ncObj)         (ncObj.isNull() ? C_unknown_str : ncObj.getName())
#define GET_SAFE_NC_NAME_P(ncObjPtr)    (IS_INVALID_NC_P(ncObjPtr) ? C_unknown_str : ncObjPtr->getName())

#define GET_NC_TYPE_ID(ncObj)           ncObj.getType().getId()
#define GET_NC_TYPE_ID_P(ncObjPtr)      ncObjPtr->getType().getId()
#define GET_NC_TYPE_NAME(ncObj)         ncObj.getType().getName()
#define GET_NC_TYPE_NAME_P(ncObjPtr)    ncObjPtr->getType().getName()

#define GET_NC_DIM_COUNT(ncObj)         ncObj.getDimCount()
#define GET_NC_DIM_COUNT_P(ncObjPtr)    ncObjPtr->getDimCount()

#define GET_NC_VAR_COUNT(ncObj)         ncObj.getVarCount()
#define GET_NC_VAR_COUNT_P(ncObjPtr)    ncObjPtr->getVarCount()

#define GET_NC_VARS(ncObj)              ncObj.getVars()
#define GET_NC_VARS_P(ncObjPtr)         ncObjPtr->getVars()

////////////////////////////////////////////////////////////////////////

static const std::string C_unknown_str = std::string("unknown");

static const std::string fill_value_att_name    = "_FillValue";
static const std::string missing_value_att_name = "missing_value";

////////////////////////////////////////////////////////////////////////


extern int get_data_size(netCDF::NcVar *);
extern int get_dim_count(const netCDF::NcVar *);
extern int get_dim_size(const netCDF::NcDim *);
extern int get_dim_size(const netCDF::NcVar *, const int dim_offset);

extern netCDF::NcVarAtt   *get_nc_att(const netCDF::NcVar  *, const ConcatString &, bool exit_on_error = false);
extern netCDF::NcGroupAtt *get_nc_att(const netCDF::NcFile *, const ConcatString &, bool exit_on_error = false);

extern netCDF::NcDim  get_nc_dim(const netCDF::NcFile *, const std::string &dim_name);
extern netCDF::NcDim  get_nc_dim(const netCDF::NcVar *, const std::string &dim_name);
extern netCDF::NcDim  get_nc_dim(const netCDF::NcVar *, const int dim_offset);

////////////////////////////////////////////////////////////////////////

#endif   /*  __NC_UTILS_CORE_H__  */

////////////////////////////////////////////////////////////////////////
