/////////////////////////////////
//
// Library   : Static Analysis
// Unit      : AbstractDomain
// File      : AbstractDomain.h
// Author    : Franck Vedrine
// Copyright : CEA LIST - 2019-2020 - all rights reserved
//
// Description :
//   Definition of a generic C Interface to manipulate abstract domains
//   = interval, disjunctions, ...
//

#ifndef Analyzer_Abstract_Domain_H
#define Analyzer_Abstract_Domain_H

#if defined(_WIN32)
#define DLL_CDECL __cdecl
#else
#define DLL_CDECL 
#endif

#if defined(_WIN32) && defined(_USRDLL)
#ifdef DLL_EXPORTS
#define DLL_API __declspec(dllexport)
#else
#define DLL_API __declspec(dllimport)
#endif // APE_EXPORTS
#else
#define DLL_API
#endif // _USRDLL

#ifdef _MSC_VER
#pragma pack(push,4)
#endif

#if defined(_MSC_VER) && !defined(__cplusplus)
#define inline __inline
#endif

#ifdef __cplusplus
extern "C" {
#endif

#define DefineStartGenericFunctions 
#define DefineEndGenericFunctions 
#define DefineDomainPrefix(x) domain_##x
#include "Analyzer/Scalar/AbstractDomain.inch"
#undef DefineDomainPrefix
#undef DefineStartGenericFunctions
#undef DefineEndGenericFunctions

#ifdef __cplusplus
}
#endif

#ifdef _MSC_VER
#pragma pack(pop)
#endif

#endif // Analyzer_Abstract_Domain_H

