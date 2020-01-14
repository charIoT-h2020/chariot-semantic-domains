/////////////////////////////////
//
// Library   : Analyzer/Scalar
// Unit      : approximate native scalar elements
// File      : ConstantFloatElement.cpp
// Author    : Franck Vedrine
// Copyright : CEA LIST - 2019-2020 - all rights reserved
//
// Description :
//   Implementation of a class of floating-point elements that rely on native host implementation.
//   All this stuff is rather defined for source code analysis.
//

#include "Analyzer/Scalar/Approximate/Native/ConstantFloatElement.h"
// #include "Analyzer/Scalar/Scalar.hpp"

namespace Analyzer {}

#include "Analyzer/Scalar/Approximate/Native/ConstantFloatElement.template"

namespace Analyzer { namespace Scalar { namespace Floating { namespace Approximate {

bool
BaseElement::querySimplification(const QueryOperation&, QueryOperation::Environment& env) const {
   return true;
}
   
/*
template class Approximate::Floating::TBaseElement<Approximate::Floating::BaseElement, Approximate::Floating::FloatElement>;
template class Approximate::Floating::TBaseElement<Approximate::Floating::BaseElement, Approximate::Floating::DoubleElement>;
template class Approximate::Floating::TBaseElement<Approximate::Floating::BaseElement, Approximate::Floating::LongDoubleElement>;
*/

template bool TBaseElement<BaseElement, FloatElement>::constraintArithmeticAssignWithConstantResult(VirtualOperation const&, Approximate::VirtualElement&, Approximate::VirtualElement const&, Approximate::ConstraintEnvironment::Argument, Approximate::ConstraintEnvironment&);
template bool TBaseElement<BaseElement, FloatElement>::applyPlusAssign(VirtualOperation const&, Approximate::EvaluationEnvironment&);
template bool TBaseElement<BaseElement, FloatElement>::applyMinusAssign(VirtualOperation const&, Approximate::EvaluationEnvironment&);
template bool TBaseElement<BaseElement, FloatElement>::applyTimesAssign(VirtualOperation const&, Approximate::EvaluationEnvironment&);
template bool TBaseElement<BaseElement, FloatElement>::applyDivideAssign(VirtualOperation const&, Approximate::EvaluationEnvironment&);
template bool TBaseElement<BaseElement, FloatElement>::applyOppositeAssign(VirtualOperation const&, Approximate::EvaluationEnvironment&);
template bool TBaseElement<BaseElement, FloatElement>::applyAbsAssign(VirtualOperation const&, Approximate::EvaluationEnvironment&);
template bool TBaseElement<BaseElement, DoubleElement>::constraintArithmeticAssignWithConstantResult(VirtualOperation const&, Approximate::VirtualElement&, Approximate::VirtualElement const&, Approximate::ConstraintEnvironment::Argument, Approximate::ConstraintEnvironment&);
template bool TBaseElement<BaseElement, DoubleElement>::applyPlusAssign(VirtualOperation const&, Approximate::EvaluationEnvironment&);
template bool TBaseElement<BaseElement, DoubleElement>::applyMinusAssign(VirtualOperation const&, Approximate::EvaluationEnvironment&);
template bool TBaseElement<BaseElement, DoubleElement>::applyTimesAssign(VirtualOperation const&, Approximate::EvaluationEnvironment&);
template bool TBaseElement<BaseElement, DoubleElement>::applyDivideAssign(VirtualOperation const&, Approximate::EvaluationEnvironment&);
template bool TBaseElement<BaseElement, DoubleElement>::applyOppositeAssign(VirtualOperation const&, Approximate::EvaluationEnvironment&);
template bool TBaseElement<BaseElement, DoubleElement>::applyAbsAssign(VirtualOperation const&, Approximate::EvaluationEnvironment&);
template bool TBaseElement<BaseElement, DoubleElement>::applyAcos(VirtualOperation const&, Approximate::EvaluationEnvironment&);
template bool TBaseElement<BaseElement, DoubleElement>::applyAsin(VirtualOperation const&, Approximate::EvaluationEnvironment&);
template bool TBaseElement<BaseElement, DoubleElement>::applyAtan(VirtualOperation const&, Approximate::EvaluationEnvironment&);
template bool TBaseElement<BaseElement, DoubleElement>::applyAtan2(VirtualOperation const&, Approximate::EvaluationEnvironment&);
template bool TBaseElement<BaseElement, DoubleElement>::applyCeil(VirtualOperation const&, Approximate::EvaluationEnvironment&);
template bool TBaseElement<BaseElement, DoubleElement>::applyCos(VirtualOperation const&, Approximate::EvaluationEnvironment&);
template bool TBaseElement<BaseElement, DoubleElement>::applyCosh(VirtualOperation const&, Approximate::EvaluationEnvironment&);
template bool TBaseElement<BaseElement, DoubleElement>::applyExp(VirtualOperation const&, Approximate::EvaluationEnvironment&);
template bool TBaseElement<BaseElement, DoubleElement>::applyFabs(VirtualOperation const&, Approximate::EvaluationEnvironment&);
template bool TBaseElement<BaseElement, DoubleElement>::applyFloor(VirtualOperation const&, Approximate::EvaluationEnvironment&);
template bool TBaseElement<BaseElement, DoubleElement>::applyFmod(VirtualOperation const&, Approximate::EvaluationEnvironment&);
template bool TBaseElement<BaseElement, DoubleElement>::applyFrexp(VirtualOperation const&, Approximate::EvaluationEnvironment&);
template bool TBaseElement<BaseElement, DoubleElement>::applyLdexp(VirtualOperation const&, Approximate::EvaluationEnvironment&);
template bool TBaseElement<BaseElement, DoubleElement>::applyLog(VirtualOperation const&, Approximate::EvaluationEnvironment&);
template bool TBaseElement<BaseElement, DoubleElement>::applyLog10(VirtualOperation const&, Approximate::EvaluationEnvironment&);
template bool TBaseElement<BaseElement, DoubleElement>::applyModf(VirtualOperation const&, Approximate::EvaluationEnvironment&);
template bool TBaseElement<BaseElement, DoubleElement>::applyPow(VirtualOperation const&, Approximate::EvaluationEnvironment&);
template bool TBaseElement<BaseElement, DoubleElement>::applySin(VirtualOperation const&, Approximate::EvaluationEnvironment&);
template bool TBaseElement<BaseElement, DoubleElement>::applySinh(VirtualOperation const&, Approximate::EvaluationEnvironment&);
template bool TBaseElement<BaseElement, DoubleElement>::applySqrt(VirtualOperation const&, Approximate::EvaluationEnvironment&);
template bool TBaseElement<BaseElement, DoubleElement>::applyTan(VirtualOperation const&, Approximate::EvaluationEnvironment&);
template bool TBaseElement<BaseElement, DoubleElement>::applyTanh(VirtualOperation const&, Approximate::EvaluationEnvironment&);
template bool TBaseElement<BaseElement, LongDoubleElement>::constraintArithmeticAssignWithConstantResult(VirtualOperation const&, Approximate::VirtualElement&, Approximate::VirtualElement const&, Approximate::ConstraintEnvironment::Argument, Approximate::ConstraintEnvironment&);
template bool TBaseElement<BaseElement, LongDoubleElement>::applyPlusAssign(VirtualOperation const&, Approximate::EvaluationEnvironment&);
template bool TBaseElement<BaseElement, LongDoubleElement>::applyMinusAssign(VirtualOperation const&, Approximate::EvaluationEnvironment&);
template bool TBaseElement<BaseElement, LongDoubleElement>::applyTimesAssign(VirtualOperation const&, Approximate::EvaluationEnvironment&);
template bool TBaseElement<BaseElement, LongDoubleElement>::applyDivideAssign(VirtualOperation const&, Approximate::EvaluationEnvironment&);
template bool TBaseElement<BaseElement, LongDoubleElement>::applyOppositeAssign(VirtualOperation const&, Approximate::EvaluationEnvironment&);
template bool TBaseElement<BaseElement, LongDoubleElement>::applyAbsAssign(VirtualOperation const&, Approximate::EvaluationEnvironment&);
template bool TBaseElement<BaseElement, FloatElement>::querySizes(Approximate::Details::RealOperationElement::QueryOperation const&, Approximate::VirtualQueryOperation::Environment&) const;
template bool TBaseElement<BaseElement, FloatElement>::queryCompareSpecial(Approximate::Details::RealOperationElement::QueryOperation const&, Approximate::VirtualQueryOperation::Environment&) const;
template bool TBaseElement<BaseElement, FloatElement>::queryGuardAll(Approximate::Details::RealOperationElement::QueryOperation const&, Approximate::VirtualQueryOperation::Environment&) const;
template bool TBaseElement<BaseElement, DoubleElement>::querySizes(Approximate::Details::RealOperationElement::QueryOperation const&, Approximate::VirtualQueryOperation::Environment&) const;
template bool TBaseElement<BaseElement, DoubleElement>::queryCompareSpecial(Approximate::Details::RealOperationElement::QueryOperation const&, Approximate::VirtualQueryOperation::Environment&) const;
template bool TBaseElement<BaseElement, DoubleElement>::queryGuardAll(Approximate::Details::RealOperationElement::QueryOperation const&, Approximate::VirtualQueryOperation::Environment&) const;
template bool TBaseElement<BaseElement, LongDoubleElement>::querySizes(Approximate::Details::RealOperationElement::QueryOperation const&, Approximate::VirtualQueryOperation::Environment&) const;
template bool TBaseElement<BaseElement, LongDoubleElement>::queryCompareSpecial(Approximate::Details::RealOperationElement::QueryOperation const&, Approximate::VirtualQueryOperation::Environment&) const;
template bool TBaseElement<BaseElement, LongDoubleElement>::queryGuardAll(Approximate::Details::RealOperationElement::QueryOperation const&, Approximate::VirtualQueryOperation::Environment&) const;

}}}} // end of namespace Analyzer::Scalar::Floating::Approximate

