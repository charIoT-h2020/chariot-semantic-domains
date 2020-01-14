/////////////////////////////////
//
// Library   : Analyzer/Scalar
// Unit      : approximate native scalar elements
// File      : ConstantMultiBit.cpp
// Author    : Franck Vedrine
// Copyright : CEA LIST - 2019-2020 - all rights reserved
//
// Description :
//   Implementation of an integer class of multibit elements.
//   This definition relies on independent host analyses.
//

#include "Analyzer/Scalar/Approximate/General/ConstantMultiBit.h"
// #include "Analyzer/Scalar/Scalar.hpp"

namespace Analyzer {}

#include "Analyzer/Scalar/Approximate/General/ConstantMultiBit.template"

namespace Analyzer { namespace Scalar {

namespace MultiBit { namespace Approximate {

namespace DConstantElement {

bool
Base::applyCompareFloat(const VirtualOperation& aoperation, EvaluationEnvironment& env) {
   typedef Scalar::DReal::TCastMultiFloatOperation<Operation> FloatOperation;
   AssumeCondition(dynamic_cast<const FloatOperation*>(&aoperation))
   AssumeCondition(dynamic_cast<const Base*>(&env.getFirstArgument()))
   const FloatOperation& operation = (const FloatOperation&) aoperation;
   CastMultiFloatPointerOperation castFloatOperation;
   castFloatOperation.setSizeExponent(operation.getSizeExponent());
   castFloatOperation.setSizeMantissa(operation.getSizeMantissa());
   castFloatOperation.setSigned(operation.isSigned());
   PPVirtualElement thisCast = inherited::Methods::apply(*this, castFloatOperation, EvaluationParameters(env));
   PPVirtualElement sourceCast = inherited::Methods::apply(env.getFirstArgument(), castFloatOperation, EvaluationParameters(env));
   TransmitEnvironment applyEnv(env);
   applyEnv.setFirstArgument(*sourceCast);
   Scalar::Floating::Operation floatOperation;
   switch (operation.getType()) {
      case Operation::TCompareLessFloat: floatOperation.setCompareLess(); break;
      case Operation::TCompareLessOrEqualFloat: floatOperation.setCompareLessOrEqual(); break;
      case Operation::TCompareEqualFloat: floatOperation.setCompareEqual(); break;
      case Operation::TCompareDifferentFloat: floatOperation.setCompareDifferent(); break;
      case Operation::TCompareGreaterOrEqualFloat: floatOperation.setCompareGreaterOrEqual(); break;
      case Operation::TCompareGreaterFloat: floatOperation.setCompareGreater(); break;
      default: AssumeUncalled
   };
   thisCast->apply(floatOperation, applyEnv);
   env.presult() = applyEnv.presult();
   return true;
}

bool
Base::constraintCompareFloat(const VirtualOperation& aoperation,
      VirtualElement& source, ConstraintEnvironment::Argument arg, ConstraintEnvironment& env) {
   typedef Scalar::DReal::TCastMultiFloatOperation<Operation> FloatOperation;
   AssumeCondition(dynamic_cast<const FloatOperation*>(&aoperation))
   const FloatOperation& operation = (const FloatOperation&) aoperation;
   Scalar::Floating::Operation floatOperation;
   switch (operation.getType()) {
      case Operation::TCompareLessFloat:
         if (arg.isResult())
            floatOperation.setCompareLess();
         else
            floatOperation.setCompareGreater();
         break;
      case Operation::TCompareLessOrEqualFloat:
         if (arg.isResult())
            floatOperation.setCompareLessOrEqual();
         else
            floatOperation.setCompareGreaterOrEqual();
         break;
      case Operation::TCompareEqualFloat: floatOperation.setCompareEqual(); break;
      case Operation::TCompareDifferentFloat: floatOperation.setCompareDifferent(); break;
      case Operation::TCompareGreaterOrEqualFloat:
         if (arg.isResult())
            floatOperation.setCompareGreaterOrEqual();
         else
            floatOperation.setCompareLessOrEqual();
         break;
      case Operation::TCompareGreaterFloat:
         if (arg.isResult())
            floatOperation.setCompareGreater();
         else
            floatOperation.setCompareLess();
         break;
      default:
         AssumeUncalled
   };

   CastMultiFloatPointerOperation castFloatOperation;
   castFloatOperation.setSizeExponent(operation.getSizeExponent());
   castFloatOperation.setSizeMantissa(operation.getSizeMantissa());
   castFloatOperation.setSigned(operation.isSigned());
   PPVirtualElement thisCast = Methods::apply(*this, castFloatOperation, EvaluationEnvironment::Init(env));
   PPVirtualElement sourceCast = Methods::apply(source, castFloatOperation, EvaluationEnvironment::Init(env));
   ConstraintTransmitEnvironment constraintEnv(env, sourceCast, arg);

   thisCast->constraint(floatOperation, *Methods::newTrue(*thisCast), constraintEnv);
   sourceCast.absorbElement((VirtualElement*) constraintEnv.extractFirstArgument());
   Scalar::Floating::CastMultiBitOperationPointer castResult;
   castResult.setSize(getSizeInBits());
   castResult.setSigned(operation.isSigned());
   sourceCast = Methods::apply(*sourceCast, castResult, EvaluationEnvironment::Init(env));
   IntersectEnvironment intersectEnv(env, arg.queryInverse());
   source.intersectWith(*sourceCast, intersectEnv);
   if (intersectEnv.hasResult())
      env.absorb((VirtualElement*) intersectEnv.presult().extractElement(), arg.queryInverse());
   return true;
}

} // end of namespace DConstantElement

}}}} // end of namespace Analyzer::Scalar::MultiBit::Approximate

template class
Analyzer::Scalar::MultiBit::Approximate::DConstantElement::TConstantElement<
      Analyzer::Scalar::MultiBit::Implementation::MultiBitElement>;

