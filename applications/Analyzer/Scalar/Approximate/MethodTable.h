/////////////////////////////////
//
// Library   : Analyzer/Scalar
// Unit      : approximate scalar elements
// File      : MethodTable.h
// Author    : Franck Vedrine
// Copyright : CEA LIST - 2019-2020 - all rights reserved
//
// Description :
//   Definition of virtual method tables for approximate scalar elements.
//

#ifndef Analyzer_Scalar_Approximate_MethodTableH
#define Analyzer_Scalar_Approximate_MethodTableH

#include "Analyzer/Scalar/Approximate/VirtualElement.h"

namespace Analyzer { namespace Scalar { namespace Approximate {

namespace Details {

class BaseConstants {
  public:
   static const int NBIntersectToMethods = 0;
   static const int NBIntersectMethods = 0;
   static const int NBApplyMethods = 0;
   static const int NBApplyToMethods = 0;
   static const int NBConstraintCompareMethods = 0;
   static const int NBBinaryConstraintMethods = 0;
   static const int NBUnaryConstraintMethods = 0;
};

template <class TypeDomain>
class TDomainTraits {
  public:
   typedef TypeDomain Domain;
   typedef typename TypeDomain::Constants Constants;
   static const int NBConstantsIntersectToMethods = Constants::NBIntersectToMethods;
   static const int NBConstantsIntersectMethods = Constants::NBIntersectMethods;
   static const int NBApplyMethods = Constants::NBApplyMethods;
   static const int NBApplyToMethods = Constants::NBApplyToMethods;
   static const int NBConstraintCompareMethods = Constants::NBConstraintCompareMethods;
   static const int NBBinaryConstraintMethods = Constants::NBBinaryConstraintMethods;
   static const int NBUnaryConstraintMethods = Constants::NBUnaryConstraintMethods;

   typedef typename TypeDomain::Operation Operation;
   typedef bool (TypeDomain::*PointerMergeIntersectToMethod)(VirtualElement&, EvaluationEnvironment&) const;
   typedef bool (TypeDomain::*PointerMergeIntersectMethod)(const VirtualElement&, EvaluationEnvironment&);
   typedef bool (TypeDomain::*PointerApplyMethod)(const VirtualOperation&, EvaluationEnvironment&);
   typedef bool (TypeDomain::*PointerApplyToMethod)(const VirtualOperation&, VirtualElement&, EvaluationEnvironment&) const;
   typedef bool (TypeDomain::*PointerConstraintCompareMethod)
      (const VirtualOperation&, VirtualElement&, ConstraintEnvironment::Argument, ConstraintEnvironment&);
   typedef bool (TypeDomain::*PointerConstraintBinaryMethod)
      (const VirtualOperation&, VirtualElement&, const VirtualElement&, ConstraintEnvironment::Argument, ConstraintEnvironment&);
   typedef bool (*PointerConstraintBinaryGenericFunction)
      (VirtualElement& element, const VirtualOperation&, VirtualElement&, const VirtualElement&, ConstraintEnvironment&);
   typedef bool (TypeDomain::*PointerConstraintBinaryArgMethod)
      (const VirtualOperation&, VirtualElement&, const VirtualElement&, ConstraintEnvironment&);
   typedef bool (TypeDomain::*PointerConstraintUnaryGenericMethod)
      (const VirtualOperation& operation, const VirtualElement&, ConstraintEnvironment&);
   typedef bool (*PointerConstraintUnaryGenericFunction)
      (VirtualElement&, const VirtualOperation& operation, const VirtualElement&, ConstraintEnvironment&);
   typedef bool (TypeDomain::*PointerQueryMethod)(const typename TypeDomain::QueryOperation&, typename TypeDomain::QueryOperation::Environment&) const;
};

template <class TypeDomainTraits>
class MethodMergeIntersectToTable;

template <class TypeDomainTraits>
class MethodMergeIntersectToNode : public PNT::DecisionNode<typename TypeDomainTraits::PointerMergeIntersectToMethod> {
  private:
   typedef PNT::DecisionNode<typename TypeDomainTraits::PointerMergeIntersectToMethod> inherited;
   typedef typename TypeDomainTraits::PointerMergeIntersectToMethod PointerMergeIntersectToMethod;
   
  protected:
   friend class MethodMergeIntersectToTable<TypeDomainTraits>;

  public:
   MethodMergeIntersectToNode() {}
   MethodMergeIntersectToNode(PointerMergeIntersectToMethod method) : inherited(method) {}
   void setMethod(PointerMergeIntersectToMethod method) { inherited::setMethod(method); }
   void resetMethod(PointerMergeIntersectToMethod method) { inherited::resetMethod(method); }

   bool execute(const typename TypeDomainTraits::Domain& object, VirtualElement& source, EvaluationEnvironment& env) const
      {  return (inherited::hasMethod()) ? (object.*inherited::getMethod())(source, env) : false; }
};

template <class TypeDomainTraits>
class MethodMergeIntersectToTable
   : public PNT::ConstMethodTable<MethodMergeIntersectToNode<TypeDomainTraits>,
         TypeDomainTraits::NBConstantsIntersectToMethods> {
  private:
   typedef PNT::ConstMethodTable<MethodMergeIntersectToNode<TypeDomainTraits>,
         TypeDomainTraits::NBConstantsIntersectToMethods> inherited;

  public:
   MethodMergeIntersectToTable() {}
   MethodMergeIntersectToTable(const MethodMergeIntersectToTable<TypeDomainTraits>& table) = default;

   const MethodMergeIntersectToNode<TypeDomainTraits>& operator[](int index) const { return inherited::operator[](index); }
   MethodMergeIntersectToNode<TypeDomainTraits>& operator[](int index) { return inherited::elementAt(index); }
   MethodMergeIntersectToNode<TypeDomainTraits>& elementAt(int index) { return inherited::elementAt(index); }
   TemplateDefineCopy(MethodMergeIntersectToTable, TypeDomainTraits)
};

template <class TypeDomainTraits>
class MethodMergeIntersectTable;

template <class TypeDomainTraits>
class MethodMergeIntersectNode : public PNT::DecisionNode<typename TypeDomainTraits::PointerMergeIntersectMethod> {
  private:
   typedef PNT::DecisionNode<typename TypeDomainTraits::PointerMergeIntersectMethod> inherited;
   typedef typename TypeDomainTraits::PointerMergeIntersectMethod PointerMergeIntersectMethod;
   
