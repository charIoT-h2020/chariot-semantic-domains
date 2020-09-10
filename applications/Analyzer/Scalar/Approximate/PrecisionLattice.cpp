/////////////////////////////////
//
// Library   : Analyzer/Scalar
// Unit      : precision lattice
// File      : PrecisionLattice.cpp
// Author    : Franck Vedrine
// Copyright : CEA LIST - 2019-2020 - all rights reserved
//
// Description :
//   Implementation of a lattice for the precision requirements.
//   It is exclusively used in backward analysis.
//

#include "Analyzer/Scalar/Approximate/PrecisionLattice.h"
#include "Analyzer/Scalar/Approximate/General/ConstantMultiBitShare.h"
#include "Analyzer/Scalar/Approximate/Guard.h"

// #include "Analyzer/Interpretation/Scalar/Scalar.hpp"

namespace Analyzer { namespace Scalar { namespace Approximate {

PrecisionLattice::Required
PrecisionLattice::queryRequired(const VirtualElement& concrete, TypeOperation typeOperation, bool* isComplete) {
   switch (concrete.getApproxKind().type()) {
      case ApproxKind::TConstant: case ApproxKind::TCompilationConstant:
         if (isComplete) *isComplete = true;
         return RConstant;
      case ApproxKind::TVariable:
         AssumeCondition(dynamic_cast<const VirtualExactToApproximateElement*>(&concrete))
         return queryRequired(((const VirtualExactToApproximateElement&) concrete).domain(), typeOperation, isComplete);
      case ApproxKind::TFormalOperation:
         {  const FormalImplementation& formalConcrete = concrete.asFormal();
            bool isLocalComplete = false;
            unsigned int requiredField = queryRequired(formalConcrete.fstArg(), typeOperation, &isLocalComplete);
            
            if (formalConcrete.hasSndArg()) {
               bool isTempComplete = false;
               requiredField = requiredField | queryRequired(formalConcrete.sndArg(), typeOperation, &isTempComplete);
               isLocalComplete = isLocalComplete || isTempComplete;
            };
            if (isComplete) *isComplete = isLocalComplete;
            return (Required) requiredField;
         };
      case ApproxKind::TGuard:
         {  AssumeCondition(dynamic_cast<const Guard*>(&concrete))
            const Guard& guardConcrete = (const Guard&) concrete;
            bool isLocalComplete = true;
            unsigned int requiredField = 0;
            if (guardConcrete.hasThen()) {
               isLocalComplete = false;
               requiredField = queryRequired(guardConcrete.getThen(), typeOperation, &isLocalComplete);
            }
            if (guardConcrete.hasElse()) {
               bool isTempComplete = false;
               requiredField = requiredField | queryRequired(guardConcrete.getElse(), typeOperation, &isTempComplete);
               isLocalComplete = isLocalComplete || isTempComplete;
            };
            if (isComplete) *isComplete = isLocalComplete;
            return (Required) requiredField;
         };
      case ApproxKind::TPointer:
         if (isComplete) *isComplete = true;
         return RConstant;
      case ApproxKind::TUnprecise:
         return RTop;
      case ApproxKind::TInterval:
         AssumeCondition(dynamic_cast<const Approximate::Details::VirtualIntegerInterval*>(&concrete))
         if (isComplete) *isComplete = true;
         {  if (typeOperation == TOInteger || typeOperation == TOMultiBit) {
               const Approximate::Details::VirtualIntegerInterval& vieInterval = (const Approximate::Details::VirtualIntegerInterval&) concrete;
               if ((vieInterval.getMin().getApproxKind().isConstant())
                     && (vieInterval.getMax().getApproxKind().isConstant()))
                  return RIntervalWithConstantBounds;
            };
         };
         return RIntervalWithExactBounds;
      case ApproxKind::TDisjunction:
         if (isComplete) *isComplete = true;
         AssumeCondition(dynamic_cast<const Disjunction*>(&concrete))
         {  Required result = RUndefined;
            Disjunction::ElementsList::Cursor exactCursor(((const Disjunction&) concrete).exacts());
            while (exactCursor.setToNext())
               result = (Required) (result | queryRequired(exactCursor.elementAt(), typeOperation, NULL));
            Disjunction::ElementsList::Cursor mayCursor(((const Disjunction&) concrete).mays());
            while (mayCursor.setToNext())
               result = (Required) (result | queryRequired(mayCursor.elementAt(), typeOperation, NULL));
            if (result == RConstant) {
               if (concrete.countAtomic() > 1)
                  result = RConstantDisjunction;
            }
            else if (result == RExact) {
               if (concrete.countAtomic() > 1)
                  result = RExactDisjunction;
            };
            return result;
         };
      case ApproxKind::TConjunction: { AssumeUnimplemented }
      case ApproxKind::TTop:
         if (isComplete) *isComplete = true;
         return RTop;
      default:
         {  AssumeUncalled }
   };
   if (isComplete) *isComplete = true;
   return RTop;
}
   
void
PrecisionLattice::_setFromDomain(const VirtualElement& concreteElement) {
   AssumeCondition(!hasRequiredField())
   switch (Methods::queryTypeOperation(concreteElement)) {
      case ExternNativeOperationEnvironment::TOBit: setTypeOperation(TOBit); break;
      case ExternNativeOperationEnvironment::TOInteger: setTypeOperation(TOInteger); break;
      case ExternNativeOperationEnvironment::TOMultiBit: setTypeOperation(TOMultiBit); break;
      case ExternNativeOperationEnvironment::TOFloating: setTypeOperation(TOFloating); break;
      case ExternNativeOperationEnvironment::TORational: setTypeOperation(TORational); break;
      case ExternNativeOperationEnvironment::TOReal: setTypeOperation(TOReal); break;
      default:
         {  AssumeUncalled }
   };
   bool isComplete = false;
   mergeRequiredField(queryRequired(concreteElement, getTypeOperation(), &isComplete));
   if (isComplete) mergeCompleteField(1);
/* switch (concreteElement.getApproxKind().type()) {
      case ApproxKind::TConstant: case ApproxKind::TCompilationConstant:
         mergeRequiredField(RConstant);
         mergeCompleteField(1);
         break;
      case ApproxKind::TVariable:
         AssumeCondition(dynamic_cast<const VirtualExactToApproximateElement*>(&concreteElement))
         _setFromDomain(((const VirtualExactToApproximateElement&) concreteElement).domain());
         // mergeRequiredField(RExact);
         // mergeCompleteField(1);
         break;
      case ApproxKind::TFormalOperation:
         {  const FormalImplementation& concrete = concreteElement.asFormal();
            PrecisionLattice fstArg(Init().setBitSize(concrete.fstArg().getSizeInBits()));
            fstArg._setFromDomain(concrete.fstArg());
            PNT::TPassPointer<PrecisionLattice, PPAbstractElement> sndArg;
            EvaluationEnvironment applyEnv(EPExactStopErrorStates);
            if (concrete.hasSndArg()) {
               sndArg.absorbElement(new PrecisionLattice(Init().setBitSize(concrete.sndArg().getSizeInBits())));
               sndArg->_setFromDomain(concrete.sndArg());
               applyEnv.setFirstArgument(*sndArg);
            };
            assume(fstArg.apply(concrete.getOperation(), applyEnv));
            AssumeCondition(!applyEnv.hasResult())
            setLatticeField(fstArg.queryLatticeField());
            setTypeOperationField(fstArg.queryTypeOperationField());
         };
         return;
      case ApproxKind::TPointer:
         mergeRequiredField(RConstant);
         mergeCompleteField(1);
         break;
      case ApproxKind::TUnprecise:
         mergeRequiredField(RTop);
         break;
      case ApproxKind::TInterval:
         AssumeCondition(dynamic_cast<const Scalar::Approximate::Details::VirtualIntegerInterval*>(&concreteElement))
         {  TypeOperation typeOperation = getTypeOperation();
            if (typeOperation == TOInteger || typeOperation == TOMultiBit) {
               const auto& interval = (const Scalar::Approximate::Details::VirtualIntegerInterval&) concreteElement;
               if ((interval.getMin().getApproxKind().isConstant())
                     && (interval.getMax().getApproxKind().isConstant()))
                  mergeRequiredField(RIntervalWithConstantBounds);
            };
            if (!hasRequiredField())
               mergeRequiredField(RIntervalWithExactBounds);
         }
         mergeCompleteField(1);
         break;
      case ApproxKind::TDisjunction:
         if (concreteElement.countAtomic() != 1)
            mergeRequiredField(RConstantDisjunction);
         else
            mergeRequiredField(RConstant);
         mergeCompleteField(1);
         break;
      case ApproxKind::TConjunction: { AssumeUnimplemented }
      case ApproxKind::TTop:
         mergeRequiredField(RTop);
         mergeCompleteField(1);
         break;
      default:
         {  AssumeUncalled }
   }; */
}

bool
PrecisionLattice::constraint(const VirtualOperation& aoperation, const VirtualElement& aresult, ConstraintEnvironment& env) {
   AssumeCondition(dynamic_cast<const PrecisionLattice*>(&aresult))
   const PrecisionLattice& result = (const PrecisionLattice&) aresult;
   int type = queryTypeOperationField();
   if (type == TOUndefined) {
      AssumeCondition(dynamic_cast<const PrecisionLattice*>(&env.getFirstArgument()))
      type = ((const PrecisionLattice&) env.getFirstArgument()).queryTypeOperationField();
   };
   switch (type) {
      case TOBit:
         {  const Scalar::Bit::Operation& operation = (const Scalar::Bit::Operation&) aoperation;
            int latticeField = (result.queryRequiredField() <= RExact)
               ? result.queryRequiredField() : RTop;
            if (result.queryCompleteField())
               latticeField |= 1U << (END_Required_OF_MASK - START_Required_OF_MASK);
            bool isStable = ((queryLatticeField() & latticeField) == queryLatticeField());
            if ((operation.getArgumentsNumber() == 0)) {
               if (!isStable) {
                  setLatticeField(latticeField);
                  env.setUnstable();
               };
            }
            else if ((operation.getArgumentsNumber() == 1)) {
               AssumeCondition(dynamic_cast<const PrecisionLattice*>(&env.getFirstArgument()))
               PrecisionLattice& source = (PrecisionLattice&) env.getFirstArgument();
               bool isSourceStable = (source.queryLatticeField() & latticeField) == source.queryLatticeField();
               if (!isStable) {
                  setLatticeField(latticeField);
                  env.setUnstable();
               };
               if (!isSourceStable) {
                  source.setLatticeField(latticeField);
                  env.setUnstable();
               };
            }
            else
               {  AssumeUncalled }
            break;
         }
      case TOInteger:
         {  const Scalar::Integer::Operation& operation = (const Scalar::Integer::Operation&) aoperation;
            if ((operation.getType() >= Scalar::Integer::Operation::TCastChar)
                  && (operation.getType() < Scalar::Integer::Operation::EndOfUnary)) {
               bool isStable = (queryRequiredField() & result.queryRequiredField()) == queryRequiredField();
               if (!isStable) {
                  setLatticeField(result.queryLatticeField());
                  env.setUnstable();
               };
            }
            else if ((operation.getType() >= Scalar::Integer::Operation::StartOfBinary)
                  && (operation.getType() < Scalar::Integer::Operation::EndOfBinary)) {
               AssumeCondition(dynamic_cast<const PrecisionLattice*>(&env.getFirstArgument()))
               PrecisionLattice& source = (PrecisionLattice&) env.getFirstArgument();
               bool isStable = (queryRequiredField() & result.queryRequiredField()) == queryRequiredField();
               bool isSourceStable = (source.queryRequiredField() & result.queryRequiredField()) == source.queryRequiredField();
               if (!isStable) {
                  setLatticeField(queryLatticeField() & result.queryLatticeField());
                  env.setUnstable();
               };
               if (!isSourceStable) {
                  source.setLatticeField(source.queryLatticeField() & result.queryLatticeField());
                  env.setUnstable();
               };
            }
            else if ((operation.getType() >= Scalar::Integer::Operation::StartOfCompare)
                  && (operation.getType() < Scalar::Integer::Operation::EndOfCompare)) {
               unsigned int latticeField = (result.queryRequiredField() <= RExact)
                  ? result.queryRequiredField() : RTop;
               if (result.queryCompleteField())
                  latticeField |= 1U << (END_Required_OF_MASK - START_Required_OF_MASK);
               PrecisionLattice& source = (PrecisionLattice&) env.getFirstArgument();
               bool isStable = (queryLatticeField() & latticeField) == (unsigned) queryLatticeField();
               bool isSourceStable = (source.queryLatticeField() & latticeField) == (unsigned) source.queryLatticeField();
               if (!isStable) {
                  setLatticeField(latticeField);
                  env.setUnstable();
               };
               if (!isSourceStable) {
                  source.setLatticeField(latticeField);
                  env.setUnstable();
               };
            }
            else if ((operation.getType() == Scalar::Integer::Operation::TMinAssign)
                  && (operation.getType() == Scalar::Integer::Operation::TMaxAssign)) {
               // constant x intervalle -> constant
               // intervalle x constant -> constant
               if (result.queryRequiredField() == RConstant) {

               }
               else if (result.queryRequiredField() == RExact) {

               }
               else { // Partager le code avec le cas précédent
                  unsigned int latticeField = (result.queryRequiredField() <= RExact)
                     ? result.queryRequiredField() : RTop;
                  if (result.queryCompleteField())
                     latticeField |= 1U << (END_Required_OF_MASK - START_Required_OF_MASK);
                  PrecisionLattice& source = (PrecisionLattice&) env.getFirstArgument();
                  bool isStable = (queryLatticeField() & latticeField) == (unsigned) queryLatticeField();
                  bool isSourceStable = (source.queryLatticeField() & latticeField) == (unsigned) source.queryLatticeField();
                  if (!isStable) {
                     setLatticeField(latticeField);
                     env.setUnstable();
                  };
                  if (!isSourceStable) {
                     source.setLatticeField(latticeField);
                     env.setUnstable();
                  };
               };
            }
            break;
         }
      case TOMultiBit:
         {  const Scalar::MultiBit::Operation& operation = (const Scalar::MultiBit::Operation&) aoperation;
            if (((operation.getType() >= Scalar::MultiBit::Operation::StartOfBinary)
						   && (operation.getType() < Scalar::MultiBit::Operation::EndOfBinary))
					   || (operation.getType() == Scalar::MultiBit::Operation::TConcat)
					   || (operation.getType() == Scalar::MultiBit::Operation::TBitSet)
					   || ((operation.getType() >= Scalar::MultiBit::Operation::TTimesSignedAssign)
						   && (operation.getType() <= Scalar::MultiBit::Operation::TDivideFloatAssign))
					   || ((operation.getType() >= Scalar::MultiBit::Operation::TModuloSignedAssign)
                     && (operation.getType() != Scalar::MultiBit::Operation::TBitNegateAssign)
						   && (operation.getType() < Scalar::MultiBit::Operation::EndOfInteger))) {
               AssumeCondition(dynamic_cast<const PrecisionLattice*>(&env.getFirstArgument()))
               PrecisionLattice& source = (PrecisionLattice&) env.getFirstArgument();
               bool isStable = (queryRequiredField() & result.queryRequiredField()) == queryRequiredField();
               bool isSourceStable = (source.queryRequiredField() & result.queryRequiredField()) == source.queryRequiredField();
               if (!isStable) {
                  setLatticeField(queryLatticeField() & result.queryLatticeField());
                  env.setUnstable();
               };
               if (!isSourceStable) {
                  source.setLatticeField(source.queryLatticeField() & result.queryLatticeField());
                  env.setUnstable();
               };
            }
            else if (((operation.getType() >= Scalar::MultiBit::Operation::TExtendWithZero)
							&& (operation.getType() < Scalar::MultiBit::Operation::EndOfUnary))
						|| (operation.getType() == Scalar::MultiBit::Operation::TOppositeSignedAssign)
						|| (operation.getType() == Scalar::MultiBit::Operation::TOppositeFloatAssign)
						|| (operation.getType() == Scalar::MultiBit::Operation::TBitNegateAssign)) {
               bool isStable = (queryLatticeField() & result.queryLatticeField()) == queryLatticeField();
               if (!isStable) {
                  setLatticeField(result.queryLatticeField());
                  env.setUnstable();
               };
            }
            else if ((operation.getType() >= Scalar::MultiBit::Operation::StartOfCompare)
                  && (operation.getType() < Scalar::MultiBit::Operation::EndOfCompare)) {
               unsigned int latticeField = (result.queryRequiredField() <= RIntervalWithExactBounds) /* RExact */
                  ? result.queryRequiredField() : RTop;
               if (result.queryCompleteField())
                  latticeField |= 1U << (END_Required_OF_MASK - START_Required_OF_MASK);
               PrecisionLattice& source = (PrecisionLattice&) env.getFirstArgument();
               bool isStable = (queryLatticeField() & latticeField) == (unsigned) queryLatticeField();
               bool isSourceStable = (source.queryLatticeField() & latticeField) == (unsigned) source.queryLatticeField();
               if (!isStable) {
                  setLatticeField(latticeField);
                  env.setUnstable();
               };
               if (!isSourceStable) {
                  source.setLatticeField(latticeField);
                  env.setUnstable();
               };
            }
            else if ((operation.getType() >= Scalar::MultiBit::Operation::TMinSignedAssign)
                  && (operation.getType() <= Scalar::MultiBit::Operation::TMaxFloatAssign)) {
               // constant x intervalle -> constant
               // intervalle x constant -> constant
               if (result.queryRequiredField() == RConstant) {

               }
               else if (result.queryRequiredField() == RExact) {

               }
               else { // Partager le code avec le cas précédent
                  unsigned int latticeField = (result.queryRequiredField() <= RExact)
                     ? result.queryRequiredField() : RTop;
                  if (result.queryCompleteField())
                     latticeField |= 1U << (END_Required_OF_MASK - START_Required_OF_MASK);
                  PrecisionLattice& source = (PrecisionLattice&) env.getFirstArgument();
                  bool isStable = (queryLatticeField() & latticeField) == (unsigned) queryLatticeField();
                  bool isSourceStable = (source.queryLatticeField() & latticeField) == (unsigned) source.queryLatticeField();
                  if (!isStable) {
                     setLatticeField(latticeField);
                     env.setUnstable();
                  };
                  if (!isSourceStable) {
                     source.setLatticeField(latticeField);
                     env.setUnstable();
                  };
               };
            }
            break;
         }
      case TOFloating:
         {  // const Scalar::Floating::Operation& operation = (const Scalar::Floating::Operation&) aoperation;
            break;
         }
      case TORational:
         {  // const Scalar::Rational::Operation& operation = (const Scalar::Rational::Operation&) aoperation;
            break;
         }
      case TOReal:
         {  // const Scalar::Real::Operation& operation = (const Scalar::Real::Operation&) aoperation;
            break;
         }
      default:
         AssumeUncalled
   };
   return true;
}

bool
PrecisionLattice::apply(const VirtualOperation& aoperation, Scalar::EvaluationEnvironment& aenv) {
   {  EvaluationEnvironment::GuardLatticeCreation guard((EvaluationEnvironment&) aenv);
      intersectLatticeCreation((EvaluationEnvironment&) aenv, (Required) queryRequiredField());

      switch (aoperation.getArgumentsNumber()) {
         case 0:
            if (!ppveDomain->apply(aoperation, aenv))
               return false;
            break;
         case 1:
            {  const VirtualElement& argument = (const VirtualElement&) aenv.getFirstArgument();
               AssumeCondition(dynamic_cast<const PrecisionLattice*>(&argument))
               aenv.setFirstArgument(*((const PrecisionLattice&) argument).ppveDomain);
               bool fResult = ppveDomain->apply(aoperation, aenv);
               aenv.setFirstArgument(argument);
               if (!fResult)
                  return false;
               break;
            };
         case 2:
            {  const VirtualElement& firstArgument = (const VirtualElement&) aenv.getFirstArgument();
               const VirtualElement& secondArgument = (const VirtualElement&) aenv.getSecondArgument();
               AssumeCondition(dynamic_cast<const PrecisionLattice*>(&firstArgument) && dynamic_cast<const PrecisionLattice*>(&secondArgument))
               aenv.setFirstArgument(*((const PrecisionLattice&) firstArgument).ppveDomain);
               aenv.setSecondArgument(*((const PrecisionLattice&) secondArgument).ppveDomain);
               bool fResult = ppveDomain->apply(aoperation, aenv);
               aenv.setFirstArgument(firstArgument);
               aenv.setSecondArgument(secondArgument);
               if (!fResult)
                  return false;
               break;
            };
         default:
            AssumeUncalled
      };
   };
   if (aenv.hasResult()) {
      if (aoperation.isAssign()) {
         ppveDomain = aenv.presult();
         clearLatticeField();
         clearTypeOperationField();
         _setFromDomain(*ppveDomain);
      }
      else {
         PPVirtualElement result = aenv.presult();
         aenv.presult().setElement(*this);
         ((PrecisionLattice&) *aenv.presult()).clearLatticeField();
         ((PrecisionLattice&) *aenv.presult()).clearTypeOperationField();
         ((PrecisionLattice&) *aenv.presult()).setFromDomain(result);
      };
   }
   else if (aoperation.isAssign()) {
      clearLatticeField();
      clearTypeOperationField();
      _setFromDomain(*ppveDomain);
   };
   return true;
}

bool
PrecisionLattice::mergeWith(const Scalar::VirtualElement& asource, Scalar::EvaluationEnvironment& env) {
   AssumeCondition(dynamic_cast<const PrecisionLattice*>(&asource))
   const auto& source = (const PrecisionLattice&) asource;
   if (ppveDomain.isValid()) {
      if (source.ppveDomain.isValid())
         ppveDomain = Methods::merge(ppveDomain, *source.ppveDomain, (EvaluationEnvironment&) env);
   }
   else if (source.ppveDomain.isValid())
      ppveDomain = source.ppveDomain;
   mergeLatticeField(source.queryLatticeField());
   return true;
}

}}} // end of namespace Analyzer::Scalar::Approximate

