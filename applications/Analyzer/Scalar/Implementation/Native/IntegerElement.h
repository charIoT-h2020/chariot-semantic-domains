/////////////////////////////////
//
// Library   : Analyzer/Scalar
// Unit      : implementation of scalar elements specific to source code analysis
// File      : IntegerElement.h
// Author    : Franck Vedrine
// Copyright : CEA LIST - 2019-2020 - all rights reserved
//
// Description :
//   Definition of the integer classes
//

#ifndef Analyzer_Scalar_Implementation_IntegerElementH
#define Analyzer_Scalar_Implementation_IntegerElementH

#include "Analyzer/Scalar/Implementation/ImplementationElement.h"

namespace Analyzer { namespace Scalar {
namespace Integer { namespace Implementation {

class CharElement;
class SignedCharElement;
class UnsignedCharElement;
class ShortElement;
class UnsignedShortElement;
class IntElement;
class UnsignedIntElement;
class LongElement;
class UnsignedLongElement;

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

#define DefineTypeIntegerElement    CharElement
#define DefineTypeImplementation      int
#define DefineTypeRealImplementation  char
#define DefineTypeInherited         Scalar::Implementation::ImplementationElement
#define DefineConstructor           inherited::setConstantSigned();
#include "Analyzer/Scalar/Implementation/Native/IntegerElement.inch"
#undef DefineConstructor
#undef DefineTypeInherited
#undef DefineTypeRealImplementation
#undef DefineTypeImplementation
#undef DefineTypeIntegerElement

#define DefineTypeIntegerElement    SignedCharElement
#define DefineTypeImplementation      int
#define DefineTypeRealImplementation  signed char
#define DefineTypeInherited         Scalar::Implementation::ImplementationElement
#define DefineConstructor           inherited::setConstantSigned();
#include "Analyzer/Scalar/Implementation/Native/IntegerElement.inch"
#undef DefineConstructor
#undef DefineTypeInherited
#undef DefineTypeRealImplementation
#undef DefineTypeImplementation
#undef DefineTypeIntegerElement

#define DefineTypeIntegerElement    UnsignedCharElement
#define DefineTypeImplementation      unsigned int
#define DefineTypeRealImplementation  unsigned char
#define DefineTypeInherited         Scalar::Implementation::ImplementationElement
#define DefineConstructor           inherited::setConstantUnsigned();
#include "Analyzer/Scalar/Implementation/Native/IntegerElement.inch"
#undef DefineConstructor
#undef DefineTypeInherited
#undef DefineTypeRealImplementation
#undef DefineTypeImplementation
#undef DefineTypeIntegerElement

#define DefineTypeIntegerElement    ShortElement
#define DefineTypeImplementation      int
#define DefineTypeRealImplementation  short int
#define DefineTypeInherited         Scalar::Implementation::ImplementationElement
#define DefineConstructor           inherited::setConstantSigned();
#include "Analyzer/Scalar/Implementation/Native/IntegerElement.inch"
#undef DefineConstructor
#undef DefineTypeInherited
#undef DefineTypeRealImplementation
#undef DefineTypeImplementation
#undef DefineTypeIntegerElement

#define DefineTypeIntegerElement    UnsignedShortElement
#define DefineTypeImplementation      unsigned int
#define DefineTypeRealImplementation  unsigned short int
#define DefineTypeInherited         Scalar::Implementation::ImplementationElement
#define DefineConstructor           inherited::setConstantUnsigned();
#include "Analyzer/Scalar/Implementation/Native/IntegerElement.inch"
#undef DefineConstructor
#undef DefineTypeInherited
#undef DefineTypeRealImplementation
#undef DefineTypeImplementation
#undef DefineTypeIntegerElement

#define DefineTypeIntegerElement    IntElement
#define DefineTypeImplementation      int
#define DefineTypeRealImplementation  int
#define DefineTypeInherited         Scalar::Implementation::ImplementationElement
#define DefineConstructor           inherited::setConstantSigned();
#include "Analyzer/Scalar/Implementation/Native/IntegerElement.inch"
#undef DefineConstructor
#undef DefineTypeInherited
#undef DefineTypeRealImplementation
#undef DefineTypeImplementation
#undef DefineTypeIntegerElement

#define DefineTypeIntegerElement    UnsignedIntElement
#define DefineTypeImplementation      unsigned int
#define DefineTypeRealImplementation  unsigned int
#define DefineTypeInherited         Scalar::Implementation::ImplementationElement
#define DefineConstructor           inherited::setConstantUnsigned();
#include "Analyzer/Scalar/Implementation/Native/IntegerElement.inch"
#undef DefineConstructor
#undef DefineTypeInherited
#undef DefineTypeRealImplementation
#undef DefineTypeImplementation
#undef DefineTypeIntegerElement

#define DefineTypeIntegerElement    LongElement
#define DefineTypeImplementation      long int
#define DefineTypeRealImplementation  long int
#define DefineTypeInherited         Scalar::Implementation::ImplementationElement
#define DefineConstructor           inherited::setConstantSigned();
#include "Analyzer/Scalar/Implementation/Native/IntegerElement.inch"
#undef DefineConstructor
#undef DefineTypeInherited
#undef DefineTypeRealImplementation
#undef DefineTypeImplementation
#undef DefineTypeIntegerElement

#define DefineTypeIntegerElement    UnsignedLongElement
#define DefineTypeImplementation      unsigned long int
#define DefineTypeRealImplementation  unsigned long int
#define DefineTypeInherited         Scalar::Implementation::ImplementationElement
#define DefineConstructor           inherited::setConstantUnsigned();
#include "Analyzer/Scalar/Implementation/Native/IntegerElement.inch"
#undef DefineConstructor
#undef DefineTypeInherited
#undef DefineTypeRealImplementation
#undef DefineTypeImplementation
#undef DefineTypeIntegerElement

#undef DefineTypeCastAssign

}}}} // end of namespace Analyzer::Scalar::Integer::Implementation

#endif // Analyzer_Interpretation_Scalar_Implementation_IntegerElementH