  protected:
   friend class MethodMergeIntersectTable<TypeDomainTraits>;

  public:
   MethodMergeIntersectNode() {}
   MethodMergeIntersectNode(PointerMergeIntersectMethod method) : inherited(method) {}

   bool execute(typename TypeDomainTraits::Domain& object, const VirtualElement& source, EvaluationEnvironment& env) const
      {  return (inherited::hasMethod()) ? (object.*inherited::getMethod())(source, env) : false; }
};

template <class TypeDomainTraits>
class MethodMergeIntersectTable : public PNT::ConstMethodTable<MethodMergeIntersectNode<TypeDomainTraits>,
      TypeDomainTraits::NBConstantsIntersectMethods> {
  private:
   typedef PNT::ConstMethodTable<MethodMergeIntersectNode<TypeDomainTraits>,
         TypeDomainTraits::NBConstantsIntersectMethods> inherited;

  public:
   MethodMergeIntersectTable() {}
   MethodMergeIntersectTable(const MethodMergeIntersectTable& table) = default;
   DefineCopy(MethodMergeIntersectTable)
   
   void clear() { inherited::clear(); }
};

/*****************************************/
/* Definition of the apply method tables */
/*****************************************/

template <class TypeDomainTraits>
class MultiApplyMethod : public EnhancedObject {
  private:
   typedef EnhancedObject inherited;

  public:
   MultiApplyMethod() {}
   MultiApplyMethod(const MultiApplyMethod<TypeDomainTraits>& source) = default;
   TemplateDefineCopy(MultiApplyMethod, TypeDomainTraits)

   virtual bool execute(typename TypeDomainTraits::Domain& object, const VirtualOperation& operation, EvaluationEnvironment& env) const
      {  AssumeUncalled return false; }
};

template <class TypeDomainTraits>
class MethodApplyTable;

template <class TypeDomainTraits>
class MethodApplyDecisionNode : public PNT::DecisionNode<typename TypeDomainTraits::PointerApplyMethod> {
  private:
   typedef PNT::DecisionNode<typename TypeDomainTraits::PointerApplyMethod> inherited;
   typedef typename TypeDomainTraits::PointerApplyMethod PointerApplyMethod;
     
  protected:
   friend class MethodApplyTable<TypeDomainTraits>;

  public:
   MethodApplyDecisionNode() {}
   MethodApplyDecisionNode(PointerApplyMethod method) : inherited(method) {}

   void setMethod(PointerApplyMethod method) { inherited::setMethod(method); }
   void resetMethod(PointerApplyMethod method) { inherited::resetMethod(method); }
   void setArray(MultiApplyMethod<TypeDomainTraits>* newArray) { inherited::setArray(newArray); }
   void resetArray(MultiApplyMethod<TypeDomainTraits>* newArray) { inherited::resetArray(newArray); }
   bool execute(typename TypeDomainTraits::Domain& object, const VirtualOperation& operation, EvaluationEnvironment& env) const
      {  return (inherited::hasMethod()) ? (object.*inherited::getMethod())(operation, env)
            : (inherited::hasArray() ? ((const MultiApplyMethod<TypeDomainTraits>&) inherited::getArray()).execute(object, operation, env)
            : false);
      }
};

template <class TypeDomainTraits>
class MethodApplyTable : public PNT::ConstMethodTable<MethodApplyDecisionNode<TypeDomainTraits>,
      TypeDomainTraits::NBApplyMethods> {
  private:
   typedef MethodApplyDecisionNode<TypeDomainTraits> DecisionNode;
   typedef PNT::ConstMethodTable<DecisionNode, TypeDomainTraits::NBApplyMethods> inherited;

  public:
   DecisionNode& elementAt(typename TypeDomainTraits::Operation::Type type)
      {  return inherited::elementAt(TypeDomainTraits::Constants::convertApplyOperationToIndex(type)); }
   const DecisionNode& operator[](typename TypeDomainTraits::Operation::Type type) const
      {  return const_cast<MethodApplyTable&>(*this).elementAt(type); }

  public:
   MethodApplyTable() {}
   MethodApplyTable(const MethodApplyTable<TypeDomainTraits>& source) =  default;
   TemplateDefineCopy(MethodApplyTable, TypeDomainTraits)
};

/*******************************************/
/* Definition of the applyTo method tables */
/*******************************************/

template <class TypeDomainTraits>
class MultiApplyToMethod : public EnhancedObject {
  private:
   typedef EnhancedObject inherited;

  public:
   MultiApplyToMethod() {}
   MultiApplyToMethod(const MultiApplyToMethod<TypeDomainTraits>& source) = default;
   TemplateDefineCopy(MultiApplyToMethod, TypeDomainTraits)

   virtual bool execute(const typename TypeDomainTraits::Domain& object, const VirtualOperation& operation,
         VirtualElement& argument, EvaluationEnvironment& env) const
      {  AssumeUncalled return false; }
};

template <class TypeDomainTraits>
class MethodApplyToTable;

template <class TypeDomainTraits>
class MethodApplyToDecisionNode : public PNT::DecisionNode<typename TypeDomainTraits::PointerApplyToMethod> {
  private:
   typedef PNT::DecisionNode<typename TypeDomainTraits::PointerApplyToMethod> inherited;
   typedef typename TypeDomainTraits::PointerApplyToMethod PointerApplyToMethod;

  protected:
   friend class MethodApplyToTable<TypeDomainTraits>;

