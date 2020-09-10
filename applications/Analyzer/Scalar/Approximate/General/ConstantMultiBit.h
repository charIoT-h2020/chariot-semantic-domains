/////////////////////////////////
//
// Library   : Analyzer/Scalar
// Unit      : approximate native scalar elements
// File      : ConstantMultiBit.h
// Author    : Franck Vedrine
// Copyright : CEA LIST - 2019-2020 - all rights reserved
//
// Description :
//   Definition of an integer class of multibit elements.
//   This definition relies on independent host analyses.
//

#ifndef Analyzer_Scalar_Approximate_ConstantMultiBitH
#define Analyzer_Scalar_Approximate_ConstantMultiBitH

#include "Analyzer/Scalar/Approximate/General/ConstantBit.h"
#include "Analyzer/Scalar/Implementation/General/MultiBitElement.h"

namespace Analyzer { namespace Scalar { namespace Approximate {

template <class TypeAConstant>
class MBitFieldMask;

} // end of namespace Approximate

namespace MultiBit { namespace Approximate {

namespace DConstantElement {

class Base : public TVirtualElement<Scalar::Approximate::Details::IntOperationElement> {
  private:
   typedef TVirtualElement<Scalar::Approximate::Details::IntOperationElement> inherited;

  public:
   class Constants : public Scalar::Approximate::Details::IntOperationElement::Constants,
                     public Scalar::Approximate::Details::BaseConstants {
     public:
      static const int NBApplyMethods = Scalar::MultiBit::Operation::EndOfType;
      static int convertApplyOperationToIndex(Scalar::MultiBit::Operation::Type type) { return type; }
      
      static const int NBConstraintCompareMethods
         = Scalar::MultiBit::Operation::EndOfCompare - Scalar::MultiBit::Operation::TCompareLessSigned;
      static int convertCompareConstraintOperationToIndex(Scalar::MultiBit::Operation::Type type)
         {  return type - Scalar::MultiBit::Operation::TCompareLessSigned; }
      
      static const int NBBinaryConstraintMethods
         = Scalar::MultiBit::Operation::EndOfType - Scalar::MultiBit::Operation::EndOfUnary - NBConstraintCompareMethods + 2;
      static int convertBinaryConstraintOperationToIndex(Scalar::MultiBit::Operation::Type type)
         {  return (type == Scalar::MultiBit::Operation::TConcat) ? 0
               : ((type == Scalar::MultiBit::Operation::TBitSet) ? 1
               : ((type < Scalar::MultiBit::Operation::EndOfBinary) ? (type - Scalar::MultiBit::Operation::EndOfUnary + 2)
               : (type - Scalar::MultiBit::Operation::EndOfCompare + (Scalar::MultiBit::Operation::EndOfBinary - Scalar::MultiBit::Operation::EndOfUnary + 2))));
         }

      static const int NBUnaryConstraintMethods
         = Scalar::MultiBit::Operation::EndOfUnary + 2;
      static int convertUnaryConstraintToOperationToIndex(Scalar::MultiBit::Operation::Type type)
         {  return (type < Scalar::MultiBit::Operation::EndOfUnary) ? type
               : ((type == Scalar::MultiBit::Operation::TBitNegateAssign) ? (1+Scalar::MultiBit::Operation::EndOfUnary) : -1);
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
     public:
      MethodConstraintTable();
   };

   template <class TypeTraits> bool applyBinaryFloatAssign(TypeTraits traits,
         const VirtualOperation& operation, EvaluationEnvironment& env);
   template <class TypeTraits> bool applyUnaryFloatAssign(TypeTraits traits,
         const VirtualOperation& operation, EvaluationEnvironment& env);
   template <class TypeTraits> bool constraintBinaryFloat(TypeTraits traits,
         const VirtualOperation& operation, VirtualElement& source, const VirtualElement& result,
         ConstraintEnvironment::Argument arg, ConstraintEnvironment& env);
   template <class OperationTraits, class TypeConstant>
   static bool constraintTimesOperationAssignWithConstantResult(OperationTraits, TypeConstant& thisElement,
         const VirtualOperation&, VirtualElement& sourceElement, const VirtualElement& resultElement,
         ConstraintEnvironment::Argument arg, ConstraintEnvironment& env);
   template <class OperationTraits, class TypeConstant>
   static bool constraintDivideOperationAssignWithConstantResultOnArgSource(OperationTraits,
         TypeConstant& thisElement, const VirtualOperation&, VirtualElement& sourceElement,
         const VirtualElement& resultElement, ConstraintEnvironment& env);
   template <class OperationTraits, class TypeConstant>
   static bool constraintDivideOperationAssignWithConstantResultOnArgResult(OperationTraits,
         TypeConstant& thisElement, const VirtualOperation&, VirtualElement& sourceElement,
         const VirtualElement& resultElement, ConstraintEnvironment& env);
   template <class OperationTraits, class TypeConstant>
   static bool constraintModuloOperationAssignWithConstantResultOnArgResult(OperationTraits,
         TypeConstant& thisElement, const VirtualOperation&, VirtualElement& sourceElement,
         const VirtualElement& resultElement, ConstraintEnvironment& env);
   template <class OperationTraits, class TypeConstant>
   static bool constraintModuloOperationAssignWithConstantResultOnArgSource(OperationTraits,
         TypeConstant& thisElement, const VirtualOperation&, VirtualElement& sourceElement,
         const VirtualElement& resultElement, ConstraintEnvironment& env);

  public:
   template <class TypeConcrete, class TypeDomainTraits>
   class MethodQueryTable : public Scalar::Approximate::Details::MethodQueryTable<TypeDomainTraits> {
     private:
      typedef Scalar::Approximate::Details::MethodQueryTable<TypeDomainTraits> inherited;
     public:
      MethodQueryTable();
   };

   bool applyCompareFloat(const VirtualOperation& operation, EvaluationEnvironment& env);
   bool constraintCompareFloat(const VirtualOperation& operation,
         VirtualElement& source, ConstraintEnvironment::Argument arg, ConstraintEnvironment& env);
         
  public:
   Base(const Init& init) : inherited(init) {}
   Base(const Base& source) = default;
   
   virtual int countAtomic() const { return 1; }
};

template <class TypeImplementation>
class TConstantElement : public Base, protected TypeImplementation {
  public:
   typedef Approximate::VirtualElement VirtualElement;
   typedef Approximate::VirtualQueryOperation VirtualQueryOperation;
   typedef Approximate::ConstraintEnvironment ConstraintEnvironment;
   friend class Scalar::Approximate::MBitFieldMask<TConstantElement<TypeImplementation> >;
   friend class Base;
   
