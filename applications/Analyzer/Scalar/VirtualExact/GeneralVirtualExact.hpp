/////////////////////////////////
//
// Library   : Analyzer/Scalar
// Unit      : scalar elements common to compilation constants and symbolic execution
// File      : GeneralVirtualExact.hpp
// Author    : Franck Vedrine
// Copyright : CEA LIST - 2019-2020 - all rights reserved
//
// Description :
//   Definition of a an intermediate hierarchy of classes
//     that are very helpful to define the compilation constants
//     and the scalar elements used in simulation/symbolic execution.
//   All this stuff is rather defined for host independent code analysis.
//

#ifndef Analyzer_Scalar_VirtualExact_GeneralVirtualExactH
#define Analyzer_Scalar_VirtualExact_GeneralVirtualExactH

namespace Analyzer { namespace Scalar {

namespace Bit { namespace VirtualExact {
   
template <class TypeBase> class TBitElement;

}} // end of namespace Bit::VirtualExact

namespace MultiBit { namespace VirtualExact {
   
template <class TypeBase, class TypeMultiTraits> class TMultiBitElement;

}} // end of namespace MultiBit::VirtualExact

namespace Floating { namespace VirtualExact {
   
template <class TypeBase, class TypeMultiTraits> class TMultiFloatElement;

}} // end of namespace Floating::VirtualExact

namespace VirtualExact {

template <class TypeBase> class TUndefElement;

}}} // end of namespace Analyzer::Scalar::VirtualExact

#include "Analyzer/Scalar/VirtualExact/General/BitElement.h"
#include "Analyzer/Scalar/VirtualExact/General/MultiBitElement.h"
#include "Analyzer/Scalar/VirtualExact/General/MultiFloatElement.h"
#include "Analyzer/Scalar/VirtualExact/TopElement.h"

#endif // Analyzer_Scalar_VirtualExact_GeneralVirtualExactH