  public:
   MethodApplyToDecisionNode() {}
   MethodApplyToDecisionNode(PointerApplyToMethod method) : inherited(method) {}
   void setArray(MultiApplyToMethod<TypeDomainTraits>* newArray) { inherited::setArray(newArray); }
   void resetArray(MultiApplyToMethod<TypeDomainTraits>* newArray) { inherited::resetArray(newArray); }
   void setMethod(typename TypeDomainTraits::PointerApplyToMethod method) { inherited::setMethod(method); }
   void resetMethod(typename TypeDomainTraits::PointerApplyToMethod method) { inherited::resetMethod(method); }
   bool execute(const typename TypeDomainTraits::Domain& object, const VirtualOperation& operation,
         VirtualElement& argument, EvaluationEnvironment& env) const
      {  return inherited::hasMethod() ? (object.*inherited::getMethod())(operation, argument, env) : (inherited::hasArray()
            ? ((const MultiApplyToMethod<TypeDomainTraits>&) inherited::getArray()).execute(object, operation, argument, env)
            : false);
      }
};

template <class TypeDomainTraits>
class MethodApplyToTable : public PNT::ConstMethodTable<MethodApplyToDecisionNode<TypeDomainTraits>,
      TypeDomainTraits::NBApplyToMethods> {
  private:
   typedef MethodApplyToDecisionNode<TypeDomainTraits> DecisionNode;
   typedef PNT::ConstMethodTable<DecisionNode, TypeDomainTraits::NBApplyToMethods> inherited;

  protected:
   DecisionNode& elementAt(typename TypeDomainTraits::Operation::Type type)
      {  return inherited::elementAt(TypeDomainTraits::Constants::convertApplyToOperationToIndex(type)); }
  
  public:
   MethodApplyToTable() {}
   MethodApplyToTable(const MethodApplyToTable<TypeDomainTraits>& table) : inherited(table) {}
   TemplateDefineCopy(MethodApplyToTable, TypeDomainTraits)

   const DecisionNode& operator[](typename TypeDomainTraits::Operation::Type type) const
      { return const_cast<MethodApplyToTable&>(*this).elementAt(type); }
};

/**********************************************/
/* Definition of the constraint method tables */
/**********************************************/

template <class TypeDomainTraits>
class MultiConstraintCompareMethod : public EnhancedObject {
  private:
   typedef EnhancedObject inherited;

  public:
   MultiConstraintCompareMethod() {}
   MultiConstraintCompareMethod(const MultiConstraintCompareMethod<TypeDomainTraits>& source) = default;
   TemplateDefineCopy(MultiConstraintCompareMethod, TypeDomainTraits)

   virtual bool execute(typename TypeDomainTraits::Domain& object, const VirtualOperation& operation,
         VirtualElement& source, ConstraintEnvironment::Argument arg, ConstraintEnvironment& env) const
      {  AssumeUncalled return false; }
};

template <class TypeDomainTraits>
class MethodConstraintTable;

template <class TypeDomainTraits>
class MethodConstraintCompareDecisionNode : public PNT::DecisionNode<typename TypeDomainTraits::PointerConstraintCompareMethod> {
  private:
   typedef PNT::DecisionNode<typename TypeDomainTraits::PointerConstraintCompareMethod> inherited;
   typedef typename TypeDomainTraits::PointerConstraintCompareMethod PointerConstraintCompareMethod;

  protected:
   friend class MethodConstraintTable<TypeDomainTraits>;

  public:
   MethodConstraintCompareDecisionNode() {}
   void setArray(MultiConstraintCompareMethod<TypeDomainTraits>* newArray) { inherited::setArray(newArray); }
   void resetArray(MultiConstraintCompareMethod<TypeDomainTraits>* newArray) { inherited::resetArray(newArray); }

   void setMethod(PointerConstraintCompareMethod method) { inherited::setMethod(method); }
   void resetMethod(PointerConstraintCompareMethod method) { inherited::resetMethod(method); }
   bool execute(typename TypeDomainTraits::Domain& object, const VirtualOperation& operation,
         VirtualElement& source, ConstraintEnvironment::Argument arg, ConstraintEnvironment& env) const
      {  return inherited::hasMethod() ? (object.*inherited::getMethod())(operation, source, arg, env) : (inherited::hasArray()
            ? ((const MultiConstraintCompareMethod<TypeDomainTraits>&) inherited::getArray()).execute(object, operation, source, arg, env)
            : false);
      }
};

template <class TypeDomainTraits>
class MethodConstraintCompareTable
      : public PNT::ConstMethodTable<MethodConstraintCompareDecisionNode<TypeDomainTraits>,
            TypeDomainTraits::NBConstraintCompareMethods> {
  private:
   typedef MethodConstraintCompareDecisionNode<TypeDomainTraits> DecisionNode;
   typedef PNT::ConstMethodTable<DecisionNode, TypeDomainTraits::NBConstraintCompareMethods> inherited;

  protected:
   friend class MethodConstraintTable<TypeDomainTraits>;

  public:
   const DecisionNode& operator[](typename TypeDomainTraits::Operation::Type type) const
      {  return const_cast<MethodConstraintCompareTable&>(*this).elementAt(type); }
   DecisionNode& elementAt(typename TypeDomainTraits::Operation::Type type)
      {  return inherited::elementAt(TypeDomainTraits::Constants::convertCompareConstraintOperationToIndex(type)); }
   DecisionNode& operator[](typename TypeDomainTraits::Operation::Type type)
      {  return inherited::elementAt(TypeDomainTraits::Constants::convertCompareConstraintOperationToIndex(type)); }

  public:
   MethodConstraintCompareTable() { inherited::clear(); }
   MethodConstraintCompareTable(const MethodConstraintCompareTable<TypeDomainTraits>& table) = default;
   TemplateDefineCopy(MethodConstraintCompareTable, TypeDomainTraits)
};

/* Binary method tables */

template <class TypeDomainTraits>
class MultiConstraintBinaryMethod : public EnhancedObject {
  private:
   typedef EnhancedObject inherited;

  public:
   MultiConstraintBinaryMethod() {}
   MultiConstraintBinaryMethod(const MultiConstraintBinaryMethod<TypeDomainTraits>& source) = default;
   TemplateDefineCopy(MultiConstraintBinaryMethod, TypeDomainTraits)