   class SignedOperationTraits : public Base::SignedOperationTraits {
     public:
      static void setToNegative(TypeImplementation& implementation) { implementation.neg().inc(); }
      static bool isStrictPositive(const TypeImplementation& implementation)
         {  return !implementation.cbitArray(implementation.getSize()-1) && !implementation.isZero(); }
      static bool isStrictNegative(const TypeImplementation& implementation)
         {  return implementation.cbitArray(implementation.getSize()-1); }
   };

   class UnsignedOperationTraits : public Base::UnsignedOperationTraits {
     public:
      static void setToNegative(TypeImplementation& implementation) { AssumeUncalled }
      static bool isStrictPositive(const TypeImplementation& implementation)
         {  return !implementation.isZero(); }
      static bool isStrictNegative(const TypeImplementation& implementation) { return false; }
   };
   
  private:
   typedef TConstantElement<TypeImplementation> thisType;
   typedef Base inherited;
  protected:
   typedef TypeImplementation inheritedImplementation;

  protected:
   template <class TypeDomainTraits>
   class MethodApplyTable : public inherited::MethodApplyTable<TConstantElement<TypeImplementation>, TypeDomainTraits> {
     public:
      MethodApplyTable() {}
   };

   template <class TypeDomainTraits>
   class MethodConstraintTable : public inherited::MethodConstraintTable<TConstantElement<TypeImplementation>, TypeDomainTraits> {
     public:
      MethodConstraintTable() {}
   };
   
  public:
   template <class TypeDomainTraits>
   class MethodQueryTable : public inherited::MethodQueryTable<TConstantElement<TypeImplementation>, TypeDomainTraits> {
     public:
      MethodQueryTable() {}
   };
   typedef typename inherited::Constants Constants;
   typedef Scalar::Approximate::Details::TDomainTraits<TConstantElement<TypeImplementation> > DomainTraits;

  private:
   typedef MethodConstraintTable<DomainTraits> LocalMethodConstraintTable;

  public:
   static LocalMethodConstraintTable mctMethodConstraintTable;

  public:
   static VirtualElement& getFirstArgument(EvaluationEnvironment& env)
      {  return (VirtualElement&) env.getFirstArgument(); }

