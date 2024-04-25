// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2024
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*


//////////////////////////////////////////////////////////////////


#ifndef __ENUM_CLASS_AS_INT_HPP__
#define __ENUM_CLASS_AS_INT_HPP__


//////////////////////////////////////////////////////////////////

template <typename Enumeration>
auto enum_class_as_int(Enumeration const value)
    -> typename std::underlying_type<Enumeration>::type
{
    return static_cast<typename std::underlying_type<Enumeration>::type>(value);
}

//////////////////////////////////////////////////////////////////


#endif  //  __ENUM_CLASS_AS_INT_HPP__


//////////////////////////////////////////////////////////////////