   virtual bool execute(typename TypeDomainTraits::Domain& object, const VirtualOperation& operation, VirtualElement& source,
         const VirtualElement& result, ConstraintEnvironment::Argument arg, ConstraintEnvironment& env) const
      {  AssumeUncalled return false; }
};

template <class TypeDomainTraits>
class MethodConstraintBinaryDecisionNode : public EnhancedObject,
      public PNT::DecisionNode<typename TypeDomainTraits::PointerConstraintBinaryMethod> {
  private:
   typedef EnhancedObject inherited;
   typedef PNT::DecisionNode<typename TypeDomainTraits::PointerConstraintBinaryMethod> DecisionNode;
   typedef MethodConstraintBinaryDecisionNode<TypeDomainTraits> thisType;
   
  protected:
   friend class MethodConstraintTable<TypeDomainTraits>;

   TemplateDefineExtendedParameters(1, DecisionNode)
   typename TypeDomainTraits::PointerConstraintBinaryGenericFunction getFunction() const
      {  typename TypeDomainTraits::PointerConstraintBinaryMethod method = DecisionNode::getMethod();
         typename TypeDomainTraits::PointerConstraintBinaryGenericFunction result = nullptr;
         memcpy(&result, &method, sizeof(result));
         return result;
         // return *((typename TypeDomainTraits::PointerConstraintBinaryGenericFunction*) &DecisionNode::getMethod());
      }
   bool isFunction() const { return hasOwnField(); }
   
   bool execute(typename TypeDomainTraits::Domain& object, const VirtualOperation& operation,
         const VirtualElement& result, ConstraintEnvironment& env) const
      {  return DecisionNode::hasMethod()
            ? (isFunction() ? (*getFunction())(object, operation, result, env)
            : (object.*DecisionNode::getMethod())(operation, result, env))
            : false;
      }
      
  public:
   MethodConstraintBinaryDecisionNode() {}
   MethodConstraintBinaryDecisionNode(typename TypeDomainTraits::PointerConstraintBinaryMethod method)
      :  DecisionNode(method) {}
   MethodConstraintBinaryDecisionNode(const thisType& source) = default;
   thisType& operator=(const thisType& source) = default;
   TemplateDefineCopy(MethodConstraintBinaryDecisionNode, TypeDomainTraits)
   DTemplateDefineAssign(MethodConstraintBinaryDecisionNode, TypeDomainTraits)

   void setArray(MultiConstraintBinaryMethod<TypeDomainTraits>* multiMethod) { DecisionNode::setArray(multiMethod); }
   void resetArray(MultiConstraintBinaryMethod<TypeDomainTraits>* multiMethod) { DecisionNode::resetArray(multiMethod); }
   void setMethod(typename TypeDomainTraits::PointerConstraintBinaryMethod method) { DecisionNode::setMethod(method); }
   void resetMethod(typename TypeDomainTraits::PointerConstraintBinaryMethod method) { DecisionNode::resetMethod(method); }
   void setFunction(typename TypeDomainTraits::PointerConstraintBinaryGenericFunction function)
      {  typename TypeDomainTraits::PointerConstraintBinaryMethod method = nullptr;
         memcpy(&method, &function, sizeof(function));
         // *((typename TypeDomainTraits::PointerConstraintBinaryGenericFunction*) &method) = function;
         DecisionNode::setMethod(method);
         mergeOwnField(1);
      }
   
   bool execute(typename TypeDomainTraits::Domain& object, const VirtualOperation& operation,
         VirtualElement& source, const VirtualElement& result,
         ConstraintEnvironment::Argument arg, ConstraintEnvironment& env) const
      {  return DecisionNode::hasMethod()
            ? (isFunction() ? (arg.isResult()
               ? (*getFunction())(object, operation, source, result, env)
               : (*getFunction())(source, operation, object, result, env))
               : (object.*DecisionNode::getMethod())(operation, source, result, arg, env))
            : (DecisionNode::hasArray()
               ? ((const MultiConstraintBinaryMethod<TypeDomainTraits>&) DecisionNode::getArray())
                     .execute(object, operation, source, result, arg, env)
            : false);
      }
};

template <class TypeDomainTraits>
class GenericBinaryConstraintMethod : public MultiConstraintBinaryMethod<TypeDomainTraits> {
  private:
   typedef MultiConstraintBinaryMethod<TypeDomainTraits> inherited;
   typedef GenericBinaryConstraintMethod<TypeDomainTraits> thisType;
   typename TypeDomainTraits::PointerConstraintBinaryGenericFunction pcbmGenericMethod;

  public:
   GenericBinaryConstraintMethod(typename TypeDomainTraits::PointerConstraintBinaryGenericFunction genericMethod)
      : pcbmGenericMethod(genericMethod) {}
   GenericBinaryConstraintMethod(const thisType& source) = default;
   thisType& operator=(const thisType& source) = default;
   TemplateDefineCopy(GenericBinaryConstraintMethod, TypeDomainTraits)
   DTemplateDefineAssign(GenericBinaryConstraintMethod, TypeDomainTraits)

   virtual bool execute(typename TypeDomainTraits::Domain& object, const VirtualOperation& operation,
         VirtualElement& source, const VirtualElement& result,
         ConstraintEnvironment::Argument arg, ConstraintEnvironment& env) const override
      {  return (arg.isResult())
            ? (*pcbmGenericMethod)(object, operation, source, result, env)
            : (*pcbmGenericMethod)(source, operation, object, result, env);
      }
};

template <class TypeDomainTraits>
class GenericAndConstantResultBinaryConstraintMethod : public MultiConstraintBinaryMethod<TypeDomainTraits> {
  private:
   typedef MultiConstraintBinaryMethod<TypeDomainTraits> inherited;
   typedef GenericAndConstantResultBinaryConstraintMethod<TypeDomainTraits> thisType;
   typename TypeDomainTraits::PointerConstraintBinaryGenericFunction pcbmGenericMethod;
   typename TypeDomainTraits::PointerConstraintBinaryMethod pcbmConstantResultMethod;

  public:
   GenericAndConstantResultBinaryConstraintMethod(
         typename TypeDomainTraits::PointerConstraintBinaryGenericFunction genericMethod,
         typename TypeDomainTraits::PointerConstraintBinaryMethod constantResultMethod)
      :  pcbmGenericMethod(genericMethod), pcbmConstantResultMethod(constantResultMethod) {}
   GenericAndConstantResultBinaryConstraintMethod(const thisType& source) = default;
   thisType& operator=(const thisType& source) = default;
   TemplateDefineCopy(GenericAndConstantResultBinaryConstraintMethod, TypeDomainTraits)
   DTemplateDefineAssign(GenericAndConstantResultBinaryConstraintMethod, TypeDomainTraits)

