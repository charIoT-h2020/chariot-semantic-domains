/////////////////////////////////
//
// Library   : Analyzer/Scalar
// Unit      : implementation of scalar elements specific to source code analysis
// File      : FloatElement.cpp
// Author    : Franck Vedrine
// Copyright : CEA LIST - 2019-2020 - all rights reserved
//
// Description :
//   Implementation of the floating point classes
//

#include "Analyzer/Scalar/Implementation/Native/FloatElement.h"
// #include "Analyzer/Scalar/Scalar.hpp"

namespace Analyzer {} // for precompiled headers

#include <limits.h>
#include <float.h>
#include <math.h>

namespace Analyzer { namespace Scalar {
namespace Floating { namespace Implementation {

#define DefineTypeCastAssign                                                             \
   DefineFloatAssign(FloatElement)                                                       \
   DefineFloatAssign(DoubleElement)                                                      \
   DefineFloatAssign(LongDoubleElement)
 
#define DefineTypeFloatElement FloatElement
#define DefineTypeImplementation float
#define DefineTypeMax FLT_MAX
#define DefineTypeMin FLT_MIN
#define DefineTypeEpsilon FLT_EPSILON
#define DefineTypeMantDig FLT_MANT_DIG
#define DefineTypeMaxExp FLT_MAX_EXP
#include "Analyzer/Scalar/Implementation/Native/FloatElement.incc"
#undef DefineTypeFloatElement
#undef DefineTypeImplementation
#undef DefineTypeMax
#undef DefineTypeMin
#undef DefineTypeEpsilon
#undef DefineTypeMantDig
#undef DefineTypeMaxExp

#define DefineTypeFloatElement DoubleElement
#define DefineTypeImplementation double
#define DefineTypeMax DBL_MAX
#define DefineTypeMin DBL_MIN
#define DefineTypeEpsilon DBL_EPSILON
#define DefineTypeMantDig DBL_MANT_DIG
#define DefineTypeMaxExp DBL_MAX_EXP
#define DefineTypeLibSupported
#include "Analyzer/Scalar/Implementation/Native/FloatElement.incc"
#undef DefineTypeLibSupported
#undef DefineTypeFloatElement
#undef DefineTypeImplementation
#undef DefineTypeMax
#undef DefineTypeMin
#undef DefineTypeEpsilon
#undef DefineTypeMantDig
#undef DefineTypeMaxExp

#define DefineTypeFloatElement LongDoubleElement
#define DefineTypeImplementation long double
#define DefineTypeMax LDBL_MAX
#define DefineTypeMin LDBL_MIN
#define DefineTypeEpsilon LDBL_EPSILON
#define DefineTypeMantDig LDBL_MANT_DIG
#define DefineTypeMaxExp LDBL_MAX_EXP
#define DefineNeedLongExponent
#include "Analyzer/Scalar/Implementation/Native/FloatElement.incc"
#undef DefineNeedLongExponent
#undef DefineTypeFloatElement
#undef DefineTypeImplementation
#undef DefineTypeMax
#undef DefineTypeMin
#undef DefineTypeEpsilon
#undef DefineTypeMantDig
#undef DefineTypeMaxExp

#undef DefineTypeCastAssign

}}}} // end of namespace Analyzer::Scalar::Floating::Implementation

