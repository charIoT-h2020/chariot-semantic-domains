/////////////////////////////////
//
// Library   : Analyzer/Scalar
// Unit      : approximate scalar elements
// File      : VirtualElement.cpp
// Author    : Franck Vedrine
// Copyright : CEA LIST - 2019-2020 - all rights reserved
//
// Description :
//   Implementation of a hierarchy for approximate scalar elements.
//   The approximate elements can be used in abstract interpretation
//     based analyses, in pure formal analyses or in model checking.
//

#include "Analyzer/Scalar/Approximate/VirtualElement.h"
#include "Analyzer/Scalar/Approximate/MethodTable.h"
#include "Analyzer/Scalar/Approximate/Guard.h"
// #include "Analyzer/Scalar/Scalar.hpp"

namespace Analyzer {}

#include "Collection/Collection.template"

namespace Analyzer { namespace Scalar { namespace Approximate {

int VirtualElement::NBApplyCount = 0;
int VirtualElement::NBMergeCount = 0;
int VirtualElement::NBConstraintCount = 0;

AbstractElement::ZeroResult
VirtualElement::Methods::applyBoolean(const VirtualElement& source,
      const VirtualOperation& operation, const VirtualElement& arg) {
   AssumeCondition(operation.isConst())
   EvaluationEnvironment copyEnv(EPMayPropagateErrorStates);
   copyEnv.setFirstArgument(arg);
   assume(const_cast<VirtualElement&>(source).apply(operation, copyEnv));
   return copyEnv.getResult().queryZeroResult();
}

bool
VirtualElement::guardConstant(bool condition, PPVirtualElement thenElement, PPVirtualElement elseElement, EvaluationEnvironment& env)
{  env.presult() = condition ? thenElement : elseElement;
   if (!env.presult().isValid())
      env.setEmpty();
   return true;
}

bool
VirtualElement::guardApproximate(PPVirtualElement thenElement, PPVirtualElement elseElement, EvaluationEnvironment& env) {
   ZeroResult result = queryZeroResult();
   if (result.isTrue() || result.isFalse()) {
      env.presult() = result.isTrue() ? thenElement : elseElement;
      if (!env.presult().isValid())
         env.setEmpty();
   }
   else if (result.mayBeTrue() || result.mayBeFalse()) {
      env.mergeVerdictDegradate();
      if (thenElement.isValid() && elseElement.isValid()) {
         thenElement->mergeWith(*elseElement, env);
         if (!env.hasResult())
            env.presult() = thenElement;
      }
      else if (thenElement.isValid())
         env.presult() = thenElement;
      else if (elseElement.isValid())
         env.presult() = elseElement;
      else
         {  AssumeUncalled }
   };
   return true;
}

bool
VirtualElement::guardFormal(PPVirtualElement thenElement, PPVirtualElement elseElement, EvaluationEnvironment& env) {
   ZeroResult result = queryZeroResult();
   if (!thenElement.isValid() && !elseElement.isValid())
      env.setEmpty();
   else if (result.isTrue() || result.isFalse()) {
      env.presult() = result.isTrue() ? elseElement : thenElement;
      if (!env.presult().isValid())
         env.setEmpty();
   }
   else if (result.mayBeTrue() || result.mayBeFalse()) {
      AssumeCondition(thenElement->getSizeInBits() == elseElement->getSizeInBits())
      Guard* guard = new Guard(Guard::Init().setTable(thenElement.isValid() ? *thenElement : *elseElement)
         .setBitSize(thenElement->getSizeInBits()));
      env.presult().absorbElement(guard);
      guard->setCondition(*this, thenElement, elseElement);
   };
   return true;
}

bool
VirtualElement::constraint(const VirtualOperation& operation, const VirtualElement& resultElement,
      ConstraintEnvironment& env) {
   ++NBConstraintCount;
   int args = operation.getArgumentsNumber();
   bool result = false;
   if ((getApproxKind().isAtomic()) && ((args == 0)
         || ((env.getFirstArgument().getApproxKind().isAtomic()) && ((args == 1)
         || (env.getSecondArgument().getApproxKind().isAtomic()))))) {
      if (env.getLatticeCreation().isFormal()
            || ((getApproxKind().isConstant()) && ((args == 0)
            || ((env.getFirstArgument().getApproxKind().isConstant()) && ((args == 1)
            || (env.getSecondArgument().getApproxKind().isConstant())))))) {
         EvaluationEnvironment applyEnv = EvaluationEnvironment(EvaluationApplyParameters(env));
         PPVirtualElement thisElement(*this);
         int args = operation.getArgumentsNumber();
         if (args > 0) {
            applyEnv.setFirstArgument(env.getFirstArgument());
            if (args > 1)
               applyEnv.setSecondArgument(env.getSecondArgument());
         };
         thisElement->apply(operation, applyEnv);
         if (applyEnv.hasResult())
            thisElement = applyEnv.presult();
         EvaluationEnvironment intersectionEnv = EvaluationEnvironment(EvaluationEnvironment::Init(env));
         thisElement->intersectWith(resultElement, intersectionEnv);
         if (intersectionEnv.isEmpty()) {
            env.setEmpty();
            result = true;
         }
         else if ((getApproxKind().isConstant()) && ((args == 0)
               || ((env.getFirstArgument().getApproxKind().isConstant()) && ((args == 1)
               || (env.getSecondArgument().getApproxKind().isConstant())))))
            result = true;
      };
   };
   if ((args > 0) && (env.getFirstArgument().getApproxKind().isUnknown()
            || (env.getFirstArgument().getApproxKind().compareConstraint(getApproxKind()) == CRLess))) {
      PNT::PassPointer<VirtualOperation> garbage;
      bool isConstAssign = operation.isConstWithAssign();
      if (isConstAssign)
         garbage.setElement(operation);
      result = (isConstAssign)
         ? env.getFirstArgument().constraintTo(garbage->setAssign(),
               *this, resultElement, env)
         : env.getFirstArgument().constraintTo(operation, *this, resultElement, env);

      /* if (isConstAssign && !operation.isSimple())
         garbage.setElement(operation);
      result = (isConstAssign) ? env.getFirstArgument().constraintTo(operation.isSimple()
            ? (const VirtualOperation&) VirtualOperation(operation).setAssignCorrespondant()
            : garbage->setAssignCorrespondant(), *this, resultElement, env)
         : env.getFirstArgument().constraintTo(operation, *this, resultElement, env); */
      AssumeCondition(result)
   };
   return result;
}

bool
VirtualElement::queryIdentity(const VirtualElement&, const VirtualQueryOperation&, VirtualQueryOperation::Environment&)
   {  return true; }

bool
VirtualElement::queryBitNativeOperation(const VirtualElement&, const VirtualQueryOperation&, VirtualQueryOperation::Environment& env)
   {  ((ExternNativeOperationEnvironment&) env).setBit(); return true; }
   
bool
VirtualElement::queryIntegerNativeOperation(const VirtualElement&, const VirtualQueryOperation&, VirtualQueryOperation::Environment& env)
   {  ((ExternNativeOperationEnvironment&) env).setInteger(); return true; }

bool
VirtualElement::queryMultiBitNativeOperation(const VirtualElement&, const VirtualQueryOperation&, VirtualQueryOperation::Environment& env)
   {  ((ExternNativeOperationEnvironment&) env).setMultiBit(); return true; }

bool
VirtualElement::queryFloatingNativeOperation(const VirtualElement&, const VirtualQueryOperation&, VirtualQueryOperation::Environment& env)
   {  ((ExternNativeOperationEnvironment&) env).setFloating(); return true; }

bool
VirtualElement::queryRationalNativeOperation(const VirtualElement&, const VirtualQueryOperation&, VirtualQueryOperation::Environment& env)
   {  ((ExternNativeOperationEnvironment&) env).setRational(); return true; }

bool
VirtualElement::queryRealNativeOperation(const VirtualElement&, const VirtualQueryOperation&, VirtualQueryOperation::Environment& env)
   {  ((ExternNativeOperationEnvironment&) env).setReal(); return true; }

bool
VirtualElement::queryCopy(const VirtualElement& source, const VirtualQueryOperation&, VirtualQueryOperation::Environment& aenv) {
   AssumeCondition(dynamic_cast<const VirtualQueryOperation::DuplicationEnvironment*>(&aenv))
   VirtualQueryOperation::DuplicationEnvironment& env = (VirtualQueryOperation::DuplicationEnvironment&) aenv;
   env.absorbResult(source.createSCopy());
   return true;
}

bool
VirtualElement::querySpecialize(const VirtualElement&, const VirtualQueryOperation&, VirtualQueryOperation::Environment&)
   {  return true; }
   
namespace Details {

void
IntOperationElement::CompareSpecialEnvironment::unify() {
   if (!hasDomainField())
      return;
   int min=BEqualMinint, max=BEqualMaxint;
   while (!(queryDomainField() & min))
      min <<= 1;
   while (!(queryDomainField() & max))
      max >>= 1;
   while (min != max) {
      mergeDomainField(min);
      min <<= 1;
   };
}

void
RealOperationElement::CompareSpecialEnvironment::unify() {
   if (!hasOwnField())
      return;
   int min=BLessM1, max=BGreater1;
   while (!(queryOwnField() & min))
      min <<= 1;
   while (!(queryOwnField() & max))
      max >>= 1;
   while (min != max) {
      mergeOwnField(min);
      min <<= 1;
   };
}

void
IntOperationElement::BitDomainEnvironment::mergeWith(const VirtualQueryOperation::CompareSpecialEnvironment& asource) {
   const BitDomainEnvironment& source = (const BitDomainEnvironment&) asource;
   AssumeCondition((mbeFixed.getSize() == source.mbeFixed.getSize()))
   if ((uLastBitPos < 0) && (source.uLastBitPos >= 0)) {
      uLastBitPos = source.uLastBitPos;
      mbeFixed = source.mbeFixed;
      return;
   };
   if (source.uLastBitPos < 0)
      return;
   int lastMinPos = (uLastBitPos > source.uLastBitPos) ? uLastBitPos : source.uLastBitPos;
   int lastBytePos = lastMinPos / (8*sizeof(unsigned int));
   int lastPos = mbeFixed.getSize();
   int byteIndex = (lastPos + (8*sizeof(unsigned int)-1)) / (8*sizeof(unsigned int));
   if ((--byteIndex >= lastBytePos) && (mbeFixed[byteIndex] == source.mbeFixed[byteIndex])) {
      lastPos -= ((lastPos-1) % 8*sizeof(unsigned int))+1;
      while ((--byteIndex >= lastBytePos) && (mbeFixed[byteIndex] == source.mbeFixed[byteIndex]))
         lastPos -= 8*sizeof(unsigned int);
   };
   lastMinPos %= (8*sizeof(unsigned int));
   if ((byteIndex < lastBytePos) || ((byteIndex == lastBytePos) && (lastMinPos > 0)
            && ((mbeFixed[byteIndex] >> lastMinPos) == (source.mbeFixed[byteIndex] >> lastMinPos)))) {
      if (uLastBitPos < source.uLastBitPos) {
         uLastBitPos = source.uLastBitPos;
         mbeFixed = source.mbeFixed;
      };
   }
   else {
      lastPos += Numerics::DInteger::Access
            ::log_base_2(mbeFixed[byteIndex] ^ source.mbeFixed[byteIndex]);
      AssumeCondition(lastPos > lastMinPos)
      MultiBitElement intersect(mbeFixed.getSize());
      mbeFixed &= (const MultiBitElement&) (intersect.neg() <<= lastPos);
   };
}

void
IntOperationElement::BitDomainEnvironment::intersectWith(const VirtualQueryOperation::CompareSpecialEnvironment& asource) {
   const BitDomainEnvironment& source = (const BitDomainEnvironment&) asource;
   AssumeCondition(mbeFixed.getSize() == source.mbeFixed.getSize())
   if ((uLastBitPos < 0) || (source.uLastBitPos < 0)) {
      uLastBitPos = -1;
      mbeFixed.clear();
      return;
   };
   int lastMinPos = (uLastBitPos > source.uLastBitPos) ? uLastBitPos : source.uLastBitPos;
   int lastBytePos = lastMinPos / (8*sizeof(unsigned int));
   int lastPos = mbeFixed.getSize();
   int byteIndex = (lastPos + (8*sizeof(unsigned int)-1)) / (8*sizeof(unsigned int));
   if ((--byteIndex >= lastBytePos) && (mbeFixed[byteIndex] == source.mbeFixed[byteIndex])) {
      lastPos -= ((lastPos-1) % 8*sizeof(unsigned int))+1;
      while ((--byteIndex >= lastBytePos) && (mbeFixed[byteIndex] == source.mbeFixed[byteIndex]))
         lastPos -= 8*sizeof(unsigned int);
   };
   lastMinPos %= (8*sizeof(unsigned int));
   if ((byteIndex < lastBytePos) || ((byteIndex == lastBytePos) && (lastMinPos > 0)
            && ((mbeFixed[byteIndex] >> lastMinPos) == (source.mbeFixed[byteIndex] >> lastMinPos)))) {
      if (uLastBitPos > source.uLastBitPos) {
         uLastBitPos = source.uLastBitPos;
         mbeFixed = source.mbeFixed;
      };
   }
   else
      uLastBitPos = -1;
}

void
IntOperationElement::BoundDomainEnvironment::mergeWith(const VirtualQueryOperation::CompareSpecialEnvironment& asource) {
   const BoundDomainEnvironment& source = (const BoundDomainEnvironment&) asource;
   if (doesRequireLower()) {
      VirtualOperation minAssign = Methods::queryMinAssign(*ppveLower);
      ppveLower = Methods::applyAssign(ppveLower, minAssign, *source.ppveLower, EPMayStopErrorStates);
   };
   if (doesRequireUpper()) {
      VirtualOperation maxAssign = Methods::queryMaxAssign(*ppveUpper);
      ppveUpper = Methods::applyAssign(ppveUpper, maxAssign, *source.ppveUpper, EPMayStopErrorStates);
   };
}

void
IntOperationElement::BoundDomainEnvironment::intersectWith(const VirtualQueryOperation::CompareSpecialEnvironment& asource) {
   const BoundDomainEnvironment& source = (const BoundDomainEnvironment&) asource;
   if (doesRequireLower()) {
      VirtualOperation maxAssign = Methods::queryMaxAssign(*ppveLower);
      ppveLower = Methods::applyAssign(ppveLower, maxAssign, *source.ppveLower, EPMayStopErrorStates);
   };
   if (doesRequireUpper()) {
      VirtualOperation minAssign = Methods::queryMinAssign(*ppveUpper);
      ppveUpper = Methods::applyAssign(ppveUpper, minAssign, *source.ppveUpper, EPMayStopErrorStates);
   };
   if (ppveLower.isValid() && ppveUpper.isValid()) {
      ppveLower = Methods::constraintBoolean(ppveLower, Methods::queryCompareGreaterOrEqual(*ppveLower),
            ppveUpper, true, EPMayStopErrorStates);
      if (!ppveLower.isValid())
         ppveUpper.release();
   };
}

void
IntOperationElement::BoundDomainEnvironment::fill() {
   if (doesRequireLower())
      ppveLower = Methods::newMin(ioeReference);
   if (doesRequireUpper())
      ppveUpper = Methods::newMax(ioeReference);
}

IntOperationElement::CompareSpecialEnvironment&
IntOperationElement::CompareSpecialEnvironment::opposite() {
   unsigned int domain = queryDomainField();
   unsigned int inverseDomain = domain & BEqualMinint;
   unsigned int bitIndex = BEqualMinint << 1, inverseBitIndex = BEND;
   while (inverseBitIndex > BEqualMinint) {
      inverseBitIndex >>= 1;
      if (domain & bitIndex)
         inverseDomain |= inverseBitIndex;
      bitIndex <<= 1;
   };
   setDomainField(inverseDomain);
   return *this;
}

bool
IntOperationElement::constraintCastBit(VirtualElement& thisElement, const VirtualOperation& operation,
      const VirtualElement& resultElement, ConstraintEnvironment& env) {
   Scalar::Bit::CastMultiBitOperation inverseOperation;
   PPVirtualElement naturalElement
      = Methods::apply(resultElement, inverseOperation.setSize(thisElement.getSizeInBits()),
            EvaluationParameters(env).stopErrorStates());

   env.mergeVerdictExact();
   IntersectEnvironment intersectionEnv(env);
   naturalElement->intersectWithTo(thisElement, intersectionEnv);
   env.presult() = intersectionEnv.hasResult()
      ? intersectionEnv.presult() : (const PPAbstractElement&) naturalElement;
   return true;
}

bool
IntOperationElement::constraintCastInteger(VirtualElement& thisElement, const VirtualOperation& operation,
      const VirtualElement& resultElement, ConstraintEnvironment& env) {
   AssumeCondition(dynamic_cast<const Scalar::Integer::Operation*>(&operation))
   InverseCastOperationEnvironment inverseCastEnvironment((const Scalar::Integer::Operation&) operation);
   assume(thisElement.query(QueryOperation().setInverseCast(), inverseCastEnvironment));
   
   PPVirtualElement naturalElement
      = Methods::apply(resultElement, inverseCastEnvironment.result(),
            EvaluationParameters(env).stopErrorStates());

   int diffSize = thisElement.getSizeInBits() - resultElement.getSizeInBits();
   EvaluationEnvironment applyEnv = EvaluationEnvironment(EvaluationParameters(env));
   thisElement.apply(operation, applyEnv);
   if (env.doesStopErrorStates() || (diffSize <= 0) || !applyEnv.hasEvaluationError()) {
      env.mergeVerdictExact();
      IntersectEnvironment intersectionEnv(env);
      naturalElement->intersectWithTo(thisElement, intersectionEnv);
      env.presult() = intersectionEnv.hasResult()
         ? intersectionEnv.presult() : (const PPAbstractElement&) naturalElement;
      return true;
   };
   env.mergeVerdictDegradate();

   EvaluationParameters evaluationParams(env);

   PPVirtualElement sureOverflowPart = Methods::newTop(thisElement),
      oneOverflowPart = Methods::newOne(thisElement),
      shift = Methods::newIntForShift(thisElement, resultElement.getSizeInBits()+1);
   sureOverflowPart = Methods::applyAssign(sureOverflowPart,
         Methods::queryLeftShiftAssign(thisElement), *shift, evaluationParams);
   oneOverflowPart = Methods::applyAssign(oneOverflowPart,
         Methods::queryLeftShiftAssign(thisElement), *shift, evaluationParams);
   sureOverflowPart = Methods::constraintBoolean(sureOverflowPart, Methods::queryCompareGreaterOrEqual(thisElement),
         oneOverflowPart, true, evaluationParams);
   sureOverflowPart = Methods::applyAssign(sureOverflowPart, Methods::queryBitOrAssign(thisElement),
         *naturalElement, evaluationParams);
   
   PNT::TPassPointer<VirtualDisjunction, PPAbstractElement> disjunction
      = Methods::newDisjunction(thisElement);
   disjunction->add(sureOverflowPart, env.getInformationKind());
   disjunction->add(naturalElement, env.getInformationKind());

   IntersectEnvironment intersectionEnv(env);
   thisElement.intersectWith(*disjunction, intersectionEnv);
   return true;
}

template <class TypeDerived>
bool
TIntAndRealElement<TypeDerived>::constraintMinAssign(VirtualElement& thisElement, const VirtualOperation&,
      VirtualElement& sourceElement, const VirtualElement& resultElement, ConstraintEnvironment& env) {
   // min(a,b) = c -> a >= c, b >= c and a > c => b = c and b > c => a = c
   env.mergeVerdictExact();

   {  ConstraintEnvironment thisEnv = ConstraintEnvironment(EvaluationParameters(env));
      thisEnv.setFirstArgument(resultElement);
      assume(thisElement.constraint(TypeDerived::Methods::queryCompareGreaterOrEqual(thisElement),
         *TypeDerived::Methods::newTrue(thisElement), thisEnv));
      env.mergeWithIntersection(thisEnv, Argument().setResult());
      if (env.isEmpty())
         return true;
   };
   
   if (VirtualElement::Methods::applyBoolean(env.hasResult() ? env.getResult() : thisElement,
            TypeDerived::Methods::queryCompareGreater(thisElement), resultElement)) {
      IntersectEnvironment intersectionEnv(env, Argument().setFst());
      assume(sourceElement.intersectWith(resultElement, intersectionEnv));
   }
   else {
      ConstraintEnvironment sourceEnv = ConstraintEnvironment(EvaluationParameters(env));
      sourceEnv.setFirstArgument(resultElement);
      assume(sourceElement.constraint(TypeDerived::Methods::queryCompareGreaterOrEqual(sourceElement),
            *TypeDerived::Methods::newTrue(sourceElement), sourceEnv));
      env.mergeWithIntersection(sourceEnv, Argument().setFst());
      if (env.isEmpty())
         return true;

      if (VirtualElement::Methods::applyBoolean(env.getFirstArgument(),
            TypeDerived::Methods::queryCompareGreater(thisElement), resultElement)) {
         IntersectEnvironment intersectionEnv(env, Argument().setResult());
         assume((env.hasResult() ? env.getResult()
                                 : thisElement).intersectWith(resultElement, intersectionEnv));
      }
      else if (!VirtualElement::Methods::applyBoolean(env.hasResult() ? env.getResult() : thisElement,
               TypeDerived::Methods::queryCompareEqual(thisElement), resultElement)
            || !VirtualElement::Methods::applyBoolean(env.getFirstArgument(),
               TypeDerived::Methods::queryCompareEqual(thisElement), resultElement))
         env.mergeVerdictDegradate();
   };

   return true;
}

template <class TypeDerived>
bool
TIntAndRealElement<TypeDerived>::constraintMaxAssign(VirtualElement& thisElement, const VirtualOperation&,
      VirtualElement& sourceElement, const VirtualElement& resultElement, ConstraintEnvironment& env) {
   // max(a,b) = c -> a <= c, b <= c and a < c => b = c and b < c => a = c
   env.mergeVerdictExact();

   {  ConstraintEnvironment thisEnv = ConstraintEnvironment(EvaluationParameters(env));
      thisEnv.setFirstArgument(resultElement);
      assume(thisElement.constraint(TypeDerived::Methods::queryCompareLessOrEqual(thisElement),
         *TypeDerived::Methods::newTrue(thisElement), thisEnv));
      env.mergeWithIntersection(thisEnv, Argument().setResult());
      if (env.isEmpty())
         return true;
   }
   
   if (VirtualElement::Methods::applyBoolean(env.hasResult() ? env.getResult() : thisElement,
         TypeDerived::Methods::queryCompareLess(thisElement), resultElement)) {
      IntersectEnvironment intersectionEnv(env, Argument().setFst());
      assume(env.getFirstArgument().intersectWith(resultElement, intersectionEnv));
   }
   else {
      ConstraintEnvironment sourceEnv = ConstraintEnvironment(EvaluationParameters(env));
      sourceEnv.setFirstArgument(resultElement);
      assume(sourceElement.constraint(TypeDerived::Methods::queryCompareLessOrEqual(sourceElement),
            *TypeDerived::Methods::newTrue(sourceElement), sourceEnv));
      env.mergeWithIntersection(sourceEnv, Argument().setFst());
      if (env.isEmpty())
         return true;

      if (VirtualElement::Methods::applyBoolean(env.getFirstArgument(),
            TypeDerived::Methods::queryCompareLess(thisElement), resultElement)) {
         IntersectEnvironment intersectionEnv(env, Argument().setResult());
         assume((env.hasResult() ? env.getResult()
                                 : thisElement).intersectWith(resultElement, intersectionEnv));
      }
      else if (!TypeDerived::Methods::applyBoolean(env.hasResult() ? env.getResult() : thisElement,
               TypeDerived::Methods::queryCompareEqual(thisElement), resultElement)
            || !VirtualElement::Methods::applyBoolean(env.getFirstArgument(),
               TypeDerived::Methods::queryCompareEqual(thisElement), resultElement))
         env.mergeVerdictDegradate();
   };

   return true;
}

bool
IntOperationElement::constraintPrevAssign(VirtualElement& thisElement, const VirtualOperation&,
      const VirtualElement& resultElement, ConstraintEnvironment& env) {
   env.mergeVerdict(EvaluationVerdict().setPrecise());
   PPVirtualElement arg = Methods::applyAssign(PPVirtualElement(resultElement),
         Methods::queryNextAssign(thisElement), EvaluationApplyParameters(env));
   IntersectEnvironment intersectionEnv(env);
   thisElement.intersectWith(*arg, intersectionEnv);
   return true;
}

bool
IntOperationElement::constraintNextAssign(VirtualElement& thisElement, const VirtualOperation&,
      const VirtualElement& resultElement, ConstraintEnvironment& env) {
   env.mergeVerdict(EvaluationVerdict().setPrecise());
   PPVirtualElement arg = Methods::applyAssign(PPVirtualElement(resultElement),
         Methods::queryPrevAssign(thisElement), EvaluationApplyParameters(env));
   IntersectEnvironment intersectionEnv(env);
   thisElement.intersectWith(*arg, intersectionEnv);
   return true;
}

bool
IntOperationElement::constraintPlusAssign(VirtualElement& thisElement, const VirtualOperation& operation,
      VirtualElement& sourceElement, const VirtualElement& resultElement, ConstraintEnvironment& env) {
   // a+b = x -> a = x-b and b = x-a
   env.mergeVerdictExact();
   PPVirtualElement fstArg = Methods::apply(resultElement,
         *Methods::queryMinus(thisElement, operation), sourceElement, EvaluationApplyParameters(env));
   PPVirtualElement sndArg = Methods::apply(resultElement,
         *Methods::queryMinus(thisElement, operation), thisElement, EvaluationApplyParameters(env));
   IntersectEnvironment thisIntersectionEnv(env, Argument().setResult()),
                        sourceIntersectionEnv(env, Argument().setFst());
   thisElement.intersectWith(*fstArg, thisIntersectionEnv);
   sourceElement.intersectWith(*sndArg, sourceIntersectionEnv);
   return true;
}

bool
IntOperationElement::constraintMinusAssign(VirtualElement& thisElement, const VirtualOperation& operation,
      VirtualElement& sourceElement, const VirtualElement& resultElement, ConstraintEnvironment& env) {
   // a-b = x -> a = b+x and b = a-x
   env.mergeVerdictExact();
   PPVirtualElement fstArg = Methods::apply(resultElement,
         *Methods::queryPlus(thisElement, operation), sourceElement, EvaluationApplyParameters(env));
   PPVirtualElement sndArg = Methods::apply(thisElement,
         *Methods::queryMinus(thisElement, operation), resultElement, EvaluationApplyParameters(env));
   IntersectEnvironment thisIntersectionEnv(env, Argument().setResult()),
                        sourceIntersectionEnv(env, Argument().setFst());
   thisElement.intersectWith(*fstArg, thisIntersectionEnv);
   sourceElement.intersectWith(*sndArg, sourceIntersectionEnv);
   return true;
}

bool
IntOperationElement::constraintTimesAssign(VirtualElement& thisElement, const VirtualOperation& operation,
      VirtualElement& sourceElement, const VirtualElement& resultElement, ConstraintEnvironment& env) {
   env.mergeVerdictExact();
   if (env.hasEvaluationError() || (env.getInformationKind().isSure()))
      // We could try some intervals for under-approximation
      return true;

   QueryOperation::CompareSpecialEnvironment
      thisCompare = Methods::compareSpecial(thisElement),
      resultCompare = Methods::compareSpecial(resultElement),
      sourceCompare = Methods::compareSpecial(sourceElement);
   if (resultCompare.mayBeZero() && (thisCompare.mayBeZero() || sourceCompare.mayBeZero()))
      return true; // dynamic partitioning could give more information
   PPVirtualElement
      intersectFstArg = Methods::apply(resultElement, *Methods::queryDivide(thisElement, operation),
            sourceElement, EvaluationParameters().stopErrorStates()),
      intersectSndArg = Methods::apply(resultElement, *Methods::queryDivide(thisElement, operation),
            thisElement, EvaluationParameters().stopErrorStates());
   if (env.getInformationKind().isExact()) {
      PNT::TPassPointer<VirtualDisjunction, PPAbstractElement> disjunctionFstArg = Methods::newDisjunction(thisElement);
      disjunctionFstArg->addMay(intersectFstArg);
      intersectFstArg = disjunctionFstArg;

      PNT::TPassPointer<VirtualDisjunction, PPAbstractElement> disjunctionSndArg = Methods::newDisjunction(thisElement);
      disjunctionSndArg->addMay(intersectSndArg);
      intersectSndArg = disjunctionSndArg;
   };
   IntersectEnvironment
      thisIntersectionEnv(env, Argument().setResult()), sourceIntersectionEnv(env, Argument().setFst());
   thisElement.intersectWith(*intersectFstArg, thisIntersectionEnv);
   sourceElement.intersectWith(*intersectSndArg, sourceIntersectionEnv);
   return true;
}

bool
IntOperationElement::constraintDivideAssign(VirtualElement& thisElement, const VirtualOperation& operation,
      VirtualElement& sourceElement, const VirtualElement& resultElement, ConstraintEnvironment& env) {
   // x/y = z;
   env.mergeVerdictDegradate();
   const EvaluationParameters natural = EvaluationApplyParameters(env);
   PPVirtualElement thisConstraint, sourceConstraint;
   if (env.doesPropagateErrorStates() && env.isOverflow()) {
      // Division by -1 -> we add Minint as may
      thisConstraint = Methods::newMin(thisElement);
      sourceConstraint = Methods::newMinusOne(thisElement);
      thisConstraint = Methods::intersect(thisConstraint, thisElement, natural);
      sourceConstraint = Methods::intersect(sourceConstraint, sourceElement, natural);
   };

   QueryOperation::CompareSpecialEnvironment
      thisCompare = Methods::compareSpecial(thisElement),
      resultCompare = Methods::compareSpecial(resultElement),
      sourceCompare = Methods::compareSpecial(sourceElement);

   if (resultCompare.mayBeZero() && thisCompare.mayBeZero())
      return true; // dynamic partitioning could give more information

   if (resultCompare.mayBeZero()) { // thisCompare.isNotZero() : a/b == 0
      PPVirtualElement sourcePositive, sourceNegative, thisPositive, thisNegative;
      EvaluationParameters mayStopErrorStates(EPMayStopErrorStates);
      mayStopErrorStates.setApproximatePartFrom(env);
      if (thisCompare.mayBeGreaterZero()) { // b > a
         thisPositive.setElement(thisElement);
         if (!thisCompare.isGreaterZero())
            thisPositive = Methods::constraint(thisPositive, Methods::queryCompareGreaterOrEqual(thisElement),
               *Methods::newOne(thisElement), *Methods::newTrue(thisElement), mayStopErrorStates);
         sourcePositive = Methods::constraint(sourceElement, Methods::queryCompareGreater(thisElement),
               thisPositive, *Methods::newTrue(thisElement), mayStopErrorStates);
      };
      if (thisCompare.mayBeLessZero()) { // b < a
         thisNegative.setElement(thisElement);
         if (!thisCompare.isLessZero()) {
            PPVirtualElement minusOne = Methods::newMinusOne(thisElement);
            thisNegative = Methods::constraint(thisNegative, Methods::queryCompareLessOrEqual(thisElement),
               minusOne, *Methods::newTrue(thisElement), mayStopErrorStates);
         };
         sourceNegative = Methods::constraint(sourceElement, Methods::queryCompareLess(thisElement),
               thisNegative, *Methods::newTrue(thisElement), mayStopErrorStates);
      };

      thisConstraint = Methods::merge(thisConstraint, thisPositive, natural);
      thisConstraint = Methods::merge(thisConstraint, thisNegative, natural);
      sourceConstraint = Methods::merge(sourceConstraint, sourcePositive, natural);
      sourceConstraint = Methods::merge(sourceConstraint, sourceNegative, natural);
   };

   VirtualElement *sourcePositive=nullptr, *sourceNegative=nullptr, *thisPositive=nullptr,
      *thisNegative=nullptr;
   const VirtualElement *resultPositive=nullptr, *resultNegative=nullptr;
   PPVirtualElement ssourcePositive, ssourceNegative, sthisPositive, sthisNegative,
      sresultPositive, sresultNegative;

   if (sourceCompare.isGreaterZero())
      sourcePositive = &sourceElement;
   else if (sourceCompare.isLessZero())
      sourceNegative = &sourceElement;
   else {
      EvaluationParameters mayStopErrorStates(EPMayStopErrorStates);
      mayStopErrorStates.setApproximatePartFrom(env);
      ssourcePositive = Methods::constraint(sourceElement, Methods::queryCompareGreaterOrEqual(thisElement),
         *Methods::newOne(thisElement), *Methods::newTrue(thisElement), mayStopErrorStates),
      ssourceNegative = Methods::constraint(sourceElement, Methods::queryCompareLess(thisElement),
         *Methods::newMinusOne(thisElement), *Methods::newTrue(thisElement), mayStopErrorStates),
      sourcePositive = ssourcePositive.key();
      sourceNegative = ssourceNegative.key();
   };

   if (resultCompare.isGreaterZero())
      resultPositive = &resultElement;
   else if (resultCompare.isLessZero())
      resultNegative = &resultElement;
   else {
      EvaluationParameters mayStopErrorStates(EPMayStopErrorStates);
      mayStopErrorStates.setApproximatePartFrom(env);
      sresultPositive = Methods::constraint(resultElement, Methods::queryCompareGreaterOrEqual(thisElement),
         *Methods::newOne(thisElement), *Methods::newTrue(thisElement), mayStopErrorStates),
      sresultNegative = Methods::constraint(resultElement, Methods::queryCompareLess(thisElement),
         *Methods::newMinusOne(thisElement), *Methods::newTrue(thisElement), mayStopErrorStates),
      resultPositive = sresultPositive.key();
      resultNegative = sresultNegative.key();
   };

   if (thisCompare.isGreaterZero())
      thisPositive = &thisElement;
   else if (thisCompare.isLessZero())
      thisNegative = &thisElement;
   else {
      EvaluationParameters mayStopErrorStates(EPMayStopErrorStates);
      mayStopErrorStates.setApproximatePartFrom(env);
      sthisPositive = Methods::constraint(thisElement, Methods::queryCompareGreaterOrEqual(thisElement),
         *Methods::newOne(thisElement), *Methods::newTrue(thisElement), mayStopErrorStates),
      sthisNegative = Methods::constraint(thisElement, Methods::queryCompareLess(thisElement),
         *Methods::newMinusOne(thisElement), *Methods::newTrue(thisElement), mayStopErrorStates),
      thisPositive = sthisPositive.key();
      thisNegative = sthisNegative.key();
   };

   if (thisPositive && sourcePositive && resultPositive) { // x,y,z > 0
      // z <= x/y < z+1 => x in [y*z, y*(z+1)-1]
      EvaluationParameters mayStopErrorStates(EPMayStopErrorStates);
      mayStopErrorStates.setApproximatePartFrom(env);
      PPVirtualElement
         thisMin = Methods::apply(*sourcePositive, *Methods::queryTimes(thisElement, operation),
            *resultPositive, mayStopErrorStates),
         thisMax = Methods::applyAssign(
            Methods::applyAssign(
               Methods::applyAssign(PPVirtualElement(*resultPositive),
                  Methods::queryNextAssign(thisElement), mayStopErrorStates), // z+1
               Methods::queryTimesAssign(thisElement),
               *sourcePositive, mayStopErrorStates),
            Methods::queryPrevAssign(thisElement), mayStopErrorStates); // y*(z+1)-1

      PPVirtualElement thisPositiveConstraint = Methods::constraint(PPVirtualElement(thisElement),
            Methods::queryCompareGreaterOrEqual(thisElement), thisMin, *Methods::newTrue(thisElement), mayStopErrorStates);
      thisPositiveConstraint = Methods::constraint(thisPositiveConstraint,
            Methods::queryCompareLessOrEqual(thisElement), thisMax, *Methods::newTrue(thisElement), mayStopErrorStates);
      thisConstraint = Methods::merge(thisConstraint, thisPositiveConstraint, natural);

      // y in [(x+1)/(z+1), x/z]
      PPVirtualElement
         sourceMax = Methods::apply(*thisPositive, *Methods::queryDivide(thisElement, operation), *resultPositive, mayStopErrorStates),
         sourceMin = Methods::applyAssign(Methods::applyAssign(PPVirtualElement(*thisPositive),
                  Methods::queryNextAssign(thisElement), mayStopErrorStates), // x+1
            Methods::queryDivideAssign(thisElement),
            *Methods::applyAssign(PPVirtualElement(*resultPositive),
               Methods::queryNextAssign(thisElement), mayStopErrorStates), mayStopErrorStates); // (x+1)/(z+1)

      PPVirtualElement sourcePositiveConstraint = Methods::constraint(PPVirtualElement(sourceElement),
            Methods::queryCompareGreaterOrEqual(thisElement), sourceMin, *Methods::newTrue(thisElement), mayStopErrorStates);
      sourcePositiveConstraint = Methods::constraint(sourcePositiveConstraint,
            Methods::queryCompareLessOrEqual(thisElement), sourceMax, *Methods::newTrue(thisElement), mayStopErrorStates);
      sourceConstraint = Methods::merge(sourceConstraint, sourcePositiveConstraint, natural);
   };

   if (thisPositive && sourceNegative && resultNegative) { // x >= 0 and y,z < 0
      EvaluationParameters mayStopErrorStates(EPMayStopErrorStates);
      mayStopErrorStates.setApproximatePartFrom(env);
      // z-1 < x/y <= z => x in [y*z, y*(z-1)-1]
      PPVirtualElement
         thisMin = Methods::apply(*sourceNegative,
            *Methods::queryTimes(thisElement, operation),
            *resultNegative, mayStopErrorStates),
         thisMax = Methods::applyAssign(
            Methods::applyAssign(
               Methods::applyAssign(PPVirtualElement(*resultNegative),
                  Methods::queryPrevAssign(thisElement), mayStopErrorStates), // z-1
               Methods::queryTimesAssign(thisElement),
               *sourceNegative, mayStopErrorStates),
            Methods::queryPrevAssign(thisElement), mayStopErrorStates); // y*(z-1)-1

      PPVirtualElement thisPositiveConstraint = Methods::constraint(PPVirtualElement(thisElement),
            Methods::queryCompareGreaterOrEqual(thisElement), thisMin, *Methods::newTrue(thisElement), mayStopErrorStates);
      thisPositiveConstraint = Methods::constraint(thisPositiveConstraint,
            Methods::queryCompareLessOrEqual(thisElement), thisMax, *Methods::newTrue(thisElement), mayStopErrorStates);
      thisConstraint = Methods::merge(thisConstraint, thisPositiveConstraint, natural);

      // y in [(x+1)/(z-1), x/z]
      PPVirtualElement
         sourceMax = Methods::apply(*thisPositive,
            *Methods::queryDivide(thisElement, operation),
            *resultNegative, mayStopErrorStates),
         sourceMin = Methods::applyAssign(
            Methods::applyAssign(PPVirtualElement(*thisPositive),
               Methods::queryNextAssign(thisElement), mayStopErrorStates), // x+1
            Methods::queryDivideAssign(thisElement),
            *Methods::applyAssign(PPVirtualElement(*resultNegative),
               Methods::queryPrevAssign(thisElement), mayStopErrorStates), mayStopErrorStates); // (x+1)/(z-1)

      PPVirtualElement sourceNegativeConstraint = Methods::constraint(PPVirtualElement(sourceElement),
            Methods::queryCompareGreaterOrEqual(thisElement), sourceMin, *Methods::newTrue(thisElement), mayStopErrorStates);
      sourceNegativeConstraint = Methods::constraint(sourceNegativeConstraint,
            Methods::queryCompareLessOrEqual(thisElement), sourceMax, *Methods::newTrue(thisElement), mayStopErrorStates);
      sourceConstraint = Methods::merge(sourceConstraint, sourceNegativeConstraint, natural);
   };

   if (thisNegative && sourceNegative && resultPositive) { // x,y < 0 and z >= 0
      EvaluationParameters mayStopErrorStates(EPMayStopErrorStates);
      mayStopErrorStates.setApproximatePartFrom(env);
      // z <= x/y < z+1 => x in [y*(z+1)+1, y*z]
      PPVirtualElement
         thisMax = Methods::apply(*sourceNegative,
            *Methods::queryTimes(thisElement, operation),
            *resultPositive, mayStopErrorStates),
         thisMin = Methods::applyAssign(
            Methods::applyAssign(
               Methods::applyAssign(PPVirtualElement(*resultPositive),
                  Methods::queryNextAssign(thisElement), mayStopErrorStates), // z+1
               Methods::queryTimesAssign(thisElement),
               *sourceNegative, mayStopErrorStates),
            Methods::queryNextAssign(thisElement), mayStopErrorStates); // y*(z+1)+1

      PPVirtualElement thisNegativeConstraint = Methods::constraint(PPVirtualElement(thisElement),
            Methods::queryCompareGreaterOrEqual(thisElement), thisMin, *Methods::newTrue(thisElement), mayStopErrorStates);
      thisNegativeConstraint = Methods::constraint(thisNegativeConstraint,
            Methods::queryCompareLessOrEqual(thisElement), thisMax, *Methods::newTrue(thisElement), mayStopErrorStates);
      thisConstraint = Methods::merge(thisConstraint, thisNegativeConstraint, natural);

      // y in [x/z, (x-1)/(z+1)]
      PPVirtualElement
         sourceMin = Methods::apply(*thisNegative, *Methods::queryDivide(thisElement, operation),
            *resultPositive, mayStopErrorStates),
         sourceMax = Methods::applyAssign(
            Methods::applyAssign(PPVirtualElement(*thisNegative),
               Methods::queryPrevAssign(thisElement), mayStopErrorStates), // x-1
            Methods::queryDivideAssign(thisElement),
               *Methods::applyAssign(PPVirtualElement(*resultPositive),
                  Methods::queryNextAssign(thisElement), mayStopErrorStates),
            mayStopErrorStates); // (x-1)/(z+1)

      PPVirtualElement sourceNegativeConstraint = Methods::constraint(PPVirtualElement(sourceElement),
            Methods::queryCompareGreaterOrEqual(thisElement), sourceMin, *Methods::newTrue(thisElement), mayStopErrorStates);
      sourceNegativeConstraint = Methods::constraint(sourceNegativeConstraint,
            Methods::queryCompareLessOrEqual(thisElement), sourceMax, *Methods::newTrue(thisElement), mayStopErrorStates);
      sourceConstraint = Methods::merge(sourceConstraint, sourceNegativeConstraint, natural);
   };

   if (thisNegative && sourcePositive && resultNegative) { // y >= 0 and x,z < 0
      EvaluationParameters mayStopErrorStates(EPMayStopErrorStates);
      mayStopErrorStates.setApproximatePartFrom(env);
      // z-1 < x/y <= z => x in [y*(z-1)+1, y*z]
      PPVirtualElement
         thisMax = Methods::apply(*sourcePositive, *Methods::queryTimes(thisElement, operation), resultElement, mayStopErrorStates),
         thisMin = Methods::applyAssign(
            Methods::applyAssign(
               Methods::applyAssign(PPVirtualElement(resultElement),
                  Methods::queryNextAssign(thisElement), mayStopErrorStates), // z+1
               Methods::queryTimesAssign(thisElement),
               *sourcePositive, mayStopErrorStates),
            Methods::queryNextAssign(thisElement), mayStopErrorStates); // y*(z+1)+1

      PPVirtualElement thisNegativeConstraint = Methods::constraint(PPVirtualElement(thisElement),
            Methods::queryCompareGreaterOrEqual(thisElement), thisMin, *Methods::newTrue(thisElement), mayStopErrorStates);
      thisNegativeConstraint = Methods::constraint(thisNegativeConstraint,
            Methods::queryCompareLessOrEqual(thisElement), thisMax, *Methods::newTrue(thisElement), mayStopErrorStates);
      thisConstraint = Methods::merge(thisConstraint, thisNegativeConstraint, natural);

      // y in [(x-1)/(z-1), x/z]
      PPVirtualElement
         sourceMax = Methods::apply(*thisNegative, *Methods::queryDivide(thisElement, operation),
            *resultNegative, mayStopErrorStates),
         sourceMin = Methods::applyAssign(
            Methods::applyAssign(PPVirtualElement(*thisNegative),
               Methods::queryPrevAssign(thisElement), mayStopErrorStates), // x-1
            Methods::queryDivideAssign(thisElement),
            *Methods::applyAssign(PPVirtualElement(*resultNegative),
               Methods::queryPrevAssign(thisElement), mayStopErrorStates),
            mayStopErrorStates); // (x-1)/(z-1)

      PPVirtualElement sourcePositiveConstraint = Methods::constraint(PPVirtualElement(sourceElement),
            Methods::queryCompareGreaterOrEqual(thisElement), sourceMin, *Methods::newTrue(thisElement), mayStopErrorStates);
      if (sourcePositiveConstraint.isValid())
         sourcePositiveConstraint = Methods::constraint(sourcePositiveConstraint,
               Methods::queryCompareLessOrEqual(thisElement), sourceMax, *Methods::newTrue(thisElement), mayStopErrorStates);
      sourceConstraint = Methods::merge(sourceConstraint, sourcePositiveConstraint, natural);
   };

   if (!thisConstraint.isValid() || !sourceConstraint.isValid())
      env.setEmpty();
   else {
      IntersectEnvironment thisIntersectionEnv(env, Argument().setResult());
      IntersectEnvironment sourceIntersectionEnv(env, Argument().setFst());
      thisElement.intersectWith(*thisConstraint, thisIntersectionEnv);
      sourceElement.intersectWith(*sourceConstraint, sourceIntersectionEnv);
   };
   return true;
}

bool
IntOperationElement::constraintOppositeAssign(VirtualElement& thisElement, const VirtualOperation& operation,
      const VirtualElement& resultElement, ConstraintEnvironment& env) {
   env.mergeVerdictExact();
   IntersectEnvironment intersectionEnv(env);
   thisElement.intersectWith(*Methods::apply(resultElement,
         *Methods::queryOpposite(thisElement, operation), EvaluationApplyParameters(env)),
      intersectionEnv);
   return true;
}

bool
IntOperationElement::constraintModuloAssign(VirtualElement& thisElement, const VirtualOperation&,
      VirtualElement& sourceElement, const VirtualElement& resultElement, ConstraintEnvironment& env) {
   env.mergeVerdictDegradate();
   QueryOperation::CompareSpecialEnvironment
      thisCompare = Methods::compareSpecial(thisElement),
      resultCompare = Methods::compareSpecial(resultElement),
      sourceCompare = Methods::compareSpecial(sourceElement);
   if (thisCompare.isGreaterOrEqualZero() && sourceCompare.isGreaterOrEqualZero()
         && resultCompare.isGreaterOrEqualZero()) {
      EvaluationParameters mayStopErrorStates(EPMayStopErrorStates);
      mayStopErrorStates.setApproximatePartFrom(env);
      ConstraintEnvironment thisConstraintEnv(mayStopErrorStates);
      thisConstraintEnv.absorbFirstArgument(resultElement.createSCopy());
      thisElement.constraint(Methods::queryCompareGreaterOrEqual(thisElement), *Methods::newTrue(thisElement), thisConstraintEnv);
      env.mergeWithIntersection(thisConstraintEnv);

      PPVirtualElement result((VirtualElement*) thisConstraintEnv.extractFirstArgument(), PNT::Pointer::Init());
      PPVirtualElement source((VirtualElement*) env.extractFirstArgument(), PNT::Pointer::Init());
      PPVirtualElement constrainedSource = Methods::constraintBoolean(source, Methods::queryCompareGreater(thisElement),
         result, true, mayStopErrorStates);
      if (constrainedSource.isValid())
         env.absorbFirstArgument(constrainedSource.extractElement());
      else
         env.setEmpty();
   };
   return true;
}

bool
IntOperationElement::constraintBitOrAssign(VirtualElement& thisElement, const VirtualOperation&,
      VirtualElement& sourceElement, const VirtualElement& resultElement, ConstraintEnvironment& env) {
   env.mergeVerdictDegradate();
   QueryOperation::CompareSpecialEnvironment
      thisCompare = Methods::compareSpecial(thisElement),
      resultCompare = Methods::compareSpecial(resultElement),
      sourceCompare = Methods::compareSpecial(sourceElement);

   if (resultCompare.isGreaterOrEqualZero()) {
      PPVirtualElement thisElementConstraint(thisElement), sourceElementConstraint(thisElement);
      PPVirtualElement zero, result(resultElement);
      EvaluationParameters mayStopErrorStates(EPMayStopErrorStates);
      mayStopErrorStates.setApproximatePartFrom(env);
      if (thisCompare.mayBeLessZero())
         thisElementConstraint = Methods::constraintBoolean(thisElementConstraint, Methods::queryCompareGreaterOrEqual(thisElement),
            zero = Methods::newZero(thisElement), true, mayStopErrorStates);
      if (sourceCompare.mayBeLessZero())
         sourceElementConstraint = Methods::constraintBoolean(sourceElementConstraint, Methods::queryCompareGreaterOrEqual(thisElement),
            zero = Methods::newZero(thisElement), true, mayStopErrorStates);
      if (thisElementConstraint.isValid())
         thisElementConstraint = Methods::constraintBoolean(thisElementConstraint, Methods::queryCompareLessOrEqual(thisElement),
            result, true, mayStopErrorStates);
      if (sourceElementConstraint.isValid())
         sourceElementConstraint = Methods::constraintBoolean(sourceElementConstraint, Methods::queryCompareLessOrEqual(thisElement),
            result, true, mayStopErrorStates);

      if (thisElementConstraint.isValid() && sourceElementConstraint.isValid()) {
         env.presult() = thisElementConstraint;
         env.absorbFirstArgument(sourceElementConstraint.extractElement());
      }
      else
         env.setEmpty();
   }
   else if (resultCompare.isLessZero() && thisCompare.isLessZero() && sourceCompare.isLessZero()) {
      EvaluationParameters mayStopErrorStates(EPMayStopErrorStates);
      mayStopErrorStates.setApproximatePartFrom(env);
      ConstraintEnvironment thisConstraintEnv(mayStopErrorStates);
      thisConstraintEnv.absorbFirstArgument(resultElement.createSCopy());
      thisElement.constraint(Methods::queryCompareLessOrEqual(thisElement), *Methods::newTrue(thisElement), thisConstraintEnv);
      if (thisConstraintEnv.hasResult())
         env.presult() = thisConstraintEnv.presult();

      if (!thisConstraintEnv.isEmpty()) {
         PPVirtualElement source((VirtualElement*) env.extractFirstArgument(), PPVirtualElement::Init());
         source = Methods::constraint(source, Methods::queryCompareLessOrEqual(thisElement),
            resultElement, *Methods::newTrue(thisElement), mayStopErrorStates);
         if (source.isValid())
            env.absorbFirstArgument(source.extractElement());
         else
            env.setEmpty();
      };
   };

   return true;
}

bool
IntOperationElement::constraintBitAndAssign(VirtualElement& thisElement, const VirtualOperation&, VirtualElement& sourceElement,
      const VirtualElement& resultElement, ConstraintEnvironment& env) {
   env.mergeVerdictDegradate();
   QueryOperation::CompareSpecialEnvironment
      thisCompare = Methods::compareSpecial(thisElement),
      resultCompare = Methods::compareSpecial(resultElement),
      sourceCompare = Methods::compareSpecial(sourceElement);

   EvaluationParameters mayStopErrorStates(EPMayStopErrorStates);
   mayStopErrorStates.setApproximatePartFrom(env);
   if (resultCompare.isGreaterOrEqualZero() && sourceCompare.isGreaterOrEqualZero()
         && thisCompare.isGreaterOrEqualZero()) {
      ConstraintEnvironment thisConstraintEnv(mayStopErrorStates);
      thisConstraintEnv.absorbFirstArgument(resultElement.createSCopy());
      auto compareGreaterOrEqual = Methods::queryCompareGreaterOrEqual(thisElement);
      PPVirtualElement trueElement = Methods::newTrue(thisElement);
      thisElement.constraint(Methods::queryCompareGreaterOrEqual(thisElement),
            *trueElement, thisConstraintEnv);
      if (thisConstraintEnv.hasResult())
         env.presult() = thisConstraintEnv.presult();

      if (!thisConstraintEnv.isEmpty()) {
         PPVirtualElement source((VirtualElement*) env.extractFirstArgument(), PPVirtualElement::Init());
         source = Methods::constraint(source, compareGreaterOrEqual,
            resultElement, *trueElement, mayStopErrorStates);
         if (source.isValid())
            env.absorbFirstArgument(source.extractElement());
         else
            env.setEmpty();
      };
   }
   else if (resultCompare.isLessZero()) {
      PPVirtualElement thisElementConstraint(thisElement), sourceElementConstraint(thisElement);
      if (thisCompare.mayBeGreaterZero())
         thisElementConstraint = Methods::constraint(thisElementConstraint, Methods::queryCompareLess(thisElement),
            *Methods::newZero(thisElement), mayStopErrorStates);
      if (sourceCompare.mayBeGreaterZero())
         sourceElementConstraint = Methods::constraint(sourceElementConstraint, Methods::queryCompareLess(thisElement),
            *Methods::newZero(thisElement), mayStopErrorStates);
      thisElementConstraint = Methods::constraint(thisElementConstraint, Methods::queryCompareGreaterOrEqual(thisElement),
         resultElement, mayStopErrorStates);
      sourceElementConstraint = Methods::constraint(sourceElementConstraint, Methods::queryCompareGreaterOrEqual(thisElement),
         resultElement, mayStopErrorStates);

      if (thisElementConstraint.isValid() && sourceElementConstraint.isValid()) {
         env.presult() = thisElementConstraint;
         env.absorbFirstArgument(sourceElementConstraint.extractElement());
      }
      else
         env.setEmpty();
   };

   return true;
}

bool
IntOperationElement::constraintBitExclusiveOrAssign(VirtualElement& thisElement, const VirtualOperation& operation, VirtualElement& sourceElement,
      const VirtualElement& resultElement, ConstraintEnvironment& env) {
   env.mergeVerdictExact();
   PPVirtualElement intersectThis = Methods::apply(sourceElement, Methods::queryBitExclusiveOr(thisElement), 
         resultElement, EvaluationApplyParameters(env));
   PPVirtualElement intersectSource = Methods::apply(thisElement, Methods::queryBitExclusiveOr(thisElement), 
         resultElement, EvaluationApplyParameters(env));
   IntersectEnvironment sourceEnv(env, Argument().setFst());
   assume(sourceElement.intersectWith(*intersectSource, sourceEnv));
   IntersectEnvironment thisEnv(env, Argument().setResult());
   assume(thisElement.intersectWith(*intersectThis, thisEnv));
   return true;
}

bool
IntOperationElement::constraintBinaryNoPropagation(VirtualElement&, const VirtualOperation&, VirtualElement&,
      const VirtualElement&, ConstraintEnvironment& env) {
   env.mergeVerdictDegradate();
   return true;
}

bool
IntOperationElement::constraintBitNegateAssign(VirtualElement& thisElement, const VirtualOperation&, const VirtualElement& resultElement,
      ConstraintEnvironment& env) {
   env.mergeVerdictExact();
   PPVirtualElement thisElementConstraint = Methods::apply(resultElement, Methods::queryBitNegate(thisElement),
         EvaluationApplyParameters(env));
   IntersectEnvironment intersectionEnv(env);
   thisElement.intersectWith(*thisElementConstraint, intersectionEnv);
   return true;
}

bool
IntOperationElement::constraintRotateAssign(VirtualElement& thisElement, const VirtualOperation& operation,
      VirtualElement& sourceElement, const VirtualElement& resultElement, ConstraintEnvironment& env) {
   env.mergeVerdictDegradate();
   PPVirtualElement thisElementConstraint = Methods::applyAssign(PPVirtualElement(resultElement),
         Methods::queryInverseRotateAssignOperation(thisElement, operation), sourceElement, EvaluationApplyParameters(env));
   IntersectEnvironment intersectionEnv(env);
   thisElement.intersectWith(*thisElementConstraint, intersectionEnv);
   return true;
}

bool
IntOperationElement::constraintLogicalAndAssign(VirtualElement& thisElement, const VirtualOperation&,
      VirtualElement& sourceElement, const VirtualElement& resultElement, ConstraintEnvironment& env) {
   QueryOperation::CompareSpecialEnvironment resultCompare = Methods::compareSpecial(resultElement);

   if (resultCompare.isNotZero()) {
      PPVirtualElement argument((VirtualElement*) env.extractFirstArgument(), PPVirtualElement::Init());
      {  ConstraintTransmitEnvironment thisConstraintEnv(env, Methods::newZero(thisElement), Argument().setResult());
         thisElement.constraint(Methods::queryCompareDifferent(thisElement), *Methods::newTrue(thisElement), thisConstraintEnv);
      };
      {  ConstraintTransmitEnvironment sourceConstraintEnv(env, Methods::newZero(sourceElement), Argument().setFst());
         sourceElement.constraint(Methods::queryCompareDifferent(sourceElement), *Methods::newTrue(thisElement), sourceConstraintEnv);
      };
   }
   else if (resultCompare.isZero()) {
      QueryOperation::CompareSpecialEnvironment thisCompare = Methods::compareSpecial(thisElement),
         sourceCompare = Methods::compareSpecial(sourceElement);
      if (thisCompare.isNotZero()) {
         IntersectEnvironment intersectionEnv(env, Argument().setFst());
         sourceElement.intersectWith(*Methods::newZero(thisElement), intersectionEnv);
      }
      else if (sourceCompare.isNotZero()) {
         IntersectEnvironment intersectionEnv(env, Argument().setResult());
         thisElement.intersectWith(*Methods::newZero(thisElement), intersectionEnv);
      }
      else if (!thisCompare.isZero() && !sourceCompare.isZero())
         env.mergeVerdictDegradate();
   }
   else
      env.mergeVerdictDegradate();
   return true;
}

bool
IntOperationElement::constraintLogicalOrAssign(VirtualElement& thisElement, const VirtualOperation&, VirtualElement& sourceElement,
      const VirtualElement& resultElement, ConstraintEnvironment& env) {
   QueryOperation::CompareSpecialEnvironment resultCompare = Methods::compareSpecial(resultElement);

   if (resultCompare.isZero()) {
      IntersectEnvironment thisIntersectionEnv(env, Argument().setResult());
      thisElement.intersectWith(*Methods::newZero(thisElement), thisIntersectionEnv);

      IntersectEnvironment sourceIntersectionEnv(env, Argument().setFst());
      sourceElement.intersectWith(*Methods::newZero(sourceElement), sourceIntersectionEnv);
   }
   else if (resultCompare.isNotZero()) {
      QueryOperation::CompareSpecialEnvironment thisCompare = Methods::compareSpecial(thisElement),
         sourceCompare = Methods::compareSpecial(sourceElement);
      if (thisCompare.isZero()) {
         PPVirtualElement argument((VirtualElement*) env.extractFirstArgument(), PPVirtualElement::Init());
         ConstraintTransmitEnvironment
            sourceConstraintEnv(env, Methods::newZero(thisElement), Argument().setFst());
         sourceElement.constraint(Methods::queryCompareDifferent(thisElement), *Methods::newTrue(thisElement), sourceConstraintEnv);
      }
      else if (sourceCompare.isZero()) {
         PPVirtualElement argument((VirtualElement*) env.extractFirstArgument(), PPVirtualElement::Init());
         ConstraintTransmitEnvironment thisConstraintEnv(env, Methods::newZero(thisElement), Argument().setResult());
         thisElement.constraint(Methods::queryCompareDifferent(thisElement), *Methods::newTrue(thisElement), thisConstraintEnv);
      }
      else if (thisCompare.mayBeZero() && sourceCompare.mayBeZero())
         env.mergeVerdictDegradate();
   }
   else
      env.mergeVerdictDegradate();
   return true;
}

bool
IntOperationElement::constraintLogicalNegateAssign(VirtualElement& thisElement, const VirtualOperation&, const VirtualElement& resultElement,
      ConstraintEnvironment& env) {
   QueryOperation::CompareSpecialEnvironment resultCompare = Methods::compareSpecial(resultElement);
   if (resultCompare.isZero()) {
      ConstraintTransmitEnvironment thisConstraintEnv(env, Methods::newZero(thisElement), Argument().setResult());
      thisElement.constraint(Methods::queryCompareDifferent(thisElement), *Methods::newTrue(thisElement), thisConstraintEnv);
   }
   else if (resultCompare.isNotZero()) {
      IntersectEnvironment intersectionEnv(env, Argument().setResult());
      thisElement.intersectWith(*Methods::newZero(thisElement), intersectionEnv);
   }
   else
      env.mergeVerdictDegradate();
   return true;
}

PPVirtualElement
IntOperationElement::Methods::constraintBoolean(PPVirtualElement source, bool result,
      const EvaluationParameters& params) {
   EvaluationEnvironment env(params);
   
   if (result) {
      PNT::TPassPointer<VirtualDisjunction, PPAbstractElement> intersection = newDisjunction(*source);
      bool isUnsigned = !Methods::isSigned(*source); 
      if (!isUnsigned)
         intersection->add(newInterval(*source, VirtualIntegerInterval::Init().setInterval(newMin(*source), newMinusOne(*source)).setUnsigned(isUnsigned)),
               params.getInformationKind());
      intersection->add(newInterval(*source, VirtualIntegerInterval::Init().setInterval(newOne(*source), newMax(*source)).setUnsigned(isUnsigned)),
            params.getInformationKind());
      source->intersectWith(*intersection, env);
   }
   else
      source->intersectWith(*newZero(*source), env);

   return env.isEmpty() ? PPVirtualElement() : (env.hasResult()
         ? PPVirtualElement(env.presult()) : source);
}

PPVirtualElement
IntOperationElement::Methods::constraintBoolean(PPVirtualElement source, const VirtualOperation&
      compareOperation, PPVirtualElement& operand, bool result, const EvaluationParameters& params) {
   EvaluationEnvironment env(params);
   
   PPVirtualElement resultElement;
   if (result)
      resultElement = newTrue(*source);
   else
      resultElement = newFalse(*source);
   
   return constraint(source, compareOperation, operand, *resultElement, params);
}

bool
IntOperationElement::queryGuardAll(const VirtualElement& element, const VirtualQueryOperation&, VirtualQueryOperation::Environment& aenv) {
   AssumeCondition(dynamic_cast<const VirtualQueryOperation::GuardEnvironment*>(&aenv))
   VirtualQueryOperation::GuardEnvironment& env = (VirtualQueryOperation::GuardEnvironment&) aenv;
   env.presult() = Methods::newTrue(element);
   return true;
}

bool
RealOperationElement::queryGuardAll(const VirtualElement& element, const VirtualQueryOperation&, VirtualQueryOperation::Environment& aenv) {
   AssumeCondition(dynamic_cast<const VirtualQueryOperation::GuardEnvironment*>(&aenv))
   VirtualQueryOperation::GuardEnvironment& env = (VirtualQueryOperation::GuardEnvironment&) aenv;
   env.presult() = Methods::newTrue(element);
   return true;
}

template class TIntAndRealElement<IntOperationElement>;
template class TIntAndRealElement<RealOperationElement>;

} // end of namespace Details

EvaluationEnvironment::Init VirtualElement::EPMayStopErrorStates = EvaluationEnvironment::Init().setMay().setLowLevel().stopErrorStates();
EvaluationEnvironment::Init VirtualElement::EPMayPropagateErrorStates = EvaluationEnvironment::Init().setMay().setLowLevel().propagateErrorStates();
EvaluationEnvironment::Init VirtualElement::EPExactStopErrorStates = EvaluationEnvironment::Init().setExact().setLowLevel().stopErrorStates();
EvaluationEnvironment::Init VirtualElement::EPExactPropagateErrorStates = EvaluationEnvironment::Init().setExact().setLowLevel().propagateErrorStates();
EvaluationEnvironment::Init VirtualElement::EPSureStopErrorStates = EvaluationEnvironment::Init().setSure().setLowLevel().stopErrorStates();
EvaluationEnvironment::Init VirtualElement::EPSurePropagateErrorStates = EvaluationEnvironment::Init().setSure().setLowLevel().propagateErrorStates();

} // end of namespace Approximate

namespace Bit { namespace Approximate { namespace Details {

using namespace Scalar::Approximate::Details;

bool
BitElement::queryGuardAll(const VirtualElement& element, const VirtualQueryOperation&, VirtualQueryOperation::Environment& aenv) {
   AssumeCondition(dynamic_cast<const VirtualQueryOperation::GuardEnvironment*>(&aenv))
   VirtualQueryOperation::GuardEnvironment& env = (VirtualQueryOperation::GuardEnvironment&) aenv;
   env.presult() = Methods::newTrue(element);
   return true;
}

bool
BaseAlgorithms::queryInverseCastOperation(const VirtualElement&, const VirtualQueryOperation&, VirtualQueryOperation::Environment& aenv) {
   QueryOperation::InverseCastOperationEnvironment& env = (QueryOperation::InverseCastOperationEnvironment&) aenv;
   Scalar::Bit::Operation::Type type = ((const Scalar::Bit::Operation&) env.reference()).getType();
   if ((type >= Scalar::Bit::Operation::TCastChar) && (type <= Scalar::Bit::Operation::TCastUnsignedInt))
      env.resultAsInteger().setCastBit();
   else if (type == Scalar::Bit::Operation::TCastMultiBit)
      env.resultAsMultiBit().setCastBit();
   else
      { AssumeUncalled }
   return true;
}

}}} // end of namespace Bit::Approximate::Details

namespace MultiBit { namespace Approximate { namespace Details {

using namespace Scalar::Approximate::Details;

class BaseAlgorithms::ConstraintConstants : public BaseConstants {
  private:
   typedef MultiBit::Operation Operation;

