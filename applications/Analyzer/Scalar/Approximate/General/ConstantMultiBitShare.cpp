/////////////////////////////////
//
// Library   : Analyzer/Scalar
// Unit      : approximate scalar elements
// File      : ConstantMultiBitShare.cpp
// Author    : Franck Vedrine
// Copyright : CEA LIST - 2019-2020 - all rights reserved
//
// Description :
//   Implementation of an approximated integer class for multibit operations
//     but existentially quantified. It represents only one value but
//     still unknown. This value may be shared with other values which expresses
//     relational information.
//   This definition relies on independent host analyses.
//

#include "Analyzer/Scalar/Approximate/General/ConstantMultiBitShare.h"
#include "Analyzer/Scalar/Approximate/Top.h"
#include "Analyzer/Scalar/Approximate/FormalOperation.h"
#include "Analyzer/Scalar/Approximate/IntervalInteger.h"
// #include "Analyzer/Scalar/Scalar.hpp"

namespace Analyzer {}

#include "Analyzer/Scalar/Approximate/MethodTable.template"

namespace Analyzer { namespace Scalar { namespace Approximate {

int VirtualExactToApproximateElement::SharedApproximate::uSharedIdentifier = 0; // [TODO] not reentrant

PPVirtualElement
VirtualExactToApproximateElement::simplify() {
   ApproxKind approxKind = sapDomain->shared().getApproxKind();
   if (approxKind.isConstant())
      return sapDomain->isSingleReferenced() ? sapDomain->extractShared() : PPVirtualElement(sapDomain->shared());
   else if (approxKind.isVariable()) {
      AssumeCondition(dynamic_cast<const VirtualExactToApproximateElement*>(&sapDomain->shared()))
      VirtualExactToApproximateElement& subElement = (VirtualExactToApproximateElement&) sapDomain->sshared();
      PPVirtualElement result = subElement.simplify();
      if (result.isValid()) { // alpha renaming
         SharedApproximate* oldDomain = &*subElement.sapDomain;
         subElement.sapDomain->moveAllCallsTo(*sapDomain);
         AssumeCondition(oldDomain != &*subElement.sapDomain)
         AssumeCondition(!oldDomain->isReferenced())
         delete oldDomain;
         sapDomain->setShared(result);
         approxKind = sapDomain->shared().getApproxKind();
         if (approxKind.isConstant())
            return sapDomain->isSingleReferenced() ? sapDomain->extractShared() : PPVirtualElement(sapDomain->shared());
      }
      else {
         SharedApproximate* oldDomain = &*sapDomain;
         sapDomain->moveAllCallsTo(*subElement.sapDomain);
         AssumeCondition(oldDomain != &*sapDomain)
         AssumeCondition(!oldDomain->isReferenced())
         delete oldDomain;
      };
   }
   else if (approxKind.isFormalConstruction()) {
      if (sapDomain->shared().isAtomic())
         return sapDomain->isSingleReferenced() ? sapDomain->extractShared() : PPVirtualElement(sapDomain->shared());
      // depending on Bit, MultiBit, can perform inner propagation
   };
   return PPVirtualElement();
}

VirtualExactToApproximateElement::VirtualExactToApproximateElement(const Init& init, PPVirtualElement shared/*, VirtualMemoryModifier* parent */)
   :  inherited(init), sapDomain(new SharedApproximate(shared), PNT::Pointer::Init())
   {  setType(ApproxKind::TVariable);
      // if (parent)
      //    sapDomain.setParent(*parent);
   }

VirtualExactToApproximateElement::VirtualExactToApproximateElement(const VirtualExactToApproximateElement& source)
   :  inherited(source), sapDomain(source.sapDomain) {}

void
VirtualExactToApproximateElement::SpecializationNotification::_update(PNT::SharedPointer& acall) {
   SharedApproximatePointer& call = (SharedApproximatePointer&) acall;
   if (!sapSpecializedDomain)
      sapSpecializedDomain = new SharedApproximate(getOrigin()->shared());
   PNT::SharedPointer* caller = &acall;
   if (call.apvmcParent.isValid() && call.apvmcParent->getContentUpdated(caller, *pvmdModificationDate))
      ((SharedApproximatePointer&) *caller).assign(sapSpecializedDomain, PNT::Pointer::Init());

   // PPVirtualElement& updateDomain = call.apvmcParent->getContentUpdated(call, *pvmdModificationDate);
   // AssumeCondition(dynamic_cast<const VirtualExactToApproximateElement*>(updateDomain.key()))
   // ((VirtualExactToApproximateElement&) *updateDomain).sapDomain.assign(sapSpecializedDomain, PNT::Pointer::Init());
}

bool
VirtualExactToApproximateElement::mergeWith(const Scalar::VirtualElement& asource, Scalar::EvaluationEnvironment& env) {
   bool result = inherited::mergeWith(asource, env);
   if (!result) {
      const VirtualElement& source = (const VirtualElement&) asource;
      if (source.getType().isVariable()) {
         AssumeCondition(dynamic_cast<const VirtualExactToApproximateElement*>(&source))
         auto& exactSource = const_cast<VirtualExactToApproximateElement&>((const VirtualExactToApproximateElement&) source);
         if (sapDomain == exactSource.sapDomain)
            return true;
         assumeSingle();
         exactSource.assumeSingle();
         result = sapDomain->sshared().mergeWith(exactSource.sapDomain->sshared(), env);
         if (result && env.hasResult())
            sapDomain->setShared(env.presult());
      }
      else {
         assumeSingle();
         result = sapDomain->sshared().mergeWith(source, env);
         if (result && env.hasResult())
            sapDomain->setShared(env.presult());
      };
   };
   return result;
}

bool
VirtualExactToApproximateElement::contain(const Scalar::VirtualElement& asource, Scalar::EvaluationEnvironment& aenv) const {
   bool result = inherited::contain(asource, aenv);
   if (!result) {
      const VirtualElement& source = (const VirtualElement&) asource;
      EvaluationEnvironment& env = (EvaluationEnvironment&) aenv;
      if (source.getType().isVariable()) {
         AssumeCondition(dynamic_cast<const VirtualExactToApproximateElement*>(&source))
         const auto& exactSource = (const VirtualExactToApproximateElement&) source;
         if (sapDomain == exactSource.sapDomain)
            return true;
         result = sapDomain->shared().contain(exactSource.sapDomain->shared(), env);
         if (!exactSource.sapDomain->isSingleReferenced()) {
            // AssumeUnimplemented
            // if (env.isTotalApplied())
            //    env.setPartialApplied();
         };
      }
      else {
         result = sapDomain->shared().contain(source, env);
         // AssumeUnimplemented
         // if (env.isTotalApplied())
         //    env.setPartialApplied();
      };
   };
   return result;
}

bool
VirtualExactToApproximateElement::intersectWith(const VirtualElement& source, EvaluationEnvironment& env) {
   bool result = inherited::intersectWith(source, env);
   if (!result) {
      if (source.getType().isVariable()) {
         AssumeCondition(dynamic_cast<const VirtualExactToApproximateElement*>(&source))
         auto& exactSource = const_cast<VirtualExactToApproximateElement&>((const VirtualExactToApproximateElement&) source);
         if (sapDomain == exactSource.sapDomain)
            return true;
         if (!env.isTopLevel()) {
            assumeSingle();
            exactSource.assumeSingle();
            result = sapDomain->sshared().intersectWith(exactSource.sapDomain->sshared(), env);
            if (result && env.hasResult()) {
               if (!((const VirtualElement&) *env.presult()).getType().isConstant())
                  sapDomain->setShared(env.presult());
            };
         }
         else {
            PPVirtualElement intersection(sapDomain->shared());
            result = intersection->intersectWith(exactSource.sapDomain->shared(), env);
            if (!result)
               return result;
            if (env.hasResult())
               intersection = env.presult();
            PPVirtualElement constantResult;
            if (intersection->getType().isConstant())
               constantResult.fullAssign(intersection);
            SharedApproximate* sharedDomain = nullptr;
            if (sapDomain->isSingleReferenced())
               sharedDomain = &*sapDomain;
            else
               sharedDomain = specializeOthers(env.getSModificationDate(), sharedDomain);
            sharedDomain = exactSource.specializeOthers(env.getSModificationDate(), sharedDomain);
            AssumeCondition(sharedDomain)
            if (&*sapDomain != sharedDomain) {
               sapDomain.release();
               sapDomain.setElement(*sharedDomain);
            };
            sharedDomain->setShared(intersection);
            if (constantResult.isValid())
               env.presult() = constantResult;
         };
      }
      else {
         if (!env.isTopLevel()) {
            assumeSingle();
            result = sapDomain->sshared().intersectWith(source, env);
            if (result && env.hasResult()) {
               if (!((const VirtualElement&) *env.presult()).getType().isConstant())
                  sapDomain->setShared(env.presult());
            };
         }
         else {
            PPVirtualElement intersection(sapDomain->shared());
            result = intersection->intersectWith(source, env);
            if (!result)
               return result;
            if (env.hasResult())
               intersection = env.presult();
            PPVirtualElement constantResult;
            if (intersection->getType().isConstant())
               constantResult.fullAssign(intersection);
            SharedApproximate* sharedDomain = nullptr;
            if (sapDomain->isSingleReferenced())
               sharedDomain = &*sapDomain;
            else
               sharedDomain = specializeOthers(env.getSModificationDate(), sharedDomain);
            AssumeCondition(sharedDomain)
            if (&*sapDomain != sharedDomain) {
               sapDomain.release();
               sapDomain.setElement(*sharedDomain);
            };
            sharedDomain->setShared(intersection);
            if (constantResult.isValid())
               env.presult() = constantResult;
         };
      };
   }
   else { // intersection with a constant
      if (!env.isTopLevel())
         return true;
      if (env.hasResult() && !sapDomain->isSingleReferenced()) {
         PPVirtualElement intersection = env.presult();
         PPVirtualElement constantResult;
         if (intersection->getType().isConstant())
            constantResult.fullAssign(intersection);
         SharedApproximate* sharedDomain = specializeOthers(env.getSModificationDate(), nullptr);
         AssumeCondition(sharedDomain)
         sapDomain.release();
         sapDomain.setElement(*sharedDomain);
         sharedDomain->setShared(intersection);
         if (constantResult.isValid())
            env.presult() = constantResult;
      };
   };
   return result;
}

bool
VirtualExactToApproximateElement::intersectWithTo(VirtualElement& source, EvaluationEnvironment& env) const {
   if (!env.isTopLevel())
      return source.intersectWith(sapDomain->shared(), env);
   else {
      PPVirtualElement intersection(sapDomain->shared());
      if (!intersection->intersectWith(source, env))
         return false;
      if (env.hasResult())
         intersection = env.presult();

      PNT::TPassPointer<VirtualExactToApproximateElement, PPAbstractElement>
         copy(*this);
      const_cast<VirtualExactToApproximateElement*>(this)->assumeSingle();
      PPVirtualElement constantResult;
      if (intersection->getType().isConstant())
         constantResult.fullAssign(intersection);
      SharedApproximate* sharedDomain = nullptr;
      if (copy->sapDomain->isSingleReferenced())
         sharedDomain = &*copy->sapDomain;
      else
         sharedDomain = copy->specializeOthers(env.getSModificationDate(), sharedDomain);
      AssumeCondition(sharedDomain)
      if (&*copy->sapDomain != sharedDomain) {
         copy->sapDomain.release();
         copy->sapDomain.setElement(*sharedDomain);
      };
      sharedDomain->setShared(intersection);
      if (constantResult.isValid())
         env.presult() = constantResult;
      else
         env.presult() = intersection;
      return true;
   };
}

bool
VirtualExactToApproximateElement::constraint(const VirtualOperation& operation, const VirtualElement& result, ConstraintEnvironment& env) {
   if (!inherited::constraint(operation, result, env)) {
      if (!env.isTopLevel()) {
         assumeSingle();
         if (!sapDomain->sshared().constraint(operation, result, env))
            return false;
         if (env.hasResult()) {
            PPVirtualElement copyResult;
            if (((const VirtualElement&) *env.presult()).getType().isConstant())
               copyResult.fullAssign(env.presult());
            sapDomain->setShared(env.presult());
            if (copyResult.isValid())
               env.presult() = copyResult;
         };
      }
      else {
         PPVirtualElement thisConstraint(sapDomain->shared());
         if (!thisConstraint->constraint(operation, result, env))
            return false;
         if (env.isEmpty())
            return true;
         if (env.hasResult())
            thisConstraint = env.presult();
         PPVirtualElement constantResult;
         if (thisConstraint->getType().isConstant())
            constantResult.fullAssign(thisConstraint);
         SharedApproximate* sharedDomain = nullptr;
         if (sapDomain->isSingleReferenced())
            sharedDomain = &*sapDomain;
         else
            sharedDomain = specializeOthers(env.getSModificationDate(), sharedDomain);
         AssumeCondition(sharedDomain)
         thisConstraint = Methods::intersect(thisConstraint, sapDomain->shared(), EvaluationParameters(env));
         if (thisConstraint.isValid())
            sharedDomain->setShared(thisConstraint);
         else {
            sharedDomain->setShared(sapDomain->shared());
            env.setEmpty();
         };
         if (sharedDomain != &*sapDomain) {
            sapDomain.release();
            sapDomain.setElement(*sharedDomain);
         };
         if (constantResult.isValid())
            env.presult() = constantResult;
      };
   }
   else {
      if (env.isEmpty())
         return true;
      if (!env.isTopLevel())
         return true;
      if (env.hasResult() && !sapDomain->isSingleReferenced()) {
         PPVirtualElement thisConstraint = env.presult();
         PPVirtualElement constantResult;
         if (thisConstraint->getType().isConstant())
            constantResult.fullAssign(thisConstraint);
         SharedApproximate* sharedDomain = specializeOthers(env.getSModificationDate(), nullptr);
         AssumeCondition(sharedDomain)
         thisConstraint = Methods::intersect(thisConstraint, sapDomain->shared(), EvaluationParameters(env));
         if (thisConstraint.isValid())
            sharedDomain->setShared(thisConstraint);
         else {
            sharedDomain->setShared(sapDomain->shared());
            env.setEmpty();
         };
         sapDomain.release();
         sapDomain.setElement(*sharedDomain);
         if (constantResult.isValid())
            env.presult() = constantResult;
      }
   }
   return true;
}

bool
VirtualExactToApproximateElement::constraintTo(const VirtualOperation& operation,
      VirtualElement& source, const VirtualElement& result, ConstraintEnvironment& env) {
   if (!inherited::constraintTo(operation, source, result, env)) {
      if (!env.isTopLevel()) {
         assumeSingle();
         ConstraintTransmitEnvironment copyEnv(env);
         PPVirtualElement thisArg((VirtualElement*) copyEnv.extractFirstArgument(), PNT::Pointer::Init()); // this
         copyEnv.absorbFirstArgument(sapDomain->sshared().createSCopy());
         if (!source.constraint(operation, result, copyEnv))
            return false;
         if (copyEnv.isEmpty()) {
            env.setEmpty();
            return true;
         }
         if (copyEnv.hasFirstArgument()) {
            PPVirtualElement copyArg;
            if (thisArg->getType().isConstant())
               copyArg.fullAssign(thisArg);
            sapDomain->setShared(thisArg);
            if (copyArg.isValid())
               env.absorbFirstArgument(copyArg.extractElement());
         }
         if (copyEnv.hasResult())
            env.presult() = copyEnv.presult();
      }
      else {
         ConstraintTransmitEnvironment copyEnv(env, PPVirtualElement(sapDomain->sshared()), ConstraintParameters(env));
         if (!source.constraint(operation, result, copyEnv))
            return false;
         if (copyEnv.isEmpty()) {
            env.setEmpty();
            return true;
         };
         if (copyEnv.hasFirstArgument()) {
            PPVirtualElement argumentConstraint((VirtualElement*) copyEnv.extractFirstArgument(), PNT::Pointer::Init());
            PPVirtualElement constantArgument;
            if (argumentConstraint->getType().isConstant())
               constantArgument.fullAssign(argumentConstraint);
            SharedApproximate* sharedDomain = nullptr;
            if (sapDomain->isSingleReferenced())
               sharedDomain = &*sapDomain;
            else
               sharedDomain = specializeOthers(env.getSModificationDate(), sharedDomain);
            AssumeCondition(sharedDomain)
            argumentConstraint = Methods::intersect(argumentConstraint, sapDomain->shared(), EvaluationParameters(env));
            if (argumentConstraint.isValid())
               sharedDomain->setShared(argumentConstraint);
            else {
               sharedDomain->setShared(sapDomain->shared());
               env.setEmpty();
            };
            if (sharedDomain != &*sapDomain) {
               sapDomain.release();
               sapDomain.setElement(*sharedDomain);
            }
            if (constantArgument.isValid())
               env.absorbFirstArgument(constantArgument.extractElement());
         };
         if (copyEnv.hasResult()) {
            if (source.isValid()) {
               IntersectEnvironment intersectEnv(copyEnv);
               PPVirtualElement temp = copyEnv.presult();
               temp->intersectWith(source, intersectEnv);
               if (intersectEnv.hasResult())
                  env.presult() = intersectEnv.presult();
               else
                  env.presult() = temp;
            }
            else
               env.presult() = copyEnv.presult();
         };
      };
   };
   return true;
}

bool
VirtualExactToApproximateElement::queryCopy(const VirtualElement& asource, const VirtualQueryOperation& operation, VirtualQueryOperation::Environment& aenv) {
   AssumeCondition(dynamic_cast<const VirtualExactToApproximateElement*>(&asource) && dynamic_cast<const VirtualQueryOperation::DuplicationEnvironment*>(&aenv))
   VirtualQueryOperation::DuplicationEnvironment& env = (VirtualQueryOperation::DuplicationEnvironment&) aenv;
   VirtualExactToApproximateElement* result = ((const VirtualExactToApproximateElement&) asource).createSCopy();
   env.presult().absorbElement(result);
   const auto& source = ((const VirtualExactToApproximateElement&) asource);
   if (env.hasMemoryModifier())
      result->setParent(env.getMemoryModifier());
   else if (source.sapDomain.hasParent())
      result->setParent(source.sapDomain.parent());
   if (env.hasOriginCaller() && &env.originCaller() == &source.sapDomain)
      env.setOriginCaller(result->sapDomain);
   return true;
}

bool
VirtualExactToApproximateElement::querySpecialize(const VirtualElement& asource, const VirtualQueryOperation& operation, VirtualQueryOperation::Environment& aenv) {
   AssumeCondition(dynamic_cast<const VirtualExactToApproximateElement*>(&asource) && dynamic_cast<const VirtualQueryOperation::DuplicationEnvironment*>(&aenv))
   VirtualQueryOperation::DuplicationEnvironment& env = (VirtualQueryOperation::DuplicationEnvironment&) aenv;
   VirtualExactToApproximateElement& result = const_cast<VirtualExactToApproximateElement&>((const VirtualExactToApproximateElement&) asource);
   if (env.hasMemoryModifier())
      result.setParent(env.getMemoryModifier());
   return true;
}

namespace Details {

template <class TypeBase>
void
TCompareOperationElement<TypeBase>::_write(STG::IOObject::OSBase& out, const STG::IOObject::FormatParameters& aparams) const {
   const auto& params = (const typename inherited::FormatParameters&) aparams;
   const char* extendFloat = !params.isDeterministic() ? "float" : "F";
   const char* extendSigned = !params.isDeterministic() ? "signed" : "S";
   const char* extendUnsigned = !params.isDeterministic() ? "unsigned" : "U";
   out.put('(');
   fstArg().write(out, params);
   out.put(' ');
   switch (((const MultiBitOperation&) inherited::getOperation()).getType()) {
      case MultiBitOperation::TCompareLessSigned: out << "<" << extendSigned; break;
      case MultiBitOperation::TCompareLessOrEqualSigned: out << "<=" << extendSigned; break;
      case MultiBitOperation::TCompareLessUnsigned: out << "<" << extendUnsigned; break;
      case MultiBitOperation::TCompareLessOrEqualUnsigned: out << "<=" << extendUnsigned; break;
      case MultiBitOperation::TCompareLessFloat: out << "<" << extendFloat; break;
      case MultiBitOperation::TCompareLessOrEqualFloat: out << "<=" << extendFloat; break;
      case MultiBitOperation::TCompareEqual: out << "=="; break;
      case MultiBitOperation::TCompareEqualFloat: out << "==" << extendFloat; break;
      case MultiBitOperation::TCompareDifferentFloat: out << "!=" << extendFloat; break;
      case MultiBitOperation::TCompareDifferent: out << "!="; break;
      case MultiBitOperation::TCompareGreaterOrEqualFloat: out << ">=" << extendFloat; break;
      case MultiBitOperation::TCompareGreaterFloat: out << ">" << extendFloat; break;
      case MultiBitOperation::TCompareGreaterOrEqualUnsigned: out << ">=" << extendUnsigned; break;
      case MultiBitOperation::TCompareGreaterUnsigned: out << ">" << extendUnsigned; break;
      case MultiBitOperation::TCompareGreaterOrEqualSigned: out << ">=" << extendSigned; break;
      case MultiBitOperation::TCompareGreaterSigned: out << ">" << extendSigned; break;
      default: { AssumeUncalled }
   };
   out.put(' ');
   sndArg().write(out, params);
   out.put(')');
}

template <class TypeBase>
AbstractElement::ZeroResult
TCompareOperationElement<TypeBase>::queryZeroResult() const {
   VirtualElement* fstArgument = &inherited::fstArg();
   if (fstArgument->getApproxKind().isVariable()) {
      AssumeCondition(dynamic_cast<const VirtualExactToApproximateElement*>(fstArgument))
      fstArgument = &((const VirtualExactToApproximateElement&) fstArg()).domain();
   }
   else if (fstArgument->getApproxKind().isFormalConstruction())
      fstArgument = nullptr;
   VirtualElement* sndArgument = &inherited::sndArg();
   if (sndArgument->getApproxKind().isVariable()) {
      AssumeCondition(dynamic_cast<const VirtualExactToApproximateElement*>(sndArgument))
      sndArgument = &((const VirtualExactToApproximateElement&) sndArg()).domain();
   }
   else if (sndArgument->getApproxKind().isFormalConstruction())
      sndArgument = nullptr;
   return (fstArgument && sndArgument)
      ?  IntOperationElement::Methods::applyBoolean(
               *fstArgument, inherited::getOperation(), *sndArgument)
      :  typename inherited::ZeroResult();
}
      
template <class TypeBase>
bool
TCompareOperationElement<TypeBase>::mergeWithTo(VirtualElement& asource, EvaluationEnvironment& env) const {
   env.mergeVerdictDegradate();
   PPVirtualElement multiBitTop = IntOperationElement::Methods::newTop(fstArg());
   env.presult() = convertArgumentTopToResult(multiBitTop);
   return true;
}

template <class TypeBase>
bool
TCompareOperationElement<TypeBase>::containTo(const VirtualElement& asource, EvaluationEnvironment& env) const {
   env.mergeVerdictDegradate();
   env.setPartialApplied();
   return true;
}

template <class TypeBase>
bool
TCompareOperationElement<TypeBase>::intersectWithTo(VirtualElement& asource, EvaluationEnvironment& env) const {
   env.mergeVerdictDegradate();
   // with a guard
   return true;
}

template <class TypeBase>
bool
TCompareOperationElement<TypeBase>::mergeWith(const Scalar::VirtualElement& asource, Scalar::EvaluationEnvironment& aenv) {
   if (inherited::mergeWith(asource, aenv))
      return true;
   EvaluationEnvironment& env = (EvaluationEnvironment&) aenv;
   if (((const VirtualElement&) asource).getApproxKind().isFormalOperation()) {
      AssumeCondition(dynamic_cast<const typename inherited::FormalOperationElement*>(&asource))
      const typename inherited::FormalOperationElement& source = (const typename inherited::FormalOperationElement&) asource;
      MultiBitOperation::Type type = ((const MultiBitOperation&) source.getOperation()).getType();
      if (type >= MultiBitOperation::StartOfCompare && type < MultiBitOperation::EndOfCompare) {
         if (fstArg().getApproxKind().isVariable()) {
            if (source.fstArg().getApproxKind().isVariable()) {
               AssumeCondition(dynamic_cast<const VirtualExactToApproximateElement*>(&fstArg()) && dynamic_cast<const VirtualExactToApproximateElement*>(&source.fstArg()))
               if (((const VirtualExactToApproximateElement&) fstArg()).isEqual((const VirtualExactToApproximateElement&) source.fstArg())) {
                  MultiBitOperation::Type thisTypeCompare = getOperationType();
                  MultiBitOperation::Type sourceTypeCompare = ((const MultiBitOperation&) source.getOperation()).getType();
                  if (thisTypeCompare == sourceTypeCompare) {
                     inherited::assume(ssndArg()->mergeWith(source.sndArg(), env));
                     if (env.hasResult())
                        ssndArg() = env.presult();
                     return true;
                  }
                  else if (thisTypeCompare == 1+sourceTypeCompare) {
                  }
                  else if (sourceTypeCompare == 1+thisTypeCompare) {
                  };
               };
            };
            if (source.sndArg().getApproxKind().isVariable()) {
               AssumeCondition(dynamic_cast<const VirtualExactToApproximateElement*>(&fstArg()) && dynamic_cast<const VirtualExactToApproximateElement*>(&source.sndArg()))
               if (((const VirtualExactToApproximateElement&) fstArg()).isEqual((const VirtualExactToApproximateElement&) source.sndArg())) {
                  MultiBitOperation::Type thisTypeCompare = getOperationType();
                  MultiBitOperation::Type sourceTypeCompare = ((const MultiBitOperation&) source.getOperation()).getType();
                  if (sourceTypeCompare < MultiBitOperation::TCompareEqual)
                     sourceTypeCompare = (MultiBitOperation::Type) (MultiBitOperation::EndOfCompare - (sourceTypeCompare - MultiBitOperation::StartOfCompare) - 1);
                  else if (sourceTypeCompare > MultiBitOperation::TCompareDifferent)
                     sourceTypeCompare = (MultiBitOperation::Type) (MultiBitOperation::EndOfCompare - (sourceTypeCompare - MultiBitOperation::StartOfCompare) - 1);
                  if (thisTypeCompare == sourceTypeCompare) {
                     inherited::assume(ssndArg()->mergeWith(source.fstArg(), env));
                     if (env.hasResult())
                        ssndArg() = env.presult();
                     return true;
                  }
                  else if (thisTypeCompare == 1+sourceTypeCompare) {
                  }
                  else if (sourceTypeCompare == 1+thisTypeCompare) {
                  };
               };
            };
         };
         if (sndArg().getApproxKind().isVariable()) {
            if (source.fstArg().getApproxKind().isVariable()) {
               AssumeCondition(dynamic_cast<const VirtualExactToApproximateElement*>(&sndArg()) && dynamic_cast<const VirtualExactToApproximateElement*>(&source.fstArg()))
               if (((const VirtualExactToApproximateElement&) sndArg()).isEqual((const VirtualExactToApproximateElement&) source.fstArg())) {
                  MultiBitOperation::Type thisTypeCompare = getOperationType();
                  if (thisTypeCompare < MultiBitOperation::TCompareEqual)
                     thisTypeCompare = (MultiBitOperation::Type) (MultiBitOperation::EndOfCompare - (thisTypeCompare - MultiBitOperation::StartOfCompare) - 1);
                  else if (thisTypeCompare > MultiBitOperation::TCompareDifferent)
                     thisTypeCompare = (MultiBitOperation::Type) (MultiBitOperation::EndOfCompare - (thisTypeCompare - MultiBitOperation::StartOfCompare) - 1);
                  MultiBitOperation::Type sourceTypeCompare = ((const MultiBitOperation&) source.getOperation()).getType();
                  if (thisTypeCompare == sourceTypeCompare) {
                     inherited::assume(sfstArg()->mergeWith(source.sndArg(), env));
                     if (env.hasResult())
                        sfstArg() = env.presult();
                     return true;
                  }
                  else if (thisTypeCompare == 1+sourceTypeCompare) {
                  }
                  else if (sourceTypeCompare == 1+thisTypeCompare) {
                  };
               };
            };
            if (source.sndArg().getApproxKind().isVariable()) {
               AssumeCondition(dynamic_cast<const VirtualExactToApproximateElement*>(&sndArg()) && dynamic_cast<const VirtualExactToApproximateElement*>(&source.sndArg()))
               if (((const VirtualExactToApproximateElement&) sndArg()).isEqual((const VirtualExactToApproximateElement&) source.sndArg())) {
                  MultiBitOperation::Type thisTypeCompare = getOperationType();
                  MultiBitOperation::Type sourceTypeCompare = ((const MultiBitOperation&) source.getOperation()).getType();
                  if (thisTypeCompare < MultiBitOperation::TCompareEqual)
                     thisTypeCompare = (MultiBitOperation::Type) (MultiBitOperation::EndOfCompare - (thisTypeCompare - MultiBitOperation::StartOfCompare) - 1);
                  else if (sourceTypeCompare > MultiBitOperation::TCompareDifferent)
                     thisTypeCompare = (MultiBitOperation::Type) (MultiBitOperation::EndOfCompare - (thisTypeCompare - MultiBitOperation::StartOfCompare) - 1);
                  if (sourceTypeCompare < MultiBitOperation::TCompareEqual)
                     sourceTypeCompare = (MultiBitOperation::Type) (MultiBitOperation::EndOfCompare - (sourceTypeCompare - MultiBitOperation::StartOfCompare) - 1);
                  else if (sourceTypeCompare > MultiBitOperation::TCompareDifferent)
                     thisTypeCompare = (MultiBitOperation::Type) (MultiBitOperation::EndOfCompare - (sourceTypeCompare - MultiBitOperation::StartOfCompare) - 1);
                  if (thisTypeCompare == sourceTypeCompare) {
                     inherited::assume(sfstArg()->mergeWith(source.fstArg(), env));
                     if (env.hasResult())
                        sfstArg() = env.presult();
                     return true;
                  }
                  else if (thisTypeCompare == 1+sourceTypeCompare) {
                  }
                  else if (sourceTypeCompare == 1+thisTypeCompare) {
                  };
               };
            };
         };
      };
   };
   env.mergeVerdictDegradate();
   PPVirtualElement multiBitTop = Approximate::Details::IntOperationElement::Methods::newTop(fstArg());
   env.presult() = convertArgumentTopToResult(multiBitTop);
   return true;
}

template <class TypeBase>
bool
TCompareOperationElement<TypeBase>::contain(const Scalar::VirtualElement& asource, Scalar::EvaluationEnvironment& aenv) const {
   if (inherited::contain(asource, aenv))
      return true;
   EvaluationEnvironment& env = (EvaluationEnvironment&) aenv;
   if (((const VirtualElement&) asource).getApproxKind().isFormalOperation()) {
      AssumeCondition(dynamic_cast<const typename inherited::FormalOperationElement*>(&asource))
      const typename inherited::FormalOperationElement& source = (const typename inherited::FormalOperationElement&) asource;
      MultiBitOperation::Type type = ((const MultiBitOperation&) source.getOperation()).getType();
      if (type >= MultiBitOperation::StartOfCompare && type < MultiBitOperation::EndOfCompare) {
         if (fstArg().getApproxKind().isVariable()) {
            if (source.fstArg().getApproxKind().isVariable()) {
               AssumeCondition(dynamic_cast<const VirtualExactToApproximateElement*>(&fstArg()) && dynamic_cast<const VirtualExactToApproximateElement*>(&source.fstArg()))
               if (((const VirtualExactToApproximateElement&) fstArg()).isEqual((const VirtualExactToApproximateElement&) source.fstArg())) {
                  MultiBitOperation::Type thisTypeCompare = getOperationType();
                  MultiBitOperation::Type sourceTypeCompare = ((const MultiBitOperation&) source.getOperation()).getType();
                  if (thisTypeCompare == sourceTypeCompare) {
                     bool result = sndArg().contain(source.sndArg(), env);
                     return result;
                  }
                  else if (thisTypeCompare == 1+sourceTypeCompare) {
                  }
                  else if (sourceTypeCompare == 1+thisTypeCompare) {
                  };
               };
               EvaluationEnvironment domainEnv = EvaluationEnvironment(typename inherited::EvaluationParameters(env)); // \forall this. \exists source. ...
               inherited::assume(((const VirtualExactToApproximateElement&) fstArg()).domain().contain(((const VirtualExactToApproximateElement&) source.fstArg()).domain(), domainEnv));
               if (domainEnv.isTotalApplied()) {
                  MultiBitOperation::Type thisTypeCompare = getOperationType();
                  MultiBitOperation::Type sourceTypeCompare = ((const MultiBitOperation&) source.getOperation()).getType();
                  if (thisTypeCompare == sourceTypeCompare) {
                     EvaluationEnvironment resultEnv = EvaluationEnvironment(typename inherited::EvaluationParameters(env));
                     bool result = sndArg().contain(source.sndArg(), resultEnv);
                     if (!result || resultEnv.isTotalApplied())
                        return result;
                  }
                  else if (thisTypeCompare == 1+sourceTypeCompare) {
                  }
                  else if (sourceTypeCompare == 1+thisTypeCompare) {
                  };
               };
            };
            if (source.sndArg().getApproxKind().isVariable()) {
               AssumeCondition(dynamic_cast<const VirtualExactToApproximateElement*>(&fstArg()) && dynamic_cast<const VirtualExactToApproximateElement*>(&source.sndArg()))
               if (((const VirtualExactToApproximateElement&) fstArg()).isEqual((const VirtualExactToApproximateElement&) source.sndArg())) {
                  MultiBitOperation::Type thisTypeCompare = getOperationType();
                  MultiBitOperation::Type sourceTypeCompare = ((const MultiBitOperation&) source.getOperation()).getType();
                  if (sourceTypeCompare < MultiBitOperation::TCompareEqual)
                     sourceTypeCompare = (MultiBitOperation::Type) (MultiBitOperation::EndOfCompare - (sourceTypeCompare - MultiBitOperation::StartOfCompare) - 1);
                  else if (sourceTypeCompare > MultiBitOperation::TCompareDifferent)
                     sourceTypeCompare = (MultiBitOperation::Type) (MultiBitOperation::EndOfCompare - (sourceTypeCompare - MultiBitOperation::StartOfCompare) - 1);
                  if (thisTypeCompare == sourceTypeCompare) {
                     bool result = sndArg().contain(source.fstArg(), env);
                     return result;
                  }
                  else if (thisTypeCompare == 1+sourceTypeCompare) {
                  }
                  else if (sourceTypeCompare == 1+thisTypeCompare) {
                  };
               };
            };
         };
         if (sndArg().getApproxKind().isVariable()) {
            if (source.fstArg().getApproxKind().isVariable()) {
               AssumeCondition(dynamic_cast<const VirtualExactToApproximateElement*>(&sndArg()) && dynamic_cast<const VirtualExactToApproximateElement*>(&source.fstArg()))
               if (((const VirtualExactToApproximateElement&) sndArg()).isEqual((const VirtualExactToApproximateElement&) source.fstArg())) {
                  MultiBitOperation::Type thisTypeCompare = getOperationType();
                  if (thisTypeCompare < MultiBitOperation::TCompareEqual)
                     thisTypeCompare = (MultiBitOperation::Type) (MultiBitOperation::EndOfCompare - (thisTypeCompare - MultiBitOperation::StartOfCompare) - 1);
                  else if (thisTypeCompare > MultiBitOperation::TCompareDifferent)
                     thisTypeCompare = (MultiBitOperation::Type) (MultiBitOperation::EndOfCompare - (thisTypeCompare - MultiBitOperation::StartOfCompare) - 1);
                  MultiBitOperation::Type sourceTypeCompare = ((const MultiBitOperation&) source.getOperation()).getType();
                  if (thisTypeCompare == sourceTypeCompare) {
                     bool result = fstArg().contain(source.sndArg(), env);
                     return result;
                  }
                  else if (thisTypeCompare == 1+sourceTypeCompare) {
                  }
                  else if (sourceTypeCompare == 1+thisTypeCompare) {
                  };
               };
            };
            if (source.sndArg().getApproxKind().isVariable()) {
               AssumeCondition(dynamic_cast<const VirtualExactToApproximateElement*>(&sndArg()) && dynamic_cast<const VirtualExactToApproximateElement*>(&source.sndArg()))
               if (((const VirtualExactToApproximateElement&) sndArg()).isEqual((const VirtualExactToApproximateElement&) source.sndArg())) {
                  MultiBitOperation::Type thisTypeCompare = getOperationType();
                  MultiBitOperation::Type sourceTypeCompare = ((const MultiBitOperation&) source.getOperation()).getType();
                  if (thisTypeCompare < MultiBitOperation::TCompareEqual)
                     thisTypeCompare = (MultiBitOperation::Type) (MultiBitOperation::EndOfCompare - (thisTypeCompare - MultiBitOperation::StartOfCompare) - 1);
                  else if (sourceTypeCompare > MultiBitOperation::TCompareDifferent)
                     thisTypeCompare = (MultiBitOperation::Type) (MultiBitOperation::EndOfCompare - (thisTypeCompare - MultiBitOperation::StartOfCompare) - 1);
                  if (sourceTypeCompare < MultiBitOperation::TCompareEqual)
                     sourceTypeCompare = (MultiBitOperation::Type) (MultiBitOperation::EndOfCompare - (sourceTypeCompare - MultiBitOperation::StartOfCompare) - 1);
                  else if (sourceTypeCompare > MultiBitOperation::TCompareDifferent)
                     thisTypeCompare = (MultiBitOperation::Type) (MultiBitOperation::EndOfCompare - (sourceTypeCompare - MultiBitOperation::StartOfCompare) - 1);
                  if (thisTypeCompare == sourceTypeCompare) {
                     bool result = fstArg().contain(source.fstArg(), env);
                     return result;
                  }
                  else if (thisTypeCompare == 1+sourceTypeCompare) {
                  }
                  else if (sourceTypeCompare == 1+thisTypeCompare) {
                  };
               };
            };
         };
      };
   };
   if (((const VirtualElement&) asource).getApproxKind().isTop())
      return true;
   env.mergeVerdictDegradate();
   env.setPartialApplied();
   return true;
}

template <class TypeBase>
bool
TCompareOperationElement<TypeBase>::intersectWith(const VirtualElement& asource, EvaluationEnvironment& aenv) {
   if (inherited::intersectWith(asource, aenv))
      return true;
   EvaluationEnvironment& env = (EvaluationEnvironment&) aenv;
   if (((const VirtualElement&) asource).getApproxKind().isFormalOperation()) {
      AssumeCondition(dynamic_cast<const typename inherited::FormalOperationElement*>(&asource))
      const typename inherited::FormalOperationElement& source = (const typename inherited::FormalOperationElement&) asource;
      MultiBitOperation::Type type = ((const MultiBitOperation&) source.getOperation()).getType();
      if (type >= MultiBitOperation::StartOfCompare && type < MultiBitOperation::EndOfCompare) {
         if (fstArg().getApproxKind().isVariable()) {
            if (source.fstArg().getApproxKind().isVariable()) {
               AssumeCondition(dynamic_cast<const VirtualExactToApproximateElement*>(&fstArg()) && dynamic_cast<const VirtualExactToApproximateElement*>(&source.fstArg()))
               if (((const VirtualExactToApproximateElement&) fstArg()).isEqual((const VirtualExactToApproximateElement&) source.fstArg())) {
                  MultiBitOperation::Type thisTypeCompare = getOperationType();
                  MultiBitOperation::Type sourceTypeCompare = ((const MultiBitOperation&) source.getOperation()).getType();
                  if (thisTypeCompare == sourceTypeCompare) {
                     inherited::assume(ssndArg()->intersectWith(source.sndArg(), env));
                     if (env.hasResult())
                        ssndArg() = env.presult();
                     return true;
                  }
                  else if (thisTypeCompare == 1+sourceTypeCompare) {
                  }
                  else if (sourceTypeCompare == 1+thisTypeCompare) {
                  };
               };
            };
            if (source.sndArg().getApproxKind().isVariable()) {
               AssumeCondition(dynamic_cast<const VirtualExactToApproximateElement*>(&fstArg()) && dynamic_cast<const VirtualExactToApproximateElement*>(&source.sndArg()))
               if (((const VirtualExactToApproximateElement&) fstArg()).isEqual((const VirtualExactToApproximateElement&) source.sndArg())) {
                  MultiBitOperation::Type thisTypeCompare = getOperationType();
                  MultiBitOperation::Type sourceTypeCompare = ((const MultiBitOperation&) source.getOperation()).getType();
                  if (sourceTypeCompare < MultiBitOperation::TCompareEqual)
                     sourceTypeCompare = (MultiBitOperation::Type) (MultiBitOperation::EndOfCompare - (sourceTypeCompare - MultiBitOperation::StartOfCompare) - 1);
                  else if (sourceTypeCompare > MultiBitOperation::TCompareDifferent)
                     sourceTypeCompare = (MultiBitOperation::Type) (MultiBitOperation::EndOfCompare - (sourceTypeCompare - MultiBitOperation::StartOfCompare) - 1);
                  if (thisTypeCompare == sourceTypeCompare) {
                     inherited::assume(ssndArg()->intersectWith(source.fstArg(), env));
                     if (env.hasResult())
                        ssndArg() = env.presult();
                     return true;
                  }
                  else if (thisTypeCompare == 1+sourceTypeCompare) {
                  }
                  else if (sourceTypeCompare == 1+thisTypeCompare) {
                  };
               };
            };
         };
         if (sndArg().getApproxKind().isVariable()) {
            if (source.fstArg().getApproxKind().isVariable()) {
               AssumeCondition(dynamic_cast<const VirtualExactToApproximateElement*>(&sndArg()) && dynamic_cast<const VirtualExactToApproximateElement*>(&source.fstArg()))
               if (((const VirtualExactToApproximateElement&) sndArg()).isEqual((const VirtualExactToApproximateElement&) source.fstArg())) {
                  MultiBitOperation::Type thisTypeCompare = getOperationType();
                  if (thisTypeCompare < MultiBitOperation::TCompareEqual)
                     thisTypeCompare = (MultiBitOperation::Type) (MultiBitOperation::EndOfCompare - (thisTypeCompare - MultiBitOperation::StartOfCompare) - 1);
                  else if (thisTypeCompare > MultiBitOperation::TCompareDifferent)
                     thisTypeCompare = (MultiBitOperation::Type) (MultiBitOperation::EndOfCompare - (thisTypeCompare - MultiBitOperation::StartOfCompare) - 1);
                  MultiBitOperation::Type sourceTypeCompare = ((const MultiBitOperation&) source.getOperation()).getType();
                  if (thisTypeCompare == sourceTypeCompare) {
                     inherited::assume(sfstArg()->intersectWith(source.sndArg(), env));
                     if (env.hasResult())
                        sfstArg() = env.presult();
                     return true;
                  }
                  else if (thisTypeCompare == 1+sourceTypeCompare) {
                  }
                  else if (sourceTypeCompare == 1+thisTypeCompare) {
                  };
               };
            };
            if (source.sndArg().getApproxKind().isVariable()) {
               AssumeCondition(dynamic_cast<const VirtualExactToApproximateElement*>(&sndArg()) && dynamic_cast<const VirtualExactToApproximateElement*>(&source.sndArg()))
               if (((const VirtualExactToApproximateElement&) sndArg()).isEqual((const VirtualExactToApproximateElement&) source.sndArg())) {
                  MultiBitOperation::Type thisTypeCompare = getOperationType();
                  MultiBitOperation::Type sourceTypeCompare = ((const MultiBitOperation&) source.getOperation()).getType();
                  if (thisTypeCompare < MultiBitOperation::TCompareEqual)
                     thisTypeCompare = (MultiBitOperation::Type) (MultiBitOperation::EndOfCompare - (thisTypeCompare - MultiBitOperation::StartOfCompare) - 1);
                  else if (sourceTypeCompare > MultiBitOperation::TCompareDifferent)
                     thisTypeCompare = (MultiBitOperation::Type) (MultiBitOperation::EndOfCompare - (thisTypeCompare - MultiBitOperation::StartOfCompare) - 1);
                  if (sourceTypeCompare < MultiBitOperation::TCompareEqual)
                     sourceTypeCompare = (MultiBitOperation::Type) (MultiBitOperation::EndOfCompare - (sourceTypeCompare - MultiBitOperation::StartOfCompare) - 1);
                  else if (sourceTypeCompare > MultiBitOperation::TCompareDifferent)
                     thisTypeCompare = (MultiBitOperation::Type) (MultiBitOperation::EndOfCompare - (sourceTypeCompare - MultiBitOperation::StartOfCompare) - 1);
                  if (thisTypeCompare == sourceTypeCompare) {
                     inherited::assume(sfstArg()->intersectWith(source.fstArg(), env));
                     if (env.hasResult())
                        sfstArg() = env.presult();
                     return true;
                  }
                  else if (thisTypeCompare == 1+sourceTypeCompare) {
                  }
                  else if (sourceTypeCompare == 1+thisTypeCompare) {
                  };
               };
            };
         };
      };
   };
   if (((const VirtualElement&) asource).getApproxKind().isTop())
      return true;
   env.mergeVerdictDegradate();
   PPVirtualElement multiBitTop = Approximate::Details::IntOperationElement::Methods::newTop(fstArg());
   env.presult() = convertArgumentTopToResult(multiBitTop);
   return true;
}

template <class TypeBase>
bool
TCompareOperationElement<TypeBase>::applyTop(const VirtualOperation& operation, EvaluationEnvironment& env) {
   PPVirtualElement multiBitTop = Approximate::Details::IntOperationElement::Methods::newTop(fstArg());
   env.presult() = convertArgumentTopToResult(multiBitTop);
   return true;
}

template <class TypeBase>
bool
TCompareOperationElement<TypeBase>::applyTrue(const VirtualOperation& operation, EvaluationEnvironment& env) {
   env.presult() = Approximate::Details::IntOperationElement::Methods::newTrue(fstArg());
   return true;
}

template <class TypeBase>
bool
TCompareOperationElement<TypeBase>::applyMultiBitTrue(const VirtualOperation& operation, EvaluationEnvironment& env) {
   env.presult() = Approximate::Details::IntOperationElement::Methods::newTrue(fstArg());
   env.presult() = VirtualElement::Methods::apply((VirtualElement&) *env.presult(),
         Bit::CastMultiBitOperation().setSize(1), env);
   return true;
}

template <class TypeBase>
bool
TCompareOperationElement<TypeBase>::applyFalse(const VirtualOperation& operation, EvaluationEnvironment& env) {
   env.presult() = Approximate::Details::IntOperationElement::Methods::newFalse(fstArg());
   return true;
}

template <class TypeBase>
bool
TCompareOperationElement<TypeBase>::applyMultiBitFalse(const VirtualOperation& operation, EvaluationEnvironment& env) {
   env.presult() = Approximate::Details::IntOperationElement::Methods::newFalse(fstArg());
   env.presult() = VirtualElement::Methods::apply((VirtualElement&) *env.presult(),
         Bit::CastMultiBitOperation().setSize(1), env);
   return true;
}

template <class TypeBase>
bool
TCompareOperationElement<TypeBase>::applyIdentity(const VirtualOperation& operation, EvaluationEnvironment& env) {
   return true;
}

template <class TypeBase>
bool
TCompareOperationElement<TypeBase>::applyCastIntegerSigned(const VirtualOperation& operation, EvaluationEnvironment& env) {
   AssumeUnimplemented
}

template <class TypeBase>
bool
TCompareOperationElement<TypeBase>::applyCastIntegerUnsigned(const VirtualOperation& operation, EvaluationEnvironment& env) {
   AssumeUnimplemented
}

template <class TypeBase>
bool
TCompareOperationElement<TypeBase>::applyNegateAssign(const VirtualOperation& operation, EvaluationEnvironment& env) {
   MultiBitOperation::Type compareType = ((const MultiBitOperation&) getOperation()).getType();
   switch (compareType) {
      case MultiBitOperation::TCompareLessSigned: compareType = MultiBitOperation::TCompareGreaterOrEqualSigned; break;
      case MultiBitOperation::TCompareLessOrEqualSigned: compareType = MultiBitOperation::TCompareGreaterSigned; break;
      case MultiBitOperation::TCompareLessUnsigned: compareType = MultiBitOperation::TCompareGreaterOrEqualUnsigned; break;
      case MultiBitOperation::TCompareLessOrEqualUnsigned: compareType = MultiBitOperation::TCompareGreaterUnsigned; break;
      case MultiBitOperation::TCompareLessFloat: compareType = MultiBitOperation::TCompareGreaterOrEqualFloat; break;
      case MultiBitOperation::TCompareLessOrEqualFloat: compareType = MultiBitOperation::TCompareGreaterFloat; break;
      case MultiBitOperation::TCompareEqual: compareType = MultiBitOperation::TCompareDifferent; break;
      case MultiBitOperation::TCompareEqualFloat: compareType = MultiBitOperation::TCompareDifferentFloat; break;
      case MultiBitOperation::TCompareDifferentFloat: compareType = MultiBitOperation::TCompareEqualFloat; break;
      case MultiBitOperation::TCompareDifferent: compareType = MultiBitOperation::TCompareEqual; break;
      case MultiBitOperation::TCompareGreaterOrEqualFloat: compareType = MultiBitOperation::TCompareLessFloat; break;
      case MultiBitOperation::TCompareGreaterFloat: compareType = MultiBitOperation::TCompareLessOrEqualFloat; break;
      case MultiBitOperation::TCompareGreaterOrEqualUnsigned: compareType = MultiBitOperation::TCompareLessUnsigned; break;
      case MultiBitOperation::TCompareGreaterUnsigned: compareType = MultiBitOperation::TCompareLessOrEqualUnsigned; break;
      case MultiBitOperation::TCompareGreaterOrEqualSigned: compareType = MultiBitOperation::TCompareLessSigned; break;
      case MultiBitOperation::TCompareGreaterSigned: compareType = MultiBitOperation::TCompareLessOrEqualSigned; break;
      default:
         {  AssumeUncalled }
   };
   const_cast<MultiBitOperation&>(((const MultiBitOperation&) inherited::getOperation())).setType(compareType);
   return true;
}

template <class TypeBase>
bool
TCompareOperationElement<TypeBase>::applyPlusAssign(const VirtualOperation& operation, EvaluationEnvironment& env) {
   const VirtualElement& source = env.getFirstArgument();
   VirtualElement::ZeroResult argument = source.queryZeroResult();
   if (argument.isZero())
      return true;
   if (argument.isDifferentZero())
      return applyNegateAssign(operation, env);
   env.mergeVerdictDegradate();
   return applyTop(operation, env);
}

template <class TypeBase>
bool
TCompareOperationElement<TypeBase>::applyMinMaxAssign(const VirtualOperation& operation, EvaluationEnvironment& env) {
   env.mergeVerdictDegradate();
   return applyTop(operation, env);
}

template <class TypeBase>
bool
TCompareOperationElement<TypeBase>::applyOrAssign(const VirtualOperation& operation, EvaluationEnvironment& env) {
   const VirtualElement& asource = env.getFirstArgument();
   VirtualElement::ZeroResult argument = asource.queryZeroResult();
   if (argument.isZero())
      return true;
   if (argument.isDifferentZero())
      return this->isBoolean() ? applyTrue(operation, env) : applyMultiBitTrue(operation, env);
   if (asource.getApproxKind().isFormalOperation()) {
      AssumeCondition(dynamic_cast<const TFormalOperationElement<TypeBase>*>(&asource))
      const TFormalOperationElement<TypeBase>& source = (const TFormalOperationElement<TypeBase>&) asource;
      MultiBit::Operation::Type sourceCompare = ((const MultiBit::Operation&) source.getOperation()).getType();
      MultiBit::Operation::Type thisCompare = ((const MultiBit::Operation&) getOperation()).getType();
      bool hasFound = false;
      bool isInverse = false;
      bool isFstFstEqual = inherited::Methods::applyBoolean(fstArg(), MultiBit::Operation().setCompareEqual(), source.fstArg()).isTrue();
      if (isFstFstEqual) {
         bool isSndSndEqual = inherited::Methods::applyBoolean(sndArg(), MultiBit::Operation().setCompareEqual(), source.sndArg()).isTrue();
         if (isSndSndEqual)
            hasFound = true;
      };
      if (!hasFound) {
         bool isFstSndEqual = inherited::Methods::applyBoolean(fstArg(), MultiBit::Operation().setCompareEqual(), source.sndArg()).isTrue();
         if (isFstSndEqual) {
            bool isSndFstEqual = inherited::Methods::applyBoolean(sndArg(), MultiBit::Operation().setCompareEqual(), source.fstArg()).isTrue();
            if (isSndFstEqual) {
               hasFound = true;
               isInverse = true;
            };
         };
      };
      if (hasFound) {
         switch (thisCompare) {
            case MultiBit::Operation::TCompareLessSigned:
               if (!isInverse ? (sourceCompare == MultiBit::Operation::TCompareLessSigned)
                              : (sourceCompare == MultiBit::Operation::TCompareGreaterSigned))
                  return true;
               if ((!isInverse ? (sourceCompare == MultiBit::Operation::TCompareLessOrEqualSigned)
                               : (sourceCompare == MultiBit::Operation::TCompareGreaterOrEqualSigned))
                     || sourceCompare == MultiBit::Operation::TCompareDifferent) {
                  env.setResult(source);
                  return true;
               };
               if (sourceCompare == MultiBit::Operation::TCompareEqual) {
                  inherited::ppvoOperation.setElement(MultiBit::Operation().setCompareLessOrEqualSigned());
                  return true;
               };
               if (!isInverse ? (sourceCompare == MultiBit::Operation::TCompareGreaterOrEqualSigned)
                              : (sourceCompare == MultiBit::Operation::TCompareLessOrEqualSigned))
                  return this->isBoolean() ? applyTrue(operation, env) : applyMultiBitTrue(operation, env);
               if (!isInverse ? (sourceCompare == MultiBit::Operation::TCompareGreaterSigned)
                              : (sourceCompare == MultiBit::Operation::TCompareLessSigned)) {
                  inherited::ppvoOperation.setElement(MultiBit::Operation().setCompareDifferent());
                  return true;
               };
               break;
            case MultiBit::Operation::TCompareLessOrEqualSigned:
               if (!isInverse) {
                  if (sourceCompare == MultiBit::Operation::TCompareLessSigned
                        || sourceCompare == MultiBit::Operation::TCompareLessOrEqualSigned
                        || sourceCompare == MultiBit::Operation::TCompareEqual)
                     return true;
                  if (sourceCompare == MultiBit::Operation::TCompareDifferent
                        || sourceCompare == MultiBit::Operation::TCompareGreaterOrEqualSigned
                        || sourceCompare == MultiBit::Operation::TCompareGreaterSigned)
                     return this->isBoolean() ? applyTrue(operation, env) : applyMultiBitTrue(operation, env);
               }
               else {
                  if (sourceCompare == MultiBit::Operation::TCompareGreaterSigned
                        || sourceCompare == MultiBit::Operation::TCompareGreaterOrEqualSigned
                        || sourceCompare == MultiBit::Operation::TCompareEqual)
                     return true;
                  if (sourceCompare == MultiBit::Operation::TCompareDifferent
                        || sourceCompare == MultiBit::Operation::TCompareLessOrEqualSigned
                        || sourceCompare == MultiBit::Operation::TCompareLessSigned)
                     return this->isBoolean() ? applyTrue(operation, env) : applyMultiBitTrue(operation, env);
               };
               break;
            case MultiBit::Operation::TCompareLessUnsigned:
               if (!isInverse ? (sourceCompare == MultiBit::Operation::TCompareLessUnsigned)
                              : (sourceCompare == MultiBit::Operation::TCompareGreaterUnsigned))
                  return true;
               if ((!isInverse ? (sourceCompare == MultiBit::Operation::TCompareLessOrEqualUnsigned)
                               : (sourceCompare == MultiBit::Operation::TCompareGreaterOrEqualUnsigned))
                     || sourceCompare == MultiBit::Operation::TCompareDifferent) {
                  env.setResult(source);
                  return true;
               };
               if (sourceCompare == MultiBit::Operation::TCompareEqual) {
                  inherited::ppvoOperation.setElement(MultiBit::Operation().setCompareLessOrEqualUnsigned());
                  return true;
               };
               if (!isInverse ? (sourceCompare == MultiBit::Operation::TCompareGreaterOrEqualUnsigned)
                              : (sourceCompare == MultiBit::Operation::TCompareLessOrEqualUnsigned))
                  return this->isBoolean() ? applyTrue(operation, env) : applyMultiBitTrue(operation, env);
               if (!isInverse ? (sourceCompare == MultiBit::Operation::TCompareGreaterUnsigned)
                              : (sourceCompare == MultiBit::Operation::TCompareLessUnsigned)) {
                  inherited::ppvoOperation.setElement(MultiBit::Operation().setCompareDifferent());
                  return true;
               };
               break;
            case MultiBit::Operation::TCompareLessOrEqualUnsigned:
               if (!isInverse) {
                  if (sourceCompare == MultiBit::Operation::TCompareLessUnsigned
                        || sourceCompare == MultiBit::Operation::TCompareLessOrEqualUnsigned
                        || sourceCompare == MultiBit::Operation::TCompareEqual)
                     return true;
                  if (sourceCompare == MultiBit::Operation::TCompareDifferent
                        || sourceCompare == MultiBit::Operation::TCompareGreaterOrEqualUnsigned
                        || sourceCompare == MultiBit::Operation::TCompareGreaterUnsigned)
                     return this->isBoolean() ? applyTrue(operation, env) : applyMultiBitTrue(operation, env);
               }
               else {
                  if (sourceCompare == MultiBit::Operation::TCompareGreaterUnsigned
                        || sourceCompare == MultiBit::Operation::TCompareGreaterOrEqualUnsigned
                        || sourceCompare == MultiBit::Operation::TCompareEqual)
                     return true;
                  if (sourceCompare == MultiBit::Operation::TCompareDifferent
                        || sourceCompare == MultiBit::Operation::TCompareLessOrEqualUnsigned
                        || sourceCompare == MultiBit::Operation::TCompareLessUnsigned)
                     return this->isBoolean() ? applyTrue(operation, env) : applyMultiBitTrue(operation, env);
               };
               break;
            case MultiBit::Operation::TCompareLessFloat:
               if (!isInverse ? (sourceCompare == MultiBit::Operation::TCompareLessFloat)
                              : (sourceCompare == MultiBit::Operation::TCompareGreaterFloat))
                  return true;
               if ((!isInverse ? (sourceCompare == MultiBit::Operation::TCompareLessOrEqualFloat)
                               : (sourceCompare == MultiBit::Operation::TCompareGreaterOrEqualFloat))
                     || sourceCompare == MultiBit::Operation::TCompareDifferentFloat) {
                  env.setResult(source);
                  return true;
               };
               if (sourceCompare == MultiBit::Operation::TCompareEqualFloat) {
                  inherited::ppvoOperation.setElement(MultiBit::CompareLessOrEqualFloatOperation()
                        .setPart((const MultiBit::CompareEqualFloatOperation&) source.getOperation()));
                  return true;
               };
               if (!isInverse ? (sourceCompare == MultiBit::Operation::TCompareGreaterOrEqualFloat)
                              : (sourceCompare == MultiBit::Operation::TCompareLessOrEqualFloat))
                  return this->isBoolean() ? applyTrue(operation, env) : applyMultiBitTrue(operation, env);
               if (!isInverse && (sourceCompare == MultiBit::Operation::TCompareGreaterFloat)) {
                  inherited::ppvoOperation.setElement(MultiBit::CompareDifferentFloatOperation()
                        .setPart((const MultiBit::CompareGreaterFloatOperation&) source));
                  return true;
               };
               if (isInverse && (sourceCompare == MultiBit::Operation::TCompareLessFloat)) {
                  inherited::ppvoOperation.setElement(MultiBit::CompareDifferentFloatOperation()
                        .setPart((const MultiBit::CompareLessFloatOperation&) source));
                  return true;
               };
               break;
            case MultiBit::Operation::TCompareLessOrEqualFloat:
               if (!isInverse) {
                  if (sourceCompare == MultiBit::Operation::TCompareLessFloat
                        || sourceCompare == MultiBit::Operation::TCompareLessOrEqualFloat
                        || sourceCompare == MultiBit::Operation::TCompareEqualFloat)
                     return true;
                  if (sourceCompare == MultiBit::Operation::TCompareDifferentFloat
                        || sourceCompare == MultiBit::Operation::TCompareGreaterOrEqualFloat
                        || sourceCompare == MultiBit::Operation::TCompareGreaterFloat)
                     return this->isBoolean() ? applyTrue(operation, env) : applyMultiBitTrue(operation, env);
               }
               else {
                  if (sourceCompare == MultiBit::Operation::TCompareGreaterFloat
                        || sourceCompare == MultiBit::Operation::TCompareGreaterOrEqualFloat
                        || sourceCompare == MultiBit::Operation::TCompareEqualFloat)
                     return true;
                  if (sourceCompare == MultiBit::Operation::TCompareDifferentFloat
                        || sourceCompare == MultiBit::Operation::TCompareLessOrEqualFloat
                        || sourceCompare == MultiBit::Operation::TCompareLessFloat)
                     return this->isBoolean() ? applyTrue(operation, env) : applyMultiBitTrue(operation, env);
               };
               break;
            case MultiBit::Operation::TCompareEqual:
               if (sourceCompare == MultiBit::Operation::TCompareEqual)
                  return true;
               if (sourceCompare == MultiBit::Operation::TCompareLessSigned
                     || sourceCompare == MultiBit::Operation::TCompareGreaterSigned
                     || sourceCompare == MultiBit::Operation::TCompareLessUnsigned
                     || sourceCompare == MultiBit::Operation::TCompareGreaterUnsigned) {
                  MultiBit::Operation newCompare;
                  if (sourceCompare == MultiBit::Operation::TCompareLessSigned
                        || sourceCompare == MultiBit::Operation::TCompareGreaterSigned) {
                     if (!isInverse ? (sourceCompare == MultiBit::Operation::TCompareLessSigned)
                                    : (sourceCompare == MultiBit::Operation::TCompareGreaterSigned))
                        newCompare.setCompareLessOrEqualSigned();
                     else
                        newCompare.setCompareGreaterOrEqualSigned();
                  }
                  else {
                     if (!isInverse ? (sourceCompare == MultiBit::Operation::TCompareLessUnsigned)
                                    : (sourceCompare == MultiBit::Operation::TCompareGreaterUnsigned))
                        newCompare.setCompareLessOrEqualUnsigned();
                     else
                        newCompare.setCompareGreaterOrEqualUnsigned();
                  };
                  inherited::ppvoOperation.setElement(newCompare);
                  return true;
               }
               if (sourceCompare == MultiBit::Operation::TCompareLessOrEqualSigned
                     || sourceCompare == MultiBit::Operation::TCompareGreaterOrEqualSigned
                     || sourceCompare == MultiBit::Operation::TCompareLessOrEqualUnsigned
                     || sourceCompare == MultiBit::Operation::TCompareGreaterOrEqualUnsigned) {
                  env.setResult(source);
                  return true;
               };
               if (sourceCompare == MultiBit::Operation::TCompareDifferent)
                  return this->isBoolean() ? applyTrue(operation, env) : applyMultiBitTrue(operation, env);
               break;
            case MultiBit::Operation::TCompareEqualFloat:
               if (sourceCompare == MultiBit::Operation::TCompareEqualFloat)
                  return true;
               if (sourceCompare == MultiBit::Operation::TCompareLessFloat
                     || sourceCompare == MultiBit::Operation::TCompareGreaterFloat) {
                  if (!isInverse && (sourceCompare == MultiBit::Operation::TCompareLessFloat))
                     inherited::ppvoOperation.setElement(MultiBit::CompareLessOrEqualFloatOperation()
                           .setPart((const MultiBit::CompareLessFloatOperation&) source.getOperation()));
                  else if (isInverse && (sourceCompare == MultiBit::Operation::TCompareGreaterFloat))
                     inherited::ppvoOperation.setElement(MultiBit::CompareLessOrEqualFloatOperation()
                           .setPart((const MultiBit::CompareGreaterFloatOperation&) source.getOperation()));
                  else if (!isInverse && (sourceCompare == MultiBit::Operation::TCompareGreaterFloat))
                     inherited::ppvoOperation.setElement(MultiBit::CompareGreaterOrEqualFloatOperation()
                           .setPart((const MultiBit::CompareLessFloatOperation&) source.getOperation()));
                  else if (isInverse && (sourceCompare == MultiBit::Operation::TCompareLessFloat))
                     inherited::ppvoOperation.setElement(MultiBit::CompareGreaterOrEqualFloatOperation()
                           .setPart((const MultiBit::CompareLessFloatOperation&) source.getOperation()));
                  return true;
               }
               if (sourceCompare == MultiBit::Operation::TCompareLessOrEqualFloat
                     || sourceCompare == MultiBit::Operation::TCompareGreaterOrEqualFloat) {
                  env.setResult(source);
                  return true;
               };
               if (sourceCompare == MultiBit::Operation::TCompareDifferentFloat)
                  return this->isBoolean() ? applyTrue(operation, env) : applyMultiBitTrue(operation, env);
               break;
            case MultiBit::Operation::TCompareDifferentFloat:
               if (sourceCompare == MultiBit::Operation::TCompareDifferentFloat
                     || sourceCompare == MultiBit::Operation::TCompareLessFloat
                     || sourceCompare == MultiBit::Operation::TCompareGreaterFloat)
                  return true;
               if (sourceCompare == MultiBit::Operation::TCompareEqualFloat
                     || sourceCompare == MultiBit::Operation::TCompareLessOrEqualFloat
                     || sourceCompare == MultiBit::Operation::TCompareGreaterOrEqualFloat)
                  return this->isBoolean() ? applyTrue(operation, env) : applyMultiBitTrue(operation, env);
               break;
            case MultiBit::Operation::TCompareDifferent:
               if (sourceCompare == MultiBit::Operation::TCompareDifferent
                     || sourceCompare == MultiBit::Operation::TCompareLessSigned
                     || sourceCompare == MultiBit::Operation::TCompareGreaterSigned
                     || sourceCompare == MultiBit::Operation::TCompareLessUnsigned
                     || sourceCompare == MultiBit::Operation::TCompareGreaterUnsigned)
                  return true;
               if (sourceCompare == MultiBit::Operation::TCompareEqual
                     || sourceCompare == MultiBit::Operation::TCompareLessOrEqualSigned
                     || sourceCompare == MultiBit::Operation::TCompareGreaterOrEqualSigned
                     || sourceCompare == MultiBit::Operation::TCompareLessOrEqualUnsigned
                     || sourceCompare == MultiBit::Operation::TCompareGreaterOrEqualUnsigned)
                  return this->isBoolean() ? applyTrue(operation, env) : applyMultiBitTrue(operation, env);
               break;
            case MultiBit::Operation::TCompareGreaterOrEqualFloat:
               if (!isInverse) {
                  if (sourceCompare == MultiBit::Operation::TCompareGreaterFloat
                        || sourceCompare == MultiBit::Operation::TCompareGreaterOrEqualFloat
                        || sourceCompare == MultiBit::Operation::TCompareEqualFloat)
                     return true;
                  if (sourceCompare == MultiBit::Operation::TCompareDifferentFloat
                        || sourceCompare == MultiBit::Operation::TCompareLessOrEqualFloat
                        || sourceCompare == MultiBit::Operation::TCompareLessFloat)
                     return this->isBoolean() ? applyTrue(operation, env) : applyMultiBitTrue(operation, env);
               }
               else {
                  if (sourceCompare == MultiBit::Operation::TCompareLessFloat
                        || sourceCompare == MultiBit::Operation::TCompareLessOrEqualFloat
                        || sourceCompare == MultiBit::Operation::TCompareEqualFloat)
                     return true;
                  if (sourceCompare == MultiBit::Operation::TCompareDifferentFloat
                        || sourceCompare == MultiBit::Operation::TCompareGreaterOrEqualFloat
                        || sourceCompare == MultiBit::Operation::TCompareGreaterFloat)
                     return this->isBoolean() ? applyTrue(operation, env) : applyMultiBitTrue(operation, env);
               };
               break;
            case MultiBit::Operation::TCompareGreaterFloat:
               if (!isInverse ? (sourceCompare == MultiBit::Operation::TCompareGreaterFloat)
                              : (sourceCompare == MultiBit::Operation::TCompareLessFloat))
                  return true;
               if ((!isInverse ? (sourceCompare == MultiBit::Operation::TCompareGreaterOrEqualFloat)
                               : (sourceCompare == MultiBit::Operation::TCompareLessOrEqualFloat))
                     || sourceCompare == MultiBit::Operation::TCompareDifferentFloat) {
                  env.setResult(source);
                  return true;
               };
               if (sourceCompare == MultiBit::Operation::TCompareEqualFloat) {
                  inherited::ppvoOperation.setElement(MultiBit::CompareGreaterOrEqualFloatOperation()
                        .setPart(((const MultiBit::CompareEqualFloatOperation&) source.getOperation())));
                  return true;
               };
               if (!isInverse ? (sourceCompare == MultiBit::Operation::TCompareLessOrEqualFloat)
                              : (sourceCompare == MultiBit::Operation::TCompareGreaterOrEqualFloat))
                  return this->isBoolean() ? applyTrue(operation, env) : applyMultiBitTrue(operation, env);
               if (!isInverse && (sourceCompare == MultiBit::Operation::TCompareLessFloat)) {
                  inherited::ppvoOperation.setElement(MultiBit::CompareDifferentFloatOperation()
                        .setPart(((const MultiBit::CompareLessFloatOperation&) source.getOperation())));
                  return true;
               };
               if (isInverse && (sourceCompare == MultiBit::Operation::TCompareGreaterFloat)) {
                  inherited::ppvoOperation.setElement(MultiBit::CompareDifferentFloatOperation()
                        .setPart(((const MultiBit::CompareGreaterFloatOperation&) source.getOperation())));
                  return true;
               };
               break;
            case MultiBit::Operation::TCompareGreaterOrEqualUnsigned:
               if (!isInverse) {
                  if (sourceCompare == MultiBit::Operation::TCompareGreaterUnsigned
                        || sourceCompare == MultiBit::Operation::TCompareGreaterOrEqualUnsigned
                        || sourceCompare == MultiBit::Operation::TCompareEqual)
                     return true;
                  if (sourceCompare == MultiBit::Operation::TCompareDifferent
                        || sourceCompare == MultiBit::Operation::TCompareLessOrEqualUnsigned
                        || sourceCompare == MultiBit::Operation::TCompareLessUnsigned)
                     return this->isBoolean() ? applyTrue(operation, env) : applyMultiBitTrue(operation, env);
               }
               else {
                  if (sourceCompare == MultiBit::Operation::TCompareLessUnsigned
                        || sourceCompare == MultiBit::Operation::TCompareLessOrEqualUnsigned
                        || sourceCompare == MultiBit::Operation::TCompareEqual)
                     return true;
                  if (sourceCompare == MultiBit::Operation::TCompareDifferent
                        || sourceCompare == MultiBit::Operation::TCompareGreaterOrEqualUnsigned
                        || sourceCompare == MultiBit::Operation::TCompareGreaterUnsigned)
                     return this->isBoolean() ? applyTrue(operation, env) : applyMultiBitTrue(operation, env);
               };
               break;
            case MultiBit::Operation::TCompareGreaterUnsigned:
               if (!isInverse ? (sourceCompare == MultiBit::Operation::TCompareGreaterUnsigned)
                              : (sourceCompare == MultiBit::Operation::TCompareLessUnsigned))
                  return true;
               if ((!isInverse ? (sourceCompare == MultiBit::Operation::TCompareGreaterOrEqualUnsigned)
                               : (sourceCompare == MultiBit::Operation::TCompareLessOrEqualUnsigned))
                     || sourceCompare == MultiBit::Operation::TCompareDifferent) {
                  env.setResult(source);
                  return true;
               };
               if (sourceCompare == MultiBit::Operation::TCompareEqual) {
                  inherited::ppvoOperation.setElement(MultiBit::Operation().setCompareGreaterOrEqualUnsigned());
                  return true;
               };
               if (!isInverse ? (sourceCompare == MultiBit::Operation::TCompareLessOrEqualUnsigned)
                              : (sourceCompare == MultiBit::Operation::TCompareGreaterOrEqualUnsigned))
                  return this->isBoolean() ? applyTrue(operation, env) : applyMultiBitTrue(operation, env);
               if (!isInverse ? (sourceCompare == MultiBit::Operation::TCompareLessUnsigned)
                              : (sourceCompare == MultiBit::Operation::TCompareGreaterUnsigned)) {
                  inherited::ppvoOperation.setElement(MultiBit::Operation().setCompareDifferent());
                  return true;
               };
               break;
            case MultiBit::Operation::TCompareGreaterOrEqualSigned:
               if (!isInverse) {
                  if (sourceCompare == MultiBit::Operation::TCompareGreaterSigned
                        || sourceCompare == MultiBit::Operation::TCompareGreaterOrEqualSigned
                        || sourceCompare == MultiBit::Operation::TCompareEqual)
                     return true;
                  if (sourceCompare == MultiBit::Operation::TCompareDifferent
                        || sourceCompare == MultiBit::Operation::TCompareLessOrEqualSigned
                        || sourceCompare == MultiBit::Operation::TCompareLessSigned)
                     return this->isBoolean() ? applyTrue(operation, env) : applyMultiBitTrue(operation, env);
               }
               else {
                  if (sourceCompare == MultiBit::Operation::TCompareLessSigned
                        || sourceCompare == MultiBit::Operation::TCompareLessOrEqualSigned
                        || sourceCompare == MultiBit::Operation::TCompareEqual)
                     return true;
                  if (sourceCompare == MultiBit::Operation::TCompareDifferent
                        || sourceCompare == MultiBit::Operation::TCompareGreaterOrEqualSigned
                        || sourceCompare == MultiBit::Operation::TCompareGreaterSigned)
                     return this->isBoolean() ? applyTrue(operation, env) : applyMultiBitTrue(operation, env);
               };
               break;
            case MultiBit::Operation::TCompareGreaterSigned:
               if (!isInverse ? (sourceCompare == MultiBit::Operation::TCompareGreaterSigned)
                              : (sourceCompare == MultiBit::Operation::TCompareLessSigned))
                  return true;
               if ((!isInverse ? (sourceCompare == MultiBit::Operation::TCompareGreaterOrEqualSigned)
                               : (sourceCompare == MultiBit::Operation::TCompareLessOrEqualSigned))
                     || sourceCompare == MultiBit::Operation::TCompareDifferent) {
                  env.setResult(source);
                  return true;
               };
               if (sourceCompare == MultiBit::Operation::TCompareEqual) {
                  inherited::ppvoOperation.setElement(MultiBit::Operation().setCompareGreaterOrEqualSigned());
                  return true;
               };
               if (!isInverse ? (sourceCompare == MultiBit::Operation::TCompareLessOrEqualSigned)
                              : (sourceCompare == MultiBit::Operation::TCompareGreaterOrEqualSigned))
                  return this->isBoolean() ? applyTrue(operation, env) : applyMultiBitTrue(operation, env);
               if (!isInverse ? (sourceCompare == MultiBit::Operation::TCompareLessSigned)
                              : (sourceCompare == MultiBit::Operation::TCompareGreaterSigned)) {
                  inherited::ppvoOperation.setElement(MultiBit::Operation().setCompareDifferent());
                  return true;
               };
               break;
            default:
               break;
         };
      };
   }
   env.mergeVerdictDegradate();
   return applyTop(operation, env);
}

template <class TypeBase>
bool
TCompareOperationElement<TypeBase>::applyAndAssign(const VirtualOperation& operation, EvaluationEnvironment& env) {
   const VirtualElement& asource = env.getFirstArgument();
   VirtualElement::ZeroResult argument = asource.queryZeroResult();
   if (argument.isZero())
      return this->isBoolean() ? applyFalse(operation, env) : applyMultiBitFalse(operation, env);
   if (argument.isDifferentZero())
      return true;
   if (asource.getApproxKind().isFormalOperation()) {
      AssumeCondition(dynamic_cast<const typename inherited::FormalOperationElement*>(&asource))
      const typename inherited::FormalOperationElement& source = (const typename inherited::FormalOperationElement&) asource;
      MultiBitOperation::Type type = ((const MultiBitOperation&) source.getOperation()).getType();
      if (type >= MultiBitOperation::StartOfCompare && type < MultiBitOperation::EndOfCompare) {
         if (fstArg().getApproxKind().isVariable()) {
            if (source.fstArg().getApproxKind().isVariable()) {
               AssumeCondition(dynamic_cast<const VirtualExactToApproximateElement*>(&fstArg()) && dynamic_cast<const VirtualExactToApproximateElement*>(&source.fstArg()))
               if (((const VirtualExactToApproximateElement&) fstArg()).isEqual((const VirtualExactToApproximateElement&) source.fstArg())) {
                  MultiBitOperation::Type thisTypeCompare = getOperationType();
                  MultiBitOperation::Type sourceTypeCompare = ((const MultiBitOperation&) source.getOperation()).getType();
                  if (thisTypeCompare == sourceTypeCompare) {
                     if (thisTypeCompare < MultiBitOperation::TCompareEqual) {
                        MultiBitOperation operation;
                        operation.setType(thisTypeCompare);
                        operation.setConst();
                        VirtualElement::Methods::BooleanResult lessOrEqualResult = VirtualElement::Methods::applyBoolean(sndArg(), operation, source.sndArg());
                        if (lessOrEqualResult.isTrue())
                           return true;
                        if (lessOrEqualResult.isFalse()) {
                           env.setResult(source);
                           return true;
                        };
                        MultiBitOperation::Type minType = (MultiBitOperation::Type) ((thisTypeCompare - MultiBitOperation::StartOfCompare) / 2 + MultiBitOperation::EndOfCompare);
                        MultiBitOperation minOperation;
                        minOperation.setType(minType);
                        ssndArg() = VirtualElement::Methods::applyAssign(ssndArg(), minOperation, source.sndArg(), VirtualElement::Methods::EvaluationParameters(env));
                        return true;
                     }
                     else if (thisTypeCompare > MultiBitOperation::TCompareDifferent) {
                        MultiBitOperation operation;
                        operation.setType(thisTypeCompare);
                        operation.setConst();
                        VirtualElement::Methods::BooleanResult greateOrEqualResult = VirtualElement::Methods::applyBoolean(sndArg(), operation, source.sndArg());
                        if (greateOrEqualResult.isTrue())
                           return true;
                        if (greateOrEqualResult.isFalse()) {
                           env.setResult(source);
                           return true;
                        };
                        MultiBitOperation::Type maxType = (MultiBitOperation::Type) (MultiBitOperation::EndOfAtomic - 1 - (MultiBitOperation::EndOfCompare - thisTypeCompare - 1) / 2);
                        MultiBitOperation maxOperation;
                        maxOperation.setType(maxType);
                        ssndArg() = VirtualElement::Methods::applyAssign(ssndArg(), maxOperation, source.sndArg(), VirtualElement::Methods::EvaluationParameters(env));
                        return true;
                     }
                     else if (thisTypeCompare == MultiBitOperation::TCompareEqual || thisTypeCompare == MultiBitOperation::TCompareEqualFloat) {
                        MultiBitOperation operation;
                        operation.setType(thisTypeCompare);
                        operation.setConst();
                        VirtualElement::Methods::BooleanResult equalResult = VirtualElement::Methods::applyBoolean(sndArg(), operation, source.sndArg());
                        if (equalResult.isTrue())
                           return true;
                        if (equalResult.isFalse())
                           return this->isBoolean() ? applyFalse(operation, env) : applyMultiBitFalse(operation, env);
                        return applyTop(operation, env);
                     }
                  }
                  else if (thisTypeCompare == MultiBitOperation::TCompareEqual || thisTypeCompare == MultiBitOperation::TCompareEqualFloat) {
                     MultiBitOperation operation;
                     operation.setType(sourceTypeCompare);
                     operation.setConst();
                     VirtualElement::Methods::BooleanResult lessOrEqualResult = VirtualElement::Methods::applyBoolean(sndArg(), operation, source.sndArg());
                     if (lessOrEqualResult.isTrue())
                        return true;
                     if (lessOrEqualResult.isFalse())
                        return this->isBoolean() ? applyFalse(operation, env) : applyMultiBitFalse(operation, env);
                  }
                  else if (sourceTypeCompare == MultiBitOperation::TCompareEqual) {
                     MultiBitOperation operation;
                     operation.setType(thisTypeCompare);
                     operation.setConst();
                     VirtualElement::Methods::BooleanResult lessOrEqualResult = VirtualElement::Methods::applyBoolean(source.sndArg(), operation, sndArg());
                     if (lessOrEqualResult.isTrue()) {
                        env.presult().setElement(source);
                        return true;
                     };
                     if (lessOrEqualResult.isFalse())
                        return this->isBoolean() ? applyFalse(operation, env) : applyMultiBitFalse(operation, env);
                  };
               };
            };
            if (source.sndArg().getApproxKind().isVariable()) {
               AssumeCondition(dynamic_cast<const VirtualExactToApproximateElement*>(&fstArg()) && dynamic_cast<const VirtualExactToApproximateElement*>(&source.sndArg()))
               if (((const VirtualExactToApproximateElement&) fstArg()).isEqual((const VirtualExactToApproximateElement&) source.sndArg())) {
                  MultiBitOperation::Type thisTypeCompare = getOperationType();
                  MultiBitOperation::Type sourceTypeCompare = ((const MultiBitOperation&) source.getOperation()).getType();
                  if (sourceTypeCompare < MultiBitOperation::TCompareEqual)
                     sourceTypeCompare = (MultiBitOperation::Type) (MultiBitOperation::EndOfCompare - (sourceTypeCompare - MultiBitOperation::StartOfCompare) - 1);
                  else if (sourceTypeCompare > MultiBitOperation::TCompareDifferent)
                     sourceTypeCompare = (MultiBitOperation::Type) (MultiBitOperation::EndOfCompare - (sourceTypeCompare - MultiBitOperation::StartOfCompare) - 1);
                  if (thisTypeCompare == sourceTypeCompare) {
                     if (thisTypeCompare < MultiBitOperation::TCompareEqual) {
                        MultiBitOperation operation;
                        operation.setType(thisTypeCompare);
                        operation.setConst();
                        VirtualElement::Methods::BooleanResult lessOrEqualResult = VirtualElement::Methods::applyBoolean(sndArg(), operation, source.fstArg());
                        if (lessOrEqualResult.isTrue())
                           return true;
                        if (lessOrEqualResult.isFalse()) {
                           env.setResult(source);
                           return true;
                        };
                        MultiBitOperation::Type minType = (MultiBitOperation::Type) ((thisTypeCompare - MultiBitOperation::StartOfCompare) / 2 + MultiBitOperation::EndOfCompare);
                        MultiBitOperation minOperation;
                        minOperation.setType(minType);
                        ssndArg() = VirtualElement::Methods::applyAssign(ssndArg(), minOperation, source.fstArg(), VirtualElement::Methods::EvaluationParameters(env));
                        return true;
                     }
                     else if (thisTypeCompare > MultiBitOperation::TCompareDifferent) {
                        MultiBitOperation operation;
                        operation.setType(thisTypeCompare);
                        operation.setConst();
                        VirtualElement::Methods::BooleanResult greateOrEqualResult = VirtualElement::Methods::applyBoolean(sndArg(), operation, source.fstArg());
                        if (greateOrEqualResult.isTrue())
                           return true;
                        if (greateOrEqualResult.isFalse()) {
                           env.setResult(source);
                           return true;
                        };
                        MultiBitOperation::Type maxType = (MultiBitOperation::Type) (MultiBitOperation::EndOfAtomic - 1 - (MultiBitOperation::EndOfCompare - thisTypeCompare - 1) / 2);
                        MultiBitOperation maxOperation;
                        maxOperation.setType(maxType);
                        ssndArg() = VirtualElement::Methods::applyAssign(ssndArg(), maxOperation, source.fstArg(), VirtualElement::Methods::EvaluationParameters(env));
                        return true;
                     }
                     else if (thisTypeCompare == MultiBitOperation::TCompareEqual || thisTypeCompare == MultiBitOperation::TCompareEqualFloat) {
                        MultiBitOperation operation;
                        operation.setType(thisTypeCompare);
                        operation.setConst();
                        VirtualElement::Methods::BooleanResult equalResult = VirtualElement::Methods::applyBoolean(sndArg(), operation, source.fstArg());
                        if (equalResult.isTrue())
                           return true;
                        if (equalResult.isFalse())
                           return this->isBoolean() ? applyFalse(operation, env) : applyMultiBitFalse(operation, env);
                        return applyTop(operation, env);
                     }
                  }
                  else if (thisTypeCompare == MultiBitOperation::TCompareEqual || thisTypeCompare == MultiBitOperation::TCompareEqualFloat) {
                     MultiBitOperation operation;
                     operation.setType(sourceTypeCompare);
                     operation.setConst();
                     VirtualElement::Methods::BooleanResult lessOrEqualResult = VirtualElement::Methods::applyBoolean(sndArg(), operation, source.fstArg());
                     if (lessOrEqualResult.isTrue())
                        return true;
                     if (lessOrEqualResult.isFalse())
                        return this->isBoolean() ? applyFalse(operation, env) : applyMultiBitFalse(operation, env);
                  }
                  else if (sourceTypeCompare == MultiBitOperation::TCompareEqual) {
                     MultiBitOperation operation;
                     operation.setType(thisTypeCompare);
                     operation.setConst();
                     VirtualElement::Methods::BooleanResult lessOrEqualResult = VirtualElement::Methods::applyBoolean(source.fstArg(), operation, sndArg());
                     if (lessOrEqualResult.isTrue()) {
                        env.presult().setElement(source);
                        return true;
                     };
                     if (lessOrEqualResult.isFalse())
                        return this->isBoolean() ? applyFalse(operation, env) : applyMultiBitFalse(operation, env);
                  };
               };
            };
         };
         if (sndArg().getApproxKind().isVariable()) {
            if (source.fstArg().getApproxKind().isVariable()) {
               AssumeCondition(dynamic_cast<const VirtualExactToApproximateElement*>(&sndArg()) && dynamic_cast<const VirtualExactToApproximateElement*>(&source.fstArg()))
               if (((const VirtualExactToApproximateElement&) sndArg()).isEqual((const VirtualExactToApproximateElement&) source.fstArg())) {
                  MultiBitOperation::Type thisTypeCompare = getOperationType();
                  if (thisTypeCompare < MultiBitOperation::TCompareEqual)
                     thisTypeCompare = (MultiBitOperation::Type) (MultiBitOperation::EndOfCompare - (thisTypeCompare - MultiBitOperation::StartOfCompare) - 1);
                  else if (thisTypeCompare > MultiBitOperation::TCompareDifferent)
                     thisTypeCompare = (MultiBitOperation::Type) (MultiBitOperation::EndOfCompare - (thisTypeCompare - MultiBitOperation::StartOfCompare) - 1);
                  MultiBitOperation::Type sourceTypeCompare = ((const MultiBitOperation&) source.getOperation()).getType();
                  if (thisTypeCompare == sourceTypeCompare) {
                     if (thisTypeCompare < MultiBitOperation::TCompareEqual) {
                        MultiBitOperation operation;
                        operation.setType(thisTypeCompare);
                        operation.setConst();
                        VirtualElement::Methods::BooleanResult lessOrEqualResult = VirtualElement::Methods::applyBoolean(fstArg(), operation, source.sndArg());
                        if (lessOrEqualResult.isTrue())
                           return true;
                        if (lessOrEqualResult.isFalse()) {
                           env.setResult(source);
                           return true;
                        };
                        MultiBitOperation::Type minType = (MultiBitOperation::Type) ((thisTypeCompare - MultiBitOperation::StartOfCompare) / 2 + MultiBitOperation::EndOfCompare);
                        MultiBitOperation minOperation;
                        minOperation.setType(minType);
                        sfstArg() = VirtualElement::Methods::applyAssign(sfstArg(), minOperation, source.sndArg(), VirtualElement::Methods::EvaluationParameters(env));
                        return true;
                     }
                     else if (thisTypeCompare > MultiBitOperation::TCompareDifferent) {
                        MultiBitOperation operation;
                        operation.setType(thisTypeCompare);
                        operation.setConst();
                        VirtualElement::Methods::BooleanResult greateOrEqualResult = VirtualElement::Methods::applyBoolean(fstArg(), operation, source.sndArg());
                        if (greateOrEqualResult.isTrue())
                           return true;
                        if (greateOrEqualResult.isFalse()) {
                           env.setResult(source);
                           return true;
                        };
                        MultiBitOperation::Type maxType = (MultiBitOperation::Type) (MultiBitOperation::EndOfAtomic - 1 - (MultiBitOperation::EndOfCompare - thisTypeCompare - 1) / 2);
                        MultiBitOperation maxOperation;
                        maxOperation.setType(maxType);
                        sfstArg() = VirtualElement::Methods::applyAssign(sfstArg(), maxOperation, source.sndArg(), VirtualElement::Methods::EvaluationParameters(env));
                        return true;
                     }
                     else if (thisTypeCompare == MultiBitOperation::TCompareEqual || thisTypeCompare == MultiBitOperation::TCompareEqualFloat) {
                        MultiBitOperation operation;
                        operation.setType(thisTypeCompare);
                        operation.setConst();
                        VirtualElement::Methods::BooleanResult equalResult = VirtualElement::Methods::applyBoolean(fstArg(), operation, source.sndArg());
                        if (equalResult.isTrue())
                           return true;
                        if (equalResult.isFalse())
                           return this->isBoolean() ? applyFalse(operation, env) : applyMultiBitFalse(operation, env);
                        return applyTop(operation, env);
                     }
                  }
                  else if (thisTypeCompare == MultiBitOperation::TCompareEqual || thisTypeCompare == MultiBitOperation::TCompareEqualFloat) {
                     MultiBitOperation operation;
                     operation.setType(sourceTypeCompare);
                     operation.setConst();
                     VirtualElement::Methods::BooleanResult lessOrEqualResult = VirtualElement::Methods::applyBoolean(fstArg(), operation, source.sndArg());
                     if (lessOrEqualResult.isTrue())
                        return true;
                     if (lessOrEqualResult.isFalse())
                        return this->isBoolean() ? applyFalse(operation, env) : applyMultiBitFalse(operation, env);
                  }
                  else if (sourceTypeCompare == MultiBitOperation::TCompareEqual) {
                     MultiBitOperation operation;
                     operation.setType(thisTypeCompare);
                     operation.setConst();
                     VirtualElement::Methods::BooleanResult lessOrEqualResult = VirtualElement::Methods::applyBoolean(source.sndArg(), operation, fstArg());
                     if (lessOrEqualResult.isTrue()) {
                        env.presult().setElement(source);
                        return true;
                     };
                     if (lessOrEqualResult.isFalse())
                        return this->isBoolean() ? applyFalse(operation, env) : applyMultiBitFalse(operation, env);
                  };
               };
            };
            if (source.sndArg().getApproxKind().isVariable()) {
               AssumeCondition(dynamic_cast<const VirtualExactToApproximateElement*>(&sndArg()) && dynamic_cast<const VirtualExactToApproximateElement*>(&source.sndArg()))
               if (((const VirtualExactToApproximateElement&) sndArg()).isEqual((const VirtualExactToApproximateElement&) source.sndArg())) {
                  MultiBitOperation::Type thisTypeCompare = getOperationType();
                  MultiBitOperation::Type sourceTypeCompare = ((const MultiBitOperation&) source.getOperation()).getType();
                  if (thisTypeCompare < MultiBitOperation::TCompareEqual)
                     thisTypeCompare = (MultiBitOperation::Type) (MultiBitOperation::EndOfCompare - (thisTypeCompare - MultiBitOperation::StartOfCompare) - 1);
                  else if (sourceTypeCompare > MultiBitOperation::TCompareDifferent)
                     thisTypeCompare = (MultiBitOperation::Type) (MultiBitOperation::EndOfCompare - (thisTypeCompare - MultiBitOperation::StartOfCompare) - 1);
                  if (sourceTypeCompare < MultiBitOperation::TCompareEqual)
                     sourceTypeCompare = (MultiBitOperation::Type) (MultiBitOperation::EndOfCompare - (sourceTypeCompare - MultiBitOperation::StartOfCompare) - 1);
                  else if (sourceTypeCompare > MultiBitOperation::TCompareDifferent)
                     thisTypeCompare = (MultiBitOperation::Type) (MultiBitOperation::EndOfCompare - (sourceTypeCompare - MultiBitOperation::StartOfCompare) - 1);
                  if (thisTypeCompare == sourceTypeCompare) {
                     if (thisTypeCompare < MultiBitOperation::TCompareEqual) {
                        MultiBitOperation operation;
                        operation.setType(thisTypeCompare);
                        operation.setConst();
                        VirtualElement::Methods::BooleanResult lessOrEqualResult = VirtualElement::Methods::applyBoolean(fstArg(), operation, source.fstArg());
                        if (lessOrEqualResult.isTrue())
                           return true;
                        if (lessOrEqualResult.isFalse()) {
                           env.setResult(source);
                           return true;
                        };
                        MultiBitOperation::Type minType = (MultiBitOperation::Type) ((thisTypeCompare - MultiBitOperation::StartOfCompare) / 2 + MultiBitOperation::EndOfCompare);
                        MultiBitOperation minOperation;
                        minOperation.setType(minType);
                        sfstArg() = VirtualElement::Methods::applyAssign(sfstArg(), minOperation, source.fstArg(), VirtualElement::Methods::EvaluationParameters(env));
                        return true;
                     }
                     else if (thisTypeCompare > MultiBitOperation::TCompareDifferent) {
                        MultiBitOperation operation;
                        operation.setType(thisTypeCompare);
                        operation.setConst();
                        VirtualElement::Methods::BooleanResult greateOrEqualResult = VirtualElement::Methods::applyBoolean(fstArg(), operation, source.fstArg());
                        if (greateOrEqualResult.isTrue())
                           return true;
                        if (greateOrEqualResult.isFalse()) {
                           env.setResult(source);
                           return true;
                        };
                        MultiBitOperation::Type maxType = (MultiBitOperation::Type) (MultiBitOperation::EndOfAtomic - 1 - (MultiBitOperation::EndOfCompare - thisTypeCompare - 1) / 2);
                        MultiBitOperation maxOperation;
                        maxOperation.setType(maxType);
                        sfstArg() = VirtualElement::Methods::applyAssign(sfstArg(), maxOperation, source.fstArg(), VirtualElement::Methods::EvaluationParameters(env));
                        return true;
                     }
                     else if (thisTypeCompare == MultiBitOperation::TCompareEqual || thisTypeCompare == MultiBitOperation::TCompareEqualFloat) {
                        MultiBitOperation operation;
                        operation.setType(thisTypeCompare);
                        operation.setConst();
                        VirtualElement::Methods::BooleanResult equalResult = VirtualElement::Methods::applyBoolean(fstArg(), operation, source.fstArg());
                        if (equalResult.isTrue())
                           return true;
                        if (equalResult.isFalse())
                           return this->isBoolean() ? applyFalse(operation, env) : applyMultiBitFalse(operation, env);
                        return applyTop(operation, env);
                     }
                  }
                  else if (thisTypeCompare == MultiBitOperation::TCompareEqual || thisTypeCompare == MultiBitOperation::TCompareEqualFloat) {
                     MultiBitOperation operation;
                     operation.setType(sourceTypeCompare);
                     operation.setConst();
                     VirtualElement::Methods::BooleanResult lessOrEqualResult = VirtualElement::Methods::applyBoolean(fstArg(), operation, source.fstArg());
                     if (lessOrEqualResult.isTrue())
                        return true;
                     if (lessOrEqualResult.isFalse())
                        return this->isBoolean() ? applyFalse(operation, env) : applyMultiBitFalse(operation, env);
                  }
                  else if (sourceTypeCompare == MultiBitOperation::TCompareEqual) {
                     MultiBitOperation operation;
                     operation.setType(thisTypeCompare);
                     operation.setConst();
                     VirtualElement::Methods::BooleanResult lessOrEqualResult = VirtualElement::Methods::applyBoolean(source.fstArg(), operation, fstArg());
                     if (lessOrEqualResult.isTrue()) {
                        env.presult().setElement(source);
                        return true;
                     };
                     if (lessOrEqualResult.isFalse())
                        return this->isBoolean() ? applyFalse(operation, env) : applyMultiBitFalse(operation, env);
                  };
               };
            };
         };
      };
   };
   env.mergeVerdictDegradate();
   return applyTop(operation, env);
}

template <class TypeBase>
bool
TCompareOperationElement<TypeBase>::applyToCommutativeAssign(const VirtualOperation& operation,
      VirtualElement& thisElement, EvaluationEnvironment& aenv) const {
   TransmitEnvironment env(aenv);
   env.setFirstArgument(thisElement);
   PNT::AutoPointer<VirtualOperation> equalOperation(operation);
   if (!equalOperation->isConstWithAssign())
      equalOperation->setConstWithAssign();
   if (!const_cast<thisType&>(*this).apply(*equalOperation, env))
      return false;
   aenv.presult() = env.presult();
   return true;
}

template <class TypeBase>
bool
TCompareOperationElement<TypeBase>::applyToCommutative(const VirtualOperation& operation, VirtualElement& thisElement, EvaluationEnvironment& env) const {
   TransmitEnvironment applyEnv(env);
   applyEnv.setFirstArgument(thisElement);
   if (!const_cast<thisType&>(*this).apply(operation, applyEnv))
      return false;
   env.presult() = applyEnv.presult();
   return true;
}

template <class TypeBase>
bool
TCompareOperationElement<TypeBase>::queryCompareSpecial(const VirtualElement& source, const VirtualQueryOperation& operation, VirtualQueryOperation::Environment& env) {
   ((Approximate::Details::IntOperationElement::QueryOperation::CompareSpecialEnvironment&) env).fill();
   return true;
}

} // end of namespace Details
} // end of namespace Approximate

namespace Bit { namespace Approximate {

#define DefineInitApply(TypeOperation)                                                             \
   elementAt(Operation::T##TypeOperation).setMethod(&CompareOperationElement::apply##TypeOperation);

#define DefineInitOApply(TypeOperation, TypeRedirection)                                           \
   elementAt(Operation::T##TypeOperation).setMethod(&CompareOperationElement::apply##TypeRedirection);

#define DefineInitBiApply(TypeOperation)                                                           \
   elementAt(Operation::T##TypeOperation).setArray(new ConstantAndGenericApplyMethod(              \
      &CompareOperationElement::apply##TypeOperation##Constant,                                    \
      &CompareOperationElement::apply##TypeOperation));

#define DefineInitOBiApply(TypeOperation, TypeMethod)                                              \
   elementAt(Operation::T##TypeOperation).setArray(new ConstantAndGenericApplyMethod(              \
      &CompareOperationElement::apply##TypeMethod##Constant,                                       \
      &CompareOperationElement::apply##TypeMethod));

CompareOperationElement::MethodApplyTable::MethodApplyTable() {
#include "StandardClasses/UndefineNew.h"
   clear();

   if ((char) 0xFF < 0)
      DefineInitOApply(CastChar, CastIntegerSigned)
   else
      DefineInitOApply(CastChar, CastIntegerUnsigned)
   DefineInitOApply(CastInt, CastIntegerSigned)
   DefineInitOApply(CastUnsignedInt, CastIntegerUnsigned)
   DefineInitApply(CastMultiBit)
   
   DefineInitOApply(PrevAssign, NegateAssign)
   DefineInitOApply(NextAssign, NegateAssign)
   DefineInitOApply(PlusAssign, PlusAssign)
   DefineInitOApply(MinusAssign, PlusAssign)

   DefineInitOApply(CompareLess, CompareLessOrGreater)
   DefineInitOApply(CompareLessOrEqual, CompareLessOrGreaterOrEqual)
   DefineInitApply(CompareEqual)
   DefineInitApply(CompareDifferent)
   DefineInitOApply(CompareGreaterOrEqual, CompareLessOrGreaterOrEqual)
   DefineInitOApply(CompareGreater, CompareLessOrGreater)
   DefineInitOApply(MinAssign, MinMaxAssign)
   DefineInitOApply(MaxAssign, MinMaxAssign)

   DefineInitApply(OrAssign)
   DefineInitApply(AndAssign)
   DefineInitApply(ExclusiveOrAssign)
   DefineInitApply(NegateAssign)
#include "StandardClasses/DefineNew.h"
}

#undef DefineInitApply
#undef DefineInitOApply
#undef DefineInitBiApply
#undef DefineInitOBiApply

#define DefineInitApplyTo(TypeOperation)                                                        \
   elementAt(Operation::T##TypeOperation).setMethod(&CompareOperationElement::applyTo##TypeOperation);
#define DefineInitOApplyTo(TypeOperation, TypeMethod)                                                        \
   elementAt(Operation::T##TypeOperation).setMethod(&CompareOperationElement::applyTo##TypeMethod);

CompareOperationElement::MethodApplyToTable::MethodApplyToTable() {
#include "StandardClasses/UndefineNew.h"
   clear();
   DefineInitOApplyTo(PlusAssign, CommutativeAssign)
   DefineInitOApplyTo(MinusAssign, CommutativeAssign)

   DefineInitOApplyTo(CompareLess, CompareLessOrGreater)
   DefineInitOApplyTo(CompareLessOrEqual, CompareLessOrGreaterOrEqual)
   DefineInitOApplyTo(CompareEqual, Commutative)
   DefineInitOApplyTo(CompareDifferent, Commutative)
   DefineInitOApplyTo(CompareGreaterOrEqual, CompareLessOrGreaterOrEqual)
   DefineInitOApplyTo(CompareGreater, CompareLessOrGreater)
   DefineInitOApplyTo(MinAssign, CommutativeAssign)
   DefineInitOApplyTo(MaxAssign, CommutativeAssign)

   DefineInitOApplyTo(OrAssign, CommutativeAssign)
   DefineInitOApplyTo(AndAssign, CommutativeAssign)
   DefineInitOApplyTo(ExclusiveOrAssign, CommutativeAssign)
#include "StandardClasses/DefineNew.h"
}

#undef DefineInitOApplyTo
#undef DefineInitApplyTo

#define DefineInitUnaryConstraint(TypeOperation) \
   mcutUnaryTable[Operation::T##TypeOperation].setMethod(&CompareOperationElement::constraint##TypeOperation);

#define DefineInitUnaryOConstraint(TypeOperation, TypeMethod) \
   mcutUnaryTable[Operation::T##TypeOperation].setMethod(&CompareOperationElement::constraint##TypeMethod);

#define DefineInitBinaryConstraint(TypeOperation) \
   mcbtBinaryTable[Operation::T##TypeOperation].setMethod(&CompareOperationElement::constraint##TypeOperation);

CompareOperationElement::MethodConstraintTable::MethodConstraintTable() {
#include "StandardClasses/UndefineNew.h"
// DefineInitUnaryOGIntConstraint(CastChar, CastInteger)
// DefineInitUnaryOGIntConstraint(CastInt, CastInteger)
// DefineInitUnaryOGIntConstraint(CastUnsignedInt, CastInteger)
   DefineInitUnaryConstraint(CastMultiBit)

   DefineInitUnaryOConstraint(PrevAssign, NegateAssign)
   DefineInitUnaryOConstraint(NextAssign, NegateAssign)

   DefineInitBinaryConstraint(PlusAssign)
   DefineInitBinaryConstraint(MinusAssign)

   DefineInitBinaryConstraint(OrAssign)
   DefineInitBinaryConstraint(AndAssign)
   DefineInitBinaryConstraint(ExclusiveOrAssign)
   DefineInitUnaryConstraint(NegateAssign)
#include "StandardClasses/DefineNew.h"
}

#undef DefineInitUnaryConstraint
#undef DefineInitUnaryOConstraint
#undef DefineInitBinaryConstraint

CompareOperationElement::MethodApplyTable CompareOperationElement::matMethodApplyTable;
CompareOperationElement::MethodApplyToTable CompareOperationElement::mattMethodApplyToTable;
CompareOperationElement::MethodConstraintTable CompareOperationElement::mctMethodConstraintTable;

#define DefineInitQuery(TypeOperation) \
   elementAt(QueryOperation::TypeOperation).setMethod(&CompareOperationElement::query##TypeOperation);

CompareOperationElement::QueryTable::QueryTable() {
#include "StandardClasses/UndefineNew.h"
   FunctionQueryTable* table = &elementAt(VirtualQueryOperation::TCompareSpecial);
   table->setSize(2);
   // (*table)[0].setMethod(&ConstantElement::queryGuardAll);
   (*table)[1].setMethod(&CompareOperationElement::queryCompareSpecial);
  
   table = &elementAt(VirtualQueryOperation::TSimplification);
   table->setSize(1);
   (*table)[0].setMethod(&CompareOperationElement::querySimplification);
  
   table = &elementAt(VirtualQueryOperation::TDomain);
   table->setSize(QueryOperation::EndOfTypeDomain);

#define DefineInitQueryMethodTable(TypeOperation) \
   (*table)[QueryOperation::TD##TypeOperation].setMethod(&CompareOperationElement::queryDispatchOnTopResult);
   
   DefineInitQueryMethodTable(Disjunction)
   DefineInitQueryMethodTable(MultiBitConstant)
   DefineInitQueryMethodTable(IntConstant)
   DefineInitQueryMethodTable(Top)
   DefineInitQueryMethodTable(False)
   DefineInitQueryMethodTable(True)

#undef DefineInitQueryMethodTable

   table = &elementAt(VirtualQueryOperation::TDuplication);
   table->setSize(3);
   (*table)[QueryOperation::TCDClone].resetMethod(&CompareOperationElement::queryCopy);
   (*table)[QueryOperation::TCDSpecialize].resetMethod(&CompareOperationElement::querySpecialize);

   table = &elementAt(VirtualQueryOperation::TExtern);
   table->setSize(VirtualQueryOperation::EndOfTypeExtern);
   // (*table)[VirtualQueryOperation::TENativeOperation].setMethod(&CompareOperationElement::queryBitNativeOperation);
#include "StandardClasses/DefineNew.h"
}
#undef DefineInitQuery

CompareOperationElement::QueryTable CompareOperationElement::mqtMethodQueryTable;

#define DefineTypeOperation Bit
#define DefineTypeObject CompareOperationElement
#include "Analyzer/Scalar/Approximate/VirtualCall.incc"
#undef DefineTypeObject
#undef DefineTypeOperation

bool
CompareOperationElement::intersectWith(const VirtualElement& asource, EvaluationEnvironment& env) {
   if (!env.isTopLevel()) {
      if (inherited::intersectWith(asource, env))
         return true;
   };
   ConstraintEnvironment constraintEnvironment = ConstraintEnvironment(EvaluationParameters(env));
   if (constraintEnvironment.getLatticeCreation().isFormal())
      constraintEnvironment.setTopLatticeCreationAuthorizeFormal();
   constraintEnvironment.absorbFirstArgument(sndArg().createSCopy());
   if (!fstArg().constraint(getOperation(), asource, constraintEnvironment)) return false;
   if (constraintEnvironment.hasResult())
      sfstArg() = constraintEnvironment.presult();
   ssndArg().absorbElement((VirtualElement*) constraintEnvironment.extractFirstArgument());
   inherited::intersectWith(asource, env);
   return true;
}

bool
CompareOperationElement::applyCastMultiBit(const VirtualOperation& aoperation, EvaluationEnvironment& env) {
   const CastMultiBitOperation& operation = (const CastMultiBitOperation&) aoperation;
   AssumeCondition(operation.getResultSize() >= 1)
   typedef MultiBit::Approximate::CompareOperationElement CompareOperationElement;
   auto* result = new CompareOperationElement((const CompareOperationElement::Init&)
         CompareOperationElement::Init().set1(), ((const MultiBitOperation&) getOperation()).getType());
   env.presult().absorbElement(result);
   result->sfstArg().setElement(fstArg());
   result->ssndArg().setElement(sndArg());
   if (operation.getResultSize() > 1) {
      PPVirtualElement compare = env.presult();
      MultiBit::Approximate::ExtendOperationElement* result;
      {  MultiBit::Approximate::ExtendOperationElement::Init init;
         init.setBitSize(operation.getResultSize());
         init.setUnsignedExtension(operation.getResultSize()-1);
         result = new MultiBit::Approximate::ExtendOperationElement(init);
      };
      env.presult().absorbElement(result);
      result->sfstArg() = compare;
   }
   return true;
}

bool
CompareOperationElement::applyCompareLessOrGreater(const VirtualOperation& operation, EvaluationEnvironment& env) {
   const VirtualElement& source = env.getFirstArgument();
   ZeroResult argument = source.queryZeroResult();
   bool isLess = ((const Operation&) operation).getType() < Operation::TCompareEqual;
   if (isLess ? argument.isZero() : argument.isDifferentZero())
      return applyFalse(operation, env);
   if (isLess ? argument.isDifferentZero() : argument.isZero()) {
      if (isLess) {
         PNT::TPassPointer<CompareOperationElement, PPAbstractElement> copy(*this);
         bool result = copy->applyNegateAssign(operation, env);
         if (result && !env.hasResult())
            env.presult() = copy;
         return result;
      }
      else {
         env.setResult(*this);
         return true;
      };
   };
   env.mergeVerdictDegradate();
   return applyTop(operation, env);
}

bool
CompareOperationElement::applyCompareLessOrGreaterOrEqual(const VirtualOperation& operation, EvaluationEnvironment& env) {
   const VirtualElement& asource = env.getFirstArgument();
   ZeroResult argument = asource.queryZeroResult();
   bool isLess = ((const Operation&) operation).getType() < Operation::TCompareEqual;
   if (isLess ? argument.isZero() : argument.isDifferentZero()) {
      if (isLess) {
         PNT::TPassPointer<CompareOperationElement, PPAbstractElement> copy(*this);
         bool result = copy->applyNegateAssign(operation, env);
         if (result && !env.hasResult())
            env.presult() = copy;
         return result;
      }
      else {
         env.setResult(*this);
         return true;
      };
   };
      
   if (isLess ? argument.isDifferentZero() : argument.isZero())
      return applyTrue(operation, env);
   if (asource.getApproxKind().isFormalOperation()) {
      AssumeCondition(dynamic_cast<const FormalOperationElement*>(&asource))
      // const FormalOperationElement& source = (const FormalOperationElement&) asource;
      // MultiBitOperation::Type type = ((const MultiBitOperation&) source.getOperation()).getType();
/*    if (type >= MultiBitOperation::StartOfCompare && type < MultiBitOperation::EndOfCompare) {
         if (fstArg().getApproxKind().isVariable()) {
            if (source.fstArg().getApproxKind().isVariable()) {
               AssumeCondition(dynamic_cast<const VirtualExactToApproximateElement*>(&fstArg()) && dynamic_cast<const VirtualExactToApproximateElement*>(&source.fstArg()))
               if (((const VirtualExactToApproximateElement&) fstArg()).isEqual((const VirtualExactToApproximateElement&) source.fstArg())) {

               };
            };
            if (source.sndArg().getApproxKind().isVariable()) {
               AssumeCondition(dynamic_cast<const VirtualExactToApproximateElement*>(&fstArg()) && dynamic_cast<const VirtualExactToApproximateElement*>(&source.sndArg()))
               if (((const VirtualExactToApproximateElement&) fstArg()).isEqual((const VirtualExactToApproximateElement&) source.sndArg())) {

               };
            };
         };
         if (sndArg().getApproxKind().isVariable()) {
            if (source.fstArg().getApproxKind().isVariable()) {
               AssumeCondition(dynamic_cast<const VirtualExactToApproximateElement*>(&sndArg()) && dynamic_cast<const VirtualExactToApproximateElement*>(&source.fstArg()))
               if (((const VirtualExactToApproximateElement&) sndArg()).isEqual((const VirtualExactToApproximateElement&) source.fstArg())) {

               };
            };
            if (source.sndArg().getApproxKind().isVariable()) {
               AssumeCondition(dynamic_cast<const VirtualExactToApproximateElement*>(&sndArg()) && dynamic_cast<const VirtualExactToApproximateElement*>(&source.sndArg()))
               if (((const VirtualExactToApproximateElement&) sndArg()).isEqual((const VirtualExactToApproximateElement&) source.sndArg())) {

               };
            };
         };
      }; */
   };
   env.mergeVerdictDegradate();
   return applyTop(operation, env);
}

bool
CompareOperationElement::applyCompareEqual(const VirtualOperation& operation, EvaluationEnvironment& env) {
   const VirtualElement& source = env.getFirstArgument();
   ZeroResult argument = source.queryZeroResult();
   if (argument.isZero()) {
      PNT::TPassPointer<CompareOperationElement, PPAbstractElement> copy(*this);
      bool result = copy->applyNegateAssign(operation, env);
      if (result && !env.hasResult())
         env.presult() = copy;
      return result;
   };
   if (argument.isDifferentZero()) {
      env.setResult(*this);
      return true;
   };      
   env.mergeVerdictDegradate();
   return applyTop(operation, env);
}

bool
CompareOperationElement::applyCompareDifferent(const VirtualOperation& operation, EvaluationEnvironment& env) {
   const VirtualElement& source = env.getFirstArgument();
   ZeroResult argument = source.queryZeroResult();
   if (argument.isZero()) {
      env.setResult(*this);
      return true;
   };      
   if (argument.isDifferentZero()) {
      PNT::TPassPointer<CompareOperationElement, PPAbstractElement> copy(*this);
      bool result = copy->applyNegateAssign(operation, env);
      if (result && !env.hasResult())
         env.presult() = copy;
      return result;
   };
   env.mergeVerdictDegradate();
   return applyTop(operation, env);
}

bool
CompareOperationElement::applyExclusiveOrAssign(const VirtualOperation& operation, EvaluationEnvironment& env) {
   bool result = applyNegateAssign(operation, env);
   if (!result)
      return false;
   if (env.hasResult()) {
      PPVirtualElement negate = env.presult();
      Operation equalOperation;
      result = negate->apply(equalOperation.setCompareEqual(), env);
   }
   else
      result = applyCompareEqual(operation, env);
   return result;
}

bool
CompareOperationElement::applyToMinusAssign(const VirtualOperation& operation, VirtualElement& source, EvaluationEnvironment& env) const {
   PNT::TPassPointer<CompareOperationElement, PPAbstractElement> thisCopy(*this);
   TransmitEnvironment applyEnv(env);
   bool result = thisCopy->applyNegateAssign(operation, applyEnv);
   if (!result)
      return false;
   if (applyEnv.hasResult())
      thisCopy = applyEnv.presult();
   applyEnv.merge().clear();
   applyEnv.setFirstArgument(*thisCopy);
   assume(source.apply(Operation().setPlusAssign(), applyEnv));
   if (applyEnv.hasResult())
      env.presult() = applyEnv.presult();
   return true;
}

bool
CompareOperationElement::applyToCompareLessOrGreater(const VirtualOperation& aoperation, VirtualElement& thisElement, EvaluationEnvironment& env) const {
   TransmitEnvironment applyEnv(env);
   applyEnv.setFirstArgument(thisElement);
   Operation operation;
   const Operation& originalOperation = (const Operation&) aoperation;
   switch (originalOperation.getType()) {
      case Operation::TCompareLess:
         operation.setCompareGreaterOrEqual();
         break;
      case Operation::TCompareGreater:
         operation.setCompareLessOrEqual();
         break;
      default:
         {  AssumeUncalled }
   };
   return const_cast<CompareOperationElement&>(*this).applyCompareLessOrGreaterOrEqual(operation, applyEnv);
}

bool
CompareOperationElement::applyToCompareLessOrGreaterOrEqual(const VirtualOperation& aoperation, VirtualElement& thisElement, EvaluationEnvironment& env) const {
   TransmitEnvironment applyEnv(env);
   applyEnv.setFirstArgument(thisElement);
   Operation operation;
   const Operation& originalOperation = (const Operation&) aoperation;
   switch (originalOperation.getType()) {
      case Operation::TCompareLessOrEqual:
         operation.setCompareGreater();
         break;
      case Operation::TCompareGreaterOrEqual:
         operation.setCompareLess();
         break;
      default:
         {  AssumeUncalled }
   };
   return const_cast<CompareOperationElement&>(*this).applyCompareLessOrGreaterOrEqual(operation, applyEnv);
}

bool
CompareOperationElement::querySimplification(const VirtualElement& thisElement, const VirtualQueryOperation& qoOperation, VirtualQueryOperation::Environment& aenv) {
   AssumeCondition(dynamic_cast<const CompareOperationElement*>(&thisElement) && dynamic_cast<const SimplificationEnvironment*>(&aenv))
   CompareOperationElement& thisCompare = const_cast<CompareOperationElement&>((const CompareOperationElement&) thisElement);
   SimplificationEnvironment& env = (SimplificationEnvironment&) aenv;
   Methods::BooleanResult compareResult = IntOperationElement::Methods::applyBoolean(
         thisCompare.fstArg(), thisCompare.getOperation(), thisCompare.sndArg());
   if (compareResult.isTrue())
      env.presult() = IntOperationElement::Methods::newTrue(thisCompare.fstArg());
   else if (compareResult.isFalse())
      env.presult() = IntOperationElement::Methods::newFalse(thisCompare.fstArg());
   return true;
}

bool
CompareOperationElement::querySimplificationAsConstantDisjunction(const VirtualElement& thisElement, const VirtualQueryOperation& qoOperation, VirtualQueryOperation::Environment& aenv) {
   AssumeCondition(dynamic_cast<const IntOperationElement::SimplificationEnvironment*>(&aenv))
   AssumeCondition(dynamic_cast<const CompareOperationElement*>(&thisElement))
   CompareOperationElement& thisCompare = const_cast<CompareOperationElement&>((const CompareOperationElement&) thisElement);
   SimplificationEnvironment& env = (SimplificationEnvironment&) aenv;

   PNT::TPassPointer<VirtualDisjunction, PPAbstractElement> disjunction = Methods::newDisjunction(thisElement);
   disjunction->addMay(IntOperationElement::Methods::newTrue(thisCompare.fstArg()));
   disjunction->addMay(IntOperationElement::Methods::newFalse(thisCompare.fstArg()));
   env.presult() = disjunction;
   return true;
}

bool
CompareOperationElement::queryDispatchOnTopResult(const VirtualElement& thisElement, const VirtualQueryOperation& operation, VirtualQueryOperation::Environment& qoeEnv) {
   AssumeCondition(dynamic_cast<const CompareOperationElement*>(&thisElement))
   CompareOperationElement& thisCompare = const_cast<CompareOperationElement&>((const CompareOperationElement&) thisElement);
   PPVirtualElement top = IntOperationElement::Methods::newTop(thisCompare.fstArg());
   EvaluationEnvironment env = EvaluationEnvironment(EvaluationEnvironment::Init());
   assume(top->apply(Scalar::MultiBit::CastBitOperation(), env));
   if (env.hasResult())
      top = env.presult();
   return top->query(operation, qoeEnv);
}

bool
CompareOperationElement::constraintCastMultiBit(const VirtualOperation& operation,
      const VirtualElement& resultElement, ConstraintEnvironment& env) {
   Scalar::MultiBit::CastBitOperation inverseOperation;
   PPVirtualElement ppveNaturalElement = Methods::apply(resultElement, inverseOperation, EvaluationParameters(env).stopErrorStates());

   IntersectEnvironment intersectionEnv(env, Argument().setResult());
   intersectWith(*ppveNaturalElement, intersectionEnv);
   return true;
}

bool
CompareOperationElement::constraintNegateAssign(const VirtualOperation&,
      const VirtualElement& resultElement, ConstraintEnvironment& env) {
   env.mergeVerdict(EvaluationVerdict().setPrecise());
   PPVirtualElement arg = Methods::applyAssign(PPVirtualElement(resultElement), NegateAssignOperation(),
      EvaluationApplyParameters(env));
   IntersectEnvironment intersectionEnv(env);
   intersectWith(*arg, intersectionEnv);
   return true;
}

bool
CompareOperationElement::constraintPlusAssign(const VirtualOperation& operation,
      VirtualElement& sourceElement, const VirtualElement& resultElement, Argument argument,
      ConstraintEnvironment& env) { // a+b = x -> a = x-b et b = x-a
   env.mergeVerdictExact();
   PPVirtualElement fstArg = Methods::applyAssign(PPVirtualElement(resultElement), MinusAssignOperation(), sourceElement, EvaluationApplyParameters(env));
   PPVirtualElement sndArg = Methods::applyAssign(PPVirtualElement(resultElement), MinusAssignOperation(), *this, EvaluationApplyParameters(env));
   IntersectEnvironment thisIntersectionEnv(env, Argument().setResult()),
                        sourceIntersectionEnv(env, Argument().setFst());
   intersectWith(*fstArg, thisIntersectionEnv);
   sourceElement.intersectWith(*sndArg, sourceIntersectionEnv);
   return true;
}

bool
CompareOperationElement::constraintMinusAssign(const VirtualOperation& operation,
      VirtualElement& sourceElement, const VirtualElement& resultElement, Argument argument,
      ConstraintEnvironment& env) { // a-b = x -> a = b+x et b = a-x
   env.mergeVerdictExact();
   PPVirtualElement fstArg = Methods::applyAssign(PPVirtualElement(resultElement), PlusAssignOperation(), sourceElement, EvaluationApplyParameters(env));
   PPVirtualElement sndArg = Methods::applyAssign(PPVirtualElement(*this), MinusAssignOperation(), resultElement, EvaluationApplyParameters(env));
   IntersectEnvironment thisIntersectionEnv(env, Argument().setResult()),
                        sourceIntersectionEnv(env, Argument().setFst());
   intersectWith(*fstArg, thisIntersectionEnv);
   sourceElement.intersectWith(*sndArg, sourceIntersectionEnv);
   return true;
}

bool
CompareOperationElement::constraintOrAssign(const VirtualOperation&, VirtualElement& sourceElement,
      const VirtualElement& resultElement, Argument argument, ConstraintEnvironment& env) {
   ZeroResult booleanResult = resultElement.queryZeroResult();
   if (booleanResult.isZero()) {
      IntersectEnvironment intersectionEnvironment(env, Argument().setResult());
      intersectWith(resultElement, intersectionEnvironment);
      IntersectEnvironment sourceIntersectionEnvironment(env, Argument().setFst());
      sourceElement.intersectWith(resultElement, sourceIntersectionEnvironment);
      return true;
   };
   env.mergeVerdictExact();

   return true;
}

bool
CompareOperationElement::constraintAndAssign(const VirtualOperation&, VirtualElement& sourceElement,
      const VirtualElement& resultElement, Argument argument, ConstraintEnvironment& env) {
   ZeroResult booleanResult = resultElement.queryZeroResult();
   if (booleanResult.isDifferentZero()) {
      IntersectEnvironment intersectionEnvironment(env, Argument().setResult());
      intersectWith(resultElement, intersectionEnvironment);
      IntersectEnvironment sourceIntersectionEnvironment(env, Argument().setFst());
      sourceElement.intersectWith(resultElement, sourceIntersectionEnvironment);
      return true;
   };
   env.mergeVerdictExact();

   return true;
}

bool
CompareOperationElement::constraintExclusiveOrAssign(const VirtualOperation& operation,
      VirtualElement& sourceElement, const VirtualElement& resultElement, Argument argument,
      ConstraintEnvironment& env) {
   ZeroResult booleanResult = resultElement.queryZeroResult();
   if (booleanResult.isZero()) {
      IntersectEnvironment intersectionEnvironment(env, Argument().setResult());
      intersectWith(sourceElement, intersectionEnvironment);
      IntersectEnvironment sourceIntersectionEnvironment(env, Argument().setFst());
      sourceElement.intersectWith(*this, sourceIntersectionEnvironment);
      return true;
   };
   if (booleanResult.isDifferentZero()) {
      PPVirtualElement thisCopy(*this), sourceCopy(sourceElement);
      thisCopy = Methods::applyAssign(thisCopy, NegateAssignOperation(), EvaluationApplyParameters(env));
      sourceCopy = Methods::applyAssign(sourceCopy, NegateAssignOperation(), EvaluationApplyParameters(env));
      IntersectEnvironment intersectionEnvironment(env, Argument().setResult());
      intersectWith(*sourceCopy, intersectionEnvironment);
      IntersectEnvironment sourceIntersectionEnvironment(env, Argument().setFst());
      sourceElement.intersectWith(*thisCopy, sourceIntersectionEnvironment);
      return true;
   };
   env.mergeVerdictExact();
   return true;
}

}} // end of namespace Bit::Approximate

namespace MultiBit { namespace Approximate {

#define DefineInitApply(TypeOperation)                                                             \
   elementAt(Operation::T##TypeOperation).setMethod(&CompareOperationElement::apply##TypeOperation);

#define DefineInitOApply(TypeOperation, TypeRedirection)                                           \
   elementAt(Operation::T##TypeOperation).setMethod(&CompareOperationElement::apply##TypeRedirection);

#define DefineInitBiApply(TypeOperation)                                                           \
   elementAt(Operation::T##TypeOperation).setArray(new ConstantAndGenericApplyMethod(              \
      &CompareOperationElement::apply##TypeOperation##Constant,                                     \
      &CompareOperationElement::apply##TypeOperation));

#define DefineInitOBiApply(TypeOperation, TypeMethod)                                              \
   elementAt(Operation::T##TypeOperation).setArray(new ConstantAndGenericApplyMethod(              \
      &CompareOperationElement::apply##TypeMethod##Constant,                                        \
      &CompareOperationElement::apply##TypeMethod));

CompareOperationElement::MethodApplyTable::MethodApplyTable() {
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
   DefineInitApply(CastBit)
   DefineInitApply(CastShiftBit)
   // DefineInitOApply(CastMultiFloat, CastDouble)
   // DefineInitOApply(CastMultiFloatPointer, CastDouble)
   // DefineInitOApply(CastReal, CastDouble)
   // DefineInitOApply(CastRational, CastDouble)
   
   DefineInitOApply(PrevSignedAssign, NegateAssign)
   DefineInitOApply(PrevUnsignedAssign, NegateAssign)
   DefineInitOApply(NextSignedAssign, NegateAssign)
   DefineInitOApply(NextUnsignedAssign, NegateAssign)
   DefineInitOApply(PlusSignedAssign, PlusAssign)
   DefineInitOApply(PlusUnsignedAssign, PlusAssign)
   DefineInitOApply(PlusUnsignedWithSignedAssign, PlusAssign)
   DefineInitOApply(PlusFloatAssign, Float)
   DefineInitOApply(MinusSignedAssign, PlusAssign)
   DefineInitOApply(MinusUnsignedAssign, PlusAssign)
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

   DefineInitOApply(BitOrAssign, OrAssign)
   DefineInitOApply(BitAndAssign, AndAssign)
   DefineInitOApply(BitExclusiveOrAssign, ExclusiveOrAssign)
   DefineInitOApply(BitNegateAssign, NegateAssign)
   DefineInitBiApply(LeftShiftAssign)
   DefineInitOBiApply(LogicalRightShiftAssign, RightShiftAssign)
   DefineInitOBiApply(ArithmeticRightShiftAssign, RightShiftAssign)
   // DefineInitBiApply(LeftRotateAssign)
   // DefineInitBiApply(RightRotateAssign)
#include "StandardClasses/DefineNew.h"
}

#undef DefineInitApply
#undef DefineInitOApply
#undef DefineInitBiApply
#undef DefineInitOBiApply

#define DefineInitApplyTo(TypeOperation)                                                        \
   elementAt(Operation::T##TypeOperation).setMethod(&CompareOperationElement::applyTo##TypeOperation);
#define DefineInitOApplyTo(TypeOperation, TypeMethod)                                                        \
   elementAt(Operation::T##TypeOperation).setMethod(&CompareOperationElement::applyTo##TypeMethod);
CompareOperationElement::MethodApplyToTable::MethodApplyToTable() {
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
   // DefineInitOApplyTo(LeftShiftAssign, ShiftAssign)
   // DefineInitOApplyTo(LogicalRightShiftAssign, ShiftAssign)
   // DefineInitOApplyTo(ArithmeticRightShiftAssign, ShiftAssign)
   // DefineInitOApplyTo(LeftRotateAssign, RotateAssign)
   // DefineInitOApplyTo(RightRotateAssign, RotateAssign)
#include "StandardClasses/DefineNew.h"
}
#undef DefineInitOApplyTo
#undef DefineInitApplyTo

#define DefineInitUnaryConstraint(TypeOperation) \
   mcutUnaryTable[Operation::T##TypeOperation].setMethod(&CompareOperationElement::constraint##TypeOperation);

#define DefineInitUnaryGConstraint(TypeOperation) \
   mcutUnaryTable[Operation::T##TypeOperation].setFunction(&MultiBit::Approximate::Details::BaseAlgorithms::constraint##TypeOperation);

#define DefineInitUnaryOConstraint(TypeOperation, TypeMethod) \
   mcutUnaryTable[Operation::T##TypeOperation].setMethod(&VirtualElement::constraint##TypeMethod);

#define DefineInitUnaryOGConstraint(TypeOperation, TypeMethod) \
   mcutUnaryTable[Operation::T##TypeOperation].setFunction(&MultiBit::Approximate::Details::BaseAlgorithms::constraint##TypeMethod);

#define DefineInitUnaryOGIntConstraint(TypeOperation, TypeMethod) \
   mcutUnaryTable[Operation::T##TypeOperation].setFunction(&IntOperationElement::constraint##TypeMethod);

#define DefineInitUnaryGIntConstraint(TypeOperation) \
   mcutUnaryTable[Operation::T##TypeOperation].setFunction(&IntOperationElement::constraint##TypeOperation);

#define DefineInitBinaryConstraint(TypeOperation) \
   mcbtBinaryTable[Operation::T##TypeOperation].setMethod(&CompareOperationElement::constraint##TypeOperation);

#define DefineInitBinaryGConstraint(TypeOperation) \
   mcbtBinaryTable[Operation::T##TypeOperation].setFunction(&MultiBit::Approximate::Details::BaseAlgorithms::constraint##TypeOperation);

#define DefineInitBinaryGIntConstraint(TypeOperation) \
   mcbtBinaryTable[Operation::T##TypeOperation].setFunction(&IntOperationElement::constraint##TypeOperation);

#define DefineInitBinaryOGConstraint(TypeOperation, TypeFunction) \
   mcbtBinaryTable[Operation::T##TypeOperation].setFunction(&MultiBit::Approximate::Details::BaseAlgorithms::constraint##TypeFunction);

#define DefineInitBinaryOGIntConstraint(TypeOperation, TypeFunction) \
   mcbtBinaryTable[Operation::T##TypeOperation].setFunction(&IntOperationElement::constraint##TypeFunction);

#define DefineInitBinaryConstraintGeneric(TypeOperation)                                         \
   mcbtBinaryTable[Operation::T##TypeOperation].setArray(                                        \
      new Scalar::Approximate::Details::GenericBinaryConstraintMethod<DomainTraits>(&CompareOperationElement::constraint##TypeOperation));

#define DefineInitOBinaryConstraintGeneric(TypeOperation, TypeMethod)                            \
   mcbtBinaryTable[Operation::T##TypeOperation].setArray(                                        \
      new Scalar::Approximate::Details::GenericBinaryConstraintMethod<DomainTraits>(&CompareOperationElement::constraint##TypeMethod));

CompareOperationElement::MethodConstraintTable::MethodConstraintTable() {
#include "StandardClasses/UndefineNew.h"
   DefineInitUnaryGConstraint(ExtendWithZero)
   DefineInitUnaryGConstraint(ExtendWithSign)
   DefineInitBinaryGConstraint(Concat)
   DefineInitUnaryGConstraint(Reduce)
   DefineInitBinaryGConstraint(BitSet)

   DefineInitUnaryConstraint(CastBit)
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

   // DefineInitBinaryOConstraint(PlusSignedAssign, PlusAssign)
   // DefineInitBinaryOConstraint(PlusUnsignedAssign, PlusAssign)
   // DefineInitBinaryOConstraint(PlusUnsignedWithSignedAssign, PlusAssign)
   // DefineInitBinaryOConstraint(MinusSignedAssign, MinusAssign)
   // DefineInitBinaryOConstraint(MinusUnsignedAssign, MinusAssign)
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
   // DefineInitBinaryGIntConstraint(BitOrAssign)
   // DefineInitBinaryGIntConstraint(BitAndAssign)
   // DefineInitBinaryGIntConstraint(BitExclusiveOrAssign)
   DefineInitBinaryConstraint(BitOrAssign)
   DefineInitBinaryConstraint(BitAndAssign)
   DefineInitBinaryConstraint(BitExclusiveOrAssign)
   
   DefineInitUnaryConstraint(BitNegateAssign)
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

CompareOperationElement::MethodApplyTable CompareOperationElement::matMethodApplyTable;
CompareOperationElement::MethodApplyToTable CompareOperationElement::mattMethodApplyToTable;
CompareOperationElement::MethodConstraintTable CompareOperationElement::mctMethodConstraintTable;

#define DefineInitQuery(TypeOperation) \
   elementAt(QueryOperation::TypeOperation).setMethod(&ConcatOperationElement::query##TypeOperation);

CompareOperationElement::QueryTable::QueryTable() {
#include "StandardClasses/UndefineNew.h"
   FunctionQueryTable* table = &elementAt(VirtualQueryOperation::TCompareSpecial);
   table->setSize(QueryOperation::EndOfTypeCompareSpecial);
   (*table)[QueryOperation::TCSGuard].setMethod(&CompareOperationElement::queryGuardAll);
   (*table)[QueryOperation::TCSNatural].setMethod(&CompareOperationElement::queryCompareSpecial);
   (*table)[QueryOperation::TCSSigned].setMethod(&CompareOperationElement::queryCompareSpecial);
   (*table)[QueryOperation::TCSUnsigned].setMethod(&CompareOperationElement::queryCompareSpecial);
   (*table)[QueryOperation::TCSBitDomain].setMethod(&CompareOperationElement::queryDispatchOnTopResult);
   
   table = &elementAt(VirtualQueryOperation::TSimplification);
   table->setSize(QueryOperation::EndOfTypeSimplification);
   (*table)[QueryOperation::TSConstant].setMethod(&Top::queryFailSimplification);
   (*table)[QueryOperation::TSInterval].setMethod(&CompareOperationElement::querySimplificationAsInterval);
   (*table)[QueryOperation::TSConstantDisjunction].setMethod(&CompareOperationElement::querySimplificationAsConstantDisjunction);

   table = &elementAt(VirtualQueryOperation::TDomain);
   table->setSize(QueryOperation::EndOfTypeDomain);

#define DefineInitQueryMethodTable(TypeOperation)                              \
   (*table)[QueryOperation::TD##TypeOperation].setMethod(&CompareOperationElement::queryDispatchOnTopResult);
   
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

   table = &elementAt(VirtualQueryOperation::TDuplication);
   table->setSize(3);
   (*table)[QueryOperation::TCDClone].resetMethod(&CompareOperationElement::queryCopy);
   (*table)[QueryOperation::TCDSpecialize].resetMethod(&CompareOperationElement::querySpecialize);

   table = &elementAt(VirtualQueryOperation::TExtern);
   table->setSize(QueryOperation::EndOfTypeExtern);
   (*table)[QueryOperation::TESigned].setMethod(&CompareOperationElement::queryDispatchOnTopResult);
#include "StandardClasses/DefineNew.h"
}
#undef DefineInitQuery

CompareOperationElement::QueryTable CompareOperationElement::mqtMethodQueryTable;

#define DefineTypeOperation MultiBit
#define DefineTypeObject CompareOperationElement
#include "Analyzer/Scalar/Approximate/VirtualCall.incc"
#undef DefineTypeObject
#undef DefineTypeOperation

bool
CompareOperationElement::intersectWith(const VirtualElement& asource, EvaluationEnvironment& env) {
   if (!env.isTopLevel()) {
      if (inherited::intersectWith(asource, env))
         return true;
   };
   EvaluationEnvironment copyEnv = EvaluationEnvironment(EvaluationParameters(env));
   const_cast<VirtualElement&>(asource).apply(Scalar::MultiBit::CastBitOperation(), copyEnv);
   PPVirtualElement source = copyEnv.presult();
   ConstraintEnvironment constraintEnvironment = ConstraintEnvironment(EvaluationParameters(env));
   if (constraintEnvironment.getLatticeCreation().isFormal())
      constraintEnvironment.setTopLatticeCreationAuthorizeFormal();
   constraintEnvironment.absorbFirstArgument(sndArg().createSCopy());
   if (!fstArg().constraint(getOperation(), *source, constraintEnvironment)) return false;
   if (constraintEnvironment.hasResult())
      sfstArg() = constraintEnvironment.presult();
   ssndArg().absorbElement((VirtualElement*) constraintEnvironment.extractFirstArgument());
   return true;
}

bool
CompareOperationElement::applyExtend(const VirtualOperation& operation, EvaluationEnvironment& env) {
   PPVirtualElement multiBitTop = Approximate::Details::IntOperationElement::Methods::newTop(fstArg());
   multiBitTop = Approximate::Details::IntOperationElement::Methods::applyAssign(multiBitTop, Scalar::MultiBit::ReduceOperation().setLowBit(0).setHighBit(0), Methods::EvaluationParameters(env));
   bool result = multiBitTop->apply(operation, env);
   if (result && !env.hasResult())
      env.presult() = multiBitTop;
   return result;
}

bool
CompareOperationElement::applyConcat(const VirtualOperation& operation, EvaluationEnvironment& env) {
   MultiBit::Approximate::ConcatOperationElement* result;
   {  MultiBit::Approximate::ConcatOperationElement::Init init;
      init.setBitSize(1 + env.getFirstArgument().getSizeInBits());
      result = new MultiBit::Approximate::ConcatOperationElement(init);
   };
   env.presult().absorbElement(result);
   result->sfstArg().setElement(*this);
   result->ssndArg().setElement(env.getFirstArgument());
   return true;
}

bool
CompareOperationElement::applyReduce(const VirtualOperation& operation, EvaluationEnvironment& env) {
   return true;
}

bool
CompareOperationElement::applyBitSet(const VirtualOperation& operation, EvaluationEnvironment& env) {
   AssumeCondition(env.getFirstArgument().getSizeInBits() == 1)
   env.presult().setElement(env.getFirstArgument());
   return true;
   // PPVirtualElement multiBitTop = IntOperationElement::Methods::newTop(fstArg());
   // multiBitTop = IntOperationElement::Methods::applyAssign(multiBitTop, Scalar::MultiBit::ReduceOperation().setLowBit(0).setHighBit(0), Methods::EvaluationParameters(env));
   // bool result = multiBitTop->apply(operation, env);
   // if (result && !env.hasResult())
   //    env.presult() = multiBitTop;
   // return result;
}

bool
CompareOperationElement::applyCastBit(const VirtualOperation& operation, EvaluationEnvironment& env) {
   typedef Bit::Approximate::CompareOperationElement CompareOperationElement;
   auto* result = new CompareOperationElement((const CompareOperationElement::Init&)
         CompareOperationElement::Init().set1(),
         ((const Operation&) getOperation()).getType());
   env.presult().absorbElement(result);
   result->sfstArg().setElement(fstArg());
   result->ssndArg().setElement(sndArg());
   return true;
}

bool
CompareOperationElement::applyCastShiftBit(const VirtualOperation& operation, EvaluationEnvironment& env) {
   typedef Bit::Approximate::CompareOperationElement CompareOperationElement;
   auto* result = new CompareOperationElement((const CompareOperationElement::Init&)
         CompareOperationElement::Init().set1(),
         ((const Operation&) getOperation()).getType());
   env.presult().absorbElement(result);
   result->sfstArg().setElement(fstArg());
   result->ssndArg().setElement(sndArg());
   return true;
}

bool
CompareOperationElement::applyCompareLessOrGreater(const VirtualOperation& operation, EvaluationEnvironment& env) {
   const VirtualElement& source = env.getFirstArgument();
   ZeroResult argument = source.queryZeroResult();
   bool isLess = ((const Operation&) operation).getType() < Operation::TCompareEqual;
   if (isLess ? argument.isZero() : argument.isDifferentZero())
      return applyFalse(operation, env);
   typedef Bit::Approximate::CompareOperationElement CompareOperationElement;
   if (isLess ? argument.isDifferentZero() : argument.isZero()) {
      if (isLess) {
         auto* resultAsBit = new CompareOperationElement((const CompareOperationElement::Init&)
               CompareOperationElement::Init().set1(),
               ((const Operation&) getOperation()).getType());
         PPVirtualElement resultElement(resultAsBit, PNT::Pointer::Init());
         resultAsBit->sfstArg().setElement(fstArg());
         resultAsBit->ssndArg().setElement(sndArg());
         bool result = resultAsBit->applyNegateAssign(operation, env);
         if (result && !env.hasResult())
            env.presult() = resultElement;
         return result;
      }
      else
         return applyCastBit(operation, env);
   };
   env.mergeVerdictDegradate();
   auto* resultAsBit = new CompareOperationElement((const CompareOperationElement::Init&)
         CompareOperationElement::Init().set1(),
         ((const Operation&) getOperation()).getType());
   PPVirtualElement resultElement(resultAsBit, PNT::Pointer::Init());
   resultAsBit->sfstArg().setElement(fstArg());
   resultAsBit->ssndArg().setElement(sndArg());
   bool result = resultAsBit->applyTop(operation, env);
   if (result && !env.hasResult())
      env.presult() = resultElement;
   return result;
}

bool
CompareOperationElement::applyCompareLessOrGreaterOrEqual(const VirtualOperation& operation, EvaluationEnvironment& env) {
   const VirtualElement& asource = env.getFirstArgument();
   ZeroResult argument = asource.queryZeroResult();
   bool isLess = ((const Operation&) operation).getType() < Operation::TCompareEqual;
   typedef Bit::Approximate::CompareOperationElement CompareOperationElement;
   if (isLess ? argument.isZero() : argument.isDifferentZero()) {
      if (isLess) {
         auto* resultAsBit = new CompareOperationElement((const CompareOperationElement::Init&)
               CompareOperationElement::Init().set1(),
               ((const Operation&) getOperation()).getType());
         PPVirtualElement resultElement(resultAsBit, PNT::Pointer::Init());
         resultAsBit->sfstArg().setElement(fstArg());
         resultAsBit->ssndArg().setElement(sndArg());
         bool result = resultAsBit->applyNegateAssign(operation, env);
         if (result && !env.hasResult())
            env.presult() = resultElement;
         return result;
      }
      else
         return applyCastBit(operation, env);
   };
      
   if (isLess ? argument.isDifferentZero() : argument.isZero())
      return applyTrue(operation, env);
   if (asource.getApproxKind().isFormalOperation()) {
      AssumeCondition(dynamic_cast<const FormalOperationElement*>(&asource))
      // const FormalOperationElement& source = (const FormalOperationElement&) asource;
      // MultiBitOperation::Type type = ((const MultiBitOperation&) source.getOperation()).getType();
/*    if (type >= MultiBitOperation::StartOfCompare && type < MultiBitOperation::EndOfCompare) {
         if (fstArg().getApproxKind().isVariable()) {
            if (source.fstArg().getApproxKind().isVariable()) {
               AssumeCondition(dynamic_cast<const VirtualExactToApproximateElement*>(&fstArg()) && dynamic_cast<const VirtualExactToApproximateElement*>(&source.fstArg()))
               if (((const VirtualExactToApproximateElement&) fstArg()).isEqual((const VirtualExactToApproximateElement&) source.fstArg())) {

               };
            };
            if (source.sndArg().getApproxKind().isVariable()) {
               AssumeCondition(dynamic_cast<const VirtualExactToApproximateElement*>(&fstArg()) && dynamic_cast<const VirtualExactToApproximateElement*>(&source.sndArg()))
               if (((const VirtualExactToApproximateElement&) fstArg()).isEqual((const VirtualExactToApproximateElement&) source.sndArg())) {

               };
            };
         };
         if (sndArg().getApproxKind().isVariable()) {
            if (source.fstArg().getApproxKind().isVariable()) {
               AssumeCondition(dynamic_cast<const VirtualExactToApproximateElement*>(&sndArg()) && dynamic_cast<const VirtualExactToApproximateElement*>(&source.fstArg()))
               if (((const VirtualExactToApproximateElement&) sndArg()).isEqual((const VirtualExactToApproximateElement&) source.fstArg())) {

               };
            };
            if (source.sndArg().getApproxKind().isVariable()) {
               AssumeCondition(dynamic_cast<const VirtualExactToApproximateElement*>(&sndArg()) && dynamic_cast<const VirtualExactToApproximateElement*>(&source.sndArg()))
               if (((const VirtualExactToApproximateElement&) sndArg()).isEqual((const VirtualExactToApproximateElement&) source.sndArg())) {

               };
            };
         };
      }; */
   };
   env.mergeVerdictDegradate();
   auto* resultAsBit = new CompareOperationElement((const CompareOperationElement::Init&)
         CompareOperationElement::Init().set1(),
         ((const Operation&) getOperation()).getType());
   PPVirtualElement resultElement(resultAsBit, PNT::Pointer::Init());
   resultAsBit->sfstArg().setElement(fstArg());
   resultAsBit->ssndArg().setElement(sndArg());
   bool result = resultAsBit->applyTop(operation, env);
   if (result && !env.hasResult())
      env.presult() = resultElement;
   return result;
}

bool
CompareOperationElement::applyCompareEqual(const VirtualOperation& operation, EvaluationEnvironment& env) {
   const VirtualElement& source = env.getFirstArgument();
   ZeroResult argument = source.queryZeroResult();
   typedef Bit::Approximate::CompareOperationElement CompareOperationElement;
   if (argument.isZero()) {
      CompareOperationElement* resultAsBit = new CompareOperationElement((const CompareOperationElement::Init&)
            CompareOperationElement::Init().set1(),
            ((const Operation&) getOperation()).getType());
      PPVirtualElement resultElement(resultAsBit, PNT::Pointer::Init());
      resultAsBit->sfstArg().setElement(fstArg());
      resultAsBit->ssndArg().setElement(sndArg());
      bool result = resultAsBit->applyNegateAssign(operation, env);
      if (result && !env.hasResult())
         env.presult() = resultElement;
      return result;
   };
   if (argument.isDifferentZero())
      return applyCastBit(operation, env);
   env.mergeVerdictDegradate();
   CompareOperationElement* resultAsBit = new CompareOperationElement((const CompareOperationElement::Init&)
         CompareOperationElement::Init().set1(),
         ((const Operation&) getOperation()).getType());
   PPVirtualElement resultElement(resultAsBit, PNT::Pointer::Init());
   resultAsBit->sfstArg().setElement(fstArg());
   resultAsBit->ssndArg().setElement(sndArg());
   bool result = resultAsBit->applyTop(operation, env);
   if (result && !env.hasResult())
      env.presult() = resultElement;
   return result;
}

bool
CompareOperationElement::applyCompareDifferent(const VirtualOperation& operation, EvaluationEnvironment& env) {
   const VirtualElement& source = env.getFirstArgument();
   ZeroResult argument = source.queryZeroResult();
   if (argument.isZero())
      return applyCastBit(operation, env);
   typedef Bit::Approximate::CompareOperationElement CompareOperationElement;
   if (argument.isDifferentZero()) {
      auto* resultAsBit = new CompareOperationElement((const CompareOperationElement::Init&)
            CompareOperationElement::Init().set1(),
            ((const Operation&) getOperation()).getType());
      PPVirtualElement resultElement(resultAsBit, PNT::Pointer::Init());
      resultAsBit->sfstArg().setElement(fstArg());
      resultAsBit->ssndArg().setElement(sndArg());
      bool result = resultAsBit->applyNegateAssign(operation, env);
      if (result && !env.hasResult())
         env.presult() = resultElement;
      return result;
   };
   env.mergeVerdictDegradate();
   auto* resultAsBit = new CompareOperationElement((const CompareOperationElement::Init&)
         CompareOperationElement::Init().set1(),
         ((const Operation&) getOperation()).getType());
   PPVirtualElement resultElement(resultAsBit, PNT::Pointer::Init());
   resultAsBit->sfstArg().setElement(fstArg());
   resultAsBit->ssndArg().setElement(sndArg());
   bool result = resultAsBit->applyTop(operation, env);
   if (result && !env.hasResult())
      env.presult() = resultElement;
   return result;
}

bool
CompareOperationElement::applyFloat(const VirtualOperation& operation, EvaluationEnvironment& env) {
   PPVirtualElement multiBitTop = Approximate::Details::IntOperationElement::Methods::newTop(fstArg());
   multiBitTop = Approximate::Details::IntOperationElement::Methods::applyAssign(multiBitTop,
      ReduceOperation().setLowBit(0).setHighBit(0), Methods::EvaluationParameters(env));
   bool result = multiBitTop->apply(operation, env);
   if (result && !env.hasResult())
      env.presult() = multiBitTop;
   return result;
}

bool
CompareOperationElement::applyTimesAssign(const VirtualOperation& operation, EvaluationEnvironment& env) {
   PPVirtualElement multiBitTop = Approximate::Details::IntOperationElement::Methods::newTop(fstArg());
   multiBitTop = Approximate::Details::IntOperationElement::Methods::applyAssign(multiBitTop,
      Scalar::MultiBit::ReduceOperation().setLowBit(0).setHighBit(0), Methods::EvaluationParameters(env));
   bool result = multiBitTop->apply(operation, env);
   if (result && !env.hasResult())
      env.presult() = multiBitTop;
   return result;
}

bool
CompareOperationElement::applyDivideAssign(const VirtualOperation& operation, EvaluationEnvironment& env) {
   PPVirtualElement multiBitTop = Approximate::Details::IntOperationElement::Methods::newTop(fstArg());
   multiBitTop = Approximate::Details::IntOperationElement::Methods::applyAssign(multiBitTop,
      Scalar::MultiBit::ReduceOperation().setLowBit(0).setHighBit(0), Methods::EvaluationParameters(env));
   bool result = multiBitTop->apply(operation, env);
   if (result && !env.hasResult())
      env.presult() = multiBitTop;
   return result;
}

bool
CompareOperationElement::applyOppositeSignedAssign(const VirtualOperation& operation, EvaluationEnvironment& env) {
   return true;
}

bool
CompareOperationElement::applyModuloAssign(const VirtualOperation& operation, EvaluationEnvironment& env) {
   PPVirtualElement multiBitTop = Approximate::Details::IntOperationElement::Methods::newTop(fstArg());
   multiBitTop = Approximate::Details::IntOperationElement::Methods::applyAssign(multiBitTop,
      Scalar::MultiBit::ReduceOperation().setLowBit(0).setHighBit(0), Methods::EvaluationParameters(env));
   bool result = multiBitTop->apply(operation, env);
   if (result && !env.hasResult())
      env.presult() = multiBitTop;
   return result;
}

bool
CompareOperationElement::applyExclusiveOrAssign(const VirtualOperation& operation, EvaluationEnvironment& env) {
   bool result = applyNegateAssign(operation, env);
   if (!result)
      return false;
   if (env.hasResult()) {
      PPVirtualElement negate = env.presult();
      Operation equalOperation;
      result = negate->apply(equalOperation.setCompareEqual(), env);
   }
   else
      result = applyCompareEqual(operation, env);
   PPVirtualElement bitResult = env.presult();
   bitResult->apply(Scalar::Bit::CastMultiBitOperation().setSize(1), env);
   return result;
}

bool
CompareOperationElement::applyLeftShiftAssignConstant(const VirtualOperation& operation,
      const VirtualElement& source, EvaluationEnvironment& env) {
   PPVirtualElement multiBitTop = Approximate::Details::IntOperationElement::Methods::newTop(fstArg());
   multiBitTop = Approximate::Details::IntOperationElement::Methods::applyAssign(multiBitTop,
         Scalar::MultiBit::ReduceOperation().setLowBit(0).setHighBit(0), Methods::EvaluationParameters(env));
   bool result = multiBitTop->apply(operation, env);
   if (result && !env.hasResult())
      env.presult() = multiBitTop;
   return result;
}

bool
CompareOperationElement::applyLeftShiftAssign(const VirtualOperation& operation,
      const VirtualElement& source, EvaluationEnvironment& env) {
   PPVirtualElement multiBitTop = Approximate::Details::IntOperationElement::Methods::newTop(fstArg());
   multiBitTop = Approximate::Details::IntOperationElement::Methods::applyAssign(multiBitTop,
         Scalar::MultiBit::ReduceOperation().setLowBit(0).setHighBit(0), Methods::EvaluationParameters(env));
   bool result = multiBitTop->apply(operation, env);
   if (result && !env.hasResult())
      env.presult() = multiBitTop;
   return result;
}

bool
CompareOperationElement::applyRightShiftAssignConstant(const VirtualOperation& operation,
      const VirtualElement& source, EvaluationEnvironment& env) {
   PPVirtualElement multiBitTop = Approximate::Details::IntOperationElement::Methods::newTop(fstArg());
   multiBitTop = Approximate::Details::IntOperationElement::Methods::applyAssign(multiBitTop,
         Scalar::MultiBit::ReduceOperation().setLowBit(0).setHighBit(0), Methods::EvaluationParameters(env));
   bool result = multiBitTop->apply(operation, env);
   if (result && !env.hasResult())
      env.presult() = multiBitTop;
   return result;
}

bool
CompareOperationElement::applyRightShiftAssign(const VirtualOperation& operation,
      const VirtualElement& source, EvaluationEnvironment& env) {
   PPVirtualElement multiBitTop = Approximate::Details::IntOperationElement::Methods::newTop(fstArg());
   multiBitTop = Approximate::Details::IntOperationElement::Methods::applyAssign(multiBitTop,
         Scalar::MultiBit::ReduceOperation().setLowBit(0).setHighBit(0), Methods::EvaluationParameters(env));
   bool result = multiBitTop->apply(operation, env);
   if (result && !env.hasResult())
      env.presult() = multiBitTop;
    return result;
}
 
bool
CompareOperationElement::applyToBitSet(const VirtualOperation& operation, VirtualElement& source, EvaluationEnvironment& env) const {
   if (source.getSizeInBits() == 1) {
      env.presult().setElement(*this);
      return true;
   };
   if (source.getType().isVariable()) {
      AssumeCondition(dynamic_cast<const VirtualExactToApproximateElement*>(&source))
      return ((VirtualExactToApproximateElement&) source).sapply(operation, env);
   };
   PPVirtualElement multiBitTop = Approximate::Details::IntOperationElement::Methods::newTop(fstArg());
   multiBitTop = Approximate::Details::IntOperationElement::Methods::applyAssign(multiBitTop, Scalar::MultiBit::ReduceOperation().setLowBit(0).setHighBit(0), Methods::EvaluationParameters(env));
   TransmitEnvironment envCopy(env);
   envCopy.setFirstArgument(*multiBitTop);
   bool result = source.apply(operation, envCopy);
   if (result && envCopy.hasResult())
      env.presult() = envCopy.presult();
   return result;
}

bool
CompareOperationElement::applyToConcat(const VirtualOperation& operation, VirtualElement& source, EvaluationEnvironment& env) const {
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
CompareOperationElement::applyToMinusAssign(const VirtualOperation& operation, VirtualElement& source, EvaluationEnvironment& env) const {
   PNT::TPassPointer<CompareOperationElement, PPAbstractElement> thisCopy(*this);
   TransmitEnvironment applyEnv(env);
   bool result = thisCopy->applyNegateAssign(operation, applyEnv);
   if (!result)
      return false;
   if (applyEnv.hasResult())
      thisCopy = applyEnv.presult();
   applyEnv.merge().clear();
   applyEnv.setFirstArgument(*thisCopy);
   Operation plusOperation((const Operation&) operation);
   switch (((const Operation&) operation).getType()) {
      case Operation::TMinusSignedAssign: plusOperation.setType(Operation::TPlusSignedAssign); break;
      case Operation::TMinusUnsignedAssign: plusOperation.setType(Operation::TPlusUnsignedAssign); break;
      case Operation::TMinusFloatAssign: plusOperation.setType(Operation::TPlusFloatAssign); break;
      default:
         {  AssumeUncalled }
   };
   result = source.apply(plusOperation, applyEnv);
   if (applyEnv.hasResult())
      env.presult() = applyEnv.presult();
   return result;
}

bool
CompareOperationElement::applyToCompareLessOrGreater(const VirtualOperation& aoperation, VirtualElement& thisElement, EvaluationEnvironment& env) const {
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
   return const_cast<CompareOperationElement&>(*this).applyCompareLessOrGreaterOrEqual(operation, applyEnv);
}

bool
CompareOperationElement::applyToCompareLessOrGreaterOrEqual(const VirtualOperation& aoperation, VirtualElement& thisElement, EvaluationEnvironment& env) const {
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
   return const_cast<CompareOperationElement&>(*this).applyCompareLessOrGreaterOrEqual(operation, applyEnv);
}

bool
CompareOperationElement::applyToDivideAssign(const VirtualOperation& operation,
      VirtualElement& source, EvaluationEnvironment& env) const {
   PPVirtualElement multiBitTop = IntOperationElement::Methods::newTop(fstArg());
   multiBitTop = IntOperationElement::Methods::applyAssign(multiBitTop, Scalar::MultiBit::ReduceOperation().setLowBit(0).setHighBit(0), Methods::EvaluationParameters(env));
   TransmitEnvironment envCopy(env);
   envCopy.setFirstArgument(*multiBitTop);
   bool result = source.apply(operation, envCopy);
   if (result && envCopy.hasResult())
      env.presult() = envCopy.presult();
   return result;
}

bool
CompareOperationElement::applyToModuloAssign(const VirtualOperation& operation, VirtualElement& source, EvaluationEnvironment& env) const {
   PPVirtualElement multiBitTop = IntOperationElement::Methods::newTop(fstArg());
   multiBitTop = IntOperationElement::Methods::applyAssign(multiBitTop, Scalar::MultiBit::ReduceOperation().setLowBit(0).setHighBit(0), Methods::EvaluationParameters(env));
   TransmitEnvironment envCopy(env);
   envCopy.setFirstArgument(*multiBitTop);
   bool result = source.apply(operation, envCopy);
   if (result && envCopy.hasResult())
      env.presult() = envCopy.presult();
   return result;
}

bool
CompareOperationElement::applyToFloat(const VirtualOperation& operation, VirtualElement& source, EvaluationEnvironment& env) const {
   AssumeUnimplemented
   return true;
}

bool
CompareOperationElement::constraintCastBit(const VirtualOperation& operation,
      const VirtualElement& resultElement, ConstraintEnvironment& env) {
   Scalar::Bit::CastMultiBitOperation inverseOperation;
   PPVirtualElement ppveNaturalElement = Methods::apply(resultElement, inverseOperation.setSize(1), EvaluationParameters(env).stopErrorStates());

   IntersectEnvironment intersectionEnv(env, Argument().setResult());
   intersectWith(*ppveNaturalElement, intersectionEnv);
   return true;
}

bool
CompareOperationElement::constraintBitNegateAssign(const VirtualOperation&,
      const VirtualElement& resultElement, ConstraintEnvironment& env) {
   env.mergeVerdict(EvaluationVerdict().setPrecise());
   PPVirtualElement arg = Methods::applyAssign(PPVirtualElement(resultElement),
      Scalar::MultiBit::BitNegateAssignOperation(), EvaluationApplyParameters(env));
   IntersectEnvironment intersectionEnv(env);
   intersectWith(*arg, intersectionEnv);
   return true;
}

bool
CompareOperationElement::constraintPlusAssign(const VirtualOperation& operation, VirtualElement& source,
      const VirtualElement& resultElement, Argument argument, ConstraintEnvironment& env) { // a+b = x -> a = x-b et b = x-a
   env.mergeVerdictExact();
   PPVirtualElement fstArg = Methods::applyAssign(PPVirtualElement(resultElement),
         Scalar::MultiBit::MinusUnsignedAssignOperation(), source, EvaluationApplyParameters(env));
   PPVirtualElement sndArg = Methods::applyAssign(PPVirtualElement(resultElement),
         Scalar::MultiBit::MinusUnsignedAssignOperation(), *this, EvaluationApplyParameters(env));
   IntersectEnvironment thisIntersectionEnv(env, Argument().setResult()),
         sourceIntersectionEnv(env, Argument().setFst());
   intersectWith(*fstArg, thisIntersectionEnv);
   source.intersectWith(*sndArg, sourceIntersectionEnv);
   return true;
}

bool
CompareOperationElement::constraintMinusAssign(const VirtualOperation& operation, VirtualElement& source,
      const VirtualElement& resultElement, Argument argument, ConstraintEnvironment& env) { // a-b = x -> a = b+x et b = a-x
   env.mergeVerdictExact();
   PPVirtualElement fstArg = Methods::applyAssign(PPVirtualElement(resultElement),
         Scalar::MultiBit::PlusUnsignedAssignOperation(), source, EvaluationApplyParameters(env));
   PPVirtualElement sndArg = Methods::applyAssign(PPVirtualElement(*this),
         Scalar::MultiBit::PlusUnsignedAssignOperation(), resultElement, EvaluationApplyParameters(env));
   IntersectEnvironment thisIntersectionEnv(env, Argument().setResult()),
         sourceIntersectionEnv(env, Argument().setFst());
   intersectWith(*fstArg, thisIntersectionEnv);
   source.intersectWith(*sndArg, sourceIntersectionEnv);
   return true;
}

bool
CompareOperationElement::constraintBitOrAssign(const VirtualOperation&,
      VirtualElement& source, const VirtualElement& resultElement, Argument argument,
      ConstraintEnvironment& env) {
   ZeroResult booleanResult = resultElement.queryZeroResult();
   if (booleanResult.isZero()) {
      IntersectEnvironment intersectionEnvironment(env, Argument().setResult());
      intersectWith(resultElement, intersectionEnvironment);
      IntersectEnvironment sourceIntersectionEnvironment(env, Argument().setFst());
      source.intersectWith(resultElement, sourceIntersectionEnvironment);
      return true;
   };
   env.mergeVerdictExact();

   return true;
}

bool
CompareOperationElement::constraintBitAndAssign(const VirtualOperation&, VirtualElement& source,
      const VirtualElement& resultElement, Argument argument, ConstraintEnvironment& env) {
   ZeroResult booleanResult = resultElement.queryZeroResult();
   if (booleanResult.isDifferentZero()) {
      IntersectEnvironment intersectionEnvironment(env, Argument().setResult());
      intersectWith(resultElement, intersectionEnvironment);
      IntersectEnvironment sourceIntersectionEnvironment(env, Argument().setFst());
      source.intersectWith(resultElement, sourceIntersectionEnvironment);
      return true;
   };
   env.mergeVerdictExact();

   return true;
}

bool
CompareOperationElement::constraintBitExclusiveOrAssign(const VirtualOperation& operation, VirtualElement& source,
      const VirtualElement& resultElement, Argument argument, ConstraintEnvironment& env) {
   ZeroResult booleanResult = resultElement.queryZeroResult();
   if (booleanResult.isZero()) {
      IntersectEnvironment intersectionEnvironment(env, Argument().setResult());
      intersectWith(source, intersectionEnvironment);
      IntersectEnvironment sourceIntersectionEnvironment(env, Argument().setFst());
      source.intersectWith(*this, sourceIntersectionEnvironment);
      return true;
   };
   if (booleanResult.isDifferentZero()) {
      PPVirtualElement thisCopy(*this), sourceCopy(source);
      thisCopy = Methods::applyAssign(thisCopy, Scalar::MultiBit::BitNegateAssignOperation(), EvaluationApplyParameters(env));
      sourceCopy = Methods::applyAssign(sourceCopy, Scalar::MultiBit::BitNegateAssignOperation(), EvaluationApplyParameters(env));
      IntersectEnvironment intersectionEnvironment(env, Argument().setResult());
      intersectWith(*sourceCopy, intersectionEnvironment);
      IntersectEnvironment sourceIntersectionEnvironment(env, Argument().setFst());
      source.intersectWith(*thisCopy, sourceIntersectionEnvironment);
      return true;
   };
   env.mergeVerdictExact();
   return true;
}

bool
CompareOperationElement::queryCompareSpecial(const VirtualElement& source,
      const VirtualQueryOperation& operation, VirtualQueryOperation::Environment& aenv) {
   QueryOperation::CompareSpecialEnvironment& env = (QueryOperation::CompareSpecialEnvironment&) aenv;
   env.mergeBehaviour(QueryOperation::CompareSpecialEnvironment::BEqual0);
   env.mergeBehaviour(QueryOperation::CompareSpecialEnvironment::BEqual1);
   return true;
}

bool
CompareOperationElement::querySimplification(const VirtualElement& thisElement, const VirtualQueryOperation& operation, VirtualQueryOperation::Environment& aenv) {
   AssumeCondition(dynamic_cast<const CompareOperationElement*>(&thisElement) && dynamic_cast<const SimplificationEnvironment*>(&aenv))
   CompareOperationElement& thisCompare = const_cast<CompareOperationElement&>((const CompareOperationElement&) thisElement);
   SimplificationEnvironment& env = (SimplificationEnvironment&) aenv;
   Methods::BooleanResult compareResult = IntOperationElement::Methods::applyBoolean(
         thisCompare.fstArg(), thisCompare.getOperation(), thisCompare.sndArg());
   if (compareResult.isTrue()) {
      PPVirtualElement one = Approximate::Details::IntOperationElement::Methods::newOne(thisCompare.fstArg());
      EvaluationEnvironment envCopy = EvaluationEnvironment(EvaluationEnvironment::Init());
      one->apply(Scalar::MultiBit::ReduceOperation().setLowBit(0).setHighBit(0), envCopy);
      if (envCopy.hasResult())
         one = envCopy.presult();
      env.presult() = one;
   }
   else if (compareResult.isFalse()) {
      PPVirtualElement zero = Approximate::Details::IntOperationElement::Methods::newZero(thisCompare.fstArg());
      EvaluationEnvironment envCopy = EvaluationEnvironment(EvaluationEnvironment::Init());
      zero->apply(Scalar::MultiBit::ReduceOperation().setLowBit(0).setHighBit(0), envCopy);
      if (envCopy.hasResult())
         zero = envCopy.presult();
      env.presult() = zero;
   };
   return true;
}

bool
CompareOperationElement::querySimplificationAsConstantDisjunction(const VirtualElement& thisElement, const VirtualQueryOperation& operation, VirtualQueryOperation::Environment& aenv) {
   AssumeCondition(dynamic_cast<const SimplificationEnvironment*>(&aenv))
   AssumeCondition(dynamic_cast<const CompareOperationElement*>(&thisElement))
   CompareOperationElement& thisCompare = const_cast<CompareOperationElement&>((const CompareOperationElement&) thisElement);
   SimplificationEnvironment& env = (SimplificationEnvironment&) aenv;

   PNT::TPassPointer<VirtualDisjunction, PPAbstractElement> disjunction = Methods::newDisjunction(thisElement);
   PPVirtualElement one = Approximate::Details::IntOperationElement::Methods::newOne(thisCompare.fstArg());
   EvaluationEnvironment envCopy = EvaluationEnvironment(EvaluationEnvironment::Init());
   one->apply(Scalar::MultiBit::ReduceOperation().setLowBit(0).setHighBit(0), envCopy);
   if (envCopy.hasResult())
      one = envCopy.presult();
   PPVirtualElement zero = IntOperationElement::Methods::newZero(thisCompare.fstArg());
   envCopy.clear();
   zero->apply(Scalar::MultiBit::ReduceOperation().setLowBit(0).setHighBit(0), envCopy);
   if (envCopy.hasResult())
      zero = envCopy.presult();
   
   disjunction->addMay(zero);
   disjunction->addMay(one);
   env.presult() = disjunction;
   return true;
}

bool
CompareOperationElement::querySimplificationAsInterval(const VirtualElement& thisElement,
      const VirtualQueryOperation& operation, VirtualQueryOperation::Environment& aenv) {
   AssumeCondition(dynamic_cast<const SimplificationEnvironment*>(&aenv))
   AssumeCondition(dynamic_cast<const CompareOperationElement*>(&thisElement))
   CompareOperationElement& thisCompare = const_cast<CompareOperationElement&>((const CompareOperationElement&) thisElement);
   SimplificationEnvironment& env = (SimplificationEnvironment&) aenv;

   PPVirtualElement zero = Approximate::Details::IntOperationElement::Methods::newZero(thisCompare.fstArg());
   PPVirtualElement one = Approximate::Details::IntOperationElement::Methods::newOne(thisCompare.fstArg());
   EvaluationEnvironment envCopy = EvaluationEnvironment(EvaluationEnvironment::Init());
   one->apply(Scalar::MultiBit::ReduceOperation().setLowBit(0).setHighBit(0), envCopy);
   if (envCopy.hasResult())
      one = envCopy.presult();
   envCopy.clear();
   zero->apply(Scalar::MultiBit::ReduceOperation().setLowBit(0).setHighBit(0), envCopy);
   if (envCopy.hasResult())
      zero = envCopy.presult();
   env.presult().absorbElement(new Interval(Interval::Init().setInterval(zero, one)));
   return true;
}

bool
CompareOperationElement::queryDispatchOnTopResult(const VirtualElement& thisElement, const VirtualQueryOperation& operation, VirtualQueryOperation::Environment& aenv) {
   AssumeCondition(dynamic_cast<const CompareOperationElement*>(&thisElement))
   CompareOperationElement& thisCompare = const_cast<CompareOperationElement&>((const CompareOperationElement&) thisElement);
   PPVirtualElement top = Approximate::Details::IntOperationElement::Methods::newTop(thisCompare.fstArg());
   {  EvaluationEnvironment envCopy = EvaluationEnvironment(EvaluationEnvironment::Init());
      top->apply(Scalar::MultiBit::ReduceOperation().setLowBit(0).setHighBit(0), envCopy);
      if (envCopy.hasResult())
         top = envCopy.presult();
   };
   return top->query(operation, aenv);
}

bool
ExactToApproximateElement::applyCompare(const Operation& operation, EvaluationEnvironment& env) {
   if (env.getFirstArgument().getApproxKind().isVariable()) {
      AssumeCondition(dynamic_cast<const ExactToApproximateElement*>(&env.getFirstArgument()))
      if (isEqual((const ExactToApproximateElement&)env.getFirstArgument())) {
         switch (operation.getType()) {
            case Operation::TCompareLessSigned:
            case Operation::TCompareLessUnsigned:
            case Operation::TCompareLessFloat:
            case Operation::TCompareDifferentFloat:
            case Operation::TCompareDifferent:
            case Operation::TCompareGreaterFloat:
            case Operation::TCompareGreaterUnsigned:
            case Operation::TCompareGreaterSigned:
               env.presult() = Approximate::Details::IntOperationElement::Methods::newFalse(domain());
               return true;
            case Operation::TCompareLessOrEqualSigned:
            case Operation::TCompareLessOrEqualUnsigned:
            case Operation::TCompareLessOrEqualFloat:
            case Operation::TCompareEqual:
            case Operation::TCompareEqualFloat:
            case Operation::TCompareGreaterOrEqualFloat:
            case Operation::TCompareGreaterOrEqualUnsigned:
            case Operation::TCompareGreaterOrEqualSigned:
               env.presult() = Approximate::Details::IntOperationElement::Methods::newTrue(domain());
               return true;
            default:
               break;
         };
      }
   }
   TransmitEnvironment transmitEnv(env);
   transmitEnv.setFirstArgument(env.getFirstArgument());
   bool result = domain().apply(operation, transmitEnv);
   if (!result || !transmitEnv.hasResult())
      return false;
   ZeroResult comparisonResult = transmitEnv.presult()->queryZeroResult();
   if (comparisonResult.mayBeTrue() && comparisonResult.mayBeFalse()) {
      transmitEnv.presult().release();
      transmitEnv.clearMask();
      typedef Bit::Approximate::CompareOperationElement CompareOperationElement;
      auto* comparisonResult = new CompareOperationElement((const CompareOperationElement::Init&)
            CompareOperationElement::Init().set1(), operation.getType());
      env.presult().absorbElement(comparisonResult);
      comparisonResult->sfstArg().absorbElement(createSCopy());
      comparisonResult->ssndArg().setElement(env.getFirstArgument());
   }
   else
      env.presult() = transmitEnv.presult();
   return true;
}

bool
ExactToApproximateElement::applyToCompare(const Operation& operation, VirtualElement& argument, EvaluationEnvironment& env) const {
   TransmitEnvironment transmitEnv(env);
   transmitEnv.setFirstArgument(domain());
   bool result = argument.apply(operation, transmitEnv);
   if (!result || !transmitEnv.hasResult())
      return false;
   ZeroResult comparisonResult = transmitEnv.presult()->queryZeroResult();
   if (comparisonResult.mayBeTrue() && comparisonResult.mayBeFalse()) {
      transmitEnv.presult().release();
      transmitEnv.clearMask();
      typedef Bit::Approximate::CompareOperationElement CompareOperationElement;
      auto* comparisonResult = new CompareOperationElement((const CompareOperationElement::Init&)
            CompareOperationElement::Init().set1(), operation.getType());
      env.presult().absorbElement(comparisonResult);
      comparisonResult->sfstArg().setElement(env.getFirstArgument());
      comparisonResult->ssndArg().absorbElement(createSCopy());
   }
   else
      env.presult() = transmitEnv.presult();
   return true;
}

bool
ExactToApproximateElement::applyExtend(const Operation& aoperation, EvaluationEnvironment& env) {
   const Scalar::MultiBit::Operation& operation = (const Scalar::MultiBit::Operation&) aoperation;
   ExtendOperationElement* result;
   {  ExtendOperationElement::Init init;
      AssumeCondition(dynamic_cast<const Scalar::MultiBit::ExtendOperation*>(&operation))
      int extension = ((const Scalar::MultiBit::ExtendOperation&) operation).getExtension();
      init.setBitSize(getSizeInBits() + extension);
      if (operation.getType() == Scalar::MultiBit::Operation::TExtendWithZero)
         init.setUnsignedExtension(extension);
      else {
         AssumeCondition(operation.getType() == Scalar::MultiBit::Operation::TExtendWithSign)
            init.setSignedExtension(extension);
      }
      result = new ExtendOperationElement(init);
   };
   env.presult().absorbElement(result);
   result->sfstArg().absorbElement(createSCopy());
   return true;
}

bool
ExactToApproximateElement::applyConcat(const Operation& aoperation, EvaluationEnvironment& env) {
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
ExactToApproximateElement::applyReduce(const Operation& aoperation, EvaluationEnvironment& env) {
   const Scalar::MultiBit::ReduceOperation& operation = (const Scalar::MultiBit::ReduceOperation&) aoperation;
   if ((operation.getLowBit() == operation.getHighBit())
         && (operation.getLowBit() == domain().getSizeInBits() - 1) && operation.getLowBit() > 2) {
      typedef Approximate::Details::IntOperationElement::QueryOperation QueryOperation;
      QueryOperation::DomainEnvironment zeroEnvironment;
      zeroEnvironment.setInit(VirtualElement::Init().setBitSize(operation.getLowBit()+1));
      domain().query(QueryOperation().setNewZero(), zeroEnvironment);
      PPVirtualElement zero = zeroEnvironment.presult();
      TransmitEnvironment applyEnv(env);
      applyEnv.setFirstArgument(*zero);
      assume(applyCompare(Scalar::MultiBit::Operation().setCompareLessSigned(), applyEnv));
      env.presult() = Methods::apply((const VirtualElement&) *applyEnv.presult(), Scalar::Bit::CastMultiBitOperation().setSize(1), EvaluationParameters(env));
      return true;
   };
   if (operation.getLowBit() == 0 && operation.getHighBit() == domain().getSizeInBits() - 1) {
      if (operation.isConst())
         env.presult().setElement(*this);
      return true;
   };
   return false;
}

bool
ExactToApproximateElement::applyMinusAssign(const Operation& aoperation, EvaluationEnvironment& env) {
   typedef Scalar::MultiBit::Operation Operation;
   if (env.getFirstArgument().getApproxKind().isVariable()) {
      AssumeCondition(dynamic_cast<const ExactToApproximateElement*>(&env.getFirstArgument()))
      if (isEqual((const ExactToApproximateElement&) env.getFirstArgument())) {
         switch (((const Operation&) aoperation).getType()) {
            case Operation::TCompareDifferentFloat:
            case Operation::TCompareDifferent:
               env.presult() = Approximate::Details::IntOperationElement::Methods::newFalse(domain());
               return true;
            case Operation::TCompareEqual:
            case Operation::TCompareEqualFloat:
               env.presult() = Approximate::Details::IntOperationElement::Methods::newTrue(domain());
               return true;
            default:
               break;
         };
      }
   }
   MinusOperationElement* minusResult = new MinusOperationElement((const MinusOperationElement::Init&)
         MinusOperationElement::Init().setBitSize(getSizeInBits()));
   env.presult().absorbElement(minusResult);
   minusResult->setOperation(aoperation);
   minusResult->sfstArg().absorbElement(createSCopy());
   minusResult->ssndArg().setElement(env.getFirstArgument());
   return true;
}

bool
ExactToApproximateElement::apply(const VirtualOperation& aoperation, Scalar::EvaluationEnvironment& aenv) {
   EvaluationEnvironment& env = (EvaluationEnvironment&) aenv;
   if (VirtualElement::apply(aoperation, aenv))
      return true;
   if (!env.getLatticeCreation().isTop()) {
      const Operation& operation = (const Operation&) aoperation;
      Operation::Type type = operation.getType();
      if (type >= Operation::StartOfCompare && type < Operation::EndOfCompare)
         return applyCompare(operation, env);
      if (type == Operation::TExtendWithZero || type == Operation::TExtendWithSign)
         return applyExtend(operation, env);
      if (type == Operation::TConcat)
         return applyConcat(operation, env);
      if (type == Operation::TMinusSignedAssign || type == Operation::TMinusUnsignedAssign)
         return applyMinusAssign(operation, env);
      if (type == Operation::TReduce) {
         if (applyReduce(operation, env))
            return true;
      };
   };

   return inherited::apply(aoperation, aenv);
}

bool
ExactToApproximateElement::applyTo(const VirtualOperation& aoperation, Approximate::VirtualElement& argument,
      EvaluationEnvironment& env) const {
   if (!env.getLatticeCreation().isTop()) {
      const Operation& operation = (const Operation&) aoperation;
      // AssumeCondition(getType() > argument.getType())
      Operation::Type type = operation.getType();
      if (type >= Operation::StartOfCompare && type < Operation::EndOfCompare)
         return applyToCompare(operation, argument, env);
   };
   return inherited::applyTo(aoperation, argument, env);
}

}} // end of namespace MultiBit::Approximate

}} // end of namespace Analyzer::Interpretation::Scalar

