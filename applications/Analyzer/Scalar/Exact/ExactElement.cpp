/////////////////////////////////
//
// Library   : Analyzer/Scalar
// Unit      : scalar elements common to compilation constants and symbolic execution
// File      : ExactElement.h
// Author    : Franck Vedrine
// Copyright : CEA LIST - 2019-2020 - all rights reserved
//
// Description :
//   Implementation of scalar elements used in simulation/symbolic execution.
//

#include "Analyzer/Scalar/Exact/ExactElement.h"
// #include "Analyzer/Scalar/Scalar.hpp"

namespace Analyzer {}

#include "Analyzer/Scalar/VirtualExact/NativeVirtualExact.template"
#include "Analyzer/Scalar/VirtualExact/GeneralVirtualExact.template"

namespace Analyzer { namespace Scalar { namespace Exact {

VirtualElement*
VirtualElement::createBitElement(bool value)
   { return new BitElement(value); }

VirtualElement*
VirtualElement::createIntElement(int value)
   { return new IntElement(Init().setInitialValue(Scalar::Integer::Implementation::IntElement(value))); }

VirtualElement*
VirtualElement::createIntElement(const Init& init) { return new IntElement(init); }

VirtualElement*
VirtualElement::createBoolElement(bool value)
   { return new IntElement(Init().setInitialValue(Scalar::Integer::Implementation::IntElement(value))); }

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
   {  return new UndefElement(init); }

} // end of namespace Exact

template class Bit::VirtualExact::TBitElement<Exact::VirtualElement>;
template class Integer::VirtualExact::TCharElement<Exact::VirtualElement>;
template class Integer::VirtualExact::TSignedCharElement<Exact::VirtualElement>;
template class Integer::VirtualExact::TUnsignedCharElement<Exact::VirtualElement>;
template class Integer::VirtualExact::TShortElement<Exact::VirtualElement>;
template class Integer::VirtualExact::TUnsignedShortElement<Exact::VirtualElement>;
template class Integer::VirtualExact::TIntElement<Exact::VirtualElement>;
template class Integer::VirtualExact::TUnsignedIntElement<Exact::VirtualElement>;
template class Integer::VirtualExact::TLongElement<Exact::VirtualElement>;
template class Integer::VirtualExact::TUnsignedLongElement<Exact::VirtualElement>;
template class Floating::VirtualExact::TFloatElement<Exact::VirtualElement>;
template class Floating::VirtualExact::TDoubleElement<Exact::VirtualElement>;
template class Floating::VirtualExact::TLongDoubleElement<Exact::VirtualElement>;
template class VirtualExact::TUndefElement<Exact::VirtualElement>;
template class MultiBit::VirtualExact::TMultiBitElement<Exact::VirtualElement, MultiBit::Implementation::Details::MultiBitElement>;

}} // end of namespace Analyzer::Scalar