   virtual bool execute(typename TypeDomainTraits::Domain& object, const VirtualOperation& operation,
         VirtualElement& source, const VirtualElement& result,
         ConstraintEnvironment::Argument arg, ConstraintEnvironment& env) const override
      {  bool executionResult = false;
         if (result.getApproxKind().isConstant()) {
            // if (arg.isResult())
               executionResult = (object.*pcbmConstantResultMethod)(operation, source, result, arg, env);
            /* else {
               AssumeCondition(dynamic_cast<const typename TypeDomainTraits::Domain*>(&source))
               executionResult = (((typename TypeDomainTraits::Domain&) source).*pcbmConstantResultMethod)(operation, object, result, arg, env);
            }; */
         }
         else {
            if (arg.isResult())
               executionResult = (*pcbmGenericMethod)(object, operation, source, result, env);
            else
               executionResult = (*pcbmGenericMethod)(source, operation, object, result, env);
         };
         return executionResult;
      }
/*    {  return result.getApproxKind().isConstant() ? (arg.isResult()
            ? (*pcbmGenericMethod)(object, operation, source, result, env)
            : (*pcbmGenericMethod)(source, operation, object, result, env))
            : (object.*pcbmConstantResultMethod)(operation, source, result, arg, env);
      } */
};

template <class TypeDomainTraits>
class GenericAndConstantResultSourceBinaryConstraintMethod : public MultiConstraintBinaryMethod<TypeDomainTraits> {
  private:
   typedef MultiConstraintBinaryMethod<TypeDomainTraits> inherited;
   typedef GenericAndConstantResultSourceBinaryConstraintMethod<TypeDomainTraits> thisType;
   typename TypeDomainTraits::PointerConstraintBinaryGenericFunction pcbmGenericMethod;
   typename TypeDomainTraits::PointerConstraintBinaryArgMethod pcbmConstantResultArgResultMethod;
   typename TypeDomainTraits::PointerConstraintBinaryArgMethod pcbmConstantResultArgSourceMethod;

  public:
   GenericAndConstantResultSourceBinaryConstraintMethod(
      typename TypeDomainTraits::PointerConstraintBinaryGenericFunction genericMethod,
      typename TypeDomainTraits::PointerConstraintBinaryArgMethod constantResultArgResultMethod,
      typename TypeDomainTraits::PointerConstraintBinaryArgMethod constantResultArgSourceMethod)
      :  pcbmGenericMethod(genericMethod),
         pcbmConstantResultArgResultMethod(constantResultArgResultMethod),
         pcbmConstantResultArgSourceMethod(constantResultArgSourceMethod) {}
   GenericAndConstantResultSourceBinaryConstraintMethod(const thisType& source) = default;
   thisType& operator=(const thisType& source) = default;
   TemplateDefineCopy(GenericAndConstantResultSourceBinaryConstraintMethod, TypeDomainTraits)
   DTemplateDefineAssign(GenericAndConstantResultSourceBinaryConstraintMethod, TypeDomainTraits)

   virtual bool execute(typename TypeDomainTraits::Domain& object, const VirtualOperation& operation,
         VirtualElement& source, const VirtualElement& result,
         ConstraintEnvironment::Argument arg, ConstraintEnvironment& env) const override
      {  return (!result.getApproxKind().isConstant()) ? ((arg.isResult())
               ? (*pcbmGenericMethod)(object, operation, source, result, env)
               : (*pcbmGenericMethod)(source, operation, object, result, env))
            : ((arg.isResult())
               ? (object.*pcbmConstantResultArgResultMethod)(operation, source, result, env)
               : (object.*pcbmConstantResultArgSourceMethod)(operation, source, result, env));
      }
};

template <class TypeDomainTraits>
class MethodConstraintBinaryTable
   :  public PNT::ConstMethodTable<MethodConstraintBinaryDecisionNode<TypeDomainTraits>,
         TypeDomainTraits::NBBinaryConstraintMethods> {
  private:
   typedef MethodConstraintBinaryDecisionNode<TypeDomainTraits> DecisionNode;
   typedef PNT::ConstMethodTable<DecisionNode, TypeDomainTraits::NBBinaryConstraintMethods> inherited;
   typedef MethodConstraintBinaryTable<TypeDomainTraits> thisType;

  public:
   const DecisionNode& operator[](typename TypeDomainTraits::Operation::Type type) const
      {  return const_cast<thisType&>(*this).elementAt(type); }
   DecisionNode& elementAt(typename TypeDomainTraits::Operation::Type type)
      {  return inherited::elementAt(TypeDomainTraits::Constants::convertBinaryConstraintOperationToIndex(type)); }
   DecisionNode& operator[](typename TypeDomainTraits::Operation::Type type)
      {  return inherited::elementAt(TypeDomainTraits::Constants::convertBinaryConstraintOperationToIndex(type)); }

  public:
   MethodConstraintBinaryTable() { inherited::clear(); }
   MethodConstraintBinaryTable(const thisType& source) = default;
   TemplateDefineCopy(MethodConstraintBinaryTable, TypeDomainTraits)
};

// definition of the unary constraints

template <class TypeDomainTraits>
class MethodConstraintUnaryTable;

template <class TypeDomainTraits>
class MethodConstraintUnaryDecisionNode
   :  public PNT::DecisionNode<typename TypeDomainTraits::PointerConstraintUnaryGenericMethod> {
  private:
   typedef PNT::DecisionNode<typename TypeDomainTraits::PointerConstraintUnaryGenericMethod> inherited;
   friend class MethodConstraintTable<TypeDomainTraits>;
   
  protected:
   TemplateDefineExtendedParameters(1, inherited)
   typename TypeDomainTraits::PointerConstraintUnaryGenericFunction getFunction() const
      {  typename TypeDomainTraits::PointerConstraintUnaryGenericMethod method = inherited::getMethod();
         typename TypeDomainTraits::PointerConstraintUnaryGenericFunction result = nullptr;
         memcpy(&result, &method, sizeof(result));
         return result;
         // return *((typename TypeDomainTraits::PointerConstraintUnaryGenericFunction*) &inherited::getMethod());
      }
   bool isFunction() const { return hasOwnField(); }
   
  public:
   MethodConstraintUnaryDecisionNode() {}
   MethodConstraintUnaryDecisionNode(typename TypeDomainTraits::PointerConstraintUnaryGenericMethod& method)
      :  inherited(method) {}

   void setMethod(typename TypeDomainTraits::PointerConstraintUnaryGenericMethod method) { inherited::setMethod(method); }
   void resetMethod(typename TypeDomainTraits::PointerConstraintUnaryGenericMethod method) { inherited::resetMethod(method); }
   void setFunction(typename TypeDomainTraits::PointerConstraintUnaryGenericFunction function)
      {  typename TypeDomainTraits::PointerConstraintUnaryGenericMethod method = nullptr;
         memcpy(&method, &function, sizeof(function));
         // *((typename TypeDomainTraits::PointerConstraintUnaryGenericFunction*) &method) = function;
         inherited::setMethod(method);
         mergeOwnField(1);
      }
   bool execute(typename TypeDomainTraits::Domain& object, const VirtualOperation& operation,
         const VirtualElement& result, ConstraintEnvironment& env) const
      {  return inherited::hasMethod()
            ? (isFunction() ? (*getFunction())(object, operation, result, env)
            : (object.*inherited::getMethod())(operation, result, env))
            : false;
      }
};

template <class TypeDomainTraits>
class MethodConstraintUnaryTable
   :  public PNT::ConstMethodTable<MethodConstraintUnaryDecisionNode<TypeDomainTraits>,
         TypeDomainTraits::NBUnaryConstraintMethods> {
  private:
   typedef MethodConstraintUnaryDecisionNode<TypeDomainTraits> DecisionNode;
   typedef PNT::ConstMethodTable<DecisionNode, TypeDomainTraits::NBUnaryConstraintMethods> inherited;
   typedef MethodConstraintUnaryTable<TypeDomainTraits> thisType;

  public:
   const DecisionNode& operator[](typename TypeDomainTraits::Operation::Type type) const
      {  return const_cast<thisType&>(*this).elementAt(type); }
   DecisionNode& operator[](typename TypeDomainTraits::Operation::Type type)
      {  return const_cast<thisType&>(*this).elementAt(type); }
   DecisionNode& elementAt(typename TypeDomainTraits::Operation::Type type)
      {  return inherited::elementAt(TypeDomainTraits::Constants::convertUnaryConstraintToOperationToIndex(type)); }

  public:
   MethodConstraintUnaryTable() { inherited::clear(); }
   MethodConstraintUnaryTable(const thisType& source) = default;
   TemplateDefineCopy(MethodConstraintUnaryTable, TypeDomainTraits)
};

template <class TypeDomainTraits>
class MethodConstraintTable : public EnhancedObject {
  private:
   typedef EnhancedObject inherited;
   typedef MethodConstraintTable<TypeDomainTraits> thisType;

