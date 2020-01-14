/////////////////////////////////
//
// Library   : Analyzer/Scalar
// Unit      : scalar elements common to compilation constants and symbolic execution
// File      : FloatElement.h
// Author    : Franck Vedrine
// Copyright : CEA LIST - 2019-2020 - all rights reserved
//
// Description :
//   Definition of floating-point classes that are very helpful to define
//     the compilation constants and the scalar elements used in
//     simulation/symbolic execution.
//   All this stuff is rather defined for source code analysis.
//

#ifndef Analyzer_Scalar_VirtualExact_FloatElementH
#define Analyzer_Scalar_VirtualExact_FloatElementH

#include "Analyzer/Scalar/VirtualExact/Native/IntegerElement.h"
#include "Analyzer/Scalar/Implementation/Native/FloatElement.h"

namespace Analyzer { namespace Scalar { namespace Floating { namespace VirtualExact {

#define DefineDeclareUnaryMethod(TypeOperation)                                                   \
bool apply##TypeOperation(const Operation& operation, EvaluationEnvironment& env);

#define DefineDeclareBinaryMethod(TypeOperation)                                                  \
bool apply##TypeOperation(const Operation& operation, EvaluationEnvironment& env);

#define DefineDeclareOBinaryMethod(TypeOperation, TypeArg)                                        \
bool apply##TypeOperation##TypeArg(const Operation& operation, EvaluationEnvironment& env);

/****************************************/
/* Definition of the class FloatElement */
/****************************************/

#define DefineTypeObject TFloatElement
#define DefineTypeInheritedImplementation Implementation::FloatElement
#include "Analyzer/Scalar/VirtualExact/Native/FloatElement.inch"
#undef DefineTypeObject
#undef DefineTypeInheritedImplementation

/*****************************************/
/* Definition of the class DoubleElement */
/*****************************************/

#define DefineTypeObject TDoubleElement
#define DefineTypeInheritedImplementation Implementation::DoubleElement
#define DefineIncludeFloatOperation
#include "Analyzer/Scalar/VirtualExact/Native/FloatElement.inch"
#undef DefineIncludeFloatOperation
#undef DefineTypeObject
#undef DefineTypeInheritedImplementation

/*********************************************/
/* Definition of the class LongDoubleElement */
/*********************************************/

#define DefineTypeObject TLongDoubleElement
#define DefineTypeInheritedImplementation Implementation::LongDoubleElement
#include "Analyzer/Scalar/VirtualExact/Native/FloatElement.inch"
#undef DefineTypeObject
#undef DefineTypeInheritedImplementation

#undef DefineDeclareUnaryMethod
#undef DefineDeclareBinaryMethod
#undef DefineDeclareOBinaryMethod

}}}} // end of namespace Analyzer::Scalar::Floating::VirtualExact

#endif // Analyzer_Scalar_VirtualExact_FloatElementH

