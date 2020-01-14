/////////////////////////////////
//
// Library   : Analyzer/Scalar
// Unit      : approximate scalar elements
// File      : FormalOperation.h
// Author    : Franck Vedrine
// Copyright : CEA LIST - 2019-2020 - all rights reserved
//
// Description :
//   Implementation of a class of formal operations.
//

#include "Analyzer/Scalar/Approximate/FormalOperation.h"
#include "Analyzer/Scalar/Approximate/Top.h"
#include "Analyzer/Scalar/Approximate/General/ConstantMultiBitShare.h"
// #include "Analyzer/Scalar/Scalar.hpp"

namespace Analyzer {}

#include "Analyzer/Scalar/Approximate/VirtualElement.template"

namespace Analyzer { namespace Scalar {

namespace MultiBit { namespace Approximate {

bool
ConcatOperationElement::ConcatenationCursor::setToFirst() {
   acePathTo.deleteAll();
   pveElement = &ceSupport.fstArg();
   while (pveElement->getApproxKind().isFormalOperation()
         && (((const Operation&) ((const TFormalOperationElement<Scalar::Approximate::Details::IntOperationElement>&)
            *pveElement).getOperation()).getType() == Operation::TConcat)) {
      AssumeCondition(dynamic_cast<const ConcatOperationElement*>(pveElement))
      acePathTo.insertAtEnd((ConcatOperationElement*) pveElement);
      pveElement = &((ConcatOperationElement&) *pveElement).fstArg();
   };
   return true;
}

bool
ConcatOperationElement::ConcatenationCursor::setToNext() {
   if (!pveElement)
      return setToFirst();
   bool hasFound = false;
   while (!hasFound && !acePathTo.isEmpty()) {
      if (&acePathTo.getLast().fstArg() == pveElement) {
         hasFound = true;
         pveElement = &acePathTo.getLast().sndArg();
      };
      if (!hasFound) {
         pveElement = &acePathTo.getSLast();
         acePathTo.deleteLast();
      };
   };
   if (!hasFound) {
      if (&ceSupport.fstArg() == pveElement) {
         hasFound = true;
         pveElement = &ceSupport.sndArg();
      }
      else
         pveElement = const_cast<ConcatOperationElement*>(&ceSupport);
   };
   if (hasFound) {
      while (pveElement->getApproxKind().isFormalOperation()
            && (((const Operation&) ((const TFormalOperationElement<Scalar::Approximate::Details::IntOperationElement>&)
               *pveElement).getOperation()).getType() == Operation::TConcat)) {
         AssumeCondition(dynamic_cast<const ConcatOperationElement*>(pveElement))
         acePathTo.insertAtEnd((ConcatOperationElement*) pveElement);
         pveElement = &((ConcatOperationElement&) *pveElement).fstArg();
      };
   }
   else
      pveElement = nullptr;
   return hasFound;
}

bool
ConcatOperationElement::ConcatenationCursor::setToLast() {
   acePathTo.deleteAll();
   pveElement = &ceSupport.sndArg();
   while (pveElement->getApproxKind().isFormalOperation()
         && (((const Operation&) ((const TFormalOperationElement<Scalar::Approximate::Details::IntOperationElement>&)
            *pveElement).getOperation()).getType() == Operation::TConcat)) {
      AssumeCondition(dynamic_cast<const ConcatOperationElement*>(pveElement))
      acePathTo.insertAtEnd((ConcatOperationElement*) pveElement);
      pveElement = &((ConcatOperationElement&) *pveElement).sndArg();
   };
   return true;
}

bool
ConcatOperationElement::ConcatenationCursor::setToPrevious() {
   if (!pveElement)
      return setToLast();
   bool hasFound = false;
   while (!hasFound && !acePathTo.isEmpty()) {
      if (&acePathTo.getLast().sndArg() == pveElement) {
         hasFound = true;
         pveElement = &acePathTo.getLast().fstArg();
      };
      if (!hasFound) {
         pveElement = &acePathTo.getSLast();
         acePathTo.deleteLast();
      };
   };
   if (!hasFound) {
      if (&ceSupport.sndArg() == pveElement) {
         hasFound = true;
         pveElement = &ceSupport.fstArg();
      }
      else
         pveElement = const_cast<ConcatOperationElement*>(&ceSupport);
   };
   if (hasFound) {
      while (pveElement->getApproxKind().isFormalOperation()
            && (((const Operation&) ((const TFormalOperationElement<Scalar::Approximate::Details::IntOperationElement>&)
               *pveElement).getOperation()).getType() == Operation::TConcat)) {
         AssumeCondition(dynamic_cast<const ConcatOperationElement*>(pveElement))
         acePathTo.insertAtEnd((ConcatOperationElement*) pveElement);
         pveElement = &((ConcatOperationElement&) *pveElement).sndArg();
      };
   }
   else
      pveElement = nullptr;
   return hasFound;
}

VirtualElement*
ConcatOperationElement::ConcatenationCursor::freeBeforeAndReplaceRoot() {
   bool hasFound = false;
   Path pathBefore(acePathTo);
   VirtualElement* elementBefore = pveElement;
   while (!hasFound && !pathBefore.isEmpty()) {
      if (&pathBefore.getLast().sndArg() == elementBefore) {
         hasFound = true;
         elementBefore = &pathBefore.getLast().fstArg();
      };
      if (!hasFound) {
         elementBefore = &pathBefore.getSLast();
         pathBefore.deleteLast();
      };
   };
   if (!hasFound) {
      if (&ceSupport.sndArg() == elementBefore) {
         hasFound = true;
         elementBefore = &ceSupport.fstArg();
      }
      else if (!elementBefore) {
         hasFound = true;
         elementBefore = &ceSupport.sndArg();
      }
      else
         elementBefore = const_cast<ConcatOperationElement*>(&ceSupport);
   };
   AssumeCondition(hasFound)
   int deleteIndex = pathBefore.count()-1;
   while (elementBefore->getApproxKind().isFormalOperation()
         && (((const Operation&) ((const TFormalOperationElement<Scalar::Approximate::Details::IntOperationElement>&)
            *elementBefore).getOperation()).getType() == Operation::TConcat)) {
      AssumeCondition(dynamic_cast<const ConcatOperationElement*>(elementBefore))
      pathBefore.insertAtEnd((ConcatOperationElement*) elementBefore);
      elementBefore = &((ConcatOperationElement&) *elementBefore).sndArg();
   };
   if (!pathBefore.isEmpty()) {
      ConcatOperationElement* parent = &pathBefore.getSLast();
      ConcatOperationElement* parentParent = (pathBefore.count() > 1)
         ? const_cast<ConcatOperationElement*>(&pathBefore[pathBefore.count()-2])
         : const_cast<ConcatOperationElement*>(&ceSupport);
      PPVirtualElement element;
      if (pveElement) {
         AssumeCondition(&parent->sndArg() == elementBefore)
         parent->ssndArg().release(); // delete elementBefore;
         element = parent->sfstArg();
      }
      else {
         AssumeCondition(&parent->fstArg() == elementBefore)
         parent->sfstArg().release(); // delete elementBefore;
         element = parent->ssndArg();
      };
      if (&parentParent->fstArg() == parent) // delete parent
         parentParent->sfstArg() = element;
      else
         parentParent->ssndArg() = element;
      if (acePathTo.count() > deleteIndex && &acePathTo[deleteIndex] == parent) {
         Path::Cursor cursor(acePathTo);
         cursor.setIndex(deleteIndex);
         acePathTo.deleteAt(cursor);
      };
      return nullptr;
   }
   else {
      PPVirtualElement element;
      if (pveElement) {
         AssumeCondition(&ceSupport.sndArg() == elementBefore)
         const_cast<ConcatOperationElement&>(ceSupport).ssndArg().release(); // delete elementBefore;
         element = const_cast<ConcatOperationElement&>(ceSupport).sfstArg();
      }
      else {
         AssumeCondition(&ceSupport.fstArg() == elementBefore)
         const_cast<ConcatOperationElement&>(ceSupport).sfstArg().release(); // delete elementBefore;
         element = const_cast<ConcatOperationElement&>(ceSupport).ssndArg();
      };

      if (!acePathTo.isEmpty())
         acePathTo.deleteFirst();
      return element.extractElement();
   };
}

VirtualElement*
ConcatOperationElement::ConcatenationCursor::freeNextAndReplaceRoot() {
   bool hasFound = false;
   Path pathBefore(acePathTo);
   VirtualElement* elementBefore = pveElement;
   while (!hasFound && !pathBefore.isEmpty()) {
      if (&pathBefore.getLast().fstArg() == elementBefore) {
         hasFound = true;
         elementBefore = &pathBefore.getLast().sndArg();
      };
      if (!hasFound) {
         elementBefore = &pathBefore.getSLast();
         pathBefore.deleteLast();
      };
   };
   if (!hasFound) {
      if (&ceSupport.fstArg() == elementBefore) {
         hasFound = true;
         elementBefore = &ceSupport.sndArg();
      }
      else if (!elementBefore) {
         hasFound = true;
         elementBefore = &ceSupport.fstArg();
      }
      else
         elementBefore = const_cast<ConcatOperationElement*>(&ceSupport);
   };
   AssumeCondition(hasFound)
   int deleteIndex = pathBefore.count()-1;
   while (elementBefore->getApproxKind().isFormalOperation()
         && (((const Operation&) ((const TFormalOperationElement<Scalar::Approximate::Details::IntOperationElement>&)
            *elementBefore).getOperation()).getType() == Operation::TConcat)) {
      AssumeCondition(dynamic_cast<const ConcatOperationElement*>(elementBefore))
      pathBefore.insertAtEnd((ConcatOperationElement*) elementBefore);
      elementBefore = &((ConcatOperationElement&) *elementBefore).fstArg();
   };
   if (!pathBefore.isEmpty()) {
      ConcatOperationElement* parent = &pathBefore.getSLast();
      ConcatOperationElement* parentParent = (pathBefore.count() > 1)
         ? const_cast<ConcatOperationElement*>(&pathBefore[pathBefore.count()-2])
         : const_cast<ConcatOperationElement*>(&ceSupport);
      PPVirtualElement element;
      if (pveElement) {
         AssumeCondition(&parent->fstArg() == elementBefore)
         parent->sfstArg().release(); // delete elementBefore;
         element = parent->ssndArg();
      }
      else {
         AssumeCondition(&parent->sndArg() == elementBefore)
         parent->ssndArg().release(); // delete elementBefore;
         element = parent->sfstArg();
      };
      if (&parentParent->sndArg() == parent) // delete parent
         parentParent->ssndArg() = element;
      else
         parentParent->sfstArg() = element;
      if (acePathTo.count() > deleteIndex && &acePathTo[deleteIndex] == parent) {
         Path::Cursor cursor(acePathTo);
         cursor.setIndex(deleteIndex);
         acePathTo.deleteAt(cursor);
      };
      return nullptr;
   }
   else {
      PPVirtualElement element;
      if (pveElement) {
         AssumeCondition(&ceSupport.fstArg() == elementBefore)
         const_cast<ConcatOperationElement&>(ceSupport).sfstArg().release(); // delete elementBefore;
         element = const_cast<ConcatOperationElement&>(ceSupport).ssndArg();
      }
      else {
         AssumeCondition(&ceSupport.sndArg() == elementBefore)
         const_cast<ConcatOperationElement&>(ceSupport).ssndArg().release(); // delete elementBefore;
         element = const_cast<ConcatOperationElement&>(ceSupport).sfstArg();
      };

      if (!acePathTo.isEmpty())
         acePathTo.deleteFirst();
      return element.extractElement();
   };
}

void
ConcatOperationElement::ConcatenationCursor::replaceElementAtWith(VirtualElement* newElement) {
   PPVirtualElement snewElement(newElement, PNT::Pointer::Init());
   AssumeCondition(pveElement)
   if (!acePathTo.isEmpty()) {
      if (&ceSupport.fstArg() == pveElement)
         const_cast<ConcatOperationElement&>(ceSupport).sfstArg() = snewElement;
      else {
         AssumeCondition(&ceSupport.sndArg() == pveElement)
         const_cast<ConcatOperationElement&>(ceSupport).ssndArg() = snewElement;
      };
   }
   else {
      ConcatOperationElement& parent = acePathTo.getSLast();
      if (&parent.fstArg() == pveElement)
         parent.sfstArg() = snewElement;
      else {
         AssumeCondition(&parent.sndArg() == pveElement)
         parent.ssndArg() = snewElement;
      };
   }
   pveElement = newElement;
}

PPVirtualElement
ConcatOperationElement::queryTopResult() const {
   PPVirtualElement fstTop = Methods::newTop(fstArg());
   AssumeCondition(dynamic_cast<const Top*>(fstTop.key()))
   return PPVirtualElement(new Top(Top::Init().setTable(*this)
         .setBitSize(getSizeInBits()).setApplyTop(((const Top&) *fstTop).getApplyFunction())
         .setApplyToTop(((const Top&) *fstTop).getApplyToFunction())
         .setConstraintTop(((const Top&) *fstTop).getConstraintFunction())), PNT::Pointer::Init());
}

bool
ConcatOperationElement::applyExtend(const VirtualOperation& operation, EvaluationEnvironment& env) {
   TransmitEnvironment applyEnv(env);
   if (!fstArg().apply(operation, applyEnv))
      return false;
   if (applyEnv.hasResult())
      sfstArg() = applyEnv.presult();
   return true;
}
   
bool
ConcatOperationElement::applyConcat(const VirtualOperation& operation, EvaluationEnvironment& env) {
   TransmitEnvironment applyEnv(env);
   if (!sndArg().apply(operation, applyEnv))
      return false;
   if (applyEnv.hasResult())
      ssndArg() = applyEnv.presult();
   return true;
}

bool
ConcatOperationElement::applyReduce(const VirtualOperation& aoperation, EvaluationEnvironment& env) {
   AssumeCondition(dynamic_cast<const ReduceOperation*>(&aoperation))
   const ReduceOperation& operation = (const ReduceOperation&) aoperation;
   int sndSize = sndArg().getSizeInBits();
   bool result = false;
   if (operation.getLowBit() < sndSize) {
      if (operation.getHighBit() < sndSize) {
         result = sndArg().apply(aoperation, env);
         if (!env.hasResult())
            env.presult() = ssndArg();
      }
      else {
         ReduceOperation fstOperation(operation), sndOperation(operation);
         fstOperation.getSLowBit() = 0;
         fstOperation.getSHighBit() -= sndSize;
         sndOperation.getSHighBit() = sndSize-1;
         {  TransmitEnvironment applyEnv(env);
            result = sndArg().apply(sndOperation, applyEnv);
            if (!result)
               return false;
            if (applyEnv.hasResult())
               ssndArg() = applyEnv.presult();
         };
         {  TransmitEnvironment applyEnv(env);
            result = fstArg().apply(fstOperation, applyEnv);
            if (!result)
               return false;
            if (applyEnv.hasResult())
               sfstArg() = applyEnv.presult();
         };
      };
   }
   else {
      ReduceOperation fstOperation(operation);
      fstOperation.getSLowBit() -= sndSize;
      fstOperation.getSHighBit() -= sndSize;
      result = fstArg().apply(fstOperation, env);
      if (!env.hasResult())
         env.presult() = sfstArg();
   };
   return result;
}
   
bool
ConcatOperationElement::applyBitSet(const VirtualOperation& aoperation, EvaluationEnvironment& env) {
   AssumeCondition(dynamic_cast<const BitSetOperation*>(&aoperation))
   const BitSetOperation& operation = (const BitSetOperation&) aoperation;
   int sndSize = sndArg().getSizeInBits();
   bool result = false;
   if (operation.getLowBit() < sndSize) {
      if (operation.getHighBit() < sndSize) {
         if (operation.getLowBit() == 0 && operation.getHighBit() == sndSize-1) {
            ssndArg().setElement(env.getFirstArgument());
            return true;
         };
         result = sndArg().apply(aoperation, env);
         if (env.hasResult())
            ssndArg() = env.presult();
      }
      else {
         BitSetOperation fstOperation(operation), sndOperation(operation);
         fstOperation.getSLowBit() = 0;
         fstOperation.getSHighBit() -= sndSize;
         sndOperation.getSHighBit() = sndSize-1;
         PPVirtualElement argumentFstPart(env.getFirstArgument()), argumentSndPart(env.getFirstArgument());
         argumentSndPart = Methods::applyAssign(argumentSndPart, ReduceOperation()
               .setLowBit(0).setHighBit(sndSize-1-operation.getLowBit()), env);
         argumentFstPart = Methods::applyAssign(argumentFstPart, ReduceOperation()
               .setLowBit(sndSize-operation.getLowBit()).setHighBit(operation.getHighBit()-operation.getLowBit()), env);
         {  TransmitEnvironment applyEnv(env);
            applyEnv.setFirstArgument(*argumentSndPart);
            result = sndArg().apply(sndOperation, applyEnv);
            if (!result)
               return false;
            if (applyEnv.hasResult())
               ssndArg() = applyEnv.presult();
         };
         {  TransmitEnvironment applyEnv(env);
            applyEnv.setFirstArgument(*argumentFstPart);
            result = fstArg().apply(fstOperation, applyEnv);
            if (!result)
               return false;
            if (applyEnv.hasResult())
               sfstArg() = applyEnv.presult();
         };
      };
   }
   else {
      if (operation.getLowBit() == sndSize && operation.getHighBit() == sndSize+fstArg().getSizeInBits()-1) {
         sfstArg().setElement(env.getFirstArgument());
         return true;
      };
      BitSetOperation fstOperation(operation);
      fstOperation.getSLowBit() -= sndSize;
      fstOperation.getSHighBit() -= sndSize;
      result = fstArg().apply(fstOperation, env);
      if (env.hasResult())
         sfstArg() = env.presult();
   };
   return result;
}
   
bool
ConcatOperationElement::applyCastIntegerSigned(const VirtualOperation& operation, EvaluationEnvironment& env) {
   if (fstArg().queryZeroResult().isTrue())
      return sndArg().apply(operation, env);
   else {
      PPVirtualElement opposite = Methods::apply(fstArg(), NextSignedAssignOperation(), EvaluationParameters(env));
      if (opposite->queryZeroResult().isTrue()) {
         opposite = Methods::apply(sndArg(), PrevSignedAssignOperation(), env);
         opposite = Methods::applyAssign(opposite, OppositeSignedAssignOperation(), env);
         opposite = Methods::applyAssign(opposite, PrevSignedAssignOperation(), env);
         opposite->apply(operation, env);
         if (env.hasResult())
            opposite = env.presult();
         env.presult() = opposite;
         return true;
      };
   };
   PPVirtualElement top = queryTopResult();
   top = Methods::apply(*top, operation, EvaluationParameters(env));
   bool result = top.isValid();
   if (!result)
      return false;
   if (top->getSizeInBits() <= sndArg().getSizeInBits()) {
      result = sndArg().apply(operation, env);
      if (operation.isAssign() && !env.hasResult())
         env.presult() = ssndArg();
   }
   else {
      top = queryTopResult();
      result = top->apply(operation, env);
      env.mergeVerdictDegradate();
   }
   return result;
}
   
bool
ConcatOperationElement::applyCastShiftBit(const VirtualOperation& aoperation, EvaluationEnvironment& env) {
   AssumeCondition(dynamic_cast<const CastShiftBitOperation*>(&aoperation))
   int shift = ((const CastShiftBitOperation&) aoperation).getShift();
   int sndSize = sndArg().getSizeInBits();
   bool result = false;
   if (shift < sndSize)
      result = sndArg().apply(aoperation, env);
   else {
      CastShiftBitOperation fstOperation((const CastShiftBitOperation&) aoperation);
      fstOperation.getSShift() -= sndSize;
      result = fstArg().apply(fstOperation, env);
   };
   return result;
}
   
bool
ConcatOperationElement::applyCastIntegerUnsigned(const VirtualOperation& operation, EvaluationEnvironment& env) {
   if (fstArg().queryZeroResult().isTrue())
      return sndArg().apply(operation, env);
   PPVirtualElement top = queryTopResult();
   top = Methods::apply(*top, operation, EvaluationParameters(env));
   bool result = top.isValid();
   if (!result)
      return false;
   if (top->getSizeInBits() <= sndArg().getSizeInBits()) {
      result = sndArg().apply(operation, env);
      if (operation.isAssign() && !env.hasResult())
         env.presult() = ssndArg();
   }
   else {
      top = queryTopResult();
      result = top->apply(operation, env);
      env.mergeVerdictDegradate();
   }
   return result;
}
   
bool
ConcatOperationElement::applyCastDouble(const VirtualOperation& operation, EvaluationEnvironment& env) {
   bool result;
   if (ppveFstArg->getApproxKind().isConstant() && ppveFstArg->getSizeInBits() == 32 && ppveSndArg->getApproxKind().isTop()) {
      PPVirtualElement resultElement(*ppveFstArg);
      PPVirtualElement concat = Methods::newZero(*ppveSndArg);
      resultElement = Methods::applyAssign(resultElement, ConcatOperation(), *concat, env);
      result = resultElement->apply(operation, env);
   }
   else {
      PPVirtualElement top = queryTopResult();
      result = top->apply(operation, env);
      env.mergeVerdictDegradate();
   };
   return result;
}
   
bool
ConcatOperationElement::applyPrevAssign(const VirtualOperation& operation, EvaluationEnvironment& env) {
   ZeroResult sndResult = sndArg().queryZeroResult();
   bool result = false;
   {  TransmitEnvironment applyEnv(env);
      result = sndArg().apply(operation, applyEnv);
      if (!result)
         return false;
      if (applyEnv.hasResult())
         ssndArg() = applyEnv.presult();
      applyEnv.clearErrors();
   };
   if (sndResult.isZero()) {
      result = fstArg().apply(operation, env);
      if (env.hasResult())
         sfstArg() = env.presult();
   }
   else if (sndResult.mayBeZero()) {
      PPVirtualElement copyFst(fstArg());
      {  TransmitEnvironment applyEnv(env);
         copyFst = Methods::applyAssign(copyFst, operation, applyEnv);
         applyEnv.approximateErrors();
      };
      sfstArg() = Methods::merge(sfstArg(), *copyFst, env);
      env.mergeVerdictDegradate();
   };
   return result;
}
   
bool
ConcatOperationElement::applyNextAssign(const VirtualOperation& operation, EvaluationEnvironment& env) {
   bool result = false;
   {  TransmitEnvironment applyEnv(env);
      result = sndArg().apply(operation, applyEnv);
      if (!result)
         return false;
      if (applyEnv.hasResult())
         ssndArg() = applyEnv.presult();
      applyEnv.clearErrors();
   };
   ZeroResult sndResult = sndArg().queryZeroResult();
   if (sndResult.isZero()) {
      result = fstArg().apply(operation, env);
      if (env.hasResult())
         sfstArg() = env.presult();
   }
   else if (sndResult.mayBeZero()) {
      PPVirtualElement copyFst(fstArg());
      {  TransmitEnvironment applyEnv(env);
         copyFst = Methods::applyAssign(copyFst, operation, applyEnv);
         applyEnv.approximateErrors();
      };
      sfstArg() = Methods::merge(sfstArg(), *copyFst, env);
      env.mergeVerdictDegradate();
   };
   return result;
}
   
bool
ConcatOperationElement::applyPlusAssign(const VirtualOperation& aoperation, EvaluationEnvironment& env) {
   PPVirtualElement argLow = Methods::applyAssign(env.getFirstArgument(), ReduceOperation()
         .setLowBit(0).setHighBit(sndArg().getSizeInBits()-1), env);
   PPVirtualElement argHigh = Methods::applyAssign(env.getFirstArgument(), ReduceOperation()
         .setLowBit(sndArg().getSizeInBits()).setHighBit(getSizeInBits()-1), env);
   bool mayCarry = false, sureCarry = false;
   {  TransmitEnvironment applyEnv(env);
      applyEnv.setFirstArgument(*argLow);
      if (!sndArg().apply(PlusUnsignedAssignOperation(), applyEnv))
         return false;
      if (applyEnv.hasResult())
         ssndArg() = applyEnv.presult();
      sureCarry = applyEnv.isSureOverflow();
      mayCarry = applyEnv.isOverflow();
      applyEnv.clearErrors();
   };
   
   {  TransmitEnvironment applyEnv(env);
      applyEnv.setFirstArgument(*argHigh);
      if (!fstArg().apply(aoperation, applyEnv))
         return false;
      if (applyEnv.hasResult())
         sfstArg() = applyEnv.presult();
   };
   if (sureCarry) {
      Operation::Type type = ((const Operation&) aoperation).getType();
      bool result = false;
      if ((type == Operation::TPlusSignedAssign)
            || (type == Operation::TPlusUnsignedWithSignedAssign))
         result = fstArg().apply(NextSignedAssignOperation(), env);
      else
         result = fstArg().apply(NextUnsignedAssignOperation(), env);
      if (!result)
         return false;
      if (env.hasResult())
         sfstArg() = env.presult();
   }
   else if (mayCarry) {
      PPVirtualElement copyFst(fstArg());
      {  TransmitEnvironment applyEnv(env);
         Operation::Type type = ((const Operation&) aoperation).getType();
         if ((type == Operation::TPlusSignedAssign)
               || (type == Operation::TPlusUnsignedWithSignedAssign))
            copyFst = Methods::applyAssign(copyFst, NextSignedAssignOperation(), applyEnv);
         else
            copyFst = Methods::applyAssign(copyFst, NextUnsignedAssignOperation(), applyEnv);
         applyEnv.approximateErrors();
      };
      sfstArg() = Methods::merge(sfstArg(), *copyFst, env);
      env.mergeVerdictDegradate();
   };
   return true;
}
   
bool
ConcatOperationElement::applyFloat(const VirtualOperation& operation, EvaluationEnvironment& env) {
   PPVirtualElement top = queryTopResult();
   bool result = top->apply(operation, env);
   env.mergeVerdictDegradate();
   return result;
}
   
bool
ConcatOperationElement::applyMinusAssign(const VirtualOperation& operation, EvaluationEnvironment& env) {
   PPVirtualElement argLow = Methods::applyAssign(env.getFirstArgument(), ReduceOperation()
         .setLowBit(0).setHighBit(sndArg().getSizeInBits()-1), env);
   PPVirtualElement argHigh = Methods::applyAssign(env.getFirstArgument(), ReduceOperation()
         .setLowBit(sndArg().getSizeInBits()).setHighBit(getSizeInBits()-1), env);
   bool mayCarry = false, sureCarry = false;
   {  TransmitEnvironment applyEnv(env);
      applyEnv.setFirstArgument(*argLow);
      if (!sndArg().apply(MinusUnsignedAssignOperation(), applyEnv))
         return false;
      if (applyEnv.hasResult())
         ssndArg() = applyEnv.presult();
      sureCarry = applyEnv.isSureOverflow();
      mayCarry = applyEnv.isOverflow();
      applyEnv.clearErrors();
   };
   
   {  TransmitEnvironment applyEnv(env);
      applyEnv.setFirstArgument(*argHigh);
      if (!fstArg().apply(operation, applyEnv))
         return false;
      if (applyEnv.hasResult())
         sfstArg() = applyEnv.presult();
   };
   if (sureCarry) {
      Operation::Type type = ((const Operation&) operation).getType();
      bool result = false;
      if (type == Operation::TMinusSignedAssign)
         result = fstArg().apply(PrevSignedAssignOperation(), env);
      else
         result = fstArg().apply(PrevUnsignedAssignOperation(), env);
      if (!result)
         return false;
      if (env.hasResult())
         sfstArg() = env.presult();
   }
   else if (mayCarry) {
      PPVirtualElement copyFst(fstArg());
      {  TransmitEnvironment applyEnv(env);
         Operation::Type type = ((const Operation&) operation).getType();
         if (type == Operation::TMinusSignedAssign)
            copyFst = Methods::applyAssign(copyFst, PrevSignedAssignOperation(), applyEnv);
         else
            copyFst = Methods::applyAssign(copyFst, PrevUnsignedAssignOperation(), applyEnv);
         applyEnv.approximateErrors();
      };
      sfstArg() = Methods::merge(sfstArg(), *copyFst, env);
      env.mergeVerdictDegradate();
   };
   return true;
}
   
bool
ConcatOperationElement::applyCompareLessOrGreater(const VirtualOperation& operation, EvaluationEnvironment& env) {
   ZeroResult zeroResult;
   PPVirtualElement argHigh = Methods::applyAssign(env.getFirstArgument(), ReduceOperation()
         .setLowBit(sndArg().getSizeInBits()).setHighBit(getSizeInBits()-1), env);
   TransmitEnvironment applyEnv(env);
   applyEnv.setFirstArgument(*argHigh);
   bool result = fstArg().apply(operation, applyEnv);
   if (!result)
      return false;
   zeroResult = applyEnv.getResult().queryZeroResult();
   if (zeroResult.isTrue()) {
      env.presult() = applyEnv.presult();
      return true;
   };
   PPVirtualElement isEqualElement = Methods::apply(fstArg(), CompareEqualOperation(), applyEnv.getFirstArgument(), EvaluationParameters(env));
   if (isEqualElement->queryZeroResult().isFalse() && zeroResult.isFalse()) {
      env.presult() = applyEnv.presult();
      return true;
   };
   PPVirtualElement argLow = Methods::applyAssign(env.getFirstArgument(), ReduceOperation()
         .setLowBit(0).setHighBit(sndArg().getSizeInBits()-1), env);
   applyEnv.merge();
   if (isEqualElement->queryZeroResult().isTrue()) {
      applyEnv.presult().release();
      applyEnv.setFirstArgument(*argLow);
      result = sndArg().apply(operation, applyEnv);
      env.presult() = applyEnv.presult();
      return result;
   };
   TransmitEnvironment sndEnv(env);
   sndEnv.setFirstArgument(*argLow);
   result = sndArg().apply(operation, sndEnv);
   sndEnv.merge();
   if (!result)
      return false;
   TransmitEnvironment guardEnv(env);
   result = isEqualElement->guard(sndEnv.presult(), applyEnv.presult(), guardEnv);
   if (!result)
      return false;
   if (guardEnv.hasResult())
      isEqualElement = guardEnv.presult();
   env.presult() = isEqualElement;
   return result;
}

bool
ConcatOperationElement::applyCompareLessOrGreaterOrEqual(const VirtualOperation& operation, EvaluationEnvironment& env) {
   ZeroResult zeroResult;
   PPVirtualElement argHigh = Methods::applyAssign(env.getFirstArgument(), ReduceOperation()
         .setLowBit(sndArg().getSizeInBits()).setHighBit(getSizeInBits()-1), env);
   TransmitEnvironment applyEnv(env);
   applyEnv.setFirstArgument(*argHigh);
   bool result = fstArg().apply(operation, applyEnv);
   if (!result)
      return false;
   zeroResult = applyEnv.getResult().queryZeroResult();
   if (zeroResult.isFalse()) {
      env.presult() = applyEnv.presult();
      return true;
   };
   PPVirtualElement isEqualElement = Methods::apply(fstArg(), CompareEqualOperation(), env.getFirstArgument(), EvaluationParameters(env));
   if (isEqualElement->queryZeroResult().isFalse() && zeroResult.isTrue()) {
      env.presult() = applyEnv.presult();
      return true;
   };
   PPVirtualElement argLow = Methods::applyAssign(env.getFirstArgument(), ReduceOperation()
         .setLowBit(0).setHighBit(sndArg().getSizeInBits()-1), env);
   applyEnv.merge();
   if (isEqualElement->queryZeroResult().isTrue()) {
      applyEnv.presult().release();
      applyEnv.setFirstArgument(*argLow);
      result = sndArg().apply(operation, applyEnv);
      env.presult() = applyEnv.presult();
      return result;
   };
   TransmitEnvironment sndEnv(env);
   sndEnv.setFirstArgument(*argLow);
   result = sndArg().apply(operation, sndEnv);
   sndEnv.merge();
   if (!result)
      return false;
   TransmitEnvironment guardEnv(env);
   result = isEqualElement->guard(sndEnv.presult(), applyEnv.presult(), guardEnv);
   if (!result)
      return false;
   if (guardEnv.hasResult())
      isEqualElement = guardEnv.presult();
   env.presult() = isEqualElement;
   return result;
}

bool
ConcatOperationElement::applyCompareEqual(const VirtualOperation& operation, EvaluationEnvironment& env) {
   ZeroResult zeroResult;
   PPVirtualElement argHigh = Methods::applyAssign(env.getFirstArgument(), ReduceOperation()
         .setLowBit(sndArg().getSizeInBits()).setHighBit(getSizeInBits()-1), env);
   TransmitEnvironment applyEnv(env);
   applyEnv.setFirstArgument(*argHigh);
   bool result = fstArg().apply(operation, applyEnv);
   if (!result)
      return false;
   zeroResult = applyEnv.getResult().queryZeroResult();
   if (zeroResult.isFalse()) {
      env.presult() = applyEnv.presult();
      return true;
   };

   PPVirtualElement argLow = Methods::applyAssign(env.getFirstArgument(), ReduceOperation()
         .setLowBit(0).setHighBit(sndArg().getSizeInBits()-1), env);
   applyEnv.merge();
   if (zeroResult.isTrue()) {
      applyEnv.presult().release();
      applyEnv.setFirstArgument(*argLow);
      result = sndArg().apply(operation, applyEnv);
      env.presult() = applyEnv.presult();
      return result;
   };
   TransmitEnvironment sndEnv(env);
   sndEnv.setFirstArgument(*argLow);
   result = sndArg().apply(operation, sndEnv);
   sndEnv.merge();
   if (!result)
      return false;
   TransmitEnvironment guardEnv(env);
   result = applyEnv.getResult().guard(sndEnv.presult(), Methods::newFalse(*this), guardEnv);
   if (!result)
      return false;
   if (guardEnv.hasResult())
      env.presult() = guardEnv.presult();
   else
      env.presult() = applyEnv.presult();
   return result;
}

bool
ConcatOperationElement::applyCompareDifferent(const VirtualOperation& operation, EvaluationEnvironment& env) {
   ZeroResult zeroResult;
   PPVirtualElement argHigh = Methods::applyAssign(env.getFirstArgument(), ReduceOperation()
         .setLowBit(sndArg().getSizeInBits()).setHighBit(getSizeInBits()-1), env);
   TransmitEnvironment applyEnv(env);
   applyEnv.setFirstArgument(*argHigh);
   bool result = fstArg().apply(operation, applyEnv);
   if (!result)
      return false;
   zeroResult = applyEnv.getResult().queryZeroResult();
   if (zeroResult.isTrue()) {
      env.presult() = applyEnv.presult();
      return true;
   };

   PPVirtualElement argLow = Methods::applyAssign(env.getFirstArgument(), ReduceOperation()
         .setLowBit(0).setHighBit(sndArg().getSizeInBits()-1), env);
   applyEnv.merge();
   if (zeroResult.isFalse()) {
      applyEnv.presult().release();
      applyEnv.setFirstArgument(*argLow);
      result = sndArg().apply(operation, applyEnv);
      env.presult() = applyEnv.presult();
      return result;
   };
   TransmitEnvironment sndEnv(env);
   sndEnv.setFirstArgument(*argLow);
   result = sndArg().apply(operation, sndEnv);
   sndEnv.merge();
   if (!result)
      return false;
   TransmitEnvironment guardEnv(env);
   result = applyEnv.getResult().guard(Methods::newTrue(*this), sndEnv.presult(), guardEnv);
   if (!result)
      return false;
   if (guardEnv.hasResult())
      env.presult() = guardEnv.presult();
   else
      env.presult() = applyEnv.presult();
   return result;
}

bool
ConcatOperationElement::applyMinMaxAssign(const VirtualOperation& aoperation, EvaluationEnvironment& env) {
   ZeroResult zeroResult;
   PPVirtualElement argHigh = Methods::applyAssign(env.getFirstArgument(), ReduceOperation()
         .setLowBit(sndArg().getSizeInBits()).setHighBit(getSizeInBits()-1), env);
   Operation::Type type = ((const Operation&) aoperation).getType();
   if (type == Operation::TMinSignedAssign)
      zeroResult = Methods::apply(fstArg(), CompareGreaterSignedOperation(),
            *argHigh, EvaluationParameters(env))->queryZeroResult();
   else if (type == Operation::TMinUnsignedAssign)
      zeroResult = Methods::apply(fstArg(), CompareGreaterUnsignedOperation(),
            *argHigh, EvaluationParameters(env))->queryZeroResult();
   else if (type == Operation::TMaxSignedAssign)
      zeroResult = Methods::apply(fstArg(), CompareLessSignedOperation(),
            *argHigh, EvaluationParameters(env))->queryZeroResult();
   else
      zeroResult = Methods::apply(fstArg(), CompareLessUnsignedOperation(),
            *argHigh, EvaluationParameters(env))->queryZeroResult();
   if (zeroResult.isFalse())
      return true;
   PPVirtualElement isEqualElement = Methods::apply(fstArg(), CompareEqualOperation(), env.getFirstArgument(), EvaluationParameters(env));
   if (isEqualElement->queryZeroResult().isFalse() && zeroResult.isTrue()) {
      env.presult() = env.getFirstArgument();
      return true;
   };
   PPVirtualElement argLow = Methods::applyAssign(env.getFirstArgument(), ReduceOperation()
         .setLowBit(0).setHighBit(sndArg().getSizeInBits()-1), env);
   if (isEqualElement->queryZeroResult().isTrue()) {
      TransmitEnvironment applyEnv(env);
      applyEnv.setFirstArgument(*argLow);
      if (type == Operation::TMinSignedAssign)
         zeroResult = Methods::apply(fstArg(), CompareGreaterSignedOperation(),
               *argHigh, EvaluationParameters(env))->queryZeroResult();
      else if (type == Operation::TMinUnsignedAssign)
         zeroResult = Methods::apply(fstArg(), CompareGreaterUnsignedOperation(),
               *argHigh, EvaluationParameters(env))->queryZeroResult();
      else if (type == Operation::TMaxSignedAssign)
         zeroResult = Methods::apply(fstArg(), CompareLessSignedOperation(),
               *argHigh, EvaluationParameters(env))->queryZeroResult();
      else
         zeroResult = Methods::apply(fstArg(), CompareLessUnsignedOperation(),
               *argHigh, EvaluationParameters(env))->queryZeroResult();
      if (zeroResult.isFalse())
         return true;
      if (zeroResult.isTrue()) {
         env.presult() = env.getFirstArgument();
         return true;
      };
   };
   PPVirtualElement lessHighElement, lessOrEqualLowElement;
   if (type == Operation::TMinSignedAssign) {
      lessHighElement = Methods::apply(fstArg(), CompareLessSignedOperation(), *argHigh, EvaluationParameters(env));
      lessOrEqualLowElement = Methods::apply(sndArg(), CompareLessOrEqualSignedOperation(), *argLow, EvaluationParameters(env));
   }
   else if (type == Operation::TMinUnsignedAssign) {
      lessHighElement = Methods::apply(fstArg(), CompareLessUnsignedOperation(), *argHigh, EvaluationParameters(env));
      lessOrEqualLowElement = Methods::apply(sndArg(), CompareLessOrEqualUnsignedOperation(), *argLow, EvaluationParameters(env));
   }
   else if (type == Operation::TMaxSignedAssign) {
      lessHighElement = Methods::apply(fstArg(), CompareGreaterSignedOperation(), *argHigh, EvaluationParameters(env));
      lessOrEqualLowElement = Methods::apply(sndArg(), CompareGreaterOrEqualSignedOperation(), *argLow, EvaluationParameters(env));
   }
   else {
      lessHighElement = Methods::apply(fstArg(), CompareGreaterUnsignedOperation(), *argHigh, EvaluationParameters(env));
      lessOrEqualLowElement = Methods::apply(sndArg(), CompareGreaterOrEqualUnsignedOperation(), *argLow, EvaluationParameters(env));
   };
   // result = lessHighElement ? *this : (isEqualElement ? (lessOrEqualLowElement ? *this : source) : source);
   TransmitEnvironment applyEnv(env);
   if (!lessOrEqualLowElement->guard(*this, env.getFirstArgument(), applyEnv))
      return false;
   applyEnv.merge();
   if (applyEnv.hasResult())
      lessOrEqualLowElement = applyEnv.presult();
   if (!isEqualElement->guard(lessOrEqualLowElement, env.getFirstArgument(), applyEnv))
      return false;
   if (applyEnv.hasResult())
      isEqualElement = applyEnv.presult();
   applyEnv.merge();
   if (!lessHighElement->guard(*this, isEqualElement, applyEnv))
      return false;
   if (applyEnv.hasResult())
      lessHighElement = applyEnv.presult();
   applyEnv.merge();
   env.presult() = lessHighElement;
   return true;
}

bool
ConcatOperationElement::applyTimesAssign(const VirtualOperation& operation, EvaluationEnvironment& env) {
   int lowBits = sndArg().getSizeInBits(), highBits = fstArg().getSizeInBits();
   PPVirtualElement argHigh = Methods::applyAssign(env.getFirstArgument(), ReduceOperation()
         .setLowBit(lowBits).setHighBit(lowBits + highBits - 1), env);
   PPVirtualElement argLow = Methods::applyAssign(env.getFirstArgument(), ReduceOperation()
         .setLowBit(0).setHighBit(lowBits-1), env);
   PPVirtualElement thisLowElement(sndArg());
   {  TransmitEnvironment applyEnv(env);
      applyEnv.setIntervalLatticeCreation(); // setTopLatticeCreation();
      argLow = Methods::applyAssign(argLow, ExtendOperation()
            .setExtendWithZero(highBits), applyEnv);
      thisLowElement = Methods::applyAssign(thisLowElement, ExtendOperation()
            .setExtendWithZero(highBits), applyEnv);
      applyEnv.setFirstArgument(*argLow);
      if (!thisLowElement->apply(TimesUnsignedAssignOperation(), applyEnv))
         return false;
      if (applyEnv.hasResult())
         thisLowElement = applyEnv.presult();
   };
   PPVirtualElement lowElement = Methods::applyAssign(*thisLowElement, ReduceOperation().setLowBit(0).setHighBit(lowBits-1), EvaluationParameters(env));
   PPVirtualElement fstOut = Methods::applyAssign(*thisLowElement, ReduceOperation().setLowBit(lowBits).setHighBit(highBits + lowBits-1), EvaluationParameters(env));
   PPVirtualElement sndOut, thdOut, fthOut;
   
   if (highBits < lowBits) {
      argLow = Methods::applyAssign(argLow, ReduceOperation()
            .setLowBit(0).setHighBit(highBits-1), EvaluationParameters(env));
      sndOut = Methods::applyAssign(fstArg(), operation, *argLow, env);
      thdOut = Methods::applyAssign(Methods::applyAssign(sndArg(), ReduceOperation()
               .setLowBit(0).setHighBit(highBits-1), EvaluationParameters(env)),
            operation, *argHigh, env);
   }
   else if (highBits > lowBits) {
      argLow = Methods::applyAssign(argLow, ReduceOperation()
               .setLowBit(0).setHighBit(highBits-1), EvaluationParameters(env));
      sndOut = Methods::applyAssign(fstArg(), operation, *argLow, env);
      thdOut = Methods::applyAssign(Methods::applyAssign(sndArg(), ExtendOperation()
               .setExtendWithZero(highBits-lowBits), EvaluationParameters(env)),
            operation, *argHigh, env);
   }
   else {
      sndOut = Methods::applyAssign(fstArg(), operation, *argLow, env);
      thdOut = Methods::applyAssign(sndArg(), operation, *argHigh, env);
   };
   fthOut = Methods::applyAssign(fstArg(), operation, *argHigh, env);
   {  TransmitEnvironment localEnv(env);
      PPVirtualElement shift = Methods::newIntForShift(*fthOut, lowBits);
      localEnv.setFirstArgument(*shift);
      fthOut = Methods::applyAssign(fthOut, LeftShiftAssignOperation(), localEnv);
   };
   Operation plusOperation;
   Operation::Type type = ((const Operation&) operation).getType();
   if (type == Operation::TTimesUnsignedAssign)
      plusOperation = PlusUnsignedAssignOperation();
   else // type == Operation::TTimesSignedOperation
      plusOperation = PlusSignedAssignOperation();
   fstOut = Methods::applyAssign(fstOut, plusOperation, *sndOut, env);
   fstOut = Methods::applyAssign(fstOut, plusOperation, *thdOut, env);
   fstOut = Methods::applyAssign(fstOut, plusOperation, *fthOut, env);
   sfstArg() = lowElement;
   ssndArg() = fstOut;
   return true;
}

bool
ConcatOperationElement::applyDivideAssign(const VirtualOperation& operation, EvaluationEnvironment& env) {
   PPVirtualElement top = queryTopResult();
   bool result = top->apply(operation, env);
   env.mergeVerdictDegradate();
   return result;
}
   
bool
ConcatOperationElement::applyOppositeSignedAssign(const VirtualOperation& operation, EvaluationEnvironment& env) {
   TransmitEnvironment applyEnv(env);
   if (!fstArg().apply(operation, applyEnv))
      return false;
   if (applyEnv.hasResult())
      sfstArg() = applyEnv.presult();
   applyEnv.merge();
   sfstArg() = Methods::applyAssign(sfstArg(), PrevSignedAssignOperation(), env);
   if (!sndArg().apply(operation, applyEnv))
      return false;
   if (applyEnv.hasResult())
      ssndArg() = applyEnv.presult();
   return true;
}
   
bool
ConcatOperationElement::applyModuloAssign(const VirtualOperation& operation, EvaluationEnvironment& env) {
   PPVirtualElement top = queryTopResult();
   bool result = top->apply(operation, env);
   env.mergeVerdictDegradate();
   return result;
}

bool
ConcatOperationElement::applyBitBinaryAssign(const VirtualOperation& operation, EvaluationEnvironment& env) {
   int lowBits = sndArg().getSizeInBits(), highBits = fstArg().getSizeInBits();
   PPVirtualElement argHigh = Methods::applyAssign(env.getFirstArgument(), ReduceOperation()
         .setLowBit(lowBits).setHighBit(lowBits + highBits - 1), env);
   PPVirtualElement argLow = Methods::applyAssign(env.getFirstArgument(), ReduceOperation()
         .setLowBit(0).setHighBit(lowBits-1), env);
   TransmitEnvironment applyEnv(env);
   applyEnv.setFirstArgument(*argHigh);
   if (!fstArg().apply(operation, applyEnv))
      return false;
   if (applyEnv.hasResult())
      sfstArg() = applyEnv.presult();
   applyEnv.merge();
   applyEnv.setFirstArgument(*argLow);
   if (!sndArg().apply(operation, applyEnv))
      return false;
   if (applyEnv.hasResult())
      ssndArg() = applyEnv.presult();
   return true;
}
   
bool
ConcatOperationElement::applyBitUnaryAssign(const VirtualOperation& operation, EvaluationEnvironment& env) {
   TransmitEnvironment applyEnv(env);
   if (!fstArg().apply(operation, applyEnv))
      return false;
   if (applyEnv.hasResult())
      sfstArg() = applyEnv.presult();
   applyEnv.merge();
   if (!sndArg().apply(operation, applyEnv))
      return false;
   if (applyEnv.hasResult())
      ssndArg() = applyEnv.presult();
   return true;
}

bool
ConcatOperationElement::applyLeftShiftAssignConstant(const VirtualOperation& operation,
      const VirtualElement& source, EvaluationEnvironment& env) {
   TransmitEnvironment applyEnv(env);
   applyEnv.setFirstArgument(source);
   if (!fstArg().apply(operation, applyEnv))
      return false;
   if (applyEnv.hasResult())
      sfstArg() = applyEnv.presult();
   applyEnv.merge();

   PPVirtualElement shift = Methods::newIntForShift(source, sndArg().getSizeInBits());
   assume(shift->apply(Scalar::MultiBit::MinusUnsignedAssignOperation(), applyEnv));
   PPVirtualElement highSndPart;
   if (!applyEnv.isNegativeOverflow()) {
      if (applyEnv.hasResult())
         shift = applyEnv.presult();
      applyEnv.merge();

      applyEnv.setFirstArgument(*shift);
      assume(sndArg().apply(LogicalRightShiftAssignOperation().setConstWithAssign(), applyEnv));
      PPVirtualElement highSndPart = applyEnv.presult();
      applyEnv.merge();

      if (sndArg().getSizeInBits() > fstArg().getSizeInBits())
         assume(highSndPart->apply(ReduceOperation()
                  .setLowBit(0).setHighBit(fstArg().getSizeInBits()-1), applyEnv));
      else if (sndArg().getSizeInBits() < fstArg().getSizeInBits())
         assume(highSndPart->apply(ExtendOperation()
            .setExtendWithZero(fstArg().getSizeInBits() - sndArg().getSizeInBits()), applyEnv));
      if (applyEnv.hasResult())
         highSndPart = applyEnv.presult();
      applyEnv.merge();
   }
   else { // applyEnv.isNegativeOverflow(): source > sndArg().getSizeInBits()
      applyEnv.clearMask();
      highSndPart.setElement(sndArg());
      if (sndArg().getSizeInBits() > fstArg().getSizeInBits())
         assume(highSndPart->apply(ReduceOperation()
                  .setLowBit(0).setHighBit(fstArg().getSizeInBits()-1), applyEnv));
      else if (sndArg().getSizeInBits() < fstArg().getSizeInBits())
         assume(highSndPart->apply(ExtendOperation()
            .setExtendWithZero(fstArg().getSizeInBits() - sndArg().getSizeInBits()), applyEnv));
      if (applyEnv.hasResult())
         highSndPart = applyEnv.presult();
      applyEnv.merge();

      shift.setElement(source);
      PPVirtualElement arg = Methods::newIntForShift(source, sndArg().getSizeInBits());
      applyEnv.setFirstArgument(*arg);
      assume(shift->apply(Scalar::MultiBit::MinusUnsignedAssignOperation(), applyEnv));
      AssumeCondition(!applyEnv.isNegativeOverflow())
      if (applyEnv.hasResult())
         shift = applyEnv.presult();
      applyEnv.merge();

      applyEnv.setFirstArgument(*shift);
      assume(highSndPart->apply(Scalar::MultiBit::LeftShiftAssignOperation(), applyEnv));
      if (applyEnv.hasResult())
         highSndPart = applyEnv.presult();
      applyEnv.merge();
   };

   applyEnv.setFirstArgument(*highSndPart);
   assume(fstArg().apply(BitOrAssignOperation(), applyEnv));
   if (applyEnv.hasResult())
      sfstArg() = applyEnv.presult();
   applyEnv.merge();

   applyEnv.setFirstArgument(source);
   assume(sndArg().apply(operation, applyEnv));
   if (applyEnv.hasResult())
      ssndArg() = applyEnv.presult();
   return true;
}

bool
ConcatOperationElement::applyLeftShiftAssign(const VirtualOperation& operation,
      const VirtualElement& source, EvaluationEnvironment& env) {
   PPVirtualElement top = queryTopResult();
   bool result = top->apply(operation, env);
   env.mergeVerdictDegradate();
   if (!env.hasResult())
      env.presult() = top;
   return result;
}
   
bool
ConcatOperationElement::applyLogicalRightShiftAssignConstant(const VirtualOperation& operation,
      const VirtualElement& source, EvaluationEnvironment& env) {
   TransmitEnvironment applyEnv(env);
   applyEnv.setFirstArgument(source);
   if (!sndArg().apply(operation, applyEnv))
      return false;
   if (applyEnv.hasResult())
      ssndArg() = applyEnv.presult();
   applyEnv.merge();

   PPVirtualElement shift = Methods::newIntForShift(source, fstArg().getSizeInBits());
   assume(shift->apply(MinusUnsignedAssignOperation(), applyEnv));
   PPVirtualElement lowFstPart;
   if (!applyEnv.isNegativeOverflow()) {
      if (applyEnv.hasResult())
         shift = applyEnv.presult();
      applyEnv.merge();

      applyEnv.setFirstArgument(*shift);
      assume(fstArg().apply(LogicalRightShiftAssignOperation().setConstWithAssign(), applyEnv));
      lowFstPart = applyEnv.presult();
      applyEnv.merge();
      assume(lowFstPart->apply(LeftShiftAssignOperation(), applyEnv));
      if (applyEnv.hasResult())
         lowFstPart = applyEnv.presult();
      applyEnv.merge();

      if (fstArg().getSizeInBits() > sndArg().getSizeInBits())
         assume(lowFstPart->apply(ReduceOperation()
               .setLowBit(fstArg().getSizeInBits()-sndArg().getSizeInBits())
               .setHighBit(fstArg().getSizeInBits()-1), applyEnv));
      else if (fstArg().getSizeInBits() < sndArg().getSizeInBits()) {
         assume(lowFstPart->apply(ExtendOperation()
            .setExtendWithZero(sndArg().getSizeInBits() - fstArg().getSizeInBits()), applyEnv));
         if (applyEnv.hasResult())
            lowFstPart = applyEnv.presult();
         applyEnv.merge();
         PPVirtualElement additionalShift
             = Methods::newIntForShift(*lowFstPart, sndArg().getSizeInBits() - fstArg().getSizeInBits());
         applyEnv.setFirstArgument(*additionalShift);
         assume(lowFstPart->apply(LeftShiftAssignOperation(), applyEnv));
      };
      if (applyEnv.hasResult())
         lowFstPart = applyEnv.presult();
      applyEnv.merge();
   }
   else { // applyEnv.isNegativeOverflow(): source > fstArg().getSizeInBits()
      applyEnv.clearMask();
      lowFstPart.setElement(fstArg());
      if (fstArg().getSizeInBits() > sndArg().getSizeInBits())
         assume(lowFstPart->apply(ReduceOperation()
                  .setLowBit(0).setHighBit(sndArg().getSizeInBits()-1), applyEnv));
      else if (fstArg().getSizeInBits() < sndArg().getSizeInBits())
         assume(lowFstPart->apply(ExtendOperation()
            .setExtendWithZero(sndArg().getSizeInBits() - fstArg().getSizeInBits()), applyEnv));
      if (applyEnv.hasResult())
         lowFstPart = applyEnv.presult();
      applyEnv.merge();

      shift = Methods::newIntForShift(source, sndArg().getSizeInBits());
      PPVirtualElement shiftArg(source);
      PPVirtualElement arg = Methods::newIntForShift(source, fstArg().getSizeInBits());
      applyEnv.setFirstArgument(*arg);
      assume(shiftArg->apply(MinusUnsignedAssignOperation(), applyEnv));
      AssumeCondition(!applyEnv.isNegativeOverflow())
      if (applyEnv.hasResult())
         shiftArg = applyEnv.presult();
      applyEnv.merge();
      applyEnv.setFirstArgument(*shiftArg);
      assume(shift->apply(MinusUnsignedAssignOperation(), applyEnv));
      if (!applyEnv.isNegativeOverflow()) {
         if (applyEnv.hasResult())
            shiftArg = applyEnv.presult();
         applyEnv.merge();

         applyEnv.setFirstArgument(*shift);
         assume(lowFstPart->apply(LeftShiftAssignOperation(), applyEnv));
         if (applyEnv.hasResult())
            lowFstPart = applyEnv.presult();
         applyEnv.merge();
      }
      else
         lowFstPart.release();
   };

   if (lowFstPart.isValid()) {
      applyEnv.setFirstArgument(*lowFstPart);
      assume(sndArg().apply(BitOrAssignOperation(), applyEnv));
      if (applyEnv.hasResult())
         ssndArg() = applyEnv.presult();
      applyEnv.merge();
   };

   applyEnv.setFirstArgument(source);
   assume(fstArg().apply(operation, applyEnv));
   if (applyEnv.hasResult())
      sfstArg() = applyEnv.presult();
   return true;
}

bool
ConcatOperationElement::applyLogicalRightShiftAssign(const VirtualOperation& operation,
      const VirtualElement& source, EvaluationEnvironment& env) {
   PPVirtualElement top = queryTopResult();
   bool fResult = top->apply(operation, env);
   env.mergeVerdictDegradate();
   if (!env.hasResult())
      env.presult() = top;
   return fResult;
}

bool
ConcatOperationElement::applyArithmeticRightShiftAssignConstant(const VirtualOperation& operation,
      const VirtualElement& source, EvaluationEnvironment& env) {
   // [TODO]
   PPVirtualElement top = queryTopResult();
   bool result = top->apply(operation, env);
   env.mergeVerdictDegradate();
   if (!env.hasResult())
      env.presult() = top;
   return result;
}
   
bool
ConcatOperationElement::applyArithmeticRightShiftAssign(const VirtualOperation& operation,
      const VirtualElement& source, EvaluationEnvironment& env) {
   PPVirtualElement top = queryTopResult();
   bool result = top->apply(operation, env);
   env.mergeVerdictDegradate();
   if (!env.hasResult())
      env.presult() = top;
   return result;
}
   
bool
ConcatOperationElement::applyLeftRotateAssignConstant(const VirtualOperation& operation, const VirtualElement& source, EvaluationEnvironment& env) {
   // AssumeUnimplemented = gain back the shift in an int value <-> TDIntForShift
   PPVirtualElement top = queryTopResult();
   bool result = top->apply(operation, env);
   env.mergeVerdictDegradate();
   if (!env.hasResult())
      env.presult() = top;
   return result;
}
   
bool
ConcatOperationElement::applyLeftRotateAssign(const VirtualOperation& operation, const VirtualElement& veSource, EvaluationEnvironment& env) {
   PPVirtualElement top = queryTopResult();
   bool result = top->apply(operation, env);
   env.mergeVerdictDegradate();
   if (!env.hasResult())
      env.presult() = top;
   return result;
}
   
bool
ConcatOperationElement::applyRightRotateAssignConstant(const VirtualOperation& operation, const VirtualElement& veSource, EvaluationEnvironment& env) {
   // AssumeUnimplemented = gain back the shift in an int value <-> TDIntForShift
   PPVirtualElement top = queryTopResult();
   bool result = top->apply(operation, env);
   env.mergeVerdictDegradate();
   if (!env.hasResult())
      env.presult() = top;
   return result;
}
   
bool
ConcatOperationElement::applyRightRotateAssign(const VirtualOperation& operation, const VirtualElement& veSource, EvaluationEnvironment& env) {
   PPVirtualElement top = queryTopResult();
   bool result = top->apply(operation, env);
   env.mergeVerdictDegradate();
   if (!env.hasResult())
      env.presult() = top;
   return result;
}
   
#define DefineInitApply(TypeOperation)                                                             \
   elementAt(Operation::T##TypeOperation).setMethod(&ConcatOperationElement::apply##TypeOperation);

#define DefineInitOApply(TypeOperation, TypeRedirection)                                           \
   elementAt(Operation::T##TypeOperation).setMethod(&ConcatOperationElement::apply##TypeRedirection);

#define DefineInitBiApply(TypeOperation)                                                           \
   elementAt(Operation::T##TypeOperation).setArray(new ConstantAndGenericApplyMethod(              \
      &ConcatOperationElement::apply##TypeOperation##Constant,                                     \
      &ConcatOperationElement::apply##TypeOperation));

#define DefineInitOBiApply(TypeOperation, TypeMethod)                                              \
   elementAt(Operation::T##TypeOperation).setArray(new ConstantAndGenericApplyMethod(              \
      &ConcatOperationElement::apply##TypeMethod##Constant,                                        \
      &ConcatOperationElement::apply##TypeMethod));

ConcatOperationElement::MethodApplyTable::MethodApplyTable() {
#include "StandardClasses/UndefineNew.h"
   clear();

   DefineInitOApply(ExtendWithZero, Extend)
   DefineInitOApply(ExtendWithSign, Extend)
   DefineInitApply(Concat)
   DefineInitApply(Reduce)
   DefineInitApply(BitSet)
   if ((char) 0xFF < 0)
      DefineInitOApply(CastChar, CastIntegerSigned)
   else
      DefineInitOApply(CastChar, CastIntegerUnsigned)
   DefineInitOApply(CastInt, CastIntegerSigned)
   DefineInitOApply(CastUnsignedInt, CastIntegerUnsigned)
   DefineInitOApply(CastMultiBit, CastIntegerUnsigned)
   // DefineInitOApply(CastBit, CastIntegerUnsigned)
   DefineInitApply(CastShiftBit)
   DefineInitOApply(CastMultiFloat, CastDouble)
   DefineInitOApply(CastMultiFloatPointer, CastDouble)
   DefineInitOApply(CastReal, CastDouble)
   DefineInitOApply(CastRational, CastDouble)
   
   DefineInitOApply(PrevSignedAssign, PrevAssign)
   DefineInitOApply(PrevUnsignedAssign, PrevAssign)
   DefineInitOApply(NextSignedAssign, NextAssign)
   DefineInitOApply(NextUnsignedAssign, NextAssign)
   DefineInitOApply(PlusSignedAssign, PlusAssign)
   DefineInitOApply(PlusUnsignedAssign, PlusAssign)
   DefineInitOApply(PlusUnsignedWithSignedAssign, PlusAssign)
   DefineInitOApply(PlusFloatAssign, Float)
   DefineInitOApply(MinusSignedAssign, MinusAssign)
   DefineInitOApply(MinusUnsignedAssign, MinusAssign)
   DefineInitOApply(MinusFloatAssign, Float)

   DefineInitOApply(TimesSignedAssign, TimesAssign)
   DefineInitOApply(TimesUnsignedAssign, TimesAssign)
   DefineInitOApply(DivideSignedAssign, DivideAssign)
   DefineInitOApply(DivideUnsignedAssign, DivideAssign)
   DefineInitOApply(ModuloSignedAssign, ModuloAssign)
   DefineInitOApply(ModuloUnsignedAssign, ModuloAssign)
   DefineInitApply(OppositeSignedAssign)

   DefineInitOApply(CompareLessSigned, CompareLessOrGreater)
   DefineInitOApply(CompareLessOrEqualSigned, CompareLessOrGreaterOrEqual)
   DefineInitOApply(CompareLessUnsigned, CompareLessOrGreater)
   DefineInitOApply(CompareLessOrEqualUnsigned, CompareLessOrGreaterOrEqual)
   // DefineInitApply(CompareLessFloat)
   // DefineInitApply(CompareLessOrEqualFloat)
   DefineInitApply(CompareEqual)
   // DefineInitApply(CompareEqualFloat)
   DefineInitApply(CompareDifferent)
   // DefineInitApply(CompareDifferentFloat)
   DefineInitOApply(CompareGreaterOrEqualUnsigned, CompareLessOrGreaterOrEqual)
   DefineInitOApply(CompareGreaterUnsigned, CompareLessOrGreater)
   DefineInitOApply(CompareGreaterOrEqualSigned, CompareLessOrGreaterOrEqual)
   DefineInitOApply(CompareGreaterSigned, CompareLessOrGreater)
   // DefineInitApply(CompareGreaterFloat)
   // DefineInitApply(CompareGreaterOrEqualFloat)
   DefineInitOApply(MinSignedAssign, MinMaxAssign)
   DefineInitOApply(MinUnsignedAssign, MinMaxAssign)
   // DefineInitApply(MinFloatAssign)
   DefineInitOApply(MaxSignedAssign, MinMaxAssign)
   DefineInitOApply(MaxUnsignedAssign, MinMaxAssign)
   // DefineInitApply(MaxFloatAssign)

   DefineInitOApply(BitOrAssign, BitBinaryAssign)
   DefineInitOApply(BitAndAssign, BitBinaryAssign)
   DefineInitOApply(BitExclusiveOrAssign, BitBinaryAssign)
   DefineInitOApply(BitNegateAssign, BitUnaryAssign)
   DefineInitBiApply(LeftShiftAssign)
   DefineInitOBiApply(LogicalRightShiftAssign, LogicalRightShiftAssign)
   DefineInitOBiApply(ArithmeticRightShiftAssign, ArithmeticRightShiftAssign)
   DefineInitBiApply(LeftRotateAssign)
   DefineInitBiApply(RightRotateAssign)
#include "StandardClasses/DefineNew.h"
}

#undef DefineInitApply
#undef DefineInitOApply
#undef DefineInitBiApply
#undef DefineInitOBiApply

ConcatOperationElement::MethodApplyTable ConcatOperationElement::matMethodApplyTable;

bool
ConcatOperationElement::applyToConcat(const VirtualOperation& operation, VirtualElement& thisElement, EvaluationEnvironment& env) const {
   TransmitEnvironment applyEnv(env);
   applyEnv.setFirstArgument(fstArg());
   if (!thisElement.apply(operation, applyEnv))
      return false;
   ConcatOperationElement* result = createSCopy();
   env.presult().absorbElement(result);
   if (applyEnv.hasResult())
      result->sfstArg() = applyEnv.presult();
   else
      result->sfstArg().setElement(thisElement);
   return true;
}
   
bool
ConcatOperationElement::applyToBitSet(const VirtualOperation& aoperation, VirtualElement& thisElement, EvaluationEnvironment& env) const {
   AssumeCondition(dynamic_cast<const BitSetOperation*>(&aoperation))
   const BitSetOperation& operation = (const BitSetOperation&) aoperation;
   PPVirtualElement thisLow;
   if (operation.getLowBit() > 0) {
      TransmitEnvironment applyEnv(env);
      ReduceOperation lowReduce;
      assume(thisElement.apply(lowReduce.setLowBit(0).setHighBit(operation.getLowBit()-1).setConstWithAssign(), applyEnv));
      thisLow = applyEnv.presult();
   };
   PPVirtualElement current;
   TransmitEnvironment applyEnv(env);
   Scalar::MultiBit::Operation concatOperation;
   concatOperation.setConcat();
   if (operation.getHighBit()+1 < thisElement.getSizeInBits()) {
      ReduceOperation highReduce;
      highReduce.setLowBit(operation.getHighBit()+1).setHighBit(thisElement.getSizeInBits()-1);
      if (operation.isConstWithAssign())
         highReduce.setConstWithAssign();
      assume(thisElement.apply(highReduce, applyEnv));
      current = applyEnv.presult();
      applyEnv.merge();
      applyEnv.setFirstArgument(fstArg());
      if (!current.isValid()) {
         if (operation.isConstWithAssign())
            concatOperation.setConstWithAssign();
         assume(thisElement.apply(concatOperation, applyEnv));
         current = applyEnv.presult();
         if (concatOperation.isConstWithAssign())
            concatOperation.clearConstWithAssign();
      }
      else {
         assume(current->apply(concatOperation, applyEnv));
         if (applyEnv.presult().isValid())
            current = applyEnv.presult();
      };
      applyEnv.merge();
   }
   else
      current.setElement(fstArg());
   applyEnv.setFirstArgument(sndArg());
   if (!current.isValid()) {
      if (operation.isConstWithAssign())
         concatOperation.setConstWithAssign();
      assume(thisElement.apply(concatOperation, applyEnv));
      if (concatOperation.isConstWithAssign())
         concatOperation.clearConstWithAssign();
      current = applyEnv.presult();
   }
   else {
      assume(current->apply(concatOperation, applyEnv));
      if (applyEnv.presult().isValid())
         current = applyEnv.presult();
   };
   applyEnv.merge();
   if (thisLow.isValid()) {
      applyEnv.setFirstArgument(*thisLow);
      if (!current.isValid()) {
         if (operation.isConstWithAssign())
            concatOperation.setConstWithAssign();
         assume(thisElement.apply(concatOperation, applyEnv));
         if (concatOperation.isConstWithAssign())
            concatOperation.clearConstWithAssign();
         current = applyEnv.presult();
      }
      else {
         assume(current->apply(concatOperation, applyEnv));
         if (applyEnv.presult().isValid())
            current = applyEnv.presult();
      };
      applyEnv.merge();
   };
   if (current.isValid())
      env.presult() = current;
   return true;
}

bool
ConcatOperationElement::applyToCommutativeAssign(const VirtualOperation& aoperation, VirtualElement& thisElement, EvaluationEnvironment& env) const {
   TransmitEnvironment applyEnv(env);
   applyEnv.setFirstArgument(thisElement);
   Operation operation((const Operation&) aoperation);
   if (!operation.isConst())
      operation.setConstWithAssign();
   if (!const_cast<ConcatOperationElement&>(*this).apply(operation, applyEnv))
      return false;
   env.presult() = applyEnv.presult();
   return true;
}

bool
ConcatOperationElement::applyToCommutative(const VirtualOperation& aoperation, VirtualElement& thisElement, EvaluationEnvironment& env) const {
   TransmitEnvironment applyEnv(env);
   applyEnv.setFirstArgument(thisElement);
   Operation operation((const Operation&) aoperation);
   if (!const_cast<ConcatOperationElement&>(*this).apply(operation, applyEnv))
      return false;
   env.presult() = applyEnv.presult();
   return true;
}

bool
ConcatOperationElement::applyToMinusAssign(const VirtualOperation& operation, VirtualElement& thisElement, EvaluationEnvironment& env) const {
   PPVirtualElement argLow = Methods::applyAssign(thisElement, ReduceOperation()
         .setLowBit(0).setHighBit(sndArg().getSizeInBits()-1), env);
   PPVirtualElement argHigh = Methods::applyAssign(thisElement, ReduceOperation()
         .setLowBit(sndArg().getSizeInBits()).setHighBit(getSizeInBits()-1), env);
   PPVirtualElement firstArgument, secondArgument;
   bool mayCarry = false, sureCarry = false;
   {  TransmitEnvironment applyEnv(env);
      applyEnv.setFirstArgument(sndArg());
      if (!argLow->apply(MinusUnsignedAssignOperation(), applyEnv))
         return false;
      if (applyEnv.hasResult())
         secondArgument = applyEnv.presult();
      else
         secondArgument = argLow;
      sureCarry = applyEnv.isSureOverflow();
      mayCarry = applyEnv.isOverflow();
      applyEnv.clearErrors();
   };
   
   {  TransmitEnvironment applyEnv(env);
      applyEnv.setFirstArgument(fstArg());
      if (!argHigh->apply(operation, applyEnv))
         return false;
      if (applyEnv.hasResult())
         firstArgument = applyEnv.presult();
      else
         firstArgument = argHigh;
   };
   if (sureCarry) {
      Operation::Type type = ((const Operation&) operation).getType();
      bool result = false;
      if (type == Operation::TMinusSignedAssign)
         result = firstArgument->apply(PrevSignedAssignOperation(), env);
      else
         result = firstArgument->apply(PrevUnsignedAssignOperation(), env);
      if (!result)
         return false;
      if (env.hasResult())
         firstArgument = env.presult();
   }
   else if (mayCarry) {
      PPVirtualElement copyFst(fstArg());
      {  TransmitEnvironment applyEnv(env);
         Operation::Type type = ((const Operation&) operation).getType();
         if (type == Operation::TMinusSignedAssign)
            copyFst = Methods::applyAssign(copyFst, PrevSignedAssignOperation(), applyEnv);
         else
            copyFst = Methods::applyAssign(copyFst, PrevUnsignedAssignOperation(), applyEnv);
         applyEnv.approximateErrors();
      };
      firstArgument = Methods::merge(firstArgument, *copyFst, env);
      env.mergeVerdictDegradate();
   };
   ConcatOperationElement* result = createSCopy();
   env.presult().absorbElement(result);
   result->sfstArg() = firstArgument;
   result->ssndArg() = secondArgument;
   return true;
}

bool
ConcatOperationElement::applyToDivideAssign(const VirtualOperation& operation, VirtualElement& thisElement, EvaluationEnvironment& env) const {
   PPVirtualElement top = queryTopResult();
   bool result = top->apply(operation, env);
   env.mergeVerdictDegradate();
   if (!env.hasResult())
      env.presult() = top;
   return result;
}

bool
ConcatOperationElement::applyToModuloAssign(const VirtualOperation& operation, VirtualElement& thisElement, EvaluationEnvironment& env) const {
   PPVirtualElement top = queryTopResult();
   bool result = top->apply(operation, env);
   env.mergeVerdictDegradate();
   if (!env.hasResult())
      env.presult() = top;
   return result;
}

bool
ConcatOperationElement::applyToCompareLessOrGreater(const VirtualOperation& aoperation, VirtualElement& thisElement, EvaluationEnvironment& env) const {
   TransmitEnvironment applyEnv(env);
   applyEnv.setFirstArgument(thisElement);
   Operation operation;
   const Operation& originalOperation = (const Operation&) aoperation;
   switch (originalOperation.getType()) {
      case Operation::TCompareLessSigned:
         operation.setCompareGreaterOrEqualSigned(); break;
      case Operation::TCompareLessUnsigned:
         operation.setCompareGreaterOrEqualUnsigned(); break;
      case Operation::TCompareGreaterSigned:
         operation.setCompareLessOrEqualSigned(); break;
      case Operation::TCompareGreaterUnsigned:
         operation.setCompareLessOrEqualUnsigned(); break;
      default:
         {  AssumeUncalled }
   };
   bool result = const_cast<ConcatOperationElement&>(*this).applyCompareLessOrGreaterOrEqual(operation, applyEnv);
   env.presult() = applyEnv.presult();
   return result;
}

bool
ConcatOperationElement::applyToCompareLessOrGreaterOrEqual(const VirtualOperation& aoperation, VirtualElement& thisElement, EvaluationEnvironment& env) const {
   TransmitEnvironment applyEnv(env);
   applyEnv.setFirstArgument(thisElement);
   Operation operation;
   const Operation& originalOperation = (const Operation&) aoperation;
   switch (originalOperation.getType()) {
      case Operation::TCompareLessOrEqualSigned:
         operation.setCompareGreaterSigned(); break;
      case Operation::TCompareLessOrEqualUnsigned:
         operation.setCompareGreaterUnsigned(); break;
      case Operation::TCompareGreaterOrEqualSigned:
         operation.setCompareLessSigned(); break;
      case Operation::TCompareGreaterOrEqualUnsigned:
         operation.setCompareLessUnsigned(); break;
      default:
         {  AssumeUncalled }
   };
   bool result = const_cast<ConcatOperationElement&>(*this).applyCompareLessOrGreaterOrEqual(operation, applyEnv);
   env.presult() = applyEnv.presult();
   return result;
}

bool
ConcatOperationElement::applyToFloat(const VirtualOperation& operation, VirtualElement& thisElement, EvaluationEnvironment& env) const {
   PPVirtualElement top = queryTopResult();
   bool result = top->apply(operation, env);
   env.mergeVerdictDegradate();
   if (!env.hasResult())
      env.presult() = top;
   return result;
}

bool
ConcatOperationElement::applyToShiftAssign(const VirtualOperation& operation, VirtualElement& thisElement, EvaluationEnvironment& env) const {
   PPVirtualElement top = queryTopResult();
   bool result = top->apply(operation, env);
   env.mergeVerdictDegradate();
   if (!env.hasResult())
      env.presult() = top;
   return result;
}

bool
ConcatOperationElement::applyToRotateAssign(const VirtualOperation& operation, VirtualElement& thisElement, EvaluationEnvironment& env) const {
   PPVirtualElement top = queryTopResult();
   bool result = top->apply(operation, env);
   env.mergeVerdictDegradate();
   if (!env.hasResult())
      env.presult() = top;
   return result;
}

#define DefineInitApplyTo(TypeOperation)                                                         \
   elementAt(Operation::T##TypeOperation).setMethod(&ConcatOperationElement::applyTo##TypeOperation);
#define DefineInitOApplyTo(TypeOperation, TypeMethod)                                            \
   elementAt(Operation::T##TypeOperation).setMethod(&ConcatOperationElement::applyTo##TypeMethod);
ConcatOperationElement::MethodApplyToTable::MethodApplyToTable() {
#include "StandardClasses/UndefineNew.h"
   clear();
   DefineInitApplyTo(Concat)
   DefineInitApplyTo(BitSet)
   DefineInitOApplyTo(PlusSignedAssign, CommutativeAssign)
   DefineInitOApplyTo(PlusUnsignedAssign, CommutativeAssign)
   DefineInitOApplyTo(PlusUnsignedWithSignedAssign, CommutativeAssign)
   DefineInitOApplyTo(PlusFloatAssign, Float)
   DefineInitOApplyTo(MinusSignedAssign, MinusAssign)
   DefineInitOApplyTo(MinusUnsignedAssign, MinusAssign)
   DefineInitOApplyTo(MinusFloatAssign, Float)

   DefineInitOApplyTo(CompareLessSigned, CompareLessOrGreater)
   DefineInitOApplyTo(CompareLessUnsigned, CompareLessOrGreater)
   DefineInitOApplyTo(CompareLessFloat, Float)
   DefineInitOApplyTo(CompareLessOrEqualSigned, CompareLessOrGreaterOrEqual)
   DefineInitOApplyTo(CompareLessOrEqualUnsigned, CompareLessOrGreaterOrEqual)
   DefineInitOApplyTo(CompareLessOrEqualFloat, Float)
   DefineInitOApplyTo(CompareEqual, Commutative)
   DefineInitOApplyTo(CompareEqualFloat, Float)
   DefineInitOApplyTo(CompareDifferent, Commutative)
   DefineInitOApplyTo(CompareDifferentFloat, Float)
   DefineInitOApplyTo(CompareGreaterOrEqualSigned, CompareLessOrGreaterOrEqual)
   DefineInitOApplyTo(CompareGreaterOrEqualUnsigned, CompareLessOrGreaterOrEqual)
   DefineInitOApplyTo(CompareGreaterOrEqualFloat, Float)
   DefineInitOApplyTo(CompareGreaterSigned, CompareLessOrGreater)
   DefineInitOApplyTo(CompareGreaterUnsigned, CompareLessOrGreater)
   DefineInitOApplyTo(CompareGreaterFloat, Float)
   DefineInitOApplyTo(MinSignedAssign, CommutativeAssign)
   DefineInitOApplyTo(MinUnsignedAssign, CommutativeAssign)
   DefineInitOApplyTo(MinFloatAssign, CommutativeAssign)
   DefineInitOApplyTo(MaxSignedAssign, CommutativeAssign)
   DefineInitOApplyTo(MaxUnsignedAssign, CommutativeAssign)
   DefineInitOApplyTo(MaxFloatAssign, CommutativeAssign)

   DefineInitOApplyTo(TimesSignedAssign, CommutativeAssign)
   DefineInitOApplyTo(TimesUnsignedAssign, CommutativeAssign)
   DefineInitOApplyTo(TimesFloatAssign, CommutativeAssign)
   DefineInitOApplyTo(DivideSignedAssign, DivideAssign)
   DefineInitOApplyTo(DivideUnsignedAssign, DivideAssign)
   DefineInitOApplyTo(DivideFloatAssign, DivideAssign)
   DefineInitOApplyTo(ModuloSignedAssign, ModuloAssign)
   DefineInitOApplyTo(ModuloUnsignedAssign, ModuloAssign)
   DefineInitOApplyTo(BitOrAssign, CommutativeAssign)
   DefineInitOApplyTo(BitAndAssign, CommutativeAssign)
   DefineInitOApplyTo(BitExclusiveOrAssign, CommutativeAssign)
   DefineInitOApplyTo(LeftShiftAssign, ShiftAssign)
   DefineInitOApplyTo(LogicalRightShiftAssign, ShiftAssign)
   DefineInitOApplyTo(ArithmeticRightShiftAssign, ShiftAssign)
   DefineInitOApplyTo(LeftRotateAssign, RotateAssign)
   DefineInitOApplyTo(RightRotateAssign, RotateAssign)
#include "StandardClasses/DefineNew.h"
}
#undef DefineInitOApplyTo
#undef DefineInitApplyTo

ConcatOperationElement::MethodApplyToTable ConcatOperationElement::mattMethodApplyToTable;

#define DefineInitUnaryConstraint(TypeOperation) \
   mcutUnaryTable[Operation::T##TypeOperation].setMethod(&VirtualElement::constraint##TypeOperation);

#define DefineInitUnaryGConstraint(TypeOperation) \
   mcutUnaryTable[Operation::T##TypeOperation].setFunction(&MultiBit::Approximate::Details::BaseAlgorithms::constraint##TypeOperation);

#define DefineInitUnaryOConstraint(TypeOperation, TypeMethod) \
   mcutUnaryTable[Operation::T##TypeOperation].setMethod(&VirtualElement::constraint##TypeMethod);

#define DefineInitUnaryOGConstraint(TypeOperation, TypeMethod) \
   mcutUnaryTable[Operation::T##TypeOperation].setFunction(&MultiBit::Approximate::Details::BaseAlgorithms::constraint##TypeMethod);

#define DefineInitUnaryOGIntConstraint(TypeOperation, TypeMethod) \
   mcutUnaryTable[Operation::T##TypeOperation].setFunction(&Scalar::Approximate::Details::IntOperationElement::constraint##TypeMethod);

#define DefineInitUnaryGIntConstraint(TypeOperation) \
   mcutUnaryTable[Operation::T##TypeOperation].setFunction(&Scalar::Approximate::Details::IntOperationElement::constraint##TypeOperation);

#define DefineInitBinaryConstraint(TypeOperation) \
   mcbtBinaryTable[Operation::T##TypeOperation].setMethod(&ConcatOperationElement::constraint##TypeOperation);

#define DefineInitBinaryGConstraint(TypeOperation) \
   mcbtBinaryTable[Operation::T##TypeOperation].setFunction(&MultiBit::Approximate::Details::BaseAlgorithms::constraint##TypeOperation);

#define DefineInitBinaryGIntConstraint(TypeOperation) \
   mcbtBinaryTable[Operation::T##TypeOperation].setFunction(&Scalar::Approximate::Details::IntOperationElement::constraint##TypeOperation);

#define DefineInitBinaryOGConstraint(TypeOperation, TypeFunction) \
   mcbtBinaryTable[Operation::T##TypeOperation].setFunction(&MultiBit::Approximate::Details::BaseAlgorithms::constraint##TypeFunction);

#define DefineInitBinaryOGIntConstraint(TypeOperation, TypeFunction) \
   mcbtBinaryTable[Operation::T##TypeOperation].setFunction(&Scalar::Approximate::Details::IntOperationElement::constraint##TypeFunction);

#define DefineInitBinaryConstraintGeneric(TypeOperation)                                         \
   mcbtBinaryTable[Operation::T##TypeOperation].setArray(                                        \
      new Scalar::Approximate::Details::GenericBinaryConstraintMethod<DomainTraits>(&ConcatOperationElement::constraint##TypeOperation));

#define DefineInitOBinaryConstraintGeneric(TypeOperation, TypeMethod)                            \
   mcbtBinaryTable[Operation::T##TypeOperation].setArray(                                        \
      new Scalar::Approximate::Details::GenericBinaryConstraintMethod<DomainTraits>(&ConcatOperationElement::constraint##TypeMethod));

ConcatOperationElement::MethodConstraintTable::MethodConstraintTable() {
#include "StandardClasses/UndefineNew.h"
   DefineInitUnaryGConstraint(ExtendWithZero)
   DefineInitUnaryGConstraint(ExtendWithSign)
   DefineInitBinaryGConstraint(Concat)
   DefineInitUnaryGConstraint(Reduce)
   DefineInitBinaryGConstraint(BitSet)

   DefineInitUnaryOGIntConstraint(CastChar, CastInteger)
   DefineInitUnaryOGIntConstraint(CastInt, CastInteger)
   DefineInitUnaryOGIntConstraint(CastUnsignedInt, CastInteger)
   // DefineInitUnaryOGConstraint(CastFloat, CastDouble)
   // DefineInitUnaryOGConstraint(CastDouble, CastDouble)
   // DefineInitUnaryOGConstraint(CastLongDouble, CastDouble)

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

   DefineInitUnaryOGIntConstraint(PrevSignedAssign, PrevAssign)
   DefineInitUnaryOGIntConstraint(PrevUnsignedAssign, PrevAssign)
   DefineInitUnaryOGIntConstraint(NextSignedAssign, NextAssign)
   DefineInitUnaryOGIntConstraint(NextUnsignedAssign, NextAssign)

   DefineInitBinaryGConstraint(PlusSignedAssign)
   DefineInitBinaryGConstraint(PlusUnsignedAssign)
   DefineInitBinaryGConstraint(PlusUnsignedWithSignedAssign)
   DefineInitBinaryOGIntConstraint(MinusSignedAssign, MinusAssign)
   DefineInitBinaryOGIntConstraint(MinusUnsignedAssign, MinusAssign)
   DefineInitBinaryGConstraint(TimesSignedAssign)
   DefineInitBinaryGConstraint(TimesUnsignedAssign)
   DefineInitBinaryGConstraint(DivideSignedAssign)
   DefineInitBinaryGConstraint(DivideUnsignedAssign)

   DefineInitUnaryOGIntConstraint(OppositeSignedAssign, OppositeAssign)

   DefineInitBinaryGConstraint(ModuloSignedAssign)
   DefineInitBinaryGConstraint(ModuloUnsignedAssign)
   DefineInitBinaryGIntConstraint(BitOrAssign)
   DefineInitBinaryGIntConstraint(BitAndAssign)
   DefineInitBinaryGIntConstraint(BitExclusiveOrAssign)
   DefineInitUnaryGIntConstraint(BitNegateAssign)
   // DefineInitBinaryGConstraint(LeftShiftAssign)
   // DefineInitBinaryGConstraint(LogicalRightShiftAssign)
   // DefineInitBinaryGConstraint(ArithmeticRightShiftAssign)

   // DefineInitBinaryConstraintGeneric(LogicalAndAssign)
   // DefineInitBinaryConstraintGeneric(LogicalOrAssign)
   // DefineInitUnaryGConstraint(LogicalNegateAssign)
#include "StandardClasses/DefineNew.h"
}

#undef DefineInitUnaryConstraint
#undef DefineInitUnaryGConstraint
#undef DefineInitUnaryOGIntConstraint
#undef DefineInitUnaryGIntConstraint
#undef DefineInitUnaryOConstraint
#undef DefineInitUnaryOGConstraint
#undef DefineInitBinaryConstraint
#undef DefineInitBinaryGConstraint
#undef DefineInitBinaryOGConstraint
#undef DefineInitBinaryOGIntConstraint
#undef DefineInitBinaryGIntConstraint
#undef DefineInitBinaryConstraintGeneric
#undef DefineInitOBinaryConstraintGeneric

ConcatOperationElement::MethodConstraintTable ConcatOperationElement::mctMethodConstraintTable;

/***************************************************************/
/* Implementation of the methods ConcatOperationElement::query */
/***************************************************************/

bool
ConcatOperationElement::queryCompareSpecial(const VirtualElement& thisElement, const VirtualQueryOperation&, VirtualQueryOperation::Environment& env) {
   ((QueryOperation::CompareSpecialEnvironment&) env).fill();
   return true;
}

bool
ConcatOperationElement::querySimplification(const VirtualElement& athisElement, const VirtualQueryOperation&, VirtualQueryOperation::Environment& aenv) {
   AssumeCondition(dynamic_cast<const ConcatOperationElement*>(&athisElement)
         && dynamic_cast<const SimplificationEnvironment*>(&aenv))
   ConcatOperationElement& thisElement = const_cast<ConcatOperationElement&>((ConcatOperationElement&) athisElement);
   ConcatenationCursor cursor(thisElement);
   cursor.setToFirst();
   const VirtualElement* previous = &cursor.elementAt();
   ApproxKind kind = previous->getApproxKind();
   while (cursor.setToNext()) {
      ApproxKind nextKind = cursor.elementAt().getApproxKind();
      if (kind == nextKind) {
         EvaluationEnvironment applyEnv = EvaluationEnvironment(EvaluationParameters());
         applyEnv.setFirstArgument(*previous);
         const_cast<VirtualElement&>(cursor.elementAt()).apply(ConcatOperation(), applyEnv);
         if (applyEnv.hasResult())
            cursor.replaceElementAtWith((VirtualElement*) applyEnv.presult().extractElement());
         VirtualElement* newThis = cursor.freeBeforeAndReplaceRoot();
         if (newThis) {
            SimplificationEnvironment& env = (SimplificationEnvironment&) aenv;
            env.presult().absorbElement(newThis);
         };
      };
   };
   return true;
}

bool
ConcatOperationElement::querySimplificationAsConstantDisjunction(const VirtualElement& thisElement, const VirtualQueryOperation& operation, VirtualQueryOperation::Environment& aenv) {
   AssumeCondition(dynamic_cast<const ConcatOperationElement*>(&thisElement))
   const ConcatOperationElement& element = (const ConcatOperationElement&) thisElement;
   SimplificationEnvironment& env = ((SimplificationEnvironment&) aenv);
   if (element.fstArg().getApproxKind().isConstant()) {
      assume(element.sndArg().query(operation, aenv));
      if (!env.hasFailed()) {
         if (env.hasResult())
            env.presult() = Methods::applyAssign(element.fstArg(), ConcatOperation(), *env.presult(), EvaluationParameters());
         else
            env.presult() = Methods::applyAssign(element.fstArg(), ConcatOperation(), element.sndArg(), EvaluationParameters());
      };
   }
   else
      env.setFail();
   return true;
}

bool
ConcatOperationElement::querySimplificationAsInterval(const VirtualElement& thisElement,
      const VirtualQueryOperation& operation, VirtualQueryOperation::Environment& aenv) {
   AssumeCondition(dynamic_cast<const ConcatOperationElement*>(&thisElement))
   const ConcatOperationElement& element = (const ConcatOperationElement&) thisElement;
   if (!element.fstArg().query(operation, aenv))
      return false;
   SimplificationEnvironment& env = ((SimplificationEnvironment&) aenv);
   if (env.hasFailed())
      return true;
   typedef Scalar::Approximate::Details::VirtualIntegerInterval VirtualIntegerInterval;
   VirtualIntegerInterval* result;
   if (env.hasResult()) {
      AssumeCondition(dynamic_cast<const VirtualIntegerInterval*>(env.presult().key()))
      result = &(VirtualIntegerInterval&) *env.presult();
   }
   else {
      AssumeCondition(dynamic_cast<const VirtualIntegerInterval*>(&element.fstArg()))
      result = &(VirtualIntegerInterval&) element.fstArg();
   };
   int shiftValue = element.sndArg().getSizeInBits();
   PPVirtualElement shift = Methods::newIntForShift(element, shiftValue);
   PPVirtualElement& min = result->getSMin(), &max = result->getSMax();
   min = Methods::applyAssign(min, Scalar::MultiBit::ExtendOperation().setExtendWithZero(shiftValue), EvaluationParameters());
   min = Methods::applyAssign(min,
      Scalar::MultiBit::LeftShiftAssignOperation(), *shift, EvaluationParameters());
   max = Methods::applyAssign(max, Scalar::MultiBit::ExtendOperation().setExtendWithZero(shiftValue), EvaluationParameters());
   max = Methods::applyAssign(max,
      Scalar::MultiBit::LeftShiftAssignOperation(), *shift, EvaluationParameters());
   PPVirtualElement zeroComplement = Methods::newZero(element.sndArg());
   zeroComplement = Methods::applyAssign(zeroComplement, Scalar::MultiBit::BitNegateAssignOperation(), EvaluationParameters());
   max = Methods::applyAssign(max,
      Scalar::MultiBit::BitOrAssignOperation(), *zeroComplement, EvaluationParameters());
   if (!env.hasResult())
      env.presult() = const_cast<ConcatOperationElement&>(element).sfstArg();
   return true;
}

bool
ConcatOperationElement::queryDispatchOnTopResult(const VirtualElement& thisElement, const VirtualQueryOperation& operation, VirtualQueryOperation::Environment& aenv) {
   AssumeCondition(dynamic_cast<const ConcatOperationElement*>(&thisElement))
   return ((const ConcatOperationElement&) thisElement).queryTopResult()->query(operation, aenv);
}

bool
ConcatOperationElement::queryDispatchOnBothPart(const VirtualElement& thisElement, const VirtualQueryOperation& operation, VirtualQueryOperation::Environment& aenv) {
   AssumeCondition(dynamic_cast<const ConcatOperationElement*>(&thisElement))
   if (!((const ConcatOperationElement&) thisElement).fstArg().query(operation, aenv))
      return false;
   if (!((const ConcatOperationElement&) thisElement).sndArg().query(operation, aenv))
      return false;
   return true;
}

#define DefineInitQuery(TypeOperation) \
   elementAt(QueryOperation::TypeOperation).setMethod(&ConcatOperationElement::query##TypeOperation);

ConcatOperationElement::QueryTable::QueryTable() {
#include "StandardClasses/UndefineNew.h"
   FunctionQueryTable* table = &elementAt(VirtualQueryOperation::TCompareSpecial);
   table->setSize(QueryOperation::EndOfTypeCompareSpecial);
   (*table)[QueryOperation::TCSGuard].setMethod(&ConcatOperationElement::queryGuardAll);
   (*table)[QueryOperation::TCSNatural].setMethod(&ConcatOperationElement::queryCompareSpecial);
   (*table)[QueryOperation::TCSSigned].setMethod(&ConcatOperationElement::queryCompareSpecial);
   (*table)[QueryOperation::TCSUnsigned].setMethod(&ConcatOperationElement::queryCompareSpecial);
   (*table)[QueryOperation::TCSBitDomain].setMethod(&ConcatOperationElement::queryDispatchOnTopResult);
   
   table = &elementAt(VirtualQueryOperation::TSimplification);
   table->setSize(QueryOperation::EndOfTypeSimplification);
   (*table)[QueryOperation::TSConstant].setMethod(&Top::queryFailSimplification);
   (*table)[QueryOperation::TSInterval].setMethod(&ConcatOperationElement::querySimplificationAsInterval);
   (*table)[QueryOperation::TSConstantDisjunction].setMethod(&ConcatOperationElement::querySimplificationAsConstantDisjunction);

   table = &elementAt(VirtualQueryOperation::TDomain);
   table->setSize(QueryOperation::EndOfTypeDomain);

#define DefineInitQueryMethodTable(TypeOperation)                                                \
   (*table)[QueryOperation::TD##TypeOperation].setMethod(&ConcatOperationElement::queryDispatchOnTopResult);
   
   DefineInitQueryMethodTable(Disjunction)
   DefineInitQueryMethodTable(Interval)
   DefineInitQueryMethodTable(BooleanDisjunction)
   DefineInitQueryMethodTable(Top)
   DefineInitQueryMethodTable(False)
   DefineInitQueryMethodTable(True)
   DefineInitQueryMethodTable(IntForShift)
   DefineInitQueryMethodTable(Zero)
   DefineInitQueryMethodTable(MinusOne)
   DefineInitQueryMethodTable(One)
   DefineInitQueryMethodTable(Min)
   DefineInitQueryMethodTable(Max)

#undef DefineInitQueryMethodTable

   table = &elementAt(VirtualQueryOperation::TExtern);
   table->setSize(QueryOperation::EndOfTypeExtern);
   (*table)[QueryOperation::TESigned].setMethod(&ConcatOperationElement::queryDispatchOnTopResult);
#include "StandardClasses/DefineNew.h"
}
#undef DefineInitQuery

ConcatOperationElement::QueryTable ConcatOperationElement::mqtMethodQueryTable;

#define DefineTypeOperation MultiBit
#define DefineTypeObject ConcatOperationElement
#include "Analyzer/Scalar/Approximate/VirtualCall.incc"
#undef DefineTypeObject
#undef DefineTypeOperation

bool
ConcatOperationElement::mergeWith(const Scalar::VirtualElement& asource, Scalar::EvaluationEnvironment& aenv) {
   if (VirtualElement::mergeWith(asource, aenv))
      return true;
   EvaluationEnvironment& env = (EvaluationEnvironment&) aenv;
   const VirtualElement& source = (const VirtualElement&) asource;
   PPVirtualElement argLow = Methods::applyAssign(source, ReduceOperation()
         .setLowBit(0).setHighBit(sndArg().getSizeInBits()-1), env);
   PPVirtualElement argHigh = Methods::applyAssign(source, ReduceOperation()
         .setLowBit(sndArg().getSizeInBits()).setHighBit(getSizeInBits()-1), env);
   {  TransmitEnvironment applyEnv(env);
      if (!sndArg().mergeWith(*argLow, applyEnv))
         return false;
      if (applyEnv.hasResult())
         ssndArg() = applyEnv.presult();
   };
   {  TransmitEnvironment applyEnv(env);
      if (!fstArg().mergeWith(*argHigh, applyEnv))
         return false;
      if (applyEnv.hasResult())
         sfstArg() = applyEnv.presult();
   };
   return true;
}

bool
ConcatOperationElement::contain(const Scalar::VirtualElement& asource, Scalar::EvaluationEnvironment& aenv) const {
   EvaluationEnvironment& env = (EvaluationEnvironment&) aenv;
   const VirtualElement& source = (const VirtualElement&) asource;
   PPVirtualElement argLow = Methods::applyAssign(source, ReduceOperation()
         .setLowBit(0).setHighBit(sndArg().getSizeInBits()-1), env);
   PPVirtualElement argHigh = Methods::applyAssign(source, ReduceOperation()
         .setLowBit(sndArg().getSizeInBits()).setHighBit(getSizeInBits()-1), env);
   {  TransmitEnvironment applyEnv(env);
      if (!sndArg().contain(*argLow, applyEnv))
         return false;
      if (applyEnv.isNoneApplied())
         return true;
   };
   {  TransmitEnvironment applyEnv(env);
      if (!fstArg().contain(*argHigh, applyEnv))
         return false;
      if (applyEnv.isNoneApplied())
         return true;
   };
   return true;
}

bool
ConcatOperationElement::mergeWithTo(VirtualElement& source, EvaluationEnvironment& env) const {
   PPVirtualElement argLow = Methods::applyAssign(source, ReduceOperation()
         .setLowBit(0).setHighBit(sndArg().getSizeInBits()-1), env);
   PPVirtualElement argHigh = Methods::applyAssign(source, ReduceOperation()
         .setLowBit(sndArg().getSizeInBits()).setHighBit(getSizeInBits()-1), env);
   ConcatOperationElement* result = createSCopy();
   env.presult().absorbElement(result);
   {  TransmitEnvironment applyEnv(env);
      if (!result->sndArg().mergeWith(*argLow, applyEnv))
         return false;
      if (applyEnv.hasResult())
         result->ssndArg() = applyEnv.presult();
   };
   {  TransmitEnvironment applyEnv(env);
      if (!result->fstArg().mergeWith(*argHigh, applyEnv))
         return false;
      if (applyEnv.hasResult())
         result->sfstArg() = applyEnv.presult();
   };
   return true;
}

bool
ConcatOperationElement::containTo(const VirtualElement& source, EvaluationEnvironment& env) const {
   PPVirtualElement argLow = Methods::applyAssign(source, ReduceOperation()
         .setLowBit(0).setHighBit(sndArg().getSizeInBits()-1), env);
   PPVirtualElement argHigh = Methods::applyAssign(source, ReduceOperation()
         .setLowBit(sndArg().getSizeInBits()).setHighBit(getSizeInBits()-1), env);
   {  TransmitEnvironment applyEnv(env);
      if (!argLow->contain(sndArg(), applyEnv))
         return false;
      if (applyEnv.isNoneApplied())
         return true;
   };
   {  TransmitEnvironment applyEnv(env);
      if (!argHigh->contain(fstArg(), applyEnv))
         return false;
      if (applyEnv.isNoneApplied())
         return true;
   };
   return true;
}

bool
ConcatOperationElement::intersectWith(const VirtualElement& source, EvaluationEnvironment& env) {
   if (VirtualElement::intersectWith(source, env))
      return true;
   PPVirtualElement argLow = Methods::applyAssign(source, ReduceOperation()
         .setLowBit(0).setHighBit(sndArg().getSizeInBits()-1), env);
   PPVirtualElement argHigh = Methods::applyAssign(source, ReduceOperation()
         .setLowBit(sndArg().getSizeInBits()).setHighBit(getSizeInBits()-1), env);
   {  TransmitEnvironment applyEnv(env);
      if (!sndArg().intersectWith(*argLow, applyEnv))
         return false;
      if (applyEnv.hasResult())
         ssndArg() = applyEnv.presult();
   };
   if (env.isEmpty())
      return true;
   {  TransmitEnvironment applyEnv(env);
      if (!fstArg().intersectWith(*argHigh, applyEnv))
         return false;
      if (applyEnv.hasResult())
         sfstArg() = applyEnv.presult();
   };
   return true;
}

bool
ConcatOperationElement::intersectWithTo(VirtualElement& source, EvaluationEnvironment& env) const {
   PPVirtualElement argLow = Methods::applyAssign(source, ReduceOperation()
         .setLowBit(0).setHighBit(sndArg().getSizeInBits()-1), env);
   PPVirtualElement argHigh = Methods::applyAssign(source, ReduceOperation()
         .setLowBit(sndArg().getSizeInBits()).setHighBit(getSizeInBits()-1), env);
   ConcatOperationElement* result = createSCopy();
   env.presult().absorbElement(result);
   {  TransmitEnvironment applyEnv(env);
      if (!result->sndArg().intersectWith(*argLow, applyEnv))
         return false;
      if (applyEnv.hasResult())
         result->ssndArg() = applyEnv.presult();
   };
   {  TransmitEnvironment applyEnv(env);
      if (!result->fstArg().intersectWith(*argHigh, applyEnv))
         return false;
      if (applyEnv.hasResult())
         result->sfstArg() = applyEnv.presult();
   };
   return true;
}

/******************************************************/
/* Implementation of the class ExtendOperationElement */
/******************************************************/

PPVirtualElement
ExtendOperationElement::queryTopResult() const {
   PPVirtualElement fstTop = Methods::newTop(fstArg());
   AssumeCondition(dynamic_cast<const Top*>(fstTop.key()))
   return PPVirtualElement(new Top(Top::Init().setTable(*this)
         .setBitSize(getSizeInBits()).setApplyTop(((const Top&) *fstTop).getApplyFunction())
         .setApplyToTop(((const Top&) *fstTop).getApplyToFunction())
         .setConstraintTop(((const Top&) *fstTop).getConstraintFunction())), PNT::Pointer::Init());
}

bool
ExtendOperationElement::applyExtend(const VirtualOperation& aoperation, EvaluationEnvironment& env) {
   AssumeCondition(dynamic_cast<const ExtendOperation*>(&aoperation))
   ExtendOperation& thisOperation = (ExtendOperation&) *ppvoOperation;
   const ExtendOperation& sourceOperation = (const ExtendOperation&) aoperation;
   if (sourceOperation.getType() == Operation::TExtendWithSign
         || thisOperation.getType() == Operation::TExtendWithZero)
      thisOperation.sextension() += sourceOperation.getExtension();
   else {
      TransmitEnvironment applyEnv(env);
      if (!fstArg().apply(sourceOperation, applyEnv))
         return false;
      if (applyEnv.hasResult())
         sfstArg() = applyEnv.presult();
      thisOperation = sourceOperation;
   };
   return true;
}

PPVirtualElement
ExtendOperationElement::simplify(EvaluationEnvironment& env) {
   TransmitEnvironment applyEnv(env);
   if (env.getLatticeCreation().mayBeShareTop())
      applyEnv.setTopLatticeCreation();
   assume(fstArg().apply(getOperation(), applyEnv));
   return applyEnv.hasResult() ? applyEnv.presult() : sfstArg();
}

PPVirtualElement
ExtendOperationElement::simplifyAsConcatenation(EvaluationEnvironment& env) {
   FormalImplementation::Init init;
   init.setBitSize(getSizeInBits());
   ConcatOperationElement* result = new ConcatOperationElement(init);
   PPVirtualElement extension;
   if (getOperation().getType() == Operation::TExtendWithZero) {
      DomainEnvironment extensionEnvironment;
      extensionEnvironment.setInit(VirtualElement::Init().setBitSize(getOperation().getExtension()));
      query(QueryOperation().setNewZero(), extensionEnvironment);
      extension = extensionEnvironment.presult();
   };
   if (getOperation().getType() == Operation::TExtendWithSign) {
      CastShiftBitOperation signOperation(fstArg().getSizeInBits()-1);
      TransmitEnvironment signEnvironment(env);
      sfstArg()->apply(signOperation, signEnvironment);
      ZeroResult sign = signEnvironment.presult()->queryZeroResult();
      PPVirtualElement zero, minusOne;
      if (sign.mayBeZero()) {
         DomainEnvironment extensionEnvironment;
         extensionEnvironment.setInit(VirtualElement::Init().setBitSize(getOperation().getExtension()));
         query(QueryOperation().setNewZero(), extensionEnvironment);
         zero = extensionEnvironment.presult();
      }
      if (sign.mayBeDifferentZero()) {
         DomainEnvironment extensionEnvironment;
         extensionEnvironment.setInit(VirtualElement::Init().setBitSize(getOperation().getExtension()));
         query(QueryOperation().setNewMinusOne(), extensionEnvironment);
         minusOne = extensionEnvironment.presult();
      };
      if (zero.isValid()) {
         if (minusOne.isValid())
            extension = Methods::merge(zero, *minusOne, EvaluationParameters(env));
         else
            extension = zero;
      }
      else {
         AssumeCondition(minusOne.isValid())
         extension = minusOne;
      };
   };
   result->sfstArg() = extension;
   result->ssndArg() = sfstArg();
   return PPVirtualElement(result, PNT::Pointer::Init());
}

PPVirtualElement
ExtendOperationElement::simplify(EvaluationEnvironment& env) const {
   TransmitEnvironment applyEnv(env);
   if (env.getLatticeCreation().mayBeShareTop())
      applyEnv.setTopLatticeCreation();
   PPVirtualElement argCopy(fstArg());
   assume(argCopy->apply(getOperation(), applyEnv));
   return applyEnv.hasResult() ? applyEnv.presult() : argCopy;
}

bool
ExtendOperationElement::applyDefaultUnaryAssign(const VirtualOperation& operation, EvaluationEnvironment& env) {
   PPVirtualElement simplification = simplify(env);
   TransmitEnvironment applyEnv(env);
   assume(simplification->apply(operation, applyEnv));
   if (applyEnv.hasResult())
      env.presult() = applyEnv.presult();
   else
      env.presult() = simplification;
   if (env.getLatticeCreation().mayBeShareTop()
         && !((const VirtualElement&) *env.presult()).getApproxKind().isVariable()) {
      PPVirtualElement domain = env.presult();
      int sizeInBits = domain->getSizeInBits();
      ExactToApproximateElement* result = new ExactToApproximateElement(
            VirtualElement::Init().setBitSize(sizeInBits), domain);
      env.presult().absorbElement(result);
   };
   return true;
}

bool
ExtendOperationElement::applyDefaultUnary(const VirtualOperation& operation, EvaluationEnvironment& env) {
   PPVirtualElement simplification = ((const ExtendOperationElement&) *this).simplify(env);
   TransmitEnvironment applyEnv(env);
   assume(simplification->apply(operation, applyEnv));
   env.presult() = applyEnv.presult();
   if (env.getLatticeCreation().mayBeShareTop()
         && !((const VirtualElement&) *env.presult()).getApproxKind().isVariable()) {
      PPVirtualElement domain = env.presult();
      int sizeInBits = domain->getSizeInBits();
      ExactToApproximateElement* result = new ExactToApproximateElement(
            VirtualElement::Init().setBitSize(sizeInBits), domain);
      env.presult().absorbElement(result);
   };
   return true;
}

bool
ExtendOperationElement::applyDefaultBinaryAssign(const VirtualOperation& operation, EvaluationEnvironment& env) {
   PPVirtualElement simplification = simplify(env);
   TransmitEnvironment applyEnv(env);
   applyEnv.setFirstArgument(env.getFirstArgument());
   assume(simplification->apply(operation, applyEnv));
   if (applyEnv.hasResult())
      env.presult() = applyEnv.presult();
   else
      env.presult() = simplification;
   if (env.getLatticeCreation().mayBeShareTop()
         && !((const VirtualElement&) *env.presult()).getApproxKind().isVariable()) {
      PPVirtualElement domain = env.presult();
      int sizeInBits = domain->getSizeInBits();
      ExactToApproximateElement* result = new ExactToApproximateElement(
            VirtualElement::Init().setBitSize(sizeInBits), domain);
      env.presult().absorbElement(result);
   };
   return true;
}

bool
ExtendOperationElement::applyDefaultBinary(const VirtualOperation& operation, EvaluationEnvironment& env) {
   PPVirtualElement simplification = ((const ExtendOperationElement&) *this).simplify(env);
   TransmitEnvironment applyEnv(env);
   applyEnv.setFirstArgument(env.getFirstArgument());
   assume(simplification->apply(operation, applyEnv));
   env.presult() = applyEnv.presult();
   if (env.getLatticeCreation().mayBeShareTop()
         && !((const VirtualElement&) *env.presult()).getApproxKind().isVariable()) {
      PPVirtualElement domain = env.presult();
      int sizeInBits = domain->getSizeInBits();
      ExactToApproximateElement* result = new ExactToApproximateElement(
            VirtualElement::Init().setBitSize(sizeInBits), domain);
      env.presult().absorbElement(result);
   };
   return true;
}

bool
ExtendOperationElement::applyReduce(const VirtualOperation& aoperation, EvaluationEnvironment& env) {
   AssumeCondition(dynamic_cast<const ReduceOperation*>(&aoperation))
   const ReduceOperation& operation = (const ReduceOperation&) aoperation;

   int size = fstArg().getSizeInBits();
   bool result = false;
   if (operation.getLowBit() < size) {
      if (operation.getHighBit() < size) {
         result = fstArg().apply(aoperation, env);
         if (!env.hasResult())
            env.presult() = sfstArg();
      }
      else {
         ExtendOperation& thisOperation = (ExtendOperation&) *ppvoOperation;
         thisOperation.sextension() = (operation.getHighBit() + 1 - size);
         if (operation.getLowBit() > 0) {
            ReduceOperation fstOperation(operation);
            fstOperation.getSHighBit() = size - 1;
            {  TransmitEnvironment applyEnv(env);
               result = fstArg().apply(fstOperation, applyEnv);
               if (!result)
                  return false;
               if (applyEnv.hasResult())
                  sfstArg() = applyEnv.presult();
            };
         };
      };
      result = true;
   }
   else { // operation.getLowBit() >= size
      PPVirtualElement extension;
      if (getOperation().getType() == Operation::TExtendWithZero) {
         DomainEnvironment extensionEnvironment;
         extensionEnvironment.setInit(VirtualElement::Init().setBitSize(
            operation.getHighBit() - operation.getLowBit() + 1));
         query(QueryOperation().setNewZero(), extensionEnvironment);
         extension = extensionEnvironment.presult();
      }
      else {
         AssumeCondition(getOperation().getType() == Operation::TExtendWithSign);
         CastShiftBitOperation signOperation(fstArg().getSizeInBits()-1);
         TransmitEnvironment signEnvironment(env);
         sfstArg()->apply(signOperation, signEnvironment);
         ZeroResult sign = signEnvironment.presult()->queryZeroResult();
         PPVirtualElement zero, minusOne;
         if (sign.mayBeZero()) {
            DomainEnvironment extensionEnvironment;
            extensionEnvironment.setInit(VirtualElement::Init().setBitSize(
               operation.getHighBit() - operation.getLowBit() + 1));
            query(QueryOperation().setNewZero(), extensionEnvironment);
            zero = extensionEnvironment.presult();
         }
         if (sign.mayBeDifferentZero()) {
            DomainEnvironment extensionEnvironment;
            extensionEnvironment.setInit(VirtualElement::Init().setBitSize(
               operation.getHighBit() - operation.getLowBit() + 1));
            query(QueryOperation().setNewMinusOne(), extensionEnvironment);
            minusOne = extensionEnvironment.presult();
         };
         if (zero.isValid()) {
            if (minusOne.isValid())
               extension = Methods::merge(zero, *minusOne, EvaluationParameters(env));
            else
               extension = zero;
         }
         else {
            AssumeCondition(minusOne.isValid())
            extension = minusOne;
         };
      };
      env.presult() = extension;
      result = true;
   };
   return result;
}
   
bool
ExtendOperationElement::applyBitSet(const VirtualOperation& aoperation, EvaluationEnvironment& env) {
   AssumeCondition(dynamic_cast<const BitSetOperation*>(&aoperation))
   const BitSetOperation& operation = (const BitSetOperation&) aoperation;
   int size = fstArg().getSizeInBits();
   bool hasApplied = false;
   if (operation.getLowBit() < size) {
      if (operation.getHighBit() < size) {
         ExtendOperation& thisOperation = (ExtendOperation&) *ppvoOperation;
         if ((operation.getHighBit() < size-1) || thisOperation.getType() == Operation::TExtendWithZero) {
            assume(fstArg().apply(aoperation, env));
            if (env.hasResult())
               sfstArg() = env.presult();
            hasApplied = true;
         };
      };
   };
   if (!hasApplied) {
      // could use concatenations
      PPVirtualElement simplification = simplify(env);
      TransmitEnvironment applyEnv(env);
      assume(simplification->apply(aoperation, applyEnv));
      if (applyEnv.hasResult())
         env.presult() = applyEnv.presult();
      else
         env.presult() = simplification;
      if (env.getLatticeCreation().mayBeShareTop()
            && !((const VirtualElement&) *env.presult()).getApproxKind().isVariable()) {
         PPVirtualElement domain = env.presult();
         int sizeInBits = domain->getSizeInBits();
         ExactToApproximateElement* result = new ExactToApproximateElement(
               VirtualElement::Init().setBitSize(sizeInBits), domain);
         env.presult().absorbElement(result);
      };
   }
   return true;
}
   
bool
ExtendOperationElement::applyAsConcatenationAssign(const VirtualOperation& operation,
      EvaluationEnvironment& env) {
   PPVirtualElement simplification = simplifyAsConcatenation(env);
   TransmitEnvironment applyEnv(env);
   applyEnv.setFirstArgument(env.getFirstArgument());
   assume(simplification->apply(operation, applyEnv));
   if (applyEnv.hasResult())
      env.presult() = applyEnv.presult();
   else
      env.presult() = simplification;
   return true;
}

bool
ExtendOperationElement::applyMinusUnsignedAssign(const VirtualOperation& operation,
      EvaluationEnvironment& env) {
   if (getOperation().getType() == Operation::TExtendWithZero) {
      ReduceOperation reduction;
      int lowBit = fstArg().getSizeInBits();
      reduction.setLowBit(lowBit).setHighBit(lowBit + getOperation().getExtension() -1);
      PPVirtualElement sourceExtension = Methods::apply(env.getFirstArgument(), reduction.setConstWithAssign(), EvaluationParameters(env));
      if (sourceExtension->queryZeroResult().isZero()) {
         ReduceOperation lowReduction;
         lowReduction.setLowBit(0).setHighBit(lowBit - 1);
         TransmitEnvironment applyEnv(env);
         env.getFirstArgument().apply(lowReduction.setConstWithAssign(), applyEnv);
         PPVirtualElement sourceReduced = applyEnv.presult();
         applyEnv.merge();

         applyEnv.setFirstArgument(*sourceReduced);
         assume(fstArg().apply(Operation().setCompareLessUnsigned(), applyEnv));
         PPVirtualElement newExtension = applyEnv.presult(); 
         applyEnv.merge();
         assume(newExtension->apply(Scalar::Bit::CastMultiBitOperation().setSize(1), applyEnv));
         newExtension = applyEnv.presult();
         applyEnv.merge();

         applyEnv.setFirstArgument(*sourceReduced);
         assume(fstArg().apply(operation, applyEnv));
         if (applyEnv.hasResult())
            sfstArg() = applyEnv.presult();
         applyEnv.merge();

         // assume(sourceReduced->apply(Operation().setOppositeSignedAssign(), applyEnv));
         // if (applyEnv.hasResult())
         //    sourceReduced = applyEnv.presult();
         // applyEnv.merge();
         // applyEnv.setFirstArgument(*sourceReduced);
         // assume(fstArg().apply(Operation().setCompareGreaterUnsigned(), applyEnv));
         // PPVirtualElement newExtension = applyEnv.presult(); 
         // applyEnv.merge();
         // assume(newExtension->apply(Scalar::Bit::CastMultiBitOperation().setSize(1), applyEnv));
         // newExtension = applyEnv.presult();
         // applyEnv.merge();

         applyEnv.setFirstArgument(fstArg());
         newExtension->apply(Operation().setConcat(), applyEnv);
         if (applyEnv.hasResult())
            newExtension = applyEnv.presult();
         if (getOperation().getExtension() <= 1)
            env.presult() = newExtension;
         else {
            sfstArg() = newExtension;
            ExtendOperation newOperation;
            newOperation.setExtendWithSign(getOperation().getExtension()-1);
            ppvoOperation.setElement(newOperation);
         };
         return true; 
      };
   }
   PPVirtualElement simplification = simplifyAsConcatenation(env);
   TransmitEnvironment applyEnv(env);
   applyEnv.setFirstArgument(env.getFirstArgument());
   assume(simplification->apply(operation, applyEnv));
   if (applyEnv.hasResult())
      env.presult() = applyEnv.presult();
   else
      env.presult() = simplification;
   return true;
}

bool
ExtendOperationElement::applyCastShiftBit(const VirtualOperation& aoperation, EvaluationEnvironment& env) {
   AssumeCondition(dynamic_cast<const CastShiftBitOperation*>(&aoperation))
   int shift = ((const CastShiftBitOperation&) aoperation).getShift();
   int size = fstArg().getSizeInBits();
   bool result = false;
   if (shift < size)
      result = fstArg().apply(aoperation, env);
   else {
      ExtendOperation& thisOperation = (ExtendOperation&) *ppvoOperation;
      if (thisOperation.getType() == Operation::TExtendWithZero)
         env.presult() = Methods::newFalse(fstArg());
      else {
         AssumeCondition(size > 0)
         CastShiftBitOperation modifiedOperation((const CastShiftBitOperation&) aoperation);
         modifiedOperation.getSShift() = size-1;
         result = fstArg().apply(modifiedOperation, env);
      };
   };
   return result;
}

bool
ExtendOperationElement::applyCompareExtend(const VirtualOperation& operation,
      const ExtendOperationElement& source, EvaluationEnvironment& env) {
   const ExtendOperation& thisOperation = getOperation();
   const ExtendOperation& sourceOperation = source.getOperation();

   if (thisOperation.getType() == sourceOperation.getType()) {
      if (thisOperation.getExtension() == sourceOperation.getExtension()) {
         TransmitEnvironment applyEnv(env);
         applyEnv.setFirstArgument(source.fstArg());
         assume(fstArg().apply(operation, applyEnv));
         env.presult() = applyEnv.presult();
         return true;
      }
      if (thisOperation.getExtension() < sourceOperation.getExtension()) {
         ExtendOperation localSourceOperation(sourceOperation);
         localSourceOperation.sextension() -= thisOperation.getExtension();
         PPVirtualElement simplifiedSource;
         {  TransmitEnvironment applyEnv(env);
            if (env.getLatticeCreation().mayBeShareTop())
               applyEnv.setTopLatticeCreation();
            assume(source.fstArg().apply(localSourceOperation.setConstWithAssign(), applyEnv));
            simplifiedSource = applyEnv.presult();
         };
         TransmitEnvironment applyEnv(env);
         applyEnv.setFirstArgument(*simplifiedSource);
         assume(fstArg().apply(operation, applyEnv));
         env.presult() = applyEnv.presult();
         return true;
      };
      // thisOperation.getExtension() > sourceOperation.getExtension()
      {  ExtendOperation localThisOperation(thisOperation);
         localThisOperation.sextension() -= sourceOperation.getExtension();
         PPVirtualElement simplifiedThis;
         {  TransmitEnvironment applyEnv(env);
            if (env.getLatticeCreation().mayBeShareTop())
               applyEnv.setTopLatticeCreation();
            assume(fstArg().apply(localThisOperation.setConstWithAssign(), applyEnv));
            simplifiedThis = applyEnv.presult();
         };
         TransmitEnvironment applyEnv(env);
         applyEnv.setFirstArgument(source.fstArg());
         assume(simplifiedThis->apply(operation, applyEnv));
         env.presult() = applyEnv.presult();
         return true;
      };
   };
   PPVirtualElement thisSimplified = ((const ExtendOperationElement&) *this).simplify(env);
   PPVirtualElement sourceSimplified = source.simplify(env);
   TransmitEnvironment applyEnv(env);
   applyEnv.setFirstArgument(*sourceSimplified);
   assume(thisSimplified->apply(operation, applyEnv));
   env.presult() = applyEnv.presult();
   if (env.getLatticeCreation().mayBeShareTop()
         && !((const VirtualElement&) *env.presult()).getApproxKind().isVariable()) {
      PPVirtualElement domain = env.presult();
      int sizeInBits = domain->getSizeInBits();
      ExactToApproximateElement* result = new ExactToApproximateElement(
            VirtualElement::Init().setBitSize(sizeInBits), domain);
      env.presult().absorbElement(result);
   };
   return true;
}

bool
ExtendOperationElement::applyMinMaxAssignExtend(const VirtualOperation& operation,
      const ExtendOperationElement& source, EvaluationEnvironment& env) {
   const ExtendOperation& thisOperation = getOperation();
   const ExtendOperation& sourceOperation = source.getOperation();

   if (thisOperation.getType() == sourceOperation.getType()) {
      if (thisOperation.getExtension() == sourceOperation.getExtension()) {
         TransmitEnvironment applyEnv(env);
         applyEnv.setFirstArgument(source.fstArg());
         assume(fstArg().apply(operation, applyEnv));
         if (applyEnv.hasResult())
            sfstArg() = applyEnv.presult();
         return true;
      }
      if (thisOperation.getExtension() < sourceOperation.getExtension()) {
         ExtendOperation localSourceOperation(sourceOperation);
         localSourceOperation.sextension() -= thisOperation.getExtension();
         PPVirtualElement simplifiedSource;
         {  TransmitEnvironment applyEnv(env);
            if (env.getLatticeCreation().mayBeShareTop())
               applyEnv.setTopLatticeCreation();
            assume(source.fstArg().apply(localSourceOperation.setConstWithAssign(), applyEnv));
            simplifiedSource = applyEnv.presult();
         };
         TransmitEnvironment applyEnv(env);
         applyEnv.setFirstArgument(*simplifiedSource);
         assume(fstArg().apply(operation, applyEnv));
         if (applyEnv.hasResult())
            sfstArg() = applyEnv.presult();
         return true;
      };
      // thisOperation.getExtension() > sourceOperation.getExtension()
      {  ExtendOperation localThisOperation(thisOperation);
         localThisOperation.sextension() -= sourceOperation.getExtension();
         PPVirtualElement simplifiedThis;
         {  TransmitEnvironment applyEnv(env);
            if (env.getLatticeCreation().mayBeShareTop())
               applyEnv.setTopLatticeCreation();
            assume(fstArg().apply(localThisOperation.setConstWithAssign(), applyEnv));
            simplifiedThis = applyEnv.presult();
         };
         TransmitEnvironment applyEnv(env);
         applyEnv.setFirstArgument(source.fstArg());
         assume(simplifiedThis->apply(operation, applyEnv));
         if (applyEnv.hasResult())
            env.presult() = applyEnv.presult();
         else
            env.presult() = simplifiedThis;
         return true;
      };
   };
   PPVirtualElement thisSimplified = simplify(env);
   PPVirtualElement sourceSimplified = source.simplify(env);
   TransmitEnvironment applyEnv(env);
   applyEnv.setFirstArgument(*sourceSimplified);
   assume(thisSimplified->apply(operation, applyEnv));
   if (applyEnv.hasResult())
      env.presult() = applyEnv.presult();
   else
      env.presult() = thisSimplified;
   if (env.getLatticeCreation().mayBeShareTop()
         && !((const VirtualElement&) *env.presult()).getApproxKind().isVariable()) {
      PPVirtualElement domain = env.presult();
      int sizeInBits = domain->getSizeInBits();
      ExactToApproximateElement* result = new ExactToApproximateElement(
            VirtualElement::Init().setBitSize(sizeInBits), domain);
      env.presult().absorbElement(result);
   };
   return true;
}

bool
ExtendOperationElement::applyOppositeSignedAssign(const VirtualOperation& operation, EvaluationEnvironment& env) {
   ExtendOperation& thisOperation = (ExtendOperation&) *ppvoOperation;
   if (thisOperation.getType() == Operation::TExtendWithSign) {
      TransmitEnvironment applyEnv(env);
      if (!fstArg().apply(operation, applyEnv))
         return false;
      if (applyEnv.hasResult())
         sfstArg() = applyEnv.presult();
      return true;
   };
   // could use concatenations
   PPVirtualElement simplification = simplify(env);
   TransmitEnvironment applyEnv(env);
   assume(simplification->apply(operation, applyEnv));
   if (applyEnv.hasResult())
      env.presult() = applyEnv.presult();
   else
      env.presult() = simplification;
   if (env.getLatticeCreation().mayBeShareTop()
         && !((const VirtualElement&) *env.presult()).getApproxKind().isVariable()) {
      PPVirtualElement domain = env.presult();
      int sizeInBits = domain->getSizeInBits();
      ExactToApproximateElement* result = new ExactToApproximateElement(
            VirtualElement::Init().setBitSize(sizeInBits), domain);
      env.presult().absorbElement(result);
   };
   return true;
}

bool
ExtendOperationElement::applyBitBinaryAssignExtend(const VirtualOperation& operation,
      const ExtendOperationElement& source, EvaluationEnvironment& env) {
   const ExtendOperation& thisOperation = getOperation();
   const ExtendOperation& sourceOperation = source.getOperation();

   if (thisOperation.getType() == sourceOperation.getType()) {
      if (thisOperation.getExtension() == sourceOperation.getExtension()) {
         TransmitEnvironment applyEnv(env);
         applyEnv.setFirstArgument(source.fstArg());
         assume(fstArg().apply(operation, applyEnv));
         if (applyEnv.hasResult())
            sfstArg() = applyEnv.presult();
         return true;
      }
      if (thisOperation.getExtension() < sourceOperation.getExtension()) {
         ExtendOperation localSourceOperation(sourceOperation);
         localSourceOperation.sextension() -= thisOperation.getExtension();
         PPVirtualElement simplifiedSource;
         {  TransmitEnvironment applyEnv(env);
            if (env.getLatticeCreation().mayBeShareTop())
               applyEnv.setTopLatticeCreation();
            assume(source.fstArg().apply(localSourceOperation.setConstWithAssign(), applyEnv));
            simplifiedSource = applyEnv.presult();
         };
         TransmitEnvironment applyEnv(env);
         applyEnv.setFirstArgument(*simplifiedSource);
         assume(fstArg().apply(operation, applyEnv));
         if (applyEnv.hasResult())
            sfstArg() = applyEnv.presult();
         return true;
      };
      // thisOperation.getExtension() > sourceOperation.getExtension()
      {  ExtendOperation localThisOperation(thisOperation);
         localThisOperation.sextension() -= sourceOperation.getExtension();
         PPVirtualElement simplifiedThis;
         {  TransmitEnvironment applyEnv(env);
            if (env.getLatticeCreation().mayBeShareTop())
               applyEnv.setTopLatticeCreation();
            assume(fstArg().apply(localThisOperation.setConstWithAssign(), applyEnv));
            simplifiedThis = applyEnv.presult();
         };
         TransmitEnvironment applyEnv(env);
         applyEnv.setFirstArgument(source.fstArg());
         assume(simplifiedThis->apply(operation, applyEnv));
         if (applyEnv.hasResult())
            env.presult() = applyEnv.presult();
         else
            env.presult() = simplifiedThis;
         return true;
      };
   };
   PPVirtualElement thisSimplified = simplify(env);
   PPVirtualElement sourceSimplified = source.simplify(env);
   TransmitEnvironment applyEnv(env);
   applyEnv.setFirstArgument(*sourceSimplified);
   assume(thisSimplified->apply(operation, applyEnv));
   if (applyEnv.hasResult())
      env.presult() = applyEnv.presult();
   else
      env.presult() = thisSimplified;
   if (env.getLatticeCreation().mayBeShareTop()
         && !((const VirtualElement&) *env.presult()).getApproxKind().isVariable()) {
      PPVirtualElement domain = env.presult();
      int sizeInBits = domain->getSizeInBits();
      ExactToApproximateElement* result = new ExactToApproximateElement(
            VirtualElement::Init().setBitSize(sizeInBits), domain);
      env.presult().absorbElement(result);
   };
   return true;
}

bool
ExtendOperationElement::applyBitExclusiveOrAssignExtend(const VirtualOperation& operation,
      const ExtendOperationElement& source, EvaluationEnvironment& env) {
   const ExtendOperation& thisOperation = getOperation();
   const ExtendOperation& sourceOperation = source.getOperation();

   if (thisOperation.getType() == Operation::TExtendWithSign
         && sourceOperation.getType() == Operation::TExtendWithSign) {
      if (thisOperation.getExtension() == sourceOperation.getExtension()) {
         TransmitEnvironment applyEnv(env);
         applyEnv.setFirstArgument(source.fstArg());
         assume(fstArg().apply(operation, applyEnv));
         if (applyEnv.hasResult())
            sfstArg() = applyEnv.presult();
         return true;
      }
      if (thisOperation.getExtension() < sourceOperation.getExtension()) {
         ExtendOperation localSourceOperation(sourceOperation);
         localSourceOperation.sextension() -= thisOperation.getExtension();
         PPVirtualElement simplifiedSource;
         {  TransmitEnvironment applyEnv(env);
            if (env.getLatticeCreation().mayBeShareTop())
               applyEnv.setTopLatticeCreation();
            assume(source.fstArg().apply(localSourceOperation.setConstWithAssign(), applyEnv));
            simplifiedSource = applyEnv.presult();
         };
         TransmitEnvironment applyEnv(env);
         applyEnv.setFirstArgument(*simplifiedSource);
         assume(fstArg().apply(operation, applyEnv));
         if (applyEnv.hasResult())
            sfstArg() = applyEnv.presult();
         return true;
      };
      // thisOperation.getExtension() > sourceOperation.getExtension()
      {  ExtendOperation localThisOperation(thisOperation);
         localThisOperation.sextension() -= sourceOperation.getExtension();
         PPVirtualElement simplifiedThis;
         {  TransmitEnvironment applyEnv(env);
            if (env.getLatticeCreation().mayBeShareTop())
               applyEnv.setTopLatticeCreation();
            assume(fstArg().apply(localThisOperation.setConstWithAssign(), applyEnv));
            simplifiedThis = applyEnv.presult();
         };
         TransmitEnvironment applyEnv(env);
         applyEnv.setFirstArgument(source.fstArg());
         assume(simplifiedThis->apply(operation, applyEnv));
         if (applyEnv.hasResult())
            env.presult() = applyEnv.presult();
         else
            env.presult() = simplifiedThis;
         return true;
      };
   };
   PPVirtualElement thisSimplified = simplify(env);
   PPVirtualElement sourceSimplified = source.simplify(env);
   TransmitEnvironment applyEnv(env);
   applyEnv.setFirstArgument(*sourceSimplified);
   assume(thisSimplified->apply(operation, applyEnv));
   if (applyEnv.hasResult())
      env.presult() = applyEnv.presult();
   else
      env.presult() = thisSimplified;
   if (env.getLatticeCreation().mayBeShareTop()
         && !((const VirtualElement&) *env.presult()).getApproxKind().isVariable()) {
      PPVirtualElement domain = env.presult();
      int sizeInBits = domain->getSizeInBits();
      ExactToApproximateElement* result = new ExactToApproximateElement(
            VirtualElement::Init().setBitSize(sizeInBits), domain);
      env.presult().absorbElement(result);
   };
   return true;
}
   
bool
ExtendOperationElement::applyBitNegateAssign(const VirtualOperation& operation, EvaluationEnvironment& env) {
   const ExtendOperation& thisOperation = getOperation();

   if (thisOperation.getType() == Operation::TExtendWithSign) {
      TransmitEnvironment applyEnv(env);
      if (!fstArg().apply(operation, applyEnv))
         return false;
      if (applyEnv.hasResult())
         sfstArg() = applyEnv.presult();
      return true;
   };

   PPVirtualElement thisSimplified = simplify(env);
   TransmitEnvironment applyEnv(env);
   assume(thisSimplified->apply(operation, applyEnv));
   if (applyEnv.hasResult())
      env.presult() = applyEnv.presult();
   else
      env.presult() = thisSimplified;
   if (env.getLatticeCreation().mayBeShareTop()
         && !((const VirtualElement&) *env.presult()).getApproxKind().isVariable()) {
      PPVirtualElement domain = env.presult();
      int sizeInBits = domain->getSizeInBits();
      ExactToApproximateElement* result = new ExactToApproximateElement(
            VirtualElement::Init().setBitSize(sizeInBits), domain);
      env.presult().absorbElement(result);
   };
   return true;
}
   
bool
ExtendOperationElement::applyLeftShiftAssignConstant(const VirtualOperation& operation,
      const VirtualElement& source, EvaluationEnvironment& env) {
   // AssumeUnimplemented = gain back the shift in an int value <-> TDIntForShift
   return applyDefaultBinaryAssign(operation, env);
}
   
bool
ExtendOperationElement::applyRightShiftAssignConstant(const VirtualOperation& operation,
      const VirtualElement& source, EvaluationEnvironment& env) {
   // AssumeUnimplemented = gain back the shift in an int value <-> TDIntForShift
   return applyDefaultBinaryAssign(operation, env);
}
   
#define DefineInitApply(TypeOperation)                                                             \
   elementAt(Operation::T##TypeOperation).setMethod(&ExtendOperationElement::apply##TypeOperation);

#define DefineInitOApply(TypeOperation, TypeRedirection)                                           \
   elementAt(Operation::T##TypeOperation).setMethod(&ExtendOperationElement::apply##TypeRedirection);

#define DefineInitBiApplyExtend(TypeOperation, TypeDefaultMethod)                                  \
   elementAt(Operation::T##TypeOperation).setArray(new ExtendAndGenericApplyMethod(                \
      &ExtendOperationElement::apply##TypeOperation##Extend,                                       \
      &ExtendOperationElement::apply##TypeDefaultMethod));

#define DefineInitOBiApplyExtend(TypeOperation, TypeMethod, TypeDefaultMethod)                     \
   elementAt(Operation::T##TypeOperation).setArray(new ExtendAndGenericApplyMethod(                \
      &ExtendOperationElement::apply##TypeMethod##Extend,                                          \
      &ExtendOperationElement::apply##TypeDefaultMethod));

#define DefineInitOBiApplyConstant(TypeOperation, TypeMethod, TypeDefaultMethod)                   \
   elementAt(Operation::T##TypeOperation).setArray(new ConstantAndGenericApplyMethod(              \
      &ExtendOperationElement::apply##TypeMethod##Constant,                                        \
      &ExtendOperationElement::apply##TypeDefaultMethod));

#define DefineInitBiApplyConstant(TypeOperation, TypeDefaultMethod)                                \
   elementAt(Operation::T##TypeOperation).setArray(new ConstantAndGenericApplyMethod(              \
      &ExtendOperationElement::apply##TypeOperation##Constant,                                     \
      &ExtendOperationElement::apply##TypeDefaultMethod));

ExtendOperationElement::MethodApplyTable::MethodApplyTable() {
#include "StandardClasses/UndefineNew.h"
   clear();

   DefineInitOApply(ExtendWithZero, Extend)
   DefineInitOApply(ExtendWithSign, Extend)
   DefineInitOApply(Concat, DefaultBinaryAssign)
   DefineInitApply(Reduce)
   DefineInitApply(BitSet)
   DefineInitOApply(CastChar, DefaultUnary)
   DefineInitOApply(CastInt, DefaultUnary)
   DefineInitOApply(CastUnsignedInt, DefaultUnary)
   DefineInitOApply(CastMultiBit, DefaultUnary)
   // DefineInitOApply(CastBit, DefaultUnary)
   DefineInitApply(CastShiftBit)
   DefineInitOApply(CastMultiFloat, DefaultUnary)
   DefineInitOApply(CastMultiFloatPointer, DefaultUnary)
   DefineInitOApply(CastReal, DefaultUnary)
   DefineInitOApply(CastRational, DefaultUnary)
   
   DefineInitOApply(PrevSignedAssign, DefaultUnaryAssign)
   DefineInitOApply(PrevUnsignedAssign, DefaultUnaryAssign)
   DefineInitOApply(NextSignedAssign, DefaultUnaryAssign)
   DefineInitOApply(NextUnsignedAssign, DefaultUnaryAssign)
   DefineInitOApply(PlusSignedAssign, AsConcatenationAssign)
   DefineInitOApply(PlusUnsignedAssign, AsConcatenationAssign)
   DefineInitOApply(PlusUnsignedWithSignedAssign, AsConcatenationAssign)
   DefineInitOApply(PlusFloatAssign, DefaultBinaryAssign)
   DefineInitOApply(MinusSignedAssign, AsConcatenationAssign)
   DefineInitApply(MinusUnsignedAssign)
   DefineInitOApply(MinusFloatAssign, DefaultBinaryAssign)

   DefineInitOApply(TimesSignedAssign, DefaultBinaryAssign)
   DefineInitOApply(TimesUnsignedAssign, DefaultBinaryAssign)
   DefineInitOApply(DivideSignedAssign, DefaultBinaryAssign)
   DefineInitOApply(DivideUnsignedAssign, DefaultBinaryAssign)
   DefineInitOApply(ModuloSignedAssign, DefaultBinaryAssign)
   DefineInitOApply(ModuloUnsignedAssign, DefaultBinaryAssign)
   DefineInitApply(OppositeSignedAssign)

   DefineInitOBiApplyExtend(CompareLessSigned, Compare, DefaultBinary)
   DefineInitOBiApplyExtend(CompareLessOrEqualSigned, Compare, DefaultBinary)
   DefineInitOBiApplyExtend(CompareLessUnsigned, Compare, DefaultBinary)
   DefineInitOBiApplyExtend(CompareLessOrEqualUnsigned, Compare, DefaultBinary)
   DefineInitOApply(CompareLessFloat, DefaultBinary)
   DefineInitOApply(CompareLessOrEqualFloat, DefaultBinary)
   DefineInitOBiApplyExtend(CompareEqual, Compare, DefaultBinary)
   DefineInitOApply(CompareEqualFloat, DefaultBinary)
   DefineInitOBiApplyExtend(CompareDifferent, Compare, DefaultBinary)
   DefineInitOApply(CompareDifferentFloat, DefaultBinary)
   DefineInitOBiApplyExtend(CompareGreaterOrEqualUnsigned, Compare, DefaultBinary)
   DefineInitOBiApplyExtend(CompareGreaterUnsigned, Compare, DefaultBinary)
   DefineInitOBiApplyExtend(CompareGreaterOrEqualSigned, Compare, DefaultBinary)
   DefineInitOBiApplyExtend(CompareGreaterSigned, Compare, DefaultBinary)
   DefineInitOApply(CompareGreaterFloat, DefaultBinary)
   DefineInitOApply(CompareGreaterOrEqualFloat, DefaultBinary)
   DefineInitOBiApplyExtend(MinSignedAssign, MinMaxAssign, DefaultBinaryAssign)
   DefineInitOBiApplyExtend(MinUnsignedAssign, MinMaxAssign, DefaultBinaryAssign)
   DefineInitOApply(MinFloatAssign, DefaultBinaryAssign)
   DefineInitOBiApplyExtend(MaxSignedAssign, MinMaxAssign, DefaultBinaryAssign)
   DefineInitOBiApplyExtend(MaxUnsignedAssign, MinMaxAssign, DefaultBinaryAssign)
   DefineInitOApply(MaxFloatAssign, DefaultBinaryAssign)

   DefineInitOBiApplyExtend(BitOrAssign, BitBinaryAssign, DefaultBinaryAssign)
   DefineInitOBiApplyExtend(BitAndAssign, BitBinaryAssign, DefaultBinaryAssign)
   DefineInitBiApplyExtend(BitExclusiveOrAssign, DefaultBinaryAssign)
   DefineInitApply(BitNegateAssign)
   DefineInitBiApplyConstant(LeftShiftAssign, DefaultBinaryAssign)
   DefineInitOBiApplyConstant(LogicalRightShiftAssign, RightShiftAssign, DefaultBinaryAssign)
   DefineInitOBiApplyConstant(ArithmeticRightShiftAssign, RightShiftAssign, DefaultBinaryAssign)
   DefineInitOApply(LeftRotateAssign, DefaultBinaryAssign)
   DefineInitOApply(RightRotateAssign, DefaultBinaryAssign)
#include "StandardClasses/DefineNew.h"
}

#undef DefineInitApply
#undef DefineInitOApply
#undef DefineInitBiApplyExtend
#undef DefineInitOBiApplyExtend
#undef DefineInitBiApplyConstant
#undef DefineInitOBiApplyConstant

ExtendOperationElement::MethodApplyTable ExtendOperationElement::matMethodApplyTable;

bool
ExtendOperationElement::applyToDefaultBinary(const VirtualOperation& aoperation, VirtualElement& thisElement, EvaluationEnvironment& env) const {
   PPVirtualElement simplification = simplify(env);
   TransmitEnvironment applyEnv(env);
   applyEnv.setFirstArgument(*simplification);
   assume(thisElement.apply(aoperation, applyEnv));
   if (applyEnv.hasResult())
      env.presult() = applyEnv.presult();
   return true;
}
   
bool
ExtendOperationElement::applyToCommutativeAssign(const VirtualOperation& aoperation, VirtualElement& thisElement, EvaluationEnvironment& env) const {
   TransmitEnvironment applyEnv(env);
   applyEnv.setFirstArgument(thisElement);
   Operation operation((const Operation&) aoperation);
   if (!operation.isConst())
      operation.setConstWithAssign();
   if (!const_cast<ExtendOperationElement&>(*this).apply(operation, applyEnv))
      return false;
   env.presult() = applyEnv.presult();
   return true;
}

bool
ExtendOperationElement::applyToCommutative(const VirtualOperation& aoperation, VirtualElement& thisElement, EvaluationEnvironment& env) const {
   TransmitEnvironment applyEnv(env);
   applyEnv.setFirstArgument(thisElement);
   Operation operation((const Operation&) aoperation);
   if (!const_cast<ExtendOperationElement&>(*this).apply(operation, applyEnv))
      return false;
   env.presult() = applyEnv.presult();
   return true;
}

bool
ExtendOperationElement::applyToCompare(const VirtualOperation& aoperation, VirtualElement& thisElement, EvaluationEnvironment& env) const {
   TransmitEnvironment applyEnv(env);
   applyEnv.setFirstArgument(thisElement);
   Operation operation;
   const Operation& originalOperation = (const Operation&) aoperation;
   switch (originalOperation.getType()) {
      case Operation::TCompareLessSigned:
         operation.setCompareGreaterOrEqualSigned(); break;
      case Operation::TCompareLessUnsigned:
         operation.setCompareGreaterOrEqualUnsigned(); break;
      case Operation::TCompareGreaterSigned:
         operation.setCompareLessOrEqualSigned(); break;
      case Operation::TCompareGreaterUnsigned:
         operation.setCompareLessOrEqualUnsigned(); break;
      case Operation::TCompareLessOrEqualSigned:
         operation.setCompareGreaterSigned(); break;
      case Operation::TCompareLessOrEqualUnsigned:
         operation.setCompareGreaterUnsigned(); break;
      case Operation::TCompareGreaterOrEqualSigned:
         operation.setCompareLessSigned(); break;
      case Operation::TCompareGreaterOrEqualUnsigned:
         operation.setCompareLessUnsigned(); break;
      default:
         {  AssumeUncalled }
   };
   return const_cast<ExtendOperationElement&>(*this).apply(operation, applyEnv);
}

#define DefineInitOApplyTo(TypeOperation, TypeMethod)                                                        \
   elementAt(Operation::T##TypeOperation).setMethod(&ExtendOperationElement::applyTo##TypeMethod);
ExtendOperationElement::MethodApplyToTable::MethodApplyToTable() {
#include "StandardClasses/UndefineNew.h"
   clear();
   DefineInitOApplyTo(Concat, DefaultBinary)
   DefineInitOApplyTo(BitSet, DefaultBinary)
   DefineInitOApplyTo(PlusSignedAssign, CommutativeAssign)
   DefineInitOApplyTo(PlusUnsignedAssign, CommutativeAssign)
   DefineInitOApplyTo(PlusUnsignedWithSignedAssign, CommutativeAssign)
   DefineInitOApplyTo(PlusFloatAssign, CommutativeAssign)
   DefineInitOApplyTo(MinusSignedAssign, DefaultBinary)
   DefineInitOApplyTo(MinusUnsignedAssign, DefaultBinary)
   DefineInitOApplyTo(MinusFloatAssign, DefaultBinary)

   DefineInitOApplyTo(CompareLessSigned, Compare)
   DefineInitOApplyTo(CompareLessUnsigned, Compare)
   DefineInitOApplyTo(CompareLessFloat, DefaultBinary)
   DefineInitOApplyTo(CompareLessOrEqualSigned, Compare)
   DefineInitOApplyTo(CompareLessOrEqualUnsigned, Compare)
   DefineInitOApplyTo(CompareLessOrEqualFloat, DefaultBinary)
   DefineInitOApplyTo(CompareEqual, Commutative)
   DefineInitOApplyTo(CompareEqualFloat, DefaultBinary)
   DefineInitOApplyTo(CompareDifferent, Commutative)
   DefineInitOApplyTo(CompareDifferentFloat, DefaultBinary)
   DefineInitOApplyTo(CompareGreaterOrEqualSigned, Compare)
   DefineInitOApplyTo(CompareGreaterOrEqualUnsigned, Compare)
   DefineInitOApplyTo(CompareGreaterOrEqualFloat, DefaultBinary)
   DefineInitOApplyTo(CompareGreaterSigned, Compare)
   DefineInitOApplyTo(CompareGreaterUnsigned, Compare)
   DefineInitOApplyTo(CompareGreaterFloat, DefaultBinary)
   DefineInitOApplyTo(MinSignedAssign, CommutativeAssign)
   DefineInitOApplyTo(MinUnsignedAssign, CommutativeAssign)
   DefineInitOApplyTo(MinFloatAssign, CommutativeAssign)
   DefineInitOApplyTo(MaxSignedAssign, CommutativeAssign)
   DefineInitOApplyTo(MaxUnsignedAssign, CommutativeAssign)
   DefineInitOApplyTo(MaxFloatAssign, CommutativeAssign)

   DefineInitOApplyTo(TimesSignedAssign, CommutativeAssign)
   DefineInitOApplyTo(TimesUnsignedAssign, CommutativeAssign)
   DefineInitOApplyTo(TimesFloatAssign, CommutativeAssign)
   DefineInitOApplyTo(DivideSignedAssign, DefaultBinary)
   DefineInitOApplyTo(DivideUnsignedAssign, DefaultBinary)
   DefineInitOApplyTo(DivideFloatAssign, DefaultBinary)
   DefineInitOApplyTo(ModuloSignedAssign, DefaultBinary)
   DefineInitOApplyTo(ModuloUnsignedAssign, DefaultBinary)
   DefineInitOApplyTo(BitOrAssign, CommutativeAssign)
   DefineInitOApplyTo(BitAndAssign, CommutativeAssign)
   DefineInitOApplyTo(BitExclusiveOrAssign, CommutativeAssign)
   DefineInitOApplyTo(LeftShiftAssign, DefaultBinary)
   DefineInitOApplyTo(LogicalRightShiftAssign, DefaultBinary)
   DefineInitOApplyTo(ArithmeticRightShiftAssign, DefaultBinary)
   DefineInitOApplyTo(LeftRotateAssign, DefaultBinary)
   DefineInitOApplyTo(RightRotateAssign, DefaultBinary)
#include "StandardClasses/DefineNew.h"
}
#undef DefineInitOApplyTo

ExtendOperationElement::MethodApplyToTable ExtendOperationElement::mattMethodApplyToTable;

#define DefineInitUnaryConstraint(TypeOperation) \
   mcutUnaryTable[Operation::T##TypeOperation].setMethod(&VirtualElement::constraint##TypeOperation);

#define DefineInitUnaryGConstraint(TypeOperation) \
   mcutUnaryTable[Operation::T##TypeOperation].setFunction(&MultiBit::Approximate::Details::BaseAlgorithms::constraint##TypeOperation);

#define DefineInitUnaryOConstraint(TypeOperation, TypeMethod) \
   mcutUnaryTable[Operation::T##TypeOperation].setMethod(&VirtualElement::constraint##TypeMethod);

#define DefineInitUnaryOGConstraint(TypeOperation, TypeMethod) \
   mcutUnaryTable[Operation::T##TypeOperation].setFunction(&MultiBit::Approximate::Details::BaseAlgorithms::constraint##TypeMethod);

#define DefineInitUnaryOGIntConstraint(TypeOperation, TypeMethod) \
   mcutUnaryTable[Operation::T##TypeOperation].setFunction(&Scalar::Approximate::Details::IntOperationElement::constraint##TypeMethod);

#define DefineInitUnaryGIntConstraint(TypeOperation) \
   mcutUnaryTable[Operation::T##TypeOperation].setFunction(&Scalar::Approximate::Details::IntOperationElement::constraint##TypeOperation);

#define DefineInitBinaryConstraint(TypeOperation) \
   mcbtBinaryTable[Operation::T##TypeOperation].setMethod(&ExtendOperationElement::constraint##TypeOperation);

#define DefineInitBinaryGConstraint(TypeOperation) \
   mcbtBinaryTable[Operation::T##TypeOperation].setFunction(&MultiBit::Approximate::Details::BaseAlgorithms::constraint##TypeOperation);

#define DefineInitBinaryGIntConstraint(TypeOperation) \
   mcbtBinaryTable[Operation::T##TypeOperation].setFunction(&Scalar::Approximate::Details::IntOperationElement::constraint##TypeOperation);

#define DefineInitBinaryOGConstraint(TypeOperation, TypeFunction) \
   mcbtBinaryTable[Operation::T##TypeOperation].setFunction(&MultiBit::Approximate::Details::BaseAlgorithms::constraint##TypeFunction);

#define DefineInitBinaryOGIntConstraint(TypeOperation, TypeFunction) \
   mcbtBinaryTable[Operation::T##TypeOperation].setFunction(&Scalar::Approximate::Details::IntOperationElement::constraint##TypeFunction);

#define DefineInitBinaryConstraintGeneric(TypeOperation)                                          \
   mcbtBinaryTable[Operation::T##TypeOperation].setArray(                                  \
      new Approximate::Details::GenericBinaryConstraintMethod<DomainTraits>(&ExtendOperationElement::constraint##TypeOperation));

#define DefineInitOBinaryConstraintGeneric(TypeOperation, TypeMethod)                                          \
   mcbtBinaryTable[Operation::T##TypeOperation].setArray(                                  \
      new Approximate::Details::GenericBinaryConstraintMethod<DomainTraits>(&ExtendOperationElement::constraint##TypeMethod));

ExtendOperationElement::MethodConstraintTable::MethodConstraintTable() {
#include "StandardClasses/UndefineNew.h"
   DefineInitUnaryGConstraint(ExtendWithZero)
   DefineInitUnaryGConstraint(ExtendWithSign)
   DefineInitBinaryGConstraint(Concat)
   DefineInitUnaryGConstraint(Reduce)
   DefineInitBinaryGConstraint(BitSet)

   DefineInitUnaryOGIntConstraint(CastChar, CastInteger)
   DefineInitUnaryOGIntConstraint(CastInt, CastInteger)
   DefineInitUnaryOGIntConstraint(CastUnsignedInt, CastInteger)
   // DefineInitUnaryOGConstraint(CastFloat, CastDouble)
   // DefineInitUnaryOGConstraint(CastDouble, CastDouble)
   // DefineInitUnaryOGConstraint(CastLongDouble, CastDouble)

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

   DefineInitUnaryOGIntConstraint(PrevSignedAssign, PrevAssign)
   DefineInitUnaryOGIntConstraint(PrevUnsignedAssign, PrevAssign)
   DefineInitUnaryOGIntConstraint(NextSignedAssign, NextAssign)
   DefineInitUnaryOGIntConstraint(NextUnsignedAssign, NextAssign)

   DefineInitBinaryGConstraint(PlusSignedAssign)
   DefineInitBinaryGConstraint(PlusUnsignedAssign)
   DefineInitBinaryGConstraint(PlusUnsignedWithSignedAssign)
   DefineInitBinaryOGIntConstraint(MinusSignedAssign, MinusAssign)
   DefineInitBinaryOGIntConstraint(MinusUnsignedAssign, MinusAssign)
   DefineInitBinaryGConstraint(TimesSignedAssign)
   DefineInitBinaryGConstraint(TimesUnsignedAssign)
   DefineInitBinaryGConstraint(DivideSignedAssign)
   DefineInitBinaryGConstraint(DivideUnsignedAssign)

   DefineInitUnaryOGIntConstraint(OppositeSignedAssign, OppositeAssign)

   DefineInitBinaryGConstraint(ModuloSignedAssign)
   DefineInitBinaryGConstraint(ModuloUnsignedAssign)
   DefineInitBinaryGIntConstraint(BitOrAssign)
   DefineInitBinaryGIntConstraint(BitAndAssign)
   DefineInitBinaryGIntConstraint(BitExclusiveOrAssign)
   DefineInitUnaryGIntConstraint(BitNegateAssign)
   // DefineInitBinaryGConstraint(LeftShiftAssign)
   // DefineInitBinaryGConstraint(LogicalRightShiftAssign)
   // DefineInitBinaryGConstraint(ArithmeticRightShiftAssign)

   // DefineInitBinaryConstraintGeneric(LogicalAndAssign)
   // DefineInitBinaryConstraintGeneric(LogicalOrAssign)
   // DefineInitUnaryGConstraint(LogicalNegateAssign)
#include "StandardClasses/DefineNew.h"
}

#undef DefineInitUnaryConstraint
#undef DefineInitUnaryGConstraint
#undef DefineInitUnaryOGIntConstraint
#undef DefineInitUnaryGIntConstraint
#undef DefineInitUnaryOConstraint
#undef DefineInitUnaryOGConstraint
#undef DefineInitBinaryConstraint
#undef DefineInitBinaryGConstraint
#undef DefineInitBinaryOGConstraint
#undef DefineInitBinaryOGIntConstraint
#undef DefineInitBinaryGIntConstraint
#undef DefineInitBinaryConstraintGeneric
#undef DefineInitOBinaryConstraintGeneric

ExtendOperationElement::MethodConstraintTable ExtendOperationElement::mctMethodConstraintTable;

/***************************************************************/
/* Implementation of the methods ExtendOperationElement::query */
/***************************************************************/

bool
ExtendOperationElement::queryCompareSpecial(const VirtualElement& athisElement, const VirtualQueryOperation& queryOperation, VirtualQueryOperation::Environment& aenv) {
   AssumeCondition(dynamic_cast<const ExtendOperationElement*>(&athisElement))
   ExtendOperationElement& thisElement = const_cast<ExtendOperationElement&>((ExtendOperationElement&) athisElement);
   CompareSpecialEnvironment localEnv;
   thisElement.fstArg().query(queryOperation, localEnv);
   AssumeCondition(dynamic_cast<const CompareSpecialEnvironment*>(&aenv))
   CompareSpecialEnvironment& env = (CompareSpecialEnvironment&) aenv;
   if (thisElement.getOperation().getType() == Operation::TExtendWithZero) {
      CompareSpecialEnvironment positiveEnv;
      positiveEnv.mergeBehaviour((CompareSpecialEnvironment::Behaviour)
         (  CompareSpecialEnvironment::BEqual0
          | CompareSpecialEnvironment::BEqual1
          | CompareSpecialEnvironment::BGreater1));
      positiveEnv.intersectWith(localEnv);
      if (localEnv.mayBeLessZero()) {
         if (thisElement.fstArg().getSizeInBits() == 1)
            positiveEnv.mergeBehaviour(CompareSpecialEnvironment::BEqual1);
         positiveEnv.mergeBehaviour(CompareSpecialEnvironment::BGreater1);
         if (localEnv.mayBeMinusOne() && thisElement.getOperation().getExtension() == 1)
            positiveEnv.mergeBehaviour(CompareSpecialEnvironment::BEqualMaxint);
      };
      if (localEnv.mayBeMaxint())
         positiveEnv.mergeBehaviour(CompareSpecialEnvironment::BGreater1);
      env.mergeWith(positiveEnv);
   }
   else {
      AssumeCondition(thisElement.getOperation().getType() == Operation::TExtendWithSign)
      CompareSpecialEnvironment possibleEnv;
      possibleEnv.mergeBehaviour((CompareSpecialEnvironment::Behaviour)
         (  CompareSpecialEnvironment::BLessM1
          | CompareSpecialEnvironment::BEqualM1
          | CompareSpecialEnvironment::BEqual0
          | CompareSpecialEnvironment::BEqual1
          | CompareSpecialEnvironment::BGreater1));
      possibleEnv.intersectWith(localEnv);
      if (localEnv.mayBeMinint() || localEnv.mayBeMinintPlusOne())
         possibleEnv.mergeBehaviour(CompareSpecialEnvironment::BLessM1);
      if (localEnv.mayBeMaxint())
         possibleEnv.mergeBehaviour(CompareSpecialEnvironment::BGreater1);
      env.mergeWith(possibleEnv);
   };

   return true;
}

bool
ExtendOperationElement::querySimplification(const VirtualElement& athisElement, const VirtualQueryOperation&, VirtualQueryOperation::Environment& aenv) {
   AssumeCondition(dynamic_cast<const ExtendOperationElement*>(&athisElement)
         && dynamic_cast<const SimplificationEnvironment*>(&aenv))
   ExtendOperationElement& thisElement = const_cast<ExtendOperationElement&>((ExtendOperationElement&) athisElement);
   bool doesContinue = true;
   while (doesContinue && thisElement.fstArg().getApproxKind().isFormalOperation()) {
      doesContinue = false;
      Operation::Type typeFormalOperation = ((const Operation&)
         ((const TFormalOperationElement<Scalar::Approximate::Details::IntOperationElement>&) thisElement.fstArg()).getOperation()).getType();
      if (typeFormalOperation == thisElement.getOperation().getType()) {
         AssumeCondition(dynamic_cast<const ExtendOperationElement*>(&thisElement.fstArg()))
         PPVirtualElement newArg = ((ExtendOperationElement&) thisElement.fstArg()).sfstArg();
         ((ExtendOperation&) *thisElement.ppvoOperation).sextension()
            += ((ExtendOperationElement&) thisElement.fstArg()).getOperation().getExtension();
         thisElement.sfstArg() = newArg;
         doesContinue = true;
      };
   };
   return true;
}

bool
ExtendOperationElement::querySimplificationAsConstantDisjunction(const VirtualElement& thisElement,
      const VirtualQueryOperation& operation, VirtualQueryOperation::Environment& aenv) {
   AssumeCondition(dynamic_cast<const ExtendOperationElement*>(&thisElement))
   const ExtendOperationElement& element = (const ExtendOperationElement&) thisElement;
   element.fstArg().query(operation, aenv);
   SimplificationEnvironment& env = ((SimplificationEnvironment&) aenv);
   if (!env.hasFailed()) {
      if (env.hasResult())
         env.presult() = Methods::applyAssign(env.presult(), element.getOperation(), EvaluationParameters());
      else
         env.presult() = Methods::applyAssign(const_cast<ExtendOperationElement&>(element).sfstArg(), 
               element.getOperation(), EvaluationParameters());
   };
   return true;
}

bool
ExtendOperationElement::queryDispatchOnTopResult(const VirtualElement& thisElement, const VirtualQueryOperation& qoOperation, VirtualQueryOperation::Environment& aenv) {
   AssumeCondition(dynamic_cast<const ExtendOperationElement*>(&thisElement))
   return ((const ExtendOperationElement&) thisElement).queryTopResult()->query(qoOperation, aenv);
}

#define DefineInitQuery(TypeOperation) \
   elementAt(QueryOperation::TypeOperation).setMethod(&ExtendOperationElement::query##TypeOperation);

ExtendOperationElement::QueryTable::QueryTable() {
#include "StandardClasses/UndefineNew.h"
   FunctionQueryTable* table = &elementAt(VirtualQueryOperation::TCompareSpecial);
   table->setSize(QueryOperation::EndOfTypeCompareSpecial);
   (*table)[QueryOperation::TCSGuard].setMethod(&ExtendOperationElement::queryGuardAll);
   (*table)[QueryOperation::TCSNatural].setMethod(&ExtendOperationElement::queryCompareSpecial);
   (*table)[QueryOperation::TCSSigned].setMethod(&ExtendOperationElement::queryCompareSpecial);
   (*table)[QueryOperation::TCSUnsigned].setMethod(&ExtendOperationElement::queryCompareSpecial);
   (*table)[QueryOperation::TCSBitDomain].setMethod(&ExtendOperationElement::queryDispatchOnTopResult);
   
   table = &elementAt(VirtualQueryOperation::TSimplification);
   table->setSize(QueryOperation::EndOfTypeSimplification);
   (*table)[QueryOperation::TSConstant].setMethod(&Top::queryFailSimplification);
   (*table)[QueryOperation::TSInterval].setMethod(&ExtendOperationElement::querySimplificationAsConstantDisjunction);
   (*table)[QueryOperation::TSConstantDisjunction].setMethod(&ExtendOperationElement::querySimplificationAsConstantDisjunction);

   table = &elementAt(VirtualQueryOperation::TDomain);
   table->setSize(QueryOperation::EndOfTypeDomain);

#define DefineInitQueryMethodTable(TypeOperation)                              \
   (*table)[QueryOperation::TD##TypeOperation].setMethod(&ExtendOperationElement::queryDispatchOnTopResult);
   
   DefineInitQueryMethodTable(Disjunction)
   DefineInitQueryMethodTable(Interval)
   DefineInitQueryMethodTable(BooleanDisjunction)
   DefineInitQueryMethodTable(Top)
   DefineInitQueryMethodTable(False)
   DefineInitQueryMethodTable(True)
   DefineInitQueryMethodTable(IntForShift)
   DefineInitQueryMethodTable(Zero)
   DefineInitQueryMethodTable(MinusOne)
   DefineInitQueryMethodTable(One)
   DefineInitQueryMethodTable(Min)
   DefineInitQueryMethodTable(Max)

#undef DefineInitQueryMethodTable

   table = &elementAt(VirtualQueryOperation::TExtern);
   table->setSize(QueryOperation::EndOfTypeExtern);
   (*table)[QueryOperation::TESigned].setMethod(&ExtendOperationElement::queryDispatchOnTopResult);
#include "StandardClasses/DefineNew.h"
}

#undef DefineInitQuery

ExtendOperationElement::QueryTable ExtendOperationElement::mqtMethodQueryTable;

#define DefineTypeOperation MultiBit
#define DefineTypeObject ExtendOperationElement
#include "Analyzer/Scalar/Approximate/VirtualCall.incc"
#undef DefineTypeObject
#undef DefineTypeOperation

bool
ExtendOperationElement::mergeWith(const Scalar::VirtualElement& asource, Scalar::EvaluationEnvironment& aenv) {
   if (VirtualElement::mergeWith(asource, aenv))
      return true;
   EvaluationEnvironment& env = (EvaluationEnvironment&) aenv;
   const VirtualElement& source = (const VirtualElement&) asource;
   if (source.getApproxKind().isFormalOperation()) {
      const Operation& sourceOperation = (const Operation&)
         ((const TFormalOperationElement<Scalar::Approximate::Details::IntOperationElement>&) source).getOperation();
      if (sourceOperation.getType() == getOperation().getType()) {
         AssumeCondition(dynamic_cast<const ExtendOperationElement*>(&asource))
         const ExtendOperationElement& source = (const ExtendOperationElement&) asource;
         if (source.getOperation().getExtension() == getOperation().getExtension()) {
            fstArg().mergeWith(source.fstArg(), aenv);
            if (aenv.hasResult())
               sfstArg() = aenv.presult();
            return true;
         };
      };
   };
   PPVirtualElement thisSimplified = simplify(env);
   TransmitEnvironment applyEnv(env);
   assume(thisSimplified->mergeWith(source, applyEnv));
   if (applyEnv.hasResult())
      aenv.presult() = applyEnv.presult();
   else
      aenv.presult() = thisSimplified;
   if (env.getLatticeCreation().mayBeShareTop()
         && !((const VirtualElement&) *aenv.presult()).getApproxKind().isVariable()) {
      PPVirtualElement domain = aenv.presult();
      int sizeInBits = domain->getSizeInBits();
      ExactToApproximateElement* result = new ExactToApproximateElement(
            VirtualElement::Init().setBitSize(sizeInBits), domain);
      aenv.presult().absorbElement(result);
   };
   return true;
}

bool
ExtendOperationElement::contain(const Scalar::VirtualElement& asource, Scalar::EvaluationEnvironment& aenv) const {
   EvaluationEnvironment& env = (EvaluationEnvironment&) aenv;
   const VirtualElement& source = (const VirtualElement&) asource;

   if (source.getApproxKind().isFormalOperation()) {
      const Operation& sourceOperation = (const Operation&)
         ((const TFormalOperationElement<Scalar::Approximate::Details::IntOperationElement>&) source).getOperation();
      if (sourceOperation.getType() == getOperation().getType()) {
         AssumeCondition(dynamic_cast<const ExtendOperationElement*>(&asource))
         const ExtendOperationElement& source = (const ExtendOperationElement&) asource;
         if (source.getOperation().getExtension() == getOperation().getExtension())
            return fstArg().contain(source.fstArg(), aenv);
      };
   };
   PPVirtualElement thisSimplified = simplify(env);
   TransmitEnvironment applyEnv(env);
   assume(thisSimplified->contain(source, applyEnv));
   return true;
}

bool
ExtendOperationElement::mergeWithTo(VirtualElement& source, EvaluationEnvironment& env) const {
   PPVirtualElement thisSimplified = simplify(env);
   TransmitEnvironment applyEnv(env);
   assume(source.mergeWith(*thisSimplified, applyEnv));
   if (applyEnv.hasResult())
      env.presult() = applyEnv.presult();
   return true;
}

bool
ExtendOperationElement::containTo(const VirtualElement& source, EvaluationEnvironment& env) const {
   PPVirtualElement thisSimplified = simplify(env);
   TransmitEnvironment applyEnv(env);
   assume(source.contain(*thisSimplified, applyEnv));
   return true;
}

bool
ExtendOperationElement::intersectWith(const VirtualElement& asource, EvaluationEnvironment& env) {
   if (VirtualElement::intersectWith(asource, env))
      return true;
   if (asource.getApproxKind().isFormalOperation()) {
      const Operation& sourceOperation = (const Operation&)
         ((const TFormalOperationElement<Scalar::Approximate::Details::IntOperationElement>&) asource).getOperation();
      if (sourceOperation.getType() == getOperation().getType()) {
         AssumeCondition(dynamic_cast<const ExtendOperationElement*>(&asource))
         const ExtendOperationElement& source = (const ExtendOperationElement&) asource;
         if (source.getOperation().getExtension() == getOperation().getExtension()) {
            fstArg().intersectWith(source.fstArg(), env);
            if (env.hasResult())
               sfstArg() = env.presult();
            return true;
         };
      };
   };
   PPVirtualElement thisSimplified = simplify(env);
   TransmitEnvironment applyEnv(env);
   assume(thisSimplified->intersectWith(asource, applyEnv));
   if (applyEnv.hasResult())
      env.presult() = applyEnv.presult();
   else
      env.presult() = thisSimplified;
   if (env.getLatticeCreation().mayBeShareTop()
         && !((const VirtualElement&) *env.presult()).getApproxKind().isVariable()) {
      PPVirtualElement domain = env.presult();
      int sizeInBits = domain->getSizeInBits();
      ExactToApproximateElement* result = new ExactToApproximateElement(
            VirtualElement::Init().setBitSize(sizeInBits), domain);
      env.presult().absorbElement(result);
   };
   return true;
}

bool
ExtendOperationElement::intersectWithTo(VirtualElement& source, EvaluationEnvironment& env) const {
   PPVirtualElement thisSimplified = simplify(env);
   TransmitEnvironment applyEnv(env);
   assume(source.intersectWith(*thisSimplified, applyEnv));
   if (applyEnv.hasResult())
      env.presult() = applyEnv.presult();
   return true;
}

/*****************************************************/
/* Implementation of the class MinusOperationElement */
/*****************************************************/

bool
MinusOperationElement::applyDefaultAssign(const VirtualOperation& operation, EvaluationEnvironment& env) {
   TransmitEnvironment applyEnv(env);
   applyEnv.setTopLatticeCreation();
   applyEnv.setFirstArgument(sndArg());
   fstArg().apply(getOperation(), applyEnv);
   PPVirtualElement arg;
   if (applyEnv.hasResult())
      arg = applyEnv.presult();
   else
      arg = sfstArg();
   applyEnv.merge();
   {  TransmitEnvironment applyEnv(env);
      if (env.hasFirstArgument())
         applyEnv.setFirstArgument(env.getFirstArgument());
      arg->apply(operation, applyEnv);
      if (applyEnv.hasResult())
         env.presult() = applyEnv.presult();
      else
         env.presult() = arg;
   }
   return true;
}
   
bool
MinusOperationElement::applyDefault(const VirtualOperation& operation, EvaluationEnvironment& env) {
   TransmitEnvironment applyEnv(env);
   applyEnv.setTopLatticeCreation();
   applyEnv.setFirstArgument(sndArg());
   Operation constOperation((const Operation&) getOperation());
   if (!constOperation.isConst())
      constOperation.setConstWithAssign();
   fstArg().apply(constOperation, applyEnv);
   PPVirtualElement arg = applyEnv.presult();
   applyEnv.merge();
   {  TransmitEnvironment applyEnv(env);
      if (env.hasFirstArgument())
         applyEnv.setFirstArgument(env.getFirstArgument());
      arg->apply(operation, applyEnv);
      env.presult() = applyEnv.presult();
   }
   return true;
}

bool
MinusOperationElement::applyExtend(const VirtualOperation& aoperation, EvaluationEnvironment& env) {
   if (env.getLatticeCreation().isTop())
      return applyDefaultAssign(aoperation, env);
   const Operation& operation = (const Operation&) aoperation;
   ExtendOperationElement* result;
   {  ExtendOperationElement::Init init;
      AssumeCondition(dynamic_cast<const ExtendOperation*>(&operation))
      int extension = ((const ExtendOperation&) operation).getExtension();
      init.setBitSize(getSizeInBits() + extension);
      if (operation.getType() == Operation::TExtendWithZero)
         init.setUnsignedExtension(extension);
      else {
         AssumeCondition(operation.getType() == Operation::TExtendWithSign)
            init.setSignedExtension(extension);
      }
      result = new ExtendOperationElement(init);
   };
   env.presult().absorbElement(result);
   result->sfstArg().absorbElement(createSCopy());
   return true;
}

bool
MinusOperationElement::applyConcat(const VirtualOperation& operation, EvaluationEnvironment& env) {
   ConcatOperationElement* result;
   {  ConcatOperationElement::Init init;
      init.setBitSize(getSizeInBits() + env.getFirstArgument().getSizeInBits());
      result = new ConcatOperationElement(init);
   };
   env.presult().absorbElement(result);
   result->sfstArg().setElement(*this);
   result->ssndArg().setElement(env.getFirstArgument());
   return true;
}

bool
MinusOperationElement::applyReduce(const VirtualOperation& aoperation, EvaluationEnvironment& env) {
   const ReduceOperation& operation = (const ReduceOperation&) aoperation;
   if ((operation.getLowBit() == operation.getHighBit())
         && (operation.getLowBit() == fstArg().getSizeInBits() - 1) && operation.getLowBit() > 2) {
      CompareOperationElement* result;
      {  CompareOperationElement::Init init;
         init.setBitSize(1);
         if (((const Operation&) getOperation()).getType() == Operation::TMinusSignedAssign)
            result = new CompareOperationElement(init, Operation::TCompareLessSigned);
         else
            result = new CompareOperationElement(init, Operation::TCompareLessUnsigned);
         env.presult().absorbElement(result);
         result->sfstArg() = sfstArg();
         result->ssndArg() = ssndArg();
         return true;
      };
   };
   if (operation.getLowBit() == 0 && operation.getHighBit() == fstArg().getSizeInBits() - 1)
      return true;
   if (operation.getLowBit() == 0) {
      {  TransmitEnvironment transmitEnv(env);
         sfstArg()->apply(operation, transmitEnv);
         if (transmitEnv.hasResult())
            sfstArg() = transmitEnv.presult();
      }
      {  TransmitEnvironment transmitEnv(env);
         ssndArg()->apply(operation, transmitEnv);
         if (transmitEnv.hasResult())
            ssndArg() = transmitEnv.presult();
      }
   }
   return operation.isConst() ? applyDefault(operation, env)
      : applyDefaultAssign(operation, env);
}

bool
MinusOperationElement::applyNextAssign(const VirtualOperation& operation, EvaluationEnvironment& env) {
   Operation::Type thisOperationType = ((const Operation&) getOperation()).getType();
   Operation::Type operationType = ((const Operation&) operation).getType();
   if ((thisOperationType == Operation::TMinusUnsignedAssign
         && operationType == Operation::TNextUnsignedAssign)
      || (thisOperationType == Operation::TMinusSignedAssign
            && operationType == Operation::TNextSignedAssign)) {
      {  TransmitEnvironment applyEnv(env);
         Operation constOperation((const Operation&) operation);
         if (!constOperation.isConst())
            constOperation.setConstWithAssign();
         if (!fstArg().apply(constOperation, applyEnv))
            return false;
         if (!applyEnv.isApproximate()) {
            sfstArg() = applyEnv.presult();
            return true;
         };
         applyEnv.clearMask();
      };
      {  TransmitEnvironment applyEnv(env);
         Operation prevOperation;
         if (thisOperationType == Operation::TMinusUnsignedAssign)
            prevOperation.setPrevUnsignedAssign();
         else
            prevOperation.setPrevSignedAssign();
         if (!sndArg().apply(prevOperation.setConstWithAssign(), applyEnv))
            return false;
         if (!applyEnv.isApproximate()) {
            ssndArg() = applyEnv.presult();
            return true;
         };
         applyEnv.clearMask();
      };
   };
   TransmitEnvironment applyEnv(env);
   applyEnv.setTopLatticeCreation();
   applyEnv.setFirstArgument(sndArg());
   fstArg().apply(getOperation(), applyEnv);
   PPVirtualElement arg;
   if (applyEnv.hasResult())
      arg = applyEnv.presult();
   else
      arg = sfstArg();
   applyEnv.merge();
   {  TransmitEnvironment applyEnv(env);
      arg->apply(operation, applyEnv);
      if (applyEnv.hasResult())
         env.presult() = applyEnv.presult();
      else
         env.presult() = arg;
   }
   return true;
}

bool
MinusOperationElement::applyPrevAssign(const VirtualOperation& operation, EvaluationEnvironment& env) {
   Operation::Type thisOperationType = ((const Operation&) getOperation()).getType();
   Operation::Type operationType = ((const Operation&) operation).getType();
   if ((thisOperationType == Operation::TMinusUnsignedAssign
         && operationType == Operation::TPrevUnsignedAssign)
      || (thisOperationType == Operation::TMinusSignedAssign
            && operationType == Operation::TPrevSignedAssign)) {
      {  TransmitEnvironment applyEnv(env);
         Operation constOperation((const Operation&) operation);
         if (!constOperation.isConst())
            constOperation.setConstWithAssign();
         if (!fstArg().apply(constOperation, applyEnv))
            return false;
         if (!applyEnv.isApproximate()) {
            sfstArg() = applyEnv.presult();
            return true;
         };
         applyEnv.clearMask();
      };
      {  TransmitEnvironment applyEnv(env);
         Operation nextOperation;
         if (operationType == Operation::TPrevUnsignedAssign)
            nextOperation.setNextUnsignedAssign();
         else
            nextOperation.setNextSignedAssign();
         if (!sndArg().apply(nextOperation.setConstWithAssign(), applyEnv))
            return false;
         if (!applyEnv.isApproximate()) {
            ssndArg() = applyEnv.presult();
            return true;
         };
         applyEnv.clearMask();
      };
   };
   TransmitEnvironment applyEnv(env);
   applyEnv.setTopLatticeCreation();
   applyEnv.setFirstArgument(sndArg());
   fstArg().apply(getOperation(), applyEnv);
   PPVirtualElement arg;
   if (applyEnv.hasResult())
      arg = applyEnv.presult();
   else
      arg = sfstArg();
   applyEnv.merge();
   {  TransmitEnvironment applyEnv(env);
      arg->apply(operation, applyEnv);
      if (applyEnv.hasResult())
         env.presult() = applyEnv.presult();
      else
         env.presult() = arg;
   }
   return true;
}

bool
MinusOperationElement::applyPlusAssign(const VirtualOperation& operation, EvaluationEnvironment& env) {
   Operation::Type thisOperationType = ((const Operation&) getOperation()).getType();
   Operation::Type operationType = ((const Operation&) operation).getType();
   if ((thisOperationType == Operation::TMinusUnsignedAssign
         && operationType == Operation::TPlusUnsignedAssign)
      || (thisOperationType == Operation::TMinusSignedAssign
            && operationType == Operation::TPlusSignedAssign)) {
      {  TransmitEnvironment applyEnv(env);
         applyEnv.setFirstArgument(env.getFirstArgument());
         Operation constOperation((const Operation&) operation);
         if (!constOperation.isConst())
            constOperation.setConstWithAssign();
         if (!fstArg().apply(constOperation, applyEnv))
            return false;
         if (!applyEnv.isApproximate()) {
            sfstArg() = applyEnv.presult();
            return true;
         };
         applyEnv.clearMask();
      };
      {  TransmitEnvironment applyEnv(env);
         applyEnv.setFirstArgument(env.getFirstArgument());
         Operation constOperation((const Operation&) getOperation());
         if (!constOperation.isConst())
            constOperation.setConstWithAssign();
         if (!sndArg().apply(constOperation, applyEnv))
            return false;
         if (!applyEnv.isApproximate()) {
            ssndArg() = applyEnv.presult();
            return true;
         };
         applyEnv.clearMask();
      };
   };
   TransmitEnvironment applyEnv(env);
   applyEnv.setTopLatticeCreation();
   applyEnv.setFirstArgument(sndArg());
   fstArg().apply(getOperation(), applyEnv);
   PPVirtualElement arg;
   if (applyEnv.hasResult())
      arg = applyEnv.presult();
   else
      arg = sfstArg();
   applyEnv.merge();
   {  TransmitEnvironment applyEnv(env);
      applyEnv.setFirstArgument(env.getFirstArgument());
      arg->apply(operation, applyEnv);
      if (applyEnv.hasResult())
         env.presult() = applyEnv.presult();
      else
         env.presult() = arg;
   }
   return true;
}

bool
MinusOperationElement::applyMinusAssign(const VirtualOperation& operation, EvaluationEnvironment& env) {
   Operation::Type thisOperationType = ((const Operation&) getOperation()).getType();
   Operation::Type operationType = ((const Operation&) operation).getType();
   if (thisOperationType == operationType) {
      {  TransmitEnvironment applyEnv(env);
         applyEnv.setFirstArgument(env.getFirstArgument());
         Operation constOperation((const Operation&) operation);
         if (!constOperation.isConst())
            constOperation.setConstWithAssign();
         if (!fstArg().apply(constOperation, applyEnv))
            return false;
         if (!applyEnv.isApproximate()) {
            sfstArg() = applyEnv.presult();
            return true;
         };
         applyEnv.clearMask();
      };
      {  TransmitEnvironment applyEnv(env);
         applyEnv.setFirstArgument(env.getFirstArgument());
         Operation subOperation;
         Operation plusOperation;
         if (thisOperationType == Operation::TMinusSignedAssign)
            plusOperation.setPlusSignedAssign();
         else
            plusOperation.setPlusUnsignedAssign();
         if (!fstArg().apply(plusOperation.setConstWithAssign(), applyEnv))
            return false;
         if (!applyEnv.isApproximate()) {
            sfstArg() = applyEnv.presult();
            return true;
         };
         applyEnv.clearMask();
      };
   };
   TransmitEnvironment applyEnv(env);
   applyEnv.setFirstArgument(sndArg());
   applyEnv.setTopLatticeCreation();
   fstArg().apply(getOperation(), applyEnv);
   PPVirtualElement arg;
   if (applyEnv.hasResult())
      arg = applyEnv.presult();
   else
      arg = sfstArg();
   applyEnv.merge();
   {  TransmitEnvironment applyEnv(env);
      applyEnv.setFirstArgument(env.getFirstArgument());
      arg->apply(operation, applyEnv);
      if (applyEnv.hasResult())
         env.presult() = applyEnv.presult();
      else
         env.presult() = arg;
   }
   return true;
}
   
bool
MinusOperationElement::applyCompare(const VirtualOperation& operation, EvaluationEnvironment& env) {
   typedef Operation Operation;
   Operation::Type thisOperationType = ((const Operation&) getOperation()).getType();
   Operation::Type operationType = ((const Operation&) operation).getType();
   bool hasSimplification = env.getFirstArgument().queryZeroResult().isZero();
   if (hasSimplification) {
      if (thisOperationType == Operation::TMinusSignedAssign)
         hasSimplification = (operationType == Operation::TCompareLessSigned)
            || (operationType == Operation::TCompareEqual)
            || (operationType == Operation::TCompareDifferent)
            || (operationType == Operation::TCompareGreaterSigned)
            || (operationType == Operation::TCompareLessOrEqualSigned)
            || (operationType == Operation::TCompareGreaterOrEqualSigned);
      else
         hasSimplification = (operationType == Operation::TCompareLessUnsigned)
            || (operationType == Operation::TCompareEqual)
            || (operationType == Operation::TCompareDifferent)
            || (operationType == Operation::TCompareGreaterUnsigned)
            || (operationType == Operation::TCompareLessOrEqualUnsigned)
            || (operationType == Operation::TCompareGreaterOrEqualUnsigned);
   };
   if (hasSimplification) {
      typedef Bit::Approximate::CompareOperationElement CompareOperationElement;
      auto* comparisonResult = new CompareOperationElement((const CompareOperationElement::Init&)
            Init().set1(), ((const Operation&) operation).getType());
      env.presult().absorbElement(comparisonResult);
      comparisonResult->sfstArg().setElement(fstArg());
      comparisonResult->ssndArg().setElement(sndArg());
      return true;
   };

   TransmitEnvironment applyEnv(env);
   applyEnv.setTopLatticeCreation();
   applyEnv.setFirstArgument(sndArg());
   fstArg().apply(getOperation(), applyEnv);
   PPVirtualElement arg;
   if (applyEnv.hasResult())
      arg = applyEnv.presult();
   else
      arg.setElement(fstArg());
   applyEnv.merge();
   {  TransmitEnvironment applyEnv(env);
      applyEnv.setFirstArgument(env.getFirstArgument());
      arg->apply(operation, applyEnv);
      if (applyEnv.hasResult())
         env.presult() = applyEnv.presult();
      else
         env.presult() = arg;
   }
   return true;
}
   

bool
MinusOperationElement::applyOppositeSignedAssign(const VirtualOperation& operation, EvaluationEnvironment& env) {
   PPVirtualElement temp = sfstArg();
   sfstArg() = ssndArg();
   ssndArg() = temp;
   return true;
}
   
#define DefineInitApply(TypeOperation)                                                             \
   elementAt(Operation::T##TypeOperation).setMethod(&MinusOperationElement::apply##TypeOperation);

#define DefineInitOApply(TypeOperation, TypeRedirection)                                           \
   elementAt(Operation::T##TypeOperation).setMethod(&MinusOperationElement::apply##TypeRedirection);

MinusOperationElement::MethodApplyTable::MethodApplyTable() {
#include "StandardClasses/UndefineNew.h"
   clear();

   DefineInitOApply(ExtendWithZero, Extend)
   DefineInitOApply(ExtendWithSign, Extend)
   DefineInitApply(Concat)
   DefineInitApply(Reduce)
   DefineInitOApply(BitSet, DefaultAssign)
   DefineInitOApply(CastChar, Default)
   DefineInitOApply(CastInt, Default)
   DefineInitOApply(CastUnsignedInt, Default)
   DefineInitOApply(CastMultiBit, Default)
   // DefineInitOApply(CastBit, Default)
   DefineInitOApply(CastShiftBit, Default)
   DefineInitOApply(CastMultiFloat, Default)
   DefineInitOApply(CastMultiFloatPointer, Default)
   DefineInitOApply(CastReal, Default)
   DefineInitOApply(CastRational, Default)
   
   DefineInitOApply(PrevSignedAssign, PrevAssign)
   DefineInitOApply(PrevUnsignedAssign, PrevAssign)
   DefineInitOApply(NextSignedAssign, NextAssign)
   DefineInitOApply(NextUnsignedAssign, NextAssign)
   DefineInitOApply(PlusSignedAssign, PlusAssign)
   DefineInitOApply(PlusUnsignedAssign, PlusAssign)
   DefineInitOApply(PlusUnsignedWithSignedAssign, PlusAssign)
   DefineInitOApply(PlusFloatAssign, DefaultAssign)
   DefineInitOApply(MinusSignedAssign, MinusAssign)
   DefineInitOApply(MinusUnsignedAssign, MinusAssign)
   DefineInitOApply(MinusFloatAssign, DefaultAssign)

   DefineInitOApply(TimesSignedAssign, DefaultAssign)
   DefineInitOApply(TimesUnsignedAssign, DefaultAssign)
   DefineInitOApply(DivideSignedAssign, DefaultAssign)
   DefineInitOApply(DivideUnsignedAssign, DefaultAssign)
   DefineInitOApply(ModuloSignedAssign, DefaultAssign)
   DefineInitOApply(ModuloUnsignedAssign, DefaultAssign)
   DefineInitApply(OppositeSignedAssign)

   DefineInitOApply(CompareLessSigned, Compare)
   DefineInitOApply(CompareLessOrEqualSigned, Compare)
   DefineInitOApply(CompareLessUnsigned, Compare)
   DefineInitOApply(CompareLessOrEqualUnsigned, Compare)
   // DefineInitApply(CompareLessFloat)
   // DefineInitApply(CompareLessOrEqualFloat)
   DefineInitOApply(CompareEqual, Compare)
   // DefineInitApply(CompareEqualFloat)
   DefineInitOApply(CompareDifferent, Compare)
   // DefineInitApply(CompareDifferentFloat)
   DefineInitOApply(CompareGreaterOrEqualUnsigned, Compare)
   DefineInitOApply(CompareGreaterUnsigned, Compare)
   DefineInitOApply(CompareGreaterOrEqualSigned, Compare)
   DefineInitOApply(CompareGreaterSigned, Compare)
   // DefineInitApply(CompareGreaterFloat)
   // DefineInitApply(CompareGreaterOrEqualFloat)
   DefineInitOApply(MinSignedAssign, DefaultAssign)
   DefineInitOApply(MinUnsignedAssign, DefaultAssign)
   // DefineInitApply(MinFloatAssign)
   DefineInitOApply(MaxSignedAssign, DefaultAssign)
   DefineInitOApply(MaxUnsignedAssign, DefaultAssign)
   // DefineInitApply(MaxFloatAssign)

   DefineInitOApply(BitOrAssign, DefaultAssign)
   DefineInitOApply(BitAndAssign, DefaultAssign)
   DefineInitOApply(BitExclusiveOrAssign, DefaultAssign)
   DefineInitOApply(BitNegateAssign, DefaultAssign)
   DefineInitOApply(LeftShiftAssign, DefaultAssign)
   DefineInitOApply(LogicalRightShiftAssign, DefaultAssign)
   DefineInitOApply(ArithmeticRightShiftAssign, DefaultAssign)
   DefineInitOApply(LeftRotateAssign, DefaultAssign)
   DefineInitOApply(RightRotateAssign, DefaultAssign)
#include "StandardClasses/DefineNew.h"
}

#undef DefineInitApply
#undef DefineInitOApply

MinusOperationElement::MethodApplyTable MinusOperationElement::matMethodApplyTable;

bool
MinusOperationElement::applyToDefault(const VirtualOperation& operation,
      VirtualElement& thisElement, EvaluationEnvironment& env) const {
   TransmitEnvironment applyEnv(env);
   applyEnv.setTopLatticeCreation();
   PPVirtualElement simplification(fstArg());
   applyEnv.setFirstArgument(sndArg());
   simplification->apply(getOperation(), applyEnv);
   if (applyEnv.hasResult())
      simplification = applyEnv.presult();
   applyEnv.merge();
   {  TransmitEnvironment applyEnv(env);
      applyEnv.setFirstArgument(*simplification);
      thisElement.apply(operation, applyEnv);
      if (applyEnv.hasResult())
         env.presult() = applyEnv.presult();
   }
   return true;
}
   
bool
MinusOperationElement::applyToCommutativeAssign(const VirtualOperation& aoperation, VirtualElement& thisElement, EvaluationEnvironment& env) const {
   TransmitEnvironment applyEnv(env);
   applyEnv.setFirstArgument(thisElement);
   Operation operation((const Operation&) aoperation);
   if (!operation.isConstWithAssign())
      operation.setConstWithAssign();
   if (!const_cast<MinusOperationElement&>(*this).apply(operation, applyEnv))
      return false;
   env.presult() = applyEnv.presult();
   return true;
}

bool
MinusOperationElement::applyToConcat(const VirtualOperation& operation, VirtualElement& source, EvaluationEnvironment& env) const {
   ConcatOperationElement* result;
   {  ConcatOperationElement::Init init;
      init.setBitSize(1 + source.getSizeInBits());
      result = new ConcatOperationElement(init);
   };
   env.presult().absorbElement(result);
   result->sfstArg().setElement(source);
   result->ssndArg().setElement(*this);
   return true;
}

bool
MinusOperationElement::applyToCommutative(const VirtualOperation& aoperation, VirtualElement& thisElement, EvaluationEnvironment& env) const {
   TransmitEnvironment applyEnv(env);
   applyEnv.setFirstArgument(thisElement);
   Operation operation((const Operation&) aoperation);
   if (!const_cast<MinusOperationElement&>(*this).apply(operation, applyEnv))
      return false;
   env.presult() = applyEnv.presult();
   return true;
}

bool
MinusOperationElement::applyToMinusAssign(const VirtualOperation& operation, VirtualElement& thisElement, EvaluationEnvironment& env) const {
   Operation::Type thisOperationType = ((const Operation&) getOperation()).getType();
   Operation::Type operationType = ((const Operation&) operation).getType();
   if (thisOperationType != operationType)
      return applyToDefault(operation, thisElement, env);

   MinusOperationElement* copyThis = createSCopy();
   PPVirtualElement garbageCopy(copyThis, PNT::Pointer::Init());
   PPVirtualElement temp = copyThis->sfstArg();
   copyThis->sfstArg() = copyThis->ssndArg();   
   copyThis->ssndArg() = temp;
   Operation plusOperation;
   if (thisOperationType == Operation::TMinusSignedAssign)
      plusOperation.setPlusSignedAssign();
   else
      plusOperation.setPlusUnsignedAssign();
   TransmitEnvironment applyEnv(env);
   applyEnv.setFirstArgument(thisElement);
   return copyThis->applyMinusAssign(plusOperation, applyEnv);
}

bool
MinusOperationElement::applyToCompare(const VirtualOperation& aoperation, VirtualElement& thisElement, EvaluationEnvironment& env) const {
   TransmitEnvironment applyEnv(env);
   applyEnv.setFirstArgument(thisElement);
   Operation operation;
   const Operation& originalOperation = (const Operation&) aoperation;
   switch (originalOperation.getType()) {
      case Operation::TCompareLessSigned:
         operation.setCompareGreaterOrEqualSigned(); break;
      case Operation::TCompareLessUnsigned:
         operation.setCompareGreaterOrEqualUnsigned(); break;
      case Operation::TCompareGreaterSigned:
         operation.setCompareLessOrEqualSigned(); break;
      case Operation::TCompareGreaterUnsigned:
         operation.setCompareLessOrEqualUnsigned(); break;
      case Operation::TCompareLessOrEqualSigned:
         operation.setCompareGreaterSigned(); break;
      case Operation::TCompareLessOrEqualUnsigned:
         operation.setCompareGreaterUnsigned(); break;
      case Operation::TCompareGreaterOrEqualSigned:
         operation.setCompareLessSigned(); break;
      case Operation::TCompareGreaterOrEqualUnsigned:
         operation.setCompareLessUnsigned(); break;
      case Operation::TCompareEqual:
         operation.setCompareEqual(); break;
      case Operation::TCompareDifferent:
         operation.setCompareDifferent(); break;
      default:
         return applyToDefault(aoperation, thisElement, env);
   };
   bool result = const_cast<MinusOperationElement&>(*this).applyCompare(operation, applyEnv);
   env.presult() = applyEnv.presult();
   return result;
}

#define DefineInitApplyTo(TypeOperation)                                                         \
   elementAt(Operation::T##TypeOperation).setMethod(&MinusOperationElement::applyTo##TypeOperation);
#define DefineInitOApplyTo(TypeOperation, TypeMethod)                                            \
   elementAt(Operation::T##TypeOperation).setMethod(&MinusOperationElement::applyTo##TypeMethod);

MinusOperationElement::MethodApplyToTable::MethodApplyToTable() {
#include "StandardClasses/UndefineNew.h"
   clear();
   DefineInitApplyTo(Concat)
   DefineInitOApplyTo(BitSet, Default)
   DefineInitOApplyTo(PlusSignedAssign, CommutativeAssign)
   DefineInitOApplyTo(PlusUnsignedAssign, CommutativeAssign)
   DefineInitOApplyTo(PlusUnsignedWithSignedAssign, CommutativeAssign)
   DefineInitOApplyTo(PlusFloatAssign, Default)
   DefineInitOApplyTo(MinusSignedAssign, MinusAssign)
   DefineInitOApplyTo(MinusUnsignedAssign, MinusAssign)
   DefineInitOApplyTo(MinusFloatAssign, Default)

   DefineInitOApplyTo(CompareLessSigned, Compare)
   DefineInitOApplyTo(CompareLessUnsigned, Compare)
   DefineInitOApplyTo(CompareLessFloat, Default)
   DefineInitOApplyTo(CompareLessOrEqualSigned, Compare)
   DefineInitOApplyTo(CompareLessOrEqualUnsigned, Compare)
   DefineInitOApplyTo(CompareLessOrEqualFloat, Default)
   DefineInitOApplyTo(CompareEqual, Commutative)
   DefineInitOApplyTo(CompareEqualFloat, Default)
   DefineInitOApplyTo(CompareDifferent, Commutative)
   DefineInitOApplyTo(CompareDifferentFloat, Default)
   DefineInitOApplyTo(CompareGreaterOrEqualSigned, Compare)
   DefineInitOApplyTo(CompareGreaterOrEqualUnsigned, Compare)
   DefineInitOApplyTo(CompareGreaterOrEqualFloat, Default)
   DefineInitOApplyTo(CompareGreaterSigned, Compare)
   DefineInitOApplyTo(CompareGreaterUnsigned, Compare)
   DefineInitOApplyTo(CompareGreaterFloat, Default)
   DefineInitOApplyTo(MinSignedAssign, CommutativeAssign)
   DefineInitOApplyTo(MinUnsignedAssign, CommutativeAssign)
   DefineInitOApplyTo(MinFloatAssign, CommutativeAssign)
   DefineInitOApplyTo(MaxSignedAssign, CommutativeAssign)
   DefineInitOApplyTo(MaxUnsignedAssign, CommutativeAssign)
   DefineInitOApplyTo(MaxFloatAssign, CommutativeAssign)

   DefineInitOApplyTo(TimesSignedAssign, CommutativeAssign)
   DefineInitOApplyTo(TimesUnsignedAssign, CommutativeAssign)
   DefineInitOApplyTo(TimesFloatAssign, CommutativeAssign)
   DefineInitOApplyTo(DivideSignedAssign, Default)
   DefineInitOApplyTo(DivideUnsignedAssign, Default)
   DefineInitOApplyTo(DivideFloatAssign, Default)
   DefineInitOApplyTo(ModuloSignedAssign, Default)
   DefineInitOApplyTo(ModuloUnsignedAssign, Default)
   DefineInitOApplyTo(BitOrAssign, CommutativeAssign)
   DefineInitOApplyTo(BitAndAssign, CommutativeAssign)
   DefineInitOApplyTo(BitExclusiveOrAssign, Default)
   DefineInitOApplyTo(LeftShiftAssign, Default)
   DefineInitOApplyTo(LogicalRightShiftAssign, Default)
   DefineInitOApplyTo(ArithmeticRightShiftAssign, Default)
   DefineInitOApplyTo(LeftRotateAssign, Default)
   DefineInitOApplyTo(RightRotateAssign, Default)
#include "StandardClasses/DefineNew.h"
}
#undef DefineInitOApplyTo
#undef DefineInitApplyTo

MinusOperationElement::MethodApplyToTable MinusOperationElement::mattMethodApplyToTable;

bool
BaseOperationElement::constraintCompareGeneric(const VirtualOperation& operation, VirtualElement& source,
      Argument arg, ConstraintEnvironment& env) {
   TransmitEnvironment applyEnv(env);
   applyEnv.setTopLatticeCreation();
   applyEnv.setFirstArgument(sndArg());
   PPVirtualElement thisElement(fstArg());
   thisElement->apply(getOperation(), applyEnv);
   if (applyEnv.hasResult())
      thisElement = applyEnv.presult();
   applyEnv.merge();
   
   PPVirtualElement trueElement = Methods::newTrue(*thisElement);
   ConstraintTransmitEnvironment transmitEnv(env, source, arg);
   assume(thisElement->constraint(operation, *trueElement, transmitEnv));
   return true;
}

#define DefineInitUnaryConstraint(TypeOperation) \
   mcutUnaryTable[Operation::T##TypeOperation].setMethod(&VirtualElement::constraint##TypeOperation);

#define DefineInitUnaryGConstraint(TypeOperation) \
   mcutUnaryTable[Operation::T##TypeOperation].setFunction(&MultiBit::Approximate::Details::BaseAlgorithms::constraint##TypeOperation);

#define DefineInitUnaryOConstraint(TypeOperation, TypeMethod) \
   mcutUnaryTable[Operation::T##TypeOperation].setMethod(&VirtualElement::constraint##TypeMethod);

#define DefineInitUnaryOGConstraint(TypeOperation, TypeMethod) \
   mcutUnaryTable[Operation::T##TypeOperation].setFunction(&MultiBit::Approximate::Details::BaseAlgorithms::constraint##TypeMethod);

#define DefineInitUnaryOGIntConstraint(TypeOperation, TypeMethod) \
   mcutUnaryTable[Operation::T##TypeOperation].setFunction(&Scalar::Approximate::Details::IntOperationElement::constraint##TypeMethod);

#define DefineInitUnaryGIntConstraint(TypeOperation) \
   mcutUnaryTable[Operation::T##TypeOperation].setFunction(&Scalar::Approximate::Details::IntOperationElement::constraint##TypeOperation);

#define DefineInitBinaryConstraint(TypeOperation) \
   mcbtBinaryTable[Operation::T##TypeOperation].setMethod(&MinusOperationElement::constraint##TypeOperation);

#define DefineInitBinaryGConstraint(TypeOperation) \
   mcbtBinaryTable[Operation::T##TypeOperation].setFunction(&MultiBit::Approximate::Details::BaseAlgorithms::constraint##TypeOperation);

#define DefineInitBinaryGIntConstraint(TypeOperation) \
   mcbtBinaryTable[Operation::T##TypeOperation].setFunction(&Scalar::Approximate::Details::IntOperationElement::constraint##TypeOperation);

#define DefineInitBinaryOGConstraint(TypeOperation, TypeFunction) \
   mcbtBinaryTable[Operation::T##TypeOperation].setFunction(&MultiBit::Approximate::Details::BaseAlgorithms::constraint##TypeFunction);

#define DefineInitBinaryOGIntConstraint(TypeOperation, TypeFunction) \
   mcbtBinaryTable[Operation::T##TypeOperation].setFunction(&Scalar::Approximate::Details::IntOperationElement::constraint##TypeFunction);

#define DefineInitBinaryConstraintGeneric(TypeOperation)                                         \
   mcbtBinaryTable[Operation::T##TypeOperation].setArray(                                        \
      new Scalar::Approximate::Details::GenericBinaryConstraintMethod<DomainTraits>(&MinusOperationElement::constraint##TypeOperation));

#define DefineInitOBinaryConstraintGeneric(TypeOperation, TypeMethod)                            \
   mcbtBinaryTable[Operation::T##TypeOperation].setArray(                                        \
      new Scalar::Approximate::Details::GenericBinaryConstraintMethod<DomainTraits>(&MinusOperationElement::constraint##TypeMethod));

#define DefineInitOCompareConstraint(TypeOperation, TypeMethod) \
   mcctCompareTable[Operation::T##TypeOperation].setMethod(&BaseOperationElement::constraint##TypeMethod);

MinusOperationElement::MethodConstraintTable::MethodConstraintTable() {
#include "StandardClasses/UndefineNew.h"
   DefineInitUnaryGConstraint(ExtendWithZero)
   DefineInitUnaryGConstraint(ExtendWithSign)
   DefineInitBinaryGConstraint(Concat)
   DefineInitUnaryGConstraint(Reduce)
   DefineInitBinaryGConstraint(BitSet)

   DefineInitUnaryOGIntConstraint(CastChar, CastInteger)
   DefineInitUnaryOGIntConstraint(CastInt, CastInteger)
   DefineInitUnaryOGIntConstraint(CastUnsignedInt, CastInteger)
   // DefineInitUnaryOGConstraint(CastFloat, CastDouble)
   // DefineInitUnaryOGConstraint(CastDouble, CastDouble)
   // DefineInitUnaryOGConstraint(CastLongDouble, CastDouble)

   DefineInitOCompareConstraint(CompareLessSigned, CompareGeneric)
   DefineInitOCompareConstraint(CompareLessUnsigned, CompareGeneric)
   DefineInitOCompareConstraint(CompareLessOrEqualSigned, CompareGeneric)
   DefineInitOCompareConstraint(CompareLessOrEqualUnsigned, CompareGeneric)
   DefineInitOCompareConstraint(CompareEqual, CompareGeneric)
   DefineInitOCompareConstraint(CompareDifferent, CompareGeneric)
   DefineInitOCompareConstraint(CompareGreaterOrEqualSigned, CompareGeneric)
   DefineInitOCompareConstraint(CompareGreaterOrEqualUnsigned, CompareGeneric)
   DefineInitOCompareConstraint(CompareGreaterSigned, CompareGeneric)
   DefineInitOCompareConstraint(CompareGreaterUnsigned, CompareGeneric)
   DefineInitOBinaryConstraintGeneric(MinUnsignedAssign, MinAssign)
   DefineInitOBinaryConstraintGeneric(MinSignedAssign, MinAssign)
   DefineInitOBinaryConstraintGeneric(MaxUnsignedAssign, MaxAssign)
   DefineInitOBinaryConstraintGeneric(MaxSignedAssign, MaxAssign)

   DefineInitUnaryOGIntConstraint(PrevSignedAssign, PrevAssign)
   DefineInitUnaryOGIntConstraint(PrevUnsignedAssign, PrevAssign)
   DefineInitUnaryOGIntConstraint(NextSignedAssign, NextAssign)
   DefineInitUnaryOGIntConstraint(NextUnsignedAssign, NextAssign)

   DefineInitBinaryGConstraint(PlusSignedAssign)
   DefineInitBinaryGConstraint(PlusUnsignedAssign)
   DefineInitBinaryGConstraint(PlusUnsignedWithSignedAssign)
   DefineInitBinaryOGIntConstraint(MinusSignedAssign, MinusAssign)
   DefineInitBinaryOGIntConstraint(MinusUnsignedAssign, MinusAssign)
   DefineInitBinaryGConstraint(TimesSignedAssign)
   DefineInitBinaryGConstraint(TimesUnsignedAssign)
   DefineInitBinaryGConstraint(DivideSignedAssign)
   DefineInitBinaryGConstraint(DivideUnsignedAssign)

   DefineInitUnaryOGIntConstraint(OppositeSignedAssign, OppositeAssign)

   DefineInitBinaryGConstraint(ModuloSignedAssign)
   DefineInitBinaryGConstraint(ModuloUnsignedAssign)
   DefineInitBinaryGIntConstraint(BitOrAssign)
   DefineInitBinaryGIntConstraint(BitAndAssign)
   DefineInitBinaryGIntConstraint(BitExclusiveOrAssign)
   DefineInitUnaryGIntConstraint(BitNegateAssign)
   // DefineInitBinaryGConstraint(LeftShiftAssign)
   // DefineInitBinaryGConstraint(LogicalRightShiftAssign)
   // DefineInitBinaryGConstraint(ArithmeticRightShiftAssign)

#include "StandardClasses/DefineNew.h"
}

#undef DefineInitOCompareConstraint
#undef DefineInitUnaryConstraint
#undef DefineInitUnaryGConstraint
#undef DefineInitUnaryOGIntConstraint
#undef DefineInitUnaryGIntConstraint
#undef DefineInitUnaryOConstraint
#undef DefineInitUnaryOGConstraint
#undef DefineInitBinaryConstraint
#undef DefineInitBinaryGConstraint
#undef DefineInitBinaryOGConstraint
#undef DefineInitBinaryOGIntConstraint
#undef DefineInitBinaryGIntConstraint
#undef DefineInitBinaryConstraintGeneric
#undef DefineInitOBinaryConstraintGeneric

MinusOperationElement::MethodConstraintTable MinusOperationElement::mctMethodConstraintTable;

/**************************************************************/
/* Implementation of the methods MinusOperationElement::query */
/**************************************************************/

bool
MinusOperationElement::queryCompareSpecial(const VirtualElement& thisElement, const VirtualQueryOperation& queryOperation, VirtualQueryOperation::Environment& env) {
   ((QueryOperation::CompareSpecialEnvironment&) env).fill();
   return true;
}

bool
MinusOperationElement::querySimplification(const VirtualElement& thisElement, const VirtualQueryOperation&, VirtualQueryOperation::Environment& env) {
   return true;
}

bool
MinusOperationElement::querySimplificationAsConstantDisjunction(const VirtualElement& thisElement,
      const VirtualQueryOperation& operation, VirtualQueryOperation::Environment& env) {
   return true;
}

bool
MinusOperationElement::queryDispatchOnTopResult(const VirtualElement& thisElement, const VirtualQueryOperation& operation, VirtualQueryOperation::Environment& env) {
   AssumeCondition(dynamic_cast<const MinusOperationElement*>(&thisElement))
   return Methods::newTop(((const MinusOperationElement&) thisElement).fstArg())->query(operation, env);
}

#define DefineInitQuery(TypeOperation) \
   elementAt(QueryOperation::TypeOperation).setMethod(&MinusOperationElement::query##TypeOperation);

MinusOperationElement::QueryTable::QueryTable() {
#include "StandardClasses/UndefineNew.h"
   FunctionQueryTable* table = &elementAt(VirtualQueryOperation::TCompareSpecial);
   table->setSize(QueryOperation::EndOfTypeCompareSpecial);
   (*table)[QueryOperation::TCSGuard].setMethod(&MinusOperationElement::queryGuardAll);
   (*table)[QueryOperation::TCSNatural].setMethod(&MinusOperationElement::queryCompareSpecial);
   (*table)[QueryOperation::TCSSigned].setMethod(&MinusOperationElement::queryCompareSpecial);
   (*table)[QueryOperation::TCSUnsigned].setMethod(&MinusOperationElement::queryCompareSpecial);
   (*table)[QueryOperation::TCSBitDomain].setMethod(&MinusOperationElement::queryDispatchOnTopResult);
   
   table = &elementAt(VirtualQueryOperation::TSimplification);
   table->setSize(QueryOperation::EndOfTypeSimplification);
   (*table)[QueryOperation::TSConstant].setMethod(&Top::queryFailSimplification);
   (*table)[QueryOperation::TSInterval].setMethod(&MinusOperationElement::querySimplificationAsConstantDisjunction);
   (*table)[QueryOperation::TSConstantDisjunction].setMethod(&MinusOperationElement::querySimplificationAsConstantDisjunction);

   table = &elementAt(VirtualQueryOperation::TDomain);
   table->setSize(QueryOperation::EndOfTypeDomain);

#define DefineInitQueryMethodTable(TypeOperation)                              \
   (*table)[QueryOperation::TD##TypeOperation].setMethod(&MinusOperationElement::queryDispatchOnTopResult);
   
   DefineInitQueryMethodTable(Disjunction)
   DefineInitQueryMethodTable(Interval)
   DefineInitQueryMethodTable(BooleanDisjunction)
   DefineInitQueryMethodTable(Top)
   DefineInitQueryMethodTable(False)
   DefineInitQueryMethodTable(True)
   DefineInitQueryMethodTable(IntForShift)
   DefineInitQueryMethodTable(Zero)
   DefineInitQueryMethodTable(MinusOne)
   DefineInitQueryMethodTable(One)
   DefineInitQueryMethodTable(Min)
   DefineInitQueryMethodTable(Max)

#undef DefineInitQueryMethodTable

   table = &elementAt(VirtualQueryOperation::TExtern);
   table->setSize(QueryOperation::EndOfTypeExtern);
   (*table)[QueryOperation::TESigned].setMethod(&MinusOperationElement::queryDispatchOnTopResult);
#include "StandardClasses/DefineNew.h"
}
#undef DefineInitQuery

MinusOperationElement::QueryTable MinusOperationElement::mqtMethodQueryTable;

#define DefineTypeOperation MultiBit
#define DefineTypeObject MinusOperationElement
#include "Analyzer/Scalar/Approximate/VirtualCall.incc"
#undef DefineTypeObject
#undef DefineTypeOperation

bool
MinusOperationElement::mergeWith(const Scalar::VirtualElement& asource, Scalar::EvaluationEnvironment& aenv) {
   if (VirtualElement::mergeWith(asource, aenv))
      return true;
   TransmitEnvironment applyEnv((EvaluationEnvironment&) aenv);
   applyEnv.setTopLatticeCreation();
   applyEnv.setFirstArgument(sndArg());
   fstArg().apply(getOperation(), applyEnv);
   PPVirtualElement arg;
   if (applyEnv.hasResult())
      arg = applyEnv.presult();
   else
      arg = sfstArg();
   applyEnv.merge();
   assume(arg->mergeWith(asource, aenv));
   if (!aenv.hasResult())
      aenv.presult() = arg;
   return true;
}

bool
MinusOperationElement::contain(const Scalar::VirtualElement& asource, Scalar::EvaluationEnvironment& aenv) const {
   if (VirtualElement::contain(asource, aenv))
      return true;
   TransmitEnvironment applyEnv((EvaluationEnvironment&) aenv);
   applyEnv.setTopLatticeCreation();
   applyEnv.setFirstArgument(sndArg());
   Operation operation((const Operation&) getOperation());
   if (!operation.isConst())
      operation.setConstWithAssign();
   fstArg().apply(operation, applyEnv);
   PPVirtualElement arg = applyEnv.presult();
   return arg->contain(asource, aenv);
}

bool
MinusOperationElement::mergeWithTo(VirtualElement& source, EvaluationEnvironment& env) const {
   TransmitEnvironment applyEnv(env);
   applyEnv.setTopLatticeCreation();
   applyEnv.setFirstArgument(sndArg());
   Operation operation((const Operation&) getOperation());
   if (!operation.isConst())
      operation.setConstWithAssign();
   fstArg().apply(operation, applyEnv);
   PPVirtualElement arg = applyEnv.presult();
   return source.mergeWith(*arg, env);
}

bool
MinusOperationElement::containTo(const VirtualElement& source, EvaluationEnvironment& env) const {
   TransmitEnvironment applyEnv(env);
   applyEnv.setTopLatticeCreation();
   applyEnv.setFirstArgument(sndArg());
   Operation operation((const Operation&) getOperation());
   if (!operation.isConst())
      operation.setConstWithAssign();
   fstArg().apply(operation, applyEnv);
   PPVirtualElement arg = applyEnv.presult();
   if (source.getType().isVariable()) {
      AssumeCondition(dynamic_cast<const VirtualExactToApproximateElement*>(&source))
      const VirtualExactToApproximateElement& exactSource = (VirtualExactToApproximateElement&) source;
      return exactSource.domain().contain(*arg, env);
   }
   return source.contain(*arg, env);
}

bool
MinusOperationElement::intersectWith(const VirtualElement& source, EvaluationEnvironment& env) {
   if (VirtualElement::intersectWith(source, env))
      return true;
   TransmitEnvironment applyEnv(env);
   applyEnv.setTopLatticeCreation();
   applyEnv.setFirstArgument(sndArg());
   fstArg().apply(getOperation(), applyEnv);
   PPVirtualElement arg;
   if (applyEnv.hasResult())
      arg = applyEnv.presult();
   else
      arg = sfstArg();
   applyEnv.merge();
   assume(arg->intersectWith(source, env));
   if (!env.hasResult())
      env.presult() = arg;
   return true;
}

bool
MinusOperationElement::intersectWithTo(VirtualElement& source, EvaluationEnvironment& env) const {
   TransmitEnvironment applyEnv(env);
   applyEnv.setTopLatticeCreation();
   applyEnv.setFirstArgument(sndArg());
   Operation operation((const Operation&) getOperation());
   if (!operation.isConst())
      operation.setConstWithAssign();
   fstArg().apply(operation, applyEnv);
   PPVirtualElement arg = applyEnv.presult();
   return source.intersectWith(*arg, env);
}

}} // end of namespace MultiBit::Approximate

}} // end of namespace Analyzer::Scalar


