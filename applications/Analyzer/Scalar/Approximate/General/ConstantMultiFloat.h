/////////////////////////////////
//
// Library   : Analyzer/Scalar
// Unit      : approximate native scalar elements
// File      : ConstantMultiFloat.h
// Author    : Franck Vedrine
// Copyright : CEA LIST - 2019-2020 - all rights reserved
//
// Description :
//   Definition of a floating point class of multibit elements
//     for the mantissa and the exponent.
//   This definition relies on independent host analyses.
//

#ifndef Analyzer_Scalar_Approximate_ConstantMultiFloatH
#define Analyzer_Scalar_Approximate_ConstantMultiFloatH

#include "Analyzer/Scalar/Approximate/General/ConstantBit.h"
#include "Analyzer/Scalar/Constant/ConstantElement.h"

namespace Analyzer { namespace Scalar {

namespace Floating { namespace Approximate {

namespace DMultiConstantElement {

class Base : public TVirtualElement<Scalar::Approximate::Details::RealOperationElement> {
  private:
   typedef TVirtualElement<Scalar::Approximate::Details::RealOperationElement> inherited;

  public:
   class Constants : public Scalar::Approximate::Details::RealOperationElement::Constants, public Scalar::Approximate::Details::BaseConstants {
     public:
      static const int NBApplyMethods = Operation::EndOfType;
      static int convertApplyOperationToIndex(Operation::Type type) { return type; }
      
      static const int NBConstraintCompareMethods
         = Operation::EndOfCompare - Operation::StartOfCompare;
      static int convertCompareConstraintOperationToIndex(Operation::Type type)
         {  return type - Operation::StartOfCompare; }
      
      static const int NBBinaryConstraintMethods
         = Operation::EndOfSecondBinary - Operation::EndOfCompare
            + Operation::EndOfBinary - Operation::EndOfCast;
      static int convertBinaryConstraintOperationToIndex(Operation::Type type)
         {  return (type < Operation::EndOfBinary) ? (type - Operation::EndOfCast)
               : (type - Operation::EndOfCompare
                     + (Operation::EndOfBinary - Operation::EndOfCast));
         }
   
      static const int NBUnaryConstraintMethods = Operation::EndOfCast+1;
      static int convertUnaryConstraintToOperationToIndex(Operation::Type type)
         {  return (type < Operation::EndOfSecondBinary) ? type
               : (type - Operation::EndOfSecondBinary + Operation::EndOfCast);
         }
   };

  protected:
   template <class TypeConcrete, class TypeDomainTraits>
   class MethodApplyTable : public Scalar::Approximate::Details::MethodApplyTable<TypeDomainTraits> {
     private:
      typedef Scalar::Approximate::Details::MethodApplyTable<TypeDomainTraits> inherited;
      
     public:
      MethodApplyTable();
   };

   template <class TypeConcrete, class TypeDomainTraits>
   class MethodConstraintTable : public Scalar::Approximate::Details::MethodConstraintForConstantsTable<TypeDomainTraits> {
     private:
      typedef Scalar::Approximate::Details::MethodConstraintForConstantsTable<TypeDomainTraits> inherited;
     public:
      MethodConstraintTable();
   };

  public:
   template <class TypeConcrete, class TypeDomainTraits>
   class MethodQueryTable : public Scalar::Approximate::Details::MethodQueryTable<TypeDomainTraits> {
     private:
      typedef Scalar::Approximate::Details::MethodQueryTable<TypeDomainTraits> inherited;
     public:
      MethodQueryTable();
   };

  public:
   Base(const InitFloat& init) : inherited(init) {}
   Base(const Base& source) : inherited(source) {}

   bool querySimplification(const QueryOperation&, QueryOperation::Environment& env) const;
   
   virtual int countAtomic() const { return 1; }
   virtual VirtualElement* _createCompareResultElement(bool value) const { AssumeUncalled return nullptr; }
   virtual VirtualElement* _createMultiBitElement(const Init& init) const { AssumeUncalled return nullptr; }
   virtual Constant::VirtualElement* _createConstantMultiFloatElement(const Constant::VirtualElement::InitFloat& init) const { AssumeUncalled return nullptr; }
   VirtualElement* createCompareResultElement(bool value) const { return _createCompareResultElement(value); }
   VirtualElement* createMultiBitElement(const Init& init) const { return _createMultiBitElement(init); }
   Constant::VirtualElement* createConstantMultiFloatElement(const Constant::VirtualElement::InitFloat& init) const { return _createConstantMultiFloatElement(init); }
};

template <class TypeImplementation>
class TMultiConstantElement : public Base, protected TypeImplementation {
  public:
   typedef Scalar::Approximate::VirtualElement VirtualElement;
   typedef Scalar::Approximate::VirtualQueryOperation VirtualQueryOperation;
   typedef Scalar::Approximate::ConstraintEnvironment ConstraintEnvironment;
   friend class Base;
   typedef Floating::Operation Operation;

