// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2019
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*


////////////////////////////////////////////////////////////////////////


#ifndef  __CONFIG_OBJECT_TYPES_H__
#define  __CONFIG_OBJECT_TYPES_H__


////////////////////////////////////////////////////////////////////////


enum ConfigObjectType {

   IntegerType, 
   FloatType, 
   BooleanType, 
   StringType, 
   DictionaryType, 
   ArrayType, 
   PwlFunctionType, 
   ThresholdType, 

   // VariableType, //  poor name?

   UserFunctionType, 




   no_config_object_type

};


////////////////////////////////////////////////////////////////////////


#endif   /*  __CONFIG_OBJECT_TYPES_H__  */


////////////////////////////////////////////////////////////////////////