  protected:
   virtual void _write(OSBase& out, const STG::IOObject::FormatParameters& aparams) const override
      {  const FormatParameters& params = (const FormatParameters&) aparams;
         if (!params.isDeterministic()) {
            out << "MultiBit::Constant 0x";
            implementation().write(out, typename inheritedImplementation::FormatParameters()
               .setFullHexaDecimal(inheritedImplementation::getSize()));
         }
         else {
            out << "0x";
            implementation().write(out, typename inheritedImplementation::FormatParameters()
               .setFullHexaDecimal(inheritedImplementation::getSize()));
            out.put('_').write((int) getSizeInBits(), false);
         }
      }
   virtual ComparisonResult _compare(const EnhancedObject& asource) const override
      {  ComparisonResult result = inherited::_compare(asource);
         if (result == CREqual) {
            AssumeCondition(dynamic_cast<const thisType*>((const inherited*) &castFromCopyHandler(asource)))
            const thisType& source = castFromCopyHandler(asource);
            result = inheritedImplementation::_compare(inheritedImplementation::castToCopyHandler((const inheritedImplementation&) source));
         };
         return result;
      }

  public:
   TConstantElement(const inherited::Init& init)
      : inherited(init), inheritedImplementation(init.getBitSize())
      {  if (init.hasInitialValue())
            inheritedImplementation::assign(init.getInitialValue());
         setConstant();
      }
   TConstantElement(const thisType& source) = default;
   thisType& operator=(const thisType& source) = default;
   TemplateDefineCopy(TConstantElement, TypeImplementation)
   DTemplateDefineAssign(TConstantElement, TypeImplementation)
   DTemplateCompare(TConstantElement, TypeImplementation)
   StaticInheritConversions(thisType, inherited)

   virtual bool isValid() const override { return inherited::isValid() && inheritedImplementation::isValid(); }

   const inheritedImplementation& implementation() const { return (const inheritedImplementation&) *this; }
   inheritedImplementation& implementation() { return (inheritedImplementation&) *this; }

   typedef typename inheritedImplementation::BitArray BitArray;
   BitArray bitArray(int index) { return inheritedImplementation::bitArray(index); }
   bool bitArray(int index) const { return inheritedImplementation::bitArray(index); }
   bool cbitArray(int index) const { return bitArray(index); }
   void setBitArray(int index, bool value) { inheritedImplementation::setBitArray(index, value); }

   typedef typename inheritedImplementation::ArrayProperty ArrayProperty;
   unsigned int operator[](int index) const { return inheritedImplementation::operator[](index); }
   ArrayProperty operator[](int index) { return inheritedImplementation::operator[](index); }
   unsigned int array(int index) const { return inheritedImplementation::array(index); }
   unsigned int carray(int index) const { return array(index); }
   ArrayProperty array(int index) { return inheritedImplementation::array(index); }
   
   int getSize() const { return inheritedImplementation::getSize(); }
   virtual int _getSizeInBits() const override { return getSize(); }

   const inheritedImplementation& getElement() const { return (const inheritedImplementation&) *this; }
   unsigned int getValue() const { return inheritedImplementation::getValue(); }
   PPVirtualElement newIntegerFromBase2(inheritedImplementation& match) const;

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

   typedef typename inherited::QueryOperation QueryOperation;
   bool queryNewBitDomain(const QueryOperation&, typename QueryOperation::Environment& env) const;
   bool queryCompareSpecialSigned(const QueryOperation&, typename QueryOperation::Environment& env) const;
   bool queryGuardAll(const QueryOperation&, typename QueryOperation::Environment& env) const;
   bool queryCompareSpecialUnsigned(const QueryOperation&, typename QueryOperation::Environment& env) const;
   bool queryBound(const QueryOperation&, typename QueryOperation::Environment& env) const;

   static bool queryNewDisjunction(const VirtualElement&, const VirtualQueryOperation&, VirtualQueryOperation::Environment& env);
   static bool queryNewInterval(const VirtualElement&, const VirtualQueryOperation&, VirtualQueryOperation::Environment& env);
   static bool queryIsSigned(const VirtualElement&, const VirtualQueryOperation&, VirtualQueryOperation::Environment& env);
   static bool queryInverseCastOperation(const VirtualElement&, const VirtualQueryOperation&, VirtualQueryOperation::Environment& env);

   static const thisType& querySameArgument(const EvaluationEnvironment& env)
      {  AssumeCondition(dynamic_cast<const thisType*>(&env.getFirstArgument()))
         return (const thisType&) env.getFirstArgument();
      }

   bool queryCompareSigned(const VirtualOperation& operation, EvaluationEnvironment& env) const;
   bool queryCompareUnsigned(const VirtualOperation& operation, EvaluationEnvironment& env) const;

#define DefineDeclareBinaryMethod(TypeOperation)                                                  \
bool apply##TypeOperation(const VirtualOperation& operation, EvaluationEnvironment& env);