  private:
   typedef TMultiConstantElement<TypeImplementation> thisType;
   typedef Base inherited;
  protected:
   typedef TypeImplementation inheritedImplementation;

  protected:
   template <class TypeDomainTraits>
   class MethodApplyTable : public inherited::MethodApplyTable<TMultiConstantElement<TypeImplementation>, TypeDomainTraits> {
     public:
      MethodApplyTable() {}
   };

   template <class TypeDomainTraits>
   class MethodConstraintTable : public inherited::MethodConstraintTable<TMultiConstantElement<TypeImplementation>, TypeDomainTraits> {
     public:
      MethodConstraintTable() {}
   };
   
  public:
   template <class TypeDomainTraits>
   class MethodQueryTable : public inherited::MethodQueryTable<TMultiConstantElement<TypeImplementation>, TypeDomainTraits> {
     public:
      MethodQueryTable() {}
   };
   typedef typename inherited::Constants Constants;
   typedef Scalar::Approximate::Details::TDomainTraits<TMultiConstantElement<TypeImplementation> > DomainTraits;

  private:
   typedef MethodConstraintTable<DomainTraits> LocalMethodConstraintTable;

  public:
   static LocalMethodConstraintTable mctMethodConstraintTable;

  protected:
   void retrieveApplyCastFloatInInit(VirtualElement::InitFloat& result, PPVirtualElement floatElement, EvaluationEnvironment& env);
   
  public:
   static VirtualElement& getFirstArgument(EvaluationEnvironment& env)
      { return (VirtualElement&) env.getFirstArgument(); }

  protected:
   virtual void _write(OSBase& out, const STG::IOObject::FormatParameters& params) const override
      {  out << "Floating::MultiConstant ";
         implementation().write(out, typename inheritedImplementation::WriteParameters().setDecimal());
      }
   virtual ComparisonResult _compare(const EnhancedObject& asource) const override
      {  ComparisonResult result = inherited::_compare(asource);
         if (result == CREqual) {
            const thisType& source = castFromCopyHandler(asource);
            result = TypeImplementation::_compare(TypeImplementation::castToCopyHandler((const TypeImplementation&) source));
         }
         return result;
      }

  public:
   TMultiConstantElement(const inherited::InitFloat& init)
      : inherited(init), inheritedImplementation(init.sizeMantissa(), init.sizeExponent())
      {  if (init.hasInitialValue())
            inheritedImplementation::assign(init.getInitialValue());
         setConstant();
      }
   TMultiConstantElement(const thisType& source) = default;
   thisType& operator=(const thisType& source) = default;
   TemplateDefineCopy(TMultiConstantElement, TypeImplementation)
   DTemplateDefineAssign(TMultiConstantElement, TypeImplementation)
   DTemplateCompare(TMultiConstantElement, TypeImplementation)
   StaticInheritConversions(TMultiConstantElement<TypeImplementation>, inherited)
   virtual bool isValid() const override { return inherited::isValid() && inheritedImplementation::isValid(); }

   const inheritedImplementation& implementation() const { return (const inheritedImplementation&) *this; }
   inheritedImplementation& implementation() { return (inheritedImplementation&) *this; }

   typedef typename inheritedImplementation::Mantissa Mantissa;
   typedef typename inheritedImplementation::Exponent Exponent;
   const Mantissa& mantissa() const { return inheritedImplementation::mantissa(); }
   Mantissa& mantissa() { return inheritedImplementation::mantissa(); }
   const Exponent& exponent() const { return inheritedImplementation::exponent(); }
   Exponent& exponent() { return inheritedImplementation::exponent(); }
   bool isPositive() const { return inheritedImplementation::isPositive(); }
   bool isNegative() const { return inheritedImplementation::isNegative(); }
   void setPositive() { inheritedImplementation::setPositive(); }
   void setNegative() { inheritedImplementation::setNegative(); }
   void setSign(bool isPositive) { inheritedImplementation::setSign(isPositive); }

   int getSizeExponent() const { return inheritedImplementation::getSizeExponent(); }
   int getSizeMantissa() const { return inheritedImplementation::getSizeMantissa(); }
   virtual int _getSizeInBits() const override { return mantissa().getSize() + exponent().getSize() + 1; }

