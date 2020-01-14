/////////////////////////////////
//
// Library   : Analyzer/Scalar
// Unit      : approximate native scalar elements
// File      : ConstantIntegerElement.cpp
// Author    : Franck Vedrine
// Copyright : CEA LIST - 2019-2020 - all rights reserved
//
// Description :
//   Implementation of a class of integer elements that rely on native host implementation.
//   All this stuff is rather defined for source code analysis.
//

#include "Analyzer/Scalar/Approximate/Native/ConstantIntegerElement.h"
// #include "Analyzer/Scalar/Scalar.hpp"

namespace Analyzer {}

#include "Analyzer/Scalar/Approximate/VirtualElement.template"
#include "Analyzer/Scalar/Approximate/Native/ConstantNativeElement.template"
#include "Analyzer/Scalar/Approximate/BitFieldMask.template"

#include "Numerics/Floating.template"
#include "Numerics/HostFloating.template"

namespace Analyzer { namespace Scalar { namespace Integer { namespace Approximate {

/*******************************************/
/* Implementation of the class BaseElement */
/*******************************************/

bool
BaseElement::constraintPlusAssign(const VirtualOperation&, VirtualElement& source, const VirtualElement& result,
      Argument arg, ConstraintEnvironment& env) {
   IntersectEnvironment intersectEnv(env, arg.queryInverse());
   PPVirtualElement newElement = Methods::apply(result, Operation().setMinus(), *this, EvaluationApplyParameters(env));
   source.intersectWith(*newElement, intersectEnv);
   return true;
}

bool
BaseElement::constraintMinusAssign(const VirtualOperation&, VirtualElement& source, const VirtualElement& result,
      Argument arg, ConstraintEnvironment& env) {
   IntersectEnvironment intersectEnv(env, arg.queryInverse());
   PPVirtualElement newElement = (arg.isResult())
      ? Methods::apply(result, Operation().setPlus(), *this, EvaluationApplyParameters(env))
      : Methods::apply(*this, Operation().setMinus(), result, EvaluationApplyParameters(env));
   source.intersectWith(*newElement, intersectEnv);
   return true;
}

bool
IntElement::applyLogicalAndAssign(const VirtualOperation& operation, EvaluationEnvironment& env) {
   inheritedImplementation::assign(inheritedImplementation::getElement()
         && getSameArgument(env).inheritedImplementation::getElement());
   return true;
}
   
bool
IntElement::applyLogicalOrAssign(const VirtualOperation& operation, EvaluationEnvironment& env) {
   inheritedImplementation::assign(inheritedImplementation::getElement()
         || getSameArgument(env).inheritedImplementation::getElement());
   return true;
}
   
bool
IntElement::applyLogicalNegateAssign(const VirtualOperation& operation, EvaluationEnvironment& env) {
   inheritedImplementation::assign(!inheritedImplementation::getElement());
   return true;
}
   
bool
IntElement::constraintLeftShiftAssignWithConstantResultOnArgSource(const VirtualOperation& operation,
      VirtualElement& source, const VirtualElement& result, ConstraintEnvironment& env) {
   AssumeCondition(dynamic_cast<const BaseElement*>(&result))
   return ((const BaseElement&) result).constraintFromResultLeftShiftAssignWithConstantResultOnArgSource(
         *this, operation, source, env);
}

bool
IntElement::constraintRightShiftAssignWithConstantResultOnArgSource(const VirtualOperation& operation,
      VirtualElement& source, const VirtualElement& result, ConstraintEnvironment& env) {
   AssumeCondition(dynamic_cast<const BaseElement*>(&result))
   return ((const BaseElement&) result).constraintFromResultRightShiftAssignWithConstantResultOnArgSource(
         *this, operation, source, env);
}

bool
IntElement::constraintRotateAssignWithConstantResultOnArgSource(const VirtualOperation& operation,
      VirtualElement& source, const VirtualElement& result, ConstraintEnvironment& env) {
   AssumeCondition(dynamic_cast<const BaseElement*>(&result))
   return ((const BaseElement&) result).constraintFromResultRotateAssignWithConstantResultOnArgSource(
         *this, operation, source, env);
}

template class TBaseElement<BaseElement, IntElement>;
template class TBaseElement<BaseElement, UnsignedIntElement>;
template class TBaseElement<Details::SubElement, CharElement>;
template class TBaseElement<Details::SubElement, SignedCharElement>;
template class TBaseElement<Details::SubElement, UnsignedCharElement>;
template class TBaseElement<Details::SubElement, ShortElement>;
template class TBaseElement<Details::SubElement, UnsignedShortElement>;
template class TBaseElement<Details::SubElement, LongElement>;
template class TBaseElement<Details::SubElement, UnsignedLongElement>;

}} // end of namespace Integer::Approximate

namespace Approximate { namespace Details {

template bool TBaseElement<Integer::Approximate::BaseElement, Integer::Approximate::IntElement>::queryBound(Details::IntOperationElement::QueryOperation const&, VirtualQueryOperation::Environment&) const;
template bool TBaseElement<Integer::Approximate::BaseElement, Integer::Approximate::UnsignedIntElement>::queryBound(Details::IntOperationElement::QueryOperation const&, VirtualQueryOperation::Environment&) const;
template bool TBaseElement<Integer::Approximate::Details::SubElement, Integer::Approximate::CharElement>::queryBound(Details::IntOperationElement::QueryOperation const&, VirtualQueryOperation::Environment&) const;
template bool TBaseElement<Integer::Approximate::Details::SubElement, Integer::Approximate::SignedCharElement>::queryBound(Details::IntOperationElement::QueryOperation const&, VirtualQueryOperation::Environment&) const;
template bool TBaseElement<Integer::Approximate::Details::SubElement, Integer::Approximate::UnsignedCharElement>::queryBound(Details::IntOperationElement::QueryOperation const&, VirtualQueryOperation::Environment&) const;
template bool TBaseElement<Integer::Approximate::Details::SubElement, Integer::Approximate::ShortElement>::queryBound(Details::IntOperationElement::QueryOperation const&, VirtualQueryOperation::Environment&) const;
template bool TBaseElement<Integer::Approximate::Details::SubElement, Integer::Approximate::UnsignedShortElement>::queryBound(Details::IntOperationElement::QueryOperation const&, VirtualQueryOperation::Environment&) const;
template bool TBaseElement<Integer::Approximate::Details::SubElement, Integer::Approximate::LongElement>::queryBound(Details::IntOperationElement::QueryOperation const&, VirtualQueryOperation::Environment&) const;
template bool TBaseElement<Integer::Approximate::Details::SubElement, Integer::Approximate::UnsignedLongElement>::queryBound(Details::IntOperationElement::QueryOperation const&, VirtualQueryOperation::Environment&) const;

}} // end of namespace Approximate::Details

}} // end of namespace Analyzer::Scalar