  public:
   static const int NBUnaryConstraintMethods = Operation::EndOfUnary + 3; // BitNegate & LogicalNegate
   static int convertUnaryConstraintToOperationToIndex(Operation::Type type)
      {  return (type < Operation::EndOfUnary) ? type
            : ((type == Operation::TOppositeSignedAssign) ? (Operation::EndOfUnary)
            : ((type == Operation::TOppositeFloatAssign) ? (Operation::EndOfUnary+1)
            : ((type == Operation::TBitNegateAssign) ? (Operation::EndOfUnary+2) : -1)));
      }
   static const int NBBinaryConstraintMethods
      = Operation::EndOfType - Operation::EndOfCompare + Operation::StartOfCompare;
   static int convertBinaryConstraintOperationToIndex(Operation::Type type)
      {  return (type < Operation::EndOfExtension) ? (type == Operation::TConcat ? 0 : 1)
            : ((type < Operation::EndOfBinary) ? (2 + type - Operation::StartOfBinary)
            : (2 + type - Operation::EndOfCompare + Operation::EndOfBinary - Operation::StartOfBinary));
      }
   static const int NBConstraintCompareMethods
      = Scalar::MultiBit::Operation::EndOfCompare - Scalar::MultiBit::Operation::StartOfCompare;
   static int convertCompareConstraintOperationToIndex(Scalar::MultiBit::Operation::Type type)
      {  return type - Scalar::MultiBit::Operation::StartOfCompare; }
};

class BaseAlgorithms::ConstraintDomainTraits {
  public:
   typedef Approximate::VirtualElement Domain;
   typedef BaseAlgorithms::ConstraintConstants Constants;
   static const int NBConstantsIntersectToMethods = Constants::NBIntersectToMethods;
   static const int NBConstantsIntersectMethods = Constants::NBIntersectMethods;
   static const int NBApplyMethods = Constants::NBApplyMethods;
   static const int NBApplyToMethods = Constants::NBApplyToMethods;
   static const int NBConstraintCompareMethods = Constants::NBConstraintCompareMethods;
   static const int NBBinaryConstraintMethods = Constants::NBBinaryConstraintMethods;
   static const int NBUnaryConstraintMethods = Constants::NBUnaryConstraintMethods;

