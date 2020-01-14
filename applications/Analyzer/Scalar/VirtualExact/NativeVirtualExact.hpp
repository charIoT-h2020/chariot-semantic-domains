/////////////////////////////////
//
// Library   : Analyzer/Scalar
// Unit      : scalar elements common to compilation constants and symbolic execution
// File      : NativeVirtualExact.hpp
// Author    : Franck Vedrine
// Copyright : CEA LIST - 2019-2020 - all rights reserved
//
// Description :
//   Definition of a an intermediate hierarchy of classes
//     that are very helpful to define the compilation constants
//     and the scalar elements used in simulation/symbolic execution.
//   All this stuff is rather defined for source code analysis.
//

#ifndef Analyzer_Scalar_VirtualExact_NativeVirtualExactH
#define Analyzer_Scalar_VirtualExact_NativeVirtualExactH

namespace Analyzer { namespace Scalar {

namespace Integer { namespace VirtualExact {

template <class TypeBase> class TCharElement;
template <class TypeBase> class TSignedCharElement;
template <class TypeBase> class TUnsignedCharElement;
template <class TypeBase> class TShortElement;
template <class TypeBase> class TUnsignedShortElement;
template <class TypeBase> class TIntElement;
template <class TypeBase> class TUnsignedIntElement;
template <class TypeBase> class TLongElement;
template <class TypeBase> class TUnsignedLongElement;

}} // end of namespace Integer::VirtualExact

namespace Floating { namespace VirtualExact {
   
template <class TypeBase> class TFloatElement;
template <class TypeBase> class TDoubleElement;
template <class TypeBase> class TLongDoubleElement;

}} // end of namespace Floating::VirtualExact

namespace VirtualExact {

template <class TypeBase> class TUndefElement;

}}} // end of namespace Analyzer::Scalar::VirtualExact

#include "Analyzer/Scalar/VirtualExact/Native/IntegerElement.h"
#include "Analyzer/Scalar/VirtualExact/Native/FloatElement.h"
#include "Analyzer/Scalar/VirtualExact/TopElement.h"

#endif // Analyzer_Scalar_VirtualExact_NativeVirtualExactH