namespace Analyzer { namespace Scalar { namespace Integer { namespace Approximate {

class CastRingTraits {
  public:
   static VirtualElement* createCharElement(const VirtualElement::Init& iInit);
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
   static Constant::VirtualElement* createConstantMultiFloatElement(const Constant::VirtualElement::InitFloat& init);
};

}} // end of namespace Integer::Approximate

template class Approximate::Details::TCloseCastIntegerBasedElement<Integer::Approximate::IntElement, Integer::Approximate::CastRingTraits>;
template class Approximate::Details::TCloseCastIntegerBasedElement<Integer::Approximate::UnsignedIntElement, Integer::Approximate::CastRingTraits>;
template class Approximate::Details::TCloseCastIntegerBasedElement<Integer::Approximate::CharElement, Integer::Approximate::CastRingTraits>;
template class Approximate::Details::TCloseCastIntegerBasedElement<Integer::Approximate::SignedCharElement, Integer::Approximate::CastRingTraits>;
template class Approximate::Details::TCloseCastIntegerBasedElement<Integer::Approximate::UnsignedCharElement, Integer::Approximate::CastRingTraits>;
template class Approximate::Details::TCloseCastIntegerBasedElement<Integer::Approximate::ShortElement, Integer::Approximate::CastRingTraits>;
template class Approximate::Details::TCloseCastIntegerBasedElement<Integer::Approximate::UnsignedShortElement, Integer::Approximate::CastRingTraits>;
template class Approximate::Details::TCloseCastIntegerBasedElement<Integer::Approximate::LongElement, Integer::Approximate::CastRingTraits>;
template class Approximate::Details::TCloseCastIntegerBasedElement<Integer::Approximate::UnsignedLongElement, Integer::Approximate::CastRingTraits>;

template class Approximate::Details::TCloseCastElement<Integer::Approximate::IntElement>;
template class Approximate::Details::TCloseCastElement<Integer::Approximate::UnsignedIntElement>;
template class Approximate::Details::TCloseCastElement<Integer::Approximate::CharElement>;
template class Approximate::Details::TCloseCastElement<Integer::Approximate::SignedCharElement>;
template class Approximate::Details::TCloseCastElement<Integer::Approximate::UnsignedCharElement>;
template class Approximate::Details::TCloseCastElement<Integer::Approximate::ShortElement>;
template class Approximate::Details::TCloseCastElement<Integer::Approximate::UnsignedShortElement>;
template class Approximate::Details::TCloseCastElement<Integer::Approximate::LongElement>;
template class Approximate::Details::TCloseCastElement<Integer::Approximate::UnsignedLongElement>;

namespace Integer { namespace Approximate {

VirtualElement*
CastRingTraits::createCharElement(const VirtualElement::Init& init)
   {  return new Scalar::Approximate::Details::TCloseCastIntegerBasedElement<CharElement, CastRingTraits>(init); }

VirtualElement*
CastRingTraits::createSignedCharElement(const VirtualElement::Init& init)
   {  return new Scalar::Approximate::Details::TCloseCastIntegerBasedElement<SignedCharElement, CastRingTraits>(init); }

VirtualElement*
CastRingTraits::createUnsignedCharElement(const VirtualElement::Init& init)
   {  return new Scalar::Approximate::Details::TCloseCastIntegerBasedElement<UnsignedCharElement, CastRingTraits>(init); }

VirtualElement*
CastRingTraits::createShortElement(const VirtualElement::Init& init)
   {  return new Scalar::Approximate::Details::TCloseCastIntegerBasedElement<ShortElement, CastRingTraits>(init); }

VirtualElement*
CastRingTraits::createUnsignedShortElement(const VirtualElement::Init& init)
   {  return new Scalar::Approximate::Details::TCloseCastIntegerBasedElement<UnsignedShortElement, CastRingTraits>(init); }

VirtualElement*
CastRingTraits::createIntElement(const VirtualElement::Init& init)
   {  return new Scalar::Approximate::Details::TCloseCastIntegerBasedElement<IntElement, CastRingTraits>(init); }

VirtualElement*
CastRingTraits::createUnsignedIntElement(const VirtualElement::Init& init)
   {  return new Scalar::Approximate::Details::TCloseCastIntegerBasedElement<UnsignedIntElement, CastRingTraits>(init); }

VirtualElement*
CastRingTraits::createLongElement(const VirtualElement::Init& init)
   {  return new Scalar::Approximate::Details::TCloseCastIntegerBasedElement<LongElement, CastRingTraits>(init); }

VirtualElement*
CastRingTraits::createUnsignedLongElement(const VirtualElement::Init& init)
   {  return new Scalar::Approximate::Details::TCloseCastIntegerBasedElement<UnsignedLongElement, CastRingTraits>(init); }

VirtualElement*
CastRingTraits::createFloatElement(const VirtualElement::InitFloat& init)
   {  return new Scalar::Approximate::Details::TCloseCastFloatingBasedElement<Floating::Approximate::FloatElement, CastRingTraits>(init); }

VirtualElement*
CastRingTraits::createDoubleElement(const VirtualElement::InitFloat& init)
   {  return new Scalar::Approximate::Details::TCloseCastFloatingBasedElement<Floating::Approximate::DoubleElement, CastRingTraits>(init); }

VirtualElement*
CastRingTraits::createLongDoubleElement(const VirtualElement::InitFloat& init)
   {  return new Scalar::Approximate::Details::TCloseCastFloatingBasedElement<Floating::Approximate::LongDoubleElement, CastRingTraits>(init); }

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

Constant::VirtualElement*
CastRingTraits::createConstantMultiFloatElement(const Constant::VirtualElement::InitFloat& init)
   {  return nullptr; }

}}}} // end of namespace Analyzer::Scalar::Approximate::Integer