   DefineDeclareBinaryMethod(MinSignedAssign)
   DefineDeclareBinaryMethod(MinUnsignedAssign)
   DefineDeclareBinaryMethod(MaxSignedAssign)
   DefineDeclareBinaryMethod(MaxUnsignedAssign)
   class MinFloatTraits {
     public:
      typedef Scalar::MultiBit::MinFloatAssignOperation MultiBitOperation;
      typedef Scalar::Floating::MinAssignOperation FloatOperation;
   };
   class MaxFloatTraits {
     public:
      typedef Scalar::MultiBit::MaxFloatAssignOperation MultiBitOperation;
      typedef Scalar::Floating::MaxAssignOperation FloatOperation;
   };
   DefineDeclareBinaryMethod(MinFloatAssign)
   DefineDeclareBinaryMethod(MaxFloatAssign)

#undef DefineDeclareBinaryMethod

#define DefineDeclareConstraintCompareMethod(TypeOperation)                                       \
bool constraint##TypeOperation(const VirtualOperation&, VirtualElement& source, Argument arg, ConstraintEnvironment& env);

#define DefineDeclareConstraintBinaryMethod(TypeOperation)                                        \
bool constraint##TypeOperation(const VirtualOperation&, VirtualElement& source, const VirtualElement& result,\
   Argument arg, ConstraintEnvironment& env);

   DefineDeclareConstraintCompareMethod(CompareLessOrGreaterUnsigned)
   DefineDeclareConstraintCompareMethod(CompareLessOrGreaterOrEqualSigned)
   DefineDeclareConstraintCompareMethod(CompareLessOrEqualOrGreaterUnsigned)
   DefineDeclareConstraintCompareMethod(CompareEqual)
   DefineDeclareConstraintCompareMethod(CompareDifferent)

   bool constraintMinUnsignedAssignWithConstantResult(const VirtualOperation&, VirtualElement& source, const VirtualElement& result, Argument arg, ConstraintEnvironment& env);
   bool constraintMaxUnsignedAssignWithConstantResult(const VirtualOperation&, VirtualElement& source, const VirtualElement& result, Argument arg, ConstraintEnvironment& env);
   bool constraintMinSignedAssignWithConstantResult(const VirtualOperation&, VirtualElement& source, const VirtualElement& result, Argument arg, ConstraintEnvironment& env);
   bool constraintMaxSignedAssignWithConstantResult(const VirtualOperation&, VirtualElement& source, const VirtualElement& result, Argument arg, ConstraintEnvironment& env);
   bool constraintMinFloatAssignWithConstantResult(const VirtualOperation&, VirtualElement& source, const VirtualElement& result, Argument arg, ConstraintEnvironment& env);
   bool constraintMaxFloatAssignWithConstantResult(const VirtualOperation&, VirtualElement& source, const VirtualElement& result, Argument arg, ConstraintEnvironment& env);