  protected:
   MethodConstraintCompareTable<TypeDomainTraits> mcctCompareTable;
   MethodConstraintBinaryTable<TypeDomainTraits> mcbtBinaryTable;
   MethodConstraintUnaryTable<TypeDomainTraits> mcutUnaryTable;

  public:
   MethodConstraintTable() {}
   MethodConstraintTable(const thisType& source) = default;
   thisType& operator=(const thisType& source) = default;
   TemplateDefineCopy(MethodConstraintTable, TypeDomainTraits)
   DTemplateDefineAssign(MethodConstraintTable, TypeDomainTraits)

   const MethodConstraintCompareTable<TypeDomainTraits>& compareTable() const { return mcctCompareTable; }
   const MethodConstraintBinaryTable<TypeDomainTraits>& binaryTable() const { return mcbtBinaryTable; }
   const MethodConstraintUnaryTable<TypeDomainTraits>& unaryTable() const { return mcutUnaryTable; }
};

template <class TypeDomainTraits>
class MethodConstraintForConstantsTable : public EnhancedObject {
  private:
   typedef EnhancedObject inherited;
   typedef MethodConstraintForConstantsTable<TypeDomainTraits> thisType;

  protected:
   MethodConstraintCompareTable<TypeDomainTraits> mcctCompareTable;
   MethodConstraintBinaryTable<TypeDomainTraits> mcbtBinaryTable;

  public:
   MethodConstraintForConstantsTable() {}
   MethodConstraintForConstantsTable(const thisType& source) = default;
   thisType& operator=(const thisType& source) = default;
   TemplateDefineCopy(MethodConstraintForConstantsTable, TypeDomainTraits)
   DTemplateDefineAssign(MethodConstraintForConstantsTable, TypeDomainTraits)

   const MethodConstraintCompareTable<TypeDomainTraits>& compareTable() const { return mcctCompareTable; }
   const MethodConstraintBinaryTable<TypeDomainTraits>& binaryTable() const { return mcbtBinaryTable; }
};

/*****************************************/
/* Definition of the query method tables */
/*****************************************/

template <class TypeDomainTraits>
class MethodQueryDecisionNode : public PNT::DecisionNode<typename TypeDomainTraits::PointerQueryMethod> {
  private:
   typedef PNT::DecisionNode<typename TypeDomainTraits::PointerQueryMethod> inherited;

  public:
   MethodQueryDecisionNode() {}
   MethodQueryDecisionNode(typename TypeDomainTraits::PointerQueryMethod method) : inherited(method) {}

   void setMethod(typename TypeDomainTraits::PointerQueryMethod method) { inherited::setMethod(method); }
   void resetMethod(typename TypeDomainTraits::PointerQueryMethod method) { inherited::resetMethod(method); }
   bool execute(const typename TypeDomainTraits::Domain& object, const VirtualQueryOperation& operation,
         VirtualQueryOperation::Environment& env) const
      {  return inherited::hasMethod()
            ? (object.*inherited::getMethod())(
                  (const typename TypeDomainTraits::Domain::QueryOperation&) operation,
                  (typename TypeDomainTraits::Domain::QueryOperation::Environment&) env)
            : false;
      }
};

template <class TypeDomainTraits>
class MethodQuerySecondDecisionNode : public PNT::StaticMethodTable<MethodQueryDecisionNode<TypeDomainTraits> > {
  private:
   typedef MethodQueryDecisionNode<TypeDomainTraits> DecisionNode;
   typedef MethodQuerySecondDecisionNode<TypeDomainTraits> thisType;
   typedef PNT::StaticMethodTable<DecisionNode> inherited;
   