   typedef BaseAlgorithms::Operation Operation;
   typedef bool (VirtualElement::*PointerMergeIntersectToMethod)(VirtualElement&, EvaluationEnvironment&) const;
   typedef bool (VirtualElement::*PointerMergeIntersectMethod)(const VirtualElement&, EvaluationEnvironment&);
   typedef bool (VirtualElement::*PointerApplyMethod)(const VirtualOperation&, EvaluationEnvironment&);
   typedef bool (VirtualElement::*PointerApplyToMethod)(const VirtualOperation&, VirtualElement&, EvaluationEnvironment&) const;
   typedef bool (VirtualElement::*PointerConstraintCompareMethod)
      (const VirtualOperation&, VirtualElement&, ConstraintEnvironment::Argument, ConstraintEnvironment&);
   typedef bool (VirtualElement::*PointerConstraintBinaryMethod)
      (const VirtualOperation&, VirtualElement&, const VirtualElement&, ConstraintEnvironment::Argument, ConstraintEnvironment&);
   typedef bool (*PointerConstraintBinaryGenericFunction)
      (VirtualElement& element, const VirtualOperation&, VirtualElement&, const VirtualElement&, ConstraintEnvironment&);
   typedef bool (VirtualElement::*PointerConstraintBinaryArgMethod)
      (const VirtualOperation&, VirtualElement&, const VirtualElement&, ConstraintEnvironment&);
   typedef bool (VirtualElement::*PointerConstraintUnaryGenericMethod)
      (const VirtualOperation& operation, const VirtualElement&, ConstraintEnvironment&);
   typedef bool (*PointerConstraintUnaryGenericFunction)
      (VirtualElement&, const VirtualOperation& operation, const VirtualElement&, ConstraintEnvironment&);
   typedef bool (VirtualElement::*PointerQueryMethod)(const VirtualQueryOperation&, VirtualQueryOperation::Environment&) const;
};

class BaseAlgorithms::IGenericConstraintTable : public MethodConstraintTable<ConstraintDomainTraits> {
  public:
   IGenericConstraintTable();
};

#define DefineInitUnaryConstraint(TypeOperation) \
   mcutUnaryTable[Operation::T##TypeOperation].setFunction(&BaseAlgorithms::constraint##TypeOperation);

#define DefineInitUnaryIntConstraint(TypeOperation) \
   mcutUnaryTable[Operation::T##TypeOperation].setFunction(&Approximate::Details::IntOperationElement::constraint##TypeOperation);

#define DefineInitUnaryOIntConstraint(TypeOperation, TypeMethod) \
   mcutUnaryTable[Operation::T##TypeOperation].setFunction(&Approximate::Details::IntOperationElement::constraint##TypeMethod);

#define DefineInitBinaryConstraint(TypeOperation) \
   mcbtBinaryTable[Operation::T##TypeOperation].setFunction(&BaseAlgorithms::constraint##TypeOperation);

#define DefineInitBinaryIntConstraint(TypeOperation) \
   mcbtBinaryTable[Operation::T##TypeOperation].setFunction(&Approximate::Details::IntOperationElement::constraint##TypeOperation);
  
#define DefineInitBinaryOIntConstraint(TypeOperation, TypeMethod) \
   mcbtBinaryTable[Operation::T##TypeOperation].setFunction(&Approximate::Details::IntOperationElement::constraint##TypeMethod);
  
BaseAlgorithms::IGenericConstraintTable::IGenericConstraintTable() {
#include "StandardClasses/UndefineNew.h"
   DefineInitUnaryConstraint(ExtendWithZero)
   DefineInitUnaryConstraint(ExtendWithSign)
   DefineInitBinaryConstraint(Concat)
   DefineInitUnaryConstraint(Reduce)
   DefineInitBinaryConstraint(BitSet)

   DefineInitUnaryIntConstraint(CastBit)
   DefineInitUnaryOIntConstraint(CastChar, CastInteger)
   DefineInitUnaryOIntConstraint(CastInt, CastInteger)
   DefineInitUnaryOIntConstraint(CastUnsignedInt, CastInteger)
   // DefineInitUnaryOConstraint(CastFloat, CastDouble)
   // DefineInitUnaryOConstraint(CastDouble, CastDouble)
   // DefineInitUnaryOConstraint(CastLongDouble, CastDouble)

   // DefineInitOCompareConstraint(CompareLessSigned, CompareLess)
   // DefineInitOCompareConstraint(CompareLessUnsigned, CompareLess)
   // DefineInitOCompareConstraint(CompareLessOrEqualSigned, CompareLessOrEqual)
   // DefineInitOCompareConstraint(CompareLessOrEqualUnsigned, CompareLessOrEqual)
   // DefineInitCompareConstraint(CompareEqual)
   // DefineInitCompareConstraint(CompareDifferent)
   // DefineInitOCompareConstraint(CompareGreaterOrEqualSigned, CompareGreaterOrEqual)
   // DefineInitOCompareConstraint(CompareGreaterOrEqualUnsigned, CompareGreaterOrEqual)
   // DefineInitOCompareConstraint(CompareGreaterSigned, CompareGreater)
   // DefineInitOCompareConstraint(CompareGreaterUnsigned, CompareGreater)
   // DefineInitOBinaryConstraintGeneric(MinUnsignedAssign, MinAssign)
   // DefineInitOBinaryConstraintGeneric(MinSignedAssign, MinAssign)
   // DefineInitOBinaryConstraintGeneric(MaxUnsignedAssign, MaxAssign)
   // DefineInitOBinaryConstraintGeneric(MaxSignedAssign, MaxAssign)

   DefineInitUnaryOIntConstraint(PrevSignedAssign, PrevAssign)
   DefineInitUnaryOIntConstraint(PrevUnsignedAssign, PrevAssign)
   DefineInitUnaryOIntConstraint(NextSignedAssign, NextAssign)
   DefineInitUnaryOIntConstraint(NextUnsignedAssign, NextAssign)

   DefineInitBinaryConstraint(PlusSignedAssign)
   DefineInitBinaryConstraint(PlusUnsignedAssign)
   DefineInitBinaryConstraint(PlusUnsignedWithSignedAssign)
   DefineInitBinaryOIntConstraint(MinusSignedAssign, MinusAssign)
   DefineInitBinaryOIntConstraint(MinusUnsignedAssign, MinusAssign)
   DefineInitBinaryConstraint(TimesSignedAssign)
   DefineInitBinaryConstraint(TimesUnsignedAssign)
   DefineInitBinaryConstraint(DivideSignedAssign)
   DefineInitBinaryConstraint(DivideUnsignedAssign)

   DefineInitUnaryOIntConstraint(OppositeSignedAssign, OppositeAssign)

   DefineInitBinaryConstraint(ModuloSignedAssign)
   DefineInitBinaryConstraint(ModuloUnsignedAssign)
   DefineInitBinaryIntConstraint(BitOrAssign)
   DefineInitBinaryIntConstraint(BitAndAssign)
   DefineInitBinaryIntConstraint(BitExclusiveOrAssign)
   DefineInitUnaryIntConstraint(BitNegateAssign)
   // DefineInitBinaryGConstraint(LeftShiftAssign)
   // DefineInitBinaryGConstraint(LogicalRightShiftAssign)
   // DefineInitBinaryGConstraint(ArithmeticRightShiftAssign)

   // DefineInitBinaryConstraintGeneric(LogicalAndAssign)
   // DefineInitBinaryConstraintGeneric(LogicalOrAssign)
   // DefineInitUnaryGConstraint(LogicalNegateAssign)
#include "StandardClasses/DefineNew.h"
}

#undef DefineInitUnaryConstraint
#undef DefineInitUnaryIntConstraint
#undef DefineInitUnaryOIntConstraint
#undef DefineInitBinaryConstraint
#undef DefineInitBinaryIntConstraint
#undef DefineInitBinaryOIntConstraint

#include "StandardClasses/UndefineNew.h"
BaseAlgorithms::GenericConstraintTable::GenericConstraintTable()
   :  pTable(new IGenericConstraintTable()) {}
#include "StandardClasses/DefineNew.h"
   
BaseAlgorithms::GenericConstraintTable::~GenericConstraintTable()
   {  if (pTable) delete pTable; }
   
bool
BaseAlgorithms::GenericConstraintTable::execute(VirtualElement& top, const Operation& operation,
      const VirtualElement& result, ConstraintEnvironment& env) {
   if (operation.getArgumentsNumber() == 0)
      return pTable->unaryTable()[operation.getType()]
         .execute(top, operation, result, env);
   if ((operation.getType() >= Operation::StartOfCompare)
         && (operation.getType() < Operation::EndOfCompare)) {
      VirtualElement::ZeroResult zeroResult = result.queryZeroResult();
      if (zeroResult.isTrue()) {
         return pTable->compareTable()[operation.getType()]
            .execute(top, operation, env.getFirstArgument(), Argument().setResult(), env);
      }
      else if (zeroResult.isFalse()) {
         Operation::Type inverseOperation = (Operation::Type)
            (Operation::EndOfCompare-1 - operation.getType() + Operation::StartOfCompare);
         return pTable->compareTable()[inverseOperation]
            .execute(top, operation, env.getFirstArgument(), Argument().setResult(), env);
      }
      else
         env.mergeVerdictDegradate();
      return true;
   };
   return pTable->binaryTable()[operation.getType()]
      .execute(top, operation, env.getFirstArgument(), result, Argument().setResult(), env);
}

BaseAlgorithms::GenericConstraintTable BaseAlgorithms::gctGenericContraintTable;

bool
BaseAlgorithms::constraintConcat(VirtualElement& thisElement, const VirtualOperation&, VirtualElement& sourceElement,
      const VirtualElement& resultElement, ConstraintEnvironment& env) {
   // a.b = x -> a=reduce(x) and b = reduce(x)
   env.mergeVerdictExact();
   PPVirtualElement fstArg = Methods::applyAssign(PPVirtualElement(resultElement), ReduceOperation()
         .setLowBit(thisElement.getSizeInBits()).setHighBit(resultElement.getSizeInBits()-1), EvaluationApplyParameters(env));
   PPVirtualElement sndArg = Methods::applyAssign(PPVirtualElement(resultElement), ReduceOperation()
         .setLowBit(0).setHighBit(thisElement.getSizeInBits()-1), EvaluationApplyParameters(env));
   IntersectEnvironment thisIntersectionEnv(env, Argument().setResult()),
                        sourceIntersectionEnv(env, Argument().setFst());
   thisElement.intersectWith(*fstArg, thisIntersectionEnv);
   sourceElement.intersectWith(*sndArg, sourceIntersectionEnv);
   return true;
}

bool
BaseAlgorithms::constraintExtendWithZero(VirtualElement& thisElement, const VirtualOperation&, const VirtualElement& resultElement, ConstraintEnvironment& env) {
   env.mergeVerdict(VirtualElement::EvaluationVerdict().setPrecise());
   PPVirtualElement arg = Methods::applyAssign(PPVirtualElement(resultElement),
         ReduceOperation().setLowBit(0).setHighBit(thisElement.getSizeInBits()-1),
         EvaluationApplyParameters(env));
   PPVirtualElement extension = Methods::applyAssign(PPVirtualElement(resultElement), ReduceOperation()
         .setLowBit(thisElement.getSizeInBits()).setHighBit(resultElement.getSizeInBits()-1),
         EvaluationApplyParameters(env));
   if (!extension->queryZeroResult().mayBeZero()) {
      env.setEmpty();
      return true;
   };
   IntersectEnvironment intersectionEnv(env);
   thisElement.intersectWith(*arg, intersectionEnv);
   return true;
}

bool
BaseAlgorithms::constraintExtendWithSign(VirtualElement& thisElement, const VirtualOperation&, const VirtualElement& resultElement, ConstraintEnvironment& env) {
   env.mergeVerdict(VirtualElement::EvaluationVerdict().setPrecise());
   PPVirtualElement arg = Methods::applyAssign(PPVirtualElement(resultElement),
         ReduceOperation().setLowBit(0).setHighBit(thisElement.getSizeInBits()-1),
         EvaluationApplyParameters(env));
   PPVirtualElement extension = Methods::applyAssign(PPVirtualElement(resultElement),
         ReduceOperation().setLowBit(thisElement.getSizeInBits())
         .setHighBit(resultElement.getSizeInBits()-1), EvaluationApplyParameters(env));
   PPVirtualElement sign = Methods::applyAssign(PPVirtualElement(resultElement),
         ReduceOperation().setLowBit(thisElement.getSizeInBits()-1)
         .setHighBit(thisElement.getSizeInBits()-1), EvaluationApplyParameters(env));
   VirtualElement::ZeroResult isNegative = sign->queryZeroResult();
   if (isNegative.isFalse()) {
      if (!extension->queryZeroResult().mayBeZero()) {
         env.setEmpty();
         return true;
      };
   }
   else if (isNegative.isTrue()) {
      extension = Methods::applyAssign(extension,
            BitNegateAssignOperation(), EvaluationApplyParameters(env));
      if (!extension->queryZeroResult().mayBeZero()) {
         env.setEmpty();
         return true;
      };
   }
   else {
      if (!extension->queryZeroResult().mayBeZero()) {
         extension = Methods::applyAssign(extension,
               BitNegateAssignOperation(), EvaluationApplyParameters(env));
         if (!extension->queryZeroResult().mayBeZero()) {
            env.setEmpty();
            return true;
         };
      };
   };
   IntersectEnvironment intersectionEnv(env);
   thisElement.intersectWith(*arg, intersectionEnv);
   return true;
}

bool
BaseAlgorithms::constraintReduce(VirtualElement& thisElement, const VirtualOperation& aoperation, const VirtualElement& resultElement, ConstraintEnvironment& env) {
   AssumeCondition(dynamic_cast<const ReduceOperation*>(&aoperation))   
   const ReduceOperation& operation = (const ReduceOperation&) aoperation;
   env.mergeVerdict(VirtualElement::EvaluationVerdict().setPrecise());
   PPVirtualElement low;
   if (operation.getLowBit() > 0)
      low = Methods::applyAssign(PPVirtualElement(thisElement), ReduceOperation().setLowBit(0)
            .setHighBit(operation.getLowBit()-1), EvaluationApplyParameters(env));
   if (operation.getLowBit() > operation.getHighBit())
      return true;
   PPVirtualElement intermediate = VirtualElement::Methods::applyAssign(PPVirtualElement(thisElement),
         ReduceOperation().setLowBit(operation.getLowBit())
         .setHighBit(operation.getHighBit()), EvaluationApplyParameters(env));
   PPVirtualElement high;
   if (operation.getHighBit() < thisElement.getSizeInBits()-1)
      high = VirtualElement::Methods::applyAssign(PPVirtualElement(thisElement), ReduceOperation()
            .setLowBit(operation.getHighBit()+1).setHighBit(thisElement.getSizeInBits()-1),
            EvaluationApplyParameters(env));

   {  IntersectEnvironment intersectionEnv(env);
      intermediate->intersectWith(resultElement, intersectionEnv);
      if (intersectionEnv.hasResult())
         intermediate = intersectionEnv.presult();
   };

   if (low.isValid())
      intermediate = Methods::applyAssign(intermediate, ConcatOperation(),
            *low, EvaluationApplyParameters(env));
   if (high.isValid())
      intermediate = Methods::applyAssign(high, ConcatOperation(),
            *intermediate, EvaluationApplyParameters(env));

   IntersectEnvironment intersectionEnv(env);
   thisElement.intersectWith(*intermediate, intersectionEnv);
   return true;
}

bool
BaseAlgorithms::constraintBitSet(VirtualElement& thisElement, const VirtualOperation& aoperation, VirtualElement& sourceElement, const VirtualElement& resultElement, ConstraintEnvironment& env) {
   AssumeCondition(dynamic_cast<const BitSetOperation*>(&aoperation))
   const BitSetOperation& operation = (const BitSetOperation&) aoperation;
   env.mergeVerdict(VirtualElement::EvaluationVerdict().setPrecise());
   PPVirtualElement lowThis, lowResult;
   if (operation.getLowBit() > 0) {
      lowThis = Methods::applyAssign(PPVirtualElement(thisElement), ReduceOperation().setLowBit(0)
            .setHighBit(operation.getLowBit()-1), EvaluationApplyParameters(env));
      lowResult = Methods::applyAssign(PPVirtualElement(resultElement), ReduceOperation().setLowBit(0)
            .setHighBit(operation.getLowBit()-1), EvaluationApplyParameters(env));
   };
   if (operation.getLowBit() > operation.getHighBit())
      return true;

   PPVirtualElement intermediateThis = Methods::applyAssign(PPVirtualElement(thisElement),
         ReduceOperation().setLowBit(operation.getLowBit())
            .setHighBit(operation.getHighBit()), EvaluationApplyParameters(env));
   PPVirtualElement intermediateResult = Methods::applyAssign(PPVirtualElement(resultElement),
         ReduceOperation().setLowBit(operation.getLowBit())
            .setHighBit(operation.getHighBit()), EvaluationApplyParameters(env));

   PPVirtualElement highThis, highResult;
   if (operation.getHighBit() < thisElement.getSizeInBits()-1) {
      highThis = Methods::applyAssign(PPVirtualElement(thisElement), ReduceOperation().setLowBit(operation.getHighBit()+1)
               .setHighBit(thisElement.getSizeInBits()-1), EvaluationApplyParameters(env));
      highResult = VirtualElement::Methods::applyAssign(PPVirtualElement(resultElement),
            ReduceOperation().setLowBit(operation.getHighBit()+1)
               .setHighBit(resultElement.getSizeInBits()-1), EvaluationApplyParameters(env));
   };

   {  IntersectEnvironment sourceIntersectionEnv(env);
      sourceElement.intersectWith(*intermediateResult, sourceIntersectionEnv);
      if (sourceIntersectionEnv.hasResult())
         env.absorbFirstArgument((VirtualElement*) sourceIntersectionEnv.presult().extractElement());
   };
   {  IntersectEnvironment lowEnv(env), highEnv(env);
      if (lowThis.isValid()) {
         lowThis->intersectWith(*lowResult, lowEnv);
         if (lowEnv.hasResult())
            lowThis = lowEnv.presult();
      };
      if (highThis.isValid()) {
         highThis->intersectWith(*highResult, highEnv);
         if (highEnv.hasResult())
            highThis = highEnv.presult();
      };
   };

   if (highThis.isValid())
      intermediateThis = Methods::applyAssign(highThis,
            ConcatOperation(), *intermediateThis, EvaluationApplyParameters(env));
   if (lowThis.isValid())
      intermediateThis = Methods::applyAssign(intermediateThis, ConcatOperation(),
            *lowThis, EvaluationApplyParameters(env));

   IntersectEnvironment intersectionEnv(env);
   thisElement.intersectWith(*intermediateThis, intersectionEnv);
   return true;
}

template <class OperationTraits>
bool
BaseAlgorithms::constraintMinOperationAssign(OperationTraits traits, VirtualElement& thisElement, const VirtualOperation&,
      VirtualElement& sourceElement, const VirtualElement& resultElement, ConstraintEnvironment& env) {
   // min(a,b) = c -> a >= c, b >= c and a > c => b = c and b > c => a = c
   env.mergeVerdictExact();

   {  ConstraintEnvironment thisEnv = ConstraintEnvironment(EvaluationParameters(env));
      thisEnv.setFirstArgument(resultElement);
      VirtualElement::assume(thisElement.constraint(typename OperationTraits::GreaterOrEqualOperation(),
         *Methods::newTrue(thisElement), thisEnv));
      env.mergeWithIntersection(thisEnv, Argument().setResult());
   };
   
   if (Methods::applyBoolean(env.hasResult() ? env.getResult() : thisElement,
            typename OperationTraits::GreaterOperation(), resultElement)) {
      IntersectEnvironment intersectionEnv(env, Argument().setFst());
      VirtualElement::assume(sourceElement.intersectWith(resultElement, intersectionEnv));
   }
   else {
      ConstraintEnvironment sourceEnv = ConstraintEnvironment(EvaluationParameters(env));
      sourceEnv.setFirstArgument(resultElement);
      VirtualElement::assume(sourceElement.constraint(typename OperationTraits::GreaterOrEqualOperation(),
            *Methods::newTrue(sourceElement), sourceEnv));
      env.mergeWithIntersection(sourceEnv, Argument().setFst());

      if (Methods::applyBoolean(env.getFirstArgument(), typename OperationTraits::GreaterOperation(),
               resultElement)) {
         IntersectEnvironment intersectionEnv(env, Argument().setResult());
         VirtualElement::assume((env.hasResult() ? env.getResult() : thisElement).intersectWith(resultElement, intersectionEnv));
      }
      else if (!Methods::applyBoolean(env.hasResult() ? env.getResult() : thisElement,
               typename OperationTraits::EqualOperation(), resultElement)
            || !Methods::applyBoolean(env.getFirstArgument(),
               typename OperationTraits::EqualOperation(), resultElement))
         env.mergeVerdictDegradate();
   };

   return true;
}

template <class OperationTraits>
bool
BaseAlgorithms::constraintMaxOperationAssign(OperationTraits traits, VirtualElement& thisElement, const VirtualOperation&,
      VirtualElement& sourceElement, const VirtualElement& resultElement, ConstraintEnvironment& env) {
   // max(a,b) = c -> a <= c, b <= c and a < c => b = c and b < c => a = c
   env.mergeVerdictExact();

   {  ConstraintEnvironment thisEnv = ConstraintEnvironment(EvaluationParameters(env));
      thisEnv.setFirstArgument(resultElement);
      VirtualElement::assume(thisElement.constraint(typename OperationTraits::LessOrEqualOperation(),
         *Methods::newTrue(thisElement), thisEnv));
      env.mergeWithIntersection(thisEnv, Argument().setResult());
   }
   
   if (Methods::applyBoolean(env.hasResult() ? env.getResult() : thisElement,
         typename OperationTraits::LessOperation(), resultElement)) {
      IntersectEnvironment intersectionEnv(env, Argument().setFst());
      VirtualElement::assume(env.getFirstArgument().intersectWith(resultElement, intersectionEnv));
   }
   else {
      ConstraintEnvironment sourceEnv = ConstraintEnvironment(EvaluationParameters(env));
      sourceEnv.setFirstArgument(resultElement);
      VirtualElement::assume(sourceElement.constraint(typename OperationTraits::LessOrEqualOperation(),
            *Methods::newTrue(sourceElement), sourceEnv));
      env.mergeWithIntersection(sourceEnv, Argument().setFst());

      if (Methods::applyBoolean(env.getFirstArgument(),
            typename OperationTraits::LessOperation(), resultElement)) {
         IntersectEnvironment intersectionEnv(env, Argument().setResult());
         VirtualElement::assume((env.hasResult() ? env.getResult() : thisElement).intersectWith(resultElement, intersectionEnv));
      }
      else if (!Methods::applyBoolean(env.hasResult() ? env.getResult() : thisElement,
               typename OperationTraits::EqualOperation(), resultElement)
            || !Methods::applyBoolean(env.getFirstArgument(),
               typename OperationTraits::EqualOperation(), resultElement))
         env.mergeVerdictDegradate();
   };

   return true;
}

template <class OperationTraits>
bool
BaseAlgorithms::constraintPlusOperationAssign(OperationTraits traits, VirtualElement& thisElement, const VirtualOperation&,
      VirtualElement& sourceElement, const VirtualElement& resultElement, ConstraintEnvironment& env) { // a+b = x -> a = x-b and b = x-a
   env.mergeVerdictExact();
   PPVirtualElement fstArg = Methods::apply(resultElement, (typename OperationTraits::MinusOperation()).setConstWithAssign(),
         sourceElement, EvaluationApplyParameters(env));
   PPVirtualElement sndArg = Methods::apply(resultElement, (typename OperationTraits::MinusOperation()).setConstWithAssign(),
         thisElement, EvaluationApplyParameters(env));
   IntersectEnvironment thisIntersectionEnv(env, Argument().setResult()),
                        sourceIntersectionEnv(env, Argument().setFst());
   thisElement.intersectWith(*fstArg, thisIntersectionEnv);
   sourceElement.intersectWith(*sndArg, sourceIntersectionEnv);
   return true;
}

template <class OperationTraits>
bool
BaseAlgorithms::constraintMinusOperationAssign(OperationTraits traits, VirtualElement& thisElement, const VirtualOperation&,
      VirtualElement& sourceElement, const VirtualElement& resultElement, ConstraintEnvironment& env) { // a-b = x -> a = b+x and b = a-x
   env.mergeVerdictExact();
   PPVirtualElement fstArg = Methods::apply(resultElement, (typename OperationTraits::PlusOperation()).setConstWithAssign(), sourceElement, EvaluationApplyParameters(env));
   PPVirtualElement sndArg = Methods::apply(thisElement, (typename OperationTraits::MinusOperation()).setConstWithAssign(), resultElement, EvaluationApplyParameters(env));
   IntersectEnvironment thisIntersectionEnv(env, Argument().setResult()),
                        sourceIntersectionEnv(env, Argument().setFst());
   thisElement.intersectWith(*fstArg, thisIntersectionEnv);
   sourceElement.intersectWith(*sndArg, sourceIntersectionEnv);
   return true;
}

template <class OperationTraits>
bool
BaseAlgorithms::constraintTimesOperationAssign(OperationTraits traits, VirtualElement& thisElement, const VirtualOperation&,
      VirtualElement& sourceElement, const VirtualElement& resultElement, ConstraintEnvironment& env) {
   env.mergeVerdictExact();
   if (env.hasEvaluationError() || (env.getInformationKind().isSure()))
      // We could try some intervals for under-approximation
      return true;

   Approximate::Details::IntOperationElement::QueryOperation::CompareSpecialEnvironment
      thisCompare = Methods::compareSpecial(thisElement),
      resultCompare = Methods::compareSpecial(resultElement),
      sourceCompare = Methods::compareSpecial(sourceElement);
   if (resultCompare.mayBeZero() && (thisCompare.mayBeZero() || sourceCompare.mayBeZero()))
      return true; // dynamic partitioning could give more information
   PPVirtualElement
      intersectFstArg = Methods::apply(resultElement, (typename OperationTraits::DivideOperation()).setConstWithAssign(), sourceElement,
            EvaluationParameters().stopErrorStates()),
      intersectSndArg = Methods::apply(resultElement, (typename OperationTraits::DivideOperation()).setConstWithAssign(), thisElement,
            EvaluationParameters().stopErrorStates());
   if (env.getInformationKind().isExact()) {
      PNT::TPassPointer<VirtualDisjunction, PPAbstractElement> disjunctionFstArg = Methods::newDisjunction(thisElement);
      disjunctionFstArg->addMay(intersectFstArg);
      intersectFstArg = disjunctionFstArg;

      PNT::TPassPointer<VirtualDisjunction, PPAbstractElement> disjunctionSndArg = Methods::newDisjunction(thisElement);
      disjunctionSndArg->addMay(intersectSndArg);
      intersectSndArg = disjunctionSndArg;
   };
   IntersectEnvironment
      thisIntersectionEnv(env, Argument().setResult()), sourceIntersectionEnv(env, Argument().setFst());
   thisElement.intersectWith(*intersectFstArg, thisIntersectionEnv);
   sourceElement.intersectWith(*intersectSndArg, sourceIntersectionEnv);
   return true;
}

template <class OperationTraits>
bool
BaseAlgorithms::constraintDivideOperationAssign(OperationTraits traits, VirtualElement& thisElement, const VirtualOperation&,
      VirtualElement& sourceElement, const VirtualElement& resultElement, ConstraintEnvironment& env) { // x/y = z;
   env.mergeVerdictDegradate();
   const EvaluationParameters natural = EvaluationApplyParameters(env);
   PPVirtualElement thisElementConstraint, sourceElementConstraint;
   if (env.doesPropagateErrorStates() && env.isOverflow()) { // division by -1 -> add Minint as may
      thisElementConstraint = Methods::newMin(thisElement);
      sourceElementConstraint = Methods::newMinusOne(thisElement);
      thisElementConstraint = Methods::intersect(thisElementConstraint, thisElement, natural);
      sourceElementConstraint = Methods::intersect(sourceElementConstraint, sourceElement, natural);
   };

   Approximate::Details::IntOperationElement::CompareSpecialEnvironment
      thisCompare = OperationTraits::compareSpecial(thisElement),
      resultCompare = OperationTraits::compareSpecial(resultElement),
      sourceCompare = OperationTraits::compareSpecial(sourceElement);

   if (resultCompare.mayBeZero() && thisCompare.mayBeZero())
      return true; // dynamic partitioning could give more info

   if (resultCompare.mayBeZero()) { // thisCompare.isNotZero() : a/b == 0
      EvaluationParameters mayStopErrorStates(VirtualElement::EPMayStopErrorStates);
      mayStopErrorStates.setApproximatePartFrom(env);
      PPVirtualElement sourceElementPositive, sourceElementNegative, thisElementPositive, thisElementNegative;
      if (thisCompare.mayBeGreaterZero()) { // b > a
         thisElementPositive.setElement(thisElement);
         if (!thisCompare.isGreaterZero())
            thisElementPositive = Methods::constraint(thisElementPositive, typename OperationTraits::GreaterOrEqualOperation(),
               *Methods::newOne(thisElement), *Methods::newTrue(thisElement), mayStopErrorStates);
         sourceElementPositive = Methods::constraint(sourceElement, typename OperationTraits::GreaterOperation(),
               thisElementPositive, *Methods::newTrue(thisElement), mayStopErrorStates);
      };
      if (thisCompare.mayBeLessZero()) { // b < a
         thisElementNegative.setElement(thisElement);
         if (!thisCompare.isLessZero()) {
            PPVirtualElement minusOne = Methods::newMinusOne(thisElement);
            thisElementNegative = Methods::constraint(thisElementNegative, typename OperationTraits::LessOrEqualOperation(),
               minusOne, *Methods::newTrue(thisElement), mayStopErrorStates);
         };
         sourceElementNegative = Methods::constraint(sourceElement, typename OperationTraits::LessOperation(),
               thisElementNegative, *Methods::newTrue(thisElement), mayStopErrorStates);
      };

      thisElementConstraint = Methods::merge(thisElementConstraint, thisElementPositive, natural);
      thisElementConstraint = Methods::merge(thisElementConstraint, thisElementNegative, natural);
      sourceElementConstraint = Methods::merge(sourceElementConstraint, sourceElementPositive, natural);
      sourceElementConstraint = Methods::merge(sourceElementConstraint, sourceElementNegative, natural);
   };

   VirtualElement *sourceElementPositive=nullptr, *sourceElementNegative=nullptr, *thisElementPositive=nullptr,
      *thisElementNegative=nullptr;
   const VirtualElement *resultElementPositive=nullptr, *resultElementNegative=nullptr;
   PPVirtualElement ssourceElementPositive, ssourceElementNegative, sthisElementPositive, sthisElementNegative,
      sresultElementPositive, sresultElementNegative;

   if (sourceCompare.isGreaterZero())
      sourceElementPositive = &sourceElement;
   else if (sourceCompare.isLessZero())
      sourceElementNegative = &sourceElement;
   else {
      EvaluationParameters mayStopErrorStates(VirtualElement::EPMayStopErrorStates);
      mayStopErrorStates.setApproximatePartFrom(env);
      ssourceElementPositive = Methods::constraint(sourceElement, typename OperationTraits::GreaterOrEqualOperation(),
         *Methods::newOne(thisElement), *Methods::newTrue(thisElement), mayStopErrorStates),
      ssourceElementNegative = Methods::constraint(sourceElement, typename OperationTraits::LessOperation(),
         *Methods::newMinusOne(thisElement), *Methods::newTrue(thisElement), mayStopErrorStates),
      sourceElementPositive = ssourceElementPositive.key();
      sourceElementNegative = ssourceElementNegative.key();
   };

   if (resultCompare.isGreaterZero())
      resultElementPositive = &resultElement;
   else if (resultCompare.isLessZero())
      resultElementNegative = &resultElement;
   else {
      EvaluationParameters mayStopErrorStates(VirtualElement::EPMayStopErrorStates);
      mayStopErrorStates.setApproximatePartFrom(env);
      sresultElementPositive = Methods::constraint(resultElement, typename OperationTraits::GreaterOrEqualOperation(),
         *Methods::newOne(thisElement), *Methods::newTrue(thisElement), mayStopErrorStates),
      sresultElementNegative = Methods::constraint(resultElement, typename OperationTraits::LessOperation(),
         *Methods::newMinusOne(thisElement), *Methods::newTrue(thisElement), mayStopErrorStates),
      resultElementPositive = sresultElementPositive.key();
      resultElementNegative = sresultElementNegative.key();
   };

   if (thisCompare.isGreaterZero())
      thisElementPositive = &thisElement;
   else if (thisCompare.isLessZero())
      thisElementNegative = &thisElement;
   else {
      EvaluationParameters mayStopErrorStates(VirtualElement::EPMayStopErrorStates);
      mayStopErrorStates.setApproximatePartFrom(env);
      sthisElementPositive = Methods::constraint(thisElement, typename OperationTraits::GreaterOrEqualOperation(),
         *Methods::newOne(thisElement), *Methods::newTrue(thisElement), mayStopErrorStates);
      sthisElementNegative = Methods::constraint(thisElement, typename OperationTraits::LessOperation(),
         *Methods::newMinusOne(thisElement), *Methods::newTrue(thisElement), mayStopErrorStates);
      thisElementPositive = sthisElementPositive.key();
      thisElementNegative = sthisElementNegative.key();
   };

   if (thisElementPositive && sourceElementPositive && resultElementPositive) { // x,y,z > 0
      EvaluationParameters mayStopErrorStates(VirtualElement::EPMayStopErrorStates);
      mayStopErrorStates.setApproximatePartFrom(env);
      // z <= x/y < z+1 => x in [y*z, y*(z+1)-1]
      PPVirtualElement thisElementMin = Methods::apply(*sourceElementPositive,
            (typename OperationTraits::TimesOperation()).setConstWithAssign(),
            *resultElementPositive, mayStopErrorStates);
      PPVirtualElement thisElementMax;
      thisElementMax = Methods::applyAssign(PPVirtualElement(*resultElementPositive),
            typename OperationTraits::NextOperation(), mayStopErrorStates); // z+1
      if (thisElementMax.isValid()) // y*(z+1)
         thisElementMax = Methods::applyAssign(thisElementMax,
            typename OperationTraits::TimesOperation(), *sourceElementPositive, mayStopErrorStates);
      if (thisElementMax.isValid()) // y*(z+1)-1
         thisElementMax = Methods::applyAssign(thisElementMax,
            typename OperationTraits::PrevOperation(), mayStopErrorStates); // y*(z+1)-1

      PPVirtualElement thisElementPositiveConstraint;
      if (thisElementMin.isValid())
         thisElementPositiveConstraint = Methods::constraint(PPVirtualElement(thisElement),
            typename OperationTraits::GreaterOrEqualOperation(), thisElementMin, *Methods::newTrue(thisElement), mayStopErrorStates);
      if (thisElementPositiveConstraint.isValid() && thisElementMax.isValid())
         thisElementPositiveConstraint = Methods::constraint(thisElementPositiveConstraint,
               typename OperationTraits::LessOrEqualOperation(), thisElementMax, *Methods::newTrue(thisElement), mayStopErrorStates);
      thisElementConstraint = Methods::merge(thisElementConstraint, thisElementPositiveConstraint, natural);

      // y in [(x+1)/(z+1), x/z]
      PPVirtualElement sourceElementMax = Methods::apply(*thisElementPositive,
            (typename OperationTraits::DivideOperation()).setConstWithAssign(), *resultElementPositive, mayStopErrorStates);
      PPVirtualElement sourceElementMin = Methods::applyAssign(
            PPVirtualElement(*thisElementPositive),
            typename OperationTraits::NextOperation(), mayStopErrorStates); // x+1
      PPVirtualElement temp = Methods::applyAssign(PPVirtualElement(*resultElementPositive),
            typename OperationTraits::NextOperation(), mayStopErrorStates); // z+1
      if (sourceElementMin.isValid() && temp.isValid())
         sourceElementMin = Methods::applyAssign(sourceElementMin,
               typename OperationTraits::DivideOperation(),
               *temp, mayStopErrorStates); // (x+1)/(z+1)
      else if (!temp.isValid())
         sourceElementMin.release();

      PPVirtualElement sourceElementPositiveConstraint;
      if (sourceElementMin.isValid())
         sourceElementPositiveConstraint = Methods::constraint(
               PPVirtualElement(sourceElement), typename OperationTraits::GreaterOrEqualOperation(),
               sourceElementMin, *Methods::newTrue(thisElement), mayStopErrorStates);
      if (sourceElementPositiveConstraint.isValid() && sourceElementMax.isValid())
         sourceElementPositiveConstraint = Methods::constraint(sourceElementPositiveConstraint,
               typename OperationTraits::LessOrEqualOperation(), sourceElementMax,
               *Methods::newTrue(thisElement), mayStopErrorStates);
      sourceElementConstraint = Methods::merge(sourceElementConstraint, sourceElementPositiveConstraint, natural);
   };

   if (thisElementPositive && sourceElementNegative && resultElementNegative) { // x >= 0 and y,z < 0
      EvaluationParameters mayStopErrorStates(VirtualElement::EPMayStopErrorStates);
      mayStopErrorStates.setApproximatePartFrom(env);
      // z-1 < x/y <= z => x in [y*z, y*(z-1)-1]
      PPVirtualElement thisElementMin = Methods::apply(*sourceElementNegative,
            (typename OperationTraits::TimesOperation()).setConstWithAssign(), *resultElementNegative, mayStopErrorStates);
      PPVirtualElement thisElementMax = Methods::applyAssign(
            PPVirtualElement(*resultElementNegative),
            typename OperationTraits::PrevOperation(), mayStopErrorStates); // z-1
      if (thisElementMax.isValid()) // y*(z-1)
         thisElementMax = Methods::applyAssign(thisElementMax,
               typename OperationTraits::TimesOperation(), *sourceElementNegative, mayStopErrorStates);
      if (thisElementMax.isValid())
         thisElementMax = Methods::applyAssign(thisElementMax,
            typename OperationTraits::PrevOperation(), mayStopErrorStates); // y*(z-1)-1

      PPVirtualElement thisElementPositiveConstraint;
      if (thisElementMin.isValid())
         thisElementPositiveConstraint = Methods::constraint(PPVirtualElement(thisElement),
            typename OperationTraits::GreaterOrEqualOperation(), thisElementMin, *Methods::newTrue(thisElement), mayStopErrorStates);
      if (thisElementPositiveConstraint.isValid() && thisElementMax.isValid())
         thisElementPositiveConstraint = Methods::constraint(thisElementPositiveConstraint,
               typename OperationTraits::LessOrEqualOperation(), thisElementMax, *Methods::newTrue(thisElement), mayStopErrorStates);
      thisElementConstraint = Methods::merge(thisElementConstraint, thisElementPositiveConstraint, natural);

      // y in [(x+1)/(z-1), x/z]
      PPVirtualElement sourceElementMax = Methods::apply(*thisElementPositive,
            (typename OperationTraits::DivideOperation()).setConstWithAssign(), *resultElementNegative, mayStopErrorStates);
      PPVirtualElement sourceElementMin = Methods::applyAssign(
            PPVirtualElement(*thisElementPositive),
            typename OperationTraits::NextOperation(), mayStopErrorStates); // x+1
      PPVirtualElement temp = Methods::applyAssign(PPVirtualElement(*resultElementNegative),
            typename OperationTraits::PrevOperation(), mayStopErrorStates); // z-1
      if (sourceElementMin.isValid() && temp.isValid())
         sourceElementMin = Methods::applyAssign(sourceElementMin,
            typename OperationTraits::DivideOperation(),
            *temp, mayStopErrorStates); // (x+1)/(z-1)
      else if (!temp.isValid())
         sourceElementMin.release();

      PPVirtualElement sourceElementNegativeConstraint;
      if (sourceElementMin.isValid())
         sourceElementNegativeConstraint = Methods::constraint(PPVirtualElement(sourceElement),
               typename OperationTraits::GreaterOrEqualOperation(), sourceElementMin, *Methods::newTrue(thisElement), mayStopErrorStates);
      if (sourceElementNegativeConstraint.isValid() && sourceElementMax.isValid())
         sourceElementNegativeConstraint = Methods::constraint(sourceElementNegativeConstraint,
               typename OperationTraits::LessOrEqualOperation(), sourceElementMax, *Methods::newTrue(thisElement), mayStopErrorStates);
      sourceElementConstraint = Methods::merge(sourceElementConstraint, sourceElementNegativeConstraint, natural);
   };

   if (thisElementNegative && sourceElementNegative && resultElementPositive) { // x,y < 0 and z >= 0
      EvaluationParameters mayStopErrorStates(VirtualElement::EPMayStopErrorStates);
      mayStopErrorStates.setApproximatePartFrom(env);
      // z <= x/y < z+1 => x in [y*(z+1)+1, y*z]
      PPVirtualElement thisElementMax = Methods::apply(*sourceElementNegative,
            (typename OperationTraits::TimesOperation()).setConstWithAssign(), *resultElementPositive, mayStopErrorStates);
      PPVirtualElement thisElementMin = Methods::applyAssign(
            PPVirtualElement(*resultElementPositive),
            typename OperationTraits::NextOperation(), mayStopErrorStates); // z+1
      if (thisElementMin.isValid())
         thisElementMin = Methods::applyAssign(thisElementMin,
               typename OperationTraits::TimesOperation(), *sourceElementNegative,
               mayStopErrorStates); // y*(z+1)
      if (thisElementMin.isValid())
         thisElementMin = Methods::applyAssign(thisElementMin,
            typename OperationTraits::NextOperation(), mayStopErrorStates); // y*(z+1)+1

      PPVirtualElement thisElementNegativeConstraint;
      if (thisElementMin.isValid())
         thisElementNegativeConstraint = Methods::constraint(PPVirtualElement(thisElement),
               typename OperationTraits::GreaterOrEqualOperation(), thisElementMin, *Methods::newTrue(thisElement), mayStopErrorStates);
      if (thisElementNegativeConstraint.isValid() && thisElementMax.isValid())
         thisElementNegativeConstraint = Methods::constraint(thisElementNegativeConstraint,
               typename OperationTraits::LessOrEqualOperation(), thisElementMax, *Methods::newTrue(thisElement), mayStopErrorStates);
      thisElementConstraint = Methods::merge(thisElementConstraint, thisElementNegativeConstraint, natural);

      // y in [x/z, (x-1)/(z+1)]
      PPVirtualElement sourceElementMin = Methods::apply(*thisElementNegative,
            (typename OperationTraits::DivideOperation()).setConstWithAssign(), *resultElementPositive, mayStopErrorStates);
      PPVirtualElement sourceElementMax = Methods::applyAssign(
            PPVirtualElement(*thisElementNegative),
            typename OperationTraits::PrevOperation(), mayStopErrorStates); // x-1
      PPVirtualElement temp = Methods::applyAssign(PPVirtualElement(*resultElementPositive),
            typename OperationTraits::NextOperation(), mayStopErrorStates); // z+1
      if (sourceElementMax.isValid() && temp.isValid())
         sourceElementMax = Methods::applyAssign(sourceElementMax,
            typename OperationTraits::DivideOperation(),
            *temp, mayStopErrorStates); // (x-1)/(z+1)
      else if (!temp.isValid())
         sourceElementMax.release();

      PPVirtualElement sourceElementNegativeConstraint;
      if (sourceElementMin.isValid())
         sourceElementNegativeConstraint = Methods::constraint(PPVirtualElement(sourceElement),
               typename OperationTraits::GreaterOrEqualOperation(), sourceElementMin, *Methods::newTrue(thisElement), mayStopErrorStates);
      if (sourceElementNegativeConstraint.isValid() && sourceElementMax.isValid())
         sourceElementNegativeConstraint = Methods::constraint(sourceElementNegativeConstraint,
               typename OperationTraits::LessOrEqualOperation(), sourceElementMax, *Methods::newTrue(thisElement), mayStopErrorStates);
      sourceElementConstraint = Methods::merge(sourceElementConstraint, sourceElementNegativeConstraint, natural);
   };

   if (thisElementNegative && sourceElementPositive && resultElementNegative) { // y >= 0 and x,z < 0
      EvaluationParameters mayStopErrorStates(VirtualElement::EPMayStopErrorStates);
      mayStopErrorStates.setApproximatePartFrom(env);
      // z-1 < x/y <= z => x in [y*(z-1)+1, y*z]
      PPVirtualElement thisElementMax = Methods::apply(*sourceElementPositive,
            (typename OperationTraits::TimesOperation()).setConstWithAssign(), resultElement, mayStopErrorStates);
      PPVirtualElement thisElementMin = Methods::applyAssign(PPVirtualElement(resultElement),
            typename OperationTraits::NextOperation(), mayStopErrorStates); // z+1
      if (thisElementMin.isValid()) // y*(z+1)
         thisElementMin = Methods::applyAssign(thisElementMin,
               typename OperationTraits::TimesOperation(), *sourceElementPositive, mayStopErrorStates);
      if (thisElementMin.isValid())
         thisElementMin = Methods::applyAssign(thisElementMin,
            typename OperationTraits::NextOperation(), mayStopErrorStates); // y*(z+1)+1

      PPVirtualElement thisElementNegativeConstraint;
      if (thisElementMin.isValid())
         thisElementNegativeConstraint = Methods::constraint(PPVirtualElement(thisElement),
               typename OperationTraits::GreaterOrEqualOperation(), thisElementMin, *Methods::newTrue(thisElement), mayStopErrorStates);
      if (thisElementNegativeConstraint.isValid() && thisElementMax.isValid())
         thisElementNegativeConstraint = Methods::constraint(thisElementNegativeConstraint,
               typename OperationTraits::LessOrEqualOperation(), thisElementMax, *Methods::newTrue(thisElement), mayStopErrorStates);
      thisElementConstraint = Methods::merge(thisElementConstraint, thisElementNegativeConstraint, natural);

      // y in [(x-1)/(z-1), x/z]
      PPVirtualElement sourceElementMax = Methods::apply(*thisElementNegative,
            (typename OperationTraits::DivideOperation()).setConstWithAssign(), *resultElementNegative, mayStopErrorStates);
      PPVirtualElement sourceElementMin = Methods::applyAssign(
            PPVirtualElement(*thisElementNegative),
            typename OperationTraits::PrevOperation(), mayStopErrorStates);  // x-1
      PPVirtualElement temp = Methods::applyAssign(PPVirtualElement(*resultElementNegative),
            typename OperationTraits::PrevOperation(), mayStopErrorStates); // z-1
      if (sourceElementMin.isValid() && temp.isValid())
         sourceElementMin = Methods::applyAssign(sourceElementMin,
            typename OperationTraits::DivideOperation(),
            *temp, mayStopErrorStates); // (x-1)/(z-1)
      else if (!temp.isValid())
         sourceElementMin.release();

      PPVirtualElement sourceElementPositiveConstraint;
      if (sourceElementMin.isValid())
         sourceElementPositiveConstraint = Methods::constraint(PPVirtualElement(sourceElement),
               typename OperationTraits::GreaterOrEqualOperation(), sourceElementMin, *Methods::newTrue(thisElement), mayStopErrorStates);
      if (sourceElementPositiveConstraint.isValid() && sourceElementMax.isValid())
         sourceElementPositiveConstraint = Methods::constraint(sourceElementPositiveConstraint,
               typename OperationTraits::LessOrEqualOperation(), sourceElementMax, *Methods::newTrue(thisElement), mayStopErrorStates);
      sourceElementConstraint = Methods::merge(sourceElementConstraint, sourceElementPositiveConstraint, natural);
   };

   if (!thisElementConstraint.isValid() || !sourceElementConstraint.isValid())
      env.setEmpty();
   else {
      IntersectEnvironment thisIntersectionEnv(env, Argument().setResult());
      IntersectEnvironment sourceIntersectionEnv(env, Argument().setFst());
      thisElement.intersectWith(*thisElementConstraint, thisIntersectionEnv);
      sourceElement.intersectWith(*sourceElementConstraint, sourceIntersectionEnv);
   };
   return true;
}

template <class OperationTraits>
bool
BaseAlgorithms::constraintModuloOperationAssign(OperationTraits traits, VirtualElement& thisElement, const VirtualOperation&,
      VirtualElement& sourceElement, const VirtualElement& resultElement, ConstraintEnvironment& env) {
   env.mergeVerdictDegradate();
   Approximate::Details::IntOperationElement::QueryOperation::CompareSpecialEnvironment
      thisCompare = OperationTraits::compareSpecial(thisElement),
      resultCompare = OperationTraits::compareSpecial(resultElement),
      sourceCompare = OperationTraits::compareSpecial(sourceElement);
   if (thisCompare.isGreaterOrEqualZero() && sourceCompare.isGreaterOrEqualZero()
         && resultCompare.isGreaterOrEqualZero()) {
      EvaluationParameters mayStopErrorStates(VirtualElement::EPMayStopErrorStates);
      mayStopErrorStates.setApproximatePartFrom(env);
      ConstraintEnvironment thisConstraintEnv(mayStopErrorStates);
      thisConstraintEnv.absorbFirstArgument(resultElement.createSCopy());
      thisElement.constraint(typename OperationTraits::GreaterOrEqualOperation(), *Methods::newTrue(thisElement), thisConstraintEnv);
      env.mergeWithIntersection(thisConstraintEnv);

      PPVirtualElement newResultElement((VirtualElement*) thisConstraintEnv.extractFirstArgument(), PNT::Pointer::Init());
      PPVirtualElement newSourceElement((VirtualElement*) env.extractFirstArgument(), PNT::Pointer::Init());
      env.absorbFirstArgument(Methods::constraintBoolean(newSourceElement, typename OperationTraits::GreaterOperation(),
         newResultElement, true, mayStopErrorStates).extractElement());
   };
   return true;
}

bool
BaseAlgorithms::constraintMinUnsignedAssign(VirtualElement& thisElement, const VirtualOperation& operation,
      VirtualElement& sourceElement, const VirtualElement& resultElement, ConstraintEnvironment& env) {
   return Details::BaseAlgorithms::constraintMinOperationAssign(UnsignedOperationTraits(), thisElement,
         operation, sourceElement, resultElement, env);
}

bool
BaseAlgorithms::constraintMinSignedAssign(VirtualElement& thisElement, const VirtualOperation& operation,
      VirtualElement& sourceElement, const VirtualElement& resultElement, ConstraintEnvironment& env) {
   return Details::BaseAlgorithms::constraintMinOperationAssign(SignedOperationTraits(), thisElement,
         operation, sourceElement, resultElement, env);
}

bool
BaseAlgorithms::constraintMinFloatAssign(VirtualElement& thisElement, const VirtualOperation& operation,
      VirtualElement& sourceElement, const VirtualElement& resultElement, ConstraintEnvironment& env) {
   return Details::BaseAlgorithms::constraintMinOperationAssign(FloatOperationTraits(), thisElement,
         operation, sourceElement, resultElement, env);
}

bool
BaseAlgorithms::constraintMaxUnsignedAssign(VirtualElement& thisElement, const VirtualOperation& operation,
      VirtualElement& sourceElement, const VirtualElement& resultElement, ConstraintEnvironment& env) {
   return Details::BaseAlgorithms::constraintMaxOperationAssign(UnsignedOperationTraits(), thisElement,
         operation, sourceElement, resultElement, env);
}

bool
BaseAlgorithms::constraintMaxSignedAssign(VirtualElement& thisElement, const VirtualOperation& operation,
      VirtualElement& sourceElement, const VirtualElement& resultElement, ConstraintEnvironment& env) {
   return Details::BaseAlgorithms::constraintMaxOperationAssign(SignedOperationTraits(), thisElement,
         operation, sourceElement, resultElement, env);
}

bool
BaseAlgorithms::constraintMaxFloatAssign(VirtualElement& thisElement, const VirtualOperation& operation,
      VirtualElement& sourceElement, const VirtualElement& resultElement, ConstraintEnvironment& env) {
   return Details::BaseAlgorithms::constraintMaxOperationAssign(FloatOperationTraits(), thisElement,
         operation, sourceElement, resultElement, env);
}

bool
BaseAlgorithms::constraintPlusUnsignedAssign(VirtualElement& thisElement, const VirtualOperation& operation,
      VirtualElement& sourceElement, const VirtualElement& resultElement, ConstraintEnvironment& env) {
   return Details::BaseAlgorithms::constraintPlusOperationAssign(UnsignedOperationTraits(), thisElement,
         operation, sourceElement, resultElement, env);
}

bool
BaseAlgorithms::constraintPlusUnsignedWithSignedAssign(VirtualElement& thisElement,
      const VirtualOperation&, VirtualElement& sourceElement, const VirtualElement& resultElement,
      ConstraintEnvironment& env) { // a+b = x -> a = x-b and b = x-a
   env.mergeVerdictExact();
   VirtualElement::ZeroResult lessZero = Methods::applyBoolean(sourceElement,
         CompareLessSignedOperation(), *Methods::newZero(thisElement));
   PPVirtualElement fstArg, sndArg;
   bool hasFound = false;
   if (lessZero.isFalse()) { // b >= 0 and x >= a, cast of b into unsigned
      fstArg = Methods::apply(resultElement, MinusUnsignedAssignOperation().setConstWithAssign(),
            sourceElement, EvaluationApplyParameters(env));
      sndArg = Methods::apply(resultElement, MinusUnsignedAssignOperation().setConstWithAssign(),
            thisElement, EvaluationApplyParameters(env));
      hasFound = true;
   }
   else if (lessZero.isTrue()) { // b < 0 and x < a, cast of b into -b
      EvaluationEnvironment oppositeEnv(EvaluationApplyParameters(env).propagateErrorStates());
      PPVirtualElement newSourceElement = Methods::apply(sourceElement,
            Scalar::MultiBit::OppositeSignedAssignOperation().setConstWithAssign(), oppositeEnv);
      if (!oppositeEnv.hasEvaluationError()) {
         fstArg = Methods::apply(resultElement, PlusUnsignedWithSignedAssignOperation().setConstWithAssign(),
               *newSourceElement, EvaluationApplyParameters(env));
         sndArg = Methods::apply(thisElement, MinusUnsignedAssignOperation().setConstWithAssign(),
               resultElement, EvaluationApplyParameters(env));
         sndArg = Methods::applyAssign(sndArg, OppositeSignedAssignOperation(),
               EvaluationApplyParameters(env));
         hasFound = true;
      };
   };
   if (!hasFound) {
      EvaluationParameters exactStopErrorStates(VirtualElement::EPExactStopErrorStates);
      exactStopErrorStates.setApproximatePartFrom(env);
      PPVirtualElement lessZeroSource = Methods::constraint(PPVirtualElement(sourceElement),
            Scalar::MultiBit::CompareLessSignedOperation(), *Methods::newZero(thisElement),
            *Methods::newTrue(thisElement), EvaluationParameters(env));
      PPVirtualElement minint = Methods::newOne(thisElement);
      minint = Methods::applyAssign(minint, LeftShiftAssignOperation(),
            *Methods::newIntForShift(thisElement, thisElement.getSizeInBits()-1), exactStopErrorStates);
      PPVirtualElement minintSource = Methods::constraint(*lessZeroSource,
            CompareEqualOperation(), *minint,
            *Methods::newTrue(thisElement), EvaluationParameters(env));
      if (lessZeroSource.isValid())
         lessZeroSource = Methods::constraint(lessZeroSource,
               CompareGreaterSignedOperation(), *minint,
               *Methods::newTrue(thisElement), EvaluationParameters(env));
      PPVirtualElement greaterOrEqualZeroSource = Methods::constraint(
            PPVirtualElement(sourceElement), CompareGreaterOrEqualSignedOperation(),
            *Methods::newZero(thisElement), *Methods::newTrue(thisElement), EvaluationParameters(env));

      PPVirtualElement fstArgGreaterOrEqualZero;
      if (greaterOrEqualZeroSource.isValid())
         fstArgGreaterOrEqualZero = Methods::apply(resultElement,
               MinusUnsignedAssignOperation().setConstWithAssign(), *greaterOrEqualZeroSource,
               EvaluationApplyParameters(env));
      PPVirtualElement sndArgGreaterOrEqualZero = Methods::apply(resultElement,
            MinusUnsignedAssignOperation().setConstWithAssign(), thisElement,
            EvaluationApplyParameters(env));
      if (sndArgGreaterOrEqualZero.isValid())
         sndArgGreaterOrEqualZero = Methods::constraint(sndArgGreaterOrEqualZero,
               CompareLessOrEqualUnsignedOperation(), resultElement,
               *Methods::newTrue(thisElement), EvaluationParameters(env));

      PPVirtualElement source = Methods::apply(sourceElement, OppositeSignedAssignOperation().setConstWithAssign(),
            EvaluationApplyParameters(env).stopErrorStates());
      PPVirtualElement fstArgLessZero = Methods::apply(resultElement,
            PlusUnsignedWithSignedAssignOperation().setConstWithAssign(), *source,
            EvaluationApplyParameters(env));
      PPVirtualElement sndArgLessZero = Methods::constraint(thisElement,
            CompareGreaterUnsignedOperation(), resultElement,
            *Methods::newTrue(thisElement), EvaluationParameters(env));
      if (sndArgLessZero.isValid())
         sndArgLessZero = Methods::applyAssign(sndArgLessZero,
               MinusUnsignedAssignOperation(), resultElement,
               EvaluationApplyParameters(env).stopErrorStates());
      if (sndArgLessZero.isValid())
         sndArgLessZero = Methods::applyAssign(sndArgLessZero,
               OppositeSignedAssignOperation(), EvaluationApplyParameters(env));

      PPVirtualElement fstArgMinint = Methods::apply(resultElement,
            PlusUnsignedAssignOperation().setConstWithAssign(), *minintSource,
            EvaluationApplyParameters(env));
      PPVirtualElement sndArgMinint = Methods::apply(thisElement, MinusUnsignedAssignOperation().setConstWithAssign(),
            resultElement, EvaluationApplyParameters(env).stopErrorStates());
      if (sndArgMinint.isValid())
         sndArgMinint = Methods::intersect(sndArgMinint, *minint, EvaluationApplyParameters(env));

      fstArg = fstArgGreaterOrEqualZero;
      if (fstArgLessZero.isValid())
         fstArg = Methods::merge(fstArg, *fstArgLessZero, EvaluationApplyParameters(env));
      if (fstArgMinint.isValid())
         fstArg = Methods::merge(fstArg, *fstArgMinint, EvaluationApplyParameters(env));
      sndArg = sndArgGreaterOrEqualZero;
      if (sndArgLessZero.isValid())
         sndArg = Methods::merge(sndArg, *sndArgLessZero, EvaluationApplyParameters(env));
      if (sndArgMinint.isValid())
         sndArg = Methods::merge(sndArg, *sndArgMinint, EvaluationApplyParameters(env));
   };

   IntersectEnvironment thisIntersectionEnv(env, Argument().setResult()),
                        sourceIntersectionEnv(env, Argument().setFst());
   thisElement.intersectWith(*fstArg, thisIntersectionEnv);
   sourceElement.intersectWith(*sndArg, sourceIntersectionEnv);
   return true;
}

bool
BaseAlgorithms::constraintPlusSignedAssign(VirtualElement& thisElement, const VirtualOperation& operation,
      VirtualElement& sourceElement, const VirtualElement& resultElement, ConstraintEnvironment& env) {
   return Details::BaseAlgorithms::constraintPlusOperationAssign(SignedOperationTraits(), thisElement,
         operation, sourceElement, resultElement, env);
}

bool
BaseAlgorithms::constraintPlusFloatAssign(VirtualElement& thisElement, const VirtualOperation& operation,
      VirtualElement& sourceElement, const VirtualElement& resultElement, ConstraintEnvironment& env) {
   return Details::BaseAlgorithms::constraintPlusOperationAssign(FloatOperationTraits(), thisElement,
         operation, sourceElement, resultElement, env);
}

bool
BaseAlgorithms::constraintTimesSignedAssign(VirtualElement& thisElement, const VirtualOperation& operation,
      VirtualElement& sourceElement, const VirtualElement& resultElement, ConstraintEnvironment& env) {
   return Details::BaseAlgorithms::constraintTimesOperationAssign(SignedOperationTraits(), thisElement,
         operation, sourceElement, resultElement, env);
}

bool
BaseAlgorithms::constraintTimesUnsignedAssign(VirtualElement& thisElement, const VirtualOperation& operation,
      VirtualElement& sourceElement, const VirtualElement& resultElement, ConstraintEnvironment& env) {
   return Details::BaseAlgorithms::constraintTimesOperationAssign(UnsignedOperationTraits(), thisElement,
         operation, sourceElement, resultElement, env);
}

bool
BaseAlgorithms::constraintDivideSignedAssign(VirtualElement& thisElement, const VirtualOperation& operation,
      VirtualElement& sourceElement, const VirtualElement& resultElement, ConstraintEnvironment& env) {
   return Details::BaseAlgorithms::constraintDivideOperationAssign(SignedOperationTraits(), thisElement,
         operation, sourceElement, resultElement, env);
}

bool
BaseAlgorithms::constraintDivideUnsignedAssign(VirtualElement& thisElement, const VirtualOperation& operation,
      VirtualElement& sourceElement, const VirtualElement& resultElement, ConstraintEnvironment& env) {
   return Details::BaseAlgorithms::constraintDivideOperationAssign(UnsignedOperationTraits(), thisElement,
         operation, sourceElement, resultElement, env);
}

bool
BaseAlgorithms::constraintModuloSignedAssign(VirtualElement& thisElement, const VirtualOperation& operation,
      VirtualElement& sourceElement, const VirtualElement& resultElement, ConstraintEnvironment& env) {
   return Details::BaseAlgorithms::constraintModuloOperationAssign(SignedOperationTraits(), thisElement,
         operation, sourceElement, resultElement, env);
}

bool
BaseAlgorithms::constraintModuloUnsignedAssign(VirtualElement& thisElement, const VirtualOperation& operation,
      VirtualElement& sourceElement, const VirtualElement& resultElement, ConstraintEnvironment& env) {
   return Details::BaseAlgorithms::constraintModuloOperationAssign(UnsignedOperationTraits(), thisElement,
         operation, sourceElement, resultElement, env);
}

bool
BaseAlgorithms::queryInverseCastOperation(const VirtualElement& element, const VirtualQueryOperation&, VirtualQueryOperation::Environment& aenv) {
   QueryOperation::InverseCastOperationEnvironment& env = (QueryOperation::InverseCastOperationEnvironment&) aenv;
   switch (((const Operation&) env.reference()).getType()) {
      case Operation::TCastChar: case Operation::TCastInt: case Operation::TCastUnsignedInt:
         {  Scalar::Integer::CastMultiBitOperation* result = new Scalar::Integer::CastMultiBitOperation();
            env.absorbResult(result);
            result->setSize(element.getSizeInBits());
         };
         break;
      case Operation::TCastMultiBit:
         {  Scalar::MultiBit::CastMultiBitOperation* result = new Scalar::MultiBit::CastMultiBitOperation();
            env.absorbResult(result);
            result->setSize(element.getSizeInBits());
         };
         break;
      case Operation::TCastBit: case Operation::TCastShiftBit: // more complex
         {  Scalar::Bit::CastMultiBitOperation* result = new Scalar::Bit::CastMultiBitOperation();
            env.absorbResult(result);
            result->setSize(element.getSizeInBits());
         };
         break;
      case Operation::TCastMultiFloat:
         {  Scalar::Floating::CastMultiBitOperation* result = new Scalar::Floating::CastMultiBitOperation();
            env.absorbResult(result);
            result->setSize(element.getSizeInBits());
         };
         break;
      case Operation::TCastMultiFloatPointer:
         {  Scalar::Floating::CastMultiBitOperationPointer* result = new Scalar::Floating::CastMultiBitOperationPointer();
            env.absorbResult(result);
            result->setSize(element.getSizeInBits());
         };
         break;
      case Operation::TCastReal:
         {  Scalar::Real::CastMultiBitOperation* result = new Scalar::Real::CastMultiBitOperation();
            env.absorbResult(result);
            result->setSize(element.getSizeInBits());
         };
         break;
      case Operation::TCastRational:
         {  Scalar::Rational::CastMultiBitOperation* result = new Scalar::Rational::CastMultiBitOperation();
            env.absorbResult(result);
            result->setSize(element.getSizeInBits());
         };
         break;
      default: { AssumeUncalled }
   };
   return true;
}

#define DefineOperation(TypeOperation)                                                                            \
bool                                                                                                              \
BaseAlgorithms::query##TypeOperation##Operation(const VirtualElement&, const VirtualQueryOperation&, VirtualQueryOperation::Environment& aenv) {\
   QueryOperation::OperationEnvironment& env = (QueryOperation::OperationEnvironment&) aenv;                      \
   env.resultAsMultiBit().set##TypeOperation();                                                                   \
   return true;                                                                                                   \
}

#define DefineOOperation(TypeOperation, TypeMethod)                                                               \
bool                                                                                                              \
BaseAlgorithms::query##TypeOperation##Operation(const VirtualElement&, const VirtualQueryOperation&, VirtualQueryOperation::Environment& aenv) {\
   QueryOperation::OperationEnvironment& env = (QueryOperation::OperationEnvironment&) aenv;                      \
   env.resultAsMultiBit().set##TypeMethod();                                                                      \
   return true;                                                                                                   \
}

bool
BaseAlgorithms::queryNextAssignOperation(const VirtualElement&, const VirtualQueryOperation&, VirtualQueryOperation::Environment& aenv) {
   QueryOperation::OperationEnvironment& env = (QueryOperation::OperationEnvironment&) aenv;
   if (env.hasReference()) {
      Scalar::MultiBit::Operation::Type type = env.referenceAsMultiBit().getType();
      if (type == Scalar::MultiBit::Operation::TPrevSignedAssign || type == Scalar::MultiBit::Operation::TNextSignedAssign)
         env.resultAsMultiBit().setNextSignedAssign();
      else
         env.resultAsMultiBit().setNextUnsignedAssign();
   }
   else
      env.resultAsMultiBit().setNextUnsignedAssign();
   return true;
}

bool
BaseAlgorithms::queryPrevAssignOperation(const VirtualElement&, const VirtualQueryOperation&, VirtualQueryOperation::Environment& aenv) {
   QueryOperation::OperationEnvironment& env = (QueryOperation::OperationEnvironment&) aenv;
   if (env.hasReference()) {
      Scalar::MultiBit::Operation::Type type = env.referenceAsMultiBit().getType();
      if (type == Scalar::MultiBit::Operation::TPrevSignedAssign || type == Scalar::MultiBit::Operation::TNextSignedAssign)
         env.resultAsMultiBit().setPrevSignedAssign();
      else
         env.resultAsMultiBit().setPrevUnsignedAssign();
   }
   else
      env.resultAsMultiBit().setPrevUnsignedAssign();
   return true;
}


DefineOOperation(NextSignedAssign, NextSignedAssign)
DefineOOperation(NextUnsignedAssign, NextUnsignedAssign)
DefineOOperation(PrevSignedAssign, PrevSignedAssign)
DefineOOperation(PrevUnsignedAssign, PrevUnsignedAssign)
DefineOperation(MinusUnsigned)
DefineOperation(MinusSignedAssign)
DefineOperation(MinusUnsignedAssign)
DefineOperation(PlusSigned)
DefineOperation(PlusUnsigned)
DefineOperation(PlusSignedAssign)
DefineOperation(PlusUnsignedAssign)
DefineOperation(DivideSigned)
DefineOperation(DivideUnsigned)
DefineOperation(DivideSignedAssign)
DefineOperation(DivideUnsignedAssign)
DefineOperation(TimesSigned)
DefineOperation(TimesUnsigned)
DefineOperation(TimesSignedAssign)
DefineOperation(TimesUnsignedAssign)
DefineOperation(ModuloSigned)
DefineOperation(ModuloUnsigned)
DefineOperation(ModuloSignedAssign)
DefineOperation(ModuloUnsignedAssign)
DefineOOperation(RightShiftSigned, ArithmeticRightShift)
DefineOOperation(RightShiftUnsigned, LogicalRightShift)
DefineOOperation(RightShiftSignedAssign, ArithmeticRightShiftAssign)
DefineOOperation(RightShiftUnsignedAssign, LogicalRightShiftAssign)
DefineOperation(MinSigned)
DefineOperation(MinUnsigned)
DefineOperation(MinSignedAssign)
DefineOperation(MinUnsignedAssign)
DefineOperation(MaxSigned)
DefineOperation(MaxUnsigned)
DefineOperation(MaxSignedAssign)
DefineOperation(MaxUnsignedAssign)
DefineOperation(CompareGreaterOrEqualSigned)
DefineOperation(CompareGreaterOrEqualUnsigned)
DefineOperation(CompareGreaterSigned)
DefineOperation(CompareGreaterUnsigned)
DefineOperation(CompareLessOrEqualSigned)
DefineOperation(CompareLessOrEqualUnsigned)
DefineOperation(CompareLessSigned)
DefineOperation(CompareLessUnsigned)

bool
BaseAlgorithms::queryMinusAssignOperation(const VirtualElement&, const VirtualQueryOperation&, VirtualQueryOperation::Environment& aenv) {
   QueryOperation::OperationEnvironment& env = (QueryOperation::OperationEnvironment&) aenv;
   if (env.hasReference()) {
      Scalar::MultiBit::Operation::Type type = env.referenceAsMultiBit().getType();
      if (type == Scalar::MultiBit::Operation::TPlusSignedAssign || type == Scalar::MultiBit::Operation::TMinusSignedAssign)
         env.resultAsMultiBit().setMinusSignedAssign();
      else
         env.resultAsMultiBit().setMinusUnsignedAssign();
   }
   else
      env.resultAsMultiBit().setMinusUnsignedAssign();
   return true;
}

DefineOperation(MinusSigned)

bool
BaseAlgorithms::queryMinusOperation(const VirtualElement&, const VirtualQueryOperation&, VirtualQueryOperation::Environment& aenv) {
   QueryOperation::OperationEnvironment& env = (QueryOperation::OperationEnvironment&) aenv;
   if (env.hasReference()) {
      Scalar::MultiBit::Operation::Type type = env.referenceAsMultiBit().getType();
      if (type == Scalar::MultiBit::Operation::TPlusSignedAssign || type == Scalar::MultiBit::Operation::TMinusSignedAssign)
         env.resultAsMultiBit().setMinusSigned();
      else
         env.resultAsMultiBit().setMinusUnsigned();
   }
   else
      env.resultAsMultiBit().setMinusUnsigned();
   return true;
}

bool
BaseAlgorithms::queryPlusAssignOperation(const VirtualElement&, const VirtualQueryOperation&, VirtualQueryOperation::Environment& aenv) {
   QueryOperation::OperationEnvironment& env = (QueryOperation::OperationEnvironment&) aenv;
   if (env.hasReference()) {
      Scalar::MultiBit::Operation::Type type = env.referenceAsMultiBit().getType();
      if (type == Scalar::MultiBit::Operation::TPlusSignedAssign || type == Scalar::MultiBit::Operation::TMinusSignedAssign)
         env.resultAsMultiBit().setPlusSignedAssign();
      else
         env.resultAsMultiBit().setPlusUnsignedAssign();
   }
   else
      env.resultAsMultiBit().setPlusUnsignedAssign();
   return true;
}

bool
BaseAlgorithms::queryPlusOperation(const VirtualElement&, const VirtualQueryOperation&, VirtualQueryOperation::Environment& aenv) {
   QueryOperation::OperationEnvironment& env = (QueryOperation::OperationEnvironment&) aenv;
   if (env.hasReference()) {
      Scalar::MultiBit::Operation::Type type = env.referenceAsMultiBit().getType();
      if (type == Scalar::MultiBit::Operation::TPlusSignedAssign || type == Scalar::MultiBit::Operation::TMinusSignedAssign)
         env.resultAsMultiBit().setPlusSigned();
      else
         env.resultAsMultiBit().setPlusUnsigned();
   }
   else
      env.resultAsMultiBit().setPlusUnsigned();
   return true;
}

bool
BaseAlgorithms::queryDivideAssignOperation(const VirtualElement&, const VirtualQueryOperation&, VirtualQueryOperation::Environment& aenv) {
   QueryOperation::OperationEnvironment& env = (QueryOperation::OperationEnvironment&) aenv;
   if (env.hasReference()) {
      Scalar::MultiBit::Operation::Type type = env.referenceAsMultiBit().getType();
      if (type == Scalar::MultiBit::Operation::TTimesSignedAssign || type == Scalar::MultiBit::Operation::TDivideSignedAssign)
         env.resultAsMultiBit().setDivideSignedAssign();
      else
         env.resultAsMultiBit().setDivideUnsignedAssign();
   }
   else
      env.resultAsMultiBit().setDivideUnsignedAssign();
   return true;
}

bool
BaseAlgorithms::queryDivideOperation(const VirtualElement&, const VirtualQueryOperation&, VirtualQueryOperation::Environment& aenv) {
   QueryOperation::OperationEnvironment& env = (QueryOperation::OperationEnvironment&) aenv;
   if (env.hasReference()) {
      Scalar::MultiBit::Operation::Type type = env.referenceAsMultiBit().getType();
      if (type == Scalar::MultiBit::Operation::TTimesSignedAssign || type == Scalar::MultiBit::Operation::TDivideSignedAssign)
         env.resultAsMultiBit().setDivideSigned();
      else
         env.resultAsMultiBit().setDivideUnsigned();
   }
   else
      env.resultAsMultiBit().setDivideUnsigned();
   return true;
}

bool
BaseAlgorithms::queryTimesAssignOperation(const VirtualElement&, const VirtualQueryOperation&, VirtualQueryOperation::Environment& aenv) {
   QueryOperation::OperationEnvironment& env = (QueryOperation::OperationEnvironment&) aenv;
   if (env.hasReference()) {
      Scalar::MultiBit::Operation::Type type = env.referenceAsMultiBit().getType();
      if (type == Scalar::MultiBit::Operation::TTimesSignedAssign || type == Scalar::MultiBit::Operation::TDivideSignedAssign)
         env.resultAsMultiBit().setTimesSignedAssign();
      else
         env.resultAsMultiBit().setTimesUnsignedAssign();
   }
   else
      env.resultAsMultiBit().setTimesUnsignedAssign();
   return true;
}

bool
BaseAlgorithms::queryTimesOperation(const VirtualElement&, const VirtualQueryOperation&, VirtualQueryOperation::Environment& aenv) {
   QueryOperation::OperationEnvironment& env = (QueryOperation::OperationEnvironment&) aenv;
   if (env.hasReference()) {
      Scalar::MultiBit::Operation::Type type = env.referenceAsMultiBit().getType();
      if (type == Scalar::MultiBit::Operation::TTimesSignedAssign || type == Scalar::MultiBit::Operation::TDivideSignedAssign)
         env.resultAsMultiBit().setTimesSigned();
      else
         env.resultAsMultiBit().setTimesUnsigned();
   }
   else
      env.resultAsMultiBit().setTimesUnsigned();
   return true;
}

bool
BaseAlgorithms::queryModuloAssignOperation(const VirtualElement&, const VirtualQueryOperation&, VirtualQueryOperation::Environment& aenv) {
   QueryOperation::OperationEnvironment& env = (QueryOperation::OperationEnvironment&) aenv;
   if (env.hasReference()) {
      Scalar::MultiBit::Operation::Type type = env.referenceAsMultiBit().getType();
      if (type == Scalar::MultiBit::Operation::TTimesSignedAssign || type == Scalar::MultiBit::Operation::TDivideSignedAssign
            || type == Scalar::MultiBit::Operation::TModuloSignedAssign)
         env.resultAsMultiBit().setModuloSignedAssign();
      else
         env.resultAsMultiBit().setModuloUnsignedAssign();
   }
   else
      env.resultAsMultiBit().setModuloUnsignedAssign();
   return true;
}

bool
BaseAlgorithms::queryModuloOperation(const VirtualElement&, const VirtualQueryOperation&, VirtualQueryOperation::Environment& aenv) {
   QueryOperation::OperationEnvironment& env = (QueryOperation::OperationEnvironment&) aenv;
   if (env.hasReference()) {
      Scalar::MultiBit::Operation::Type type = env.referenceAsMultiBit().getType();
      if (type == Scalar::MultiBit::Operation::TTimesSignedAssign || type == Scalar::MultiBit::Operation::TDivideSignedAssign
            || type == Scalar::MultiBit::Operation::TModuloSignedAssign)
         env.resultAsMultiBit().setModuloSigned();
      else
         env.resultAsMultiBit().setModuloUnsigned();
   }
   else
      env.resultAsMultiBit().setModuloUnsigned();
   return true;
}

DefineOperation(LeftShift)
DefineOperation(LeftShiftAssign)
DefineOOperation(RightShift, LogicalRightShift)
DefineOOperation(RightShiftAssign, LogicalRightShiftAssign)

bool
BaseAlgorithms::queryInverseRotateAssignOperation(const VirtualElement&, const VirtualQueryOperation&, VirtualQueryOperation::Environment& aenv) {
   VirtualQueryOperation::ExtendedOperationEnvironment& env = (VirtualQueryOperation::ExtendedOperationEnvironment&) aenv;
   AssumeCondition((((const Operation&) env.reference()).getType() == Operation::TLeftRotateAssign)
      || (((const Operation&) env.reference()).getType() == Operation::TRightRotateAssign))
   Operation inverseOperation;
   if (((const Operation&) env.reference()).getType() == Operation::TLeftRotateAssign)
      inverseOperation.setRightRotateAssign();
   else
      inverseOperation.setLeftRotateAssign();
   env.result() = inverseOperation;
   return true;
}

DefineOperation(BitNegate)
DefineOperation(BitNegateAssign)
DefineOperation(BitOrAssign)
DefineOperation(BitOr)
DefineOperation(BitAndAssign)
DefineOperation(BitAnd)
DefineOperation(BitExclusiveOrAssign)
DefineOperation(BitExclusiveOr)

bool
BaseAlgorithms::queryOppositeAssignOperation(const VirtualElement&, const VirtualQueryOperation&, VirtualQueryOperation::Environment& aenv) {
   QueryOperation::OperationEnvironment& env = (QueryOperation::OperationEnvironment&) aenv;
   env.resultAsMultiBit().setOppositeSignedAssign();
   return true;
}

bool
BaseAlgorithms::queryOppositeOperation(const VirtualElement&, const VirtualQueryOperation&, VirtualQueryOperation::Environment& aenv) {
   QueryOperation::OperationEnvironment& env = (QueryOperation::OperationEnvironment&) aenv;
   env.resultAsMultiBit().setOppositeSigned();
   return true;
}

DefineOOperation(MinAssign, MinUnsignedAssign)
DefineOOperation(Min, MinUnsigned)
DefineOOperation(MaxAssign, MaxUnsignedAssign)
DefineOOperation(Max, MaxUnsigned)
DefineOOperation(CompareGreaterOrEqual, CompareGreaterOrEqualUnsigned)
DefineOOperation(CompareGreater, CompareGreaterUnsigned)
DefineOOperation(CompareLessOrEqual, CompareLessOrEqualUnsigned)
DefineOOperation(CompareLess, CompareLessUnsigned)
DefineOperation(CompareEqual)
DefineOperation(CompareDifferent)

#undef DefineOperation
#undef DefineOOperation

}}} // end of namespace MultiBit::Approximate::Details

namespace Integer { namespace Approximate { namespace Details {

using namespace Scalar::Approximate::Details;

#define DefineOperation(TypeOperation)                                                                            \
bool                                                                                                              \
BaseAlgorithms::query##TypeOperation##Operation(const VirtualElement&, const VirtualQueryOperation&, VirtualQueryOperation::Environment& aenv) {\
   QueryOperation::OperationEnvironment& env = (QueryOperation::OperationEnvironment&) aenv;                      \
   env.resultAsInteger().set##TypeOperation();                                                                    \
   return true;                                                                                                   \
}

DefineOperation(NextAssign)
DefineOperation(PrevAssign)
DefineOperation(Minus)
DefineOperation(MinusSigned)
DefineOperation(MinusAssign)
DefineOperation(Plus)
DefineOperation(PlusAssign)
DefineOperation(Divide)
DefineOperation(DivideAssign)
DefineOperation(Times)
DefineOperation(TimesAssign)
DefineOperation(ModuloAssign)
DefineOperation(Modulo)
DefineOperation(LeftShift)
DefineOperation(LeftShiftAssign)
DefineOperation(RightShift)
DefineOperation(RightShiftAssign)

bool
BaseAlgorithms::queryInverseRotateAssignOperation(const VirtualElement&, const VirtualQueryOperation&, VirtualQueryOperation::Environment& aenv) {
   VirtualQueryOperation::ExtendedOperationEnvironment& env = (VirtualQueryOperation::ExtendedOperationEnvironment&) aenv;
   AssumeCondition((((const Operation&) env.reference()).getType() == Operation::TLeftRotateAssign)
      || (((const Operation&) env.reference()).getType() == Operation::TRightRotateAssign))
   Operation inverseOperation;
   if (((const Operation&) env.reference()).getType() == Operation::TLeftRotateAssign)
      inverseOperation.setRightRotateAssign();
   else
      inverseOperation.setLeftRotateAssign();
   env.result() = inverseOperation;
   return true;
}

DefineOperation(BitNegate)
DefineOperation(BitNegateAssign)
DefineOperation(BitOrAssign)
DefineOperation(BitOr)
DefineOperation(BitAndAssign)
DefineOperation(BitAnd)
DefineOperation(BitExclusiveOrAssign)
DefineOperation(BitExclusiveOr)
DefineOperation(Opposite)
DefineOperation(OppositeAssign)
DefineOperation(MinAssign)
DefineOperation(Min)
DefineOperation(MaxAssign)
DefineOperation(Max)
DefineOperation(CompareGreaterOrEqual)
DefineOperation(CompareGreater)
DefineOperation(CompareLessOrEqual)
DefineOperation(CompareLess)
DefineOperation(CompareEqual)
DefineOperation(CompareDifferent)

#undef DefineOperation

}}} // end of namespace Integer::Approximate::Details

namespace Floating { namespace Approximate { namespace Details {

using namespace Scalar::Approximate::Details;

#define DefineOperation(TypeOperation)                                                                            \
bool                                                                                                              \
BaseAlgorithms::query##TypeOperation##Operation(const VirtualElement&, const VirtualQueryOperation&, VirtualQueryOperation::Environment& aenv) {\
   QueryOperation::OperationEnvironment& env = (QueryOperation::OperationEnvironment&) aenv;                      \
   env.resultAsFloating().set##TypeOperation();                                                                   \
   return true;                                                                                                   \
}

DefineOperation(Minus)

bool
BaseAlgorithms::queryMinusSignedOperation(const VirtualElement&, const VirtualQueryOperation&, VirtualQueryOperation::Environment& aenv) {
   QueryOperation::OperationEnvironment& env = (QueryOperation::OperationEnvironment&) aenv;
   env.resultAsFloating().setMinus();
   return true;
}

DefineOperation(MinusAssign)
DefineOperation(CompareLess)
DefineOperation(CompareLessOrEqual)
DefineOperation(CompareEqual)
DefineOperation(CompareGreaterOrEqual)
DefineOperation(CompareGreater)

#undef DefineOperation

}}} // end of namespace Floating::Approximate::Details

}} // end of namespace Analyzer::Scalar

template int COL::ImplArray::localize<COL::VirtualSortedCollection::TemplateElementEnhancedKeyCastParameters<Analyzer::Scalar::Approximate::DEnvironment::ElementResult, Analyzer::Scalar::Approximate::DEnvironment::ElementResult::Key, HandlerCopyCast<Analyzer::Scalar::Approximate::DEnvironment::ElementResult> > >(COL::VirtualSortedCollection::TemplateElementEnhancedKeyCastParameters<Analyzer::Scalar::Approximate::DEnvironment::ElementResult, Analyzer::Scalar::Approximate::DEnvironment::ElementResult::Key, HandlerCopyCast<Analyzer::Scalar::Approximate::DEnvironment::ElementResult> >::Key::KeyType, COL::VirtualSortedCollection::TemplateElementEnhancedKeyCastParameters<Analyzer::Scalar::Approximate::DEnvironment::ElementResult, Analyzer::Scalar::Approximate::DEnvironment::ElementResult::Key, HandlerCopyCast<Analyzer::Scalar::Approximate::DEnvironment::ElementResult> > const&, int, int) const;
template int COL::ImplArray::merge<COL::TSortedArray<Analyzer::Scalar::Approximate::DEnvironment::ElementResult, Analyzer::Scalar::Approximate::DEnvironment::ElementResult::Key, HandlerCopyCast<Analyzer::Scalar::Approximate::DEnvironment::ElementResult> >::GenericLocateParameters>(COL::TSortedArray<Analyzer::Scalar::Approximate::DEnvironment::ElementResult, Analyzer::Scalar::Approximate::DEnvironment::ElementResult::Key, HandlerCopyCast<Analyzer::Scalar::Approximate::DEnvironment::ElementResult> >::GenericLocateParameters const&, COL::ImplArray const&, int, int, bool, VirtualCast const*);