   DefineDeclareConstraintBinaryMethod(PlusSignedAssign)
   DefineDeclareConstraintBinaryMethod(PlusUnsignedAssign)
   DefineDeclareConstraintBinaryMethod(PlusFloatAssign)
   bool constraintPlusUnsignedWithSignedAssignWithConstantResult(const VirtualOperation&, VirtualElement& source, const VirtualElement& result, Argument arg, ConstraintEnvironment& env);
   DefineDeclareConstraintBinaryMethod(MinusSignedAssign)
   DefineDeclareConstraintBinaryMethod(MinusUnsignedAssign)
   DefineDeclareConstraintBinaryMethod(MinusFloatAssign)
   bool constraintTimesUnsignedAssignWithConstantResult(const VirtualOperation&, VirtualElement& source, const VirtualElement& result, ConstraintEnvironment::Argument arg, ConstraintEnvironment& env);
   bool constraintTimesSignedAssignWithConstantResult(const VirtualOperation&, VirtualElement& source, const VirtualElement& result, ConstraintEnvironment::Argument arg, ConstraintEnvironment& env);
   DefineDeclareConstraintBinaryMethod(TimesFloatAssign)
   bool constraintDivideUnsignedAssignWithConstantResultOnArgSource(const VirtualOperation&, VirtualElement& source, const VirtualElement& result, ConstraintEnvironment& env);
   bool constraintDivideSignedAssignWithConstantResultOnArgSource(const VirtualOperation&, VirtualElement& source, const VirtualElement& result, ConstraintEnvironment& env);
   bool constraintDivideUnsignedAssignWithConstantResultOnArgResult(const VirtualOperation&, VirtualElement& source, const VirtualElement& result, ConstraintEnvironment& env);
   bool constraintDivideSignedAssignWithConstantResultOnArgResult(const VirtualOperation&, VirtualElement& source, const VirtualElement& result, ConstraintEnvironment& env);
   DefineDeclareConstraintBinaryMethod(DivideFloatAssign)
   bool constraintModuloUnsignedAssignWithConstantResultOnArgResult(const VirtualOperation&, VirtualElement& source, const VirtualElement& result, ConstraintEnvironment& env);
   bool constraintModuloSignedAssignWithConstantResultOnArgResult(const VirtualOperation&, VirtualElement& source, const VirtualElement& result, ConstraintEnvironment& env);
   bool constraintModuloUnsignedAssignWithConstantResultOnArgSource(const VirtualOperation&, VirtualElement& source, const VirtualElement& result, ConstraintEnvironment& env);
   bool constraintModuloSignedAssignWithConstantResultOnArgSource(const VirtualOperation&, VirtualElement& source, const VirtualElement& result, ConstraintEnvironment& env);
   bool constraintBitOrAssignWithConstantResult(const VirtualOperation&, VirtualElement& source, const VirtualElement& result, ConstraintEnvironment::Argument arg, ConstraintEnvironment& env);
   bool constraintBitAndAssignWithConstantResult(const VirtualOperation&, VirtualElement& source, const VirtualElement& result, ConstraintEnvironment::Argument arg, ConstraintEnvironment& env);
   bool constraintBitExclusiveOrAssignWithConstantResult(const VirtualOperation&, VirtualElement& source, const VirtualElement& result, ConstraintEnvironment::Argument arg, ConstraintEnvironment& env);

   bool constraintLeftShiftAssignWithConstantResultOnArgSource(const VirtualOperation&, VirtualElement& source, const VirtualElement& result, ConstraintEnvironment& env);
   bool constraintRightShiftAssignWithConstantResultOnArgSource(const VirtualOperation&, VirtualElement& source, const VirtualElement& result, ConstraintEnvironment& env);
   bool constraintShiftOrRotateAssignWithConstantResultOnArgResult(const VirtualOperation&, VirtualElement& source, const VirtualElement& result, ConstraintEnvironment& env);
   bool constraintRotateAssignWithConstantResultOnArgSource(const VirtualOperation&, VirtualElement& source, const VirtualElement& result, ConstraintEnvironment& env);

#undef DefineDeclareConstraintCompareMethod
#undef DefineDeclareConstraintBinaryMethod

#define DefineDeclareUnaryMethod(TypeOperation)                                                   \
bool apply##TypeOperation(const VirtualOperation& operation, EvaluationEnvironment& env);
#define DefineDeclareBinaryMethod(TypeOperation)                                                  \
bool apply##TypeOperation(const VirtualOperation& operation, EvaluationEnvironment& env);

   DefineDeclareUnaryMethod(CastMultiBit)
   DefineDeclareUnaryMethod(Extend)
   DefineDeclareUnaryMethod(Reduce)
   DefineDeclareBinaryMethod(BitSet)

