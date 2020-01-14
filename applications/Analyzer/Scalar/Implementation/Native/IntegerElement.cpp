/////////////////////////////////
//
// Library   : Analyzer/Scalar
// Unit      : implementation of scalar elements specific to source code analysis
// File      : IntegerElement.cpp
// Author    : Franck Vedrine
// Copyright : CEA LIST - 2019-2020 - all rights reserved
//
// Description :
//   Implementation of the integer classes
//

#include "Analyzer/Scalar/Implementation/Native/IntegerElement.h"
// #include "Analyzer/Scalar/Scalar.hpp"

namespace Analyzer {}

#include <limits.h>

namespace Analyzer { namespace Scalar {
namespace Integer { namespace Implementation {

#define DefineTypeCastAssign                                                             \
   DefineIntegerAssign(CharElement)                                                      \
   DefineIntegerAssign(SignedCharElement)                                                \
   DefineIntegerAssign(UnsignedCharElement)                                              \
   DefineIntegerAssign(ShortElement)                                                     \
   DefineIntegerAssign(UnsignedShortElement)                                             \
   DefineIntegerAssign(IntElement)                                                       \
   DefineIntegerAssign(UnsignedIntElement)                                               \
   DefineIntegerAssign(LongElement)                                                      \
   DefineIntegerAssign(UnsignedLongElement)

#define DefineTypeIntegerElement       CharElement
#define DefineTypeImplementation       int
#define DefineTypeRealImplementation   char
#define DefineTypeMin                  CHAR_MIN
#define DefineTypeMax                  CHAR_MAX
#include "Analyzer/Scalar/Implementation/Native/IntegerElement.incc"
#undef DefineTypeMax
#undef DefineTypeMin
#undef DefineTypeInherited
#undef DefineTypeRealImplementation
#undef DefineTypeImplementation
#undef DefineTypeIntegerElement

#define DefineTypeIntegerElement       SignedCharElement
#define DefineTypeImplementation       int
#define DefineTypeRealImplementation   signed char
#define DefineTypeMin                  SCHAR_MIN
#define DefineTypeMax                  SCHAR_MAX
#include "Analyzer/Scalar/Implementation/Native/IntegerElement.incc"
#undef DefineTypeMax
#undef DefineTypeMin
#undef DefineTypeInherited
#undef DefineTypeRealImplementation
#undef DefineTypeImplementation
#undef DefineTypeIntegerElement

#define DefineTypeIntegerElement       UnsignedCharElement
#define DefineTypeImplementation       unsigned int
#define DefineTypeRealImplementation   unsigned char
#define DefineTypeUnsigned
#define DefineTypeMin                  0
#define DefineTypeMax                  UCHAR_MAX
#include "Analyzer/Scalar/Implementation/Native/IntegerElement.incc"
#undef DefineTypeMax
#undef DefineTypeMin
#undef DefineTypeUnsigned
#undef DefineTypeInherited
#undef DefineTypeRealImplementation
#undef DefineTypeImplementation
#undef DefineTypeIntegerElement

#define DefineTypeIntegerElement       ShortElement
#define DefineTypeImplementation       int
#define DefineTypeRealImplementation   short int
#define DefineTypeMin                  SHRT_MIN
#define DefineTypeMax                  SHRT_MAX
#include "Analyzer/Scalar/Implementation/Native/IntegerElement.incc"
#undef DefineTypeMax
#undef DefineTypeMin
#undef DefineTypeInherited
#undef DefineTypeRealImplementation
#undef DefineTypeImplementation
#undef DefineTypeIntegerElement

#define DefineTypeIntegerElement       UnsignedShortElement
#define DefineTypeImplementation       unsigned int
#define DefineTypeRealImplementation   unsigned short int
#define DefineTypeUnsigned
#define DefineTypeMin                  0
#define DefineTypeMax                  USHRT_MAX
#include "Analyzer/Scalar/Implementation/Native/IntegerElement.incc"
#undef DefineTypeMax
#undef DefineTypeMin
#undef DefineTypeUnsigned
#undef DefineTypeInherited
#undef DefineTypeRealImplementation
#undef DefineTypeImplementation
#undef DefineTypeIntegerElement

#define DefineTypeIntegerElement       IntElement
#define DefineTypeImplementation       int
#define DefineTypeRealImplementation   int
#define DefineTypeMin                  INT_MIN
#define DefineTypeMax                  INT_MAX
#define DefineTypeNeedBytes
#include "Analyzer/Scalar/Implementation/Native/IntegerElement.incc"
#undef DefineTypeNeedBytes
#undef DefineTypeMax
#undef DefineTypeMin
#undef DefineTypeInherited
#undef DefineTypeRealImplementation
#undef DefineTypeImplementation
#undef DefineTypeIntegerElement

#define DefineTypeIntegerElement       UnsignedIntElement
#define DefineTypeImplementation       unsigned int
#define DefineTypeRealImplementation   unsigned int
#define DefineTypeUnsigned
#define DefineTypeMin                  0
#define DefineTypeMax                  UINT_MAX
#define DefineTypeNeedBytes
#include "Analyzer/Scalar/Implementation/Native/IntegerElement.incc"
#undef DefineTypeNeedBytes
#undef DefineTypeMax
#undef DefineTypeMin
#undef DefineTypeUnsigned
#undef DefineTypeInherited
#undef DefineTypeRealImplementation
#undef DefineTypeImplementation
#undef DefineTypeIntegerElement

#define DefineTypeIntegerElement       LongElement
#define DefineTypeImplementation       long int
#define DefineTypeRealImplementation   long int
#define DefineTypeMin                  LONG_MIN
#define DefineTypeMax                  LONG_MAX
#define DefineTypeNeedBytes
#include "Analyzer/Scalar/Implementation/Native/IntegerElement.incc"
#undef DefineTypeNeedBytes
#undef DefineTypeMax
#undef DefineTypeMin
#undef DefineTypeInherited
#undef DefineTypeRealImplementation
#undef DefineTypeImplementation
#undef DefineTypeIntegerElement

#define DefineTypeIntegerElement       UnsignedLongElement
#define DefineTypeImplementation       unsigned long int
#define DefineTypeRealImplementation   unsigned long int
#define DefineTypeUnsigned
#define DefineTypeMin                  0
#define DefineTypeMax                  ULONG_MAX
#define DefineTypeNeedBytes
#include "Analyzer/Scalar/Implementation/Native/IntegerElement.incc"
#undef DefineTypeNeedBytes
#undef DefineTypeMax
#undef DefineTypeMin
#undef DefineTypeUnsigned
#undef DefineTypeInherited
#undef DefineTypeRealImplementation
#undef DefineTypeImplementation
#undef DefineTypeIntegerElement

#undef DefineTypeCastConstruction

}}}} // end of namespace Analyzer::Scalar::Integer::Implementation