   const inheritedImplementation& getElement() const { return (const inheritedImplementation&) *this; }

   virtual bool applyTo(const VirtualOperation&, Approximate::VirtualElement&, EvaluationEnvironment&) const override { AssumeUncalled return false; }
   virtual bool mergeWithTo(Approximate::VirtualElement&, EvaluationEnvironment&) const override { AssumeUncalled return false; }
   virtual bool containTo(const Approximate::VirtualElement&, EvaluationEnvironment&) const override { return false; }
   virtual bool intersectWithTo(Approximate::VirtualElement& element, EvaluationEnvironment& env) const override;
   virtual bool constraintTo(const VirtualOperation&, Approximate::VirtualElement& source, const Approximate::VirtualElement&, ConstraintEnvironment&) override;
   virtual ZeroResult queryZeroResult() const override { return ZeroResult().setBool(!inheritedImplementation::isZero()); }

   virtual bool apply(const VirtualOperation& operation, Scalar::EvaluationEnvironment& env) override { return inherited::apply(operation, env); }
   virtual bool contain(const Scalar::VirtualElement& source, Scalar::EvaluationEnvironment& env) const override;
   virtual bool intersectWith(const Approximate::VirtualElement& source, EvaluationEnvironment& env) override;
   virtual bool constraint(const VirtualOperation&, const Approximate::VirtualElement&, ConstraintEnvironment& env) override;

#define DefineDeclareUnaryMethod(TypeOperation)                                                   \
bool apply##TypeOperation(const VirtualOperation& operation, EvaluationEnvironment& env);
#define DefineDeclareBinaryMethod(TypeOperation)                                                  \
bool apply##TypeOperation(const VirtualOperation& operation, EvaluationEnvironment& env);
#define DefineDeclareTernaryMethod(TypeOperation)                                                 \
bool apply##TypeOperation(const VirtualOperation& operation, EvaluationEnvironment& env);

   DefineDeclareUnaryMethod(CastMultiFloat)
   DefineDeclareUnaryMethod(CastMultiBit)

   DefineDeclareBinaryMethod(PlusAssign)
   DefineDeclareBinaryMethod(MinusAssign)
   DefineDeclareBinaryMethod(TimesAssign)
   DefineDeclareBinaryMethod(DivideAssign)
   DefineDeclareUnaryMethod(OppositeAssign)
   DefineDeclareUnaryMethod(AbsAssign)

   DefineDeclareTernaryMethod(MultAddAssign)
   DefineDeclareTernaryMethod(MultSubAssign)
   DefineDeclareTernaryMethod(NegMultAddAssign)
   DefineDeclareTernaryMethod(NegMultSubAssign)
   DefineDeclareUnaryMethod(IsInftyExponent)
   DefineDeclareUnaryMethod(IsNaN)
   DefineDeclareUnaryMethod(IsQNaN)
   DefineDeclareUnaryMethod(IsSNaN)
   DefineDeclareUnaryMethod(IsPositive)
   DefineDeclareUnaryMethod(IsZeroExponent)
   DefineDeclareUnaryMethod(IsNegative)
   DefineDeclareUnaryMethod(CastMantissa)
   DefineDeclareBinaryMethod(MinAssign)
   DefineDeclareBinaryMethod(MaxAssign)

   // DefineDeclareUnaryMethod(Acos)
   // DefineDeclareUnaryMethod(Asin)
   // DefineDeclareUnaryMethod(Atan)
   // // DefineDeclareUnaryMethod(Atan2)
   // DefineDeclareUnaryMethod(Ceil)
   // DefineDeclareUnaryMethod(Cos)
   // DefineDeclareUnaryMethod(Cosh)
   // DefineDeclareUnaryMethod(Exp)
   // DefineDeclareUnaryMethod(Fabs)
   // DefineDeclareUnaryMethod(Floor)
   // // DefineDeclareUnaryMethod(Fmod)
   // // DefineDeclareUnaryMethod(Frexp)
   // // DefineDeclareUnaryMethod(Ldexp)
   // DefineDeclareUnaryMethod(Log)
   // DefineDeclareUnaryMethod(Log10)
   // // DefineDeclareUnaryMethod(Modf)
   // // DefineDeclareUnaryMethod(Pow)
   // DefineDeclareUnaryMethod(Sin)
   // DefineDeclareUnaryMethod(Sinh)
   // DefineDeclareUnaryMethod(Sqrt)
   // DefineDeclareUnaryMethod(Tan)
   // DefineDeclareUnaryMethod(Tanh)
   
#undef DefineDeclareUnaryMethod
#undef DefineDeclareBinaryMethod
#undef DefineDeclareTernaryMethod

#define DefineDeclareConstraintCompareMethod(TypeOperation)                                       \
bool constraint##TypeOperation(const VirtualOperation&, VirtualElement& seSource, Argument acArg, ConstraintEnvironment& ceeEnv);

#define DefineDeclareConstraintBinaryMethod(TypeOperation)                                        \
bool constraint##TypeOperation(const VirtualOperation&, VirtualElement& seSource, const VirtualElement& seResult,\
   Argument acArg, ConstraintEnvironment& ceeEnv);