  public:
   MethodQuerySecondDecisionNode() {}
   MethodQuerySecondDecisionNode(const thisType& source) : inherited(source) {}
   TemplateDefineCopy(MethodQuerySecondDecisionNode, TypeDomainTraits)

   bool execute(const typename TypeDomainTraits::Domain& object, const VirtualQueryOperation& operation,
         VirtualQueryOperation::Environment& env) const
      {  return inherited::elementAt(operation.getExtensions()).execute(object, operation, env); }
};

template <class TypeDomainTraits>
class MethodQueryFirstDecisionNode : public MethodQueryDecisionNode<TypeDomainTraits> {
  private:
   typedef MethodQueryDecisionNode<TypeDomainTraits> inherited;
   typedef MethodQueryFirstDecisionNode<TypeDomainTraits> thisType;

  protected:
   TemplateDefineExtendedParameters(1, inherited)

  public:
   MethodQueryFirstDecisionNode() {}
   MethodQueryFirstDecisionNode(typename TypeDomainTraits::PointerQueryMethod method) : inherited(method) {}

   thisType& absorbMethodTable(MethodQuerySecondDecisionNode<TypeDomainTraits>* source)
      {  clearOwnField();
         return (thisType&) inherited::setArray(source, inherited::CAControl);
      }
   thisType& absorbFunctionTable(VirtualElement::FunctionQueryTable* source)
      {  mergeOwnField(1);
         return (thisType&) inherited::setArray(source, inherited::CAControl);
      }
   void setMethod(typename TypeDomainTraits::PointerQueryMethod method) { inherited::setMethod(method); }
   void resetMethod(typename TypeDomainTraits::PointerQueryMethod method) { inherited::resetMethod(method); }
   bool execute(const typename TypeDomainTraits::Domain& object, const VirtualQueryOperation& operation,
         VirtualQueryOperation::Environment& env) const
      {  return inherited::hasMethod()
            ? (object.*inherited::getMethod())(
                  (const typename TypeDomainTraits::Domain::QueryOperation&) operation,
                  (typename TypeDomainTraits::Domain::QueryOperation::Environment&) env)
               : (inherited::hasArray() ? (!hasOwnField()
            ? ((const MethodQuerySecondDecisionNode<TypeDomainTraits>&) inherited::getArray())
                  .execute(object, operation, env)
            : ((const VirtualElement::FunctionQueryTable&) inherited::getArray()).execute(object, operation, env))
            : false);
      }
   const VirtualElement::FunctionQueryTable& getFunctionTable() const
      {  AssumeCondition(hasOwnField())
         return (const VirtualElement::FunctionQueryTable&) inherited::getArray();
      }
   VirtualElement::FunctionQueryTable& getFunctionTable()
      {  AssumeCondition(hasOwnField())
         return (VirtualElement::FunctionQueryTable&) inherited::getArray();
      }
   MethodQuerySecondDecisionNode<TypeDomainTraits>& getMethodTable()
      {  AssumeCondition(!hasOwnField())
         return (MethodQuerySecondDecisionNode<TypeDomainTraits>&) inherited::getArray();
      }
};

template <class TypeDomainTraits>
class MethodQueryTable : public PNT::ConstMethodTable<MethodQueryFirstDecisionNode<TypeDomainTraits>, VirtualQueryOperation::EndOfType> {
  private:
   typedef PNT::ConstMethodTable<MethodQueryFirstDecisionNode<TypeDomainTraits>, VirtualQueryOperation::EndOfType> inherited;

  public:
   MethodQueryTable() { inherited::clear(); }
   MethodQueryTable(const MethodQueryTable<TypeDomainTraits>& source) : inherited(source) {}
   TemplateDefineCopy(MethodQueryTable, TypeDomainTraits)

   const VirtualElement::FunctionQueryTable& getFunctionTable(VirtualQueryOperation::Type type) const
      {  return inherited::elementAt(type).getFunctionTable(); }
};

template <class TypeDomainTraits>
class MethodAccess {
  public:
   typedef typename TypeDomainTraits::Domain::Operation Operation;

   static bool apply(typename TypeDomainTraits::Domain& element, const Operation& operation,
         EvaluationEnvironment& env);
   static bool applyTo(const typename TypeDomainTraits::Domain& argument, const Operation& operation,
         VirtualElement& thisElement, EvaluationEnvironment& env);
   
   static bool query(const typename TypeDomainTraits::Domain& element,
         const typename TypeDomainTraits::Domain::QueryOperation& operation, VirtualQueryOperation::Environment& env);
   
   static bool mergeWithConstantInt(const typename TypeDomainTraits::Domain& thisElement, const VirtualElement& sourceElement,
         EvaluationEnvironment& env);
   static bool mergeWithConstantFloat(const typename TypeDomainTraits::Domain& thisElement, const VirtualElement& sourceElement,
         EvaluationEnvironment& env);
   static bool mergeWithInterval(typename TypeDomainTraits::Domain& thisElement, const VirtualElement& sourceElement,
         EvaluationEnvironment& env);
   
   static bool containConstant(const typename TypeDomainTraits::Domain& thisElement, const VirtualElement& sourceElement,
         EvaluationEnvironment& env);
   static bool containInterval(const typename TypeDomainTraits::Domain& thisElement, const VirtualElement& sourceElement,
         EvaluationEnvironment& env);
   
   static bool intersectWithConstant(typename TypeDomainTraits::Domain& thisElement, const VirtualElement& sourceElement,
         EvaluationEnvironment& env);
   static bool intersectWithInterval(typename TypeDomainTraits::Domain& thisElement, const VirtualElement& sourceElement,
         EvaluationEnvironment& env);
   
   static bool mergeWithTo(const typename TypeDomainTraits::Domain& argument, VirtualElement& thisElement,
         EvaluationEnvironment& env);
   static bool containToConstant(const typename TypeDomainTraits::Domain& argument, VirtualElement& thisElement,
         EvaluationEnvironment& env);
   static bool intersectWithTo(const typename TypeDomainTraits::Domain& argument, VirtualElement& thisElement,
         EvaluationEnvironment& env);
   
