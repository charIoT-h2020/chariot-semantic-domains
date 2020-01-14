/////////////////////////////////
//
// Library   : Analyzer/Scalar
// Unit      : implementation of scalar elements specific to source code analysis
// File      : FloatElement.h
// Author    : Franck Vedrine
// Copyright : CEA LIST - 2019-2020 - all rights reserved
//
// Description :
//   Definition of the floating point classes
//

#ifndef Analyzer_Scalar_Implementation_FloatElementH
#define Analyzer_Scalar_Implementation_FloatElementH

#include "Analyzer/Scalar/Implementation/ImplementationElement.h"

namespace Analyzer { namespace Scalar {
namespace Floating { namespace Implementation {

class FloatElement;
class DoubleElement;
class LongDoubleElement;

#define DefineTypeCastAssign                                                                 \
   DefineFloatAssign(FloatElement)                                                           \
   DefineFloatAssign(DoubleElement)                                                          \
   DefineFloatAssign(LongDoubleElement)

#define DefineTypeFloatElement FloatElement
#define DefineTypeImplementation float
#define DefineTypeInherited Scalar::Implementation::ImplementationElement
#include "Analyzer/Scalar/Implementation/Native/FloatElement.inch"
#undef DefineTypeFloatElement
#undef DefineTypeImplementation
#undef DefineTypeInherited

#define DefineTypeFloatElement DoubleElement
#define DefineTypeImplementation double
#define DefineTypeInherited Scalar::Implementation::ImplementationElement
#define DefineTypeLibSupported
#include "Analyzer/Scalar/Implementation/Native/FloatElement.inch"
#undef DefineTypeLibSupported
#undef DefineTypeFloatElement
#undef DefineTypeImplementation
#undef DefineTypeInherited

#define DefineTypeFloatElement LongDoubleElement
#define DefineTypeImplementation long double
#define DefineTypeInherited Scalar::Implementation::ImplementationElement
#include "Analyzer/Scalar/Implementation/Native/FloatElement.inch"
#undef DefineTypeFloatElement
#undef DefineTypeImplementation
#undef DefineTypeInherited

#undef DefineTypeCastAssign

}}}} // end of namespace Analyzer::Scalar::Floating::Implementation

#endif // Analyzer_Scalar_Implementation_FloatElementH