   DefineDeclareConstraintCompareMethod(CompareLessOrGreater)
   DefineDeclareConstraintCompareMethod(CompareLessOrGreaterOrEqual)
   DefineDeclareConstraintCompareMethod(CompareEqual)
   DefineDeclareConstraintCompareMethod(CompareDifferent)

   bool constraintMinAssignWithConstantResult(const VirtualOperation&, VirtualElement& source, const VirtualElement& result, Argument arg, ConstraintEnvironment& env);
   bool constraintMaxAssignWithConstantResult(const VirtualOperation&, VirtualElement& source, const VirtualElement& result, Argument arg, ConstraintEnvironment& env);
   bool constraintArithmeticAssignWithConstantResult(const VirtualOperation&, VirtualElement& source, const VirtualElement& result, ConstraintEnvironment::Argument arg, ConstraintEnvironment& env);

#undef DefineDeclareConstraintCompareMethod
#undef DefineDeclareConstraintBinaryMethod
   
   typedef typename inherited::QueryOperation QueryOperation;
   bool queryCompareSpecial(const QueryOperation&, typename QueryOperation::Environment& env) const;
   bool queryGuardAll(const QueryOperation&, typename QueryOperation::Environment& env) const;

   static bool queryNewDisjunction(const VirtualElement&, const VirtualQueryOperation&, VirtualQueryOperation::Environment& env);
   static bool queryNewInterval(const VirtualElement&, const VirtualQueryOperation&, VirtualQueryOperation::Environment& env);
   static bool queryInverseCastOperation(const VirtualElement&, const VirtualQueryOperation&, VirtualQueryOperation::Environment& env);
   bool querySizes(const QueryOperation&, typename QueryOperation::Environment& env) const;

   static const thisType& getSameArgument(const EvaluationEnvironment& env)
      {  AssumeCondition(dynamic_cast<const thisType*>(&env.getFirstArgument()))
         return (const thisType&) env.getFirstArgument();
      }

   bool applyCompare(const VirtualOperation& operation, EvaluationEnvironment& env);
};

} // end of namespace DMultiConstantElement

/*************************************************/
/* Definition of the class TMultiConstantElement */
/*************************************************/

template <class TypeMultiTraits>
class TMultiConstantElement : public DMultiConstantElement::TMultiConstantElement<
         Implementation::TMultiFloatElement<TypeMultiTraits> > {
  public:
   typedef Scalar::Approximate::VirtualElement VirtualElement;
   typedef Scalar::Approximate::VirtualQueryOperation VirtualQueryOperation;
   typedef Scalar::Approximate::ConstraintEnvironment ConstraintEnvironment;
   typedef TypeMultiTraits MultiTraits;
   
  private:
   typedef TMultiConstantElement<TypeMultiTraits> thisType;
   typedef DMultiConstantElement::TMultiConstantElement<Implementation::TMultiFloatElement<TypeMultiTraits> > inherited;
  protected:
   typedef typename inherited::inheritedImplementation inheritedImplementation;

  public:
   TMultiConstantElement(const typename inherited::InitFloat& init) : inherited(init) {}
   TMultiConstantElement(const thisType& source) = default;
   TemplateDefineCopy(TMultiConstantElement, TypeMultiTraits)

   virtual bool isValid() const { return inherited::isValid(); }
};

class MultiConstantElement : public DMultiConstantElement::TMultiConstantElement<
         Implementation::MultiFloatElement> {
  public:
   typedef Scalar::Approximate::VirtualElement VirtualElement;
   typedef Scalar::Approximate::VirtualQueryOperation VirtualQueryOperation;
   typedef Scalar::Approximate::ConstraintEnvironment ConstraintEnvironment;
   
  private:
   typedef DMultiConstantElement::TMultiConstantElement<Implementation::MultiFloatElement> inherited;

  protected:
   typedef inherited::inheritedImplementation inheritedImplementation;

  public:
   MultiConstantElement(const inherited::InitFloat& init) : inherited(init) {}
   MultiConstantElement(const MultiConstantElement& source) = default;
   DefineCopy(MultiConstantElement)
};

template <class TypeBased, class TypeRingTraits>
class TCloseMultiConstantElement : public TypeBased {
  private:
   typedef TypeBased inherited;
   typedef TCloseMultiConstantElement<TypeBased, TypeRingTraits> thisType;