   static bool constraint(typename TypeDomainTraits::Domain& thisElement, const Operation& operation,
         const VirtualElement& resultElement, ConstraintEnvironment& env);
   static bool constraintConstant(typename TypeDomainTraits::Domain& thisElement, const Operation& operation,
         const VirtualElement& resultElement, ConstraintEnvironment& env);
   static bool constraintTo(typename TypeDomainTraits::Domain& argument, const Operation& operation,
         VirtualElement& thisElement, const VirtualElement& resultElement, ConstraintEnvironment& env);
};

template <class TypeDomainTraits>
inline bool
MethodAccess<TypeDomainTraits>::applyTo(const typename TypeDomainTraits::Domain& argument, const Operation& operation,
         VirtualElement& thisElement, EvaluationEnvironment& env) {
   AssumeCondition(argument.getType() > thisElement.getType())
   VirtualElement::assume(argument.mattMethodApplyToTable[operation.getType()].execute(argument, operation, thisElement, env));
   return true;
}

template <class TypeDomainTraits>
inline bool
MethodAccess<TypeDomainTraits>::query(const typename TypeDomainTraits::Domain& element,
      const typename TypeDomainTraits::Domain::QueryOperation& operation, VirtualQueryOperation::Environment& env) {
   VirtualElement::assume(element.mqtMethodQueryTable[operation.getType()].execute(element, operation, env));
   return true;
}
   
template <class TypeDomainTraits>
inline bool
MethodAccess<TypeDomainTraits>::mergeWithInterval(typename TypeDomainTraits::Domain& thisElement, const VirtualElement& sourceElement,
      EvaluationEnvironment& env) {
   AssumeCondition(sourceElement.getApproxKind().compareApply(VirtualElement::ApproxKind().setInterval()) <= CREqual)
   if (sourceElement.getApproxKind().compareApply(typename VirtualElement::ApproxKind().setInterval()) == CRLess)
      thisElement.mergeWithConstant(sourceElement, env);
   else
      thisElement.mergeWithInterval(sourceElement, env);
   return true;
}
   
template <class TypeDomainTraits>
inline bool
MethodAccess<TypeDomainTraits>::containConstant(const typename TypeDomainTraits::Domain& thisElement,
      const VirtualElement& sourceElement, EvaluationEnvironment& env) {
   AssumeCondition(dynamic_cast<const typename TypeDomainTraits::Domain*>(&sourceElement))
   ComparisonResult result = thisElement.compare((const typename TypeDomainTraits::Domain&) sourceElement);
   if (result != CREqual)
      env.setNoneApplied();
   return true;
}

template <class TypeDomainTraits>
inline bool
MethodAccess<TypeDomainTraits>::containInterval(const typename TypeDomainTraits::Domain& thisElement,
      const VirtualElement& sourceElement, EvaluationEnvironment& env) {
   return ((sourceElement.getApproxKind().compareApply(VirtualElement::ApproxKind().setConstant()) <= CREqual)
         ? thisElement.containConstant(sourceElement, env) : thisElement.containInterval(sourceElement, env));
}
   
template <class TypeDomainTraits>
inline bool
MethodAccess<TypeDomainTraits>::intersectWithConstant(typename TypeDomainTraits::Domain& thisElement,
      const VirtualElement& sourceElement, EvaluationEnvironment& env) {
   AssumeCondition(dynamic_cast<const typename TypeDomainTraits::Domain*>(&sourceElement))
   ComparisonResult result = thisElement.compare((const typename TypeDomainTraits::Domain&) sourceElement);
   env.mergeVerdictExact();
   if (result != CREqual)
      env.setEmpty();
   return true;
}
   
template <class TypeDomainTraits>
inline bool
MethodAccess<TypeDomainTraits>::intersectWithInterval(typename TypeDomainTraits::Domain& thisElement, const VirtualElement& sourceElement,
      EvaluationEnvironment& env) {
   AssumeCondition(sourceElement.getApproxKind().compareConstraint(thisElement.getApproxKind()) >= CREqual)
   if (sourceElement.getApproxKind().compareConstraint(thisElement.getApproxKind()) == CRGreater)
      thisElement.intersectWithGeneric(sourceElement, env);
   else
      thisElement.intersectWithInterval(sourceElement, env);
   return true;
}
   
template <class TypeDomainTraits>
inline bool
MethodAccess<TypeDomainTraits>::mergeWithTo(const typename TypeDomainTraits::Domain& argument, VirtualElement& thisElement,
      EvaluationEnvironment& env) {
   AssumeCondition(thisElement.getApproxKind().compareApply(argument.getApproxKind()) == CRLess)
   PPVirtualElement argumentCopy(argument);
   VirtualElement::assume(argumentCopy->mergeWith(thisElement, env));
   if (!env.hasResult())
      env.presult() = argumentCopy;
   return true;
}
   
template <class TypeDomainTraits>
inline bool
MethodAccess<TypeDomainTraits>::containToConstant(const typename TypeDomainTraits::Domain& argument,
      VirtualElement& thisElement, EvaluationEnvironment& env) {
   AssumeCondition(thisElement.getApproxKind().compareApply(argument.getApproxKind()) == CRLess)
   return argument.containToConstant(thisElement, env);
}
   
template <class TypeDomainTraits>
inline bool
MethodAccess<TypeDomainTraits>::intersectWithTo(const typename TypeDomainTraits::Domain& argument,
      VirtualElement& thisElement, EvaluationEnvironment& env) {
   AssumeCondition(thisElement.getApproxKind().compareConstraint(typename VirtualElement::ApproxKind().setConstant()) >= CREqual)
   PPVirtualElement argumentCopy(argument);
   VirtualElement::assume(argumentCopy->intersectWith(thisElement, env));
   if (!env.hasResult()) {
      env.presult() = argumentCopy;
      env.setUnstable();
   };
   return true;
}
   
} // end of namespace Details

}}} // end of namespace Analyzer::Scalar::Approximate

#endif // Analyzer_Scalar_Approximate_MethodTableH

