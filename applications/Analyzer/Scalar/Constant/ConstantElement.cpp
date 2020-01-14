/////////////////////////////////
//
// Library   : Analyzer/Scalar
// Unit      : scalar elements common to compilation constants and symbolic execution
// File      : ConstantElement.cpp
// Author    : Franck Vedrine
// Copyright : CEA LIST - 2019-2020 - all rights reserved
//
// Description :
//   Implementation of the compilation constants.
//   All this stuff is rather defined for source code analysis.
//

#include "Analyzer/Scalar/Constant/ConstantElement.h"
// #include "Analyzer/Scalar/Scalar.hpp"

namespace Analyzer {}

#include "Analyzer/Scalar/VirtualExact/NativeVirtualExact.template"
#include "Analyzer/Scalar/VirtualExact/GeneralVirtualExact.template"
#include "Numerics/HostFloating.template"

namespace Analyzer { namespace Scalar { namespace Constant {

VirtualElement*
VirtualElement::createBitElement(bool value)
   {  return new BitElement(Init().setInitialValue(Integer::Implementation::IntElement(value))); }

VirtualElement*
VirtualElement::createIntElement(int value)
   {  return new IntElement(Init().setInitialValue(Integer::Implementation::IntElement(value))); }
   
VirtualElement*
VirtualElement::createIntElement(const Init& init) { return new IntElement(init); }

VirtualElement*
VirtualElement::createBoolElement(bool value)
   {  return new IntElement(Init().setInitialValue(Integer::Implementation::IntElement(value))); }

VirtualElement*
VirtualElement::createDoubleElement(const Init& init) { return new DoubleElement(init); }

VirtualElement*
VirtualElement::createCharElement(const Init& init) { return new CharElement(init); }

VirtualElement*
VirtualElement::createSignedCharElement(const Init& init) { return new SignedCharElement(init); }

VirtualElement*
VirtualElement::createUnsignedCharElement(const Init& init) { return new UnsignedCharElement(init); }

VirtualElement*
VirtualElement::createShortElement(const Init& init) { return new ShortElement(init); }

VirtualElement*
VirtualElement::createUnsignedShortElement(const Init& init) { return new UnsignedShortElement(init); }

VirtualElement*
VirtualElement::createUnsignedIntElement(const Init& init) { return new UnsignedIntElement(init); }

VirtualElement*
VirtualElement::createLongElement(const Init& init) { return new LongElement(init); }

VirtualElement*
VirtualElement::createUnsignedLongElement(const Init& init) { return new UnsignedLongElement(init); }

VirtualElement*
VirtualElement::createFloatElement(const Init& init) { return new FloatElement(init); }

VirtualElement*
VirtualElement::createLongDoubleElement(const Init& init) { return new LongDoubleElement(init); }


VirtualElement*
VirtualElement::createUndefined(const Init& init)
   { return new UndefElement(init); }

} // end of namespace Constant

template class Bit::VirtualExact::TBitElement<Constant::VirtualElement>;
template class Integer::VirtualExact::TCharElement<Constant::VirtualElement>;
template class Integer::VirtualExact::TSignedCharElement<Constant::VirtualElement>;
template class Integer::VirtualExact::TUnsignedCharElement<Constant::VirtualElement>;
template class Integer::VirtualExact::TShortElement<Constant::VirtualElement>;
template class Integer::VirtualExact::TUnsignedShortElement<Constant::VirtualElement>;
template class Integer::VirtualExact::TIntElement<Constant::VirtualElement>;
template class Integer::VirtualExact::TUnsignedIntElement<Constant::VirtualElement>;
template class Integer::VirtualExact::TLongElement<Constant::VirtualElement>;
template class Integer::VirtualExact::TUnsignedLongElement<Constant::VirtualElement>;

template class Floating::VirtualExact::TFloatElement<Constant::VirtualElement>;
template class Floating::VirtualExact::TDoubleElement<Constant::VirtualElement>;
template class Floating::VirtualExact::TLongDoubleElement<Constant::VirtualElement>;
template class VirtualExact::TUndefElement<Constant::VirtualElement>;
   
}} // end of namespace Analyzer::Scalar

template class Numerics::TFloatingBase<Numerics::FloatTraits>;
template class Numerics::TFloatingBase<Numerics::DoubleTraits>;
template class Numerics::TFloatingBase<Numerics::LongDoubleTraits>;