  protected:
   typedef Scalar::Approximate::Details::TDomainTraits<thisType> DomainTraits;
   typedef typename TypeBased::template MethodApplyTable<DomainTraits> InheritedMethodApplyTable;
   class MethodApplyTable : public InheritedMethodApplyTable {
     private:
      typedef InheritedMethodApplyTable inherited;
     public:
      MethodApplyTable();
   };
   friend class MethodApplyTable;

   typedef typename TypeBased::template MethodQueryTable<DomainTraits> InheritedMethodQueryTable;
   class MethodQueryTable : public InheritedMethodQueryTable {
     private:
      typedef InheritedMethodQueryTable inherited;
     public:
      MethodQueryTable();
   };
   friend class MethodQueryTable;

  public:
   static MethodApplyTable matMethodApplyTable;
   static MethodQueryTable mqtMethodQueryTable;
   friend class Scalar::Approximate::Details::MethodAccess<DomainTraits>;
   typedef typename inherited::inheritedImplementation inheritedImplementation;
   typedef Scalar::Approximate::Details::RealOperationElement::Methods Methods;

  public:
   typedef typename TypeBased::InitFloat Init;
   TCloseMultiConstantElement(const typename TypeBased::InitFloat& init) : inherited(init) {}
   TCloseMultiConstantElement(const thisType& source) = default;
   Template2DefineCopy(TCloseMultiConstantElement, TypeBased, TypeRingTraits)
      
   virtual VirtualElement* _createCompareResultElement(bool value) const override;
   virtual VirtualElement* _createMultiBitElement(const typename TypeBased::Init& init) const override;
   virtual Constant::VirtualElement* _createConstantMultiFloatElement(const Constant::VirtualElement::InitFloat& init) const override;
   typedef Approximate::Details::RealOperationElement::QueryOperation QueryOperation;
   
   bool applyCastFloat(const VirtualOperation& operation, EvaluationEnvironment& env);
   bool applyCastDouble(const VirtualOperation& operation, EvaluationEnvironment& env);
   bool applyCastLongDouble(const VirtualOperation& operation, EvaluationEnvironment& env);
   // bool applyCastRational(const VirtualOperation& operation, EvaluationEnvironment& env);

   virtual const VirtualElement::FunctionQueryTable& functionQueryTable(int queryIndex) const override
      {  return mqtMethodQueryTable.getFunctionTable((VirtualQueryOperation::Type) queryIndex); }

   virtual bool query(const VirtualQueryOperation&, VirtualQueryOperation::Environment& env) const override;
   virtual bool apply(const VirtualOperation& operation, Scalar::EvaluationEnvironment& env) override;
   virtual bool mergeWith(const Scalar::VirtualElement& source, Scalar::EvaluationEnvironment& env) override;

   static bool queryNewBooleanDisjunction(const VirtualElement&, const VirtualQueryOperation&, VirtualQueryOperation::Environment& env);
   static bool queryNewMultiBitConstant(const VirtualElement&, const VirtualQueryOperation&, VirtualQueryOperation::Environment& env);
   static bool queryNewMultiFloatConstant(const VirtualElement&, const VirtualQueryOperation&, VirtualQueryOperation::Environment& env);
   static bool queryNewTop(const VirtualElement&, const VirtualQueryOperation&, VirtualQueryOperation::Environment& env);
   static bool queryNewFalse(const VirtualElement&, const VirtualQueryOperation&, VirtualQueryOperation::Environment& env);
   static bool queryNewTrue(const VirtualElement&, const VirtualQueryOperation&, VirtualQueryOperation::Environment& env);
   static bool queryNewZero(const VirtualElement&, const VirtualQueryOperation&, VirtualQueryOperation::Environment& env);
   static bool queryNewMin(const VirtualElement&, const VirtualQueryOperation&, VirtualQueryOperation::Environment& env);
   static bool queryNewMax(const VirtualElement&, const VirtualQueryOperation&, VirtualQueryOperation::Environment& env);
};

}}}} // end of namespace Analyzer::Scalar::Floating::Approximate

#endif // Analyzer_Scalar_Approximate_ConstantMultiFloatH
