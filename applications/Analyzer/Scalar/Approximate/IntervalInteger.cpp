/////////////////////////////////
//
// Library   : Analyzer/Scalar
// Unit      : approximate scalar elements
// File      : IntervalInteger.cpp
// Author    : Franck Vedrine
// Copyright : CEA LIST - 2019-2020 - all rights reserved
//
// Description :
//   Implementation of a class of polymorph integer intervals.
//

#include "Analyzer/Scalar/Approximate/IntervalInteger.h"
#include "Analyzer/Scalar/Approximate/Disjunction.h"
#include "Analyzer/Scalar/Approximate/Top.h"
#include "Analyzer/Scalar/Approximate/FormalOperation.h"

// #include "Analyzer/Scalar/Scalar.hpp"

namespace Analyzer {}

#include "Analyzer/Scalar/Approximate/VirtualElement.template"

namespace Analyzer { namespace Scalar { namespace Approximate {

namespace Details {

/***********************************************/
/* Implementation of the class IntegerInterval */
/***********************************************/

/* construction methods */

void
IntegerInterval::setFromLimit(PPVirtualElement limit, Init::Compare init, bool isUnsigned) {
   setUnsignedField(isUnsigned ? 1 : 0);
   switch (init) {
      case Init::ILess:
         {  auto prevAssignOperation = querySPrevAssignOperation(*limit);
            ppveMax = Methods::applyAssign(limit, prevAssignOperation, EPExactStopErrorStates);
            if (ppveMax.isValid())
               ppveMin = newSMinDomain();
         };
         break;
      case Init::ILessOrEqual:
         {  ppveMax = limit;
            ppveMin = newSMinDomain();
         };
         break;
      case Init::IEqual:
         {  ppveMax = limit;
            ppveMin.fullAssign(ppveMax);
         };
         break;
      case Init::IGreaterOrEqual:
         {  ppveMin = limit;
            ppveMax = newSMaxDomain();
         };
         break;
      case Init::IGreater:
         {  auto nextAssignOperation = querySNextAssignOperation(*limit);
            ppveMin = Methods::applyAssign(limit, nextAssignOperation, EPExactStopErrorStates);
            if (ppveMin.isValid())
               ppveMax = newSMaxDomain();
         };
         break;
   };
}

void
IntegerInterval::setFromInterval(PPVirtualElement min,
      PPVirtualElement max, Init::BoundType type, bool isUnsigned) {
   ppveMin = min;
   ppveMax = max;

   setUnsignedField(isUnsigned ? 1 : 0);
   if (type & Init::BTExcludeMin) {
      auto nextAssignOperation = querySNextAssignOperation();
      ppveMin = Methods::applyAssign(ppveMin, nextAssignOperation, EPExactStopErrorStates);
   }
   if (type & Init::BTExcludeMax) {
      auto prevAssignOperation = querySPrevAssignOperation();
      ppveMax = Methods::applyAssign(ppveMax, prevAssignOperation, EPExactStopErrorStates);
   }
}

/* query methods */

bool
IntegerInterval::queryCompareSpecial(const VirtualElement& thisElement, const VirtualQueryOperation& operation, VirtualQueryOperation::Environment& env) {
   AssumeCondition(dynamic_cast<const IntegerInterval*>(&thisElement))
   const IntegerInterval& intervalThis = (const IntegerInterval&) thisElement;
   assume(intervalThis.ppveMin->query(intervalThis.querySCompareSpecialOperation(), env));
   assume(intervalThis.ppveMax->query(intervalThis.querySCompareSpecialOperation(), env));
   ((QueryOperation::CompareSpecialEnvironment&) env).unify();
   return true;
}

bool
IntegerInterval::queryCompareSpecialSigned(const VirtualElement& thisElement, const VirtualQueryOperation& operation, VirtualQueryOperation::Environment& env) {
   AssumeCondition(dynamic_cast<const IntegerInterval*>(&thisElement))
   const IntegerInterval* intervalThis = &(const IntegerInterval&) thisElement;
   PNT::TPassPointer<IntegerInterval, PPAbstractElement> copy;
   if (intervalThis->hasUnsignedField()) {
      EvaluationEnvironment applyEnv = EvaluationEnvironment(EvaluationParameters());
      copy.setElement(*intervalThis);
      PNT::TPassPointer<VirtualDisjunction, PPAbstractElement> conversion = copy->changeSign(applyEnv);
      if (conversion.isValid()) {
         assume(conversion->query(operation, env));
         return true;
      };
      intervalThis = &*copy;
   };

   assume(intervalThis->ppveMin->query(operation, env));
   assume(intervalThis->ppveMax->query(operation, env));
   ((QueryOperation::CompareSpecialEnvironment&) env).unify();
   return true;
}

bool
IntegerInterval::queryCompareSpecialUnsigned(const VirtualElement& thisElement, const VirtualQueryOperation& operation, VirtualQueryOperation::Environment& env) {
   AssumeCondition(dynamic_cast<const IntegerInterval*>(&thisElement))
   const IntegerInterval* intervalThis = &(const IntegerInterval&) thisElement;
   PNT::TPassPointer<IntegerInterval, PPAbstractElement> copy;
   if (!intervalThis->hasUnsignedField()) {
      EvaluationEnvironment applyEnv = EvaluationEnvironment(EvaluationParameters());
      copy.setElement(*intervalThis);
      PNT::TPassPointer<VirtualDisjunction, PPAbstractElement> conversion = copy->changeSign(applyEnv);
      if (conversion.isValid()) {
         assume(conversion->query(operation, env));
         return true;
      };
      intervalThis = &*copy;
   };

   assume(intervalThis->ppveMin->query(operation, env));
   assume(intervalThis->ppveMax->query(operation, env));
   ((QueryOperation::CompareSpecialEnvironment&) env).unify();
   return true;
}

bool
IntegerInterval::queryBitDomain(const VirtualElement& thisElement, const VirtualQueryOperation& operation, VirtualQueryOperation::Environment& aenv) {
   QueryOperation::BitDomainEnvironment& env = (QueryOperation::BitDomainEnvironment&) aenv;
   env.fill();
   return true;
}

bool
IntegerInterval::querySimplificationAsConstantDisjunction(const VirtualElement& thisElement, const VirtualQueryOperation&, VirtualQueryOperation::Environment& aenv) {
   AssumeCondition(dynamic_cast<const Details::IntOperationElement::SimplificationEnvironment*>(&aenv))
   AssumeCondition(dynamic_cast<const IntegerInterval*>(&thisElement))
   IntegerInterval& intervalThis = const_cast<IntegerInterval&>((const IntegerInterval&) thisElement);
   Details::IntOperationElement::SimplificationEnvironment& env = (Details::IntOperationElement::SimplificationEnvironment&) aenv;

   PPVirtualElement nbElements(intervalThis.ppveMax, PNT::Pointer::Duplicate());
   nbElements = Methods::applyAssign(nbElements, intervalThis.querySMinusAssignOperation(), *intervalThis.ppveMin, EvaluationParameters());
   if (Methods::apply(*nbElements, intervalThis.querySRightShiftOperation(),
         *Methods::newIntForShift(thisElement, 6), EvaluationParameters())->queryZeroResult().isFalse()) {
      PNT::TPassPointer<VirtualDisjunction, PPAbstractElement> disjunction = Methods::newDisjunction(thisElement);
      PPVirtualElement element(intervalThis.ppveMin, PNT::Pointer::Duplicate());
      VirtualOperation inc = intervalThis.querySNextAssignOperation();
      VirtualOperation compareLess = intervalThis.querySCompareLessOperation();
      VirtualOperation compareLessOrEqual = intervalThis.querySCompareLessOrEqualOperation();
      while (Methods::applyBoolean(*element, compareLess, *intervalThis.ppveMax).mayBeTrue()) {
         disjunction->addMay(element->createSCopy());
         element = Methods::applyAssign(element, inc, EPExactStopErrorStates);
         if (!element.isValid())
            break;
      };
      if (element.isValid()
            && Methods::applyBoolean(*element, compareLessOrEqual, *intervalThis.ppveMax).mayBeTrue())
         disjunction->addMay(element->createSCopy());
      env.presult() = disjunction;
   }
   else
      ((SimplificationEnvironment&) env).setFail();
   return true;
}

bool
IntegerInterval::querySimplification(const VirtualElement& thisElement, const VirtualQueryOperation&, VirtualQueryOperation::Environment& aenv) {
   AssumeCondition(dynamic_cast<const IntegerInterval*>(&thisElement)
         && dynamic_cast<const SimplificationEnvironment*>(&aenv))
   IntegerInterval& intervalThis = const_cast<IntegerInterval&>((const IntegerInterval&) thisElement);
/* DomainEnvironment domainEnv = Methods::query(*intervalThis.ppveMin, Methods::queryInfValueQuery(*intervalThis.ppveMin));
   if (&domainEnv.getResult() != intervalThis.ppveMin.key())
      intervalThis.ppveMin = domainEnv.result();
   domainEnv = Methods::query(*intervalThis.ppveMax, Methods::querySupValueQuery(*intervalThis.ppveMax));
   if (&domainEnv.getResult() != intervalThis.ppveMax.key())
      intervalThis.ppveMax = domainEnv.result();
*/

   Methods::BooleanResult
      compare = Methods::applyBoolean(*intervalThis.ppveMin, intervalThis.querySCompareLessOrEqualOperation(), *intervalThis.ppveMax);
   if (compare.isValid()) {
      SimplificationEnvironment& env = (SimplificationEnvironment&) aenv;
      if (compare) { // intervalThis.ppveMin <= intervalThis.ppveMax
         compare = Methods::applyBoolean(*intervalThis.ppveMin, intervalThis.querySCompareEqualOperation(), *intervalThis.ppveMax);
         if (compare.isValid() && compare)
            env.presult() = intervalThis.ppveMin; // intervalThis.ppveMin = intervalThis.ppveMax
      }
      else // intervalThis.ppveMin > intervalThis.ppveMax
         env.setEmpty();
   };
   return true;
}

bool
IntegerInterval::queryDispatchOnMin(const VirtualElement& thisElement, const VirtualQueryOperation& operation,
        VirtualQueryOperation::Environment& env) {
   AssumeCondition(dynamic_cast<const IntegerInterval*>(&thisElement))
   const IntegerInterval& intervalThis = (const IntegerInterval&) thisElement;
   return intervalThis.ppveMin->query(operation, env);
}

bool
IntegerInterval::queryCopy(const VirtualElement& sourceElement, const VirtualQueryOperation& operation,
        VirtualQueryOperation::Environment& aenv) {
   AssumeCondition(dynamic_cast<const IntegerInterval*>(&sourceElement) && dynamic_cast<const VirtualQueryOperation::DuplicationEnvironment*>(&aenv))
   VirtualQueryOperation::DuplicationEnvironment& env = (VirtualQueryOperation::DuplicationEnvironment&) aenv;
   IntegerInterval* result = ((const IntegerInterval&) sourceElement).createSCopy();
   env.presult().absorbElement(result);
   result->ppveMin->query(VirtualQueryOperation().setSpecialize(), aenv);
   result->ppveMax->query(VirtualQueryOperation().setSpecialize(), aenv);
   return true;
}

bool
IntegerInterval::querySpecialize(const VirtualElement& sourceElement, const VirtualQueryOperation& operation,
        VirtualQueryOperation::Environment& aenv) {
   AssumeCondition(dynamic_cast<const IntegerInterval*>(&sourceElement))
   const IntegerInterval& source = (const IntegerInterval&) sourceElement;
   source.ppveMin->query(operation, aenv);
   source.ppveMax->query(operation, aenv);
   return true;
}

/* intersection methods */

bool
IntegerInterval::intersectWithToConstant(VirtualElement& sourceElement, EvaluationEnvironment& env) const {
   if (Methods::applyBoolean(*ppveMin, querySCompareLessOrEqualOperation(), sourceElement)
         && Methods::applyBoolean(*ppveMax, querySCompareGreaterOrEqualOperation(), sourceElement)) {
      env.mergeVerdictExact();
      return true;
   };
   if (Methods::applyBoolean(*ppveMin, querySCompareGreaterOrEqualOperation(), sourceElement)
         || Methods::applyBoolean(*ppveMax, querySCompareLessOperation(), sourceElement)) {
      env.setEmpty();
      return true;
   };

   env.mergeVerdictDegradate();
   if (env.getInformationKind().isSure()) {
      env.setEmpty();
      return true;
   };
   if (env.getInformationKind().isMay())
      return true; // stable
   env.setUnstable();
   PNT::TPassPointer<VirtualDisjunction, PPAbstractElement> result = Methods::newDisjunction(sourceElement);
   result->addMay(PPVirtualElement(sourceElement));
   env.presult() = result;
   return true;
}

bool
IntegerInterval::intersectWithConstant(const VirtualElement& sourceElement, EvaluationEnvironment& env) {
   if (Methods::applyBoolean(*ppveMin, querySCompareLessOrEqualOperation(), sourceElement)
         && Methods::applyBoolean(*ppveMax, querySCompareGreaterOrEqualOperation(), sourceElement)) {
      env.mergeVerdictExact();
      env.storeResult(sourceElement.createSCopy());
      return true;
   };
   if (Methods::applyBoolean(*ppveMin, querySCompareGreaterOperation(), sourceElement)
         || Methods::applyBoolean(*ppveMax, querySCompareLessOperation(), sourceElement)) {
      env.setEmpty();
      return true;
   };

   env.mergeVerdictDegradate();
   if (env.getInformationKind().isSure()) {
      env.setEmpty();
      return true;
   };
   if (env.getInformationKind().isMay())
      return true; // stable
   env.setUnstable();
   PNT::TPassPointer<VirtualDisjunction, PPAbstractElement> result = Methods::newDisjunction(sourceElement);
   result->addMay(PPVirtualElement(sourceElement));
   env.presult() = result;
   return true;
}

bool
IntegerInterval::intersectWithGeneric(const VirtualElement& sourceElement, EvaluationEnvironment& env) {
   // AssumeCondition(false)
   env.mergeVerdictDegradate();
   return true;
}

bool
IntegerInterval::intersectWithInterval(const VirtualElement& sourceElement, EvaluationEnvironment& env) {
   env.mergeVerdictExact();
   const IntegerInterval& source = (const IntegerInterval&) sourceElement;
   if (queryUnsignedField() != source.queryUnsignedField()) {
      PNT::TPassPointer<VirtualDisjunction, PPAbstractElement> copy = changeSign(env);
      if (copy.isValid()) {
         assume(copy->intersectWith(sourceElement, env));
         if (!env.hasResult())
            env.presult() = copy;
         return true;
      };
   };
   
   {  TransmitEnvironment minEnv(env);
      minEnv.setFirstArgument(*source.ppveMin);
      assume(ppveMin->apply(querySMaxAssignOperation(), minEnv));
      if (minEnv.hasResult()) {
         env.setUnstable();
         ppveMin = minEnv.presult();
      };

      TransmitEnvironment maxEnv(env);
      maxEnv.setFirstArgument(*source.ppveMax);
      assume(ppveMax->apply(querySMinAssignOperation(), maxEnv));
      if (maxEnv.hasResult()) {
         env.setUnstable();
         ppveMax = maxEnv.presult();
      };
   }

   EvaluationParameters compareParameters = EvaluationParameters(env).stopErrorStates();
   QueryOperation::CompareSpecialEnvironment
     minMaxCompare = compareScalars(*ppveMin, *ppveMax, compareParameters, hasUnsignedField());

   if (minMaxCompare.isGreaterZero())
     env.setEmpty();
   else if (minMaxCompare.isZero())
     env.presult() = ppveMin;

   return true;
}

/* merge fusions */

bool
IntegerInterval::mergeWithToConstant(VirtualElement& sourceElement, EvaluationEnvironment& env) const {
   EvaluationEnvironment::LatticeCreation latticeCreation = env.getLatticeCreation();
   if ((latticeCreation.isTop() || latticeCreation.isShareTop())) {
      EvaluationEnvironment containEnv = EvaluationEnvironment(EvaluationEnvironment::Init(env));
      bool result = containConstant(sourceElement, containEnv);
      AssumeCondition(result)
      if (containEnv.isTotalApplied())
         env.presult().setElement(*this);
      else
         env.presult() = Details::IntOperationElement::Methods::newTop(*this);
      return true;
   };
   PNT::TPassPointer<IntegerInterval, PPAbstractElement> thisInterval(*this);
   assume(thisInterval->mergeWithConstant(sourceElement, env));
   env.setUnstable();
   if (!env.hasResult())
      env.presult() = thisInterval;
   return true;
}

bool
IntegerInterval::containToConstant(const VirtualElement& sourceElement, EvaluationEnvironment& env) const
{  env.setPartialApplied();
   return true;
}

bool
IntegerInterval::mergeWithConstant(const VirtualElement& sourceElement, EvaluationEnvironment& env) {
   // [a,b] U x
   EvaluationParameters compareParameters = EvaluationParameters(env).stopErrorStates();
   // Traiter les débordements
   QueryOperation::CompareSpecialEnvironment
      minCompare = compareScalars(sourceElement, *ppveMin, compareParameters, hasUnsignedField()),
      maxCompare = compareScalars(sourceElement, *ppveMax, compareParameters, hasUnsignedField());

   env.mergeVerdictExact();
   if (minCompare.isGreaterOrEqualZero() && maxCompare.isLessOrEqualZero())
      return true;
   EvaluationEnvironment::LatticeCreation latticeCreation = env.getLatticeCreation();
   if ((latticeCreation.isTop() || latticeCreation.isShareTop())) {
      env.presult() = Details::IntOperationElement::Methods::newTop(*this);
      return true;
   };
   env.setUnstable();
   if (minCompare.isMinusOne()) {
      ppveMin.setElement(sourceElement);
      return true;
   };
   if (maxCompare.isOne()) {
      ppveMax.setElement(sourceElement);
      return true;
   };

   if ((env.getInformationKind().isSure())
         || minCompare.mayBeLessMinusOne() || maxCompare.mayBeGreaterOne()) {
      PNT::TPassPointer<VirtualDisjunction, PPAbstractElement> result = Methods::newDisjunction(sourceElement);
      Init init;
      init.setInterval(ppveMin, ppveMax).setUnsigned(queryUnsignedField());
      result->add(Methods::newInterval(sourceElement, init), env.getInformationKind());
      result->add(sourceElement, env.getInformationKind());
      env.presult() = result;
      return true;
   };

   auto minAssignOperation = querySMinAssignOperation();
   auto maxAssignOperation = querySMaxAssignOperation();
   ppveMin = Methods::applyAssign(ppveMin, minAssignOperation, sourceElement, env);
   ppveMax = Methods::applyAssign(ppveMax, maxAssignOperation, sourceElement, env);
   return true;
}

bool
IntegerInterval::containConstant(const VirtualElement& sourceElement, EvaluationEnvironment& env) const {
   // [a,b] U x
   EvaluationParameters compareParameters = EvaluationParameters(env).stopErrorStates();
   // Traiter les débordements
   QueryOperation::CompareSpecialEnvironment
      minCompare = compareScalars(sourceElement, *ppveMin, compareParameters, hasUnsignedField()),
      maxCompare = compareScalars(*ppveMax, sourceElement, compareParameters, hasUnsignedField());

   env.mergeVerdictExact();
   if (!(minCompare.isGreaterOrEqualZero() && maxCompare.isGreaterOrEqualZero()))
      env.setNoneApplied();
   return true;
}

bool
IntegerInterval::mergeWithInterval(const VirtualElement& sourceElement, EvaluationEnvironment& env) {
   // [a,b] U [c,d]
   env.mergeVerdictExact();
   const IntegerInterval& source = (const IntegerInterval&) sourceElement;
   if (queryUnsignedField() != source.queryUnsignedField()) {
      PNT::TPassPointer<VirtualDisjunction, PPAbstractElement> copy = changeSign(env);
      if (copy.isValid()) {
         assume(copy->mergeWith(sourceElement, env));
         if (!env.hasResult())
            env.presult() = copy;
         return true;
      };
   };

   EvaluationParameters compareParameters = EvaluationParameters(env).stopErrorStates();
   QueryOperation::CompareSpecialEnvironment
      adCompare = compareScalars(*ppveMin, *source.ppveMax, compareParameters, hasUnsignedField()),
      bcCompare = compareScalars(*ppveMax, *source.ppveMin, compareParameters, hasUnsignedField());

   EvaluationEnvironment::LatticeCreation latticeCreation = env.getLatticeCreation();
   EvaluationEnvironment containEnv = EvaluationEnvironment(EvaluationEnvironment::Init(env));
   if ((latticeCreation.isTop() || latticeCreation.isShareTop()) && containInterval(sourceElement, containEnv) && !containEnv.isTotalApplied()) {
      env.presult() = Details::IntOperationElement::Methods::newTop(*this);
      return true;
   };
      
   if (bcCompare.mayBeLessMinusOne() || adCompare.mayBeGreaterOne()) { // b < c-1 or a > d+1
      PNT::TPassPointer<VirtualDisjunction, PPAbstractElement> result = Methods::newDisjunction(*ppveMin);
      Init init;
      init.setInterval(PPVirtualElement(source.ppveMin, PNT::Pointer::Duplicate()), PPVirtualElement(source.ppveMax, PNT::Pointer::Duplicate())).setUnsigned(source.queryUnsignedField());
      result->add(Methods::newInterval(sourceElement, init), env.getInformationKind());
      Init init2;
      init2.setInterval(ppveMin, ppveMax).setUnsigned(queryUnsignedField());
      result->add(Methods::newInterval(sourceElement, init2), env.getInformationKind());
      env.setUnstable();
      env.presult() = result;
      return true;
   };

   if (queryUnsignedField() != source.queryUnsignedField())
      {  AssumeUnimplemented }
   TransmitEnvironment minEnv(env);
   minEnv.setFirstArgument(*source.ppveMin);
   assume(ppveMin->apply(querySMinAssignOperation(), minEnv));
   if (!minEnv.hasResult())
      env.setUnstable();
   else
      ppveMin = minEnv.presult();

   TransmitEnvironment maxEnv(env);
   maxEnv.setFirstArgument(*source.ppveMax);
   assume(ppveMax->apply(querySMaxAssignOperation(), maxEnv));

   if (!maxEnv.hasResult())
      env.setUnstable();
   else
      ppveMax = maxEnv.presult();

   return true;
}

bool
IntegerInterval::containInterval(const VirtualElement& sourceElement, EvaluationEnvironment& env) const {
   // [a,b] contain [c,d]
   AssumeCondition(dynamic_cast<const IntegerInterval*>(&sourceElement))
   env.mergeVerdictExact();
   const IntegerInterval& source = (const IntegerInterval&) sourceElement;
   PNT::TPassPointer<IntegerInterval, PPAbstractElement> copy;
   const IntegerInterval* thisInterval = this;
   if (queryUnsignedField() != source.queryUnsignedField()) {
      copy.setElement(*this);
      PNT::TPassPointer<VirtualDisjunction, PPAbstractElement> conversion = copy->changeSign(env);
      if (conversion.isValid())
         return conversion->contain(sourceElement, env);
      thisInterval = &*copy;
   };
   EvaluationParameters compareParameters = EvaluationParameters(env).stopErrorStates();
   QueryOperation::CompareSpecialEnvironment
      acCompare = compareScalars(*thisInterval->ppveMin, *source.ppveMin, compareParameters, thisInterval->hasUnsignedField()),
      bdCompare = compareScalars(*thisInterval->ppveMax, *source.ppveMax, compareParameters, thisInterval->hasUnsignedField());
   if (!(acCompare.isLessOrEqualZero() && bdCompare.isGreaterOrEqualZero())) // a <= c and b >= d
      env.setNoneApplied();
   return true;
}

/* applyTo methods */

#define DefineImplementationApplyToCompare(TypeOperation, TypeOppositeOperation)                   \
bool                                                                                               \
IntegerInterval::applyTo##TypeOperation##Constant(const VirtualOperation& operation,               \
      VirtualElement& source, EvaluationEnvironment& env) const                                    \
{  TransmitEnvironment applyEnv(env);                                                              \
   bool result = const_cast<IntegerInterval*>(this)                                                \
      ->apply##TypeOppositeOperation##Constant(operation, source, applyEnv);                       \
   env.presult() = applyEnv.presult();                                                             \
   return result;                                                                                  \
}

/* compare methods applyTo */

DefineImplementationApplyToCompare(CompareLess, CompareGreater)
DefineImplementationApplyToCompare(CompareLessOrEqual, CompareGreaterOrEqual)
DefineImplementationApplyToCompare(CompareEqual, CompareEqual)
DefineImplementationApplyToCompare(CompareDifferent, CompareDifferent)
DefineImplementationApplyToCompare(CompareGreaterOrEqual, CompareLessOrEqual)
DefineImplementationApplyToCompare(CompareGreater, CompareLess)

#undef DefineImplementationApplyToCompare

#define DefineImplementationApplyToSymmetric(TypeOperation)                                          \
bool                                                                                               \
IntegerInterval::applyTo##TypeOperation##Constant(const VirtualOperation& operation,               \
      VirtualElement& source, EvaluationEnvironment& env) const {                                  \
   IntegerInterval* copy = nullptr;                                                                \
   PPVirtualElement result(copy = createSCopy(), PNT::Pointer::Init());                            \
   assume(copy->apply##TypeOperation##Constant(operation, source, env));                           \
   if (!env.hasResult())                                                                           \
      env.presult() = result;                                                                      \
   return true;                                                                                    \
}

bool
IntegerInterval::applyToConcatConstant(const VirtualOperation& operation,
      VirtualElement& source, EvaluationEnvironment& env) const {
   PPVirtualElement topThis = Methods::newTop(*this);
   TransmitEnvironment applyEnv(env);
   applyEnv.setFirstArgument(*topThis);
   topThis->applyTo(operation, source, applyEnv);
   if (applyEnv.hasResult())
      env.presult() = applyEnv.presult();
   env.mergeVerdictDegradate();
   return true;
}

bool
IntegerInterval::applyToBitSetConstant(const VirtualOperation& operation,
      VirtualElement& source, EvaluationEnvironment& env) const {
   AssumeCondition(dynamic_cast<const Scalar::MultiBit::BitSetOperation*>(&operation))
   int lowBit = ((const Scalar::MultiBit::BitSetOperation&) operation).getLowBit();
   int highBit = ((const Scalar::MultiBit::BitSetOperation&) operation).getHighBit();
   if (lowBit == 0) {
      IntegerInterval* result = createSCopy();
      env.presult().absorbElement(result);
      if (highBit < source.getSizeInBits()-1) {
         TransmitEnvironment localEnv(env);
         source.apply(Scalar::MultiBit::ReduceOperation().setLowBit(highBit+1)
               .setHighBit(source.getSizeInBits()-1), localEnv);
         if (localEnv.hasResult()) {
            PPVirtualElement extensionMin = localEnv.presult();
            PPVirtualElement extensionMax(*extensionMin);
            localEnv.setFirstArgument(*result->ppveMin);
            extensionMin->apply(Scalar::MultiBit::ConcatOperation(), localEnv);
            result->ppveMin = localEnv.hasResult() ? localEnv.presult() : extensionMin;
            localEnv.setFirstArgument(*result->ppveMax);
            extensionMax->apply(Scalar::MultiBit::ConcatOperation(), localEnv);
            result->ppveMax = localEnv.hasResult() ? localEnv.presult() : extensionMax;
         }
         else {
            PPVirtualElement extensionMax(source);
            localEnv.setFirstArgument(*result->ppveMin);
            source.apply(Scalar::MultiBit::ConcatOperation(), localEnv);
            if (localEnv.hasResult())
               result->ppveMin = localEnv.presult();
            else
               result->ppveMin.setElement(source);
            localEnv.setFirstArgument(*result->ppveMax);
            extensionMax->apply(Scalar::MultiBit::ConcatOperation(), localEnv);
            result->ppveMax = localEnv.hasResult() ? localEnv.presult() : extensionMax;
         };
      };
   }
   else if (!env.getLatticeCreation().mayBeFormal()) {
      env.presult() = Methods::newTop(source);
      env.mergeVerdictDegradate();
   }
   else {
      if (lowBit > 0) {
         PPVirtualElement firstArgument(source);
         firstArgument = Methods::applyAssign(firstArgument, Scalar::MultiBit::ReduceOperation()
            .setLowBit(0).setHighBit(lowBit-1), env);
         PPVirtualElement secondArgument(*this);
         env.presult().absorbElement(new MultiBit::Approximate::ConcatOperationElement(
               MultiBit::Approximate::ConcatOperationElement::Init().setFstArg(secondArgument)
                  .setSndArg(firstArgument)));
      }
      else
         env.presult().setElement(*this);
      if (highBit < source.getSizeInBits()-1) {
         PPVirtualElement secondArgument(source);
         secondArgument = Methods::applyAssign(secondArgument, Scalar::MultiBit::ReduceOperation()
            .setLowBit(highBit+1).setHighBit(source.getSizeInBits()-1), env);
         PPVirtualElement firstArgument = env.presult();
         env.presult().absorbElement(new MultiBit::Approximate::ConcatOperationElement(
            MultiBit::Approximate::ConcatOperationElement::Init().setFstArg(secondArgument)
               .setSndArg(firstArgument)));
      };
   };

   return true;
}

bool
IntegerInterval::applyToPlusAssignConstant(const VirtualOperation& operation,
      VirtualElement& source, EvaluationEnvironment& env) const {
   IntegerInterval* copy = nullptr;
   PPVirtualElement result(copy = createSCopy(), PNT::Pointer::Init());
   assume(copy->applyPlusAssignConstant(operation, source, env));
   if (!env.hasResult())
      env.presult() = result;
   return true;
}

DefineImplementationApplyToSymmetric(MinAssign)
DefineImplementationApplyToSymmetric(MaxAssign)
// DefineImplementationApplyToSymmetric(PlusAssign)

#undef DefineImplementationApplyToSymmetric

bool
IntegerInterval::applyToMinusAssignConstant(const VirtualOperation&, VirtualElement& source,
      EvaluationEnvironment& env) const {
   // x - [a,b] =   [x-max(b, Maxint+x+1), x-max(a, Maxint+x)] with overflow propagation (not the internal operations)
   //             U [x-b, x-a] without any overflow propagation
   //             U [x-min(b, Minint+x), x-min(a, Minint+x-1)] with overflow propagation (not the internal operations)
   TransmitEnvironment minEnv(env, EPExactStopErrorStates), maxEnv(env, EPExactStopErrorStates);
   PPVirtualElement newMin, newMax;
   // compute [x-b, x-a] without any overflow propagation
   newMin = Methods::apply(source, querySMinusOperation(), *ppveMax, minEnv);
   if (minEnv.isEmpty()) {
      minEnv.clearEmpty();
      if (minEnv.isOnlyPositiveOverflow()) {
         if (env.doesStopErrorStates()) {
            env.setEmpty();
            return true;
         };
      }
      else {
         AssumeCondition(minEnv.isNegativeOverflow())
         // newMin = newSMinDomain();
      };
   };

   newMax = Methods::apply(source, querySMinusOperation(), *ppveMin, maxEnv);
   if (maxEnv.isEmpty()) {
      maxEnv.clearEmpty();
      if (maxEnv.isOnlyNegativeOverflow()) {
         if (env.doesStopErrorStates()) {
            env.setEmpty();
            return true;
         };
      }
      else {
         AssumeCondition(maxEnv.isPositiveOverflow())
         // newMax = newSMaxDomain();
      };
   };

   if (!newMin.isValid() || !newMax.isValid()) { // x-b > Maxint or x-a < Minint
      minEnv.setPropagateError();
      maxEnv.setPropagateError();
      Init init;
      EvaluationParameters exactPropagateErrorStates(EPExactPropagateErrorStates);
      exactPropagateErrorStates.setApproximatePartFrom(env);
      init.setInterval(Methods::apply(source, querySMinusOperation(), *ppveMax, exactPropagateErrorStates),
            Methods::apply(source, querySMinusOperation(), *ppveMin, exactPropagateErrorStates))
         .setUnsigned(queryUnsignedField());
      env.presult() = Methods::newInterval(source, init);
      return true;
   };

   // most frequent case: no overflow
   env.presult() = Methods::newInterval(source, Init().setInterval(newMin, newMax).setUnsigned(queryUnsignedField()));
   if (env.doesStopErrorStates() || (!minEnv.hasEvaluationError() && !maxEnv.hasEvaluationError()))
      return true;

   // overflow
   if (minEnv.isNegativeOverflow()) {
      // [x-max(b, Maxint+x+1), x-max(a, Maxint+x)] with overflow propagation (not the internal operations)
      PNT::TPassPointer<IntegerInterval, PPAbstractElement> lowPartition;
      EvaluationParameters exactStopErrorStates(EPExactStopErrorStates);
      exactStopErrorStates.setApproximatePartFrom(env);
      PPVirtualElement bound = Methods::applyAssign(
         newSMaxDomain(), querySPlusAssignOperation(), source, exactStopErrorStates);
      EvaluationParameters exactPropagateErrorStates(EPExactPropagateErrorStates);
      exactPropagateErrorStates.setApproximatePartFrom(env);
      PPVirtualElement lowMax = Methods::apply(*ppveMin, querySMaxOperation(), *bound, exactPropagateErrorStates);
      bound = Methods::applyAssign(bound, querySNextAssignOperation(), exactPropagateErrorStates);
      if (!bound.isValid()) {
         env.presult() = Methods::newTop(source);
         return true;
      };
      PPVirtualElement lowMin = Methods::apply(*ppveMax, querySMaxOperation(), *bound, exactPropagateErrorStates);
      lowMin = Methods::apply(source, querySMinusOperation(), *lowMin, exactStopErrorStates);
      lowMax = Methods::apply(source, querySMinusOperation(), *lowMax, exactStopErrorStates);
      lowPartition = Methods::newInterval(source, Init().setInterval(lowMin, lowMax).setUnsigned(queryUnsignedField()));
      TransmitEnvironment mergeEnv(env);
      env.getResult().mergeWith(*lowPartition, mergeEnv);
      if (mergeEnv.hasResult())
         env.presult() = mergeEnv.presult();
   };
   if (maxEnv.isPositiveOverflow()) {
      // [x-min(b, Minint+x), x-min(a, Minint+x-1)] with overflow propagation (not the internal operations)
      PNT::TPassPointer<IntegerInterval, PPAbstractElement> highPartition;
      EvaluationParameters exactPropagateErrorStates(EPExactPropagateErrorStates);
      exactPropagateErrorStates.setApproximatePartFrom(env);
      EvaluationParameters exactStopErrorStates(EPExactStopErrorStates);
      exactStopErrorStates.setApproximatePartFrom(env);
      PPVirtualElement bound = Methods::applyAssign(
            newSMinDomain(), querySPlusAssignOperation(), source, exactStopErrorStates);
      PPVirtualElement
         highMin = Methods::apply(*ppveMax, querySMaxOperation(), *bound, exactPropagateErrorStates),
         highMax = Methods::apply(*ppveMin, querySMaxOperation(),
            *Methods::applyAssign(bound, querySPrevAssignOperation(), exactPropagateErrorStates), exactPropagateErrorStates);
      highMin = Methods::apply(source, querySMinusOperation(), *highMin, exactStopErrorStates);
      highMax = Methods::apply(source, querySMinusOperation(), *highMax, exactStopErrorStates);
      highPartition = Methods::newInterval(source, Init().setInterval(highMin, highMax).setUnsigned(queryUnsignedField()));
      TransmitEnvironment mergeEnv(env);
      env.getResult().mergeWith(*highPartition, mergeEnv);
      if (mergeEnv.hasResult())
         env.presult() = mergeEnv.presult();
   };

   return true;
}

/* numeric applyTo methods */

bool
IntegerInterval::applyToTimesAssignConstant(const VirtualOperation& operation,
      VirtualElement& sourceElement, EvaluationEnvironment& env) const {
   const IntegerInterval* intervalThis = this;
   PNT::TPassPointer<IntegerInterval, PPAbstractElement> copy;
   if ((bool) queryUnsignedField() != isUnsignedTimesOperation(operation)) {
      copy.setElement(*this);
      PNT::TPassPointer<VirtualDisjunction, PPAbstractElement> conversion = copy->changeSign(env);
      if (conversion.isValid()) {
         TransmitEnvironment applyEnv(env);
         applyEnv.setFirstArgument(*conversion);
         assume(sourceElement.apply(operation, applyEnv));
         return true;
      };
      intervalThis = &*copy;
   };
   QueryOperation::CompareSpecialEnvironment compareEnv
      = intervalThis->querySCompareSpecial(sourceElement);
   if (compareEnv.isZero())
      return true;
   else if (compareEnv.isOne()) {
      if (copy.isValid())
         env.presult() = copy;
      else
         env.storeResult(createSCopy());
      return true;
   }
   else if (compareEnv.isMinusOne()) {
      PPVirtualElement result(*this);
      ((IntegerInterval&) *result).applyOppositeAssign(
         Methods::queryOppositeAssign(sourceElement), env);
      if (!env.hasResult())
         env.presult() = result;
   };

   // use the commutativity of the multiplication
   PPIntegerInterval result(*this);
   TransmitEnvironment transmitEnv(env);
   assume(result->applyTimesAssignConstant(operation, sourceElement, transmitEnv));
   env.presult() = transmitEnv.hasResult() ? transmitEnv.presult() : result;
   return true;
}

PNT::TPassPointer<VirtualDisjunction, PPAbstractElement>
IntegerInterval::applyToSureCase(const QueryOperation& operation, VirtualElement& sourceElement, EvaluationEnvironment& env) const {
   PNT::TPassPointer<VirtualDisjunction, PPAbstractElement> result;
   if (!env.getInformationKind().isMay()) {
      // x op [a,b] = x op b, x op a, x op (a+b)/2 as sure
      result = Methods::newDisjunction(sourceElement);
      EvaluationParameters sure = EvaluationParameters(env).setSure();
      VirtualQueryOperation::OperationEnvironment operationEnv;
      assume(sourceElement.query(operation, operationEnv));
      bool isEmpty = true;
      PPVirtualElement min = Methods::apply(sourceElement, operationEnv.result(), *ppveMin, env, sure);
      if (!env.isEmpty()) {
         isEmpty = false;
         result->addSure(min);
      }
      else
         env.clearEmpty();
      PPVirtualElement max = Methods::apply(sourceElement, operationEnv.result(), *ppveMax, env, sure);
      if (!env.isEmpty()) {
         isEmpty = false;
         result->addSure(max);
      }
      else
         env.clearEmpty();
      PPVirtualElement middle = Methods::applyAssign(
         Methods::apply(*ppveMin, querySPlusOperation(), *ppveMax, sure),
         Methods::queryRightShiftSignedAssign(*ppveMin),
         *Methods::newIntForShift(sourceElement, 1), sure);
      middle = Methods::apply(sourceElement, operationEnv.result(), *middle, env, sure);
      if (!env.isEmpty()) {
         isEmpty = false;
         result->addSure(middle);
      }
      else
         env.clearEmpty();
      if (isEmpty)
         env.setEmpty();
   };
   return result;
}

PNT::TPassPointer<VirtualDisjunction, PPAbstractElement>
IntegerInterval::applySureCase(const QueryOperation& operation, const VirtualElement& sourceElement, EvaluationEnvironment& env) {
   PNT::TPassPointer<VirtualDisjunction, PPAbstractElement> result;
   if (!env.getInformationKind().isMay()) {
      // [a,b] op x = a op x, b op x, (a+b)/2 op x as sure
      VirtualQueryOperation::OperationEnvironment operationEnv;
      assume(sourceElement.query(operation, operationEnv));
      result = Methods::newDisjunction(sourceElement);
      EvaluationParameters sure = EvaluationParameters(env).setSure();
      bool isEmpty = false;
      TransmitEnvironment copyEnv(env, sure);
      copyEnv.setFirstArgument(sourceElement);

      assume(ppveMin->apply(operationEnv.result(), copyEnv));
      if (!copyEnv.isEmpty()) {
         isEmpty = false;
         result->addSure(copyEnv.presult());
      }
      else
         copyEnv.clearEmpty();
      env.mergeSureError(copyEnv);
      copyEnv.clear();
      copyEnv.setFirstArgument(sourceElement);

      assume(ppveMax->apply(operationEnv.result(), copyEnv));
      if (!copyEnv.isEmpty()) {
         isEmpty = false;
         result->addSure(copyEnv.presult());
      }
      else
         copyEnv.clearEmpty();
      env.mergeSureError(copyEnv);
      copyEnv.clear();
      copyEnv.setFirstArgument(sourceElement);

      assume(Methods::applyAssign(
            Methods::apply(*ppveMin, querySPlusOperation(), *ppveMax, sure),
            Methods::queryRightShiftSignedAssign(*ppveMin),
            *Methods::newIntForShift(sourceElement, 1), sure)
         ->apply(operationEnv.result(), copyEnv));
      if (!copyEnv.isEmpty()) {
         isEmpty = false;
         result->addSure(copyEnv.presult());
      }
      else
         copyEnv.clearEmpty();
      if (isEmpty)
         copyEnv.setEmpty();
      env.mergeSureError(copyEnv);
   };
   return result;
}

PNT::TPassPointer<VirtualDisjunction, PPAbstractElement>
IntegerInterval::applySureCaseInterval(const QueryOperation& operation, const IntegerInterval& source, EvaluationEnvironment& env) {
   PNT::TPassPointer<VirtualDisjunction, PPAbstractElement> result;
   if (!env.getInformationKind().isMay()) {
      // [a,b] op [c,d] = a op c, b op c, (a+b)/2 op c,
      //                = a op d, b op d, (a+b)/2 op d,
      //                = a op (c+d)/2, b op (c+d)/2, (a+b)/2 op (c+d)/2, as sure
      EvaluationParameters sure = EvaluationParameters(env).setSure();
      EvaluationParameters middleParameters(sure);
      middleParameters.stopErrorStates();
      PPVirtualElement sum =  Methods::apply(*ppveMin, querySPlusOperation(), *ppveMax, middleParameters);
      PPVirtualElement middle;
      if (sum.isValid())
         middle = Methods::applyAssign(sum, Methods::queryRightShiftSignedAssign(*ppveMin),
               *Methods::newIntForShift(*ppveMin, 1), middleParameters);
      else {
         PPVirtualElement diff = Methods::apply(*ppveMax, querySMinusOperation(), *ppveMin, middleParameters);
         diff = Methods::applyAssign(diff, Methods::queryRightShiftSignedAssign(*ppveMin),
               *Methods::newIntForShift(*ppveMin, 1), middleParameters);
         middle = Methods::applyAssign(diff, querySPlusAssignOperation(), *ppveMin, middleParameters);
      }

      sum = Methods::apply(*source.ppveMin, querySPlusOperation(), *source.ppveMax, middleParameters);
      PPVirtualElement sourceMiddle;
      if (sum.isValid())
         sourceMiddle = Methods::applyAssign(sum,
            Methods::queryRightShiftSignedAssign(*ppveMin), *Methods::newIntForShift(*ppveMin, 1), middleParameters);
      else {
         PPVirtualElement diff = Methods::apply(*source.ppveMax, querySMinusOperation(), *source.ppveMin, middleParameters);
         diff = Methods::applyAssign(diff, Methods::queryRightShiftSignedAssign(*source.ppveMin),
               *Methods::newIntForShift(*source.ppveMin, 1), middleParameters);
         sourceMiddle = Methods::applyAssign(diff, querySPlusAssignOperation(), *source.ppveMin, middleParameters);
      }

      VirtualQueryOperation::OperationEnvironment operationEnv;
      assume(ppveMin->query(operation, operationEnv));

      result = Methods::newDisjunction(*ppveMin);
      TransmitEnvironment copyEnv(env, sure);

      bool isEmpty = true;
      copyEnv.setFirstArgument(*source.ppveMin);
      assume(ppveMin->apply(operationEnv.result(), copyEnv));
      if (!copyEnv.isEmpty()) {
         isEmpty = false;
         result->addSure(copyEnv.presult());
      }
      else
         copyEnv.clearEmpty();
      copyEnv.merge().clear();
      copyEnv.setFirstArgument(*source.ppveMin);
      assume(ppveMax->apply(operationEnv.result(), copyEnv));
      if (!copyEnv.isEmpty()) {
         isEmpty = false;
         result->addSure(copyEnv.presult());
      }
      else
         copyEnv.clearEmpty();
      copyEnv.merge().clear();
      copyEnv.setFirstArgument(*source.ppveMin);
      assume(middle->apply(operationEnv.result(), copyEnv));
      if (!copyEnv.isEmpty()) {
         isEmpty = false;
         result->addSure(copyEnv.presult());
      }
      else
         copyEnv.clearEmpty();
      copyEnv.merge().clear();

      copyEnv.setFirstArgument(*source.ppveMax);
      assume(ppveMin->apply(operationEnv.result(), copyEnv));
      if (!copyEnv.isEmpty()) {
         isEmpty = false;
         result->addSure(copyEnv.presult());
      }
      else
         copyEnv.clearEmpty();
      copyEnv.merge().clear();
      copyEnv.setFirstArgument(*source.ppveMax);
      assume(ppveMax->apply(operationEnv.result(), copyEnv));
      if (!copyEnv.isEmpty()) {
         isEmpty = false;
         result->addSure(copyEnv.presult());
      }
      else
         copyEnv.clearEmpty();
      copyEnv.merge().clear();
      copyEnv.setFirstArgument(*source.ppveMax);
      assume(middle->apply(operationEnv.result(), copyEnv));
      if (!copyEnv.isEmpty()) {
         isEmpty = false;
         result->addSure(copyEnv.presult());
      }
      else
         copyEnv.clearEmpty();
      copyEnv.merge().clear();

      copyEnv.setFirstArgument(*sourceMiddle);
      assume(ppveMin->apply(operationEnv.result(), copyEnv));
      if (!copyEnv.isEmpty()) {
         isEmpty = false;
         result->addSure(copyEnv.presult());
      }
      else
         copyEnv.clearEmpty();
      copyEnv.merge().clear();
      copyEnv.setFirstArgument(*sourceMiddle);
      assume(ppveMax->apply(operationEnv.result(), copyEnv));
      if (!copyEnv.isEmpty()) {
         isEmpty = false;
         result->addSure(copyEnv.presult());
      }
      else
         copyEnv.clearEmpty();
      copyEnv.merge().clear();
      copyEnv.setFirstArgument(*sourceMiddle);
      assume(middle->apply(operationEnv.result(), copyEnv));
      if (!copyEnv.isEmpty()) {
         isEmpty = false;
         result->addSure(copyEnv.presult());
      }
      else
         copyEnv.clearEmpty();
      copyEnv.merge().clear();
      if (isEmpty)
         env.setEmpty();
   };
   return result;
}

bool
IntegerInterval::applyToDivideAssign(const VirtualOperation& operation,
      VirtualElement& sourceElement, EvaluationEnvironment& env) const {
   PNT::TPassPointer<IntegerInterval, PPAbstractElement> source
      = Methods::newInterval(sourceElement, Init().setInterval(sourceElement, sourceElement).setUnsigned(queryUnsignedField()));
   assume(source->applyDivideAssignInterval(operation, *this, env));
   if (!env.hasResult())
      env.presult() = source;
   return true;
}

bool
IntegerInterval::applyToModuloAssign(const VirtualOperation&, VirtualElement& sourceElement,
      EvaluationEnvironment& env) const {
   QueryOperation::CompareSpecialEnvironment
      minCompare = querySCompareSpecial(*ppveMin),
      maxCompare = querySCompareSpecial(*ppveMax),
      sourceCompare = querySCompareSpecial(sourceElement);

   // handle the errors like division by 0
   if (minCompare.mayBeLessOrEqualZero() && maxCompare.mayBeGreaterOrEqualZero()) {
      if (minCompare.isLessOrEqualZero() && maxCompare.isGreaterOrEqualZero()) {
         env.setDivisionByZero();
         if (!env.getInformationKind().isMay())
            env.setSureDivisionByZero();
         if (minCompare.isZero() && maxCompare.isZero()) {
            env.setEmpty();
            return true;
         };
      }
      else
         env.setDivisionByZero();
   };

   if (sourceCompare.isZero())
      return true; // 0%[a,b] = 0

   PNT::TPassPointer<VirtualDisjunction, PPAbstractElement>
      result = applyToSureCase(QueryOperation().setModulo(), sourceElement, env);
   if (env.getInformationKind().isSure()) {
      env.presult() = result;
      return true;
   };

   EvaluationParameters mayStopErrorStates(EPMayStopErrorStates);
   mayStopErrorStates.setApproximatePartFrom(env);
   PPVirtualElement source, sourcePositive, sourceNegative;
   if (maxCompare.mayBeGreaterZero()) {
      PPVirtualElement resultPositive, resultNegative;
      if (sourceCompare.mayBeGreaterZero()) { // [0,b-1]^[0,sourceElement]
         Init init;
         EvaluationParameters exactPropagateErrorStates(EPExactPropagateErrorStates);
         exactPropagateErrorStates.setApproximatePartFrom(env);
         init.setInterval(Methods::newZero(sourceElement),
               Methods::apply(*ppveMax, querySPrevAssignOperation().setConstWithAssign(),
                  exactPropagateErrorStates))
            .setUnsigned(queryUnsignedField());
         resultPositive = Methods::newInterval(sourceElement, init);
         resultPositive = Methods::constraint(resultPositive, querySCompareLessOrEqualOperation(),
               sourceElement, *Methods::newOne(sourceElement), mayStopErrorStates);
      };
      if (sourceCompare.mayBeLessZero()) { // [-b+1,0]^[sourceElement,0]
         Init init;
         init.setInterval(Methods::applyAssign(
            Methods::apply(*ppveMax, Methods::queryOppositeAssign(sourceElement)
               .setConstWithAssign(), mayStopErrorStates),
            querySNextAssignOperation(), mayStopErrorStates), Methods::newZero(sourceElement)).setSigned();
         resultNegative = Methods::newInterval(sourceElement, init);
         resultNegative = Methods::constraint(resultNegative, querySCompareGreaterOrEqualOperation(),
               sourceElement, *Methods::newOne(sourceElement), mayStopErrorStates);
      };
      // [-b+1,b-1]^[-sourceElement,sourceElement] = union of the two previous results
      sourcePositive = resultPositive.isValid() ? (resultNegative.isValid()
         ? Methods::merge(resultPositive, *resultNegative, mayStopErrorStates)
         : resultPositive) : resultNegative;
   };
   if (minCompare.mayBeLessZero()) {
      PPVirtualElement resultPositive, resultNegative;
      if (sourceCompare.mayBeLessZero()) { // [0,-a-1]^[0,-sourceElement]
         Init init;
         auto nextOperation = querySNextAssignOperation();
         nextOperation.setConstWithAssign();
         init.setInterval(Methods::newZero(sourceElement),
            Methods::apply(*ppveMin, nextOperation, mayStopErrorStates)).setSigned();
         resultPositive = Methods::newInterval(sourceElement, init);
         resultPositive = Methods::constraint(resultPositive, querySCompareGreaterOrEqualOperation(),
               sourceElement, *Methods::newOne(sourceElement), mayStopErrorStates);
         resultPositive = Methods::applyAssign(resultPositive, Methods::queryOppositeAssign(sourceElement), mayStopErrorStates);
      };
      if (sourceCompare.mayBeGreaterZero()) { // [a+1,0]^[-sourceElement,0]
         Init init;
         init.setInterval(Methods::applyAssign(
               Methods::apply(*ppveMin, Methods::queryOppositeAssign(sourceElement)
                     .setConstWithAssign(), mayStopErrorStates),
                  querySPrevAssignOperation(), mayStopErrorStates),
            *Methods::newZero(sourceElement)).setUnsigned(queryUnsignedField());
         resultNegative = Methods::newInterval(sourceElement, init);
         resultNegative = Methods::constraint(resultNegative, querySCompareLessOrEqualOperation(),
               sourceElement, *Methods::newOne(sourceElement), mayStopErrorStates);
         resultPositive = Methods::applyAssign(resultPositive, Methods::queryOppositeAssign(sourceElement),
               mayStopErrorStates);
      };
      // [-b+1,b-1]^[-sourceElement,sourceElement] = union of the two previous results
      sourceNegative = resultPositive.isValid() ? (resultNegative.isValid()
         ? Methods::merge(resultPositive, *resultNegative, mayStopErrorStates)
         : resultPositive) : resultNegative;
   };
   source = sourcePositive.isValid() ? (sourceNegative.isValid()
      ? Methods::merge(sourcePositive, *sourceNegative, mayStopErrorStates)
      : sourcePositive) : sourceNegative;

   if (result.isValid()) {
      result->addMay(source);
      env.presult() = result;
   }
   else
      env.presult() = source;
   return true;
}

/* integer applyTo methods */

bool
IntegerInterval::applyToBitOrAssignConstant(const VirtualOperation& operation, VirtualElement& sourceElement,
      EvaluationEnvironment& env) const {
   QueryOperation::CompareSpecialEnvironment sourceSpecial = Methods::compareSpecialSigned(sourceElement);
   if (sourceSpecial.isZero()) {
      env.storeResult(createSCopy());
      env.mergeVerdictExact();
      return true;
   }
   else if (sourceSpecial.isMinusOne()) {
      env.mergeVerdictExact();
      return true;
   };

   // use the commutativity of the operator |
   IntegerInterval* thisIntervalCopy = createSCopy();
   PPVirtualElement thisCopy(thisIntervalCopy, PNT::Pointer::Init());
   assume(thisIntervalCopy->applyBitOrAssignConstant(operation, sourceElement, env));
   if (!env.hasResult())
      env.presult() = thisCopy;

   return true;
}

bool
IntegerInterval::applyToBitAndAssignConstant(const VirtualOperation& operation, VirtualElement& sourceElement,
      EvaluationEnvironment& env) const {
   QueryOperation::CompareSpecialEnvironment sourceSpecial = Methods::compareSpecialSigned(sourceElement);
   if (sourceSpecial.isZero() || sourceSpecial.isMinusOne()) {
      env.mergeVerdictExact();
      if (sourceSpecial.isMinusOne())
         env.storeResult(createSCopy());
      return true;
   };

   // use the commutativity of the operator &
   IntegerInterval* thisIntervalCopy = createSCopy();
   PPVirtualElement thisCopy(thisIntervalCopy, PNT::Pointer::Init());
   assume(thisIntervalCopy->applyBitAndAssignConstant(operation, sourceElement, env));
   if (!env.hasResult())
      env.presult() = thisCopy;

   return true;
}

bool
IntegerInterval::applyToBitExclusiveOrAssignConstant(const VirtualOperation& operation, VirtualElement& sourceElement,
      EvaluationEnvironment& env) const {
   QueryOperation::CompareSpecialEnvironment sourceSpecial = Methods::compareSpecialSigned(sourceElement);
   if (sourceSpecial.isZero()) {
      env.storeResult(createSCopy());
      env.mergeVerdictExact();
      return true;
   }
   else if (sourceSpecial.isMinusOne())
      return const_cast<IntegerInterval&>(*this).apply(Methods::queryBitNegate(sourceElement), env);

   // use the commutativity of the operator ^
   IntegerInterval* thisIntervalCopy = createSCopy();
   PPVirtualElement thisCopy(thisIntervalCopy, PNT::Pointer::Init());
   assume(thisIntervalCopy->applyBitExclusiveOrAssignConstant(operation, sourceElement, env));
   if (!env.hasResult())
      env.presult() = thisCopy;

   return true;
}

// add the potential overflow errors
bool
IntegerInterval::applyToLeftShiftAssignConstant(const VirtualOperation& operation,
      VirtualElement& sourceElement, EvaluationEnvironment& env) const {
   QueryOperation::CompareSpecialEnvironment sourceSpecial = Methods::compareSpecialSigned(sourceElement);
   if (sourceSpecial.isZero() || sourceSpecial.isMinusOne()) {
      env.mergeVerdictExact();
      return true;
   };

   EvaluationEnvironment::LatticeCreation creation = env.getLatticeCreation();
   if (creation.isFormal() || creation.isDisjunction()) {
      QueryOperation::SimplificationEnvironment simplification;
      query(QueryOperation().setSimplificationAsConstantDisjunction(), simplification);
      if (!simplification.hasFailed() && simplification.hasResult()) {
         TransmitEnvironment applyEnv(env);
         applyEnv.setFirstArgument(*simplification.presult());
         return sourceElement.apply(operation, applyEnv);
      };
   };

   env.mergeVerdictDegradate();
   if (creation.isTop() || creation.isShareTop()) {
      env.presult() = Details::IntOperationElement::Methods::newTop(sourceElement);
      return true;
   };

   PNT::TPassPointer<VirtualDisjunction, PPAbstractElement>
      result = applyToSureCase(QueryOperation().setLeftShift(), sourceElement, env);

   if (!env.getInformationKind().isSure()) {
      PPVirtualElement resultElement;
      QueryOperation::CompareSpecialEnvironment minSpecial = querySCompareSpecial(*ppveMin);
      if (minSpecial.isGreaterOrEqualZero()) {
         PPVirtualElement shift;
         if (!hasUnsignedField())
            shift = Methods::newIntForShift(sourceElement, sourceElement.getSizeInBits()-2);
         else
            shift = Methods::newIntForShift(sourceElement, sourceElement.getSizeInBits()-1);

         EvaluationParameters exactStopErrorStates(EPExactStopErrorStates);
         exactStopErrorStates.setApproximatePartFrom(env);
         PPVirtualElement maxZeroShiftElement = Methods::applyAssign(
            Methods::newOne(sourceElement), Methods::queryLeftShiftAssign(sourceElement),
            *shift, exactStopErrorStates);

         if (sourceSpecial.isGreaterOrEqualZero()
               && Methods::applyBoolean(sourceElement, querySCompareLessOperation(), *maxZeroShiftElement)) {
            EvaluationParameters mayPropagateErrorStates(EPMayPropagateErrorStates);
            mayPropagateErrorStates.setApproximatePartFrom(env);
            EvaluationEnvironment minEnv(mayPropagateErrorStates), maxEnv(mayPropagateErrorStates);
            Init init;
            init.setInterval(Methods::apply(sourceElement, Methods::queryLeftShift(sourceElement), *ppveMin, minEnv),
               Methods::apply(sourceElement, Methods::queryLeftShift(sourceElement), *ppveMax, maxEnv)).setUnsigned(queryUnsignedField());
            PNT::TPassPointer<IntegerInterval, PPAbstractElement> intervalResult = Methods::newInterval(sourceElement, init);
            if (minEnv.hasEvaluationError() || maxEnv.hasEvaluationError()
                  || querySCompareSpecial(*intervalResult->ppveMax).mayBeLessZero())
               resultElement = Methods::newTop(sourceElement);
            else
               resultElement = intervalResult;
         };
      };
      if (!resultElement.isValid() && !env.getInformationKind().isSure()) {
         resultElement = Methods::newTop(sourceElement);
         env.setPositiveOverflow();
      };

      if (resultElement.isValid()) {
         if (!env.getInformationKind().isSure()) {
            if (!result.isValid()) {
               env.presult() = resultElement;
               return true;
            };
            result->addMay(resultElement);
         };
         env.presult() = result;
      }
      else if (!env.getInformationKind().isSure() && !result.isValid())
         env.presult() = Methods::newTop(sourceElement);
      else
         env.setEmpty();
   };
   return true;
}

bool
IntegerInterval::applyToRightShiftAssignConstant(const VirtualOperation& operation,
      VirtualElement& source, EvaluationEnvironment& env) const {
   // this operation is too imprecise even if source is constant
   PNT::TPassPointer<IntegerInterval, PPAbstractElement> sourceInterval
      = Methods::newInterval(source, Init().setInterval(source, source).setUnsigned(queryUnsignedField()));
   assume(sourceInterval->applyRightShiftAssignInterval(operation, *this, env));
   if (!env.isEmpty() && !env.hasResult())
      env.presult() = sourceInterval;
   return true;
}

/* bitwise methods applyTo  */

bool
IntegerInterval::applyToLogicalAndAssignConstant(const VirtualOperation& operation,
      VirtualElement& source, EvaluationEnvironment& env) const {
   return const_cast<IntegerInterval&>(*this).applyLogicalAndAssignConstant(operation, source, env);
}

bool
IntegerInterval::applyToLogicalOrAssignConstant(const VirtualOperation& operation,
      VirtualElement& source, EvaluationEnvironment& env) const {
   return const_cast<IntegerInterval&>(*this).applyLogicalOrAssignConstant(operation, source, env);
}

/*****************/
/* apply methods */
/*****************/

/* cast apply methods */

PNT::TPassPointer<VirtualDisjunction, PPAbstractElement>
IntegerInterval::changeSign(EvaluationEnvironment& env) { // [a,b] -> [a, Maxint] U [Minint, b]
   QueryOperation::CompareSpecialEnvironment
      minCompare = Methods::compareSpecialSigned(*ppveMin),
      maxCompare = Methods::compareSpecialSigned(*ppveMax);
   PNT::TPassPointer<VirtualDisjunction, PPAbstractElement> result;

   if (!hasUnsignedField()) {
      setUnsignedField(true);
      if (minCompare.mayBeLessZero() && maxCompare.mayBeGreaterOrEqualZero()) {
         const VirtualElement& min = *ppveMin;
         result = Methods::newDisjunction(min); 
         result->addExact(Methods::newInterval(min,
            Init().setInterval(Methods::newZero(min), ppveMax).setUnsigned()));
         result->addExact(Methods::newInterval(min,
            Init().setInterval(ppveMin, Methods::newMaxUnsigned(min)).setUnsigned()));
         if (!minCompare.isLessZero() || !maxCompare.isGreaterOrEqualZero())
            env.mergeVerdictDegradate();
      };
   }
   else {
      setUnsignedField(false);
      if (maxCompare.mayBeLessZero() && minCompare.mayBeGreaterOrEqualZero()) {
         const VirtualElement& min = *ppveMin;
         result = Methods::newDisjunction(min); 
         result->addExact(Methods::newInterval(min,
            Init().setInterval(Methods::newMinSigned(min), ppveMax).setSigned()));
         result->addExact(Methods::newInterval(min,
            Init().setInterval(ppveMin, Methods::newMaxSigned(min)).setSigned()));
         if (!maxCompare.isLessZero() || !minCompare.isGreaterOrEqualZero())
            env.mergeVerdictDegradate();
      };
   };
   return result;
}

PNT::TPassPointer<VirtualDisjunction, PPAbstractElement>
IntegerInterval::applySureCast(const VirtualOperation& operation, EvaluationEnvironment& env) const {
   PNT::TPassPointer<VirtualDisjunction, PPAbstractElement> result;
   if (!env.getInformationKind().isMay()) {
      // (type) [a,b] = (type) b, (type) a, (type) (a+b)/2 en sure
      result = Methods::newDisjunction(*ppveMin);
      EvaluationParameters sure = EvaluationParameters(env).setSure();
      result->addSure(Methods::apply(*ppveMin, operation, env, sure));
      result->addSure(Methods::apply(*ppveMax, operation, env, sure));
      PPVirtualElement middle
         = Methods::applyAssign(Methods::apply(*ppveMin, querySPlusOperation(), *ppveMax, sure),
            Methods::queryRightShiftSignedAssign(*ppveMin),
            *Methods::newIntForShift(*ppveMin, 1), sure);
      result->addSure(Methods::apply(*middle, operation, env, sure));
   };
   return result;
}

bool
IntegerInterval::applyDispatchMinMax(const VirtualOperation& operation, EvaluationEnvironment& env) {
   TransmitEnvironment minEnv(env), maxEnv(env);
   assume(ppveMin->apply(operation, minEnv) && ppveMax->apply(operation, maxEnv));
   if (minEnv.hasResult())
      ppveMin = minEnv.presult();
   if (maxEnv.hasResult())
      ppveMax = maxEnv.presult();
   return true;
}

bool
IntegerInterval::applyCastInteger(const VirtualOperation& operation, EvaluationEnvironment& env) {
   TransmitEnvironment minEnv(env), maxEnv(env);
   assume(ppveMin->apply(operation, minEnv) && ppveMax->apply(operation, maxEnv));

   if (maxEnv.isPositiveOverflow() || minEnv.isNegativeOverflow()) {
      env.mergeVerdictDegradate();
      EvaluationEnvironment::LatticeCreation creation = env.getLatticeCreation();
      if (creation.isTop() || creation.isShareTop()) {
         env.presult() = Details::IntOperationElement::Methods::newTop(*this);
         return true;
      };

      // returns top in case of overflow
      PNT::TPassPointer<VirtualDisjunction, PPAbstractElement> result = applySureCast(operation, env);
      if (!env.getInformationKind().isMay()) {
         if (minEnv.isSureNegativeOverflow()) {
            if (maxEnv.isSurePositiveOverflow()) {
               env.presult() = Methods::newTop(minEnv.getResult());
               return true;
            }
            else if (!maxEnv.isNegativeOverflow()) {
               Init init;
               init.setLessOrEqualThan(maxEnv.presult()).setUnsigned(queryUnsignedField()),
               result->addSure(Methods::newInterval(*init.ppveFst, init));
            };
         };
         if (maxEnv.isSurePositiveOverflow()) {
            if (!minEnv.isPositiveOverflow()) {
               Init init;
               init.setGreaterOrEqualThan(minEnv.presult()).setUnsigned(queryUnsignedField());
               result->addSure(Methods::newInterval(*init.ppveFst, init));
            };
         };
      };
      if (!result.isValid()) {
         AssumeCondition((env.getInformationKind().isMay()) && !env.hasResult())
         env.presult() = Methods::newTop(minEnv.getResult());
      }
      else
         result->addMay(Methods::newTop(minEnv.getResult()));
      env.presult() = result;
   }
   else {
      Init init;
      init.setInterval(minEnv.presult(), maxEnv.presult()).setUnsigned(queryUnsignedField());
      env.presult() = Methods::newInterval(*init.ppveFst, init);
   };
   return true;
}

bool
IntegerInterval::applyCastDouble(const VirtualOperation& operation, EvaluationEnvironment& env) {
   TransmitEnvironment minEnv(env), maxEnv(env);
   assume(ppveMin->apply(operation, minEnv) && ppveMax->apply(operation, maxEnv));
   PNT::TPassPointer<VirtualDisjunction, PPAbstractElement> result = applySureCast(operation, env);
   if (result.isValid()) {
      VirtualElement& min = minEnv.getResult();
      result->addMay(RealOperationElement::Methods::newInterval(min,
               minEnv.presult(), maxEnv.presult()));
      env.presult() = result;
   }
   else {
      AssumeCondition(env.getInformationKind().isMay())
      env.presult() = RealOperationElement::Methods::newInterval(minEnv.getResult(),
               minEnv.presult(), maxEnv.presult());
   };
   return true;
}

/* compare apply methods */

template <class TypeDecision>
bool
IntegerInterval::applyCompareConstant(TypeDecision, const VirtualElement& source, EvaluationEnvironment& env) const {
   TransmitEnvironment minEnv(env), maxEnv(env);
   minEnv.setFirstArgument(source);
   maxEnv.setFirstArgument(source);
   QueryOperation::OperationEnvironment min, max;
   ppveMin->query(TypeDecision::minOperation(), min);
   ppveMin->query(TypeDecision::maxOperation(), max);
   assume(ppveMin->apply(min.result(), minEnv) && ppveMax->apply(max.result(), maxEnv));
   bool isDifferent = TypeDecision::maxInverseResult && TypeDecision::maxResult;

   // [a,b] < x
   if (minEnv.getResult().queryZeroResult().isTrue()) { // a >= x
      env.presult() = (const PPVirtualElement&) (TypeDecision::minResult
            ? Methods::newTrue(source) : Methods::newFalse(source));
      env.mergeVerdictExact();
   }
   else if (minEnv.getResult().queryZeroResult().isFalse()) { // a < x
      ZeroResult maxResult = maxEnv.getResult().queryZeroResult();

      if (!isDifferent ? maxResult.isTrue() : maxResult.isFalse()) { // b < x
         env.mergeVerdictExact();
         env.presult() = (const PPVirtualElement&) (TypeDecision::maxResult
            ? Methods::newTrue(*ppveMin) : Methods::newFalse(*ppveMin));
      }
      else if (!isDifferent ? maxResult.isFalse() : maxResult.isTrue()) { // a < x <= b
         env.mergeVerdictExact();

         TransmitEnvironment equalEnv(env);
         equalEnv.setFirstArgument(*ppveMax);
         QueryOperation::OperationEnvironment equalQuery;
         ppveMin->query(IntegerInterval::QueryOperation().setCompareEqual(), equalQuery);
         assume(ppveMin->apply(equalQuery.result(), equalEnv));
         if (equalEnv.getResult().queryZeroResult().isTrue()) // a == b
            env.presult() = Methods::newTrue(*ppveMin);
         else {
            PNT::TPassPointer<VirtualDisjunction, PPAbstractElement> disjunction
               = Methods::newBooleanDisjunction(*ppveMin);
            disjunction->add(Methods::newFalse(*ppveMin), env.getInformationKind());
            disjunction->add(Methods::newTrue(*ppveMin), env.getInformationKind());
            env.presult() = disjunction;
         };
      }
      else { // a < x, but no information about the relative position of x w.r.t b
         env.mergeVerdictDegradate();
         if (env.getInformationKind().isSure()) {
            if (TypeDecision::minInverseResult == !TypeDecision::minResult)
               env.presult() = TypeDecision::minResult ? Methods::newFalse(*ppveMin) : Methods::newTrue(*ppveMin);
            else {
               env.setEmpty();
               return true;
            };
         }
         else {
            PNT::TPassPointer<VirtualDisjunction, PPAbstractElement> disjunction
               = Methods::newBooleanDisjunction(*ppveMin);
            disjunction->addMay(Methods::newFalse(*ppveMin));
            disjunction->addMay(Methods::newTrue(*ppveMin));
            if ((env.getInformationKind().isExact())
                  && (TypeDecision::minInverseResult == !TypeDecision::minResult))
               disjunction->addSure((PPVirtualElement) (TypeDecision::minResult ? Methods::newFalse(*ppveMin) : Methods::newTrue(*ppveMin)));
            env.presult() = disjunction;
         };
      };
   }
   else if (!isDifferent ? maxEnv.getResult().queryZeroResult().isTrue() : maxEnv.getResult().queryZeroResult().isFalse()) { // b < x
      env.mergeVerdictExact();
      env.presult() = (const PPVirtualElement&) (TypeDecision::maxResult
         ? Methods::newTrue(*ppveMin) : Methods::newFalse(*ppveMin));
   }
   else if (!isDifferent ? maxEnv.getResult().queryZeroResult().isFalse() : maxEnv.getResult().queryZeroResult().isTrue()) { // x <= b
      if (env.getInformationKind().isSure()) {
         if (TypeDecision::maxInverseResult == !TypeDecision::maxResult)
            env.presult() = TypeDecision::maxResult ? Methods::newFalse(*ppveMin) : Methods::newTrue(*ppveMin);
         else {
            env.setEmpty();
            return true;
         };
      }
      else {
         env.mergeVerdictDegradate();
         PNT::TPassPointer<VirtualDisjunction, PPAbstractElement> disjunction
            = Methods::newBooleanDisjunction(*ppveMin);
         disjunction->addMay(Methods::newFalse(*ppveMin));
         disjunction->addMay(Methods::newTrue(*ppveMin));
         if ((env.getInformationKind().isExact())
               && (TypeDecision::maxInverseResult == !TypeDecision::maxResult))
            disjunction->addSure((const PPVirtualElement&) (TypeDecision::maxResult
                     ? Methods::newFalse(*ppveMin) : Methods::newTrue(*ppveMin)));
         env.presult() = disjunction;
      };
   }
   else { // no information about the relative position of x w.r.t a and b
      if (env.getInformationKind().isSure()) {
         env.setEmpty();
         return true;
      };
      env.mergeVerdictDegradate();
      PNT::TPassPointer<VirtualDisjunction, PPAbstractElement> disjunction
         = Methods::newBooleanDisjunction(*ppveMin);
      disjunction->addMay(Methods::newFalse(*ppveMin));
      disjunction->addMay(Methods::newTrue(*ppveMin));
      env.presult() = disjunction;
   };
   return true;
}

template <class TypeDecision>
bool
IntegerInterval::applyCompareInterval(TypeDecision, const IntegerInterval& source, EvaluationEnvironment& env) const {
   TransmitEnvironment minEnv(env), maxEnv(env);
   minEnv.setFirstArgument(*source.ppveMax);
   maxEnv.setFirstArgument(*source.ppveMin);
   QueryOperation::OperationEnvironment min, max;
   ppveMin->query(TypeDecision::minOperation(), min);
   ppveMin->query(TypeDecision::maxOperation(), max);
   assume(ppveMin->apply(min.result(), minEnv) && ppveMax->apply(max.result(), maxEnv));

   // [a,b] < [c,d]
   if (minEnv.getResult().queryZeroResult().isTrue()) { // a > d
      env.presult() = (const PPVirtualElement&) (TypeDecision::minResult
            ? Methods::newTrue(*ppveMin) : Methods::newFalse(*ppveMin));
      env.mergeVerdictExact();
   }
   else if (minEnv.getResult().queryZeroResult().isFalse()) { // a <= d
      if (maxEnv.getResult().queryZeroResult().isTrue()) { // a <= d && b < c
         env.mergeVerdictExact();
         env.presult() = (const PPVirtualElement&) (TypeDecision::maxResult
               ? Methods::newTrue(*ppveMin) : Methods::newFalse(*ppveMin));
      }
      else if (maxEnv.getResult().queryZeroResult().isFalse()) { // a <= d && b >= c
         env.mergeVerdictExact();
         PNT::TPassPointer<VirtualDisjunction, PPAbstractElement> disjunction
            = Methods::newBooleanDisjunction(*ppveMin);
         disjunction->add(Methods::newFalse(*ppveMin), env.getInformationKind());
         disjunction->add(Methods::newTrue(*ppveMin), env.getInformationKind());
         env.presult() = disjunction;
      }
      else { // a <= d, but no information about the relative position of c w.r.t b
         env.mergeVerdictDegradate();
         if (env.getInformationKind().isSure()) {
            if (TypeDecision::minInverseResult == !TypeDecision::minResult)
               env.presult() = TypeDecision::minResult ? Methods::newFalse(*ppveMin) : Methods::newTrue(*ppveMin);
            else {
               env.setEmpty();
               return true;
            };
         }
         else {
            PNT::TPassPointer<VirtualDisjunction, PPAbstractElement> disjunction
               = Methods::newBooleanDisjunction(*ppveMin);
            disjunction->addMay(Methods::newFalse(*ppveMin));
            disjunction->addMay(Methods::newTrue(*ppveMin));
            if ((env.getInformationKind().isExact())
                  && (TypeDecision::minInverseResult == !TypeDecision::minResult))
               disjunction->addSure((PPVirtualElement) (TypeDecision::minResult ? Methods::newFalse(*ppveMin) : Methods::newTrue(*ppveMin)));
            env.presult() = disjunction;
         };

      };
   }
   else if (maxEnv.getResult().queryZeroResult().isTrue()) { // b < c
      env.mergeVerdictExact();
      env.presult() = (const PPVirtualElement&) (TypeDecision::maxResult
            ? Methods::newTrue(*ppveMin) : Methods::newFalse(*ppveMin));
   }
   else if (maxEnv.getResult().queryZeroResult().isTrue()) { // x <= b
      if (env.getInformationKind().isSure()) {
         env.mergeVerdictExact();
         if (TypeDecision::maxInverseResult == !TypeDecision::maxResult)
            env.presult() = TypeDecision::maxResult ? Methods::newFalse(*ppveMin) : Methods::newTrue(*ppveMin);
         else {
            env.setEmpty();
            return true;
         };
      }
      else {
         env.mergeVerdictDegradate();
         PNT::TPassPointer<VirtualDisjunction, PPAbstractElement> disjunction
            = Methods::newBooleanDisjunction(*ppveMin);
         disjunction->addMay(Methods::newFalse(*ppveMin));
         disjunction->addMay(Methods::newTrue(*ppveMin));
         if ((env.getInformationKind().isExact())
               && (TypeDecision::maxInverseResult == !TypeDecision::maxResult))
            disjunction->addSure((const PPVirtualElement&) (TypeDecision::maxResult
                     ? Methods::newFalse(*ppveMin) : Methods::newTrue(*ppveMin)));
         env.presult() = disjunction;
      };
   }
   else { // no information about the relative position of x .r.t a and b
      if (env.getInformationKind().isSure()) {
         env.setEmpty();
         return true;
      };
      env.mergeVerdictDegradate();
      PNT::TPassPointer<VirtualDisjunction, PPAbstractElement> disjunction
         = Methods::newBooleanDisjunction(*ppveMin);
      disjunction->addMay(Methods::newFalse(*ppveMin));
      disjunction->addMay(Methods::newTrue(*ppveMin));
      env.presult() = disjunction;
   };

   return true;
}

class CompareLessDecisionUnsigned { // [a,b] < x is false if a >= x and true if b < x
  public:
   static IntegerInterval::QueryOperation minOperation() { return IntegerInterval::QueryOperation().setTypeOperation(IntegerInterval::QueryOperation::TOCompareGreaterOrEqualUnsigned); }
   static IntegerInterval::QueryOperation maxOperation() { return IntegerInterval::QueryOperation().setTypeOperation(IntegerInterval::QueryOperation::TOCompareLessUnsigned); }
   static const bool minResult = false;
   static const bool maxResult = true;
   static const bool minInverseResult = !minResult;
   static const bool maxInverseResult = !maxResult;
};

class CompareLessDecisionSigned { // [a,b] < x is false if a >= x and true if b < x
  public:
   static IntegerInterval::QueryOperation minOperation() { return IntegerInterval::QueryOperation().setTypeOperation(IntegerInterval::QueryOperation::TOCompareGreaterOrEqualSigned); }
   static IntegerInterval::QueryOperation maxOperation() { return IntegerInterval::QueryOperation().setTypeOperation(IntegerInterval::QueryOperation::TOCompareLessSigned); }
   static const bool minResult = false;
   static const bool maxResult = true;
   static const bool minInverseResult = !minResult;
   static const bool maxInverseResult = !maxResult;
};

bool
IntegerInterval::applyCompareLessConstant(const VirtualOperation&,
      const VirtualElement& source, EvaluationEnvironment& env)
   {  return (!hasUnsignedField())
         ?  applyCompareConstant(CompareLessDecisionSigned(), source, env)
         :  applyCompareConstant(CompareLessDecisionUnsigned(), source, env);
   }

bool
IntegerInterval::applyCompareLessInterval(const VirtualOperation& operation,
      const IntegerInterval& source, EvaluationEnvironment& env) {
   PNT::TPassPointer<IntegerInterval, PPAbstractElement> copy;
   const IntegerInterval* intervalThis = this;
   if (queryUnsignedField() != source.queryUnsignedField()) {
      copy.setElement(*intervalThis);
      PNT::TPassPointer<VirtualDisjunction, PPAbstractElement> conversion = copy->changeSign(env);
      if (conversion.isValid()) {
         TransmitEnvironment applyEnv(env);
         applyEnv.setFirstArgument(source);
         assume(conversion->apply(operation, applyEnv));
         env.presult() = applyEnv.hasResult() ? applyEnv.presult() : (PPAbstractElement) conversion;
         return true;
      };
      intervalThis = &*copy;
   };
   return !intervalThis->hasUnsignedField()
         ?  intervalThis->applyCompareInterval(CompareLessDecisionSigned(), source, env)
         :  intervalThis->applyCompareInterval(CompareLessDecisionUnsigned(), source, env);
}

class CompareLessOrEqualDecisionUnsigned { // [a,b] <= x is false if a > x and true if b <= x
  public:
   static IntegerInterval::QueryOperation minOperation() { return IntegerInterval::QueryOperation().setTypeOperation(IntegerInterval::QueryOperation::TOCompareGreaterUnsigned); }
   static IntegerInterval::QueryOperation maxOperation() { return IntegerInterval::QueryOperation().setTypeOperation(IntegerInterval::QueryOperation::TOCompareLessOrEqualUnsigned); }
   static const bool minResult = false;
   static const bool maxResult = true;
   static const bool minInverseResult = !minResult;
   static const bool maxInverseResult = !maxResult;
};

class CompareLessOrEqualDecisionSigned { // [a,b] <= x is false if a > x and true if b <= x
  public:
   static IntegerInterval::QueryOperation minOperation() { return IntegerInterval::QueryOperation().setTypeOperation(IntegerInterval::QueryOperation::TOCompareGreaterSigned); }
   static IntegerInterval::QueryOperation maxOperation() { return IntegerInterval::QueryOperation().setTypeOperation(IntegerInterval::QueryOperation::TOCompareLessOrEqualSigned); }
   static const bool minResult = false;
   static const bool maxResult = true;
   static const bool minInverseResult = !minResult;
   static const bool maxInverseResult = !maxResult;
};

bool
IntegerInterval::applyCompareLessOrEqualConstant(const VirtualOperation&,
      const VirtualElement& source, EvaluationEnvironment& env)
   {  return !hasUnsignedField()
         ? applyCompareConstant(CompareLessOrEqualDecisionSigned(), source, env)
         : applyCompareConstant(CompareLessOrEqualDecisionUnsigned(), source, env);
   }

bool
IntegerInterval::applyCompareLessOrEqualInterval(const VirtualOperation& operation,
      const IntegerInterval& source, EvaluationEnvironment& env) {  
   PNT::TPassPointer<IntegerInterval, PPAbstractElement> copy;
   const IntegerInterval* intervalThis = this;
   if (queryUnsignedField() != source.queryUnsignedField()) {
      copy.setElement(*intervalThis);
      PNT::TPassPointer<VirtualDisjunction, PPAbstractElement> conversion = copy->changeSign(env);
      if (conversion.isValid()) {
         TransmitEnvironment applyEnv(env);
         applyEnv.setFirstArgument(source);
         assume(conversion->apply(operation, applyEnv));
         env.presult() = applyEnv.hasResult() ? applyEnv.presult() : (PPAbstractElement) conversion;
         return true;
      };
      intervalThis = &*copy;
   };
   return !intervalThis->hasUnsignedField()
      ? intervalThis->applyCompareInterval(CompareLessOrEqualDecisionSigned(), source, env)
      : intervalThis->applyCompareInterval(CompareLessOrEqualDecisionUnsigned(), source, env);
}

class CompareGreaterOrEqualDecisionSigned { // [a,b] >= x is true if a >= x and false if b < x
  public:
   static IntegerInterval::QueryOperation minOperation() { return IntegerInterval::QueryOperation().setTypeOperation(IntegerInterval::QueryOperation::TOCompareGreaterOrEqualSigned); }
   static IntegerInterval::QueryOperation maxOperation() { return IntegerInterval::QueryOperation().setTypeOperation(IntegerInterval::QueryOperation::TOCompareLessSigned); }
   static const bool minResult = true;
   static const bool maxResult = false;
   static const bool minInverseResult = !minResult;
   static const bool maxInverseResult = !maxResult;
};

class CompareGreaterOrEqualDecisionUnsigned { // [a,b] >= x is true if a >= x and false if b < x
  public:
   static IntegerInterval::QueryOperation minOperation() { return IntegerInterval::QueryOperation().setTypeOperation(IntegerInterval::QueryOperation::TOCompareGreaterOrEqualUnsigned); }
   static IntegerInterval::QueryOperation maxOperation() { return IntegerInterval::QueryOperation().setTypeOperation(IntegerInterval::QueryOperation::TOCompareLessUnsigned); }
   static const bool minResult = true;
   static const bool maxResult = false;
   static const bool minInverseResult = !minResult;
   static const bool maxInverseResult = !maxResult;
};

bool
IntegerInterval::applyCompareGreaterOrEqualConstant(const VirtualOperation&,
      const VirtualElement& source, EvaluationEnvironment& env)
   {  return !hasUnsignedField()
         ? applyCompareConstant(CompareGreaterOrEqualDecisionSigned(), source, env)
         : applyCompareConstant(CompareGreaterOrEqualDecisionUnsigned(), source, env);
   }

bool
IntegerInterval::applyCompareGreaterOrEqualInterval(const VirtualOperation& operation,
      const IntegerInterval& source, EvaluationEnvironment& env) {
   PNT::TPassPointer<IntegerInterval, PPAbstractElement> copy;
   const IntegerInterval* intervalThis = this;
   if (queryUnsignedField() != source.queryUnsignedField()) {
      copy.setElement(*intervalThis);
      PNT::TPassPointer<VirtualDisjunction, PPAbstractElement> conversion = copy->changeSign(env);
      if (conversion.isValid()) {
         TransmitEnvironment applyEnv(env);
         applyEnv.setFirstArgument(source);
         assume(conversion->apply(operation, applyEnv));
         env.presult() = applyEnv.hasResult() ? applyEnv.presult() : (PPAbstractElement) conversion;
         return true;
      };
      intervalThis = &*copy;
   };
   return !intervalThis->hasUnsignedField()
      ? intervalThis->applyCompareInterval(CompareGreaterOrEqualDecisionSigned(), source, env)
      : intervalThis->applyCompareInterval(CompareGreaterOrEqualDecisionUnsigned(), source, env);
}

class CompareGreaterDecisionUnsigned { // [a,b] > x is true if a > x and false if b <= x
  public:
   static IntegerInterval::QueryOperation minOperation() { return IntegerInterval::QueryOperation().setTypeOperation(IntegerInterval::QueryOperation::TOCompareGreaterUnsigned); }
   static IntegerInterval::QueryOperation maxOperation() { return IntegerInterval::QueryOperation().setTypeOperation(IntegerInterval::QueryOperation::TOCompareLessOrEqualUnsigned); }
   static const bool minResult = true;
   static const bool maxResult = false;
   static const bool minInverseResult = !minResult;
   static const bool maxInverseResult = !maxResult;
};

class CompareGreaterDecisionSigned { // [a,b] > x is true if a > x and false if b <= x
  public:
   static IntegerInterval::QueryOperation minOperation() { return IntegerInterval::QueryOperation().setTypeOperation(IntegerInterval::QueryOperation::TOCompareGreaterSigned); }
   static IntegerInterval::QueryOperation maxOperation() { return IntegerInterval::QueryOperation().setTypeOperation(IntegerInterval::QueryOperation::TOCompareLessOrEqualSigned); }
   static const bool minResult = true;
   static const bool maxResult = false;
   static const bool minInverseResult = !minResult;
   static const bool maxInverseResult = !maxResult;
};

bool
IntegerInterval::applyCompareGreaterConstant(const VirtualOperation&,
      const VirtualElement& source, EvaluationEnvironment& env)
   {  return !hasUnsignedField()
         ? applyCompareConstant(CompareGreaterDecisionSigned(), source, env)
         : applyCompareConstant(CompareGreaterDecisionUnsigned(), source, env);
   }

bool
IntegerInterval::applyCompareGreaterInterval(const VirtualOperation& operation,
      const IntegerInterval& source, EvaluationEnvironment& env) {
   PNT::TPassPointer<IntegerInterval, PPAbstractElement> copy;
   const IntegerInterval* intervalThis = this;
   if (queryUnsignedField() != source.queryUnsignedField()) {
      copy.setElement(*intervalThis);
      PNT::TPassPointer<VirtualDisjunction, PPAbstractElement> conversion = copy->changeSign(env);
      if (conversion.isValid()) {
         TransmitEnvironment applyEnv(env);
         applyEnv.setFirstArgument(source);
         assume(conversion->apply(operation, applyEnv));
         env.presult() = applyEnv.hasResult() ? applyEnv.presult() : (PPAbstractElement) conversion;
         return true;
      };
      intervalThis = &*copy;
   };
   return !intervalThis->hasUnsignedField()
      ? intervalThis->applyCompareInterval(CompareGreaterDecisionSigned(), source, env)
      : intervalThis->applyCompareInterval(CompareGreaterDecisionUnsigned(), source, env);
}

class CompareEqualDecisionSigned { // [a,b] == x may be true if a <= x and x <= b
  public:
   static IntegerInterval::QueryOperation minOperation() { return IntegerInterval::QueryOperation().setTypeOperation(IntegerInterval::QueryOperation::TOCompareGreaterSigned); }
   static IntegerInterval::QueryOperation maxOperation() { return IntegerInterval::QueryOperation().setTypeOperation(IntegerInterval::QueryOperation::TOCompareLessSigned); }
   static const bool minResult = false;
   static const bool maxResult = false;
   static const bool minInverseResult = false;
   static const bool maxInverseResult = false;
};

class CompareEqualDecisionUnsigned { // [a,b] == x may be true if a <= x and x <= b
  public:
   static IntegerInterval::QueryOperation minOperation() { return IntegerInterval::QueryOperation().setTypeOperation(IntegerInterval::QueryOperation::TOCompareGreaterUnsigned); }
   static IntegerInterval::QueryOperation maxOperation() { return IntegerInterval::QueryOperation().setTypeOperation(IntegerInterval::QueryOperation::TOCompareLessUnsigned); }
   static const bool minResult = false;
   static const bool maxResult = false;
   static const bool minInverseResult = false;
   static const bool maxInverseResult = false;
};

bool
IntegerInterval::applyCastBit(const VirtualOperation&, EvaluationEnvironment& env)
   {  PPVirtualElement source = Methods::newZero(*this);
      return !hasUnsignedField()
         ? applyCompareConstant(CompareEqualDecisionSigned(), *source, env)
         : applyCompareConstant(CompareEqualDecisionUnsigned(), *source, env);
   }

bool
IntegerInterval::applyCompareEqualConstant(const VirtualOperation&,
      const VirtualElement& source, EvaluationEnvironment& env)
   {  return !hasUnsignedField()
         ? applyCompareConstant(CompareEqualDecisionSigned(), source, env)
         : applyCompareConstant(CompareEqualDecisionUnsigned(), source, env);
   }

bool
IntegerInterval::applyCompareEqualInterval(const VirtualOperation& operation,
      const IntegerInterval& source, EvaluationEnvironment& env) {
   PNT::TPassPointer<IntegerInterval, PPAbstractElement> copy;
   const IntegerInterval* intervalThis = this;
   if (queryUnsignedField() != source.queryUnsignedField()) {
      copy.setElement(*intervalThis);
      PNT::TPassPointer<VirtualDisjunction, PPAbstractElement> conversion = copy->changeSign(env);
      if (conversion.isValid()) {
         TransmitEnvironment applyEnv(env);
         applyEnv.setFirstArgument(source);
         assume(conversion->apply(operation, applyEnv));
         env.presult() = applyEnv.hasResult() ? applyEnv.presult() : (PPAbstractElement) conversion;
         return true;
      };
      intervalThis = &*copy;
   };
   return !intervalThis->hasUnsignedField()
      ? intervalThis->applyCompareInterval(CompareEqualDecisionSigned(), source, env)
      : intervalThis->applyCompareInterval(CompareEqualDecisionUnsigned(), source, env);
}

class CompareDifferentDecisionSigned { // [a,b] != x is always true if a > x or x > b
  public:
   static IntegerInterval::QueryOperation minOperation() { return IntegerInterval::QueryOperation().setTypeOperation(IntegerInterval::QueryOperation::TOCompareGreaterSigned); }
   static IntegerInterval::QueryOperation maxOperation() { return IntegerInterval::QueryOperation().setTypeOperation(IntegerInterval::QueryOperation::TOCompareLessSigned); }
   static const bool minResult = true;
   static const bool maxResult = true;
   static const bool minInverseResult = true;
   static const bool maxInverseResult = true;
};

class CompareDifferentDecisionUnsigned { // [a,b] != x is always true if a > x or x > b
  public:
   static IntegerInterval::QueryOperation minOperation() { return IntegerInterval::QueryOperation().setTypeOperation(IntegerInterval::QueryOperation::TOCompareGreaterUnsigned); }
   static IntegerInterval::QueryOperation maxOperation() { return IntegerInterval::QueryOperation().setTypeOperation(IntegerInterval::QueryOperation::TOCompareLessUnsigned); }
   static const bool minResult = true;
   static const bool maxResult = true;
   static const bool minInverseResult = true;
   static const bool maxInverseResult = true;
};

bool
IntegerInterval::applyCompareDifferentConstant(const VirtualOperation&,
      const VirtualElement& source, EvaluationEnvironment& env)
   {  return !hasUnsignedField()
         ?  applyCompareConstant(CompareDifferentDecisionSigned(), source, env)
         :  applyCompareConstant(CompareDifferentDecisionUnsigned(), source, env);
   }

bool
IntegerInterval::applyCompareDifferentInterval(const VirtualOperation& operation,
      const IntegerInterval& source, EvaluationEnvironment& env) {
   PNT::TPassPointer<IntegerInterval, PPAbstractElement> copy;
   const IntegerInterval* intervalThis = this;
   if (queryUnsignedField() != source.queryUnsignedField()) {
      copy.setElement(*intervalThis);
      PNT::TPassPointer<VirtualDisjunction, PPAbstractElement> conversion = copy->changeSign(env);
      if (conversion.isValid()) {
         TransmitEnvironment applyEnv(env);
         applyEnv.setFirstArgument(source);
         assume(conversion->apply(operation, applyEnv));
         env.presult() = applyEnv.hasResult() ? applyEnv.presult() : (PPAbstractElement) conversion;
         return true;
      };
      intervalThis = &*copy;
   };
   return !intervalThis->hasUnsignedField()
      ? intervalThis->applyCompareInterval(CompareDifferentDecisionSigned(), source, env)
      : intervalThis->applyCompareInterval(CompareDifferentDecisionUnsigned(), source, env);
}

bool
IntegerInterval::applyMinAssignConstant(const VirtualOperation&,
      const VirtualElement& source, EvaluationEnvironment& env) {
   env.mergeVerdictExact();
   if (Methods::applyBoolean(*ppveMax, querySCompareLessOrEqualOperation(), source))
      return true; // min([a,b], c) = [a,b] if b < c
   if (Methods::applyBoolean(*ppveMin, querySCompareGreaterOrEqualOperation(), source)) {
      env.storeResult(source.createSCopy());
      return true; // min([a,b], c) = c if a > c (empty test)
   };

   // a <= c as may information and c >= b as may information
   if (env.getInformationKind().isMay()) { // min([a,b], c) = [a,c] as may information
      ppveMax = Methods::applyAssign(ppveMax, querySMinAssignOperation(), source, env);
      return mergeWithConstant(source, env);
   }
   else {  // min([a,b], c) = [a,min(b,c)] U {c} if c is surely less than b and if [a,b] is not empty
      EvaluationParameters sureStopErrorStates(EPSureStopErrorStates);
      sureStopErrorStates.setApproximatePartFrom(env);
      PPVirtualElement newMax = Methods::apply(*ppveMax, querySMinOperation(), source, env),
         compareFst = Methods::apply(*ppveMax, querySCompareGreaterOrEqualOperation(), source, sureStopErrorStates),
         compareSnd = Methods::apply(*ppveMin, querySCompareLessOrEqualOperation(), *ppveMax, sureStopErrorStates);
      if (compareFst->queryZeroResult().mayBeDifferentZero()
            && compareSnd->queryZeroResult().mayBeDifferentZero()) {
         TransmitEnvironment mergeEnv(env);
         assume(mergeWithConstant(source, mergeEnv));
         return true;
      }
      else if (env.getInformationKind().isExact()) {
         PNT::TPassPointer<VirtualDisjunction, PPAbstractElement> result
            = Methods::newDisjunction(*ppveMin);
         const VirtualElement& min = *ppveMin;
         result->addExact(Methods::newInterval(min, Init().setInterval(ppveMin, ppveMax).setUnsigned(queryUnsignedField())));
         result->addMay(source);
         env.presult() = result;
      };
      env.mergeVerdictDegradate();
   };
   return true;
}

bool
IntegerInterval::applyMaxAssignConstant(const VirtualOperation&,
      const VirtualElement& source, EvaluationEnvironment& env) {
   env.mergeVerdictExact();
   if (Methods::applyBoolean(*ppveMin, querySCompareGreaterOrEqualOperation(), source))
      return true; // max([a,b], c) = [a,b] if a > c
   if (Methods::applyBoolean(*ppveMax, querySCompareLessOrEqualOperation(), source)) {
      env.storeResult(source.createSCopy());
      return true; // max([a,b], c) = c if b < c (empty test)
   };

   // a <= c as may information and c >= b as may information
   if (env.getInformationKind().isMay()) { // max([a,b], c) = [c,b] as may information
      auto maxAssignOperation = querySMaxAssignOperation();
      ppveMin = Methods::applyAssign(ppveMin, maxAssignOperation, source, env);
      return mergeWithConstant(source, env);
   }
   else {  // max([a,b], c) = [max(a,c), b] U {c} if c is surely greater then a and if [a,b] is not empty
      EvaluationParameters sureStopErrorStates(EPSureStopErrorStates);
      sureStopErrorStates.setApproximatePartFrom(env);
      PPVirtualElement newMin = Methods::apply(*ppveMin, querySMaxOperation(), source, env),
         compareFst = Methods::apply(*ppveMax, querySCompareLessOrEqualOperation(), source, sureStopErrorStates),
         compareSnd = Methods::apply(*ppveMin, querySCompareLessOrEqualOperation(), *ppveMax, sureStopErrorStates);
      if (compareFst->queryZeroResult().mayBeDifferentZero()
            && compareSnd->queryZeroResult().mayBeDifferentZero()) {
         TransmitEnvironment mergeEnv(env);
         assume(mergeWithConstant(source, mergeEnv));
         return true;
      }
      else if (env.getInformationKind().isExact()) {
         PNT::TPassPointer<VirtualDisjunction, PPAbstractElement> result
            = Methods::newDisjunction(*ppveMin);
         const VirtualElement& min = *ppveMin;
         result->addExact(Methods::newInterval(min, Init().setInterval(ppveMin, ppveMax).setUnsigned(queryUnsignedField())));
         result->addMay(source);
         env.presult() = result;
      };
      env.mergeVerdictDegradate();
   };
   return true;
}

bool
IntegerInterval::applyMinAssignInterval(const VirtualOperation& operation,
      const IntegerInterval& source, EvaluationEnvironment& env) {
   if (queryUnsignedField() != source.queryUnsignedField()) {
      PNT::TPassPointer<VirtualDisjunction, PPAbstractElement> copy = changeSign(env);
      if (copy.isValid()) {
         TransmitEnvironment applyEnv(env);
         applyEnv.setFirstArgument(source);
         assume(copy->apply(operation, applyEnv));
         env.presult() = applyEnv.hasResult() ? applyEnv.presult() : (PPAbstractElement) copy;
         return true;
      };
   };
   try {
      env.setFirstArgument(*source.ppveMin);
      ppveMin->apply(operation, env);
      if (env.hasResult())
         ppveMin = env.presult();
      env.setFirstArgument(*source.ppveMax);
      ppveMax->apply(operation, env);
      if (env.hasResult())
         ppveMax = env.presult();
      env.setFirstArgument(source);
   }
   catch (...) {
      env.setFirstArgument(source);
      throw;
   };
   return true;
}

bool
IntegerInterval::applyMaxAssignInterval(const VirtualOperation& operation,
      const IntegerInterval& source, EvaluationEnvironment& env) {
   if (queryUnsignedField() != source.queryUnsignedField()) {
      PNT::TPassPointer<VirtualDisjunction, PPAbstractElement> copy = changeSign(env);
      if (copy.isValid()) {
         TransmitEnvironment applyEnv(env);
         applyEnv.setFirstArgument(source);
         assume(copy->apply(operation, applyEnv));
         env.presult() = applyEnv.hasResult() ? applyEnv.presult() : (PPAbstractElement) copy;
         return true;
      };
   };
   try {
      env.setFirstArgument(*source.ppveMin);
      ppveMin->apply(operation, env);
      if (env.hasResult())
         ppveMin = env.presult();
      env.setFirstArgument(*source.ppveMax);
      ppveMax->apply(operation, env);
      if (env.hasResult())
         ppveMax = env.presult();
      env.setFirstArgument(source);
   }
   catch (...) {
      env.setFirstArgument(source);
      throw;
   };
   return true;
}

/* numeric apply methods */

bool
IntegerInterval::applyConcat(const VirtualOperation& operation, EvaluationEnvironment& env) {
   PPVirtualElement topThis = Methods::newTop(*this);
   topThis->apply(operation, env);
   if (!env.hasResult())
      env.presult() = topThis;

   env.mergeVerdictDegradate();
   return true;
}

bool
IntegerInterval::applyBitSet(const VirtualOperation& operation, EvaluationEnvironment& env) {
   EvaluationEnvironment::LatticeCreation creation = env.getLatticeCreation();
   AssumeCondition(dynamic_cast<const Scalar::MultiBit::BitSetOperation*>(&operation))
   int lowBit = ((const Scalar::MultiBit::BitSetOperation&) operation).getLowBit();
   int highBit = ((const Scalar::MultiBit::BitSetOperation&) operation).getHighBit();
   int size = ppveMin->getSizeInBits();
   if (lowBit == 0 && highBit == size - 1) {
      env.setResult(env.getFirstArgument());
      return true;
   };

   if (creation.mayBeFormal()) {
      if (highBit < size - 1) {
         PPVirtualElement zero = Methods::newZero(*ppveMin);
         if (Methods::applyBoolean(*ppveMin,
               Scalar::MultiBit::CompareGreaterOrEqualSignedOperation(), *zero)) {
            PPVirtualElement one = Methods::newOne(*ppveMin);
            Scalar::MultiBit::Implementation::MultiBitElement shift(ppveMin->getSizeInBits());
            shift[0] = highBit+1;
            PPVirtualElement shiftElement = Methods::newConstant(*zero, shift);
            shiftElement = Methods::applyAssign(one, Scalar::MultiBit::LeftShiftAssignOperation(),
                  *shiftElement, EvaluationParameters(env));
            if (Methods::applyBoolean(*ppveMax, Scalar::MultiBit::CompareLessUnsignedOperation(),
                  *shiftElement)) {
               PPVirtualElement result;
               if (lowBit > 0) {
                  result = Methods::newTop(*ppveMin);
                  env.mergeVerdictDegradate();
                  TransmitEnvironment applyEnv(env);
                  result = Methods::applyAssign(result, Scalar::MultiBit::ReduceOperation()
                     .setLowBit(0).setHighBit(highBit), applyEnv);
                  result = Methods::newTop(*result);
               }
               else
                  result.setElement(env.getFirstArgument());
               {  MultiBit::Approximate::ExtendOperationElement* extensionResult;
                  {  MultiBit::Approximate::ExtendOperationElement::Init init;
                     init.setBitSize(size);
                     init.setUnsignedExtension(size-1-highBit);
                     extensionResult = new MultiBit::Approximate::ExtendOperationElement(init);
                  };
                  extensionResult->sfstArg() = result;
                  result.absorbElement(extensionResult);
               };
               env.presult() = result;
               return true;
            }
         };
      }
   };

   env.presult() = Methods::newTop(*ppveMin);
   env.mergeVerdictDegradate();
   return true;
}

template <class TypeDecision>
bool
IntegerInterval::applyUnaryAssign(TypeDecision, EvaluationEnvironment& env) {
   EvaluationParameters propagateError = EvaluationParameters(env).propagateErrorStates();
   TransmitEnvironment minEnv(env, propagateError), maxEnv(env, propagateError);
   QueryOperation::OperationEnvironment apply;
   ppveMin->query(TypeDecision::applyOperation(), apply);

   ppveMin->apply(apply.result(), minEnv);
   ppveMax->apply(apply.result(), maxEnv);
   if (minEnv.hasResult())
      ppveMin = minEnv.presult();
   if (maxEnv.hasResult())
      ppveMax = maxEnv.presult();

   // most frequent case: no overflow
   if (!minEnv.hasEvaluationError() && !maxEnv.hasEvaluationError())
      return true;

   if (!TypeDecision::detectOverflow(minEnv, maxEnv)) {
      QueryOperation::OperationEnvironment applyInverse, avoidBoundDomain;
      QueryOperation::DomainEnvironment boundOperation;
      ppveMin->query(TypeDecision::applyInverseOperation(), applyInverse);
      ppveMin->query(TypeDecision::avoidBoundDomainOperation(), avoidBoundDomain);
      ppveMin->query(TypeDecision::boundOperation(), boundOperation);
      // no overflow on the inf bound => overflow on the sup bound
      AssumeCondition(TypeDecision::detectSecondaryOverflow(minEnv, maxEnv));
      ppveMax = Methods::applyAssign(ppveMax, applyInverse.result(), propagateError);
      if (!ppveMax.isValid()) {
         env.presult() = Methods::newTop(*ppveMin);
         return true;
      };
      ppveMax = Methods::constraint(ppveMax, avoidBoundDomain.result(),
            *boundOperation.presult(), *Methods::newTrue(*ppveMin), propagateError);
      ppveMax = Methods::applyAssign(ppveMax, apply.result(), propagateError);
      return true;
   };

   PNT::TPassPointer<VirtualDisjunction, PPAbstractElement> result = Methods::newDisjunction(*ppveMin);

   if (TypeDecision::getAffected(ppveMin, ppveMax)->getApproxKind().isConstant()) {
      if (!TypeDecision::detectSecondaryOverflow(minEnv, maxEnv)) {
         // overflow only on the inf bound
         QueryOperation::DomainEnvironment boundOperation;
         ppveMin->query(TypeDecision::boundOperation(), boundOperation);
         if (!env.doesStopErrorStates())
            result->add(TypeDecision::getAffected(ppveMin, ppveMax)->createSCopy(), env.getInformationKind());
         const VirtualElement& unaffected = *TypeDecision::getUnaffected(ppveMin, ppveMax);
         Init init;
         init.setInterval(boundOperation.presult(), ppveMax).setUnsigned(queryUnsignedField());
         if (env.doesStopErrorStates())
            env.presult() = Methods::newInterval(unaffected, init);
         else {
            result->add(Methods::newInterval(unaffected, init), env.getInformationKind());
            env.presult() = result;
         };
         return true;
      };
      if (TypeDecision::getUnaffected(ppveMin, ppveMax)->getApproxKind().isConstant()) {
         // the interval was constant.
         env.presult() = TypeDecision::getAffected(ppveMin, ppveMax);
         return true;
      };

      // formal general case rebuilt (we have no information about the sup bound)
      if (!env.doesStopErrorStates())
         result->add(TypeDecision::getAffected(ppveMin, ppveMax)->createSCopy(), env.getInformationKind());
      QueryOperation::OperationEnvironment applyInverse, avoidBoundDomain;
      QueryOperation::DomainEnvironment boundOperation;
      ppveMin->query(TypeDecision::applyInverseOperation(), applyInverse);
      ppveMin->query(TypeDecision::avoidBoundDomainOperation(), avoidBoundDomain);
      ppveMin->query(TypeDecision::boundOperation(), boundOperation);
      
      ppveMax = Methods::applyAssign(ppveMax, applyInverse.result(), propagateError);
      ppveMax = Methods::constraint(ppveMax, avoidBoundDomain.result(),
         *boundOperation.presult(), *Methods::newTrue(*ppveMin), propagateError);
      ppveMax = Methods::applyAssign(ppveMax, apply.result(), propagateError);
      Init init;
      init.setInterval(*boundOperation.presult(), ppveMax).setUnsigned(queryUnsignedField());
      if (env.doesStopErrorStates()) {
         env.presult() = Methods::newInterval(*ppveMin, init);
         return true;
      }
      else
         result->add(Methods::newInterval(*ppveMin, init), env.getInformationKind());
   };

   // formal general case rebuilt (we have no information about the inf bound)
   QueryOperation::OperationEnvironment applyInverse, avoidBoundDomain;
   QueryOperation::DomainEnvironment boundInverseOperation, boundOperation;
   ppveMin->query(TypeDecision::applyInverseOperation(), applyInverse);
   ppveMin->query(TypeDecision::avoidBoundDomainOperation(), avoidBoundDomain);
   ppveMin->query(TypeDecision::boundInverseOperation(), boundInverseOperation);
   ppveMin->query(TypeDecision::boundOperation(), boundOperation);
   
   // rebuild the constant before the overflow
   PPVirtualElement copy(*TypeDecision::getAffected(ppveMin, ppveMax));
   copy = Methods::intersect(copy, *boundInverseOperation.presult(), propagateError);
   if (copy.isValid())
      result->add(copy, env.getInformationKind());
   else if (!env.getInformationKind().isSure())
      result->addMay(boundInverseOperation.presult());

   // contraints on the remainder
   PPVirtualElement trueElement = Methods::newTrue(*ppveMin);
   TypeDecision::getAffected(ppveMin, ppveMax) = Methods::applyAssign(TypeDecision::getAffected(ppveMin, ppveMax),
      applyInverse.result(), propagateError);
   TypeDecision::getAffected(ppveMin, ppveMax) = Methods::constraint(TypeDecision::getAffected(ppveMin, ppveMax),
      avoidBoundDomain.result(), *boundOperation.presult(), *trueElement, propagateError);
   TypeDecision::getAffected(ppveMin, ppveMax) = Methods::applyAssign(TypeDecision::getAffected(ppveMin, ppveMax),
      apply.result(), propagateError);

   if (!TypeDecision::detectSecondaryOverflow(minEnv, maxEnv)) {
      const VirtualElement& min = *ppveMin;
      Init init;
      init.setInterval(ppveMin, ppveMax).setUnsigned(queryUnsignedField());
      result->add(Methods::newInterval(min, init), env.getInformationKind());
      env.presult() = result;
      return true;
   };
   if (TypeDecision::getUnaffected(ppveMin, ppveMax)->getApproxKind().isConstant()) {
      env.presult() = TypeDecision::getUnaffected(ppveMin, ppveMax);
      return true;
   };

   // we know nothing about both bounds
   TypeDecision::getUnaffected(ppveMin, ppveMax) = Methods::applyAssign(TypeDecision::getUnaffected(ppveMin, ppveMax),
      applyInverse.result(), propagateError);
   TypeDecision::getUnaffected(ppveMin, ppveMax) = Methods::constraint(TypeDecision::getUnaffected(ppveMin, ppveMax),
      avoidBoundDomain.result(), *boundOperation.presult(), *trueElement, propagateError);
   TypeDecision::getUnaffected(ppveMin, ppveMax) = Methods::applyAssign(TypeDecision::getUnaffected(ppveMin, ppveMax),
      apply.result(), propagateError);
   const VirtualElement& min = *ppveMin;
   Init init;
   init.setInterval(TypeDecision::getAffected(ppveMin, ppveMax),
         TypeDecision::getUnaffected(ppveMin, ppveMax)).setUnsigned(queryUnsignedField());
   result->add(Methods::newInterval(min, init), env.getInformationKind());
   env.presult() = result;
   return true;
}

class PrevAssignDecisionSigned {
  public:
   static IntegerInterval::QueryOperation applyOperation()
      {  return IntegerInterval::QueryOperation().setTypeOperation(IntegerInterval::QueryOperation::TOPrevSignedAssign); }
   static IntegerInterval::QueryOperation applyInverseOperation()
      {  return IntegerInterval::QueryOperation().setTypeOperation(IntegerInterval::QueryOperation::TONextSignedAssign); }
   static bool detectOverflow(const EvaluationEnvironment& minEnv,
         const EvaluationEnvironment& maxEnv) { return minEnv.isNegativeOverflow(); }
   static bool detectSecondaryOverflow(const EvaluationEnvironment& minEnv,
         const EvaluationEnvironment& maxEnv) { return maxEnv.isNegativeOverflow(); }
   static PPVirtualElement& getAffected(PPVirtualElement& min, PPVirtualElement&)
      {  return min; }
   static PPVirtualElement& getUnaffected(PPVirtualElement&, PPVirtualElement& max)
      {  return max; }
   static IntegerInterval::QueryOperation boundOperation()
      {  return IntegerInterval::QueryOperation().setNewMinSigned(); }
   static IntegerInterval::QueryOperation boundInverseOperation()
      {  return IntegerInterval::QueryOperation().setNewMaxSigned(); }
   static IntegerInterval::QueryOperation avoidBoundDomainOperation()
      {  return IntegerInterval::QueryOperation().setTypeOperation(IntegerInterval::QueryOperation::TOCompareGreaterSigned); }
};

class PrevAssignDecisionUnsigned {
  public:
   static IntegerInterval::QueryOperation applyOperation()
      {  return IntegerInterval::QueryOperation().setTypeOperation(IntegerInterval::QueryOperation::TOPrevUnsignedAssign); }
   static IntegerInterval::QueryOperation applyInverseOperation()
      {  return IntegerInterval::QueryOperation().setTypeOperation(IntegerInterval::QueryOperation::TONextUnsignedAssign); }
   static bool detectOverflow(const EvaluationEnvironment& minEnv,
         const EvaluationEnvironment& maxEnv) { return minEnv.isNegativeOverflow(); }
   static bool detectSecondaryOverflow(const EvaluationEnvironment& minEnv,
         const EvaluationEnvironment& maxEnv) { return maxEnv.isNegativeOverflow(); }
   static PPVirtualElement& getAffected(PPVirtualElement& min, PPVirtualElement&)
      {  return min; }
   static PPVirtualElement& getUnaffected(PPVirtualElement&, PPVirtualElement& max)
      {  return max; }
   static IntegerInterval::QueryOperation boundOperation()
      {  return IntegerInterval::QueryOperation().setNewMinUnsigned(); }
   static IntegerInterval::QueryOperation boundInverseOperation()
      {  return IntegerInterval::QueryOperation().setNewMaxUnsigned(); }
   static IntegerInterval::QueryOperation avoidBoundDomainOperation()
      {  return IntegerInterval::QueryOperation().setTypeOperation(IntegerInterval::QueryOperation::TOCompareGreaterUnsigned); }
};

bool
IntegerInterval::applyPrevAssign(const VirtualOperation&, EvaluationEnvironment& env)
   {  return !hasUnsignedField()
         ? applyUnaryAssign(PrevAssignDecisionSigned(), env)
         : applyUnaryAssign(PrevAssignDecisionUnsigned(), env);
   }

class NextAssignDecisionSigned {
  public:
   static IntegerInterval::QueryOperation applyOperation()
      {  return IntegerInterval::QueryOperation().setTypeOperation(IntegerInterval::QueryOperation::TONextSignedAssign); }
   static IntegerInterval::QueryOperation applyInverseOperation()
      {  return IntegerInterval::QueryOperation().setTypeOperation(IntegerInterval::QueryOperation::TOPrevSignedAssign); }
   static bool detectOverflow(const EvaluationEnvironment& minEnv,
         const EvaluationEnvironment& maxEnv) { return maxEnv.isPositiveOverflow(); }
   static bool detectSecondaryOverflow(const EvaluationEnvironment& minEnv,
         const EvaluationEnvironment& maxEnv) { return minEnv.isPositiveOverflow(); }
   static PPVirtualElement& getAffected(PPVirtualElement&, PPVirtualElement& max)
      {  return max; }
   static PPVirtualElement& getUnaffected(PPVirtualElement& min, PPVirtualElement&)
      {  return min; }
   static IntegerInterval::QueryOperation boundOperation()
      {  return IntegerInterval::QueryOperation().setNewMaxSigned(); }
   static IntegerInterval::QueryOperation boundInverseOperation()
      {  return IntegerInterval::QueryOperation().setNewMinSigned(); }
   static IntegerInterval::QueryOperation avoidBoundDomainOperation()
      {  return IntegerInterval::QueryOperation().setTypeOperation(IntegerInterval::QueryOperation::TOCompareLessSigned); }
};

class NextAssignDecisionUnsigned {
  public:
   static IntegerInterval::QueryOperation applyOperation()
      {  return IntegerInterval::QueryOperation().setTypeOperation(IntegerInterval::QueryOperation::TONextUnsignedAssign); }
   static IntegerInterval::QueryOperation applyInverseOperation()
      {  return IntegerInterval::QueryOperation().setTypeOperation(IntegerInterval::QueryOperation::TOPrevUnsignedAssign); }
   static bool detectOverflow(const EvaluationEnvironment& minEnv,
         const EvaluationEnvironment& maxEnv) { return maxEnv.isPositiveOverflow(); }
   static bool detectSecondaryOverflow(const EvaluationEnvironment& minEnv,
         const EvaluationEnvironment& maxEnv) { return minEnv.isPositiveOverflow(); }
   static PPVirtualElement& getAffected(PPVirtualElement&, PPVirtualElement& max)
      {  return max; }
   static PPVirtualElement& getUnaffected(PPVirtualElement& min, PPVirtualElement&)
      {  return min; }
   static IntegerInterval::QueryOperation boundOperation()
      {  return IntegerInterval::QueryOperation().setNewMaxUnsigned(); }
   static IntegerInterval::QueryOperation boundInverseOperation()
      {  return IntegerInterval::QueryOperation().setNewMinUnsigned(); }
   static IntegerInterval::QueryOperation avoidBoundDomainOperation()
      {  return IntegerInterval::QueryOperation().setTypeOperation(IntegerInterval::QueryOperation::TOCompareLessUnsigned); }
};

template <class TypeDecision>
bool
IntegerInterval::applyArithmeticAssign(const TypeDecision& decision, EvaluationEnvironment& env) {
   // [a,b] + [c,d] =   [a+c, min(b, Minint-d-1)+d] with overflow propagation (not the internal operations)
   //                 U [a+c, b+d] without any overflow propagation
   //                 U [max(a, Maxint-c+1)+c, b+d] with overflow propagation (not the internal operations)
   EvaluationParameters stopError = EvaluationParameters(env).stopErrorStates();
   EvaluationParameters surePropagate = EvaluationParameters(env).propagateErrorStates();
   TransmitEnvironment minEnv(env), maxEnv(env);
   PPVirtualElement newMin(*ppveMin), newMax(*ppveMax);
   QueryOperation::OperationEnvironment apply;
   ppveMin->query(TypeDecision::applyOperation(), apply);
   
   newMin = Methods::applyAssign(newMin, apply.result(),
         decision.getInfAfter(), minEnv, surePropagate);
   minEnv.clearEmpty();
   bool isOnlyOverflow = false;
   bool isTop = false;
   if (!newMin.isValid()) {
      if (minEnv.isOnlyPositiveOverflow()) {
         if (env.doesStopErrorStates()) {
            env.setEmpty();
            return true;
         };
         isOnlyOverflow = isTop = true;
         newMin = newSMinDomain();
      }
      else {
         AssumeCondition(minEnv.isNegativeOverflow())
         // if (!(isOnlyOverflow = isTop = minEnv.isOnlyNegativeOverflow()))
            newMin = newSMinDomain();
      };
      // isOnlyOverflow = true;
   };
   if (!isOnlyOverflow) {
      newMax = Methods::applyAssign(newMax, apply.result(),
            decision.getSupAfter(), maxEnv, surePropagate);
      maxEnv.clearEmpty();
      if (!newMax.isValid()) {
         if (maxEnv.isOnlyNegativeOverflow()) {
            if (env.doesStopErrorStates()) {
               env.setEmpty();
               return true;
            };
            isTop = isOnlyOverflow = true;
         }
         else {
            AssumeCondition(maxEnv.isPositiveOverflow())
            // if (!(isOnlyOverflow = maxEnv.isOnlyPositiveOverflow()))
               newMax = newSMaxDomain();
            // isTop = isTop && isOnlyOverflow;
         };
      };
   };

   if (isOnlyOverflow) { // a+c > Maxint or b+d < Minint
      ppveMin = Methods::applyAssign(ppveMin, apply.result(),
            decision.getSource(), EvaluationParameters(env));
      ppveMax = Methods::applyAssign(ppveMax, apply.result(),
            decision.getSource(), EvaluationParameters(env));
      return true;
   };

   // more common case: no overflow or top
   if (env.doesStopErrorStates()
         || (!minEnv.hasEvaluationError() && !maxEnv.hasEvaluationError())
         || isTop) {
      ppveMin = newMin;
      ppveMax = newMax;
      return true;
   };

   // overflow
   EvaluationParameters propagateError(env);
   if (minEnv.isNegativeOverflow()) {
      // [a+c, min(b, Minint-d-1)+d] with overflow propagation (not for the internal operations)
      PNT::TPassPointer<VirtualIntegerInterval, PPAbstractElement> lowPartition;
      QueryOperation::OperationEnvironment applyInverse;
      ppveMin->query(TypeDecision::applyInverseOperation(), applyInverse);
      
      PPVirtualElement minint = newSMinDomain();
      PPVirtualElement lowMin = Methods::apply(*ppveMin, apply.result().setConstWithAssign(),
            decision.getInfAfter(), propagateError);
      PPVirtualElement lowMax;
      if (maxEnv.isNegativeOverflow()) {
         PPVirtualElement highBound = Methods::applyAssign(Methods::applyAssign(minint,
               applyInverse.result(), decision.getSupAfter(), propagateError),
               querySPrevAssignOperation(), propagateError);
         lowMax = Methods::apply(*ppveMax, querySMinOperation(), *highBound, env);
         lowMax = Methods::applyAssign(lowMax, apply.result().clearConstWithAssign(),
            decision.getSource(), propagateError);
      }
      else
         lowMax = newSMaxDomain();
      lowPartition = Methods::newInterval(*ppveMin, Init().setInterval(lowMin, lowMax).setUnsigned(queryUnsignedField()));
      TransmitEnvironment mergeEnv(env);
      if (env.hasResult())
         env.getResult().mergeWith(*lowPartition, mergeEnv);
      else {
         PNT::TPassPointer<IntegerInterval, PPAbstractElement> result(*this);
         result->ppveMin = newMin;
         result->ppveMax = newMax;
         result->mergeWith(*lowPartition, mergeEnv);
         if (!mergeEnv.hasResult())
            mergeEnv.presult() = result;
      };
      if (mergeEnv.hasResult())
         env.presult() = mergeEnv.presult();
   };
   if (maxEnv.isPositiveOverflow()) {
      // [max(a, Maxint-c+1)+c, b+d] with overflow propagation (not for the internal operations)
      QueryOperation::OperationEnvironment applyInverse;
      ppveMin->query(TypeDecision::applyInverseOperation(), applyInverse);

      PNT::TPassPointer<VirtualIntegerInterval, PPAbstractElement> highPartition;
      PPVirtualElement maxint = newSMaxDomain();
      PPVirtualElement highMax = Methods::apply(*ppveMax, apply.result().setConstWithAssign(),
            decision.getSupAfter(), propagateError);
      PPVirtualElement highMin;
      if (minEnv.isPositiveOverflow()) {
         PPVirtualElement lowBound = Methods::applyAssign(Methods::applyAssign(
               maxint, applyInverse.result(), decision.getInfAfter(), stopError),
               querySNextAssignOperation(), stopError);
         highMin = Methods::apply(*ppveMin, querySMaxOperation(), *lowBound, env);
         highMin = Methods::applyAssign(highMin, apply.result().clearConstWithAssign(),
               decision.getSource(), propagateError);
      }
      else
         highMin = newSMinDomain();

      highPartition = Methods::newInterval(*ppveMin, Init().setInterval(highMin, highMax).setUnsigned(queryUnsignedField()));
      TransmitEnvironment mergeEnv(env);
      if (env.hasResult())
         env.getResult().mergeWith(*highPartition, mergeEnv);
      else {
         PNT::TPassPointer<IntegerInterval, PPAbstractElement> result(*this);
         result->ppveMin = newMin;
         result->ppveMax = newMax;
         result->mergeWith(*highPartition, mergeEnv);
         if (!mergeEnv.hasResult())
            mergeEnv.presult() = result;
      };
      if (mergeEnv.hasResult())
         env.presult() = mergeEnv.presult();
   };

   return true;
}

bool
IntegerInterval::applyNextAssign(const VirtualOperation&, EvaluationEnvironment& env)
   {  return !hasUnsignedField()
         ?  applyUnaryAssign(NextAssignDecisionSigned(), env)
         :  applyUnaryAssign(NextAssignDecisionUnsigned(), env);
   }

class PlusAssignDecisionConstantSigned {
  private:
   const VirtualElement& seSource;
   
  public:
   typedef IntegerInterval::QueryOperation QueryOperation;
   PlusAssignDecisionConstantSigned(const VirtualElement& source) : seSource(source) {}

   static QueryOperation applyOperation()
      {  return QueryOperation().setTypeOperation(QueryOperation::TOPlusSignedAssign); }
   static QueryOperation applyInverseOperation()
      {  return QueryOperation().setTypeOperation(QueryOperation::TOMinusSignedAssign); }

   const VirtualElement& getInfAfter() const { return seSource; }
   const VirtualElement& getSupAfter() const { return seSource; }
   const VirtualElement& getSource() const { return seSource; }
};

class PlusAssignDecisionConstantUnsigned {
  private:
   const VirtualElement& seSource;
   
  public:
   typedef IntegerInterval::QueryOperation QueryOperation;
   PlusAssignDecisionConstantUnsigned(const VirtualElement& source) : seSource(source) {}

   static QueryOperation applyOperation()
      {  return QueryOperation().setTypeOperation(QueryOperation::TOPlusUnsignedAssign); }
   static QueryOperation applyInverseOperation()
      {  return QueryOperation().setTypeOperation(QueryOperation::TOMinusUnsignedAssign); }

   const VirtualElement& getInfAfter() const { return seSource; }
   const VirtualElement& getSupAfter() const { return seSource; }
   const VirtualElement& getSource() const { return seSource; }
};

bool
IntegerInterval::applyPlusAssignConstant(const VirtualOperation&,
      const VirtualElement& source, EvaluationEnvironment& env)
   {  return !hasUnsignedField()
         ? applyArithmeticAssign(PlusAssignDecisionConstantSigned(source), env)
         : applyArithmeticAssign(PlusAssignDecisionConstantUnsigned(source), env);
   }

class MinusAssignDecisionConstantSigned {
  private:
   const VirtualElement& seSource;
   
  public:
   typedef IntegerInterval::QueryOperation QueryOperation;
   MinusAssignDecisionConstantSigned(const VirtualElement& source) : seSource(source) {}
   static QueryOperation applyOperation()
      {  return QueryOperation().setTypeOperation(QueryOperation::TOMinusSignedAssign); }
   static QueryOperation applyInverseOperation()
      {  return QueryOperation().setTypeOperation(QueryOperation::TOPlusSignedAssign); }

   const VirtualElement& getInfAfter() const { return seSource; }
   const VirtualElement& getSupAfter() const { return seSource; }
   const VirtualElement& getSource() const { return seSource; }
};

class MinusAssignDecisionConstantUnsigned {
  private:
   const VirtualElement& seSource;
   
  public:
   typedef IntegerInterval::QueryOperation QueryOperation;
   MinusAssignDecisionConstantUnsigned(const VirtualElement& source) : seSource(source) {}
   static QueryOperation applyOperation()
      {  return QueryOperation().setTypeOperation(QueryOperation::TOMinusUnsignedAssign); }
   static QueryOperation applyInverseOperation()
      {  return QueryOperation().setTypeOperation(QueryOperation::TOPlusUnsignedAssign); }

   const VirtualElement& getInfAfter() const { return seSource; }
   const VirtualElement& getSupAfter() const { return seSource; }
   const VirtualElement& getSource() const { return seSource; }
};

bool
IntegerInterval::applyMinusAssignConstant(const VirtualOperation&,
      const VirtualElement& source, EvaluationEnvironment& env)
   {  return !hasUnsignedField()
         ? applyArithmeticAssign(MinusAssignDecisionConstantSigned(source), env)
         : applyArithmeticAssign(MinusAssignDecisionConstantUnsigned(source), env);
   }

class PlusAssignDecisionIntervalSigned {
  private:
   const IntegerInterval& iieSource;
   
  public:
   PlusAssignDecisionIntervalSigned(const IntegerInterval& source) : iieSource(source) {}
   static IntegerInterval::QueryOperation applyOperation()
      {  return IntegerInterval::QueryOperation().setTypeOperation(IntegerInterval::QueryOperation::TOPlusSignedAssign); }
   static IntegerInterval::QueryOperation applyInverseOperation()
      {  return IntegerInterval::QueryOperation().setTypeOperation(IntegerInterval::QueryOperation::TOMinusSignedAssign); }

   const VirtualElement& getInfAfter() const { return iieSource.getMin(); }
   const VirtualElement& getSupAfter() const { return iieSource.getMax(); }

   const IntegerInterval& getSource() const { return iieSource; }
};

class PlusAssignDecisionIntervalUnsigned {
  private:
   const IntegerInterval& iieSource;
   
  public:
   PlusAssignDecisionIntervalUnsigned(const IntegerInterval& source) : iieSource(source) {}
   static IntegerInterval::QueryOperation applyOperation()
      {  return IntegerInterval::QueryOperation().setTypeOperation(IntegerInterval::QueryOperation::TOPlusUnsignedAssign); }
   static IntegerInterval::QueryOperation applyInverseOperation()
      {  return IntegerInterval::QueryOperation().setTypeOperation(IntegerInterval::QueryOperation::TOMinusUnsignedAssign); }

   const VirtualElement& getInfAfter() const { return iieSource.getMin(); }
   const VirtualElement& getSupAfter() const { return iieSource.getMax(); }

   const IntegerInterval& getSource() const { return iieSource; }
};

bool
IntegerInterval::applyPlusAssignInterval(const VirtualOperation& operation,
      const IntegerInterval& source, EvaluationEnvironment& env) {
   if (queryUnsignedField() != source.queryUnsignedField()) {
      PNT::TPassPointer<VirtualDisjunction, PPAbstractElement> copy = changeSign(env);
      if (copy.isValid()) {
         TransmitEnvironment applyEnv(env);
         applyEnv.setFirstArgument(source);
         assume(copy->apply(operation, applyEnv));
         env.presult() = applyEnv.hasResult() ? applyEnv.presult() : (PPAbstractElement) copy;
         return true;
      };
   };
   return !hasUnsignedField()
      ? applyArithmeticAssign(PlusAssignDecisionIntervalSigned(source), env)
      : applyArithmeticAssign(PlusAssignDecisionIntervalUnsigned(source), env);
}

class MinusAssignDecisionIntervalSigned {
  private:
   const IntegerInterval& iieSource;
   
  public:
   MinusAssignDecisionIntervalSigned(const IntegerInterval& source) : iieSource(source) {}
   static IntegerInterval::QueryOperation applyOperation()
      {  return IntegerInterval::QueryOperation().setTypeOperation(IntegerInterval::QueryOperation::TOMinusSignedAssign); }
   static IntegerInterval::QueryOperation applyInverseOperation()
      {  return IntegerInterval::QueryOperation().setTypeOperation(IntegerInterval::QueryOperation::TOPlusSignedAssign); }

   const VirtualElement& getInfAfter() const { return iieSource.getMax(); }
   const VirtualElement& getSupAfter() const { return iieSource.getMin(); }

   const IntegerInterval& getSource() const { return iieSource; }
};

class MinusAssignDecisionIntervalUnsigned {
  private:
   const IntegerInterval& iieSource;
   
  public:
   MinusAssignDecisionIntervalUnsigned(const IntegerInterval& source) : iieSource(source) {}
   static IntegerInterval::QueryOperation applyOperation()
      {  return IntegerInterval::QueryOperation().setTypeOperation(IntegerInterval::QueryOperation::TOMinusUnsignedAssign); }
   static IntegerInterval::QueryOperation applyInverseOperation()
      {  return IntegerInterval::QueryOperation().setTypeOperation(IntegerInterval::QueryOperation::TOPlusUnsignedAssign); }

   const VirtualElement& getInfAfter() const { return iieSource.getMax(); }
   const VirtualElement& getSupAfter() const { return iieSource.getMin(); }

   const IntegerInterval& getSource() const { return iieSource; }
};

bool
IntegerInterval::applyMinusAssignInterval(const VirtualOperation& operation,
      const IntegerInterval& source, EvaluationEnvironment& env) {
   if (queryUnsignedField() != source.queryUnsignedField()) {
      PNT::TPassPointer<VirtualDisjunction, PPAbstractElement> copy = changeSign(env);
      if (copy.isValid()) {
         TransmitEnvironment applyEnv(env);
         applyEnv.setFirstArgument(source);
         assume(copy->apply(operation, applyEnv));
         env.presult() = applyEnv.hasResult() ? applyEnv.presult() : (PPAbstractElement) copy;
         return true;
      };
   };
   return !hasUnsignedField()
      ? applyArithmeticAssign(MinusAssignDecisionIntervalSigned(source), env)
      : applyArithmeticAssign(MinusAssignDecisionIntervalUnsigned(source), env);
}

#define DefineSetTopOnError                                                                      \
   if (minEnv.hasEvaluationError() || maxEnv.hasEvaluationError()) {                             \
      if (result.isValid()) {                                                                    \
         result->addMay(Methods::newTop(*ppveMin));                                              \
         env.presult() = result;                                                                 \
      }                                                                                          \
      else                                                                                       \
         env.presult() = Methods::newTop(*ppveMin);                                              \
      return true;                                                                               \
   };

bool
IntegerInterval::applyTimesAssignConstant(const VirtualOperation& operation,
      const VirtualElement& source, EvaluationEnvironment& env) {
   if ((bool) queryUnsignedField() != isUnsignedTimesOperation(operation)) {
      PNT::TPassPointer<VirtualDisjunction, PPAbstractElement> conversion = changeSign(env);
      if (conversion.isValid()) {
         TransmitEnvironment applyEnv(env);
         applyEnv.setFirstArgument(source);
         assume(conversion->apply(operation, applyEnv));
         env.presult() = applyEnv.hasResult() ? applyEnv.presult() : conversion;
         return true;
      };
   };
   env.mergeVerdictDegradate();
   EvaluationEnvironment::LatticeCreation creation = env.getLatticeCreation();
   if (creation.isTop() || creation.isShareTop()) {
      env.presult() = Details::IntOperationElement::Methods::newTop(*this);
      return true;
   };

   PNT::TPassPointer<VirtualDisjunction, PPAbstractElement>
      result = applySureCase(QueryOperation().setTimes(), source, env);
   if (env.getInformationKind().isSure()) {
      env.presult() = result;
      return true;
   };

   QueryOperation::CompareSpecialEnvironment sourceCompare = querySCompareSpecial(source);

   EvaluationParameters mayPropagateErrorStates(EPMayPropagateErrorStates);
   mayPropagateErrorStates.setApproximatePartFrom(env);
   TransmitEnvironment minEnv(env, mayPropagateErrorStates), maxEnv(env, mayPropagateErrorStates);
   auto timesAssignOperation = querySTimesAssignOperation();
   ppveMin = Methods::applyAssign(ppveMin, timesAssignOperation, source, minEnv);
   if (ppveMin.isValid())
      ppveMax = Methods::applyAssign(ppveMax, timesAssignOperation, source, maxEnv);
   else
      ppveMin.setElement(*ppveMax);
   DefineSetTopOnError

   if (sourceCompare.isLessOrEqualZero()) {
      PPVirtualElement temp = ppveMin;
      ppveMin = ppveMax;
      ppveMax = temp;
   }
   else if (sourceCompare.mayBeLessZero()) { // separation into two intervals
      PPVirtualElement mergeResult(*this);
      PPVirtualElement temp = ppveMin;
      ppveMin = ppveMax;
      ppveMax = temp;
      EvaluationParameters mayStopErrorStates(EPMayStopErrorStates);
      mayStopErrorStates.setApproximatePartFrom(env);
      mergeResult = Methods::merge(mergeResult, *this, mayStopErrorStates);
      env.presult() = (result.isValid()) ? Methods::merge(env.presult(), *mergeResult, mayStopErrorStates) : mergeResult;
   };
   if (result.isValid()) {
      const VirtualElement& min = *ppveMin;
      result->addMay(Methods::newInterval(min, Init().setInterval(ppveMin, ppveMax).setUnsigned(queryUnsignedField())));
      env.presult() = result;
   };
   return true;
}

bool
IntegerInterval::applyTimesAssignInterval(const VirtualOperation& operation,
      const IntegerInterval& source, EvaluationEnvironment& env) {
   if (queryUnsignedField() != source.queryUnsignedField()) {
      PNT::TPassPointer<VirtualDisjunction, PPAbstractElement> copy = changeSign(env);
      if (copy.isValid()) {
         TransmitEnvironment applyEnv(env);
         applyEnv.setFirstArgument(source);
         assume(copy->apply(operation, applyEnv));
         env.presult() = applyEnv.hasResult() ? applyEnv.presult() : (PPAbstractElement) copy;
         return true;
      };
   };

   env.mergeVerdictDegradate(); // [a,b]*[c,d]
   EvaluationEnvironment::LatticeCreation creation = env.getLatticeCreation();
   if (creation.isTop() || creation.isShareTop()) {
      env.presult() = Details::IntOperationElement::Methods::newTop(*this);
      return true;
   };

   PNT::TPassPointer<VirtualDisjunction, PPAbstractElement> result
      = applySureCaseInterval(QueryOperation().setTimes(), source, env);
   if (env.getInformationKind().isSure()) {
      env.setEmpty();
      return true;
   };

   EvaluationParameters mayPropagateErrorStates(EPMayPropagateErrorStates);
   mayPropagateErrorStates.setApproximatePartFrom(env);
   TransmitEnvironment minEnv(env, mayPropagateErrorStates), maxEnv(env, mayPropagateErrorStates);
   QueryOperation::CompareSpecialEnvironment
      minCompare = querySCompareSpecial(*ppveMin),
      maxCompare = querySCompareSpecial(*ppveMax),
      minSourceCompare = querySCompareSpecial(*source.ppveMin),
      maxSourceCompare = querySCompareSpecial(*source.ppveMax);

   EvaluationParameters mayStopErrorStates(EPMayStopErrorStates);
   mayStopErrorStates.setApproximatePartFrom(env);
   if (minCompare.isGreaterOrEqualZero()) {
      if (minSourceCompare.isGreaterOrEqualZero()) { // a >= 0 and c >= 0 -> [a*c, b*d]
         auto timesAssignOperation = querySTimesAssignOperation();
         ppveMin = Methods::applyAssign(ppveMin, timesAssignOperation, *source.ppveMin, minEnv);
         ppveMax = Methods::applyAssign(ppveMax, timesAssignOperation, *source.ppveMax, maxEnv);
         DefineSetTopOnError
      }
      else if (maxSourceCompare.isLessOrEqualZero()) { // a >= 0 and c <= 0 -> [b*c, a*d]
         PPVirtualElement temp;
         auto timesOperation = querySTimesAssignOperation();
         temp = Methods::applyAssign(ppveMin, timesOperation, *source.ppveMax, minEnv);
         ppveMin = Methods::applyAssign(ppveMax, timesOperation, *source.ppveMin, maxEnv);
         ppveMax = temp;
         DefineSetTopOnError
      }
      else { // restriction of source in two intervals [c,-1] U [0,d]
         PPVirtualElement infResult = Methods::newInterval(*ppveMin,
            Init().setInterval(Methods::apply(*ppveMax, querySTimesOperation(), *source.ppveMin, minEnv),
               Methods::newZero(*ppveMin)).setUnsigned(queryUnsignedField()));
         PPVirtualElement supResult = Methods::newInterval(*ppveMin, Init().setInterval(Methods::newZero(*ppveMin),
               Methods::apply(*ppveMax, querySTimesOperation(), *source.ppveMax, maxEnv)).setUnsigned(queryUnsignedField()));
         DefineSetTopOnError
         infResult = Methods::merge(infResult, *supResult, mayStopErrorStates);
         if (result.isValid()) {
            result->addMay(infResult);
            env.presult() = result;
         }
         else
            env.presult() = infResult;
         return true;
      };
      if (result.isValid()) {
         const VirtualElement& min = *ppveMin;
         result->addMay(Methods::newInterval(min, Init().setInterval(ppveMin, ppveMax).setUnsigned(queryUnsignedField())));
         env.presult() = result;
      };
      return true;
   };

   if (maxCompare.isLessOrEqualZero()) {
      if (minSourceCompare.isGreaterOrEqualZero()) { // b <= 0 and c >= 0 -> [a*d, b*c]
         auto timesAssignOperation = querySTimesAssignOperation();
         ppveMin = Methods::applyAssign(ppveMin, timesAssignOperation, *source.ppveMax, minEnv);
         ppveMax = Methods::applyAssign(ppveMax, timesAssignOperation, *source.ppveMin, maxEnv);
         DefineSetTopOnError
      }
      else if (maxSourceCompare.isLessOrEqualZero()) { // b <= 0 and d <= 0 -> [d*b, c*a]
         auto timesAssignOperation = querySTimesAssignOperation();
         PPVirtualElement temp(ppveMin);
         ppveMin = Methods::applyAssign(ppveMax, timesAssignOperation, *source.ppveMax, minEnv);
         ppveMax = Methods::applyAssign(temp, timesAssignOperation, *source.ppveMin, maxEnv);
         DefineSetTopOnError
      }
      else { // restriction of source in two intervals [c,-1] U [0,d]
         PPVirtualElement infResult = Methods::newInterval(*ppveMin,
            Init().setInterval(Methods::apply(*ppveMin, querySTimesOperation(), *source.ppveMax, minEnv),
               Methods::newZero(*ppveMin)).setUnsigned(queryUnsignedField()));
         PPVirtualElement supResult = Methods::newInterval(*ppveMin, Init().setInterval(Methods::newZero(*ppveMin),
               Methods::apply(*ppveMin, querySTimesOperation(), *source.ppveMin, maxEnv)).setUnsigned(queryUnsignedField()));
         DefineSetTopOnError
         infResult = Methods::merge(infResult, *supResult, mayStopErrorStates);
         if (result.isValid()) {
            result->addMay(infResult);
            env.presult() = result;
         }
         else
            env.presult() = infResult;
         return true;
      };
      if (result.isValid()) {
         const VirtualElement& min = *ppveMin;
         result->addMay(Methods::newInterval(min, Init().setInterval(ppveMin, ppveMax).setUnsigned(queryUnsignedField())));
         env.presult() = result;
      };
      return true;
   };

   // a <= 0 and b >= 0, [a,0] U [0,b] as may
   if (minSourceCompare.isGreaterOrEqualZero()) { // c >= 0 -> [a*d, 0] U [0, b*d]
      PPVirtualElement infResult = Methods::newInterval(*ppveMin,
         Init().setInterval(Methods::apply(*ppveMin, querySTimesOperation(), *source.ppveMax, minEnv),
            Methods::newZero(*ppveMin)).setUnsigned(queryUnsignedField()));
      PPVirtualElement supResult = Methods::newInterval(*ppveMin, Init().setInterval(Methods::newZero(*ppveMin),
            Methods::apply(*ppveMax, querySTimesOperation(), *source.ppveMax, maxEnv)).setUnsigned(queryUnsignedField()));
      DefineSetTopOnError
      infResult = Methods::merge(infResult, *supResult, mayStopErrorStates);
      if (result.isValid()) {
         result->addMay(infResult);
         env.presult() = result;
      }
      else
         env.presult() = infResult;
      return true;
   }
   else if (maxSourceCompare.isLessOrEqualZero()) { // d <= 0 -> [b*c, 0] U [0, a*c]
      PPVirtualElement infResult = Methods::newInterval(*ppveMin,
         Init().setInterval(Methods::apply(*ppveMax, querySTimesOperation(), *source.ppveMin, minEnv),
            Methods::newZero(*ppveMin)).setUnsigned(queryUnsignedField()));
      PPVirtualElement supResult = Methods::newInterval(*ppveMin, Init().setInterval(Methods::newZero(*ppveMin),
            Methods::apply(*ppveMin, querySTimesOperation(), *source.ppveMin, maxEnv)).setUnsigned(queryUnsignedField()));
      DefineSetTopOnError
      infResult = Methods::merge(infResult, *supResult, mayStopErrorStates);
      if (result.isValid()) {
         result->addMay(infResult);
         env.presult() = result;
      }
      else
         env.presult() = infResult;
      return true;
   };

   // restriction of source in 4 intervals ([a,0] U [0,b]) * ([c,0] U [0,d])
   // = [b*c,0] U [a*d,0] U [0,a*c] U [0,b*d]
   PPVirtualElement infFstResult = Methods::newInterval(*ppveMin,
      Init().setInterval(Methods::apply(*ppveMin, querySTimesOperation(), *source.ppveMax, minEnv),
         Methods::newZero(*ppveMin)).setUnsigned(queryUnsignedField()));
   PPVirtualElement infSndResult = Methods::newInterval(*ppveMin,
      Init().setInterval(Methods::apply(*ppveMax, querySTimesOperation(), *source.ppveMin, minEnv),
         Methods::newZero(*ppveMin)).setUnsigned(queryUnsignedField()));
   PPVirtualElement supFstResult = Methods::newInterval(*ppveMin, Init().setInterval(Methods::newZero(*ppveMin),
         Methods::apply(*ppveMin, querySTimesOperation(), *source.ppveMin, maxEnv)).setUnsigned(queryUnsignedField()));
   PPVirtualElement supSndResult = Methods::newInterval(*ppveMin, Init().setInterval(Methods::newZero(*ppveMin),
         Methods::apply(*ppveMax, querySTimesOperation(), *source.ppveMax, maxEnv)).setUnsigned(queryUnsignedField()));
   DefineSetTopOnError
   infFstResult = Methods::merge(infFstResult, *infSndResult, mayStopErrorStates);
   supFstResult = Methods::merge(supFstResult, *supSndResult, mayStopErrorStates);
   infFstResult = Methods::merge(infFstResult, *supFstResult, mayStopErrorStates);
   if (result.isValid()) {
      result->addMay(infFstResult);
      env.presult() = result;
   }
   else
      env.presult() = infFstResult;

   return true;
}

bool
IntegerInterval::applyDivideAssignConstant(const VirtualOperation&,
      const VirtualElement& source, EvaluationEnvironment& env) {
   QueryOperation::CompareSpecialEnvironment sourceCompare = querySCompareSpecial(source);
   if (sourceCompare.isZero()) {
      env.setSureDivisionByZero();
      env.setEmpty();
      return true;
   };
   if (sourceCompare.isGreaterZero()) {
      auto divideAssignOperation = querySDivideAssignOperation();
      ppveMin = Methods::applyAssign(ppveMin, divideAssignOperation, source, env);
      ppveMax = Methods::applyAssign(ppveMax, divideAssignOperation, source, env);
      return true;
   };
   TransmitEnvironment copyEnv(env);
   if (sourceCompare.isLessOrEqualZero()) {
      PPVirtualElement temp = Methods::apply(*ppveMin, querySDivideOperation(), source, copyEnv);
      if (!copyEnv.isPositiveOverflow()) {
         ppveMin = Methods::applyAssign(ppveMax, querySDivideAssignOperation(), source, copyEnv);
         ppveMax = temp;
         return true;
      };
      // handle [Minint, 0]/-1.
      bool doesAddSureMinint = ((env.getInformationKind().isMay()) || copyEnv.isSurePositiveOverflow());
      PPVirtualElement thisCopy(*this);
      PPVirtualElement minint = newSMinDomain();

      PPVirtualElement addMinint;
      if (doesAddSureMinint)
         addMinint.fullAssign(minint);
      else
         addMinint = Methods::constraint(PPVirtualElement(*this), querySCompareEqualOperation(), *minint,
           *Methods::newTrue(source), EvaluationParameters(env));

      thisCopy = Methods::constraint(thisCopy, querySCompareGreaterOperation(), *minint,
           *Methods::newTrue(source), EvaluationParameters(env));
      auto divideAssignOperation = querySDivideAssignOperation();
      temp = Methods::applyAssign(ppveMin, divideAssignOperation, source, copyEnv);
      ppveMin = Methods::applyAssign(ppveMax, divideAssignOperation, source, copyEnv);
      ppveMax = temp;
      PNT::TPassPointer<VirtualDisjunction, PPAbstractElement> result = Methods::newDisjunction(source);
      result->add(Methods::newInterval(source, Init().setInterval(ppveMin, ppveMax).setUnsigned(queryUnsignedField())), env.getInformationKind());
      result->add(addMinint, env.getInformationKind());
      env.presult() = result;
   }
   else { // we perform the union of both results
      PNT::TPassPointer<IntegerInterval, PPAbstractElement> thisCopy(*this);
      auto divideAssignOperation = querySDivideAssignOperation();
      ppveMin = Methods::applyAssign(ppveMin, divideAssignOperation, source, env);
      ppveMax = Methods::applyAssign(ppveMax, divideAssignOperation, source, env);

      PPVirtualElement temp = Methods::apply(*thisCopy->ppveMin, querySDivideOperation(), source, copyEnv);
      if (!copyEnv.isPositiveOverflow()) {
         thisCopy->ppveMin = Methods::applyAssign(thisCopy->ppveMax, divideAssignOperation, source, copyEnv);
         thisCopy->ppveMax = temp;

         TransmitEnvironment mergeEnv(env);
         mergeWithInterval(*thisCopy, mergeEnv);
         if (mergeEnv.hasResult())
            env.presult() = mergeEnv.presult();
         return true;
      };
      // handle [Minint, 0]/-1.
      bool doesAddSureMinint = ((env.getInformationKind().isMay()) || copyEnv.isSurePositiveOverflow());
      // PPVirtualElement thisSimplified(*thisCopy);
      PPVirtualElement minint = newSMinDomain();

      PPVirtualElement addMinint;
      if (doesAddSureMinint)
         addMinint.fullAssign(minint);
      else
         addMinint = Methods::constraint(PPVirtualElement(*this), querySCompareEqualOperation(), *minint,
           *Methods::newTrue(source), EvaluationParameters(env));

      // thisSimplified = Methods::constraint(thisSimplified, querySCompareGreaterOperation(), *minint,
      //      *Methods::newTrue(source), EvaluationParameters(env));
      temp = Methods::applyAssign(thisCopy->ppveMin, querySDivideAssignOperation(), source, copyEnv);
      thisCopy->ppveMin = Methods::applyAssign(thisCopy->ppveMax, querySDivideAssignOperation(), source, copyEnv);
      thisCopy->ppveMax = temp;

      TransmitEnvironment mergeEnv(env);
      mergeWith(*thisCopy, mergeEnv);
      if (mergeEnv.hasResult())
         env.presult() = mergeEnv.presult();

      if (env.hasResult())
         env.presult() = Methods::merge(env.presult(), *addMinint, env);
      else {
         mergeWith(*addMinint, mergeEnv);
         if (mergeEnv.hasResult())
            env.presult() = mergeEnv.presult();
      };
   };
   return true;
}

bool
IntegerInterval::applyDivideAssignInterval(const VirtualOperation& operation,
      const IntegerInterval& source, EvaluationEnvironment& env) {
   if (queryUnsignedField() != source.queryUnsignedField()) {
      PNT::TPassPointer<VirtualDisjunction, PPAbstractElement> copy = changeSign(env);
      if (copy.isValid()) {
         TransmitEnvironment applyEnv(env);
         applyEnv.setFirstArgument(source);
         assume(copy->apply(operation, applyEnv));
         env.presult() = applyEnv.hasResult() ? applyEnv.presult() : (PPAbstractElement) copy;
         return true;
      };
   };
   TransmitEnvironment minEnv(env), maxEnv(env); // [a,b]/[c,d]
   QueryOperation::CompareSpecialEnvironment
      minCompare = querySCompareSpecial(*ppveMin),
      maxCompare = querySCompareSpecial(*ppveMax),
      minSourceCompare = querySCompareSpecial(*source.ppveMin),
      maxSourceCompare = querySCompareSpecial(*source.ppveMax);

   bool isExact = Methods::applyBoolean(*source.ppveMin, querySCompareGreaterOrEqualOperation(),
         *source.ppveMax);
   PNT::TPassPointer<Disjunction, PPAbstractElement> disjunctionNotExact;

   if (minSourceCompare.isGreaterOrEqualZero()) { // c >= 0
      PPVirtualElement minSourceCopy;
      const VirtualElement* sourceMin = source.ppveMin.key();
      if (minSourceCompare.isZero()) { // c = 0
         env.setDivisionByZero();
         if (!env.getInformationKind().isMay())
            env.setSureDivisionByZero();
         minSourceCopy = Methods::newOne(*ppveMin); // -> c = 1
         sourceMin = minSourceCopy.key();
      }
      else if (minSourceCompare.mayBeZero()) { // c >= 0, mais indéfini
         env.setDivisionByZero();
         minSourceCopy = Methods::apply(*source.ppveMin, querySMaxOperation(),
               *Methods::newOne(*ppveMin), env);
         sourceMin = minSourceCopy.key();
      };

      bool isDisjunctionExact = true;
      EvaluationParameters mayStopErrorStates(EPMayStopErrorStates);
      mayStopErrorStates.setApproximatePartFrom(env);
      if (!isExact && !maxCompare.isLessOrEqualZero()) { // b/(c+1) >= a/c-1
         PPVirtualElement minOnSourceMinMinusOne = Methods::applyAssign(
            PPVirtualElement(*ppveMin), querySDivideAssignOperation(),
            *sourceMin, mayStopErrorStates);
         if (minOnSourceMinMinusOne.isValid())
            minOnSourceMinMinusOne = Methods::applyAssign(minOnSourceMinMinusOne,
                  querySPrevAssignOperation(), mayStopErrorStates);
         if (minOnSourceMinMinusOne.isValid()) {
            PPVirtualElement maxOnNextSourceMin = Methods::applyAssign(
               PPVirtualElement(*sourceMin), querySNextAssignOperation(), mayStopErrorStates);
            if (maxOnNextSourceMin.isValid())
               maxOnNextSourceMin = Methods::applyAssign(
                  PPVirtualElement(*ppveMax), querySDivideAssignOperation(),
                  *maxOnNextSourceMin, mayStopErrorStates);
            if (maxOnNextSourceMin.isValid())
               isDisjunctionExact = Methods::applyBoolean(*maxOnNextSourceMin,
                  querySCompareGreaterOrEqualOperation(), *minOnSourceMinMinusOne);
            else
               isDisjunctionExact = false;
         };
      };
      if (!isExact && isDisjunctionExact && !minCompare.isGreaterOrEqualZero()) {
         // a/(c+1) <= b/c+1
         PPVirtualElement maxOnSourceMinPlusOne = Methods::applyAssign(
            PPVirtualElement(*ppveMax), querySDivideAssignOperation(),
            *sourceMin, mayStopErrorStates);
         if (maxOnSourceMinPlusOne.isValid())
            maxOnSourceMinPlusOne = Methods::applyAssign(maxOnSourceMinPlusOne,
               querySNextAssignOperation(), mayStopErrorStates);
         if (!maxOnSourceMinPlusOne.isValid())
            isDisjunctionExact = false;
         else {
            PPVirtualElement minOnNextSourceMin = Methods::applyAssign(
               PPVirtualElement(*sourceMin), querySNextAssignOperation(), mayStopErrorStates);
            if (minOnNextSourceMin.isValid())
               minOnNextSourceMin = Methods::applyAssign(PPVirtualElement(*ppveMin),
                  querySDivideAssignOperation(), *minOnNextSourceMin, mayStopErrorStates);
            if (minOnNextSourceMin.isValid())
               isDisjunctionExact = Methods::applyBoolean(*minOnNextSourceMin,
                  querySCompareLessOrEqualOperation(), *maxOnSourceMinPlusOne);
         };
      };
      if ((isExact == isDisjunctionExact) == false) {
         env.mergeVerdictDegradate();
         disjunctionNotExact = applySureCaseInterval(QueryOperation().setDivide(), source, env);
         if (env.getInformationKind().isSure()) {
            env.presult() = disjunctionNotExact;
            return true;
         };
      };

      if (minCompare.isGreaterOrEqualZero()) { // a >= 0 et c > 0 -> [a/d, b/c]
         auto divideAssignOperation = querySDivideAssignOperation();
         ppveMin = Methods::applyAssign(ppveMin, divideAssignOperation, *source.ppveMax, minEnv);
         ppveMax = Methods::applyAssign(ppveMax, divideAssignOperation, *sourceMin, maxEnv);
      }
      else if (maxCompare.isLessOrEqualZero()) { // b <= 0 et c > 0 -> [a/c, b/d]
         auto divideAssignOperation = querySDivideAssignOperation();
         ppveMin = Methods::applyAssign(ppveMin, divideAssignOperation, *sourceMin, minEnv);
         ppveMax = Methods::applyAssign(ppveMax, divideAssignOperation, *source.ppveMax, maxEnv);
      }
      else { // [a,0] U [0,b] et c > 0 -> [a/c, b/c]
         // merge the different cases -> U [a/d, b/c], if a may > 0
         //                              U [a/c, b/d], if b may < 0
         PNT::TPassPointer<IntegerInterval, PPAbstractElement> negative, positive;
         if (minCompare.mayBeGreaterZero())
            negative = Methods::newInterval(*ppveMin,
               Init().setInterval(Methods::apply(*ppveMin, querySDivideOperation(), *source.ppveMax, minEnv),
                  Methods::apply(*ppveMax, querySDivideOperation(), *sourceMin, maxEnv)).setUnsigned(queryUnsignedField()));
         if (maxCompare.mayBeLessZero())
            positive = Methods::newInterval(*ppveMin,
               Init().setInterval(Methods::apply(*ppveMin, querySDivideOperation(), *sourceMin, minEnv),
                  Methods::apply(*ppveMax, querySDivideOperation(), *source.ppveMax, maxEnv)).setUnsigned(queryUnsignedField()));
         auto divideAssignOperation = querySDivideAssignOperation();
         ppveMin = Methods::applyAssign(ppveMin, divideAssignOperation, *sourceMin, minEnv);
         ppveMax = Methods::applyAssign(ppveMax, divideAssignOperation, *sourceMin, maxEnv);
         if (negative.isValid() || positive.isValid()) {
            TransmitEnvironment mergeEnv(env);
            if (negative.isValid()) {
               mergeWithInterval(*negative, mergeEnv);
               if (mergeEnv.hasResult())
                  env.presult() = mergeEnv.presult();
               mergeEnv.merge().clear();
            };
            if (positive.isValid()) {
               if (env.hasResult())
                  env.getResult().mergeWith(*positive, mergeEnv);
               else
                  mergeWithInterval(*positive, mergeEnv);
               if (mergeEnv.hasResult())
                  env.presult() = mergeEnv.presult();
               mergeEnv.merge().clear();
            };
         };
         return true;
      };
   }
   else if (maxSourceCompare.isLessOrEqualZero()) { // d <= 0
      PPVirtualElement maxSourceCopy;
      const VirtualElement* sourceMax = source.ppveMax.key();

      bool doesOverflowMinint = false, mayOverflowMinint = false;
      if (maxSourceCompare.isZero()) { // d = 0
         env.setDivisionByZero();
         if (!env.getInformationKind().isMay())
            env.setSureDivisionByZero();
         maxSourceCopy = Methods::newMinusOne(*ppveMin);
         sourceMax = maxSourceCopy.key();
         if ((mayOverflowMinint = minCompare.mayBeMinint()) != false)
            doesOverflowMinint = minCompare.isMinint();
      }
      else if (maxSourceCompare.mayBeZero()) {
         env.setDivisionByZero();
         maxSourceCopy = Methods::apply(*source.ppveMax, querySMinOperation(),
               *Methods::newMinusOne(*ppveMin), env);
         sourceMax = maxSourceCopy.key();
         mayOverflowMinint = minCompare.mayBeMinint();
      }
      else if ((mayOverflowMinint = (minCompare.mayBeMinint()
                  && maxSourceCompare.mayBeMinusOne())) != false)
         doesOverflowMinint = (minCompare.isMinint() && maxSourceCompare.isMinusOne());

      PPVirtualElement overflowMinint;
      if (mayOverflowMinint) {
         PPVirtualElement minusTwo = Methods::newMinusOne(*ppveMin);
         EvaluationParameters exactPropagateErrorStates(EPExactPropagateErrorStates);
         exactPropagateErrorStates.setApproximatePartFrom(env);
         minusTwo = Methods::applyAssign(minusTwo, querySPrevAssignOperation(), exactPropagateErrorStates);
         if (doesOverflowMinint) { // [Minint,b]/[c,-1] = [Minint+1,b]/[c,-1] U [Minint/c,Minint/-2] U Minint
            overflowMinint = Methods::newInterval(*ppveMin,
               Init().setInterval(Methods::apply(*ppveMin, querySDivideOperation(), *source.ppveMin, env),
                  Methods::apply(*ppveMin, querySDivideOperation(), *minusTwo, env)).setUnsigned(queryUnsignedField()));
            overflowMinint = Methods::merge(overflowMinint, *ppveMin, env);
            auto nextAssignOperation = querySNextAssignOperation();
            ppveMin = Methods::applyAssign(ppveMin, nextAssignOperation, EvaluationParameters(env));
         }
         else { // [a/min(d,-1),Minint] U [a/c,a/min(d,-2)] U [max(a,minint+1),b]/[c,d]
            overflowMinint = Methods::newInterval(*ppveMin,
               Init().setInterval(Methods::apply(*ppveMin, querySDivideOperation(), *source.ppveMin, env),
                  Methods::apply(*ppveMin, querySDivideOperation(),
                     *Methods::apply(*sourceMax, querySMinOperation(),
                        *minusTwo, env), env)).setUnsigned(queryUnsignedField()));
            overflowMinint = Methods::merge(overflowMinint,
               *Methods::newInterval(*ppveMin,
                  Init().setInterval(Methods::apply(*source.ppveMin, querySDivideOperation(), *sourceMax, env),
                     *newSMinDomain()).setUnsigned(queryUnsignedField())), env);
            auto maxAssignOperation = querySMaxAssignOperation();
            ppveMin = Methods::applyAssign(ppveMin, maxAssignOperation,
               *Methods::applyAssign(newSMinDomain(),
                  querySNextAssignOperation(), EvaluationParameters(env)), env);
         };
      };

      bool isDisjunctionExact = true;
      EvaluationParameters mayStopErrorStates(EPMayStopErrorStates);
      mayStopErrorStates.setApproximatePartFrom(env);
      if (!maxCompare.isLessOrEqualZero()) // b/(d-1) <= a/d+1
         isDisjunctionExact = isExact || (isDisjunctionExact && Methods::applyBoolean(
               *Methods::applyAssign(PPVirtualElement(*ppveMax), querySDivideAssignOperation(),
                  *Methods::applyAssign(PPVirtualElement(*sourceMax),
                     querySPrevAssignOperation(), mayStopErrorStates), mayStopErrorStates),
               querySCompareLessOrEqualOperation(),
               *Methods::applyAssign(Methods::applyAssign(PPVirtualElement(*ppveMin),
                     querySDivideAssignOperation(), *sourceMax, mayStopErrorStates),
                  querySNextAssignOperation(), mayStopErrorStates)));
      if (!minCompare.isGreaterOrEqualZero()) // a/(d-1) >= b/d-1
         isDisjunctionExact = isExact || (isDisjunctionExact && Methods::applyBoolean(
               *Methods::applyAssign(PPVirtualElement(*ppveMin), querySDivideAssignOperation(),
                  *Methods::applyAssign(PPVirtualElement(*sourceMax),
                     querySPrevAssignOperation(), mayStopErrorStates), mayStopErrorStates),
               querySCompareGreaterOrEqualOperation(),
               *Methods::applyAssign(Methods::applyAssign(PPVirtualElement(*ppveMax),
                     querySDivideAssignOperation(), *sourceMax, mayStopErrorStates),
                  querySPrevAssignOperation(), mayStopErrorStates)));
      if ((isExact == isDisjunctionExact) == false) {
         env.mergeVerdictDegradate();
         disjunctionNotExact = applySureCaseInterval(QueryOperation().setDivide(), source, env);
         if (env.getInformationKind().isSure()) {
            env.presult() = disjunctionNotExact;
            return true;
         };
      };

      if (minCompare.isGreaterOrEqualZero()) { // a >= 0 and d < 0 -> [b/d, a/c]
         PPVirtualElement temp = Methods::applyAssign(ppveMax, querySDivideAssignOperation(), *sourceMax, minEnv);
         auto divideAssignOperation = querySDivideAssignOperation();
         ppveMax = Methods::applyAssign(ppveMin, divideAssignOperation, *source.ppveMin, maxEnv);
         ppveMin = temp;
      }
      else if (maxCompare.isLessOrEqualZero()) { // b <= 0 et d < 0 -> [b/c, a/d]
         auto divideAssignOperation = querySDivideAssignOperation();
         PPVirtualElement temp
            = Methods::applyAssign(ppveMax, divideAssignOperation, *source.ppveMin, minEnv);
         ppveMax = Methods::applyAssign(ppveMin, divideAssignOperation, *sourceMax, maxEnv);
         ppveMin = temp;
      }
      else { // [a,0] U [0,b] et d < 0 -> [b/d, a/d]
         // merge the different cases -> U [b/d, a/c], if a may > 0
         //                              U [b/c, a/d], if b may < 0
         PNT::TPassPointer<IntegerInterval, PPAbstractElement> negative, positive;
         if (minCompare.mayBeGreaterZero())
            positive = Methods::newInterval(*ppveMin,
               Init().setInterval(Methods::apply(*ppveMax, querySDivideOperation(), *sourceMax, minEnv),
                  Methods::apply(*ppveMin, querySDivideOperation(), *source.ppveMin, maxEnv)).setUnsigned(queryUnsignedField()));
         if (maxCompare.mayBeLessZero())
            negative = Methods::newInterval(*ppveMin,
               Init().setInterval(Methods::apply(*ppveMax, querySDivideOperation(), *source.ppveMin, minEnv),
               Methods::apply(*ppveMin, querySDivideOperation(), *sourceMax, maxEnv)).setUnsigned(queryUnsignedField()));
         auto divideAssignOperation = querySDivideAssignOperation();
         PPVirtualElement temp
            = Methods::applyAssign(ppveMax, divideAssignOperation, *sourceMax, minEnv);
         ppveMax = Methods::applyAssign(ppveMin, divideAssignOperation, *sourceMax, maxEnv);
         ppveMin = temp;
         if (negative.isValid() || positive.isValid()) {
            TransmitEnvironment mergeEnv(env);
            if (negative.isValid()) {
               mergeWithInterval(*negative, mergeEnv);
               if (mergeEnv.hasResult())
                  env.presult() = mergeEnv.presult();
               mergeEnv.merge().clear();
            };
            if (positive.isValid()) {
               if (env.hasResult())
                  env.getResult().mergeWith(*positive, mergeEnv);
               else
                  mergeWithInterval(*positive, mergeEnv);
               if (mergeEnv.hasResult())
                  env.presult() = mergeEnv.presult();
               mergeEnv.merge().clear();
            };
         };
         return true;
      };
   }
   else { // [a,b] / [c,-1] U [1,d] + possible division by 0
      PNT::TPassPointer<IntegerInterval, PPAbstractElement>
         sourceNegative = Methods::newInterval(*ppveMin,
               Init().setInterval(PPVirtualElement(*source.ppveMin), Methods::newMinusOne(*ppveMin))),
         sourcePositive = Methods::newInterval(*ppveMin,
               Init().setInterval(Methods::newOne(*ppveMin), PPVirtualElement(*source.ppveMax))),
         thisCopy(*this);
      TransmitEnvironment positiveEnv(env), negativeEnv(env);
      applyDivideAssignInterval(operation, *sourceNegative, negativeEnv);
      thisCopy->applyDivideAssignInterval(operation, *sourcePositive, positiveEnv);
      if (negativeEnv.hasResult())
         env.presult() = Methods::merge(negativeEnv.presult(),
               positiveEnv.hasResult() ? positiveEnv.getResult() : *thisCopy, env);
      else if (positiveEnv.hasResult())
         env.presult() = Methods::merge(positiveEnv.presult(), *this, env);
      else {
         TransmitEnvironment mergeEnv(env);
         mergeWithInterval(*thisCopy, mergeEnv);
         if (mergeEnv.hasResult())
            env.presult() = mergeEnv.presult();
      };
      return true;
   };

   if (disjunctionNotExact.isValid()) {
      if (env.hasResult())
         env.presult() = Methods::merge(env.presult(), *disjunctionNotExact, EvaluationParameters(env));
      else {
         const VirtualElement& min = *ppveMin;
         disjunctionNotExact->addMay(Methods::newInterval(min, Init().setInterval(ppveMin, ppveMax).setUnsigned(queryUnsignedField())));
         env.presult() = disjunctionNotExact;
      };
   };
   return true;
}

bool
IntegerInterval::applyOppositeAssign(const VirtualOperation&, EvaluationEnvironment& env) {
   TransmitEnvironment minEnv(env), maxEnv(env);
   auto oppositeOperation = Methods::queryOppositeAssign(*ppveMin);
   PPVirtualElement temp = Methods::applyAssign(ppveMin, oppositeOperation, minEnv);
   ppveMin = Methods::applyAssign(ppveMax, oppositeOperation, maxEnv);
   ppveMax = temp;

   if (!minEnv.isPositiveOverflow())
      return true;

   if ((env.getInformationKind().isMay()) || (minEnv.isSurePositiveOverflow())) {
      ppveMax = newSMaxDomain();
      TransmitEnvironment mergeEnv(env);
      mergeWith(*newSMinDomain(), mergeEnv);
      if (mergeEnv.hasResult())
         env.presult() = mergeEnv.presult();
      return true;
   };

   // -[a,b] = [-b,-max(a,minint+1)] U [max(a,minint),minint]
   EvaluationParameters natural(env);
   ppveMax = Methods::applyAssign(ppveMax, Methods::queryOppositeAssign(*ppveMin), natural);
   PNT::TPassPointer<IntegerInterval, PPAbstractElement>
      minint = Methods::newInterval(*ppveMin, Init().setInterval(
         Methods::apply(*ppveMax, querySMinOperation(), *newSMinDomain(), env), newSMinDomain()));
   ppveMax = Methods::applyAssign(Methods::apply(*ppveMax, querySMaxOperation(),
         *Methods::applyAssign(newSMinDomain(), querySNextAssignOperation(), natural), natural),
      Methods::queryOppositeAssign(*ppveMin), natural);
   TransmitEnvironment mergeEnv(env);
   mergeWithInterval(*minint, mergeEnv);
   if (mergeEnv.hasResult())
      env.presult() = mergeEnv.presult();
   return true;
}

/* integer apply methods */

bool
IntegerInterval::applyModuloAssignConstant(const VirtualOperation&,
      const VirtualElement& source, EvaluationEnvironment& env) {
   QueryOperation::CompareSpecialEnvironment
      sourceCompare = querySCompareSpecial(source),
      thisCompare = querySCompareSpecial(*ppveMin);
   if (sourceCompare.mayBeZero()) {
      env.setDivisionByZero();
      if (sourceCompare.isZero()) {
         if (!env.getInformationKind().isMay())
            env.setSureDivisionByZero();
         env.setEmpty();
         return true;
      };
   };

   if (sourceCompare.isGreaterOrEqualZero() && thisCompare.isGreaterOrEqualZero()) {
      // c >= 0, a >= 0 -> compute [a,b] % c
      EvaluationParameters natural(env);
      PPVirtualElement diffBound;
      diffBound = Methods::apply(*ppveMax, querySMinusOperation(), *ppveMin, natural);
      Methods::BooleanResult compareInterval = Methods::applyBoolean(
         *diffBound, Methods::queryCompareLessSigned(*ppveMin), source);
      if (compareInterval.isValid()) {
         env.mergeVerdictExact();
         if (compareInterval) { // b-a < c
            auto moduloAssignOperation = querySModuloAssignOperation();
            ppveMin = Methods::applyAssign(ppveMin, moduloAssignOperation, source, env);
            ppveMax = Methods::applyAssign(ppveMax, moduloAssignOperation, source, env);

            compareInterval = Methods::applyBoolean(*ppveMin, querySCompareLessOrEqualOperation(), *ppveMax);
            if (compareInterval.isValid()) {
               if (!compareInterval) {
                  // a % c > b % c
                  PNT::TPassPointer<VirtualDisjunction, PPAbstractElement> result = Methods::newDisjunction(source);
                  const VirtualElement& min = *ppveMin;
                  result->add(Methods::newInterval(min, Init().setInterval(ppveMin,
                        Methods::applyAssign(PPVirtualElement(source), querySPrevAssignOperation(), natural)).setUnsigned(queryUnsignedField())),
                     env.getInformationKind());
                  result->add(Methods::newInterval(*ppveMin, Init().setInterval(Methods::newZero(*ppveMin), ppveMax).setUnsigned(queryUnsignedField())),
                     env.getInformationKind());
                  env.presult() = result;
               };
            }
            else { // not possible to know if a % c <= b % c
               env.mergeVerdictDegradate();
               PPVirtualElement newMax = ppveMax;
               ppveMax = Methods::applyAssign(
                  Methods::apply(*ppveMin, querySPlusOperation(), *diffBound, natural),
                  querySMinAssignOperation(),
                  *Methods::applyAssign(source, querySPrevAssignOperation(), natural), natural);
               PPVirtualElement newMin = Methods::applyAssign(
                  Methods::apply(*newMax, querySMinusOperation(), *diffBound, natural),
                  querySMaxAssignOperation(), *Methods::newZero(source), natural);
               mergeWithInterval(IntegerInterval(Init().setInterval(newMin, newMax)), env);
            };
         }
         else // b-a >= c -> the result is [0,c-1]
            env.presult() = Methods::newInterval(source, Init().setInterval(Methods::newZero(source),
               Methods::applyAssign(PPVirtualElement(source), querySPrevAssignOperation(), natural)).setUnsigned(queryUnsignedField()));
      }
      else { // Impossible de savoir si b-a < c.
         env.mergeVerdictDegradate();
         PNT::TPassPointer<VirtualDisjunction, PPAbstractElement>
            result = applySureCase(QueryOperation().setModulo(), source, env);
         if (!env.getInformationKind().isSure()) {
            ppveMin = Methods::newZero(source);
            EvaluationParameters mayStopErrorStates(EPMayStopErrorStates);
            mayStopErrorStates.setApproximatePartFrom(env);
            ppveMax = Methods::applyAssign(PPVirtualElement(source), querySPrevAssignOperation(), mayStopErrorStates);
            if (result.isValid())
               result->addMay(Methods::newInterval(source, Init().setInterval(ppveMin, ppveMax).setUnsigned(queryUnsignedField())));
            else
               return true;
         };
         env.presult() = result;
      };
   }
   else { // c is potentially negative or a is potentially negative
      env.mergeVerdictDegradate();
      PNT::TPassPointer<VirtualDisjunction, PPAbstractElement>
         result = applySureCase(QueryOperation().setModulo(), source, env);
      if (!env.getInformationKind().isSure()) {
         EvaluationParameters mayStopErrorStates(EPMayStopErrorStates);
         mayStopErrorStates.setApproximatePartFrom(env);
         if (sourceCompare.isGreaterOrEqualZero()) { // c >= 0 and a is potentially negative
            auto prevAssignOperation = querySPrevAssignOperation();
            auto oppositeAssignOperation = Methods::queryOppositeAssign(source);
            if (querySCompareSpecial(*ppveMax).isLessOrEqualZero()) {
               // c >= 0 and b <= 0 -> [1-c, 0] as may
               ppveMin = Methods::applyAssign(
                     Methods::applyAssign(PPVirtualElement(source), prevAssignOperation, mayStopErrorStates),
                     oppositeAssignOperation, mayStopErrorStates);
               ppveMax = Methods::newZero(source);
            }
            else { // b is potentially positive -> [1-c, c-1] as may
               ppveMin = Methods::applyAssign(
                     Methods::applyAssign(PPVirtualElement(source), prevAssignOperation, mayStopErrorStates),
                     oppositeAssignOperation, mayStopErrorStates);
               ppveMax = Methods::applyAssign(PPVirtualElement(source), prevAssignOperation, mayStopErrorStates);
            };
         }
         else if (sourceCompare.isLessOrEqualZero()) { // c <= 0
            auto nextAssignOperation = querySNextAssignOperation();
            if (thisCompare.isGreaterOrEqualZero()) { // c <= 0 and a >= 0 -> [c+1, 0] as may
               ppveMin = Methods::applyAssign(PPVirtualElement(source), nextAssignOperation, mayStopErrorStates);
               ppveMax = Methods::newZero(*ppveMin);
            }
            else if (querySCompareSpecial(*ppveMax).isLessOrEqualZero()) {
               // c <= 0 and b <= 0 -> [0, -(1+c)] as may
               ppveMin = Methods::newZero(source);
               auto oppositeAssignOperation = Methods::queryOppositeAssign(source);
               ppveMax = Methods::applyAssign(
                     Methods::applyAssign(PPVirtualElement(source), nextAssignOperation, mayStopErrorStates),
                     oppositeAssignOperation, mayStopErrorStates);
            }
            else { // c <= 0, a is potentially negative and b is potentially positive
               auto oppositeAssignOperation = Methods::queryOppositeAssign(source);
               ppveMin = Methods::applyAssign(PPVirtualElement(source), nextAssignOperation, mayStopErrorStates);
               ppveMax = Methods::applyAssign(
                     Methods::applyAssign(PPVirtualElement(source), nextAssignOperation, mayStopErrorStates),
                     oppositeAssignOperation, mayStopErrorStates);
            };
         }
         else { // impossible to know the sign of c -> [1-c,c-1] U [c+1,-1-c] as may
            auto prevAssignOperation = querySPrevAssignOperation();
            auto nextAssignOperation = querySNextAssignOperation();
            auto oppositeAssignOperation = Methods::queryOppositeAssign(source);
            ppveMin = Methods::applyAssign(PPVirtualElement(source), nextAssignOperation, mayStopErrorStates);
            ppveMax = Methods::applyAssign(
                  Methods::applyAssign(PPVirtualElement(source), nextAssignOperation, mayStopErrorStates),
                  oppositeAssignOperation, mayStopErrorStates);
            PPVirtualElement
               newMin = Methods::applyAssign(
                     Methods::applyAssign(PPVirtualElement(source), prevAssignOperation, mayStopErrorStates),
                     oppositeAssignOperation, mayStopErrorStates),
               newMax = Methods::applyAssign(PPVirtualElement(source), prevAssignOperation, mayStopErrorStates);
            if (result.isValid()) {
               result->addMay(Methods::newInterval(source, Init().setInterval(ppveMin, ppveMax).setUnsigned(queryUnsignedField())));
               result->addMay(Methods::newInterval(source, Init().setInterval(newMin, newMax).setUnsigned(queryUnsignedField())));
            }
            else
               mergeWithInterval(IntegerInterval(Init().setInterval(newMin, newMax)), env);
            return true;
         };
         if (result.isValid()) {
            result->addMay(Methods::newInterval(source, Init().setInterval(ppveMin, ppveMax).setUnsigned(queryUnsignedField())));
            env.presult() = result;
         };
      };
   };
   return true;
}

bool
IntegerInterval::applyModuloAssignInterval(const VirtualOperation& operation,
      const IntegerInterval& source, EvaluationEnvironment& env) {
   if (queryUnsignedField() != source.queryUnsignedField()) {
      PNT::TPassPointer<VirtualDisjunction, PPAbstractElement> copy = changeSign(env);
      if (copy.isValid()) {
         TransmitEnvironment applyEnv(env);
         applyEnv.setFirstArgument(source);
         assume(copy->apply(operation, applyEnv));
         env.presult() = applyEnv.hasResult() ? applyEnv.presult() : (PPAbstractElement) copy;
         return true;
      };
   };
   env.mergeVerdictDegradate();
   const PPVirtualElement& sourceMin = source.ppveMin;
   const PPVirtualElement& sourceMax = source.ppveMax;
   PNT::TPassPointer<VirtualDisjunction, PPAbstractElement>
      result = applySureCaseInterval(QueryOperation().setModulo(), source, env);
   if (!env.getInformationKind().isSure()) {
      QueryOperation::CompareSpecialEnvironment
         minSourceCompare = querySCompareSpecial(*sourceMin),
         maxSourceCompare = querySCompareSpecial(*sourceMax);
      EvaluationParameters mayStopErrorStates(EPMayStopErrorStates);
      mayStopErrorStates.setApproximatePartFrom(env);
      if (minSourceCompare.isGreaterOrEqualZero()) { // c >= 0
         if (minSourceCompare.mayBeZero()) {
            env.setDivisionByZero();
            if ((!env.getInformationKind().isMay()) && minSourceCompare.isZero())
               env.setSureDivisionByZero();
         };
         auto prevAssignOperation = querySPrevAssignOperation();
         if (querySCompareSpecial(*ppveMin).isGreaterOrEqualZero()) {
            // c >= 0 and a >= 0 -> [0, d-1] as may
            ppveMin = Methods::newZero(*ppveMin);
            ppveMax = Methods::applyAssign(PPVirtualElement(*sourceMax), prevAssignOperation, mayStopErrorStates);
         }
         else if (querySCompareSpecial(*ppveMax).isLessOrEqualZero()) {
            // c >= 0 and b <= 0 -> [1-d, 0] as may
            auto oppositeAssignOperation = Methods::queryOppositeAssign(*ppveMin);
            ppveMin = Methods::applyAssign(
               Methods::applyAssign(PPVirtualElement(*sourceMax), prevAssignOperation, mayStopErrorStates),
               oppositeAssignOperation, mayStopErrorStates);
            ppveMax = Methods::newZero(*ppveMin);
         }
         else { // a <= 0 and b >= 0 -> [1-d, d-1] as may
            auto oppositeAssignOperation = Methods::queryOppositeAssign(*ppveMin);
            ppveMin = Methods::applyAssign(
               Methods::applyAssign(PPVirtualElement(*sourceMax), prevAssignOperation, mayStopErrorStates),
               oppositeAssignOperation, mayStopErrorStates);
            ppveMax = Methods::applyAssign(PPVirtualElement(*sourceMax), prevAssignOperation, mayStopErrorStates);
         };
      }
      else if (maxSourceCompare.isLessOrEqualZero()) { // d <= 0
         if (maxSourceCompare.mayBeZero()) {
            env.setDivisionByZero();
            if ((!env.getInformationKind().isMay()) && maxSourceCompare.isZero())
               env.setSureDivisionByZero();
         };
         auto nextAssignOperation = querySNextAssignOperation();
         if (querySCompareSpecial(*ppveMin).isGreaterOrEqualZero()) {
            // d <= 0 and a >= 0 -> [c+1, 0] as may
            ppveMin = Methods::applyAssign(PPVirtualElement(*sourceMin), nextAssignOperation, mayStopErrorStates);
            if (ppveMin.isValid())
               ppveMax = Methods::newZero(*ppveMin);
            else
               env.setEmpty();
         }
         else if (querySCompareSpecial(*ppveMax).isLessOrEqualZero()) {
            // d <= 0 and b <= 0 -> [0, -(1+c)] as may
            auto oppositeAssignOperation = Methods::queryOppositeAssign(*ppveMin);
            ppveMin = Methods::newZero(*ppveMin);
            ppveMax = Methods::applyAssign(
               Methods::applyAssign(PPVirtualElement(*sourceMin), nextAssignOperation, mayStopErrorStates),
               oppositeAssignOperation, mayStopErrorStates);
         }
         else { // d <= 0, a is potentially negative and b is potentially positive
            auto oppositeAssignOperation = Methods::queryOppositeAssign(*ppveMin);
            ppveMin = Methods::applyAssign(PPVirtualElement(*sourceMin), nextAssignOperation, mayStopErrorStates);
            ppveMax = Methods::applyAssign(
               Methods::applyAssign(PPVirtualElement(*sourceMin), nextAssignOperation, mayStopErrorStates),
               oppositeAssignOperation, mayStopErrorStates);
         };
      }
      else { // c <= 0 and d >= 0 -> [1-d,d-1] U [c+1,-1-c] as may
         env.setDivisionByZero();
         if ((!env.getInformationKind().isMay()) && maxSourceCompare.isGreaterOrEqualZero()
               && minSourceCompare.isLessOrEqualZero())
            env.setSureDivisionByZero();
         auto prevAssignOperation = querySPrevAssignOperation();
         auto nextAssignOperation = querySNextAssignOperation();
         auto oppositeAssignOperation = Methods::queryOppositeAssign(*ppveMin);
         ppveMin = Methods::applyAssign(PPVirtualElement(*sourceMin), nextAssignOperation, mayStopErrorStates);
         ppveMax = Methods::applyAssign(
            Methods::applyAssign(PPVirtualElement(*sourceMin), nextAssignOperation, mayStopErrorStates),
            oppositeAssignOperation, mayStopErrorStates);
         PPVirtualElement
            newMin = Methods::applyAssign(
               Methods::applyAssign(PPVirtualElement(*sourceMax), prevAssignOperation, mayStopErrorStates),
               oppositeAssignOperation, mayStopErrorStates),
            newMax = Methods::applyAssign(PPVirtualElement(*sourceMax), prevAssignOperation, mayStopErrorStates);
         if (result.isValid()) {
            const VirtualElement& min = *ppveMin;
            result->addMay(Methods::newInterval(min, Init().setInterval(ppveMin, ppveMax).setUnsigned(queryUnsignedField())));
            result->addMay(Methods::newInterval(min, Init().setInterval(newMin, newMax).setUnsigned(queryUnsignedField())));
            env.presult() = result;
         }
         else
            mergeWithInterval(IntegerInterval(Init().setInterval(newMin, newMax)), env);
         return true;
      };
      if (result.isValid()) {
         const VirtualElement& min = *ppveMin;
         result->addMay(Methods::newInterval(min, Init().setInterval(ppveMin, ppveMax).setUnsigned(queryUnsignedField())));
         env.presult() = result;
      };
   }
   else
      env.presult() = result;
   return true;
}

template <class Application>
bool
IntegerInterval::partitionConstant(const Application& application,
      PNT::TPassPointer<VirtualDisjunction, PPAbstractElement> result, EvaluationEnvironment& env) {
   TransmitEnvironment applyEnv(env, EvaluationParameters(env).setMay());
   PNT::TPassPointer<IntegerInterval, PPAbstractElement>
      negativeResult = Methods::newInterval(*ppveMin, Init().setInterval(*ppveMin, Methods::newMinusOne(*ppveMin)).setUnsigned(queryUnsignedField()));
   assume(application.apply(*negativeResult, applyEnv));
   PPVirtualElement localResult = PPVirtualElement(applyEnv.hasResult() ? applyEnv.presult() : negativeResult);
   
   applyEnv.merge().clear();
   ppveMin = Methods::newZero(*ppveMin);
   assume(application.apply(*this, applyEnv));
   EvaluationParameters mayStopErrorStates(EPMayStopErrorStates);
   mayStopErrorStates.setApproximatePartFrom(env);
   localResult = Methods::merge(localResult, applyEnv.hasResult()
         ? applyEnv.getResult() : (const VirtualElement&) *this, mayStopErrorStates);
   if (result.isValid()) {
      result->addMay(localResult);
      env.presult() = result;
   }
   else
      env.presult() = localResult;
   return true;
}

class BitOrApplication {
  private:
   const VirtualElement& seSource;
   
  public:
   BitOrApplication(const VirtualElement& source) : seSource(source) {}
   bool apply(IntegerInterval& interval, EvaluationEnvironment& env) const
      {  return interval.applyBitOrAssignConstant(
            IntegerInterval::Methods::queryBitOrAssign(seSource), seSource, env);
      }

   const VirtualElement& getSource() const { return seSource; }
};

class BitOrIntervalApplication {
  private:
   const IntegerInterval& iieSource;
   
  public:
   BitOrIntervalApplication(const IntegerInterval& source) : iieSource(source) {}
   bool apply(IntegerInterval& interval, EvaluationEnvironment& env) const
      {  return interval.applyBitOrAssignInterval(IntegerInterval::Methods::
            queryBitOrAssign(iieSource), iieSource, env);
      }
   bool apply(IntegerInterval& interval, const IntegerInterval& newSource, EvaluationEnvironment& env) const
      {  return interval.applyBitOrAssignInterval(IntegerInterval::Methods::
            queryBitOrAssign(iieSource), newSource, env);
      }

   const IntegerInterval& getSource() const { return iieSource; }
};

bool
IntegerInterval::applyBitOrAssignConstant(const VirtualOperation&,
      const VirtualElement& source, EvaluationEnvironment& env) {
   QueryOperation::CompareSpecialEnvironment sourceSpecial = querySCompareSpecial(source);
   if (sourceSpecial.isZero() || sourceSpecial.isMinusOne()) {
      env.mergeVerdictExact();
      if (sourceSpecial.isMinusOne())
         env.storeResult(source.createSCopy());
      return true;
   };

   // optimisation possible if x & 2^k-1 (upper common bits between a and b)
   //                                    == 0 => [x|a, x|b] -> exact
   env.mergeVerdictDegradate();
   EvaluationEnvironment::LatticeCreation creation = env.getLatticeCreation();
   if (creation.isTop() || creation.isShareTop()) {
      env.presult() = Details::IntOperationElement::Methods::newTop(*this);
      return true;
   };

   PNT::TPassPointer<VirtualDisjunction, PPAbstractElement>
      result = applySureCase(QueryOperation().setBitOr(), source, env);

   if (!env.getInformationKind().isSure()) {
      QueryOperation::CompareSpecialEnvironment
         minSpecial = querySCompareSpecial(*ppveMin),
         maxSpecial = querySCompareSpecial(*ppveMax);
      if (minSpecial.mayBeLessZero() && maxSpecial.mayBeGreaterOrEqualZero())
         return partitionConstant(BitOrApplication(source), result, env);
      else if ((sourceSpecial.isGreaterOrEqualZero() && minSpecial.isGreaterOrEqualZero())
            || (sourceSpecial.isLessZero() && maxSpecial.isLessZero())) {
         // [max(x,a), x|(2^k-1)] with 2^k-1 >= b
         QueryOperation::BitDomainEnvironment bitDomain(*this);
         query(QueryOperation().setBitDomain(), bitDomain);
         EvaluationParameters mayStopErrorStates(EPMayStopErrorStates);
         mayStopErrorStates.setApproximatePartFrom(env);
         auto maxAssignOperation = querySMaxAssignOperation();
         auto bitOrAssignOperation = Methods::queryBitOrAssign(source);
         ppveMin = Methods::applyAssign(ppveMin, maxAssignOperation, source, mayStopErrorStates);
         ppveMax = Methods::applyAssign(PPVirtualElement(bitDomain.maxOne()), bitOrAssignOperation,
               source, mayStopErrorStates);
      }
      else if (sourceSpecial.isLessZero()) { // minSpecial.isGreaterOrEqualZero()
         // [max(x, a | 10..0), x|(2^k-1)] with 2^k-1 >= b
         QueryOperation::BitDomainEnvironment bitDomain(*this);
         query(QueryOperation().setBitDomain(), bitDomain);
         EvaluationParameters exactStopErrorStates(EPExactStopErrorStates);
         exactStopErrorStates.setApproximatePartFrom(env);
         EvaluationParameters mayStopErrorStates(EPMayStopErrorStates);
         mayStopErrorStates.setApproximatePartFrom(env);
         auto rightShiftAssignOperation = querySRightShiftAssignOperation();
         auto bitOrAssignOperation = Methods::queryBitOrAssign(source);
         ppveMin = Methods::applyAssign(ppveMin, bitOrAssignOperation,
            *Methods::applyAssign(Methods::newOne(source),
               rightShiftAssignOperation,
               *Methods::newIntForShift(source, source.getSizeInBits()-1), exactStopErrorStates), mayStopErrorStates);
         auto maxAssignOperation = querySMaxAssignOperation();
         ppveMin = Methods::applyAssign(ppveMin, maxAssignOperation, source, mayStopErrorStates);
         ppveMax = Methods::applyAssign(PPVirtualElement(bitDomain.maxOne()), bitOrAssignOperation,
               source, mayStopErrorStates);
      }
      else if (sourceSpecial.isGreaterOrEqualZero()) { // maxSpecial.isLessZero()
         // [max(x|10..0, a), x|(2^k-1)] with 2^k-1 >= b
         QueryOperation::BitDomainEnvironment bitDomain(*this);
         query(QueryOperation().setBitDomain(), bitDomain);
         PPVirtualElement oldMin(*ppveMin);
         EvaluationParameters mayStopErrorStates(EPMayStopErrorStates);
         mayStopErrorStates.setApproximatePartFrom(env);
         EvaluationParameters exactStopErrorStates(EPExactStopErrorStates);
         exactStopErrorStates.setApproximatePartFrom(env);
         auto maxAssignOperation = querySMaxAssignOperation();
         auto rightShiftAssignOperation = querySRightShiftAssignOperation();
         auto bitOrOperation = Methods::queryBitOr(source);
         auto bitOrAssignOperation = Methods::queryBitOrAssign(source);
         ppveMin = Methods::apply(source, bitOrOperation,
            *Methods::applyAssign(Methods::newOne(source), rightShiftAssignOperation,
               *Methods::newIntForShift(source, source.getSizeInBits()-1), exactStopErrorStates), mayStopErrorStates);
         ppveMin = Methods::applyAssign(ppveMin, maxAssignOperation, *oldMin, mayStopErrorStates);
         ppveMax = Methods::applyAssign(PPVirtualElement(bitDomain.maxOne()), bitOrAssignOperation,
               source, mayStopErrorStates);
      }
      else { // the sign of sourceSpecial is undefined => partition into 2 + merge
         QueryOperation::BitDomainEnvironment bitDomain(*this);
         query(QueryOperation().setBitDomain(), bitDomain);
         PPVirtualElement oldMin(*ppveMin), newMin;
         EvaluationParameters exactStopErrorStates(EPExactStopErrorStates);
         exactStopErrorStates.setApproximatePartFrom(env);
         EvaluationParameters mayStopErrorStates(EPMayStopErrorStates);
         mayStopErrorStates.setApproximatePartFrom(env);
         auto bitOrAssignOperation = Methods::queryBitOrAssign(source);
         auto bitOrOperation = Methods::queryBitOr(source);
         auto rightShiftAssignOperation = querySRightShiftAssignOperation();
         auto minAssignOperation = querySMinAssignOperation();
         auto maxAssignOperation = querySMaxAssignOperation();
         ppveMin = Methods::applyAssign(ppveMin, bitOrAssignOperation,
            *Methods::applyAssign(Methods::newOne(source),
               rightShiftAssignOperation,
               *Methods::newIntForShift(source, source.getSizeInBits()-1), exactStopErrorStates), mayStopErrorStates);
         if (ppveMin.isValid())
            ppveMin = Methods::applyAssign(ppveMin, maxAssignOperation, source, exactStopErrorStates);

         newMin = Methods::apply(source, bitOrOperation,
            *Methods::applyAssign(Methods::newOne(source), rightShiftAssignOperation,
               *Methods::newIntForShift(source, source.getSizeInBits()-1), exactStopErrorStates), mayStopErrorStates),
         newMin = Methods::applyAssign(newMin, maxAssignOperation, *oldMin, mayStopErrorStates);

         ppveMin = Methods::applyAssign(ppveMin, minAssignOperation, *newMin, mayStopErrorStates);
         ppveMax = Methods::applyAssign(PPVirtualElement(bitDomain.maxOne()), bitOrAssignOperation,
            source, mayStopErrorStates);
      };

      if (result.isValid()) {
         result->addMay(Methods::newInterval(source, Init().setInterval(ppveMin, ppveMax).setUnsigned(queryUnsignedField())));
         env.presult() = result;
      };
   }
   else
      env.presult() = result;

   return true;
}

class BitAndApplication {
  private:
   const VirtualElement& seSource;

  public:
   BitAndApplication(const VirtualElement& source) : seSource(source) {}
   bool apply(IntegerInterval& interval, EvaluationEnvironment& env) const
      {  return interval.applyBitAndAssignConstant(IntegerInterval::Methods::
            queryBitAndAssign(seSource), seSource, env);
      }

   const VirtualElement& getSource() const { return seSource; }
};

class BitAndIntervalApplication {
  private:
   const IntegerInterval& iieSource;
   
  public:
   BitAndIntervalApplication(const IntegerInterval& source) : iieSource(source) {}
   bool apply(IntegerInterval& interval, EvaluationEnvironment& env) const
      {  return interval.applyBitAndAssignInterval(IntegerInterval::Methods::
            queryBitAndAssign(iieSource), iieSource, env);
      }
   bool apply(IntegerInterval& interval, const IntegerInterval& newSource, EvaluationEnvironment& env) const
      {  return interval.applyBitAndAssignInterval(IntegerInterval::Methods::
            queryBitAndAssign(iieSource), newSource, env);
      }

   const IntegerInterval& getSource() const { return iieSource; }
};

bool
IntegerInterval::applyBitAndAssignConstant(const VirtualOperation&,
      const VirtualElement& source, EvaluationEnvironment& env) {
   QueryOperation::CompareSpecialEnvironment sourceSpecial = querySCompareSpecial(source);
   if (sourceSpecial.isZero() || sourceSpecial.isMinusOne()) {
      env.mergeVerdictExact();
      if (sourceSpecial.isZero())
         env.storeResult(source.createSCopy());
      return true;
   };

   // optimisation possible if x | 2^k (upper common bits between a and b)
   //                                  == 0 => [x&a, x&b] -> exact
   env.mergeVerdictDegradate();
   EvaluationEnvironment::LatticeCreation creation = env.getLatticeCreation();
   if (creation.isTop() || creation.isShareTop()) {
      env.presult() = Details::IntOperationElement::Methods::newTop(*this);
      return true;
   };

   PNT::TPassPointer<VirtualDisjunction, PPAbstractElement>
      result = applySureCase(QueryOperation().setBitAnd(), source, env);

   if (!env.getInformationKind().isSure()) {
      QueryOperation::CompareSpecialEnvironment
         minSpecial = querySCompareSpecial(*ppveMin),
         maxSpecial = querySCompareSpecial(*ppveMax);
      if (minSpecial.mayBeLessZero() && maxSpecial.mayBeGreaterOrEqualZero())
         return partitionConstant(BitAndApplication(source), result, env);
      else if ((sourceSpecial.isGreaterOrEqualZero() && minSpecial.isGreaterOrEqualZero())
            || (sourceSpecial.isLessZero() && minSpecial.isLessZero())) { // [0, min(b,x)]
         EvaluationParameters mayStopErrorStates(EPMayStopErrorStates);
         mayStopErrorStates.setApproximatePartFrom(env);
         ppveMin = sourceSpecial.isGreaterOrEqualZero()
            ? Methods::newZero(source) 
            : Methods::newMinSigned(source);
         ppveMax = Methods::applyAssign(ppveMax, querySMinAssignOperation(), source, mayStopErrorStates);
      }
      else if (sourceSpecial.isLessZero()) // minSpecial.mayBeGreaterOrEqualZero() -> [-MAXINT, b]
         ppveMin = newSMinDomain();
      else if (sourceSpecial.isGreaterOrEqualZero()) { // maxSpecial.mayBeLessZero()
         // [0, b] with the last bit of b inversed
         PPVirtualElement oldMin = ppveMin;
         EvaluationParameters mayStopErrorStates(EPMayStopErrorStates);
         mayStopErrorStates.setApproximatePartFrom(env);
         ppveMin = Methods::apply(source, Methods::queryBitOr(source),
            *Methods::applyAssign(newSMinDomain(),
               Methods::queryBitNegateAssign(source), mayStopErrorStates), mayStopErrorStates);
         auto maxAssignOperation = querySMaxAssignOperation();
         ppveMin = Methods::applyAssign(ppveMin, maxAssignOperation, *oldMin, mayStopErrorStates);
      }
      else
         ppveMin = Methods::newMinSigned(source);

      if (result.isValid()) {
         result->addMay(Methods::newInterval(source, Init().setInterval(ppveMin, ppveMax).setUnsigned(queryUnsignedField())));
         env.presult() = result;
      }
   }
   else
      env.presult() = result;

   return true;
}

class BitExclusiveOrApplication {
  private:
   const VirtualElement& seSource;
   
  public:
   BitExclusiveOrApplication(const VirtualElement& source) : seSource(source) {}
   bool apply(IntegerInterval& interval, EvaluationEnvironment& env) const
      {  return interval.applyBitExclusiveOrAssignConstant(IntegerInterval::Methods::
            queryBitExclusiveOrAssign(seSource), seSource, env);
      }
   const VirtualElement& getSource() const { return seSource; }
};

class BitExclusiveOrIntervalApplication {
  private:
   const IntegerInterval& iieSource;
   
  public:
   BitExclusiveOrIntervalApplication(const IntegerInterval& source) : iieSource(source) {}
   bool apply(IntegerInterval& interval, EvaluationEnvironment& env) const
      {  return interval.applyBitExclusiveOrAssignInterval(IntegerInterval::Methods::
            queryBitExclusiveOrAssign(iieSource), iieSource, env);
      }
   bool apply(IntegerInterval& interval, const IntegerInterval& newSource, EvaluationEnvironment& env) const
      {  return interval.applyBitExclusiveOrAssignInterval(IntegerInterval::Methods::
            queryBitExclusiveOrAssign(iieSource), newSource, env);
      }

   const IntegerInterval& getSource() const { return iieSource; }
};

bool
IntegerInterval::applyBitExclusiveOrAssignConstant(const VirtualOperation&,
      const VirtualElement& source, EvaluationEnvironment& env) {
   QueryOperation::CompareSpecialEnvironment sourceSpecial = querySCompareSpecial(source);
   if (sourceSpecial.isZero()) {
      env.mergeVerdictExact();
      return true;
   }
   else if (sourceSpecial.isMinusOne())
      return apply(Methods::queryBitNegateAssign(source), env);

   // optimisation possible if x ^ 2^k-1 (upper common bits between a and b)
   env.mergeVerdictDegradate();
   EvaluationEnvironment::LatticeCreation creation = env.getLatticeCreation();
   if (creation.isTop() || creation.isShareTop()) {
      env.presult() = Details::IntOperationElement::Methods::newTop(*this);
      return true;
   };

   PNT::TPassPointer<VirtualDisjunction, PPAbstractElement>
      result = applySureCase(QueryOperation().setBitExclusiveOr(), source, env);

   if (!env.getInformationKind().isSure()) {
      QueryOperation::CompareSpecialEnvironment
         minSpecial = querySCompareSpecial(*ppveMin),
         maxSpecial = querySCompareSpecial(*ppveMax);
      if (minSpecial.mayBeLessZero() && maxSpecial.mayBeGreaterOrEqualZero())
         return partitionConstant(BitExclusiveOrApplication(source), result, env);
      else if (sourceSpecial.isGreaterOrEqualZero() || sourceSpecial.isLessZero()) {
         // [x excor (2^k) & ~(2^k-1), x excor (2^k) | (2^k-1)] with 2^k-1 >= b
         bool doesChangeSign = maxSpecial.isLessZero();
         QueryOperation::BitDomainEnvironment bitDomain(*this);
         query(QueryOperation().setBitDomain(), bitDomain);
         PPVirtualElement maxOne = bitDomain.maxOne();
         if (maxOne->queryZeroResult().isZero()) {
            ppveMin = newSMinDomain();
            ppveMax = newSMaxDomain();
         }
         else {
            EvaluationParameters exactStopErrorStates(EPExactStopErrorStates);
            exactStopErrorStates.setApproximatePartFrom(env);
            EvaluationParameters mayStopErrorStates(EPMayStopErrorStates);
            mayStopErrorStates.setApproximatePartFrom(env);
            auto bitExclusiveOrOperation = Methods::queryBitExclusiveOr(source);
            auto nextAssignOperation = querySNextAssignOperation();
            auto bitAndAssignOperation = Methods::queryBitAndAssign(source);
            auto bitOrAssignOperation = Methods::queryBitOrAssign(source);
            auto bitNegateOperation = Methods::queryBitNegate(source);
            PPVirtualElement sourceXor2Expk = Methods::apply(source, bitExclusiveOrOperation,
               *Methods::applyAssign(PPVirtualElement(*maxOne), nextAssignOperation, exactStopErrorStates), mayStopErrorStates);
            ppveMin = Methods::applyAssign(*sourceXor2Expk, bitAndAssignOperation,
               *Methods::apply(*maxOne, bitNegateOperation, exactStopErrorStates), mayStopErrorStates);
            ppveMax = Methods::applyAssign(sourceXor2Expk, bitOrAssignOperation, *maxOne, mayStopErrorStates);
            if (doesChangeSign) {
               PPVirtualElement minDomain = newSMinDomain();
               auto bitExclusiveOrAssignOperation = Methods::queryBitExclusiveOrAssign(source);
               ppveMin = Methods::applyAssign(ppveMin, bitExclusiveOrAssignOperation,
                  *minDomain, mayStopErrorStates);
               ppveMax = Methods::applyAssign(ppveMax, bitExclusiveOrAssignOperation,
                  *minDomain, mayStopErrorStates);
            };
         };
      }
      else { // the sign of sourceSpecial is undefined => partition into 2 + merge
         QueryOperation::BitDomainEnvironment bitDomain(*this);
         source.query(QueryOperation().setBitDomain(), bitDomain);
         PPVirtualElement maxOne = bitDomain.maxOne();
         EvaluationParameters exactStopErrorStates(EPExactStopErrorStates);
         exactStopErrorStates.setApproximatePartFrom(env);
         EvaluationParameters mayStopErrorStates(EPMayStopErrorStates);
         mayStopErrorStates.setApproximatePartFrom(env);
         auto bitExclusiveOrOperation = Methods::queryBitExclusiveOr(source);
         auto nextAssignOperation = querySNextAssignOperation();
         auto bitAndAssignOperation = Methods::queryBitAndAssign(source);
         auto bitOrAssignOperation = Methods::queryBitOrAssign(source);
         auto bitNegateOperation = Methods::queryBitNegate(source);

         PPVirtualElement sourceXor2Expk = Methods::apply(source, bitExclusiveOrOperation,
            *Methods::applyAssign(PPVirtualElement(*maxOne), nextAssignOperation, exactStopErrorStates), mayStopErrorStates);
         ppveMin = Methods::applyAssign(*sourceXor2Expk, bitAndAssignOperation,
            *Methods::apply(*maxOne, bitNegateOperation, exactStopErrorStates), mayStopErrorStates);
         ppveMax = Methods::applyAssign(sourceXor2Expk, bitOrAssignOperation, *maxOne, mayStopErrorStates);
         PPVirtualElement minDomain = newSMinDomain();

         if (result.isValid()) {
            result->addMay(Methods::newInterval(source, Init().setInterval(*ppveMin, *ppveMax).setUnsigned(queryUnsignedField())));
            auto bitExclusiveOrAssignOperation = Methods::queryBitExclusiveOrAssign(source);
            ppveMin = Methods::applyAssign(ppveMin, bitExclusiveOrAssignOperation,
                  *minDomain, mayStopErrorStates);
            ppveMax = Methods::applyAssign(ppveMax, bitExclusiveOrAssignOperation,
                  *minDomain, mayStopErrorStates);
            result->addMay(Methods::newInterval(source, Init().setInterval(ppveMin, ppveMax).setUnsigned(queryUnsignedField())));
            env.presult() = result;
         }
         else
            mergeWithInterval(IntegerInterval(Init().setInterval(
               Methods::apply(*ppveMin, bitExclusiveOrOperation, *minDomain, mayStopErrorStates),
               Methods::apply(*ppveMax, bitExclusiveOrOperation, *minDomain, mayStopErrorStates))), env);
         return true;
      };

      if (result.isValid()) {
         result->addMay(Methods::newInterval(source, Init().setInterval(ppveMin, ppveMax).setUnsigned(queryUnsignedField())));
         env.presult() = result;
      };
   }
   else
      env.presult() = result;

   return true;
}

template <class Application> bool
IntegerInterval::partitionInterval(const Application& application,
      PNT::TPassPointer<VirtualDisjunction, PPAbstractElement> result,
      EvaluationEnvironment& env, bool isBothDisjunction, bool isThisDisjunction) {
   // partition in 4 + merge after the result
   EvaluationParameters may = EvaluationParameters(env).setMay();
   TransmitEnvironment applyEnv(env, may);
   PPVirtualElement localResult;
   if (isBothDisjunction) {
      // minCompare.mayBeLessZero() && maxCompare.mayBeGreaterOrEqualZero()
      //   && minSourceCompare.mayBeLessZero() && maxSourceCompare.mayBeGreaterOrEqualZero()
      PNT::TPassPointer<IntegerInterval, PPAbstractElement>
         thisNegative, thisPositiveCopy, thisNegativeCopy,
         sourcePositive, sourceNegative;
      const VirtualElement& min = *ppveMin;
      thisNegative = Methods::newInterval(min, Init().setInterval(ppveMin, Methods::newMinusOne(*ppveMin)).setUnsigned(queryUnsignedField()));
      ppveMin = Methods::newZero(*ppveMin);
      thisNegativeCopy.fullAssign(thisNegative);
      thisPositiveCopy.setElement(*this);
      sourceNegative = Methods::newInterval(*ppveMin, Init().setInterval(*application.getSource().ppveMin,
               Methods::newMinusOne(*ppveMin)).setUnsigned(queryUnsignedField()));
      sourcePositive = Methods::newInterval(*ppveMin, 
               Init().setInterval(Methods::newZero(*ppveMin), *application.getSource().ppveMax).setUnsigned(queryUnsignedField()));

      assume(application.apply(*thisNegative, *sourceNegative, applyEnv));
      localResult = (applyEnv.hasResult()) ? (const PPAbstractElement&) applyEnv.presult()
         : (const PPAbstractElement&) thisNegative;
      assume(application.apply(*this, *sourceNegative, applyEnv));
      localResult = Methods::merge(localResult, applyEnv.hasResult()
            ? applyEnv.getResult() : *this, applyEnv);
      assume(application.apply(*thisNegativeCopy, *sourcePositive, applyEnv));
      localResult = Methods::merge(localResult, applyEnv.hasResult()
            ? applyEnv.getResult() : *thisNegativeCopy, applyEnv);
      assume(application.apply(*thisPositiveCopy, *sourcePositive, applyEnv));
      localResult = Methods::merge(localResult, applyEnv.hasResult()
            ? applyEnv.getResult() : *thisPositiveCopy, applyEnv);
   }
   else if (isThisDisjunction) {
      // minCompare.mayBeLessZero() && maxCompare.mayBeGreaterOrEqualZero()
      PNT::TPassPointer<IntegerInterval, PPAbstractElement> thisNegative;
      const VirtualElement& min = *ppveMin;
      thisNegative = Methods::newInterval(min, Init().setInterval(ppveMin,
               Methods::newMinusOne(min)).setUnsigned(queryUnsignedField()));
      ppveMin = Methods::newZero(min);

      assume(application.apply(*thisNegative, applyEnv));
      localResult = (const PPVirtualElement&) (applyEnv.hasResult() ? applyEnv.presult() : thisNegative);
      assume(application.apply(*this, applyEnv));
      localResult = Methods::merge(localResult, applyEnv.hasResult()
            ? applyEnv.getResult() : *this, applyEnv);
   }
   else { // minSourceCompare.mayBeLessZero() && maxSourceCompare.mayBeGreaterOrEqualZero()
      PNT::TPassPointer<IntegerInterval, PPAbstractElement> sourcePositive, sourceNegative, thisCopy;
      thisCopy.setElement(*this);
      sourceNegative = Methods::newInterval(*ppveMin, Init().setInterval(*application.getSource().ppveMin,
               Methods::newMinusOne(*ppveMin)).setUnsigned(queryUnsignedField()));
      sourcePositive = Methods::newInterval(*ppveMin, 
               Init().setInterval(Methods::newZero(*ppveMin), *application.getSource().ppveMax).setUnsigned(queryUnsignedField()));

      assume(application.apply(*thisCopy, *sourceNegative, applyEnv));
      localResult = (applyEnv.hasResult()) ? (const PPAbstractElement&) applyEnv.presult()
         : (const PPAbstractElement&) thisCopy;
      assume(application.apply(*this, *sourcePositive, applyEnv));
      localResult = Methods::merge(localResult, applyEnv.hasResult()
            ? applyEnv.getResult() : *this, applyEnv);
   };

   if (result.isValid()) {
      result->addMay(localResult);
      env.presult() = result;
   }
   else
      env.presult() = localResult;
   return true;
}

bool
IntegerInterval::applyBitOrAssignInterval(const VirtualOperation& operation,
      const IntegerInterval& source, EvaluationEnvironment& env) {
   if (queryUnsignedField() != source.queryUnsignedField()) {
      PNT::TPassPointer<VirtualDisjunction, PPAbstractElement> copy = changeSign(env);
      if (copy.isValid()) {
         TransmitEnvironment applyEnv(env);
         applyEnv.setFirstArgument(source);
         assume(copy->apply(operation, applyEnv));
         env.presult() = applyEnv.hasResult() ? applyEnv.presult() : (PPAbstractElement) copy;
         return true;
      };
   };
   env.mergeVerdictDegradate(); // [a,b]|[b,d]
   EvaluationEnvironment::LatticeCreation creation = env.getLatticeCreation();
   if (creation.isTop() || creation.isShareTop()) {
      env.presult() = Details::IntOperationElement::Methods::newTop(*this);
      return true;
   };

   PNT::TPassPointer<VirtualDisjunction, PPAbstractElement>
      result = applySureCaseInterval(QueryOperation().setBitOr(), source, env);

   if (!env.getInformationKind().isSure()) {
      QueryOperation::CompareSpecialEnvironment
         minCompare = querySCompareSpecial(*ppveMin),
         maxCompare = querySCompareSpecial(*ppveMax),
         minSourceCompare = querySCompareSpecial(*source.ppveMin),
         maxSourceCompare = querySCompareSpecial(*source.ppveMax);

      if ((minCompare.mayBeLessZero() && maxCompare.mayBeGreaterOrEqualZero())
            || (minSourceCompare.mayBeLessZero() && maxSourceCompare.mayBeGreaterOrEqualZero()))
         return partitionInterval(BitOrIntervalApplication(source), result, env,
            minCompare.mayBeLessZero() && maxCompare.mayBeGreaterOrEqualZero()
               && minSourceCompare.mayBeLessZero() && maxSourceCompare.mayBeGreaterOrEqualZero(),
            minCompare.mayBeLessZero() && maxCompare.mayBeGreaterOrEqualZero());
      else {
         QueryOperation::BitDomainEnvironment bitThisDomain(*this), bitSourceDomain(source);
         query(QueryOperation().setBitDomain(), bitThisDomain);
         source.query(QueryOperation().setBitDomain(), bitSourceDomain);
         EvaluationParameters mayStopErrorStates(EPMayStopErrorStates);
         mayStopErrorStates.setApproximatePartFrom(env);
         auto maxAssignOperation = querySMaxAssignOperation();
         if ((minSourceCompare.isGreaterOrEqualZero() && minCompare.isGreaterOrEqualZero())
               || (maxSourceCompare.isLessZero() && maxCompare.isLessZero()))
            // [max(c,a), 2^k-1] with 2^k-1 >= max(b,d)
            ppveMin = Methods::applyAssign(ppveMin, maxAssignOperation, *source.ppveMin, mayStopErrorStates);
         else if (maxSourceCompare.isLessZero()) { // minCompare.isGreaterOrEqualZero()
            // [max(c, a | 10..0), (2^k-1)] with 2^k-1 >= max(b|10..0,d)
            EvaluationParameters exactStopErrorStates(EPExactStopErrorStates);
            exactStopErrorStates.setApproximatePartFrom(env);
            VirtualElement& min = *ppveMin;
            auto rightShiftAssignOperation = querySRightShiftAssignOperation();
            auto bitOrAssignOperation =  Methods::queryBitOrAssign(min);
            ppveMin = Methods::applyAssign(ppveMin, bitOrAssignOperation,
               *Methods::applyAssign(Methods::newOne(min), rightShiftAssignOperation,
                  *Methods::newIntForShift(min, min.getSizeInBits()-1), exactStopErrorStates), mayStopErrorStates);
            ppveMin = Methods::applyAssign(ppveMin, maxAssignOperation, *source.ppveMin, mayStopErrorStates);
            // bitThisDomain.setNegative();
         }
         else { // sourceSpecial.isGreaterOrEqualZero() && maxSpecial.isLessZero()
            // [max(x|10..0, a), x|(2^k-1)] with 2^k-1 >= b
            auto rightShiftAssignOperation = querySRightShiftAssignOperation();
            auto bitOrOperation = Methods::queryBitOr(*ppveMin);
            PPVirtualElement oldMin = ppveMin;
            EvaluationParameters exactStopErrorStates(EPExactStopErrorStates);
            exactStopErrorStates.setApproximatePartFrom(env);
            ppveMin = Methods::apply(*source.ppveMin, bitOrOperation,
               *Methods::applyAssign(Methods::newOne(*ppveMax),
                  rightShiftAssignOperation,
                  *Methods::newIntForShift(*ppveMax, ppveMax->getSizeInBits()-1), exactStopErrorStates), mayStopErrorStates);
            if (ppveMin.isValid())
               ppveMin = Methods::applyAssign(ppveMin, maxAssignOperation, *oldMin, mayStopErrorStates);
            else
               env.setEmpty();
            // bitSourceDomain.setNegative();
         };
         ppveMax = (bitThisDomain.lastPos() <= bitSourceDomain.lastPos())
            ? bitSourceDomain.maxOne() : bitThisDomain.maxOne();
      };

      if (result.isValid()) {
         const VirtualElement& min = *ppveMin;
         result->addMay(Methods::newInterval(min, Init().setInterval(ppveMin, ppveMax).setUnsigned(queryUnsignedField())));
         env.presult() = result;
      };
   }
   else
      env.presult() = result;

   return true;
}

bool
IntegerInterval::applyBitAndAssignInterval(const VirtualOperation& operation,
      const IntegerInterval& source, EvaluationEnvironment& env) {
   if (queryUnsignedField() != source.queryUnsignedField()) {
      PNT::TPassPointer<VirtualDisjunction, PPAbstractElement> copy = changeSign(env);
      if (copy.isValid()) {
         TransmitEnvironment applyEnv(env);
         applyEnv.setFirstArgument(source);
         assume(copy->apply(operation, applyEnv));
         env.presult() = applyEnv.hasResult() ? applyEnv.presult() : (PPAbstractElement) copy;
         return true;
      };
   };
   env.mergeVerdictDegradate(); // [a,b]&[b,d]
   EvaluationEnvironment::LatticeCreation creation = env.getLatticeCreation();
   if (creation.isTop() || creation.isShareTop()) {
      env.presult() = Details::IntOperationElement::Methods::newTop(*this);
      return true;
   };

   PNT::TPassPointer<VirtualDisjunction, PPAbstractElement>
      result = applySureCaseInterval(QueryOperation().setBitAnd(), source, env);

   if (!env.getInformationKind().isSure()) {
      QueryOperation::CompareSpecialEnvironment
         minCompare = querySCompareSpecial(*ppveMin),
         maxCompare = querySCompareSpecial(*ppveMax),
         minSourceCompare = querySCompareSpecial(*source.ppveMin),
         maxSourceCompare = querySCompareSpecial(*source.ppveMax);

      if ((minCompare.mayBeLessZero() && maxCompare.mayBeGreaterOrEqualZero())
            || (minSourceCompare.mayBeLessZero() && maxSourceCompare.mayBeGreaterOrEqualZero()))
         return partitionInterval(BitAndIntervalApplication(source), result, env,
            minCompare.mayBeLessZero() && maxCompare.mayBeGreaterOrEqualZero()
               && minSourceCompare.mayBeLessZero() && maxSourceCompare.mayBeGreaterOrEqualZero(),
            minCompare.mayBeLessZero() && maxCompare.mayBeGreaterOrEqualZero());
      else if ((minCompare.isGreaterOrEqualZero() && minSourceCompare.isGreaterOrEqualZero())
            || (maxCompare.isLessZero() && maxSourceCompare.isLessZero())) {
         // [0, min(b,d)] or [Minint, min(b,d)]
         EvaluationParameters mayStopErrorStates(EPMayStopErrorStates);
         mayStopErrorStates.setApproximatePartFrom(env);
         ppveMin = minCompare.isGreaterOrEqualZero() ? Methods::newZero(*ppveMin) : newSMinDomain();
         ppveMax = Methods::applyAssign(ppveMax, querySMinAssignOperation(), *source.ppveMax, mayStopErrorStates);
      }
      else if (maxSourceCompare.isLessZero()) // minCompare.isGreaterOrEqualZero() -> [0, b]
         ppveMin = Methods::newZero(*ppveMin);
      else { // minSourceCompare.isGreaterOrEqualZero() && maxCompare.isLessZero() -> [0,d]
         ppveMin = Methods::newZero(*ppveMin);
         ppveMax.fullAssign(source.ppveMax);
      };

      if (result.isValid()) {
         const VirtualElement& min = *ppveMin;
         result->addMay(Methods::newInterval(min, Init().setInterval(ppveMin, ppveMax).setUnsigned(queryUnsignedField())));
         env.presult() = result;
      };
   }
   else
      env.presult() = result;

   return true;
}

bool
IntegerInterval::applyBitExclusiveOrAssignInterval(const VirtualOperation& operation,
      const IntegerInterval& source, EvaluationEnvironment& env) {
   if (queryUnsignedField() != source.queryUnsignedField()) {
      PNT::TPassPointer<VirtualDisjunction, PPAbstractElement> copy = changeSign(env);
      if (copy.isValid()) {
         TransmitEnvironment applyEnv(env);
         applyEnv.setFirstArgument(source);
         assume(copy->apply(operation, applyEnv));
         env.presult() = applyEnv.hasResult() ? applyEnv.presult() : (PPAbstractElement) copy;
         return true;
      };
   };
   env.mergeVerdictDegradate(); // [a,b]^[b,d]
   EvaluationEnvironment::LatticeCreation creation = env.getLatticeCreation();
   if (creation.isTop() || creation.isShareTop()) {
      env.presult() = Details::IntOperationElement::Methods::newTop(*this);
      return true;
   };

   PNT::TPassPointer<VirtualDisjunction, PPAbstractElement> 
      result = applySureCaseInterval(QueryOperation().setBitExclusiveOr(), source, env);

   if (!env.getInformationKind().isSure()) {
      QueryOperation::CompareSpecialEnvironment
         minCompare = querySCompareSpecial(*ppveMin),
         maxCompare = querySCompareSpecial(*ppveMax),
         minSourceCompare = querySCompareSpecial(*source.ppveMin),
         maxSourceCompare = querySCompareSpecial(*source.ppveMax);

      if ((minCompare.mayBeLessZero() && maxCompare.mayBeGreaterOrEqualZero())
            || (minSourceCompare.mayBeLessZero() && maxSourceCompare.mayBeGreaterOrEqualZero()))
         return partitionInterval(BitExclusiveOrIntervalApplication(source), result, env,
            minCompare.mayBeLessZero() && maxCompare.mayBeGreaterOrEqualZero()
               && minSourceCompare.mayBeLessZero() && maxSourceCompare.mayBeGreaterOrEqualZero(),
            minCompare.mayBeLessZero() && maxCompare.mayBeGreaterOrEqualZero());
      else {
         // [0, max(b,d)], if b,d >= 0
         QueryOperation::BitDomainEnvironment thisBits(*this), sourceBits(source);
         query(QueryOperation().setBitDomain(), thisBits);
         source.query(QueryOperation().setBitDomain(), sourceBits);
         sourceBits.fixedPrefix().neg().neg(sourceBits.lastPos());
         thisBits.mergeWith(sourceBits);
         ppveMax = thisBits.maxOne();
      };

      if (result.isValid()) {
         const VirtualElement& min = *ppveMin;
         result->addMay(Methods::newInterval(min, Init().setInterval(ppveMin, ppveMax).setUnsigned(queryUnsignedField())));
         env.presult() = result;
      };
   }
   else
      env.presult() = result;

   return true;
}

bool
IntegerInterval::applyLeftShiftAssignConstant(const VirtualOperation& operation,
      const VirtualElement& source, EvaluationEnvironment& env) {
   EvaluationEnvironment::LatticeCreation creation = env.getLatticeCreation();
   if (creation.isFormal() || creation.isDisjunction()) {
      QueryOperation::SimplificationEnvironment simplification;
      query(QueryOperation().setSimplificationAsConstantDisjunction(), simplification);
      if (!simplification.hasFailed() && simplification.hasResult()) {
         bool result = simplification.presult()->apply(operation, env);
         if (result) { 
            if (!env.hasResult())
               env.presult() = simplification.presult();
            return result;
         };
      };
   };

   env.mergeVerdictDegradate();
   if (creation.isTop() || creation.isShareTop()) {
      env.presult() = Details::IntOperationElement::Methods::newTop(*this);
      return true;
   };

   PNT::TPassPointer<VirtualDisjunction, PPAbstractElement> 
      result = applySureCase(QueryOperation().setLeftShift(), source, env);
   if (env.getInformationKind().isSure()) {
      env.presult() = result;
      return true;
   };

   EvaluationParameters may = EvaluationParameters(env).setMay();
   TransmitEnvironment minEnv(env, may), maxEnv(env, may);
   auto leftShiftAssignOperation = Methods::queryLeftShiftAssign(source);
   ppveMin = Methods::applyAssign(ppveMin, leftShiftAssignOperation, source, minEnv);
   ppveMax = Methods::applyAssign(ppveMax, leftShiftAssignOperation, source, maxEnv);
   DefineSetTopOnError
   return true;
}

bool
IntegerInterval::applyLeftShiftAssignInterval(const VirtualOperation& operation,
      const IntegerInterval& source, EvaluationEnvironment& env) {
   if (!source.hasUnsignedField()) {
      PNT::TPassPointer<VirtualDisjunction, PPAbstractElement> copy = changeSign(env);
      if (copy.isValid()) {
         TransmitEnvironment applyEnv(env);
         applyEnv.setFirstArgument(source);
         assume(copy->apply(operation, applyEnv));
         env.presult() = applyEnv.hasResult() ? applyEnv.presult() : (PPAbstractElement) copy;
         return true;
      };
   };
   // multiplication by a power of 2
   env.mergeVerdictDegradate();
   EvaluationEnvironment::LatticeCreation creation = env.getLatticeCreation();
   if (creation.isTop() || creation.isShareTop()) {
      env.presult() = Details::IntOperationElement::Methods::newTop(*this);
      return true;
   };

   PNT::TPassPointer<VirtualDisjunction, PPAbstractElement> 
      result = applySureCaseInterval(QueryOperation().setLeftShift(), source, env);
   if (env.getInformationKind().isSure()) {
      env.presult() = result;
      return true;
   };

   EvaluationParameters may = EvaluationParameters(env).setMay();
   TransmitEnvironment minEnv(env, may), maxEnv(env, may);
   auto leftShiftAssignOperation = Methods::queryLeftShiftAssign(*ppveMin);
   ppveMin = Methods::applyAssign(ppveMin, leftShiftAssignOperation, *source.ppveMin, minEnv);
   ppveMax = Methods::applyAssign(ppveMax, leftShiftAssignOperation, *source.ppveMax, maxEnv);
   DefineSetTopOnError
   return true;
}

bool
IntegerInterval::applyRightShiftAssignConstant(const VirtualOperation&,
      const VirtualElement& source, EvaluationEnvironment& env) {
   env.mergeVerdictExact();
   TransmitEnvironment applyEnv(env);
   auto rightShiftAssignOperation = querySRightShiftAssignOperation();
   ppveMin = Methods::applyAssign(ppveMin, rightShiftAssignOperation, source, applyEnv);
   ppveMax = Methods::applyAssign(ppveMax, rightShiftAssignOperation, source, applyEnv);
   if (applyEnv.hasEvaluationError()) {
      PNT::TPassPointer<VirtualDisjunction, PPAbstractElement>
         result = applySureCase(QueryOperation().setRightShift(), source, env);
      if (!env.getInformationKind().isSure()) {
         if (result.isValid()) {
            result->addMay(Methods::newTop(source));
            env.presult() = result;
         }
         else
            env.presult() = Methods::newTop(source);
      }
      else
         env.presult() = result;
   };
   return true;
}

bool
IntegerInterval::applyRightShiftAssignInterval(const VirtualOperation& operation,
      const IntegerInterval& source, EvaluationEnvironment& env) {
   if (!source.hasUnsignedField()) {
      PNT::TPassPointer<VirtualDisjunction, PPAbstractElement> copy = changeSign(env);
      if (copy.isValid()) {
         TransmitEnvironment applyEnv(env);
         applyEnv.setFirstArgument(source);
         assume(copy->apply(operation, applyEnv));
         env.presult() = applyEnv.hasResult() ? applyEnv.presult() : (PPAbstractElement) copy;
         return true;
      };
   };
   // division by a power of 2
   env.mergeVerdictExact();
   EvaluationParameters exactStopErrorStates(EPExactStopErrorStates);
   exactStopErrorStates.setApproximatePartFrom(env);
   EvaluationParameters mayStopErrorStates(EPMayStopErrorStates);
   mayStopErrorStates.setApproximatePartFrom(env);
   PPVirtualElement sign = Methods::applyAssign(Methods::newOne(*ppveMin),
         Methods::queryLeftShiftAssign(*ppveMin),
         *Methods::newIntForShift(*ppveMin, ppveMin->getSizeInBits()-1), exactStopErrorStates);
   QueryOperation::CompareSpecialEnvironment
      minCompare = querySCompareSpecial(
         *Methods::apply(*ppveMin, Methods::queryBitAnd(*ppveMin), *sign, mayStopErrorStates)),
      maxCompare = querySCompareSpecial(
         *Methods::apply(*ppveMax, Methods::queryBitAnd(*ppveMin), *sign, mayStopErrorStates));

   bool isExact = Methods::applyBoolean(*source.ppveMin, querySCompareGreaterOrEqualOperation(),
         *source.ppveMax);
   PNT::TPassPointer<VirtualDisjunction, PPAbstractElement> disjunctionNotExact;

   bool isDisjunctionExact = true;
   if (!isExact && maxCompare.mayBeZero() && minCompare.mayBeZero()) { // b >> 1 >= a - (1<<c)
      PPVirtualElement maxDivideTwo = Methods::applyAssign(PPVirtualElement(*ppveMax),
         querySRightShiftAssignOperation(), *Methods::newIntForShift(*ppveMin, 1), mayStopErrorStates);
      PPVirtualElement minMinusTwoExponentSourceMin
         = Methods::applyAssign(PPVirtualElement(*ppveMin), querySMinusAssignOperation(),
            *Methods::applyAssign(Methods::newOne(*ppveMin),
               Methods::queryLeftShiftAssign(*ppveMin), *source.ppveMin, mayStopErrorStates),
            mayStopErrorStates);
      if (minMinusTwoExponentSourceMin.isValid())
         isDisjunctionExact = Methods::applyBoolean(*maxDivideTwo,
               querySCompareGreaterOrEqualOperation(), *minMinusTwoExponentSourceMin);
   };
   if (isDisjunctionExact && !isExact && maxCompare.mayBeDifferentZero()
         && minCompare.mayBeDifferentZero()) {
      // a >> 1 >= b + (1<<c)
      PPVirtualElement minDivideTwo = Methods::applyAssign(PPVirtualElement(*ppveMin),
         querySRightShiftAssignOperation(), *Methods::newIntForShift(*ppveMin, 1), mayStopErrorStates);
      PPVirtualElement maxPlusTwoExponentSourceMin = Methods::applyAssign(
         PPVirtualElement(*ppveMax), querySPlusAssignOperation(),
            *Methods::applyAssign(Methods::newOne(*ppveMin),
               Methods::queryLeftShiftAssign(*ppveMax), *source.ppveMin, mayStopErrorStates),
         mayStopErrorStates);
      if (maxPlusTwoExponentSourceMin.isValid())
         isDisjunctionExact = Methods::applyBoolean(*minDivideTwo,
               querySCompareGreaterOrEqualOperation(), *maxPlusTwoExponentSourceMin);
      else
         isDisjunctionExact = false;
   };
   if (maxCompare.mayBeDifferentZero() && minCompare.mayBeZero())
      isDisjunctionExact = false;
   if ((isExact == isDisjunctionExact) == false) {
      disjunctionNotExact = applySureCaseInterval(QueryOperation().setRightShift(), source, env);
      env.mergeVerdictDegradate();
      if (env.getInformationKind().isSure()) {
         env.presult() = disjunctionNotExact;
         return true;
      };
   };

   TransmitEnvironment applyEnv(env);
   PPVirtualElement
      newMin = Methods::applyAssign(PPVirtualElement(*ppveMin), querySRightShiftAssignOperation(),
         minCompare.isZero() ? *source.ppveMax : *source.ppveMin, applyEnv),
      newMax = Methods::applyAssign(PPVirtualElement(*ppveMax), querySRightShiftAssignOperation(),
         maxCompare.isZero() ? *source.ppveMin : *source.ppveMax, applyEnv);
   if (applyEnv.hasEvaluationError()) {
      if (!isExact) {
         disjunctionNotExact = applySureCaseInterval(QueryOperation().setRightShift(), source, env);
         env.mergeVerdictDegradate();
         if (env.getInformationKind().isSure())
            return true;
      };

      if (disjunctionNotExact.isValid()) {
         disjunctionNotExact->addMay(Methods::newTop(*ppveMin));
         env.presult() = disjunctionNotExact;
      }
      else
         env.presult() = Methods::newTop(*ppveMin);
      return true;
   }
   else {
      ppveMin = newMin;
      ppveMax = newMax;
   };

   if (disjunctionNotExact.isValid()) {
      const VirtualElement& min = *ppveMin;
      disjunctionNotExact->addMay(Methods::newInterval(min, Init().setInterval(ppveMin, ppveMax).setUnsigned(queryUnsignedField())));
      env.presult() = disjunctionNotExact;
   };
   return true;
}

#undef DefineSetTopOnError

bool
IntegerInterval::applyBitNegateAssign(const VirtualOperation& operation, EvaluationEnvironment& env) {
   env.mergeVerdictExact();
   auto bitNegateAssign = Methods::queryBitNegateAssign(*ppveMin);
   PPVirtualElement temp = Methods::applyAssign(ppveMin, bitNegateAssign, env);
   ppveMin = Methods::applyAssign(ppveMax, bitNegateAssign, env);
   ppveMax = temp;
   return true;
}

bool
IntegerInterval::applyLogicalAndAssignConstant(const VirtualOperation&,
      const VirtualElement& source, EvaluationEnvironment& env) {
   env.mergeVerdictExact();
   QueryOperation::CompareSpecialEnvironment sourceCompare = querySCompareSpecial(source);
   if (sourceCompare.isZero()) {
      env.presult() = Methods::newFalse(source);
      return true;
   };

   QueryOperation::CompareSpecialEnvironment
      minCompare = querySCompareSpecial(*ppveMin), maxCompare = querySCompareSpecial(*ppveMax);
   if (minCompare.isZero() && maxCompare.isZero()) {
      env.presult() = Methods::newFalse(source);
      return true;
   };
   if (sourceCompare.isNotZero() && (minCompare.isGreaterZero() || (maxCompare.isLessZero()))) {
      env.presult() = Methods::newTrue(source);
      return true;
   };

   PNT::TPassPointer<VirtualDisjunction, PPAbstractElement> disjunction
      = Methods::newBooleanDisjunction(source);
   if (minCompare.isLessOrEqualZero() && maxCompare.isGreaterOrEqualZero()) {
      disjunction->add(Methods::newFalse(source), env.getInformationKind());
      if ((minCompare.isNotZero() || maxCompare.isNotZero())
            && sourceCompare.isNotZero()) {
         disjunction->add(Methods::newTrue(source), env.getInformationKind());
         return true;
      };
      env.mergeVerdictDegradate();
      if (!env.getInformationKind().isSure())
         disjunction->addMay(Methods::newTrue(source));
      env.presult() = disjunction;
      return true;
   };
   if (!env.getInformationKind().isSure()) {
      disjunction->addMay(Methods::newFalse(source));
      disjunction->addMay(Methods::newTrue(source));
      env.presult() = disjunction;
   }
   else
      env.setEmpty();
   return true;
}

bool
IntegerInterval::applyLogicalOrAssignConstant(const VirtualOperation&,
      const VirtualElement& source, EvaluationEnvironment& env) {
   env.mergeVerdictExact();
   QueryOperation::CompareSpecialEnvironment sourceCompare = querySCompareSpecial(source);
   if (sourceCompare.isNotZero()) {
      env.presult() = Methods::newTrue(source);
      return true;
   };

   QueryOperation::CompareSpecialEnvironment
      minCompare = querySCompareSpecial(*ppveMin), maxCompare = querySCompareSpecial(*ppveMax);
   if (minCompare.isGreaterZero() || (maxCompare.isLessZero())) {
      env.presult() = Methods::newTrue(source);
      return true;
   };
   if (sourceCompare.isZero() && minCompare.isZero() && maxCompare.isZero()) {
      env.presult() = Methods::newFalse(source);
      return true;
   };

   PNT::TPassPointer<VirtualDisjunction, PPAbstractElement> disjunction = Methods::newBooleanDisjunction(source);
   if (minCompare.isNotZero() || maxCompare.isNotZero()
         || sourceCompare.isNotZero()) {
      disjunction->add(Methods::newTrue(source), env.getInformationKind());
      if (minCompare.isLessOrEqualZero() && maxCompare.isGreaterOrEqualZero()
            && sourceCompare.isNotZero()) {
         disjunction->add(Methods::newFalse(source), env.getInformationKind());
         env.presult() = disjunction;
         return true;
      };
      env.mergeVerdictDegradate();
      if (!env.getInformationKind().isSure())
         disjunction->addMay(Methods::newFalse(source));
      env.presult() = disjunction;
      return true;
   };
   if (!env.getInformationKind().isSure()) {
      disjunction->addMay(Methods::newFalse(source));
      disjunction->addMay(Methods::newTrue(source));
      env.presult() = disjunction;
   }
   else
      env.setEmpty();
   return true;
}

bool
IntegerInterval::applyLogicalAndAssignInterval(const VirtualOperation&,
      const IntegerInterval& source, EvaluationEnvironment& env) {
   env.mergeVerdictExact();
   QueryOperation::CompareSpecialEnvironment
      minSourceCompare = source.querySCompareSpecial(*source.ppveMin),
      maxSourceCompare = source.querySCompareSpecial(*source.ppveMax),
      minCompare = querySCompareSpecial(*ppveMin),
      maxCompare = querySCompareSpecial(*ppveMax);
   if ((minCompare.isZero() && maxSourceCompare.isZero())
         || (minCompare.isZero() && maxCompare.isZero())) {
      env.presult() = Methods::newFalse(*ppveMin);
      return true;
   };
   if ((minSourceCompare.isLessZero() || maxSourceCompare.isGreaterZero())
         && (minCompare.isGreaterZero() || (maxCompare.isLessZero()))) {
      env.presult() = Methods::newTrue(*ppveMin);
      return true;
   };

   PNT::TPassPointer<VirtualDisjunction, PPAbstractElement> disjunction = Methods::newBooleanDisjunction(*ppveMin);
   if ((minCompare.isLessOrEqualZero() && maxCompare.isGreaterOrEqualZero())
         || (minSourceCompare.isLessOrEqualZero() && maxSourceCompare.isGreaterOrEqualZero())) {
      disjunction->add(Methods::newFalse(*ppveMin), env.getInformationKind());
      if ((minCompare.isNotZero() || maxCompare.isNotZero())
            && (minSourceCompare.isNotZero() || maxSourceCompare.isNotZero())) {
         disjunction->add(Methods::newTrue(*ppveMin), env.getInformationKind());
         env.presult() = disjunction;
         return true;
      };
      env.mergeVerdictDegradate();
      if (!env.getInformationKind().isSure())
         disjunction->addMay(Methods::newTrue(*ppveMin));
      env.presult() = disjunction;
      return true;
   };
   if (!env.getInformationKind().isSure()) {
      disjunction->addMay(Methods::newFalse(*ppveMin));
      disjunction->addMay(Methods::newTrue(*ppveMin));
      env.presult() = disjunction;
   }
   else
      env.setEmpty();
   return true;
}

bool
IntegerInterval::applyLogicalOrAssignInterval(const VirtualOperation&,
      const IntegerInterval& source, EvaluationEnvironment& env) {
   env.mergeVerdictExact();
   QueryOperation::CompareSpecialEnvironment
      minSourceCompare = source.querySCompareSpecial(*source.ppveMin),
      maxSourceCompare = source.querySCompareSpecial(*source.ppveMax),
      minCompare = querySCompareSpecial(*ppveMin),
      maxCompare = querySCompareSpecial(*ppveMax);
   if (minCompare.isGreaterZero() || maxCompare.isLessZero()
         || minSourceCompare.isGreaterZero() || maxSourceCompare.isLessZero()) {
      env.presult() = Methods::newTrue(*ppveMin);
      return true;
   };
   if (minSourceCompare.isZero() && maxSourceCompare.isZero()
         && minCompare.isZero() && maxCompare.isZero()) {
      env.presult() = Methods::newFalse(*ppveMin);
      return true;
   };

   PNT::TPassPointer<VirtualDisjunction, PPAbstractElement> disjunction = Methods::newBooleanDisjunction(*ppveMin);
   if (minCompare.isNotZero() || maxCompare.isNotZero()
         || minSourceCompare.isNotZero() || maxSourceCompare.isNotZero()) {
      disjunction->add(Methods::newTrue(*ppveMin), env.getInformationKind());
      if (minCompare.isLessOrEqualZero() && maxCompare.isGreaterOrEqualZero()
            && minSourceCompare.isLessOrEqualZero() && maxSourceCompare.isGreaterOrEqualZero()) {
         disjunction->add(Methods::newFalse(*ppveMin), env.getInformationKind());
         env.presult() = disjunction;
         return true;
      };
      env.mergeVerdictDegradate();
      if (!env.getInformationKind().isSure())
         disjunction->addMay(Methods::newFalse(*ppveMin));
      env.presult() = disjunction;
      return true;
   };
   if (!env.getInformationKind().isSure()) {
      disjunction->addMay(Methods::newFalse(*ppveMin));
      disjunction->addMay(Methods::newTrue(*ppveMin));
      env.presult() = disjunction;
   }
   else
      env.setEmpty();
   return true;
}

bool
IntegerInterval::applyLogicalNegateAssign(const VirtualOperation&, EvaluationEnvironment& env) {
   env.mergeVerdictExact();
   QueryOperation::CompareSpecialEnvironment
      minCompare = querySCompareSpecial(*ppveMin),
      maxCompare = querySCompareSpecial(*ppveMax);
   if (minCompare.isGreaterZero() || maxCompare.isLessZero())
      env.presult() = Methods::newFalse(*ppveMin);
   else if (minCompare.isZero() && maxCompare.isZero())
      env.presult() = Methods::newTrue(*ppveMin);
   else {
      PNT::TPassPointer<VirtualDisjunction, PPAbstractElement> disjunction = Methods::newBooleanDisjunction(*ppveMin);
      if (minCompare.isNotZero() || maxCompare.isNotZero()) {
         disjunction->add(Methods::newFalse(*ppveMin), env.getInformationKind());
         if (minCompare.isLessOrEqualZero() && maxCompare.isGreaterOrEqualZero())
            disjunction->add(Methods::newFalse(*ppveMin), env.getInformationKind());
         else {
            env.mergeVerdictDegradate();
            if (!env.getInformationKind().isSure())
               disjunction->addMay(Methods::newFalse(*ppveMin));
         };
         env.presult() = disjunction;
      }
      else if (!env.getInformationKind().isSure()) {
         disjunction->addMay(Methods::newFalse(*ppveMin));
         disjunction->addMay(Methods::newTrue(*ppveMin));
         env.presult() = disjunction;
      }
      else
         env.setEmpty();
   };
   return true;
}

/**********************/
/* constraint methods */
/**********************/
// The operations on the constant lattice are no more called and are considered
// as dead code. They are only informative.
//

bool
IntegerInterval::constraintCompareLessConstant(const VirtualOperation&, VirtualElement& source,
      Argument arg, ConstraintEnvironment& env) { // [a,b] < x forced -> [a, min(x-1,b)]
   AssumeCondition(source.getApproxKind().isConstant())
   env.mergeVerdictExact();
   if (Methods::applyBoolean(*ppveMax, querySCompareLessOperation(), source))
      return true;
   EvaluationParameters natural = EvaluationApplyParameters(env).stopErrorStates();
   ppveMax = Methods::applyAssign(ppveMax, querySMinAssignOperation(),
      *Methods::applyAssign(PPVirtualElement(source), querySPrevAssignOperation(), natural), natural);

   QueryOperation::SimplificationEnvironment simplification;
   query(QueryOperation().setSimplification(), simplification);
   if (simplification.hasResult())
      env.absorb(simplification.presult().extractElement(), arg);
   return true;
}

bool
IntegerInterval::constraintCompareLessSignedConstant(const VirtualOperation&, VirtualElement& source,
      Argument arg, ConstraintEnvironment& env) { // [a,b] < x forced -> [a, min(x-1,b)]
   AssumeCondition(source.getApproxKind().isConstant())
   if (hasUnsignedField())
      {  AssumeUnimplemented }
   env.mergeVerdictExact();
   if (Methods::applyBoolean(*ppveMax, Methods::queryCompareLessSigned(source), source))
      return true;
   EvaluationParameters natural = EvaluationApplyParameters(env).stopErrorStates();
   ppveMax = Methods::applyAssign(ppveMax, Methods::queryMinSignedAssign(source),
      *Methods::applyAssign(PPVirtualElement(source), Methods::queryPrevSignedAssign(source), natural), natural);

   QueryOperation::SimplificationEnvironment simplification;
   query(QueryOperation().setSimplification(), simplification);
   if (simplification.hasResult())
      env.absorb(simplification.presult().extractElement(), arg);
   return true;
}

bool
IntegerInterval::constraintCompareLessUnsignedConstant(const VirtualOperation&, VirtualElement& source,
      Argument arg, ConstraintEnvironment& env) { // [a,b] < x forced -> [a, min(x-1,b)]
   AssumeCondition(source.getApproxKind().isConstant())
   if (!hasUnsignedField())
      {  AssumeUnimplemented }
   env.mergeVerdictExact();
   if (Methods::applyBoolean(*ppveMax, Methods::queryCompareLessUnsigned(source), source))
      return true;
   EvaluationParameters natural = EvaluationApplyParameters(env).stopErrorStates();
   ppveMax = Methods::applyAssign(ppveMax, Methods::queryMinUnsignedAssign(source),
      *Methods::applyAssign(PPVirtualElement(source), Methods::queryPrevUnsignedAssign(source), natural), natural);

   QueryOperation::SimplificationEnvironment simplification;
   query(QueryOperation().setSimplification(), simplification);
   if (simplification.hasResult())
      env.absorb(simplification.presult().extractElement(), arg);
   return true;
}

bool
IntegerInterval::constraintCompareLessOrEqualConstant(const VirtualOperation&, VirtualElement& source,
      Argument arg, ConstraintEnvironment& env) { // [a,b] <= x forced -> [a, min(x,b)]
   AssumeCondition(source.getApproxKind().isConstant())
   env.mergeVerdictExact();
   if (Methods::applyBoolean(*ppveMax, querySCompareLessOrEqualOperation(), source))
      return true;
   ppveMax = Methods::applyAssign(ppveMax, querySMinAssignOperation(), source, env);

   QueryOperation::SimplificationEnvironment simplification;
   query(QueryOperation().setSimplification(), simplification);
   if (simplification.hasResult())
      env.absorb(simplification.presult().extractElement(), arg);
   return true;
}

bool
IntegerInterval::constraintCompareLessOrEqualSignedConstant(const VirtualOperation&, VirtualElement& source,
      Argument arg, ConstraintEnvironment& env) { // [a,b] <= x forced -> [a, min(x,b)]
   AssumeCondition(source.getApproxKind().isConstant())
   if (hasUnsignedField())
      {  AssumeUnimplemented }
   env.mergeVerdictExact();
   if (Methods::applyBoolean(*ppveMax, Methods::queryCompareLessOrEqualSigned(source), source))
      return true;
   ppveMax = Methods::applyAssign(ppveMax, Methods::queryMinSignedAssign(source), source, env);

   QueryOperation::SimplificationEnvironment simplification;
   query(QueryOperation().setSimplification(), simplification);
   if (simplification.hasResult())
      env.absorb(simplification.presult().extractElement(), arg);
   return true;
}

bool
IntegerInterval::constraintCompareLessOrEqualUnsignedConstant(const VirtualOperation&, VirtualElement& source,
      Argument arg, ConstraintEnvironment& env) { // [a,b] <= x forced -> [a, min(x,b)]
   AssumeCondition(source.getApproxKind().isConstant())
   if (!hasUnsignedField())
      {  AssumeUnimplemented }
   env.mergeVerdictExact();
   if (Methods::applyBoolean(*ppveMax, Methods::queryCompareLessOrEqualUnsigned(source), source))
      return true;
   ppveMax = Methods::applyAssign(ppveMax, Methods::queryMinUnsignedAssign(source), source, env);

   QueryOperation::SimplificationEnvironment simplification;
   query(QueryOperation().setSimplification(), simplification);
   if (simplification.hasResult())
      env.absorb(simplification.presult().extractElement(), arg);
   return true;
}

bool
IntegerInterval::constraintCompareEqualConstant(const VirtualOperation&, VirtualElement& source,
      Argument arg, ConstraintEnvironment& env) { // [a,b] == x forced -> x if a <= x <= b
   AssumeCondition(source.getApproxKind().isConstant())
   env.mergeVerdictExact();
   IntersectEnvironment intersectionEnv(env, arg);
   intersectWithConstant(source, intersectionEnv);
   return true;
}

bool
IntegerInterval::constraintCompareDifferentConstant(const VirtualOperation&, VirtualElement& source,
      Argument arg, ConstraintEnvironment& env) { // [a,b] != x forced -> [a,min(b,x-1)] U [max(a,x+1), b]
   AssumeCondition(source.getApproxKind().isConstant())
   env.mergeVerdictExact();

   if (Methods::applyBoolean(*ppveMin, querySCompareGreaterOperation(), source)
      || Methods::applyBoolean(*ppveMax, querySCompareLessOperation(), source))
      return true;
   EvaluationParameters propagateError = EvaluationApplyParameters(env).propagateErrorStates();
   PPVirtualElement sourceMinusOne = Methods::applyAssign(PPVirtualElement(source),
         querySPrevAssignOperation(), propagateError);
   PPVirtualElement sourcePlusOne = Methods::applyAssign(PPVirtualElement(source),
         querySNextAssignOperation(), propagateError);
   PPVirtualElement upperInf, lowerSup;
   if (sourceMinusOne.isValid())
      upperInf = Methods::apply(*ppveMax, querySMinOperation(), *sourceMinusOne, propagateError);
   if (sourcePlusOne.isValid())
      lowerSup = Methods::apply(*ppveMin, querySMaxOperation(), *sourcePlusOne, propagateError);
   PPVirtualElement minInterval, maxInterval;
   if (upperInf.isValid()) { 
      minInterval = Methods::newInterval(source, Init().setInterval(ppveMin, upperInf).setUnsigned(queryUnsignedField()));
      minInterval = Methods::simplify(minInterval);
   };
   if (lowerSup.isValid()) { 
      maxInterval = Methods::newInterval(source, Init().setInterval(lowerSup, ppveMax).setUnsigned(queryUnsignedField()));
      maxInterval = Methods::simplify(maxInterval);
   };
   if (minInterval.isValid() && maxInterval.isValid()) {
      PNT::TPassPointer<VirtualDisjunction, PPAbstractElement> result = Methods::newDisjunction(source);
      result->add(minInterval, env.getInformationKind());
      result->add(maxInterval, env.getInformationKind());
      env.absorb(result.extractElement(), arg);
   }
   else if (minInterval.isValid())
      env.absorb(minInterval.extractElement(), arg);
   else if (maxInterval.isValid())
      env.absorb(maxInterval.extractElement(), arg);
   else
      env.setEmpty();
   return true;
}

bool
IntegerInterval::constraintCompareGreaterOrEqualConstant(const VirtualOperation&, VirtualElement& source,
      Argument arg, ConstraintEnvironment& env) { // [a,b] >= x forced -> [max(a,x), b]
   AssumeCondition(source.getApproxKind().isConstant())
   env.mergeVerdictExact();
   if (Methods::applyBoolean(*ppveMin, querySCompareGreaterOrEqualOperation(), source))
      return true;
   auto maxAssignOperation = querySMaxAssignOperation();
   ppveMin = Methods::applyAssign(ppveMin, maxAssignOperation, source, env);

   QueryOperation::SimplificationEnvironment simplification;
   query(QueryOperation().setSimplification(), simplification);
   if (simplification.hasResult())
      env.absorb(simplification.presult().extractElement(), arg);
   return true;
}

bool
IntegerInterval::constraintCompareGreaterOrEqualSignedConstant(const VirtualOperation&, VirtualElement& source,
      Argument arg, ConstraintEnvironment& env) { // [a,b] >= x forced -> [max(a,x), b]
   AssumeCondition(source.getApproxKind().isConstant())
   if (hasUnsignedField())
      {  AssumeUnimplemented }
   env.mergeVerdictExact();
   if (Methods::applyBoolean(*ppveMin, Methods::queryCompareGreaterOrEqualSigned(source), source))
      return true;
   auto maxSignedAssignOperation = Methods::queryMaxSignedAssign(source);
   ppveMin = Methods::applyAssign(ppveMin, maxSignedAssignOperation, source, env);

   QueryOperation::SimplificationEnvironment simplification;
   query(QueryOperation().setSimplification(), simplification);
   if (simplification.hasResult())
      env.absorb(simplification.presult().extractElement(), arg);
   return true;
}

bool
IntegerInterval::constraintCompareGreaterOrEqualUnsignedConstant(const VirtualOperation&, VirtualElement& source,
      Argument arg, ConstraintEnvironment& env) { // [a,b] >= x forced -> [max(a,x), b]
   AssumeCondition(source.getApproxKind().isConstant())
   if (!hasUnsignedField())
      {  AssumeUnimplemented }
   env.mergeVerdictExact();
   if (Methods::applyBoolean(*ppveMin, Methods::queryCompareGreaterOrEqualUnsigned(source), source))
      return true;
   auto maxUnsignedAssignOperation = Methods::queryMaxUnsignedAssign(source);
   ppveMin = Methods::applyAssign(ppveMin, maxUnsignedAssignOperation, source, env);

   QueryOperation::SimplificationEnvironment simplification;
   query(QueryOperation().setSimplification(), simplification);
   if (simplification.hasResult())
      env.absorb(simplification.presult().extractElement(), arg);
   return true;
}

bool
IntegerInterval::constraintCompareGreaterConstant(const VirtualOperation&, VirtualElement& source,
      Argument arg, ConstraintEnvironment& env) { // [a,b] > x forced -> [max(a,x+1), b]
   AssumeCondition(source.getApproxKind().isConstant())
   if (hasUnsignedField())
      {  AssumeUnimplemented }
   env.mergeVerdictExact();
   if (Methods::applyBoolean(*ppveMin, querySCompareGreaterOrEqualOperation(), source))
      return true;
   EvaluationParameters natural = EvaluationApplyParameters(env).stopErrorStates();
   auto maxAssignOperation = querySMaxAssignOperation();
   auto nextAssignOperation = querySNextAssignOperation();
   ppveMin = Methods::applyAssign(ppveMin, maxAssignOperation,
      *Methods::applyAssign(PPVirtualElement(source), nextAssignOperation,
         natural), natural);

   QueryOperation::SimplificationEnvironment simplification;
   query(QueryOperation().setSimplification(), simplification);
   if (simplification.hasResult())
      env.absorb(simplification.presult().extractElement(), arg);
   return true;
}

bool
IntegerInterval::constraintCompareGreaterSignedConstant(const VirtualOperation&, VirtualElement& source,
      Argument arg, ConstraintEnvironment& env) { // [a,b] > x forced -> [max(a,x+1), b]
   AssumeCondition(source.getApproxKind().isConstant())
   if (hasUnsignedField())
      {  AssumeUnimplemented }
   env.mergeVerdictExact();
   if (Methods::applyBoolean(*ppveMin, Methods::queryCompareGreaterOrEqualSigned(source), source))
      return true;
   EvaluationParameters natural = EvaluationApplyParameters(env).stopErrorStates();
   auto maxSignedAssignOperation =  Methods::queryMaxSignedAssign(source);
   auto nextSignedAssignOperation = Methods::queryNextSignedAssign(source);
   ppveMin = Methods::applyAssign(ppveMin, maxSignedAssignOperation,
      *Methods::applyAssign(PPVirtualElement(source), nextSignedAssignOperation,
         natural), natural);

   QueryOperation::SimplificationEnvironment simplification;
   query(QueryOperation().setSimplification(), simplification);
   if (simplification.hasResult())
      env.absorb(simplification.presult().extractElement(), arg);
   return true;
}

bool
IntegerInterval::constraintCompareGreaterUnsignedConstant(const VirtualOperation&, VirtualElement& source,
      Argument arg, ConstraintEnvironment& env) { // [a,b] > x forced -> [max(a,x+1), b]
   AssumeCondition(source.getApproxKind().isConstant())
   if (!hasUnsignedField())
      {  AssumeUnimplemented }
   env.mergeVerdictExact();
   if (Methods::applyBoolean(*ppveMin, Methods::queryCompareGreaterOrEqualUnsigned(source), source))
      return true;
   EvaluationParameters natural = EvaluationApplyParameters(env).stopErrorStates();
   auto maxUnsignedAssignOperation = Methods::queryMaxUnsignedAssign(source);
   auto nextUnsignedAssignOperation = Methods::queryNextUnsignedAssign(source);
   ppveMin = Methods::applyAssign(ppveMin, maxUnsignedAssignOperation,
      *Methods::applyAssign(PPVirtualElement(source), nextUnsignedAssignOperation,
         natural), natural);

   QueryOperation::SimplificationEnvironment simplification;
   query(QueryOperation().setSimplification(), simplification);
   if (simplification.hasResult())
      env.absorb(simplification.presult().extractElement(), arg);
   return true;
}

#if defined(__GNUC__) && GCC_VERSION >= 40600
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-but-set-variable"
#endif

#define DefineIntroConstraintInterval                                                             \
      PPVirtualElement ssureMin, ssureMax, ssureSourceMin, ssureSourceMax;                        \
      PPVirtualElement *sureMin = &ppveMin, *sureMax = &ppveMax,                                  \
         *sureSourceMin = &source.ppveMin, *sureSourceMax = &source.ppveMax;                      \
      if (env.getInformationKind().isExact()) {                                                   \
         env.setUnstable();                                                                       \
         ssureMin.fullAssign(ppveMin);                                                            \
         ssureMax.fullAssign(ppveMax);                                                            \
         ssureSourceMin.fullAssign(source.ppveMin);                                               \
         ssureSourceMax.fullAssign(source.ppveMax);                                               \
         sureMin = &ssureMin;                                                                     \
         sureMax = &ssureMax;                                                                     \
         sureSourceMin = &ssureSourceMin;                                                         \
         sureSourceMax = &ssureSourceMax;                                                         \
      };

#define DefineConclusionConstraintInterval                                                        \
      if (env.getInformationKind().isExact()) {                                                   \
         PNT::TPassPointer<VirtualDisjunction, PPAbstractElement> result = Methods::newDisjunction(*ppveMin);\
         const VirtualElement& min = *ppveMin;                                                    \
         result->addMay(Methods::newInterval(min, Init().setInterval(ppveMin, ppveMax).setUnsigned(queryUnsignedField()))); \
         if (ssureMin.isValid() && ssureMax.isValid())                                              \
            result->addSure(Methods::newInterval(min, Init().setInterval(ssureMin, ssureMax).setUnsigned(queryUnsignedField()))); \
         env.absorb(result.extractElement(), arg);                                                \
         result = Methods::newDisjunction(*source.ppveMin);                                       \
         result->addMay(Methods::newInterval(min, Init().setInterval(source.ppveMin, source.ppveMax).setUnsigned(queryUnsignedField())));\
         if (ssureSourceMin.isValid() && ssureSourceMax.isValid())                                  \
            result->addSure(Methods::newInterval(min, Init().setInterval(ssureSourceMin, ssureSourceMax).setUnsigned(queryUnsignedField()))); \
         env.absorb(result.extractElement(), arg.queryInverse());                                 \
      }                                                                                           \
      else {                                                                                      \
         QueryOperation::SimplificationEnvironment simplification;                                \
         query(QueryOperation().setSimplification(), simplification);                             \
         if (simplification.hasResult())                                                          \
            env.absorb(simplification.presult().extractElement(), arg);                           \
         source.query(QueryOperation().setSimplification(), simplification);                      \
         if (simplification.hasResult())                                                          \
            env.absorb(simplification.presult().extractElement(), arg.queryInverse());            \
      };

bool
IntegerInterval::constraintCompareLessInterval(const VirtualOperation& operation, IntegerInterval& source,
      Argument arg, ConstraintEnvironment& env) { // [a,b] < [c,d] forced
   if (queryUnsignedField() != source.queryUnsignedField()) {
      PNT::TPassPointer<VirtualDisjunction, PPAbstractElement> copy = changeSign(env);
      if (copy.isValid()) {
         PPVirtualElement trueElement = Methods::newTrue(*copy);
         ConstraintTransmitEnvironment transmitEnv(env, source, arg);
         assume(copy->constraint(operation, *trueElement, transmitEnv));
         return true;
      };
   };
   env.mergeVerdictExact();
   if (Methods::applyBoolean(*ppveMax, querySCompareLessOperation(), *source.ppveMin))
      return true;
   if (Methods::applyBoolean(*ppveMin, querySCompareGreaterOrEqualOperation(), *source.ppveMax)) {
      env.setEmpty();
      return true;
   };

   if (!env.getInformationKind().isSure()) {
      // as may [a,b] -> [a, min(b,d-1)] and [c,d] -> [max(c,a+1), d]
      // all the operations are in the mode "stop overflow"
      EvaluationParameters mayStopErrorStates(EPMayStopErrorStates);
      mayStopErrorStates.setApproximatePartFrom(env);
      auto minAssignOperation = querySMinAssignOperation();
      auto maxAssignOperation = querySMaxAssignOperation();
      auto prevAssignOperation = querySPrevAssignOperation();
      auto nextAssignOperation = querySNextAssignOperation();
      ppveMax = Methods::applyAssign(ppveMax, minAssignOperation,
         *Methods::applyAssign(PPVirtualElement(*source.ppveMax), prevAssignOperation, mayStopErrorStates), mayStopErrorStates);
      source.ppveMin = Methods::applyAssign(source.ppveMin, maxAssignOperation,
         *Methods::applyAssign(PPVirtualElement(*ppveMin), nextAssignOperation, mayStopErrorStates), mayStopErrorStates);
   };

   DefineIntroConstraintInterval
   if (!env.getInformationKind().isMay()) {
      // as sure [a,b] -> [a, min(b,c-1)] and [c,d] -> [max(c,b+1), d]
      // all the operations are in the mode "stop overflow"
      env.mergeVerdictDegradate(); // for the repartition of the central values
      EvaluationParameters surePropagateErrorStates(EPSurePropagateErrorStates);
      surePropagateErrorStates.setApproximatePartFrom(env);
      auto minAssignOperation = querySMinAssignOperation();
      auto maxAssignOperation = querySMaxAssignOperation();
      auto prevAssignOperation = querySPrevAssignOperation();
      auto nextAssignOperation = querySNextAssignOperation();
      *sureMax = Methods::applyAssign(*sureMax, minAssignOperation,
         *Methods::applyAssign(PPVirtualElement(**sureSourceMin), prevAssignOperation,
            surePropagateErrorStates), surePropagateErrorStates);
      *sureSourceMin = Methods::applyAssign(*sureSourceMin, maxAssignOperation,
         *Methods::applyAssign(PPVirtualElement(**sureMax), nextAssignOperation,
            surePropagateErrorStates), surePropagateErrorStates);
   };
   DefineConclusionConstraintInterval

   return true;
}

bool
IntegerInterval::constraintCompareLessSignedInterval(const VirtualOperation& operation, IntegerInterval& source,
      Argument arg, ConstraintEnvironment& env) { // [a,b] < [c,d] forced
   if (hasUnsignedField()) {
      PNT::TPassPointer<VirtualDisjunction, PPAbstractElement> copy = changeSign(env);
      if (copy.isValid()) {
         PPVirtualElement trueElement = Methods::newTrue(*copy);
         ConstraintTransmitEnvironment transmitEnv(env, source, arg);
         assume(copy->constraint(operation, *trueElement, transmitEnv));
         return true;
      };
   };
   if (source.hasUnsignedField()) {
      PNT::TPassPointer<VirtualDisjunction, PPAbstractElement> copy = source.changeSign(env);
      if (copy.isValid()) {
         PPVirtualElement trueElement = Methods::newTrue(*copy);
         ConstraintTransmitEnvironment transmitEnv(env, copy, arg);
         assume(constraint(operation, *trueElement, transmitEnv));
         return true;
      };
   };

   env.mergeVerdictExact();
   if (Methods::applyBoolean(*ppveMax, Methods::queryCompareLessSigned(*ppveMin), *source.ppveMin))
      return true;
   if (Methods::applyBoolean(*ppveMin, Methods::queryCompareGreaterOrEqualSigned(*ppveMin), *source.ppveMax)) {
      env.setEmpty();
      return true;
   };

   if (!env.getInformationKind().isSure()) {
      // as may [a,b] -> [a, min(b,d-1)] and [c,d] -> [max(c,a+1), d]
      // all the operations are in the mode "stop overflow"
      EvaluationParameters mayStopErrorStates(EPMayStopErrorStates);
      mayStopErrorStates.setApproximatePartFrom(env);
      auto minSignedAssignOperation = Methods::queryMinSignedAssign(*ppveMax);
      auto maxSignedAssignOperation = Methods::queryMaxSignedAssign(*ppveMin);
      auto prevSignedAssignOperation = Methods::queryPrevSignedAssign(*ppveMin);
      auto nextSignedAssignOperation = Methods::queryNextSignedAssign(*ppveMin);
      ppveMax = Methods::applyAssign(ppveMax, minSignedAssignOperation,
         *Methods::applyAssign(PPVirtualElement(*source.ppveMax), prevSignedAssignOperation, mayStopErrorStates), mayStopErrorStates);
      source.ppveMin = Methods::applyAssign(source.ppveMin, maxSignedAssignOperation,
         *Methods::applyAssign(PPVirtualElement(*ppveMin), nextSignedAssignOperation, mayStopErrorStates), mayStopErrorStates);
   };

   DefineIntroConstraintInterval
   if (!env.getInformationKind().isMay()) {
      // as sure [a,b] -> [a, min(b,c-1)] and [c,d] -> [max(c,b+1), d]
      // all the operations are in the mode "stop overflow"
      env.mergeVerdictDegradate(); // for the repartition of the central values
      EvaluationParameters surePropagateErrorStates(EPSurePropagateErrorStates);
      surePropagateErrorStates.setApproximatePartFrom(env);
      auto minSignedAssignOperation = Methods::queryMinSignedAssign(*ppveMax);
      auto maxSignedAssignOperation = Methods::queryMaxSignedAssign(*ppveMin);
      auto prevSignedAssignOperation = Methods::queryPrevSignedAssign(*ppveMin);
      auto nextSignedAssignOperation = Methods::queryNextSignedAssign(*ppveMin);
      *sureMax = Methods::applyAssign(*sureMax, minSignedAssignOperation,
         *Methods::applyAssign(PPVirtualElement(**sureSourceMin), prevSignedAssignOperation,
            surePropagateErrorStates), surePropagateErrorStates);
      *sureSourceMin = Methods::applyAssign(*sureSourceMin, maxSignedAssignOperation,
         *Methods::applyAssign(PPVirtualElement(**sureMax), nextSignedAssignOperation,
            surePropagateErrorStates), surePropagateErrorStates);
   };
   DefineConclusionConstraintInterval

   return true;
}

bool
IntegerInterval::constraintCompareLessUnsignedInterval(const VirtualOperation& operation, IntegerInterval& source,
      Argument arg, ConstraintEnvironment& env) { // [a,b] < [c,d] forced
   if (!hasUnsignedField()) {
      PNT::TPassPointer<VirtualDisjunction, PPAbstractElement> copy = changeSign(env);
      if (copy.isValid()) {
         PPVirtualElement trueElement = Methods::newTrue(*copy);
         ConstraintTransmitEnvironment transmitEnv(env, source, arg);
         assume(copy->constraint(operation, *trueElement, transmitEnv));
         return true;
      };
   };
   if (!source.hasUnsignedField()) {
      PNT::TPassPointer<VirtualDisjunction, PPAbstractElement> copy = source.changeSign(env);
      if (copy.isValid()) {
         PPVirtualElement trueElement = Methods::newTrue(*copy);
         ConstraintTransmitEnvironment transmitEnv(env, copy, arg);
         assume(constraint(operation, *trueElement, transmitEnv));
         return true;
      };
   };

   env.mergeVerdictExact();
   if (Methods::applyBoolean(*ppveMax, Methods::queryCompareLessUnsigned(*ppveMin), *source.ppveMin))
      return true;
   if (Methods::applyBoolean(*ppveMin, Methods::queryCompareGreaterOrEqualUnsigned(*ppveMin), *source.ppveMax)) {
      env.setEmpty();
      return true;
   };
   if (!env.getInformationKind().isSure()) {
      // as may [a,b] -> [a, min(b,d-1)] and [c,d] -> [max(c,a+1), d]
      // all the operations are in the mode "stop overflow"
      EvaluationParameters mayStopErrorStates(EPMayStopErrorStates);
      mayStopErrorStates.setApproximatePartFrom(env);
      VirtualElement& max = *ppveMax;
      auto minUnsignedAssignOperation = Methods::queryMinUnsignedAssign(max);
      auto maxUnsignedAssignOperation = Methods::queryMaxUnsignedAssign(max);
      auto prevUnsignedAssignOperation = Methods::queryPrevUnsignedAssign(max);
      auto nextUnsignedAssignOperation = Methods::queryNextUnsignedAssign(max);
      ppveMax = Methods::applyAssign(ppveMax, minUnsignedAssignOperation,
         *Methods::applyAssign(PPVirtualElement(*source.ppveMax),
         prevUnsignedAssignOperation, mayStopErrorStates), mayStopErrorStates);
      source.ppveMin = Methods::applyAssign(source.ppveMin, maxUnsignedAssignOperation,
         *Methods::applyAssign(PPVirtualElement(*ppveMin), nextUnsignedAssignOperation, mayStopErrorStates), mayStopErrorStates);
   };

   DefineIntroConstraintInterval
   if (!env.getInformationKind().isMay()) {
      // as sure [a,b] -> [a, min(b,c-1)] and [c,d] -> [max(c,b+1), d]
      // all the operations are in the mode "stop overflow"
      env.mergeVerdictDegradate(); // for the repartition of the central values
      EvaluationParameters surePropagateErrorStates(EPSurePropagateErrorStates);
      surePropagateErrorStates.setApproximatePartFrom(env);
      VirtualElement& max = *ppveMax;
      auto minUnsignedAssignOperation = Methods::queryMinUnsignedAssign(max);
      auto maxUnsignedAssignOperation = Methods::queryMaxUnsignedAssign(max);
      auto prevUnsignedAssignOperation = Methods::queryPrevUnsignedAssign(max);
      auto nextUnsignedAssignOperation = Methods::queryNextUnsignedAssign(max);
      *sureMax = Methods::applyAssign(*sureMax, minUnsignedAssignOperation,
         *Methods::applyAssign(PPVirtualElement(**sureSourceMin), prevUnsignedAssignOperation,
            surePropagateErrorStates), surePropagateErrorStates);
      *sureSourceMin = Methods::applyAssign(*sureSourceMin, maxUnsignedAssignOperation,
         *Methods::applyAssign(PPVirtualElement(**sureMax), nextUnsignedAssignOperation,
            surePropagateErrorStates), surePropagateErrorStates);
   };
   DefineConclusionConstraintInterval

   return true;
}

bool
IntegerInterval::constraintCompareLessOrEqualInterval(const VirtualOperation& operation, IntegerInterval& source,
      Argument arg, ConstraintEnvironment& env) { // [a,b] <= [c,d] forced
   if (queryUnsignedField() != source.queryUnsignedField()) {
      PNT::TPassPointer<VirtualDisjunction, PPAbstractElement> copy = changeSign(env);
      if (copy.isValid()) {
         PPVirtualElement trueElement = Methods::newTrue(*copy);
         ConstraintTransmitEnvironment transmitEnv(env, source, arg);
         assume(copy->constraint(operation, *trueElement, transmitEnv));
         return true;
      };
   };
   env.mergeVerdictExact();

   if (Methods::applyBoolean(*ppveMax, querySCompareLessOrEqualOperation(), *source.ppveMin))
      return true;
   if (Methods::applyBoolean(*ppveMin, querySCompareGreaterOperation(), *source.ppveMax)) {
      env.setEmpty();
      return true;
   };

   if (!env.getInformationKind().isSure()) {
      // as may [a,b] -> [a, min(b,d)] and [c,d] -> [max(c,a), d]
      auto minAssignOperation = querySMinAssignOperation();
      auto maxAssignOperation = querySMaxAssignOperation();
      ppveMax = Methods::applyAssign(ppveMax, minAssignOperation, *source.ppveMax, env);
      source.ppveMin = Methods::applyAssign(source.ppveMin, maxAssignOperation, *ppveMin, env);
   };

   DefineIntroConstraintInterval
   if (!env.getInformationKind().isMay()) {
      // as sure [a,b] -> [a, min(b,c)] and [c,d] -> [max(c,b), d]
      env.mergeVerdictDegradate(); // for the repartition of the central values
      EvaluationParameters surePropagateErrorStates(EPSurePropagateErrorStates);
      surePropagateErrorStates.setApproximatePartFrom(env);
      *sureMax = Methods::applyAssign(*sureMax, querySMinAssignOperation(),
         **sureSourceMin, surePropagateErrorStates);
      *sureSourceMin = Methods::applyAssign(*sureSourceMin, querySMaxAssignOperation(),
         **sureMax, surePropagateErrorStates);
   };
   DefineConclusionConstraintInterval

   return true;
}

bool
IntegerInterval::constraintCompareLessOrEqualSignedInterval(const VirtualOperation& operation, IntegerInterval& source,
      Argument arg, ConstraintEnvironment& env) { // [a,b] <= [c,d] forced
   if (hasUnsignedField()) {
      PNT::TPassPointer<VirtualDisjunction, PPAbstractElement> copy = changeSign(env);
      if (copy.isValid()) {
         PPVirtualElement trueElement = Methods::newTrue(*copy);
         ConstraintTransmitEnvironment transmitEnv(env, source, arg);
         assume(copy->constraint(operation, *trueElement, transmitEnv));
         return true;
      };
   };
   if (source.hasUnsignedField()) {
      PNT::TPassPointer<VirtualDisjunction, PPAbstractElement> copy = source.changeSign(env);
      if (copy.isValid()) {
         PPVirtualElement trueElement = Methods::newTrue(*copy);
         ConstraintTransmitEnvironment transmitEnv(env, copy, arg);
         assume(constraint(operation, *trueElement, transmitEnv));
         return true;
      };
   };
   env.mergeVerdictExact();

   if (Methods::applyBoolean(*ppveMax, Methods::queryCompareLessOrEqualSigned(*ppveMin), *source.ppveMin))
      return true;
   if (Methods::applyBoolean(*ppveMin, Methods::queryCompareGreaterSigned(*ppveMin), *source.ppveMax)) {
      env.setEmpty();
      return true;
   };
      
   if (!env.getInformationKind().isSure()) {
      // as may [a,b] -> [a, min(b,d)] and [c,d] -> [max(c,a), d]
      auto minSignedAssignOperation = Methods::queryMinSignedAssign(*ppveMin);
      auto maxSignedAssignOperation = Methods::queryMaxSignedAssign(*ppveMin);
      ppveMax = Methods::applyAssign(ppveMax, minSignedAssignOperation, *source.ppveMax, env);
      source.ppveMin = Methods::applyAssign(source.ppveMin, maxSignedAssignOperation, *ppveMin, env);
   };

   DefineIntroConstraintInterval
   if (!env.getInformationKind().isMay()) {
      // as sure [a,b] -> [a, min(b,c)] and [c,d] -> [max(c,b), d]
      env.mergeVerdictDegradate(); // for the repartition of the central values
      EvaluationParameters surePropagateErrorStates(EPSurePropagateErrorStates);
      surePropagateErrorStates.setApproximatePartFrom(env);
      auto minSignedAssignOperation = Methods::queryMinSignedAssign(*ppveMin);
      auto maxSignedAssignOperation = Methods::queryMaxSignedAssign(*ppveMin);
      *sureMax = Methods::applyAssign(*sureMax, minSignedAssignOperation,
         **sureSourceMin, surePropagateErrorStates);
      *sureSourceMin = Methods::applyAssign(*sureSourceMin, maxSignedAssignOperation,
         **sureMax, surePropagateErrorStates);
   };
   DefineConclusionConstraintInterval

   return true;
}

bool
IntegerInterval::constraintCompareLessOrEqualUnsignedInterval(const VirtualOperation& operation,
      IntegerInterval& source, Argument arg, ConstraintEnvironment& env) {
   // [a,b] <= [c,d] forced
   if (!hasUnsignedField()) {
      PNT::TPassPointer<VirtualDisjunction, PPAbstractElement> copy = changeSign(env);
      if (copy.isValid()) {
         PPVirtualElement trueElement = Methods::newTrue(*copy);
         ConstraintTransmitEnvironment transmitEnv(env, source, arg);
         assume(copy->constraint(operation, *trueElement, transmitEnv));
         return true;
      };
   };
   if (!source.hasUnsignedField()) {
      PNT::TPassPointer<VirtualDisjunction, PPAbstractElement> copy = source.changeSign(env);
      if (copy.isValid()) {
         PPVirtualElement trueElement = Methods::newTrue(*copy);
         ConstraintTransmitEnvironment transmitEnv(env, copy, arg);
         assume(constraint(operation, *trueElement, transmitEnv));
         return true;
      };
   };
   env.mergeVerdictExact();

   if (Methods::applyBoolean(*ppveMax, Methods::queryCompareLessOrEqualUnsigned(*ppveMin), *source.ppveMin))
      return true;
   if (Methods::applyBoolean(*ppveMin, Methods::queryCompareGreaterUnsigned(*ppveMin), *source.ppveMax)) {
      env.setEmpty();
      return true;
   };
      
   if (!env.getInformationKind().isSure()) {
      // as may [a,b] -> [a, min(b,d)] and [c,d] -> [max(c,a), d]
      auto minUnsignedAssignOperation = Methods::queryMinUnsignedAssign(*ppveMin);
      auto maxUnsignedAssignOperation = Methods::queryMaxUnsignedAssign(*ppveMin);
      ppveMax = Methods::applyAssign(ppveMax, minUnsignedAssignOperation, *source.ppveMax, env);
      source.ppveMin = Methods::applyAssign(source.ppveMin, maxUnsignedAssignOperation, *ppveMin, env);
   };

   DefineIntroConstraintInterval
   if (!env.getInformationKind().isMay()) {
      // as sure [a,b] -> [a, min(b,c)] and [c,d] -> [max(c,b), d]
      env.mergeVerdictDegradate(); // for the repartition of the central values
      EvaluationParameters surePropagateErrorStates(EPSurePropagateErrorStates);
      surePropagateErrorStates.setApproximatePartFrom(env);
      auto minUnsignedAssignOperation = Methods::queryMinUnsignedAssign(*ppveMin);
      auto maxUnsignedAssignOperation = Methods::queryMaxUnsignedAssign(*ppveMin);
      *sureMax = Methods::applyAssign(*sureMax, minUnsignedAssignOperation,
         **sureSourceMin, surePropagateErrorStates);
      *sureSourceMin = Methods::applyAssign(*sureSourceMin, maxUnsignedAssignOperation,
         **sureMax, surePropagateErrorStates);
   };
   DefineConclusionConstraintInterval

   return true;
}

bool
IntegerInterval::constraintCompareEqualInterval(const VirtualOperation& operation, IntegerInterval& source,
      Argument arg, ConstraintEnvironment& env) { // [a,b] == [c,d] forced
   if (queryUnsignedField() != source.queryUnsignedField()) {
      PNT::TPassPointer<VirtualDisjunction, PPAbstractElement> copy = changeSign(env);
      if (copy.isValid()) {
         PPVirtualElement trueElement = Methods::newTrue(*copy);
         ConstraintTransmitEnvironment transmitEnv(env, source, arg);
         assume(copy->constraint(operation, *trueElement, transmitEnv));
         return true;
      };
   };
   env.mergeVerdictExact();

   if (Methods::applyBoolean(*ppveMax, querySCompareLessOperation(), *source.ppveMin)
         || Methods::applyBoolean(*ppveMin, querySCompareGreaterOperation(), *source.ppveMax)) {
      env.setEmpty();
      return true;
   };

   PNT::TPassPointer<VirtualDisjunction, PPAbstractElement> result;
   if (!env.getInformationKind().isMay()) {
      PPVirtualElement sureResult;
      if (Methods::applyBoolean(*ppveMin, querySCompareLessOrEqualOperation(), *ppveMax)
            && Methods::applyBoolean(*source.ppveMin, querySCompareLessOrEqualOperation(), *source.ppveMax)) {
         if (Methods::applyBoolean(*ppveMin, querySCompareGreaterOrEqualOperation(), *source.ppveMin)
            && Methods::applyBoolean(*ppveMin, querySCompareLessOrEqualOperation(), *source.ppveMax))
            sureResult.fullAssign(ppveMin);
         else if (Methods::applyBoolean(*ppveMax, querySCompareGreaterOrEqualOperation(), *source.ppveMin)
            && Methods::applyBoolean(*ppveMax, querySCompareLessOrEqualOperation(), *source.ppveMax))
            sureResult.fullAssign(ppveMax);
         else if (Methods::applyBoolean(*source.ppveMin, querySCompareGreaterOrEqualOperation(), *ppveMin)
            && Methods::applyBoolean(*source.ppveMin, querySCompareLessOrEqualOperation(), *ppveMax))
            sureResult.fullAssign(source.ppveMin);
         else if (Methods::applyBoolean(*source.ppveMax, querySCompareGreaterOrEqualOperation(), *ppveMin)
            && Methods::applyBoolean(*source.ppveMax, querySCompareLessOrEqualOperation(), *ppveMax))
            sureResult.fullAssign(source.ppveMax);
      };
      if (env.getInformationKind().isSure()) {
         if (sureResult.isValid()) {
            env.absorb(sureResult->createSCopy(), Argument().setResult());
            env.absorb(sureResult.extractElement(), Argument().setFst());
         }
         else {
            env.setEmpty();
            return true;
         };
         return true;
      };
      result = Methods::newDisjunction(*ppveMin);
      result->addSure(sureResult);
      env.absorb(result->createSCopy(), Argument().setResult());
   };

   EvaluationParameters mayStopErrorStates(EPMayStopErrorStates);
   mayStopErrorStates.setApproximatePartFrom(env);
   auto maxAssignOperation = querySMaxAssignOperation();
   auto minAssignOperation = querySMinAssignOperation();
   ppveMin = Methods::applyAssign(ppveMin, maxAssignOperation, *source.ppveMin, mayStopErrorStates);
   ppveMax = Methods::applyAssign(ppveMax, minAssignOperation, *source.ppveMax, mayStopErrorStates);
   source.ppveMin = PPVirtualElement(*ppveMin);
   source.ppveMax = PPVirtualElement(*ppveMax);

   if (result.isValid()) {
      const VirtualElement& min = *ppveMin;
      result->addMay(Methods::newInterval(min, Init().setInterval(ppveMin, ppveMax).setUnsigned(queryUnsignedField())));
      env.absorb(result.extractElement(), Argument().setFst());
   };
   return true;
}

bool
IntegerInterval::constraintCompareDifferentInterval(const VirtualOperation& operation,
      IntegerInterval& source, Argument arg, ConstraintEnvironment& env) { // [a,b] != [c,d] forced
   if (queryUnsignedField() != source.queryUnsignedField()) {
      PNT::TPassPointer<VirtualDisjunction, PPAbstractElement> copy = changeSign(env);
      if (copy.isValid()) {
         PPVirtualElement trueElement = Methods::newTrue(*copy);
         ConstraintTransmitEnvironment transmitEnv(env, source, arg);
         assume(copy->constraint(operation, *trueElement, transmitEnv));
         return true;
      };
   };
   env.mergeVerdictExact();

   if (Methods::applyBoolean(*ppveMax, querySCompareLessOperation(), *source.ppveMin)
         || Methods::applyBoolean(*ppveMin, querySCompareGreaterOperation(), *source.ppveMax))
      return true;

   env.mergeVerdictDegradate();
   if (env.getInformationKind().isMay())
      return true;

   // [a,min(b,c-1)] U [max(a,d+1), b]
   PPVirtualElement sureLowerResult, sureUpperResult;
   EvaluationParameters sureStopErrorStates(EPSureStopErrorStates);
   sureStopErrorStates.setApproximatePartFrom(env);
   if (!Methods::applyBoolean(*ppveMin, querySCompareGreaterOrEqualOperation(), *source.ppveMin))
      sureLowerResult = Methods::newInterval(*ppveMin, Init().setInterval(PPVirtualElement(*ppveMin),
         Methods::apply(*ppveMax, querySMinOperation(), *Methods::applyAssign(
            PPVirtualElement(*source.ppveMin), querySPrevAssignOperation(), sureStopErrorStates), sureStopErrorStates)).setUnsigned(queryUnsignedField()));
   if (!Methods::applyBoolean(*ppveMax, querySCompareLessOrEqualOperation(), *source.ppveMax))
      sureUpperResult = Methods::newInterval(*ppveMin, 
         Init().setInterval(Methods::apply(*ppveMin, querySMaxOperation(),
            *Methods::applyAssign(PPVirtualElement(*source.ppveMax), querySNextAssignOperation(),
               sureStopErrorStates), sureStopErrorStates),
         PPVirtualElement(*ppveMax)).setUnsigned(queryUnsignedField()));

   // [c,min(d,a-1)] U [max(c,b+1), d]
   PPVirtualElement sureLowerArg, sureUpperArg;
   if (!Methods::applyBoolean(*source.ppveMin, querySCompareGreaterOrEqualOperation(), *ppveMin))
      sureLowerArg = Methods::newInterval(*ppveMin, Init().setInterval(PPVirtualElement(*source.ppveMin),
         Methods::apply(*source.ppveMax, querySMinOperation(), *Methods::applyAssign(
            PPVirtualElement(*ppveMin), querySPrevAssignOperation(), sureStopErrorStates), sureStopErrorStates)).setUnsigned(queryUnsignedField()));
   if (!Methods::applyBoolean(*source.ppveMax, querySCompareLessOrEqualOperation(), *ppveMax))
      sureUpperArg = Methods::newInterval(*ppveMin, 
         Init().setInterval(Methods::apply(*source.ppveMin, querySMaxOperation(),
            *Methods::applyAssign(PPVirtualElement(*ppveMax), querySNextAssignOperation(),
               sureStopErrorStates), sureStopErrorStates),
         PPVirtualElement(*source.ppveMax)).setUnsigned(queryUnsignedField()));

   PNT::TPassPointer<VirtualDisjunction, PPAbstractElement> result, argResult;
   if (env.getInformationKind().isExact()) {
      result = Methods::newDisjunction(*ppveMin);
      argResult = Methods::newDisjunction(*ppveMin);
         
      const VirtualElement& min = *ppveMin;
      result->addMay(Methods::newInterval(min, Init().setInterval(ppveMin, ppveMax).setUnsigned(queryUnsignedField())));
      result->addSure(sureLowerResult).addSure(sureUpperResult);
      const VirtualElement& sourceMin = *source.ppveMin;
      argResult->addMay(Methods::newInterval(sourceMin, Init().setInterval(source.ppveMin, source.ppveMax).setUnsigned(queryUnsignedField())));
      argResult->addSure(sureLowerArg).addSure(sureUpperArg);
      env.absorb(result.extractElement(), arg);
      env.absorb(argResult.extractElement(), arg.queryInverse());
      return true;
   };

   env.absorb(Methods::merge(sureLowerResult, sureUpperResult, sureStopErrorStates)
         .extractElement(), arg);
   env.absorb(Methods::merge(sureLowerArg, sureUpperArg, sureStopErrorStates)
         .extractElement(), arg.queryInverse());
   return true;
}

bool
IntegerInterval::constraintCompareGreaterOrEqualInterval(const VirtualOperation& operation,
      IntegerInterval& source, Argument arg, ConstraintEnvironment& env) { // [a,b] >= [c,d] forced
   if (queryUnsignedField() != source.queryUnsignedField()) {
      PNT::TPassPointer<VirtualDisjunction, PPAbstractElement> copy = changeSign(env);
      if (copy.isValid()) {
         PPVirtualElement trueElement = Methods::newTrue(*copy);
         ConstraintTransmitEnvironment transmitEnv(env, source, arg);
         assume(copy->constraint(operation, *trueElement, transmitEnv));
         return true;
      };
   };
   env.mergeVerdictExact();

   if (Methods::applyBoolean(*ppveMin, querySCompareGreaterOrEqualOperation(), *source.ppveMax))
      return true;
   if (Methods::applyBoolean(*ppveMax, querySCompareLessOperation(), *source.ppveMin)) {
      env.setEmpty();
      return true;
   };

   DefineIntroConstraintInterval
   if (!env.getInformationKind().isSure()) {
      // as may [a,b] -> [max(a,c), b] and [c,d] -> [c, min(d,b)]
      EvaluationParameters mayStopErrorStates(EPMayStopErrorStates);
      mayStopErrorStates.setApproximatePartFrom(env);
      auto maxAssignOperation = querySMaxAssignOperation();
      auto minAssignOperation = querySMinAssignOperation();
      ppveMin = Methods::applyAssign(ppveMin, maxAssignOperation, *source.ppveMin, mayStopErrorStates);
      source.ppveMax = Methods::applyAssign(source.ppveMax, minAssignOperation, *ppveMax, mayStopErrorStates);
   };

   if (!env.getInformationKind().isMay()) {
      // as sure [a,b] -> [max(a,d), b] and [c,d] -> [c, min(d,a)]
      env.mergeVerdictDegradate(); // for the repartition of the central values
      EvaluationParameters surePropagateErrorStates(EPSurePropagateErrorStates);
      surePropagateErrorStates.setApproximatePartFrom(env);
      *sureMin = Methods::applyAssign(*sureMin, querySMaxAssignOperation(),
            **sureSourceMax, surePropagateErrorStates);
      *sureSourceMax = Methods::applyAssign(*sureSourceMax, querySMinAssignOperation(),
            **sureMin, surePropagateErrorStates);
   };
   DefineConclusionConstraintInterval

   return true;
}

bool
IntegerInterval::constraintCompareGreaterOrEqualSignedInterval(const VirtualOperation& operation, IntegerInterval& source,
      Argument arg, ConstraintEnvironment& env) { // [a,b] >= [c,d] forced
   if (hasUnsignedField()) {
      PNT::TPassPointer<VirtualDisjunction, PPAbstractElement> copy = changeSign(env);
      if (copy.isValid()) {
         PPVirtualElement trueElement = Methods::newTrue(*copy);
         ConstraintTransmitEnvironment transmitEnv(env, source, arg);
         assume(copy->constraint(operation, *trueElement, transmitEnv));
         return true;
      };
   };
   if (source.hasUnsignedField()) {
      PNT::TPassPointer<VirtualDisjunction, PPAbstractElement> copy = source.changeSign(env);
      if (copy.isValid()) {
         PPVirtualElement trueElement = Methods::newTrue(*copy);
         ConstraintTransmitEnvironment transmitEnv(env, copy, arg);
         assume(constraint(operation, *trueElement, transmitEnv));
         return true;
      };
   };
   env.mergeVerdictExact();

   if (Methods::applyBoolean(*ppveMin, Methods::queryCompareGreaterOrEqualSigned(*ppveMin), *source.ppveMax))
      return true;
   if (Methods::applyBoolean(*ppveMax, Methods::queryCompareLessSigned(*ppveMin), *source.ppveMin)) {
      env.setEmpty();
      return true;
   };

   if (!env.getInformationKind().isSure()) {
      // as may [a,b] -> [max(a,c), b] and [c,d] -> [c, min(d,b)]
      EvaluationParameters mayStopErrorStates(EPMayStopErrorStates);
      mayStopErrorStates.setApproximatePartFrom(env);
      auto minSignedAssignOperation = Methods::queryMinSignedAssign(*ppveMin);
      auto maxSignedAssignOperation = Methods::queryMaxSignedAssign(*ppveMin);
      ppveMin = Methods::applyAssign(ppveMin, maxSignedAssignOperation, *source.ppveMin, mayStopErrorStates);
      source.ppveMax = Methods::applyAssign(source.ppveMax, minSignedAssignOperation, *ppveMax, mayStopErrorStates);
   };

   DefineIntroConstraintInterval
   if (!env.getInformationKind().isMay()) {
      // as sure [a,b] -> [max(a,d), b] and [c,d] -> [c, min(d,a)]
      env.mergeVerdictDegradate(); // for the repartition of the central values
      EvaluationParameters surePropagateErrorStates(EPSurePropagateErrorStates);
      surePropagateErrorStates.setApproximatePartFrom(env);
      auto minSignedAssignOperation = Methods::queryMinSignedAssign(*ppveMin);
      auto maxSignedAssignOperation = Methods::queryMaxSignedAssign(*ppveMin);
      *sureMin = Methods::applyAssign(*sureMin, maxSignedAssignOperation,
            **sureSourceMax, surePropagateErrorStates);
      *sureSourceMax = Methods::applyAssign(*sureSourceMax, minSignedAssignOperation,
            **sureMin, surePropagateErrorStates);
   };
   DefineConclusionConstraintInterval

   return true;
}

bool
IntegerInterval::constraintCompareGreaterOrEqualUnsignedInterval(const VirtualOperation& operation, IntegerInterval& source,
      Argument arg, ConstraintEnvironment& env) { // [a,b] >= [c,d] forced
   if (hasUnsignedField()) {
      PNT::TPassPointer<VirtualDisjunction, PPAbstractElement> copy = changeSign(env);
      if (copy.isValid()) {
         PPVirtualElement trueElement = Methods::newTrue(*copy);
         ConstraintTransmitEnvironment transmitEnv(env, source, arg);
         assume(copy->constraint(operation, *trueElement, transmitEnv));
         return true;
      };
   };
   if (source.hasUnsignedField()) {
      PNT::TPassPointer<VirtualDisjunction, PPAbstractElement> copy = source.changeSign(env);
      if (copy.isValid()) {
         PPVirtualElement trueElement = Methods::newTrue(*copy);
         ConstraintTransmitEnvironment transmitEnv(env, copy, arg);
         assume(constraint(operation, *trueElement, transmitEnv));
         return true;
      };
   };
   env.mergeVerdictExact();

   if (Methods::applyBoolean(*ppveMin, Methods::queryCompareGreaterOrEqualUnsigned(*ppveMin), *source.ppveMax))
      return true;
   if (Methods::applyBoolean(*ppveMax, Methods::queryCompareLessUnsigned(*ppveMin), *source.ppveMin)) {
      env.setEmpty();
      return true;
   };

   if (!env.getInformationKind().isSure()) {
      // as may [a,b] -> [max(a,c), b] and [c,d] -> [c, min(d,b)]
      EvaluationParameters mayStopErrorStates(EPMayStopErrorStates);
      mayStopErrorStates.setApproximatePartFrom(env);
      auto minUnsignedAssignOperation = Methods::queryMinUnsignedAssign(*ppveMin);
      auto maxUnsignedAssignOperation = Methods::queryMaxUnsignedAssign(*ppveMin);
      ppveMin = Methods::applyAssign(ppveMin, maxUnsignedAssignOperation, *source.ppveMin, mayStopErrorStates);
      source.ppveMax = Methods::applyAssign(source.ppveMax, minUnsignedAssignOperation, *ppveMax, mayStopErrorStates);
   };

   DefineIntroConstraintInterval
   if (!env.getInformationKind().isMay()) {
      // as sure [a,b] -> [max(a,d), b] and [c,d] -> [c, min(d,a)]
      env.mergeVerdictDegradate(); // for the repartition of the central values
      EvaluationParameters surePropagateErrorStates(EPSurePropagateErrorStates);
      surePropagateErrorStates.setApproximatePartFrom(env);
      auto minUnsignedAssignOperation = Methods::queryMinUnsignedAssign(*ppveMin);
      auto maxUnsignedAssignOperation = Methods::queryMaxUnsignedAssign(*ppveMin);
      *sureMin = Methods::applyAssign(*sureMin, maxUnsignedAssignOperation,
            **sureSourceMax, surePropagateErrorStates);
      *sureSourceMax = Methods::applyAssign(*sureSourceMax, minUnsignedAssignOperation,
            **sureMin, surePropagateErrorStates);
   };
   DefineConclusionConstraintInterval

   return true;
}

bool
IntegerInterval::constraintCompareGreaterInterval(const VirtualOperation& operation, IntegerInterval& source,
      Argument arg, ConstraintEnvironment& env) { // [a,b] > [c,d] forced
   if (queryUnsignedField() != source.queryUnsignedField()) {
      PNT::TPassPointer<VirtualDisjunction, PPAbstractElement> copy = changeSign(env);
      if (copy.isValid()) {
         PPVirtualElement trueElement = Methods::newTrue(*copy);
         ConstraintTransmitEnvironment transmitEnv(env, source, arg);
         assume(copy->constraint(operation, *trueElement, transmitEnv));
         return true;
      };
   };
   env.mergeVerdictExact();

   if (Methods::applyBoolean(*ppveMin, querySCompareGreaterOperation(), *source.ppveMax))
      return true;
   if (Methods::applyBoolean(*ppveMax, querySCompareLessOrEqualOperation(), *source.ppveMin)) {
      env.setEmpty();
      return true;
   };

   if (!env.getInformationKind().isSure()) {
      // as may [a,b] -> [max(a,c+1), b] and [c,d] -> [c, min(d,b-1)]
      env.mergeVerdictDegradate(); // for the repartition of the central values
      EvaluationParameters mayStopErrorStates(EPMayStopErrorStates);
      mayStopErrorStates.setApproximatePartFrom(env);
      auto maxAssignOperation = querySMaxAssignOperation();
      auto nextAssignOperation = querySNextAssignOperation();
      auto minAssignOperation = querySMinAssignOperation();
      auto prevAssignOperation = querySPrevAssignOperation();
      ppveMin = Methods::applyAssign(ppveMin, maxAssignOperation,
         *Methods::applyAssign(PPVirtualElement(*source.ppveMin), nextAssignOperation,
            mayStopErrorStates), mayStopErrorStates);
      source.ppveMax = Methods::applyAssign(source.ppveMax, minAssignOperation,
         *Methods::applyAssign(PPVirtualElement(*ppveMax), prevAssignOperation,
            mayStopErrorStates), mayStopErrorStates);
   };

   DefineIntroConstraintInterval
   if (!env.getInformationKind().isMay()) {
      // as sur [a,b] -> [max(a,d+1), b] and [c,d] -> [c, min(d,a-1)]
      EvaluationParameters surePropagateErrorStates(EPSurePropagateErrorStates);
      surePropagateErrorStates.setApproximatePartFrom(env);
      *sureMin = Methods::applyAssign(*sureMin, querySMaxAssignOperation(),
         *Methods::applyAssign(PPVirtualElement(**sureSourceMax), querySNextAssignOperation(),
            surePropagateErrorStates), surePropagateErrorStates);
      *sureSourceMax = Methods::applyAssign(*sureSourceMax, querySMinAssignOperation(),
         *Methods::applyAssign(PPVirtualElement(**sureMin), querySPrevAssignOperation(),
            surePropagateErrorStates), surePropagateErrorStates);
   };
   DefineConclusionConstraintInterval

   return true;
}

bool
IntegerInterval::constraintCompareGreaterSignedInterval(const VirtualOperation& operation, IntegerInterval& source,
      Argument arg, ConstraintEnvironment& env) { // [a,b] > [c,d] forced
   if (hasUnsignedField()) {
      PNT::TPassPointer<VirtualDisjunction, PPAbstractElement> copy = changeSign(env);
      if (copy.isValid()) {
         PPVirtualElement trueElement = Methods::newTrue(*copy);
         ConstraintTransmitEnvironment transmitEnv(env, source, arg);
         assume(copy->constraint(operation, *trueElement, transmitEnv));
         return true;
      };
   };
   if (source.hasUnsignedField()) {
      PNT::TPassPointer<VirtualDisjunction, PPAbstractElement> copy = source.changeSign(env);
      if (copy.isValid()) {
         PPVirtualElement trueElement = Methods::newTrue(*copy);
         ConstraintTransmitEnvironment transmitEnv(env, copy, arg);
         assume(constraint(operation, *trueElement, transmitEnv));
         return true;
      };
   };
   env.mergeVerdictExact();

   if (Methods::applyBoolean(*ppveMin, Methods::queryCompareGreaterSigned(*ppveMin), *source.ppveMax))
      return true;
   if (Methods::applyBoolean(*ppveMax, Methods::queryCompareLessOrEqualSigned(*ppveMin), *source.ppveMin)) {
      env.setEmpty();
      return true;
   };

   if (!env.getInformationKind().isSure()) {
      // as may [a,b] -> [max(a,c+1), b] and [c,d] -> [c, min(d,b-1)]
      env.mergeVerdictDegradate(); // for the repartition of the central values
      EvaluationParameters mayStopErrorStates(EPMayStopErrorStates);
      mayStopErrorStates.setApproximatePartFrom(env);
      VirtualElement& min = *ppveMin;
      auto maxSignedAssignOperation = Methods::queryMaxSignedAssign(min);
      auto nextSignedAssignOperation = Methods::queryNextSignedAssign(min);
      auto minSignedAssignOperation = Methods::queryMinSignedAssign(min);
      auto prevSignedAssignOperation =  Methods::queryPrevSignedAssign(min);
      ppveMin = Methods::applyAssign(ppveMin, maxSignedAssignOperation,
         *Methods::applyAssign(PPVirtualElement(*source.ppveMin),
            nextSignedAssignOperation, mayStopErrorStates), mayStopErrorStates);
      source.ppveMax = Methods::applyAssign(source.ppveMax, minSignedAssignOperation,
         *Methods::applyAssign(PPVirtualElement(*ppveMax), prevSignedAssignOperation,
            mayStopErrorStates), mayStopErrorStates);
   };

   DefineIntroConstraintInterval
   if (!env.getInformationKind().isMay()) {
      // as sure [a,b] -> [max(a,d+1), b] and [c,d] -> [c, min(d,a-1)]
      EvaluationParameters surePropagateErrorStates(EPSurePropagateErrorStates);
      surePropagateErrorStates.setApproximatePartFrom(env);
      VirtualElement& min = *ppveMin;
      auto maxSignedAssignOperation = Methods::queryMaxSignedAssign(min);
      auto nextSignedAssignOperation = Methods::queryNextSignedAssign(min);
      auto minSignedAssignOperation = Methods::queryMinSignedAssign(min);
      auto prevSignedAssignOperation =  Methods::queryPrevSignedAssign(min);
      *sureMin = Methods::applyAssign(*sureMin, maxSignedAssignOperation,
         *Methods::applyAssign(PPVirtualElement(**sureSourceMax), nextSignedAssignOperation,
            surePropagateErrorStates), surePropagateErrorStates);
      *sureSourceMax = Methods::applyAssign(*sureSourceMax, minSignedAssignOperation,
         *Methods::applyAssign(PPVirtualElement(**sureMin), prevSignedAssignOperation,
            surePropagateErrorStates), surePropagateErrorStates);
   };
   DefineConclusionConstraintInterval

   return true;
}

bool
IntegerInterval::constraintCompareGreaterUnsignedInterval(const VirtualOperation& operation, IntegerInterval& source,
      Argument arg, ConstraintEnvironment& env) { // [a,b] > [c,d] forced
   if (!hasUnsignedField()) {
      PNT::TPassPointer<VirtualDisjunction, PPAbstractElement> copy = changeSign(env);
      if (copy.isValid()) {
         PPVirtualElement trueElement = Methods::newTrue(*copy);
         ConstraintTransmitEnvironment transmitEnv(env, source, arg);
         assume(copy->constraint(operation, *trueElement, transmitEnv));
         return true;
      };
   };
   if (!source.hasUnsignedField()) {
      PNT::TPassPointer<VirtualDisjunction, PPAbstractElement> copy = source.changeSign(env);
      if (copy.isValid()) {
         PPVirtualElement trueElement = Methods::newTrue(*copy);
         ConstraintTransmitEnvironment transmitEnv(env, copy, arg);
         assume(constraint(operation, *trueElement, transmitEnv));
         return true;
      };
   };
   env.mergeVerdictExact();

   if (Methods::applyBoolean(*ppveMin, Methods::queryCompareGreaterUnsigned(*ppveMin), *source.ppveMax))
      return true;
   if (Methods::applyBoolean(*ppveMax, Methods::queryCompareLessOrEqualUnsigned(*ppveMin), *source.ppveMin)) {
      env.setEmpty();
      return true;
   };

   if (!env.getInformationKind().isSure()) {
      // as may [a,b] -> [max(a,c+1), b] and [c,d] -> [c, min(d,b-1)]
      env.mergeVerdictDegradate(); // for the repartition of the central values
      EvaluationParameters mayStopErrorStates(EPMayStopErrorStates);
      mayStopErrorStates.setApproximatePartFrom(env);
      VirtualElement& min = *ppveMin;
      auto maxUnsignedAssignOperation = Methods::queryMaxUnsignedAssign(min);
      auto nextUnsignedAssignOperation = Methods::queryNextUnsignedAssign(min);
      auto minUnsignedAssignOperation = Methods::queryMinUnsignedAssign(min);
      auto prevUnsignedAssignOperation =  Methods::queryPrevUnsignedAssign(min);
      ppveMin = Methods::applyAssign(ppveMin, maxUnsignedAssignOperation,
         *Methods::applyAssign(PPVirtualElement(*source.ppveMin),
         nextUnsignedAssignOperation, mayStopErrorStates), mayStopErrorStates);
      source.ppveMax = Methods::applyAssign(source.ppveMax, minUnsignedAssignOperation,
         *Methods::applyAssign(PPVirtualElement(*ppveMax), prevUnsignedAssignOperation,
            mayStopErrorStates), mayStopErrorStates);
   };

   DefineIntroConstraintInterval
   if (!env.getInformationKind().isMay()) {
      // as sure [a,b] -> [max(a,d+1), b] and [c,d] -> [c, min(d,a-1)]
      EvaluationParameters surePropagateErrorStates(EPSurePropagateErrorStates);
      surePropagateErrorStates.setApproximatePartFrom(env);
      VirtualElement& min = *ppveMin;
      auto maxUnsignedAssignOperation = Methods::queryMaxUnsignedAssign(min);
      auto nextUnsignedAssignOperation = Methods::queryNextUnsignedAssign(min);
      auto minUnsignedAssignOperation = Methods::queryMinUnsignedAssign(min);
      auto prevUnsignedAssignOperation =  Methods::queryPrevUnsignedAssign(min);
      *sureMin = Methods::applyAssign(*sureMin, maxUnsignedAssignOperation,
         *Methods::applyAssign(PPVirtualElement(**sureSourceMax), nextUnsignedAssignOperation,
            surePropagateErrorStates), surePropagateErrorStates);
      *sureSourceMax = Methods::applyAssign(*sureSourceMax, minUnsignedAssignOperation,
         *Methods::applyAssign(PPVirtualElement(**sureMin), prevUnsignedAssignOperation,
            surePropagateErrorStates), surePropagateErrorStates);
   };
   DefineConclusionConstraintInterval

   return true;
}

#undef DefineIntroConstraintInterval
#undef DefineConclusionConstraintInterval

#if defined(__GNUC__) && GCC_VERSION >= 40600
#pragma GCC diagnostic pop
#endif

bool
IntegerInterval::constraintCompareMultiple(const VirtualOperation&, VirtualElement& source,
      Argument arg, ConstraintEnvironment& env) {
   env.mergeVerdictDegradate();
   return true;
}

bool
IntegerInterval::constraintCompareLessOrEqualGeneric(const VirtualOperation& operation,
      VirtualElement& source, Argument arg, ConstraintEnvironment& env) {
   ConstraintTransmitEnvironment transmitEnv(env, source, ConstraintEnvironment::Init(env));
   assume(ppveMin->constraint(operation,  *Methods::newTrue(*ppveMin), transmitEnv));
   if (transmitEnv.hasResult())
      ppveMin = transmitEnv.presult();
   if (arg.isResult())
      env.absorbFirstArgument((VirtualElement*) transmitEnv.extractFirstArgument());
   else
      env.presult().absorbElement(transmitEnv.extractFirstArgument());
   env.mergeVerdictDegradate();
   return true;
}

bool
IntegerInterval::constraintCompareLessOrEqualSignedGeneric(const VirtualOperation& operation,
      VirtualElement& source, Argument arg, ConstraintEnvironment& env) {
   if (hasUnsignedField()) {
      PNT::TPassPointer<VirtualDisjunction, PPAbstractElement> copy = changeSign(env);
      if (copy.isValid()) {
         PPVirtualElement trueElement = Methods::newTrue(*copy);
         ConstraintTransmitEnvironment transmitEnv(env, source, arg);
         assume(copy->constraint(operation, *trueElement, transmitEnv));
         return true;
      };
   };
   ConstraintTransmitEnvironment transmitEnv(env, source, ConstraintEnvironment::Init(env));
   assume(ppveMin->constraint(operation,  *Methods::newTrue(*ppveMin), transmitEnv));
   if (transmitEnv.hasResult())
      ppveMin = transmitEnv.presult();
   if (arg.isResult())
      env.absorbFirstArgument((VirtualElement*) transmitEnv.extractFirstArgument());
   else
      env.presult().absorbElement(transmitEnv.extractFirstArgument());
   env.mergeVerdictDegradate();
   return true;
}          

bool
IntegerInterval::constraintCompareLessOrEqualUnsignedGeneric(const VirtualOperation& operation,
      VirtualElement& source, Argument arg, ConstraintEnvironment& env) {
   if (!hasUnsignedField()) {
      PNT::TPassPointer<VirtualDisjunction, PPAbstractElement> copy = changeSign(env);
      if (copy.isValid()) {
         PPVirtualElement trueElement = Methods::newTrue(*copy);
         ConstraintTransmitEnvironment transmitEnv(env, source, arg);
         assume(copy->constraint(operation, *trueElement, transmitEnv));
         return true;
      };
   };
   ConstraintTransmitEnvironment transmitEnv(env, source, ConstraintEnvironment::Init(env));
   assume(ppveMin->constraint(operation,  *Methods::newTrue(*ppveMin), transmitEnv));
   if (transmitEnv.hasResult())
      ppveMin = transmitEnv.presult();
   if (arg.isResult())
      env.absorbFirstArgument((VirtualElement*) transmitEnv.extractFirstArgument());
   else
      env.presult().absorbElement(transmitEnv.extractFirstArgument());
   env.mergeVerdictDegradate();
   return true;
}          

bool
IntegerInterval::constraintCompareEqualGeneric(const VirtualOperation& operation,
      VirtualElement& asource, Argument arg, ConstraintEnvironment& env) {
   PPVirtualElement source(asource);
   {  ConstraintTransmitEnvironment transmitEnv(env, source, ConstraintEnvironment::Init(env));
      assume(ppveMin->constraint(querySCompareLessOrEqualOperation(),
         *Methods::newTrue(*ppveMin), transmitEnv));
      if (transmitEnv.hasResult())
         ppveMin = transmitEnv.presult();
      source.absorbElement((VirtualElement*) transmitEnv.extractFirstArgument());
   };
   {  ConstraintTransmitEnvironment transmitEnv(env, source, ConstraintEnvironment::Init(env));
      assume(ppveMax->constraint(querySCompareGreaterOrEqualOperation(),
         *Methods::newTrue(*ppveMax), transmitEnv));
      if (transmitEnv.hasResult())
         ppveMax = transmitEnv.presult();
      source.absorbElement((VirtualElement*) transmitEnv.extractFirstArgument());
   };
   if (arg.isResult())
      env.absorbFirstArgument(source.extractElement());
   else
      env.presult() = source;
   env.mergeVerdictDegradate();
   return true;
}          

bool
IntegerInterval::constraintCompareGreaterOrEqualGeneric(const VirtualOperation& operation,
      VirtualElement& source, Argument arg, ConstraintEnvironment& env) {
   ConstraintTransmitEnvironment transmitEnv(env, source, ConstraintEnvironment::Init(env));
   assume(ppveMax->constraint(operation,  *Methods::newTrue(*ppveMin), transmitEnv));
   if (transmitEnv.hasResult())
      ppveMax = transmitEnv.presult();
   if (arg.isResult())
      env.absorbFirstArgument((VirtualElement*) transmitEnv.extractFirstArgument());
   else
      env.presult().absorbElement(transmitEnv.extractFirstArgument());
   env.mergeVerdictDegradate();
   return true;
}

bool
IntegerInterval::constraintCompareGreaterOrEqualSignedGeneric(const VirtualOperation& operation,
      VirtualElement& source, Argument arg, ConstraintEnvironment& env) {
   if (hasUnsignedField()) {
      PNT::TPassPointer<VirtualDisjunction, PPAbstractElement> copy = changeSign(env);
      if (copy.isValid()) {
         PPVirtualElement trueElement = Methods::newTrue(*copy);
         ConstraintTransmitEnvironment transmitEnv(env, source, arg);
         assume(copy->constraint(operation, *trueElement, transmitEnv));
         return true;
      };
   };
   ConstraintTransmitEnvironment transmitEnv(env, source, ConstraintEnvironment::Init(env));
   assume(ppveMax->constraint(operation,  *Methods::newTrue(*ppveMin), transmitEnv));
   if (transmitEnv.hasResult())
      ppveMax = transmitEnv.presult();
   if (arg.isResult())
      env.absorbFirstArgument((VirtualElement*) transmitEnv.extractFirstArgument());
   else
      env.presult().absorbElement(transmitEnv.extractFirstArgument());
   env.mergeVerdictDegradate();
   return true;
}          

bool
IntegerInterval::constraintCompareGreaterOrEqualUnsignedGeneric(const VirtualOperation& operation,
      VirtualElement& source, Argument arg, ConstraintEnvironment& env) {
   if (!hasUnsignedField()) {
      PNT::TPassPointer<VirtualDisjunction, PPAbstractElement> copy = changeSign(env);
      if (copy.isValid()) {
         PPVirtualElement trueElement = Methods::newTrue(*copy);
         ConstraintTransmitEnvironment transmitEnv(env, source, arg);
         assume(copy->constraint(operation, *trueElement, transmitEnv));
         return true;
      };
   };
   ConstraintTransmitEnvironment transmitEnv(env, source, ConstraintEnvironment::Init(env));
   assume(ppveMax->constraint(operation,  *Methods::newTrue(*ppveMin), transmitEnv));
   if (transmitEnv.hasResult())
      ppveMax = transmitEnv.presult();
   if (arg.isResult())
      env.absorbFirstArgument((VirtualElement*) transmitEnv.extractFirstArgument());
   else
      env.presult().absorbElement(transmitEnv.extractFirstArgument());
   env.mergeVerdictDegradate();
   return true;
}          

bool
IntegerInterval::constraintCastBit(const VirtualOperation& operation, const VirtualElement& result, ConstraintEnvironment& env) {
   ZeroResult resultAsBoolean = result.queryZeroResult();
   if (resultAsBoolean.mayBeZero() && resultAsBoolean.mayBeDifferentZero())
      return true;
   PPVirtualElement zero = Methods::newZero(*this);
   if (resultAsBoolean.isZero())
      return intersectWith(*zero, env);
   return constraintCompareDifferentConstant(Methods::queryCompareDifferent(*this), *zero, Argument().setResult(), env);
}

}} // end of namespace Approximate::Details

namespace MultiBit { namespace Approximate {

bool
Interval::applyCastShiftBit(const VirtualOperation& aoperation, EvaluationEnvironment& env)
   {  AssumeCondition(dynamic_cast<const CastShiftBitOperation*>(&aoperation))
      int shift = ((const CastShiftBitOperation&) aoperation).getShift();
      PPVirtualElement one = Methods::newOne(*this);
      int size = one->getSizeInBits();
      if (shift < 0 || shift >= size)
         return false;
      if (shift > 0) {
         Implementation::MultiBitElement shiftImplementation(one->getSizeInBits());
         shiftImplementation[0] = shift;
         PPVirtualElement shiftElement = Methods::newConstant(*one, shiftImplementation);
         PPVirtualElement intervalShifted;
         {  TransmitEnvironment localEnv(env);
            localEnv.setFirstArgument(*shiftElement);
            if (!hasUnsignedField())
               assume(apply(ArithmeticRightShiftAssignOperation().setConstWithAssign(),
                     localEnv));
            else
               assume(apply(LogicalRightShiftAssignOperation().setConstWithAssign(),
                     localEnv));
            intervalShifted = localEnv.presult();
         }
         return intervalShifted->apply(CastShiftBitOperation(0), env);
      }
      TransmitEnvironment localEnv(env);
      localEnv.setFirstArgument(*smax());
      smin()->apply(CompareEqualOperation(), localEnv);
      ZeroResult compareResult = localEnv.presult()->queryZeroResult();
      if (compareResult.isTrue())
         return smin()->apply(aoperation, env);
      env.mergeVerdictDegradate();
      PNT::TPassPointer<VirtualDisjunction, PPAbstractElement> disjunction
         = Methods::newBooleanDisjunction(*smin());
      disjunction->add(Methods::newFalse(*smin()), env.getInformationKind());
      disjunction->add(Methods::newTrue(*smin()), env.getInformationKind());
      env.presult() = disjunction;
      return true;
   }

bool
Interval::applyCastMultiFloat(const VirtualOperation& aoperation, EvaluationEnvironment& env)
   {  AssumeCondition(dynamic_cast<const CastMultiFloatOperation*>(&aoperation))
      const auto& operation = (const CastMultiFloatOperation&) aoperation;
      PPVirtualElement floatConstant = Details::IntOperationElement::Methods
         ::newMultiFloatConstant(*smin(), VirtualElement::InitFloat()
            .setSizeExponent(operation.getSizeExponent())
            .setSizeMantissa(operation.getSizeMantissa()));
      env.mergeVerdictDegradate();
      env.presult() = Scalar::Approximate::Details::RealOperationElement::Methods
         ::newTop(*floatConstant);
      return true;
   }

bool
Interval::applyCastMultiFloatPointer(const VirtualOperation& aoperation, EvaluationEnvironment& env)
   {  AssumeCondition(dynamic_cast<const CastMultiFloatPointerOperation*>(&aoperation))
      const auto& operation = (const CastMultiFloatPointerOperation&) aoperation;
      PPVirtualElement floatConstant = Details::IntOperationElement::Methods
         ::newMultiFloatConstant(*smin(), VirtualElement::InitFloat()
            .setSizeExponent(operation.getSizeExponent())
            .setSizeMantissa(operation.getSizeMantissa()));
      env.mergeVerdictDegradate();
      env.presult() = Scalar::Approximate::Details::RealOperationElement::Methods
         ::newTop(*floatConstant);
      return true;
   }

bool
Interval::constraintCastShiftBit(const VirtualOperation& operation, const VirtualElement& result, ConstraintEnvironment& env) {
   env.mergeVerdictDegradate();
   return true;
}

bool
Interval::constraintCastMultiFloat(const VirtualOperation& operation, const VirtualElement& result, ConstraintEnvironment& env) {
   env.mergeVerdictDegradate();
   return true;
}

bool
Interval::constraintCastMultiFloatPointer(const VirtualOperation& operation, const VirtualElement& result, ConstraintEnvironment& env) {
   env.mergeVerdictDegradate();
   return true;
}

#define DefineInitApply(TypeOperation)                                                             \
   elementAt(Operation::T##TypeOperation).setMethod(&IntegerInterval::apply##TypeOperation);

#define DefineInitOApply(TypeOperation, TypeRedirection)                                           \
   elementAt(Operation::T##TypeOperation).setMethod(&IntegerInterval::apply##TypeRedirection);

#define DefineInitBiApply(TypeOperation)                                                           \
   elementAt(Operation::T##TypeOperation).setArray(new ConstantAndIntervalApplyMethod(             \
      &IntegerInterval::apply##TypeOperation##Constant,                                            \
      &IntegerInterval::apply##TypeOperation##Interval));

#define DefineInitOBiApply(TypeOperation, TypeMethod)                                                           \
   elementAt(Operation::T##TypeOperation).setArray(new ConstantAndIntervalApplyMethod(             \
      &IntegerInterval::apply##TypeMethod##Constant,                                               \
      &IntegerInterval::apply##TypeMethod##Interval));

Interval::MethodApplyTable::MethodApplyTable() {
#include "StandardClasses/UndefineNew.h"
   clear();

   DefineInitApply(CastBit)
   elementAt(Operation::TCastShiftBit).setMethod(&Interval::applyCastShiftBit);
   elementAt(Operation::TCastMultiFloat).setMethod(&Interval::applyCastMultiFloat);
   elementAt(Operation::TCastMultiFloatPointer).setMethod(&Interval::applyCastMultiFloatPointer);
   DefineInitOApply(CastMultiBit, DispatchMinMax)
   DefineInitOApply(ExtendWithZero, DispatchMinMax)
   DefineInitOApply(ExtendWithSign, DispatchMinMax)
   DefineInitOApply(Reduce, DispatchMinMax) // AssumeUnimplemented
   DefineInitApply(BitSet)
   DefineInitApply(Concat)
   DefineInitOApply(PrevSignedAssign, PrevAssign)
   DefineInitOApply(PrevUnsignedAssign, PrevAssign)
   DefineInitOApply(NextSignedAssign, NextAssign)
   DefineInitOApply(NextUnsignedAssign, NextAssign)
   DefineInitOBiApply(PlusSignedAssign, PlusAssign)
   DefineInitOBiApply(PlusUnsignedAssign, PlusAssign)
   DefineInitOBiApply(PlusUnsignedWithSignedAssign, PlusAssign)
   // DefineInitApply(PlusFloatAssign)
   DefineInitOBiApply(MinusSignedAssign, MinusAssign)
   DefineInitOBiApply(MinusUnsignedAssign, MinusAssign)
   // DefineInitApply(MinusFloatAssign)

   DefineInitOBiApply(TimesSignedAssign, TimesAssign)
   DefineInitOBiApply(TimesUnsignedAssign, TimesAssign)
   DefineInitOBiApply(DivideSignedAssign, DivideAssign)
   DefineInitOBiApply(DivideUnsignedAssign, DivideAssign)
   DefineInitOApply(OppositeSignedAssign, OppositeAssign)

   DefineInitOBiApply(CompareLessSigned, CompareLess)
   DefineInitOBiApply(CompareLessOrEqualSigned, CompareLessOrEqual)
   DefineInitOBiApply(CompareLessUnsigned, CompareLess)
   DefineInitOBiApply(CompareLessOrEqualUnsigned, CompareLessOrEqual)
   // DefineInitApplyOMethodTable(CompareLessFloat)
   // DefineInitApplyOMethodTable(CompareLessOrEqualFloat)
   DefineInitBiApply(CompareEqual)
   // DefineInitApplyOMethodTable(CompareEqualFloat)
   DefineInitBiApply(CompareDifferent)
   // DefineInitApplyOMethodTable(CompareDifferentFloat)
   DefineInitOBiApply(CompareGreaterOrEqualUnsigned, CompareGreaterOrEqual)
   DefineInitOBiApply(CompareGreaterUnsigned, CompareGreater)
   DefineInitOBiApply(CompareGreaterOrEqualSigned, CompareGreaterOrEqual)
   DefineInitOBiApply(CompareGreaterSigned, CompareGreater)
   // DefineInitApplyOMethodTable(CompareGreaterFloat)
   // DefineInitApplyOMethodTable(CompareGreaterOrEqualFloat)
   DefineInitOBiApply(MinSignedAssign, MinAssign)
   DefineInitOBiApply(MinUnsignedAssign, MinAssign)
   // DefineInitBiApply(MinFloatAssign)
   DefineInitOBiApply(MaxSignedAssign, MaxAssign)
   DefineInitOBiApply(MaxUnsignedAssign, MaxAssign)
   // DefineInitBiApply(MaxFloatAssign)

   DefineInitOBiApply(ModuloSignedAssign, ModuloAssign)
   DefineInitOBiApply(ModuloUnsignedAssign, ModuloAssign)
   DefineInitBiApply(BitOrAssign)
   DefineInitBiApply(BitAndAssign)
   DefineInitBiApply(BitExclusiveOrAssign)
   DefineInitBiApply(LeftShiftAssign)
   DefineInitOBiApply(LogicalRightShiftAssign, RightShiftAssign)
   DefineInitOBiApply(ArithmeticRightShiftAssign, RightShiftAssign)
   DefineInitOBiApply(LeftRotateAssign, LeftShiftAssign)
   DefineInitOBiApply(RightRotateAssign, RightShiftAssign)
   DefineInitApply(BitNegateAssign)
#include "StandardClasses/DefineNew.h"
}

#undef DefineInitApply
#undef DefineInitBiApply

Interval::MethodApplyTable Interval::matMethodApplyTable;

#define DefineInitUnaryConstraint(TypeOperation) \
   mcutUnaryTable[Operation::T##TypeOperation].setMethod(&IntegerInterval::constraint##TypeOperation);

#define DefineInitUnaryGConstraint(TypeOperation) \
   mcutUnaryTable[Operation::T##TypeOperation].setFunction(&constraint##TypeOperation);

#define DefineInitUnaryOConstraint(TypeOperation, TypeMethod) \
   mcutUnaryTable[Operation::T##TypeOperation].setMethod(&IntegerInterval::constraint##TypeMethod);

#define DefineInitUnaryOGConstraint(TypeOperation, TypeMethod) \
   mcutUnaryTable[Operation::T##TypeOperation].setFunction(&constraint##TypeMethod);

#define DefineInitBinaryConstraint(TypeOperation) \
   mcbtBinaryTable[Operation::T##TypeOperation].setMethod(&IntegerInterval::constraint##TypeOperation);

#define DefineInitBinaryConstraintOnlyGeneric(TypeOperation) \
   mcbtBinaryTable[Operation::T##TypeOperation].setFunction(&IntegerInterval::constraint##TypeOperation);

#define DefineInitBinaryConstraintGeneric(TypeOperation)                                         \
   mcbtBinaryTable[Operation::T##TypeOperation].setArray(                                        \
      new Scalar::Approximate::Details::GenericBinaryConstraintMethod<DomainTraits>(&IntegerInterval::constraint##TypeOperation));

#define DefineInitOBinaryConstraintGeneric(TypeOperation, TypeMethod)                            \
   mcbtBinaryTable[Operation::T##TypeOperation].setArray(                                        \
      new Scalar::Approximate::Details::GenericBinaryConstraintMethod<DomainTraits>(&IntegerInterval::constraint##TypeMethod));

#define DefineInitOCompareConstraint(TypeOperation, TypeMethod, TypeGeneric)                     \
   mcctCompareTable[Operation::T##TypeOperation].setArray(                                       \
      new ConstantIntervalAndGenericConstraintCompareMethod(                                     \
         &IntegerInterval::constraint##TypeMethod##Constant,                                     \
         &IntegerInterval::constraint##TypeMethod##Interval,                                     \
         &IntegerInterval::constraint##TypeGeneric));

Interval::MethodConstraintTable::MethodConstraintTable() {
#include "StandardClasses/UndefineNew.h"
   DefineInitUnaryConstraint(CastBit)
   mcutUnaryTable[Operation::TCastShiftBit].setMethod(&Interval::constraintCastShiftBit);
   mcutUnaryTable[Operation::TCastMultiFloat].setMethod(&Interval::constraintCastMultiFloat);
   mcutUnaryTable[Operation::TCastMultiFloatPointer].setMethod(&Interval::constraintCastMultiFloatPointer);
/* DefineInitUnaryOGConstraint(CastChar, CastInteger)
   DefineInitUnaryOGConstraint(CastSignedChar, CastInteger)
   DefineInitUnaryOGConstraint(CastUnsignedChar, CastInteger)
   DefineInitUnaryOGConstraint(CastShort, CastInteger)
   DefineInitUnaryOGConstraint(CastUnsignedShort, CastInteger)
   DefineInitUnaryOGConstraint(CastInt, CastInteger)
   DefineInitUnaryOGConstraint(CastUnsignedInt, CastInteger)
   DefineInitUnaryOGConstraint(CastLong, CastInteger)
   DefineInitUnaryOGConstraint(CastUnsignedLong, CastInteger) */
   // DefineInitUnaryOGConstraint(CastFloat, CastDouble)
   // DefineInitUnaryOGConstraint(CastDouble, CastDouble)
   // DefineInitUnaryOGConstraint(CastLongDouble, CastDouble)

   DefineInitOCompareConstraint(CompareLessSigned, CompareLessSigned, CompareLessOrEqualSignedGeneric)
   DefineInitOCompareConstraint(CompareLessUnsigned, CompareLessUnsigned, CompareLessOrEqualUnsignedGeneric)
   DefineInitOCompareConstraint(CompareLessOrEqualSigned, CompareLessOrEqualSigned, CompareLessOrEqualSignedGeneric)
   DefineInitOCompareConstraint(CompareLessOrEqualUnsigned, CompareLessOrEqualUnsigned, CompareLessOrEqualUnsignedGeneric)
   DefineInitOCompareConstraint(CompareEqual, CompareEqual, CompareEqualGeneric)
   DefineInitOCompareConstraint(CompareDifferent, CompareDifferent, CompareMultiple)
   DefineInitOCompareConstraint(CompareGreaterOrEqualSigned, CompareGreaterOrEqualSigned, CompareGreaterOrEqualSignedGeneric)
   DefineInitOCompareConstraint(CompareGreaterOrEqualUnsigned, CompareGreaterOrEqualUnsigned, CompareGreaterOrEqualUnsignedGeneric)
   DefineInitOCompareConstraint(CompareGreaterSigned, CompareGreaterSigned, CompareGreaterOrEqualSignedGeneric)
   DefineInitOCompareConstraint(CompareGreaterUnsigned, CompareGreaterUnsigned, CompareGreaterOrEqualUnsignedGeneric)
   DefineInitOBinaryConstraintGeneric(MinUnsignedAssign, MinAssign)
   DefineInitOBinaryConstraintGeneric(MinSignedAssign, MinAssign)
   DefineInitOBinaryConstraintGeneric(MaxUnsignedAssign, MaxAssign)
   DefineInitOBinaryConstraintGeneric(MaxSignedAssign, MaxAssign)

   DefineInitUnaryOGConstraint(PrevSignedAssign, PrevAssign)
   DefineInitUnaryOGConstraint(PrevUnsignedAssign, PrevAssign)
   DefineInitUnaryOGConstraint(NextSignedAssign, NextAssign)
   DefineInitUnaryOGConstraint(NextUnsignedAssign, NextAssign)

   DefineInitBinaryConstraintOnlyGeneric(Concat)
   DefineInitBinaryConstraintOnlyGeneric(BitSet)
   DefineInitOBinaryConstraintGeneric(PlusSignedAssign, PlusAssign)
   DefineInitOBinaryConstraintGeneric(PlusUnsignedAssign, PlusAssign)
   DefineInitOBinaryConstraintGeneric(PlusUnsignedWithSignedAssign, PlusAssign)
   DefineInitOBinaryConstraintGeneric(MinusSignedAssign, MinusAssign)
   DefineInitOBinaryConstraintGeneric(MinusUnsignedAssign, MinusAssign)
   DefineInitOBinaryConstraintGeneric(TimesSignedAssign, TimesAssign)
   DefineInitOBinaryConstraintGeneric(TimesUnsignedAssign, TimesAssign)
   DefineInitOBinaryConstraintGeneric(DivideSignedAssign, DivideAssign)
   DefineInitOBinaryConstraintGeneric(DivideUnsignedAssign, DivideAssign)

   DefineInitUnaryOGConstraint(OppositeSignedAssign, OppositeAssign)

   DefineInitOBinaryConstraintGeneric(ModuloSignedAssign, ModuloAssign)
   DefineInitOBinaryConstraintGeneric(ModuloUnsignedAssign, ModuloAssign)
   DefineInitBinaryConstraintGeneric(BitOrAssign)
   DefineInitBinaryConstraintGeneric(BitAndAssign)
   DefineInitOBinaryConstraintGeneric(BitExclusiveOrAssign, BinaryNoPropagation)
   DefineInitOBinaryConstraintGeneric(LeftShiftAssign, BinaryNoPropagation)
   DefineInitOBinaryConstraintGeneric(LogicalRightShiftAssign, BinaryNoPropagation)
   DefineInitOBinaryConstraintGeneric(ArithmeticRightShiftAssign, BinaryNoPropagation)
   DefineInitUnaryGConstraint(BitNegateAssign)
#include "StandardClasses/DefineNew.h"
}

#undef DefineInitUnaryConstraint
#undef DefineInitUnaryGConstraint
#undef DefineInitUnaryOConstraint
#undef DefineInitUnaryOGConstraint
#undef DefineInitBinaryConstraint
#undef DefineInitBinaryConstraintOnlyGeneric
#undef DefineInitBinaryConstraintGeneric
#undef DefineInitOBinaryConstraintGeneric
#undef DefineInitOCompareConstraint

Interval::MethodConstraintTable Interval::mctMethodConstraintTable;

#define DefineInitQuery(TypeOperation) \
   elementAt(QueryOperation::TypeOperation).setMethod(&IntegerInterval::query##TypeOperation);

Interval::QueryTable::QueryTable() {
#include "StandardClasses/UndefineNew.h"
   FunctionQueryTable* table = &elementAt(VirtualQueryOperation::TCompareSpecial);
   table->setSize(QueryOperation::EndOfTypeCompareSpecial);
   (*table)[QueryOperation::TCSUndefined].setMethod(&Interval::queryCompareSpecial);
   (*table)[QueryOperation::TCSGuard].setMethod(&Interval::queryGuardAll);
   (*table)[QueryOperation::TCSNatural].setMethod(&Interval::queryCompareSpecial);
   (*table)[QueryOperation::TCSSigned].setMethod(&Interval::queryCompareSpecialSigned);
   (*table)[QueryOperation::TCSUnsigned].setMethod(&Interval::queryCompareSpecialUnsigned);
   (*table)[QueryOperation::TCSBitDomain].setMethod(&Interval::queryBitDomain);
   
   table = &elementAt(VirtualQueryOperation::TSimplification);
   table->setSize(QueryOperation::EndOfTypeSimplification);
   (*table)[QueryOperation::TSUndefined].setMethod(&Interval::querySimplification);
   (*table)[QueryOperation::TSConstant].setMethod(&Top::queryFailSimplification);
   (*table)[QueryOperation::TSInterval].setMethod(&Interval::queryIdentity);
   (*table)[QueryOperation::TSConstantDisjunction].setMethod(&Interval::querySimplificationAsConstantDisjunction);

   table = &elementAt(VirtualQueryOperation::TDomain);
   table->setSize(QueryOperation::EndOfTypeDomain);

#define DefineInitQueryMethodTable(TypeOperation)                              \
   (*table)[QueryOperation::TD##TypeOperation].setMethod(&Interval::queryDispatchOnMin);
#define DefineInitQueryOMethodTable(TypeOperation, TypeMethod)                 \
   (*table)[QueryOperation::TD##TypeOperation].setMethod(&Interval::queryDispatchOnMin);
   
   DefineInitQueryMethodTable(Disjunction)
   DefineInitQueryMethodTable(MultiBitConstant)
   DefineInitQueryMethodTable(MultiFloatConstant)
   DefineInitQueryMethodTable(Interval)
   DefineInitQueryMethodTable(BooleanDisjunction)
   DefineInitQueryMethodTable(Top)
   DefineInitQueryMethodTable(False)
   DefineInitQueryMethodTable(True)
   DefineInitQueryMethodTable(IntForShift)
   DefineInitQueryMethodTable(Constant)
   DefineInitQueryMethodTable(Zero)
   DefineInitQueryMethodTable(MinusOne)
   DefineInitQueryMethodTable(One)
   DefineInitQueryOMethodTable(Min, MinUnsigned)
   DefineInitQueryMethodTable(MinSigned)
   DefineInitQueryMethodTable(MinUnsigned)
   DefineInitQueryOMethodTable(Max, MaxUnsigned)
   DefineInitQueryMethodTable(MaxSigned)
   DefineInitQueryMethodTable(MaxUnsigned)

#undef DefineInitQueryMethodTable
#undef DefineInitQueryOMethodTable

   table = &elementAt(VirtualQueryOperation::TDuplication);
   table->setSize(3);
   (*table)[QueryOperation::TCDClone].setMethod(&Interval::queryCopy);
   (*table)[QueryOperation::TCDSpecialize].setMethod(&Interval::querySpecialize);

   table = &elementAt(VirtualQueryOperation::TExtern);
   table->setSize(QueryOperation::EndOfTypeExtern);
   (*table)[QueryOperation::TESigned].setMethod(&Interval::queryDispatchOnMin);
#include "StandardClasses/DefineNew.h"
}
#undef DefineInitQuery

Interval::QueryTable Interval::mqtMethodQueryTable;

#define DefineInitApplyTo(TypeOperation)                                                        \
   elementAt(Operation::T##TypeOperation).setMethod(&Interval::applyTo##TypeOperation);
#define DefineInitOApplyTo(TypeOperation, TypeMethod)                                           \
   elementAt(Operation::T##TypeOperation).setMethod(&Interval::applyTo##TypeMethod);
#define DefineInitApplyToConstant(TypeOperation)                                                \
   elementAt(Operation::T##TypeOperation).setMethod(&Interval::applyTo##TypeOperation##Constant);
#define DefineInitOApplyToConstant(TypeOperation, TypeMethod)                                   \
   elementAt(Operation::T##TypeOperation).setMethod(&Interval::applyTo##TypeMethod##Constant);

Interval::MethodApplyToTable::MethodApplyToTable() {
#include "StandardClasses/UndefineNew.h"
   clear();
   DefineInitApplyToConstant(Concat)
   DefineInitApplyToConstant(BitSet)
   DefineInitOApplyToConstant(CompareLessSigned, CompareLess)
   DefineInitOApplyToConstant(CompareLessUnsigned, CompareLess)
   DefineInitOApplyToConstant(CompareLessOrEqualSigned, CompareLessOrEqual)
   DefineInitOApplyToConstant(CompareLessOrEqualUnsigned, CompareLessOrEqual)
   DefineInitApplyToConstant(CompareEqual)
   DefineInitApplyToConstant(CompareDifferent)
   DefineInitOApplyToConstant(CompareGreaterOrEqualSigned, CompareGreaterOrEqual)
   DefineInitOApplyToConstant(CompareGreaterOrEqualUnsigned, CompareGreaterOrEqual)
   DefineInitOApplyToConstant(CompareGreaterSigned, CompareGreater)
   DefineInitOApplyToConstant(CompareGreaterUnsigned, CompareGreater)
   DefineInitOApplyToConstant(MinSignedAssign, MinAssign)
   DefineInitOApplyToConstant(MinUnsignedAssign, MinAssign)
   DefineInitOApplyToConstant(MaxSignedAssign, MaxAssign)
   DefineInitOApplyToConstant(MaxUnsignedAssign, MaxAssign)

   DefineInitOApplyToConstant(PlusSignedAssign, PlusAssign)
   DefineInitOApplyToConstant(PlusUnsignedAssign, PlusAssign)
   DefineInitOApplyToConstant(PlusUnsignedWithSignedAssign, PlusAssign)
   DefineInitOApplyToConstant(MinusSignedAssign, MinusAssign)
   DefineInitOApplyToConstant(MinusUnsignedAssign, MinusAssign)
   DefineInitOApplyToConstant(TimesSignedAssign, TimesAssign)
   DefineInitOApplyToConstant(TimesUnsignedAssign, TimesAssign)
   DefineInitOApplyTo(DivideSignedAssign, DivideAssign)
   DefineInitOApplyTo(DivideUnsignedAssign, DivideAssign)

   DefineInitOApplyTo(ModuloSignedAssign, ModuloAssign)
   DefineInitOApplyTo(ModuloUnsignedAssign, ModuloAssign)
   DefineInitApplyToConstant(BitOrAssign)
   DefineInitApplyToConstant(BitAndAssign)
   DefineInitApplyToConstant(BitExclusiveOrAssign)
   DefineInitApplyToConstant(LeftShiftAssign)
   DefineInitOApplyToConstant(LogicalRightShiftAssign, RightShiftAssign)
   DefineInitOApplyToConstant(ArithmeticRightShiftAssign, RightShiftAssign)
   DefineInitOApplyToConstant(LeftRotateAssign, RightShiftAssign)
   DefineInitOApplyToConstant(RightRotateAssign, RightShiftAssign)
#include "StandardClasses/DefineNew.h"
}
#undef DefineInitOApplyTo
#undef DefineInitApplyTo
#undef DefineInitApplyToConstant
#undef DefineInitOApplyToConstant

Interval::MethodApplyToTable Interval::mattMethodApplyToTable;

#define DefineTypeOperation MultiBit
#define DefineTypeObject Interval
#define DefineTypeInterval
#include "Analyzer/Scalar/Approximate/VirtualCall.incc"
#undef DefineTypeInterval
#undef DefineTypeObject
#undef DefineTypeOperation

}} // end of namespace MultiBit::Approximate

namespace Integer { namespace Approximate {

/**************************/
/* initialization methods */
/**************************/

#define DefineInitApply(TypeOperation)                                                             \
   elementAt(Operation::T##TypeOperation).setMethod(&IntegerInterval::apply##TypeOperation);

#define DefineInitOApply(TypeOperation, TypeRedirection)                                           \
   elementAt(Operation::T##TypeOperation).setMethod(&IntegerInterval::apply##TypeRedirection);

#define DefineInitBiApply(TypeOperation)                                                           \
   elementAt(Operation::T##TypeOperation).setArray(new ConstantAndIntervalApplyMethod(             \
      &IntegerInterval::apply##TypeOperation##Constant,                                            \
      &IntegerInterval::apply##TypeOperation##Interval));

Interval::MethodApplyTable::MethodApplyTable() {
#include "StandardClasses/UndefineNew.h"
   clear();

   DefineInitOApply(CastChar, CastInteger)
   DefineInitOApply(CastSignedChar, CastInteger)
   DefineInitOApply(CastUnsignedChar, CastInteger)
   DefineInitOApply(CastShort, CastInteger)
   DefineInitOApply(CastUnsignedShort, CastInteger)
   DefineInitOApply(CastInt, CastInteger)
   DefineInitOApply(CastUnsignedInt, CastInteger)
   DefineInitOApply(CastLong, CastInteger)
   DefineInitOApply(CastUnsignedLong, CastInteger)
   DefineInitOApply(CastFloat, CastDouble)
   DefineInitOApply(CastDouble, CastDouble)
   DefineInitOApply(CastLongDouble, CastDouble)

   DefineInitApply(PrevAssign)
   DefineInitApply(NextAssign)
   DefineInitBiApply(PlusAssign)
   DefineInitBiApply(MinusAssign)

   DefineInitBiApply(CompareLess)
   DefineInitBiApply(CompareLessOrEqual)
   DefineInitBiApply(CompareEqual)
   DefineInitBiApply(CompareDifferent)
   DefineInitBiApply(CompareGreaterOrEqual)
   DefineInitBiApply(CompareGreater)

   DefineInitBiApply(MinAssign)
   DefineInitBiApply(MaxAssign)
   DefineInitBiApply(TimesAssign)
   DefineInitBiApply(DivideAssign)
   DefineInitApply(OppositeAssign)

   DefineInitBiApply(ModuloAssign)
   DefineInitBiApply(BitOrAssign)
   DefineInitBiApply(BitAndAssign)
   DefineInitBiApply(BitExclusiveOrAssign)
   DefineInitBiApply(LeftShiftAssign)
   DefineInitBiApply(RightShiftAssign)
   DefineInitApply(BitNegateAssign)

   DefineInitBiApply(LogicalAndAssign)
   DefineInitBiApply(LogicalOrAssign)
   DefineInitApply(LogicalNegateAssign)
#include "StandardClasses/DefineNew.h"
}

#undef DefineInitApply
#undef DefineInitBiApply

Interval::MethodApplyTable Interval::matMethodApplyTable;

#define DefineInitUnaryConstraint(TypeOperation) \
   mcutUnaryTable[Operation::T##TypeOperation].setMethod(&IntegerInterval::constraint##TypeOperation);

#define DefineInitUnaryGConstraint(TypeOperation) \
   mcutUnaryTable[Operation::T##TypeOperation].setFunction(&constraint##TypeOperation);

#define DefineInitUnaryOConstraint(TypeOperation, TypeMethod) \
   mcutUnaryTable[Operation::T##TypeOperation].setMethod(&IntegerInterval::constraint##TypeMethod);

#define DefineInitUnaryOGConstraint(TypeOperation, TypeMethod) \
   mcutUnaryTable[Operation::T##TypeOperation].setFunction(&constraint##TypeMethod);

#define DefineInitBinaryConstraint(TypeOperation) \
   mcbtBinaryTable[Operation::T##TypeOperation].setMethod(&IntegerInterval::constraint##TypeOperation);

#define DefineInitBinaryConstraintGeneric(TypeOperation)                                          \
   mcbtBinaryTable[Operation::T##TypeOperation].setArray(                                         \
      new Scalar::Approximate::Details::GenericBinaryConstraintMethod<DomainTraits>(&IntegerInterval::constraint##TypeOperation));

#define DefineInitOBinaryConstraintGeneric(TypeOperation, TypeMethod)                             \
   mcbtBinaryTable[Operation::T##TypeOperation].setArray(                                         \
      new Scalar::Approximate::Details::GenericBinaryConstraintMethod<DomainTraits>(&IntegerInterval::constraint##TypeMethod));

#define DefineInitCompareConstraint(TypeOperation)                                                \
   mcctCompareTable[Operation::T##TypeOperation].setArray(                                        \
      new ConstantIntervalAndGenericConstraintCompareMethod(                                      \
         &IntegerInterval::constraint##TypeOperation##Constant,                                   \
         &IntegerInterval::constraint##TypeOperation##Interval,                                   \
         &IntegerInterval::constraintCompareMultiple));

Interval::MethodConstraintTable::MethodConstraintTable() {
#include "StandardClasses/UndefineNew.h"
   DefineInitUnaryOGConstraint(CastChar, CastInteger)
   DefineInitUnaryOGConstraint(CastSignedChar, CastInteger)
   DefineInitUnaryOGConstraint(CastUnsignedChar, CastInteger)
   DefineInitUnaryOGConstraint(CastShort, CastInteger)
   DefineInitUnaryOGConstraint(CastUnsignedShort, CastInteger)
   DefineInitUnaryOGConstraint(CastInt, CastInteger)
   DefineInitUnaryOGConstraint(CastUnsignedInt, CastInteger)
   DefineInitUnaryOGConstraint(CastLong, CastInteger)
   DefineInitUnaryOGConstraint(CastUnsignedLong, CastInteger)
   // DefineInitUnaryOGConstraint(CastFloat, CastDouble)
   // DefineInitUnaryOGConstraint(CastDouble, CastDouble)
   // DefineInitUnaryOGConstraint(CastLongDouble, CastDouble)

   DefineInitCompareConstraint(CompareLess)
   DefineInitCompareConstraint(CompareLessOrEqual)
   DefineInitCompareConstraint(CompareEqual)
   DefineInitCompareConstraint(CompareDifferent)
   DefineInitCompareConstraint(CompareGreaterOrEqual)
   DefineInitCompareConstraint(CompareGreater)
   DefineInitBinaryConstraintGeneric(MinAssign)
   DefineInitBinaryConstraintGeneric(MaxAssign)

   DefineInitUnaryGConstraint(PrevAssign)
   DefineInitUnaryGConstraint(NextAssign)

   DefineInitBinaryConstraintGeneric(PlusAssign)
   DefineInitBinaryConstraintGeneric(MinusAssign)
   DefineInitBinaryConstraintGeneric(TimesAssign)
   DefineInitBinaryConstraintGeneric(DivideAssign)

   DefineInitUnaryGConstraint(OppositeAssign)

   DefineInitBinaryConstraintGeneric(ModuloAssign)
   DefineInitBinaryConstraintGeneric(BitOrAssign)
   DefineInitBinaryConstraintGeneric(BitAndAssign)
   DefineInitOBinaryConstraintGeneric(BitExclusiveOrAssign, BinaryNoPropagation)
   DefineInitOBinaryConstraintGeneric(LeftShiftAssign, BinaryNoPropagation)
   DefineInitOBinaryConstraintGeneric(RightShiftAssign, BinaryNoPropagation)
   DefineInitUnaryGConstraint(BitNegateAssign)

   DefineInitBinaryConstraintGeneric(LogicalAndAssign)
   DefineInitBinaryConstraintGeneric(LogicalOrAssign)
   DefineInitUnaryGConstraint(LogicalNegateAssign)
#include "StandardClasses/DefineNew.h"
}

#undef DefineInitUnaryConstraint
#undef DefineInitUnaryGConstraint
#undef DefineInitUnaryOConstraint
#undef DefineInitUnaryOGConstraint
#undef DefineInitBinaryConstraint
#undef DefineInitBinaryConstraintGeneric
#undef DefineInitOBinaryConstraintGeneric
#undef DefineInitCompareConstraint

Interval::MethodConstraintTable Interval::mctMethodConstraintTable;

#define DefineInitQuery(TypeOperation) \
   elementAt(QueryOperation::TypeOperation).setMethod(&IntegerInterval::query##TypeOperation);

Interval::QueryTable::QueryTable() {
#include "StandardClasses/UndefineNew.h"
   FunctionQueryTable* table = &elementAt(VirtualQueryOperation::TCompareSpecial);
   table->setSize(QueryOperation::EndOfTypeCompareSpecial);
   (*table)[QueryOperation::TCSUndefined].setMethod(&Interval::queryCompareSpecial);
   (*table)[QueryOperation::TCSGuard].setMethod(&Interval::queryGuardAll);
   (*table)[QueryOperation::TCSNatural].setMethod(&Interval::queryCompareSpecial);
   (*table)[QueryOperation::TCSSigned].setMethod(&Interval::queryCompareSpecialSigned);
   (*table)[QueryOperation::TCSUnsigned].setMethod(&Interval::queryCompareSpecialUnsigned);
   (*table)[QueryOperation::TCSBitDomain].setMethod(&Interval::queryDispatchOnMin);
   
   table = &elementAt(VirtualQueryOperation::TSimplification);
   table->setSize(QueryOperation::EndOfTypeSimplification);
   (*table)[QueryOperation::TSUndefined].setMethod(&Interval::querySimplification);
   (*table)[QueryOperation::TSConstant].setMethod(&Top::queryFailSimplification);
   (*table)[QueryOperation::TSInterval].setMethod(&Interval::queryIdentity);
   (*table)[QueryOperation::TSConstantDisjunction].setMethod(&Interval::querySimplificationAsConstantDisjunction);

   table = &elementAt(VirtualQueryOperation::TDomain);
   table->setSize(QueryOperation::EndOfTypeDomain);

#define DefineInitQueryMethodTable(TypeOperation)                              \
   (*table)[QueryOperation::TD##TypeOperation].setMethod(&Interval::queryDispatchOnMin);
#define DefineInitQueryOMethodTable(TypeOperation, TypeMethod)                              \
   (*table)[QueryOperation::TD##TypeOperation].setMethod(&Interval::queryDispatchOnMin);
   
   DefineInitQueryMethodTable(Disjunction)
   DefineInitQueryMethodTable(MultiBitConstant)
   DefineInitQueryMethodTable(MultiFloatConstant)
   DefineInitQueryMethodTable(Interval)
   DefineInitQueryMethodTable(BooleanDisjunction)
   DefineInitQueryMethodTable(Top)
   DefineInitQueryMethodTable(False)
   DefineInitQueryMethodTable(True)
   DefineInitQueryMethodTable(IntForShift)
   DefineInitQueryMethodTable(Constant)
   DefineInitQueryMethodTable(Zero)
   DefineInitQueryMethodTable(MinusOne)
   DefineInitQueryMethodTable(One)
   DefineInitQueryOMethodTable(Min, MinUnsigned)
   DefineInitQueryOMethodTable(Max, MaxUnsigned)

#undef DefineInitQueryMethodTable
#undef DefineInitQueryOMethodTable

   table = &elementAt(VirtualQueryOperation::TDuplication);
   table->setSize(3);
   (*table)[QueryOperation::TCDClone].setMethod(&Interval::queryCopy);
   (*table)[QueryOperation::TCDSpecialize].setMethod(&Interval::querySpecialize);

   table = &elementAt(VirtualQueryOperation::TExtern);
   table->setSize(QueryOperation::EndOfTypeExtern);
   (*table)[QueryOperation::TESigned].setMethod(&Interval::queryDispatchOnMin);
#include "StandardClasses/DefineNew.h"
}
#undef DefineInitQuery

Interval::QueryTable Interval::mqtMethodQueryTable;

#define DefineInitApplyTo(TypeOperation)                                                        \
   elementAt(Operation::T##TypeOperation).setMethod(&Interval::applyTo##TypeOperation);
#define DefineInitApplyToConstant(TypeOperation)                                                \
   elementAt(Operation::T##TypeOperation).setMethod(&Interval::applyTo##TypeOperation##Constant);

Interval::MethodApplyToTable::MethodApplyToTable() {
#include "StandardClasses/UndefineNew.h"
   clear();
   DefineInitApplyToConstant(CompareLess)
   DefineInitApplyToConstant(CompareLessOrEqual)
   DefineInitApplyToConstant(CompareEqual)
   DefineInitApplyToConstant(CompareDifferent)
   DefineInitApplyToConstant(CompareGreaterOrEqual)
   DefineInitApplyToConstant(CompareGreater)
   DefineInitApplyToConstant(MinAssign)
   DefineInitApplyToConstant(MaxAssign)

   DefineInitApplyToConstant(PlusAssign)
   DefineInitApplyToConstant(MinusAssign)
   DefineInitApplyToConstant(TimesAssign)
   DefineInitApplyTo(DivideAssign)

   DefineInitApplyTo(ModuloAssign)
   DefineInitApplyToConstant(BitOrAssign)
   DefineInitApplyToConstant(BitAndAssign)
   DefineInitApplyToConstant(BitExclusiveOrAssign)
   DefineInitApplyToConstant(LeftShiftAssign)
   DefineInitApplyToConstant(RightShiftAssign)

   DefineInitApplyToConstant(LogicalAndAssign)
   DefineInitApplyToConstant(LogicalOrAssign)
#include "StandardClasses/DefineNew.h"
}
#undef DefineInitApplyTo
#undef DefineInitApplyToConstant

Interval::MethodApplyToTable Interval::mattMethodApplyToTable;

#define DefineTypeOperation Integer
#define DefineTypeObject Interval
#define DefineTypeInterval
#include "Analyzer/Scalar/Approximate/VirtualCall.incc"
#undef DefineTypeInterval
#undef DefineTypeObject
#undef DefineTypeOperation

}} // end of namespace Integer::Approximate

}} // end of namespace Analyzer::Scalar::Approximate