   DefineDeclareBinaryMethod(Concat)
   DefineDeclareUnaryMethod(PrevSignedAssign)
   DefineDeclareUnaryMethod(PrevUnsignedAssign)
   DefineDeclareUnaryMethod(NextSignedAssign)
   DefineDeclareUnaryMethod(NextUnsignedAssign)
   DefineDeclareBinaryMethod(PlusSignedAssign)
   DefineDeclareBinaryMethod(PlusUnsignedAssign)
   DefineDeclareBinaryMethod(PlusUnsignedWithSignedAssign)
   class PlusFloatTraits {
     public:
      typedef Scalar::MultiBit::PlusFloatAssignOperation MultiBitOperation;
      typedef Scalar::Floating::PlusAssignOperation FloatOperation;
   };
   DefineDeclareBinaryMethod(PlusFloatAssign)
   DefineDeclareBinaryMethod(MinusSignedAssign)
   DefineDeclareBinaryMethod(MinusUnsignedAssign)
   class MinusFloatTraits {
     public:
      typedef Scalar::MultiBit::MinusFloatAssignOperation MultiBitOperation;
      typedef Scalar::Floating::MinusAssignOperation FloatOperation;
   };
   DefineDeclareBinaryMethod(MinusFloatAssign)
   DefineDeclareBinaryMethod(TimesSignedAssign)
   DefineDeclareBinaryMethod(TimesUnsignedAssign)
   class TimesFloatTraits {
     public:
      typedef Scalar::MultiBit::TimesFloatAssignOperation MultiBitOperation;
      typedef Scalar::Floating::TimesAssignOperation FloatOperation;
   };
   DefineDeclareBinaryMethod(TimesFloatAssign)
   DefineDeclareBinaryMethod(DivideSignedAssign)
   DefineDeclareBinaryMethod(DivideUnsignedAssign)
   class DivideFloatTraits {
     public:
      typedef Scalar::MultiBit::DivideFloatAssignOperation MultiBitOperation;
      typedef Scalar::Floating::DivideAssignOperation FloatOperation;
   };
   DefineDeclareBinaryMethod(DivideFloatAssign)
   DefineDeclareUnaryMethod(OppositeSignedAssign)
   class OppositeFloatTraits {
     public:
      typedef Scalar::MultiBit::OppositeFloatAssignOperation MultiBitOperation;
      typedef Scalar::Floating::OppositeAssignOperation FloatOperation;
   };
   DefineDeclareUnaryMethod(OppositeFloatAssign)
   DefineDeclareBinaryMethod(ModuloSignedAssign)
   DefineDeclareBinaryMethod(ModuloUnsignedAssign)
   DefineDeclareBinaryMethod(BitOrAssign)
   DefineDeclareBinaryMethod(BitAndAssign)
   DefineDeclareBinaryMethod(BitExclusiveOrAssign)
   DefineDeclareUnaryMethod(BitNegateAssign)
   DefineDeclareBinaryMethod(LeftShiftAssign)
   DefineDeclareBinaryMethod(LogicalRightShiftAssign)
   DefineDeclareBinaryMethod(ArithmeticRightShiftAssign)
   DefineDeclareBinaryMethod(LeftRotateAssign)
   DefineDeclareBinaryMethod(RightRotateAssign)

#define DefineDeclareUnaryFloatMethod(TypeOperation)                                             \
   class TypeOperation##FloatTraits {                                                            \
     public:                                                                                     \
      typedef Scalar::MultiBit::TypeOperation##Operation MultiBitOperation;                      \
      typedef Scalar::Floating::TypeOperation##Operation FloatOperation;                         \
   };                                                                                            \
   bool apply##TypeOperation(const VirtualOperation& operation, EvaluationEnvironment& env);

   DefineDeclareUnaryFloatMethod(Acos)
   DefineDeclareUnaryFloatMethod(Asin)
   DefineDeclareUnaryFloatMethod(Atan)
   // DefineDeclareUnaryFloatMethod(Atan2)
   DefineDeclareUnaryFloatMethod(Ceil)
   DefineDeclareUnaryFloatMethod(Cos)
   DefineDeclareUnaryFloatMethod(Cosh)
   DefineDeclareUnaryFloatMethod(Exp)
   DefineDeclareUnaryFloatMethod(Fabs)
   DefineDeclareUnaryFloatMethod(Floor)
   // DefineDeclareUnaryFloatMethod(Fmod)
   // DefineDeclareUnaryFloatMethod(Frexp)
   // DefineDeclareUnaryFloatMethod(Ldexp)
   DefineDeclareUnaryFloatMethod(Log)
   DefineDeclareUnaryFloatMethod(Log10)
   // DefineDeclareUnaryFloatMethod(Modf)
   // DefineDeclareUnaryFloatMethod(Pow)
   DefineDeclareUnaryFloatMethod(Sin)
   DefineDeclareUnaryFloatMethod(Sinh)
   DefineDeclareUnaryFloatMethod(Sqrt)
   DefineDeclareUnaryFloatMethod(Tan)
   DefineDeclareUnaryFloatMethod(Tanh)
#undef DefineDeclareUnaryFloatMethod
   
#undef DefineDeclareUnaryMethod
#undef DefineDeclareBinaryMethod

#define DefineDeclareConstraintBinaryMethod(TypeOperation)                                         \
bool constraint##TypeOperation(VirtualElement& source,                                             \
      const VirtualElement& result, Argument arg, ConstraintEnvironment& env);

#define DefineDeclareConstraintBinaryMethodConstant(TypeOperation)                                 \
bool constraint##TypeOperation##ConstantResult(VirtualElement& source,                             \
      const VirtualElement& result, Argument arg, ConstraintEnvironment& env);

#define DefineDeclareConstraintBinaryMethodConstantDouble(TypeOperation)                           \
bool constraint##TypeOperation##ConstantResultArgResult(VirtualElement& source,                    \
      const VirtualElement& result, ConstraintEnvironment& env);                                   \
bool constraint##TypeOperation##ConstantResultArgSource(VirtualElement& source,                    \
      const VirtualElement& result, ConstraintEnvironment& env);

