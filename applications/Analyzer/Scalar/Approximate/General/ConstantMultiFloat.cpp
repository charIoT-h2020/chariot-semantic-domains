/////////////////////////////////
//
// Library   : Analyzer/Scalar
// Unit      : approximate native scalar elements
// File      : ConstantMultiFloat.cpp
// Author    : Franck Vedrine
// Copyright : CEA LIST - 2019-2020 - all rights reserved
//
// Description :
//   Implementation of a floating point class of multibit elements
//     for the mantissa and the exponent.
//   This definition relies on independent host analyses.
//

#include "Analyzer/Scalar/Approximate/General/ConstantMultiFloat.h"
// #include "Analyzer/Scalar/Scalar.hpp"

namespace Analyzer {}

#include "Analyzer/Scalar/Approximate/General/ConstantMultiFloat.template"

namespace Analyzer { namespace Scalar {

namespace Floating { namespace Approximate {

namespace DMultiConstantElement {

bool
Base::querySimplification(const QueryOperation&, QueryOperation::Environment& env) const {
   return true;
}
   
} // end of namespace DMultiConstantElement
   
class CastRingTraits {
  public:
   static VirtualElement* createCharElement(const VirtualElement::Init& init) { AssumeUncalled return nullptr; }
   static VirtualElement* createSignedCharElement(const VirtualElement::Init& init) { AssumeUncalled return nullptr; }
   static VirtualElement* createUnsignedCharElement(const VirtualElement::Init& init) { AssumeUncalled return nullptr; }
   static VirtualElement* createShortElement(const VirtualElement::Init& init) { AssumeUncalled return nullptr; }
   static VirtualElement* createUnsignedShortElement(const VirtualElement::Init& init) { AssumeUncalled return nullptr; }
   static VirtualElement* createIntElement(const VirtualElement::Init& init) { AssumeUncalled return nullptr; }
   static VirtualElement* createUnsignedIntElement(const VirtualElement::Init& init) { AssumeUncalled return nullptr; }
   static VirtualElement* createLongElement(const VirtualElement::Init& init) { AssumeUncalled return nullptr; }
   static VirtualElement* createUnsignedLongElement(const VirtualElement::Init& init) { AssumeUncalled return nullptr; }
   static VirtualElement* createFloatElement(const VirtualElement::InitFloat& init) { AssumeUncalled return nullptr; }
   static VirtualElement* createDoubleElement(const VirtualElement::InitFloat& init) { AssumeUncalled return nullptr; }
   static VirtualElement* createLongDoubleElement(const VirtualElement::InitFloat& init) { AssumeUncalled return nullptr; }
   static VirtualElement* createBitElement(const VirtualElement::Init& init) { AssumeUncalled return nullptr; }
   static VirtualElement* createMultiBitElement(const VirtualElement::Init& init) { AssumeUncalled return nullptr; }
   static VirtualElement* createMultiFloatElement(const VirtualElement::InitFloat& init) { AssumeUncalled return nullptr; }
   static Constant::VirtualElement* createConstantMultiFloatElement(const Constant::VirtualElement::InitFloat& init) { AssumeUncalled return nullptr; }
};

}}}} // end of namespace Analyzer::Scalar::Floating::Approximate

template class
Analyzer::Scalar::Floating::Approximate::DMultiConstantElement::TMultiConstantElement<
      Analyzer::Scalar::Floating::Implementation::MultiFloatElement>;

template class
Analyzer::Scalar::Floating::Approximate::TCloseMultiConstantElement<
      Analyzer::Scalar::Floating::Approximate::MultiConstantElement,
      Analyzer::Scalar::Floating::Approximate::CastRingTraits>;