/*
namespace Analyzer { namespace Scalar { namespace Floating { namespace Approximate {

class CastRingTraits {
  public:
   static VirtualElement* createCharElement(const VirtualElement::Init& init);
   static VirtualElement* createSignedCharElement(const VirtualElement::Init& init);
   static VirtualElement* createUnsignedCharElement(const VirtualElement::Init& init);
   static VirtualElement* createShortElement(const VirtualElement::Init& init);
   static VirtualElement* createUnsignedShortElement(const VirtualElement::Init& init);
   static VirtualElement* createIntElement(const VirtualElement::Init& init);
   static VirtualElement* createUnsignedIntElement(const VirtualElement::Init& init);
   static VirtualElement* createLongElement(const VirtualElement::Init& init);
   static VirtualElement* createUnsignedLongElement(const VirtualElement::Init& init);
   static VirtualElement* createFloatElement(const VirtualElement::InitFloat& init);
   static VirtualElement* createDoubleElement(const VirtualElement::InitFloat& init);
   static VirtualElement* createLongDoubleElement(const VirtualElement::InitFloat& init);
   static VirtualElement* createBitElement(const VirtualElement::Init& init);
   static VirtualElement* createMultiBitElement(const VirtualElement::Init& init);
   static VirtualElement* createMultiFloatElement(const VirtualElement::InitFloat& init);
};

template class Approximate::Details::TCloseCastFloatingBasedElement<FloatElement, CastRingTraits>;
template class Approximate::Details::TCloseCastFloatingBasedElement<DoubleElement, CastRingTraits>;
template class Approximate::Details::TCloseCastFloatingBasedElement<LongDoubleElement, CastRingTraits>;

template class Approximate::Details::TCloseCastElement<FloatElement>;
template class Approximate::Details::TCloseCastElement<DoubleElement>;
template class Approximate::Details::TCloseCastElement<LongDoubleElement>;

VirtualElement*
CastRingTraits::createCharElement(const VirtualElement::Init& init)
   {  return new Approximate::Details::TCloseCastIntegerBasedElement<Integer::CharElement, CastRingTraits>(init); }

VirtualElement*
CastRingTraits::createSignedCharElement(const VirtualElement::Init& init)
   {  return new Approximate::Details::TCloseCastIntegerBasedElement<Integer::SignedCharElement, CastRingTraits>(init); }

VirtualElement*
CastRingTraits::createUnsignedCharElement(const VirtualElement::Init& init)
   {  return new Approximate::Details::TCloseCastIntegerBasedElement<Integer::UnsignedCharElement, CastRingTraits>(init); }

VirtualElement*
CastRingTraits::createShortElement(const VirtualElement::Init& init)
   {  return new Approximate::Details::TCloseCastIntegerBasedElement<Integer::ShortElement, CastRingTraits>(init); }

VirtualElement*
CastRingTraits::createUnsignedShortElement(const VirtualElement::Init& init)
   {  return new Approximate::Details::TCloseCastIntegerBasedElement<Integer::UnsignedShortElement, CastRingTraits>(init); }

VirtualElement*
CastRingTraits::createIntElement(const VirtualElement::Init& init)
   {  return new Approximate::Details::TCloseCastIntegerBasedElement<Integer::IntElement, CastRingTraits>(init); }

VirtualElement*
CastRingTraits::createUnsignedIntElement(const VirtualElement::Init& init)
   {  return new Approximate::Details::TCloseCastIntegerBasedElement<Integer::UnsignedIntElement, CastRingTraits>(init); }

VirtualElement*
CastRingTraits::createLongElement(const VirtualElement::Init& init)
   {  return new Approximate::Details::TCloseCastIntegerBasedElement<Integer::LongElement, CastRingTraits>(init); }

VirtualElement*
CastRingTraits::createUnsignedLongElement(const VirtualElement::Init& init)
   {  return new Approximate::Details::TCloseCastIntegerBasedElement<Integer::UnsignedLongElement, CastRingTraits>(init); }

VirtualElement*
CastRingTraits::createFloatElement(const VirtualElement::InitFloat& init)
   {  return new Approximate::Details::TCloseCastFloatingBasedElement<Floating::FloatElement, CastRingTraits>(init); }

VirtualElement*
CastRingTraits::createDoubleElement(const VirtualElement::InitFloat& init)
   {  return new Approximate::Details::TCloseCastFloatingBasedElement<Floating::DoubleElement, CastRingTraits>(init); }

VirtualElement*
CastRingTraits::createLongDoubleElement(const VirtualElement::InitFloat& init)
   {  return new Approximate::Details::TCloseCastFloatingBasedElement<Floating::LongDoubleElement, CastRingTraits>(init); }

VirtualElement*
CastRingTraits::createBitElement(const VirtualElement::Init& init)
   {  return nullptr; }
// {  return new Bit::TCloseConstantElement<CastRingTraits>(init); }

VirtualElement*
CastRingTraits::createMultiBitElement(const VirtualElement::Init& init)
   {  return nullptr; }

VirtualElement*
CastRingTraits::createMultiFloatElement(const VirtualElement::InitFloat& init)
   {  return nullptr; }

}}}} // end of namespace Analyzer::Scalar::Floating::Approximate
*/