   DefineDeclareConstraintBinaryMethod(PlusAssign)
   DefineDeclareConstraintBinaryMethod(MinusAssign)
   DefineDeclareConstraintBinaryMethodConstant(TimesAssign)
   DefineDeclareConstraintBinaryMethodConstantDouble(DivideAssign)
   DefineDeclareConstraintBinaryMethodConstantDouble(ModuloAssign)
   DefineDeclareConstraintBinaryMethodConstant(BitOrAssign)
   DefineDeclareConstraintBinaryMethodConstant(BitAndAssign)
   DefineDeclareConstraintBinaryMethodConstant(BitExclusiveOrAssign)
   DefineDeclareConstraintBinaryMethodConstant(LeftShiftAssign)
   DefineDeclareConstraintBinaryMethodConstant(RightShiftAssign)

#undef DefineDeclareConstraintBinaryMethodConstantDouble
#undef DefineDeclareConstraintBinaryMethodConstant
#undef DefineDeclareConstraintBinaryMethod

};

} // end of namespace DConstantElement

/********************************************/
/* Definition of the class TConstantElement */
/********************************************/

template <class TypeMultiTraits>
class TConstantElement : public DConstantElement::TConstantElement<
      Scalar::MultiBit::Implementation::TMultiBitElement<TypeMultiTraits> > {
  public:
   typedef Approximate::VirtualElement VirtualElement;
   typedef Approximate::VirtualQueryOperation VirtualQueryOperation;
   typedef Approximate::ConstraintEnvironment ConstraintEnvironment;
   typedef TypeMultiTraits MultiTraits;
   
  private:
   typedef TConstantElement<TypeMultiTraits> thisType;
   typedef DConstantElement::TConstantElement<
      Scalar::MultiBit::Implementation::TMultiBitElement<TypeMultiTraits> > inherited;
  protected:
   typedef typename inherited::inheritedImplementation inheritedImplementation;

  protected:
   Scalar::MultiBit::Implementation::MultiBitElement queryMultiBitImplementation() const
      {  Scalar::MultiBit::Implementation::MultiBitElement result(inheritedImplementation::getSize());
         int index = (inheritedImplementation::getSize() + 8*sizeof(unsigned int) - 1)
               / (8*sizeof(unsigned int));
         while (--index >= 0)
            result[index] = inheritedImplementation::carray(index);
         return result;
      }

  public:
   TConstantElement(const typename inherited::Init& init) : inherited(init) {}
   TConstantElement(const thisType& source) = default;
   TemplateDefineCopy(TConstantElement, TypeMultiTraits)

   virtual bool isValid() const { return inherited::isValid() && inheritedImplementation::isValid(); }

   inheritedImplementation getMax() const
      {  inheritedImplementation result(inherited::getSizeInBits());
         result.neg();
         return result;
      }
   inheritedImplementation getMin() const { return inheritedImplementation(inherited::getSizeInBits()); }
};

class ConstantElement : public DConstantElement::TConstantElement<
      Scalar::MultiBit::Implementation::MultiBitElement> {
  public:
   typedef Approximate::VirtualElement VirtualElement;
   typedef Approximate::VirtualQueryOperation VirtualQueryOperation;
   typedef Approximate::ConstraintEnvironment ConstraintEnvironment;
   
  private:
   typedef DConstantElement::TConstantElement<
      Scalar::MultiBit::Implementation::MultiBitElement> inherited;

  protected:
   typedef inherited::inheritedImplementation inheritedImplementation;

  protected:
   Scalar::MultiBit::Implementation::MultiBitElement queryMultiBitImplementation() const
      {  Scalar::MultiBit::Implementation::MultiBitElement result(inheritedImplementation::getSize());
         int index = (inheritedImplementation::getSize() + 8*sizeof(unsigned int) - 1)
               / (8*sizeof(unsigned int));
         while (--index >= 0)
            result[index] = inheritedImplementation::carray(index);
         return result;
      }

  public:
   ConstantElement(const inherited::Init& init) : inherited(init) {}
   ConstantElement(const ConstantElement& source) = default;
   DefineCopy(ConstantElement)

   inheritedImplementation getMax() const
      {  inheritedImplementation result(getSizeInBits());
         result.neg();
         return result;
      }
   inheritedImplementation getMin() const { return inheritedImplementation(getSizeInBits()); }
};

template <class TypeRingTraits>
class TTopCast {
  public:
   static bool applyTop(VirtualElement& topElement, const VirtualOperation& operation, EvaluationEnvironment& env);
   static bool applyTo(const VirtualElement& topElement, VirtualElement& thisElement, const VirtualOperation& operation, EvaluationEnvironment& env);
   static bool constraintTop(VirtualElement& topElement, const VirtualOperation& operation, const VirtualElement& result, ConstraintEnvironment& env);
};

template <class TypeBased, class TypeRingTraits>
class TCloseConstantElement : public TypeBased {
  private:
   typedef TypeBased inherited;
   typedef TCloseConstantElement<TypeBased, TypeRingTraits> thisType;

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
   typedef Approximate::Details::IntOperationElement::Methods Methods;

  public:
   TCloseConstantElement(const typename TypeBased::Init& init) : inherited(init) {}
   TCloseConstantElement(const thisType& source) = default;
   Template2DefineCopy(TCloseConstantElement, TypeBased, TypeRingTraits)
      
   typedef Approximate::Details::IntOperationElement::QueryOperation QueryOperation;
   
   bool applyCastChar(const VirtualOperation& operation, EvaluationEnvironment& env);
   bool applyCastInt(const VirtualOperation& operation, EvaluationEnvironment& env);
   bool applyCastUnsignedInt(const VirtualOperation& operation, EvaluationEnvironment& env);
   bool applyCastBit(const VirtualOperation& operation, EvaluationEnvironment& env);
   bool applyCastShiftBit(const VirtualOperation& operation, EvaluationEnvironment& env);
   bool applyCastMultiFloat(const VirtualOperation& operation, EvaluationEnvironment& env);
   // bool applyCastRational(const VirtualOperation& operation, EvaluationEnvironment& env);
   bool applyCompareSigned(const VirtualOperation& operation, EvaluationEnvironment& env);
   bool applyCompareUnsigned(const VirtualOperation& operation, EvaluationEnvironment& env);

   virtual const VirtualElement::FunctionQueryTable& functionQueryTable(int queryIndex) const override
      {  return mqtMethodQueryTable.getFunctionTable((VirtualQueryOperation::Type) queryIndex); }

   virtual bool query(const VirtualQueryOperation&, VirtualQueryOperation::Environment& env) const override;
   virtual bool apply(const VirtualOperation& operation, Scalar::EvaluationEnvironment& env) override;
   virtual bool mergeWith(const Scalar::VirtualElement& source, Scalar::EvaluationEnvironment& env) override;

   static bool queryNewMultiBitConstant(const VirtualElement&, const VirtualQueryOperation&, VirtualQueryOperation::Environment& env);
   static bool queryNewMultiFloatConstant(const VirtualElement&, const VirtualQueryOperation&, VirtualQueryOperation::Environment& env);
   static bool queryNewBooleanDisjunction(const VirtualElement&, const VirtualQueryOperation&, VirtualQueryOperation::Environment& env);
   static bool queryNewTop(const VirtualElement&, const VirtualQueryOperation&, VirtualQueryOperation::Environment& env);
   static bool queryNewFalse(const VirtualElement&, const VirtualQueryOperation&, VirtualQueryOperation::Environment& env);
   static bool queryNewTrue(const VirtualElement&, const VirtualQueryOperation&, VirtualQueryOperation::Environment& env);
   static bool queryNewIntForShift(const VirtualElement&, const VirtualQueryOperation&, VirtualQueryOperation::Environment& env);
   static bool queryNewConstant(const VirtualElement&, const VirtualQueryOperation&, VirtualQueryOperation::Environment& env);
   static bool queryNewZero(const VirtualElement&, const VirtualQueryOperation&, VirtualQueryOperation::Environment& env);
   static bool queryNewMinusOne(const VirtualElement&, const VirtualQueryOperation&, VirtualQueryOperation::Environment& env);
   static bool queryNewOne(const VirtualElement&, const VirtualQueryOperation&, VirtualQueryOperation::Environment& env);
   static bool queryNewMinSigned(const VirtualElement&, const VirtualQueryOperation&, VirtualQueryOperation::Environment& env);
   static bool queryNewMinUnsigned(const VirtualElement&, const VirtualQueryOperation&, VirtualQueryOperation::Environment& env);
   static bool queryNewMaxSigned(const VirtualElement&, const VirtualQueryOperation&, VirtualQueryOperation::Environment& env);
   static bool queryNewMaxUnsigned(const VirtualElement&, const VirtualQueryOperation&, VirtualQueryOperation::Environment& env);
};

}}}} // end of namespace Analyzer::Scalar::MultiBit::Approximate

#endif // Analyzer_Scalar_Approximate_ConstantMultiBitH
