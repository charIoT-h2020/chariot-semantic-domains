/////////////////////////////////
//
// Library   : Analyzer/Scalar
// Unit      : approximate scalar elements
// File      : IntervalInteger.h
// Author    : Franck Vedrine
// Copyright : CEA LIST - 2019-2020 - all rights reserved
//
// Description :
//   Definition of a class of polymorph integer intervals.
//

#ifndef Analyzer_Scalar_Approximate_IntervalIntegerH
#define Analyzer_Scalar_Approximate_IntervalIntegerH

#include "Analyzer/Scalar/Approximate/VirtualElement.h"
#include "Analyzer/Scalar/Approximate/MethodTable.h"

namespace Analyzer { namespace Scalar { namespace Approximate {

namespace Details {

class IntegerInterval : public VirtualIntegerInterval {
  private:
   typedef VirtualIntegerInterval inherited;
   PPVirtualElement ppveMin, ppveMax;

   PNT::TPassPointer<VirtualDisjunction, PPAbstractElement> changeSign(EvaluationEnvironment& env);
   PNT::TPassPointer<VirtualDisjunction, PPAbstractElement> applySureCast(const VirtualOperation& operation, EvaluationEnvironment& env) const;
   
   PNT::TPassPointer<VirtualDisjunction, PPAbstractElement> applyToSureCase(const QueryOperation&, VirtualElement& source, EvaluationEnvironment& env) const;
   PNT::TPassPointer<VirtualDisjunction, PPAbstractElement> applySureCase(const QueryOperation&, const VirtualElement& source, EvaluationEnvironment& env);
   PNT::TPassPointer<VirtualDisjunction, PPAbstractElement> applySureCaseInterval(const QueryOperation&, const IntegerInterval& source, EvaluationEnvironment& env);

   template <class TypeDecision>
   bool applyCompareConstant(TypeDecision, const VirtualElement& source, EvaluationEnvironment& env) const;
   template <class TypeDecision>
   bool applyCompareInterval(TypeDecision, const IntegerInterval& source, EvaluationEnvironment& env) const;

   template <class TypeDecision>
   bool applyUnaryAssign(TypeDecision, EvaluationEnvironment& env);
   template <class TypeDecision>
   bool applyArithmeticAssign(const TypeDecision& decision, EvaluationEnvironment& env);

   template <class Application> bool
   partitionConstant(const Application& application, PNT::TPassPointer<VirtualDisjunction, PPAbstractElement> result,
         EvaluationEnvironment& env);
   template <class Application> bool
   partitionInterval(const Application& application, PNT::TPassPointer<VirtualDisjunction, PPAbstractElement> result,
         EvaluationEnvironment& env, bool bothDisjunction, bool thisDisjunction);

   static QueryOperation::CompareSpecialEnvironment compareScalars(const VirtualElement& fst,
         const VirtualElement& snd, const EvaluationParameters& params, bool isUnsigned);

   PPVirtualElement newSMinDomain() const { return !hasUnsignedField() ? Methods::newMinSigned(*ppveMax) : Methods::newMinUnsigned(*ppveMax); }
   PPVirtualElement newSMaxDomain() const { return !hasUnsignedField() ? Methods::newMaxSigned(*ppveMin) : Methods::newMaxUnsigned(*ppveMin); }
   QueryOperation querySCompareSpecialOperation() const { QueryOperation result; if (!hasUnsignedField()) result.setCompareSpecialSigned(); else result.setCompareSpecialUnsigned(); return result; }
   CompareSpecialEnvironment querySCompareSpecial(const VirtualElement& source) const  { return !hasUnsignedField() ? Methods::compareSpecialSigned(source) : Methods::compareSpecial(source); }
   VirtualOperation querySPrevAssignOperation() const { return !hasUnsignedField() ? Methods::queryPrevSignedAssign(*ppveMin) : Methods::queryPrevUnsignedAssign(*ppveMin); }
   VirtualOperation querySPrevAssignOperation(const VirtualElement& element) const { return !hasUnsignedField() ? Methods::queryPrevSignedAssign(element) : Methods::queryPrevUnsignedAssign(element); }
   VirtualOperation querySNextAssignOperation() const { return !hasUnsignedField() ? Methods::queryNextSignedAssign(*ppveMin) : Methods::queryNextUnsignedAssign(*ppveMin); }
   VirtualOperation querySNextAssignOperation(const VirtualElement& element) const { return !hasUnsignedField() ? Methods::queryNextSignedAssign(element) : Methods::queryNextUnsignedAssign(element); }
   VirtualOperation querySMinusAssignOperation() const { return !hasUnsignedField() ? Methods::queryMinusSignedAssign(*ppveMin) : Methods::queryMinusUnsignedAssign(*ppveMin); }
   VirtualOperation querySMinusOperation() const { return !hasUnsignedField() ? Methods::queryMinusSigned(*ppveMin) : Methods::queryMinusUnsigned(*ppveMin); }
   VirtualOperation querySPlusAssignOperation() const { return !hasUnsignedField() ? Methods::queryPlusSignedAssign(*ppveMin) : Methods::queryPlusUnsignedAssign(*ppveMin); }
   VirtualOperation querySPlusOperation() const { return !hasUnsignedField() ? Methods::queryPlusSigned(*ppveMin) : Methods::queryPlusUnsigned(*ppveMin); }
   VirtualOperation querySTimesAssignOperation() const { return !hasUnsignedField() ? Methods::queryTimesSignedAssign(*ppveMin) : Methods::queryTimesUnsignedAssign(*ppveMin); }
   VirtualOperation querySTimesOperation() const { return !hasUnsignedField() ? Methods::queryTimesSigned(*ppveMin) : Methods::queryTimesUnsigned(*ppveMin); }
   VirtualOperation querySDivideAssignOperation() const { return !hasUnsignedField() ? Methods::queryDivideSignedAssign(*ppveMin) : Methods::queryDivideUnsignedAssign(*ppveMin); }
   VirtualOperation querySDivideOperation() const { return !hasUnsignedField() ? Methods::queryDivideSigned(*ppveMin) : Methods::queryDivideUnsigned(*ppveMin); }
   VirtualOperation querySModuloAssignOperation() const { return !hasUnsignedField() ? Methods::queryModuloSignedAssign(*ppveMin) : Methods::queryModuloUnsignedAssign(*ppveMin); }
   VirtualOperation querySRightShiftOperation() const { return !hasUnsignedField() ? Methods::queryRightShiftSigned(*ppveMin) : Methods::queryRightShiftUnsigned(*ppveMin); }
   VirtualOperation querySRightShiftAssignOperation() const { return !hasUnsignedField() ? Methods::queryRightShiftSignedAssign(*ppveMin) : Methods::queryRightShiftUnsignedAssign(*ppveMin); }
   VirtualOperation querySCompareLessOperation() const { return !hasUnsignedField() ? Methods::queryCompareLessSigned(*ppveMin) : Methods::queryCompareLessUnsigned(*ppveMin); }
   VirtualOperation querySCompareLessOrEqualOperation() const { return !hasUnsignedField() ? Methods::queryCompareLessOrEqualSigned(*ppveMin) : Methods::queryCompareLessOrEqualUnsigned(*ppveMin); }
   VirtualOperation querySCompareGreaterOperation() const { return !hasUnsignedField() ? Methods::queryCompareGreaterSigned(*ppveMin) : Methods::queryCompareGreaterUnsigned(*ppveMin); }
   VirtualOperation querySCompareGreaterOrEqualOperation() const { return !hasUnsignedField() ? Methods::queryCompareGreaterOrEqualSigned(*ppveMin) : Methods::queryCompareGreaterOrEqualUnsigned(*ppveMin); }
   VirtualOperation querySCompareEqualOperation() const { return Methods::queryCompareEqual(*ppveMin); }
   VirtualOperation querySMinAssignOperation() const { return !hasUnsignedField() ? Methods::queryMinSignedAssign(*ppveMin) : Methods::queryMinUnsignedAssign(*ppveMin); }
   VirtualOperation querySMinOperation() const { return !hasUnsignedField() ? Methods::queryMinSigned(*ppveMin) : Methods::queryMinUnsigned(*ppveMin); }
   VirtualOperation querySMaxAssignOperation() const { return !hasUnsignedField() ? Methods::queryMaxSignedAssign(*ppveMin) : Methods::queryMaxUnsignedAssign(*ppveMin); }
   VirtualOperation querySMaxOperation() const { return !hasUnsignedField() ? Methods::queryMaxSigned(*ppveMin) : Methods::queryMaxUnsigned(*ppveMin); }

  protected:
   DefineExtendedParameters(1, VirtualIntegerInterval)
   DefineSubExtendedParameters(Unsigned, 1, INHERITED)
   PPVirtualElement& smin() { return ppveMin; }
   PPVirtualElement& smax() { return ppveMax; }

  private:
   void setFromLimit(PPVirtualElement limit, Init::Compare init, bool isUnsigned);
   void setFromInterval(PPVirtualElement min, PPVirtualElement max, Init::BoundType type, bool isUnsigned);
   void setFromInterval(const PPVirtualElement& min, const PPVirtualElement& max, bool isUnsigned)
      {  ppveMin = min;
         ppveMax = max;
         // AssumeCondition(ppveMin->getType() == ppveMax->getType())
         if (isUnsigned)
            setUnsignedField(1);
      }

  protected:
   virtual void _write(OSBase& out, const STG::IOObject::FormatParameters& aparams) const override
      {  const FormatParameters& params = (const FormatParameters&) aparams;
         if (!params.isDeterministic()) {
            out << "Interval " << (hasUnsignedField() ? "unsigned" : "signed") << "[";
            ppveMin->write(out, params);
            out << ", ";
            ppveMax->write(out, params);
            out.put(']');
         }
         else {
            out.put('[');
            ppveMin->write(out, params);
            out << ", ";
            ppveMax->write(out, params);
            out.put(']');
            out.put(hasUnsignedField() ? 'U' : 'S');
         }
      }
   virtual int _getSizeInBits() const override { return ppveMin->getSizeInBits(); }
  
  public:
   IntegerInterval(const Init& init) : inherited(init)
      {  inherited::setInterval();
         if (init.tType) {
            bool isUnsigned = false;
            if (init.fUnsigned == Init::UUndefined)
               isUnsigned = !Methods::isSigned(*init.ppveFst);
            else
               isUnsigned = (init.fUnsigned == Init::UUnsigned);
            if (init.tType == Init::TLimit)
               setFromLimit(init.ppveFst, (Init::Compare) init.uQualification, isUnsigned);
            else if (init.tType == Init::TBoundInterval)
               setFromInterval(init.ppveFst, init.ppveSnd, (Init::BoundType) init.uQualification, isUnsigned);
            else if (init.tType == Init::TInterval)
               setFromInterval(init.ppveFst, init.ppveSnd, isUnsigned);
         };
      }
   IntegerInterval(const IntegerInterval& source)
      :  inherited(source), ppveMin(source.ppveMin, PNT::Pointer::Duplicate()),
         ppveMax(source.ppveMax, PNT::Pointer::Duplicate()) {}
   IntegerInterval& operator=(const IntegerInterval& source)
      {  VirtualElement::operator=(source);
         ppveMin.fullAssign(source.ppveMin);
         ppveMax.fullAssign(source.ppveMax);
         return *this;
      }
   DefineCopy(IntegerInterval)
   DDefineAssign(IntegerInterval)

   virtual bool isValid() const override
      {  return inherited::isValid() && ppveMin.isValid() && ppveMax.isValid()
            && ppveMin->isValid() && ppveMax->isValid()
            && ppveMin->getType() == ppveMax->getType();
      }
   virtual int countAtomic() const override { return ppveMin->countAtomic() + ppveMax->countAtomic(); }
   virtual const VirtualElement& getMin() const override { return *ppveMin; }
   virtual const VirtualElement& getMax() const override { return *ppveMax; }
   virtual PPVirtualElement& getSMin() override { return ppveMin; }
   virtual PPVirtualElement& getSMax() override { return ppveMax; }
   virtual bool isUnsignedTimesOperation(const VirtualOperation& operation) const { return false; }

   // virtual bool mergeWithTo(VirtualElement& source, EvaluationEnvironment& env) const { AssumeUncalled }
   // virtual bool containTo(const VirtualElement& source, EvaluationEnvironment& env) const { AssumeUncalled }
   // virtual bool intersectWithTo(VirtualElement& source, EvaluationEnvironment& env) const { AssumeUncalled }
   // virtual bool mergeWith(const Scalar::VirtualElement& source, Scalar::EvaluationEnvironment& env) { AssumeUncalled }
   // virtual bool contain(const Scalar::VirtualElement& source, Scalar::EvaluationEnvironment& env) const { AssumeUncalled }
   // virtual bool intersectWith(const VirtualElement& source, EvaluationEnvironment& env) { AssumeUncalled }
   virtual bool guard(PPVirtualElement thenElement, PPVirtualElement elseElement, EvaluationEnvironment& env) override
      {  return guardApproximate(thenElement, elseElement, env); }

   /* apply methods */
      // applyTo methods
#define DefineDeclareApplyTo(TypeVirtualOperation) \
   bool applyTo##TypeVirtualOperation(const VirtualOperation&, VirtualElement&, EvaluationEnvironment&) const;
#define DefineDeclareApplyToConstant(TypeVirtualOperation) \
   bool applyTo##TypeVirtualOperation##Constant(const VirtualOperation&, VirtualElement&, EvaluationEnvironment&) const;
   
   DefineDeclareApplyToConstant(Concat)
   DefineDeclareApplyToConstant(BitSet)
   DefineDeclareApplyToConstant(CompareLess)
   DefineDeclareApplyToConstant(CompareLessOrEqual)
   DefineDeclareApplyToConstant(CompareEqual)
   DefineDeclareApplyToConstant(CompareDifferent)
   DefineDeclareApplyToConstant(CompareGreaterOrEqual)
   DefineDeclareApplyToConstant(CompareGreater)
   DefineDeclareApplyToConstant(MinAssign)
   DefineDeclareApplyToConstant(MaxAssign)

   DefineDeclareApplyToConstant(PlusAssign)
   DefineDeclareApplyToConstant(MinusAssign)
   DefineDeclareApplyToConstant(TimesAssign)
   DefineDeclareApplyTo(DivideAssign)

   DefineDeclareApplyTo(ModuloAssign)
   DefineDeclareApplyToConstant(BitOrAssign)
   DefineDeclareApplyToConstant(BitAndAssign)
   DefineDeclareApplyToConstant(BitExclusiveOrAssign)
   DefineDeclareApplyToConstant(LeftShiftAssign)
   DefineDeclareApplyToConstant(RightShiftAssign)

   DefineDeclareApplyToConstant(LogicalAndAssign)
   DefineDeclareApplyToConstant(LogicalOrAssign)

#undef DefineDeclareApplyTo
#undef DefineDeclareApplyToConstant

      // Méthodes apply
#define DefineDeclareApply(TypeOperation) \
bool apply##TypeOperation(const VirtualOperation& operation, EvaluationEnvironment& env);

#define DefineDeclareBiApply(TypeOperation) \
bool apply##TypeOperation##Constant(const VirtualOperation& operation, const VirtualElement& source, EvaluationEnvironment& env); \
bool apply##TypeOperation##Interval(const VirtualOperation& operation, const IntegerInterval& source, EvaluationEnvironment& env);
   DefineDeclareApply(DispatchMinMax)

   DefineDeclareApply(CastInteger)
   DefineDeclareApply(CastDouble)
   
   DefineDeclareBiApply(CompareLess)
   DefineDeclareBiApply(CompareLessOrEqual)
   DefineDeclareBiApply(CompareEqual)
   DefineDeclareBiApply(CompareDifferent)
   DefineDeclareBiApply(CompareGreaterOrEqual)
   DefineDeclareBiApply(CompareGreater)
   DefineDeclareBiApply(MinAssign)
   DefineDeclareBiApply(MaxAssign)

   DefineDeclareApply(CastBit)
   DefineDeclareApply(Concat)
   DefineDeclareApply(BitSet)
   DefineDeclareApply(PrevAssign)
   DefineDeclareApply(NextAssign)
   DefineDeclareBiApply(PlusAssign)
   DefineDeclareBiApply(MinusAssign)
   DefineDeclareBiApply(TimesAssign)
   DefineDeclareBiApply(DivideAssign)
   DefineDeclareApply(OppositeAssign)

   DefineDeclareBiApply(ModuloAssign)
   DefineDeclareBiApply(BitOrAssign)
   DefineDeclareBiApply(BitAndAssign)
   DefineDeclareBiApply(BitExclusiveOrAssign)
   DefineDeclareBiApply(LeftShiftAssign)
   DefineDeclareBiApply(RightShiftAssign)
   DefineDeclareApply(BitNegateAssign)

   DefineDeclareBiApply(LogicalAndAssign)
   DefineDeclareBiApply(LogicalOrAssign)
   DefineDeclareApply(LogicalNegateAssign)

#undef DefineDeclareApply
#undef DefineDeclareBiApply

     // Méthodes query
#define DefineDeclareQuery(TypeOperation) \
static bool query##TypeOperation(const VirtualElement&, const VirtualQueryOperation& operation, VirtualQueryOperation::Environment& env);

   DefineDeclareQuery(CompareSpecial)
   DefineDeclareQuery(CompareSpecialSigned)
   DefineDeclareQuery(CompareSpecialUnsigned)
   DefineDeclareQuery(BitDomain)
   DefineDeclareQuery(Simplification)
   DefineDeclareQuery(SimplificationAsConstantDisjunction)
   DefineDeclareQuery(DispatchOnMin)
   DefineDeclareQuery(Copy)
   DefineDeclareQuery(Specialize)

#undef DefineDeclareQuery

   /* intersection and merge methods */
      // intersection methods
   bool intersectWithToConstant(VirtualElement&, EvaluationEnvironment&) const;
   bool intersectWithConstant(const VirtualElement&, EvaluationEnvironment&);
   bool intersectWithInterval(const VirtualElement&, EvaluationEnvironment&);
   bool intersectWithGeneric(const VirtualElement&, EvaluationEnvironment&);

      // merge methods
   bool containToConstant(const VirtualElement&, EvaluationEnvironment&) const;
   bool containConstant(const VirtualElement&, EvaluationEnvironment&) const;
   bool containInterval(const VirtualElement&, EvaluationEnvironment&) const;

   bool mergeWithToConstant(VirtualElement&, EvaluationEnvironment&) const;
   bool mergeWithConstant(const VirtualElement&, EvaluationEnvironment&);
   bool mergeWithInterval(const VirtualElement&, EvaluationEnvironment&);

   /* constraint methods */
#define DefineDeclareConstraint(TypeOperation)                                                   \
bool constraint##TypeOperation(const VirtualOperation&, VirtualElement& source,                  \
      Argument arg, ConstraintEnvironment& env);
#define DefineDeclareBiConstraint(TypeOperation)                                                 \
bool constraint##TypeOperation##Interval(const VirtualOperation&, IntegerInterval& source, Argument arg, \
      ConstraintEnvironment& env);                                                               \
bool constraint##TypeOperation##Constant(const VirtualOperation&, VirtualElement& source, Argument arg, \
      ConstraintEnvironment& env);
   bool constraintCastBit(const VirtualOperation& operation, const VirtualElement& result, ConstraintEnvironment& env);
   DefineDeclareConstraint(CompareMultiple)
   DefineDeclareBiConstraint(CompareLess)
   DefineDeclareBiConstraint(CompareLessSigned)
   DefineDeclareBiConstraint(CompareLessUnsigned)
   DefineDeclareBiConstraint(CompareLessOrEqual)
   DefineDeclareBiConstraint(CompareLessOrEqualSigned)
   DefineDeclareBiConstraint(CompareLessOrEqualUnsigned)
   DefineDeclareConstraint(CompareLessOrEqualGeneric)
   DefineDeclareConstraint(CompareLessOrEqualSignedGeneric)
   DefineDeclareConstraint(CompareLessOrEqualUnsignedGeneric)
   DefineDeclareBiConstraint(CompareEqual)
   DefineDeclareConstraint(CompareEqualGeneric)
   DefineDeclareBiConstraint(CompareDifferent)
   DefineDeclareBiConstraint(CompareGreaterOrEqual)
   DefineDeclareBiConstraint(CompareGreaterOrEqualSigned)
   DefineDeclareBiConstraint(CompareGreaterOrEqualUnsigned)
   DefineDeclareBiConstraint(CompareGreater)
   DefineDeclareBiConstraint(CompareGreaterSigned)
   DefineDeclareBiConstraint(CompareGreaterUnsigned)
   DefineDeclareConstraint(CompareGreaterOrEqualGeneric)
   DefineDeclareConstraint(CompareGreaterOrEqualSignedGeneric)
   DefineDeclareConstraint(CompareGreaterOrEqualUnsignedGeneric)
#undef DefineDeclareConstraint
#undef DefineDeclareBiConstraint
};

typedef PNT::TPassPointer<IntegerInterval, PPVirtualElement> PPIntegerInterval;

inline IntegerInterval::QueryOperation::CompareSpecialEnvironment
IntegerInterval::compareScalars(const VirtualElement& fst, const VirtualElement& snd,
      const EvaluationParameters& params, bool isUnsigned) {
   EvaluationEnvironment diffEnv(params);
   IntOperationElement::QueryOperation::CompareSpecialEnvironment compare;
   diffEnv.setStopError();
   if (!isUnsigned) {
      PPVirtualElement diff = Methods::apply(fst, Methods::queryMinusSigned(fst), snd, diffEnv);
      if (!diffEnv.isEmpty())
         compare = IntOperationElement::Methods::compareSpecialSigned(*diff);
      if (diffEnv.hasEvaluationError()) {
         if (diffEnv.isPositiveOverflow())
            compare.mergeBehaviour(QueryOperation::CompareSpecialEnvironment::BGreater1);
         if (diffEnv.isNegativeOverflow())
            compare.mergeBehaviour(QueryOperation::CompareSpecialEnvironment::BLessM1);
      };
   }
   else {
      PPVirtualElement diff = Methods::apply(fst, Methods::queryMinusUnsigned(fst), snd, diffEnv);
      if (!diffEnv.isEmpty())
         compare = IntOperationElement::Methods::compareSpecialUnsigned(*diff);
      if (diffEnv.hasEvaluationError()) {
         diffEnv.clear();
         PPVirtualElement inverseDiff = Methods::apply(snd, Methods::queryMinusUnsigned(fst), fst, diffEnv);
         IntOperationElement::QueryOperation::CompareSpecialEnvironment inverseCompare;
         if (!diffEnv.isEmpty())
            inverseCompare = IntOperationElement::Methods::compareSpecialUnsigned(*inverseDiff);
         compare.mergeWith(inverseCompare.opposite());
      };
   }
   return compare;
}

}} // end of namespace Approximate::Details

namespace Integer { namespace Approximate {

namespace Details {

using IntegerInterval = Scalar::Approximate::Details::IntegerInterval;
using BaseConstants = Scalar::Approximate::Details::BaseConstants;
template <class TypeDomain> using TDomainTraits = Scalar::Approximate::Details::TDomainTraits<TypeDomain>;
template <class TypeDomainTraits> using MethodApplyTable = Scalar::Approximate::Details::MethodApplyTable<TypeDomainTraits>;
template <class TypeDomainTraits> using MethodApplyToTable = Scalar::Approximate::Details::MethodApplyToTable<TypeDomainTraits>;
template <class TypeDomainTraits> using MethodConstraintTable = Scalar::Approximate::Details::MethodConstraintTable<TypeDomainTraits>;
template <class TypeDomainTraits> using MethodQueryTable = Scalar::Approximate::Details::MethodQueryTable<TypeDomainTraits>;
template <class TypeDomainTraits> using MultiApplyMethod = Scalar::Approximate::Details::MultiApplyMethod<TypeDomainTraits>;
template <class TypeDomainTraits> using MultiConstraintCompareMethod = Scalar::Approximate::Details::MultiConstraintCompareMethod<TypeDomainTraits>;

} // end of namespace Details

class Interval : public TVirtualElement<Details::IntegerInterval> {
  public:
   typedef TVirtualElement<Details::IntegerInterval> IntegerInterval;

  private:
   typedef Interval thisType;
   typedef TVirtualElement<Details::IntegerInterval> inherited;

  public:
   typedef Integer::Operation Operation;
   class Constants : public inherited::Constants, public Details::BaseConstants {
     private:
      typedef Scalar::Integer::Operation Operation;

     public:
      static const int NBUnaryConstraintMethods = Operation::EndOfUnary + 3; // Opposite & BitNegate & LogicalNegate
      static int convertUnaryConstraintToOperationToIndex(Operation::Type type)
         {  return (type < Operation::EndOfUnary)
               ? type : ((type == Operation::TOppositeAssign)
               ? (type - Operation::TOppositeAssign + Operation::EndOfUnary) : ((type == Operation::TBitNegateAssign)
               ? (type + 1 - Operation::TBitNegateAssign + Operation::EndOfUnary) : ((type == Operation::TLogicalNegateAssign)
               ? (type + 2 - Operation::TLogicalNegateAssign + Operation::EndOfUnary)
               : -1)));
         }
      static const int NBBinaryConstraintMethods
         = Operation::EndOfType - Operation::EndOfCompare + Operation::StartOfCompare;
      static int convertBinaryConstraintOperationToIndex(Operation::Type type)
         {  return (type < Operation::EndOfBinary)
               ? (type - Operation::StartOfBinary)
               : (type - Operation::EndOfCompare + Operation::EndOfBinary - Operation::StartOfBinary);
         }
      static const int NBConstraintCompareMethods
         = Scalar::Integer::Operation::EndOfCompare - Scalar::Integer::Operation::TCompareLess;
      static int convertCompareConstraintOperationToIndex(Scalar::Integer::Operation::Type type)
         {  return type - Scalar::Integer::Operation::TCompareLess; }

      static const int NBApplyMethods = Operation::EndOfType;
      static int convertApplyOperationToIndex(Operation::Type type) { return type; }
      static const int NBApplyToMethods = Operation::EndOfType - Operation::EndOfUnary;
      static int convertApplyToOperationToIndex(Operation::Type type)
         { return type - Operation::EndOfUnary; }
   };
   typedef Approximate::Details::TDomainTraits<Interval> DomainTraits;

  private:
   class MethodApplyTable : public Details::MethodApplyTable<DomainTraits> {
     public:
      MethodApplyTable();
   };
   class MethodApplyToTable : public Details::MethodApplyToTable<DomainTraits> {
     public:
      MethodApplyToTable();
   };
   class MethodConstraintTable : public Details::MethodConstraintTable<DomainTraits> {
     public:
      MethodConstraintTable();
   };

   static MethodApplyTable matMethodApplyTable;
   static MethodApplyToTable mattMethodApplyToTable;
   static MethodConstraintTable mctMethodConstraintTable;

  public:
   class QueryTable : public inherited::QueryTable {
     public:
      QueryTable();
   };
   friend class QueryTable;
   static QueryTable mqtMethodQueryTable;

  protected:
   class ConstantAndIntervalApplyMethod : public Details::MultiApplyMethod<DomainTraits> {
     private:
      typedef Details::MultiApplyMethod<DomainTraits> inherited;
      typedef bool (Interval::*PointerApplyConstantMethod)
         (const VirtualOperation&, const VirtualElement&, EvaluationEnvironment&);
      typedef bool (Interval::*PointerApplyIntervalMethod)
         (const VirtualOperation&, const Details::IntegerInterval&, EvaluationEnvironment&);

      PointerApplyConstantMethod pacmConstantMethod;
      PointerApplyIntervalMethod paimIntervalMethod;

     public:
      ConstantAndIntervalApplyMethod(PointerApplyConstantMethod constantMethod,
         PointerApplyIntervalMethod intervalMethod)
         :  pacmConstantMethod(constantMethod), paimIntervalMethod(intervalMethod) {}
      ConstantAndIntervalApplyMethod(const ConstantAndIntervalApplyMethod& source) = default;
      ConstantAndIntervalApplyMethod& operator=(const ConstantAndIntervalApplyMethod& source) = default;
      DefineCopy(ConstantAndIntervalApplyMethod)
      DDefineAssign(ConstantAndIntervalApplyMethod)
         
      virtual bool execute(Interval& object, const VirtualOperation& operation, EvaluationEnvironment& env) const override
         {  auto kind = env.getFirstArgument().getApproxKind();
            return (kind.isConstant())
               ? (object.*pacmConstantMethod)((const Operation&) operation, env.getFirstArgument(), env) : (kind.isInterval()
               ? (object.*paimIntervalMethod)((const Operation&) operation,
                     (const Details::IntegerInterval&) env.getFirstArgument(), env)
               : ((const VirtualElement&) env.getFirstArgument()).applyTo((const Operation&) operation, object, env));
         }
   };
   
   class ConstantIntervalAndGenericConstraintCompareMethod : public Details::MultiConstraintCompareMethod<DomainTraits> {
     private:
      typedef Details::MultiConstraintCompareMethod<DomainTraits> inherited;
      typedef bool (Interval::*PointerConstraintCompareConstantMethod)
         (const VirtualOperation&, VirtualElement&, VirtualElement::Argument, ConstraintEnvironment&);
      typedef bool (Interval::*PointerConstraintCompareIntervalMethod)
         (const VirtualOperation&, Approximate::Details::IntegerInterval&, VirtualElement::Argument, ConstraintEnvironment&);
      typedef bool (Interval::*PointerConstraintCompareGenericMethod)
         (const VirtualOperation&, VirtualElement&, VirtualElement::Argument, ConstraintEnvironment&);

      PointerConstraintCompareConstantMethod pcccmConstantMethod;
      PointerConstraintCompareIntervalMethod pccimIntervalMethod;
      PointerConstraintCompareGenericMethod pccgmGenericMethod;
      
     protected:
      friend class MethodConstraintTable;

     public:
      ConstantIntervalAndGenericConstraintCompareMethod(
         PointerConstraintCompareConstantMethod constantMethod,
         PointerConstraintCompareIntervalMethod intervalMethod,
         PointerConstraintCompareGenericMethod genericMethod)
         :  pcccmConstantMethod(constantMethod), pccimIntervalMethod(intervalMethod),
            pccgmGenericMethod(genericMethod) {}

      bool execute(Interval& object, const VirtualOperation& operation, VirtualElement& source, Argument arg, ConstraintEnvironment& env) const
         {  return source.getApproxKind().isConstant()
               ? (object.*pcccmConstantMethod)(operation, source, arg, env) : ((source.getApproxKind().isInterval())
               ? (object.*pccimIntervalMethod)(operation, (Details::IntegerInterval&) source, arg, env)
               : (object.*pccgmGenericMethod)(operation, source, arg, env));
         }
   };

  public:
   Interval(const Init& init) : inherited(init) {}
   Interval(const Interval& source) = default;
   DefineCopy(Interval)
   virtual const FunctionQueryTable& functionQueryTable(int queryIndex) const override
      {  return ((queryIndex == VirtualQueryOperation::TOperation)
               || (queryIndex == VirtualQueryOperation::TDomain) || (queryIndex == VirtualQueryOperation::TExtern))
            ? getMin().functionQueryTable(queryIndex)
            : mqtMethodQueryTable[queryIndex];
      }

   virtual bool query(const VirtualQueryOperation&, VirtualQueryOperation::Environment& env) const override;
   virtual bool apply(const VirtualOperation& operation, Scalar::EvaluationEnvironment& env) override;
   virtual bool applyTo(const VirtualOperation&, VirtualElement&, EvaluationEnvironment& env) const override;
   virtual bool mergeWith(const Scalar::VirtualElement& source, Scalar::EvaluationEnvironment& env) override;
   virtual bool contain(const Scalar::VirtualElement& source, Scalar::EvaluationEnvironment& env) const override;
   virtual bool mergeWithTo(VirtualElement&, EvaluationEnvironment&) const override;
   virtual bool containTo(const VirtualElement&, EvaluationEnvironment&) const override;
   virtual bool intersectWith(const VirtualElement& source, EvaluationEnvironment& env) override;
   virtual bool intersectWithTo(VirtualElement&, EvaluationEnvironment&) const override;
   virtual bool constraintTo(const VirtualOperation&, VirtualElement& source, const VirtualElement& result,
         ConstraintEnvironment&) override;
   virtual bool constraint(const VirtualOperation&, const VirtualElement& result, ConstraintEnvironment&) override;
};

}} // end of namespace Integer::Approximate

namespace MultiBit { namespace Approximate {

namespace Details {

using IntegerInterval = Scalar::Approximate::Details::IntegerInterval;
using BaseConstants = Scalar::Approximate::Details::BaseConstants;
template <class TypeDomain> using TDomainTraits = Scalar::Approximate::Details::TDomainTraits<TypeDomain>;
template <class TypeDomainTraits> using MethodApplyTable = Scalar::Approximate::Details::MethodApplyTable<TypeDomainTraits>;
template <class TypeDomainTraits> using MethodApplyToTable = Scalar::Approximate::Details::MethodApplyToTable<TypeDomainTraits>;
template <class TypeDomainTraits> using MethodConstraintTable = Scalar::Approximate::Details::MethodConstraintTable<TypeDomainTraits>;
template <class TypeDomainTraits> using MethodQueryTable = Scalar::Approximate::Details::MethodQueryTable<TypeDomainTraits>;
template <class TypeDomainTraits> using MultiApplyMethod = Scalar::Approximate::Details::MultiApplyMethod<TypeDomainTraits>;
template <class TypeDomainTraits> using MultiConstraintCompareMethod = Scalar::Approximate::Details::MultiConstraintCompareMethod<TypeDomainTraits>;

} // end of namespace Details

class Interval : public TVirtualElement<Details::IntegerInterval> {
  public:
   typedef TVirtualElement<Details::IntegerInterval> IntegerInterval;

  private:
   typedef Interval thisType;
   typedef TVirtualElement<Details::IntegerInterval> inherited;

  public:
   typedef MultiBit::Operation Operation;

   class Constants : public inherited::Constants, public Details::BaseConstants {
     private:
      typedef MultiBit::Operation Operation;

     public:
      static const int NBUnaryConstraintMethods = Operation::EndOfUnary + 3; // OppositeSigned, OppositeFloat, BitNegate
      static int convertUnaryConstraintToOperationToIndex(Operation::Type type)
         {  return (type < Operation::EndOfUnary) ? type
               : ((type == Operation::TOppositeSignedAssign) ? Operation::EndOfUnary
               : ((type == Operation::TOppositeFloatAssign) ? (1+Operation::EndOfUnary)
               : ((type == Operation::TBitNegateAssign) ? (2+Operation::EndOfUnary) : -1)));
         }
      static const int NBBinaryConstraintMethods
         = Operation::EndOfType - Operation::EndOfCompare + Operation::StartOfCompare;
      static int convertBinaryConstraintOperationToIndex(Operation::Type type)
         {  return (type < Operation::EndOfBinary)
               ? ((type >= Operation::StartOfBinary) ? (type - Operation::StartOfBinary+2)
                  : ((type == Operation::TConcat) ? 0 : 1))
               : (type - Operation::EndOfCompare + Operation::EndOfBinary - Operation::StartOfBinary+2);
         }
      static const int NBConstraintCompareMethods
         = Scalar::MultiBit::Operation::EndOfCompare - Scalar::MultiBit::Operation::StartOfCompare;
      static int convertCompareConstraintOperationToIndex(Scalar::MultiBit::Operation::Type type)
         {  return type - Scalar::MultiBit::Operation::StartOfCompare; }

      static const int NBApplyMethods = Operation::EndOfType;
      static int convertApplyOperationToIndex(Operation::Type type) { return type; }
      static const int NBApplyToMethods = Operation::EndOfType - Operation::StartOfBinary + 2;
      static int convertApplyToOperationToIndex(Operation::Type type)
         {  return (type >= Operation::StartOfBinary) ? (type - Operation::StartOfBinary + 2)
                  : ((type == Operation::TConcat) ? 0 : ((type == Operation::TBitSet) ? 1 : -1));
         }
   };
   typedef Details::TDomainTraits<Interval> DomainTraits;
   
  private:
   class MethodApplyTable : public Details::MethodApplyTable<DomainTraits> {
     public:
      MethodApplyTable();
   };
   class MethodApplyToTable : public Details::MethodApplyToTable<DomainTraits> {
     public:
      MethodApplyToTable();
   };
   class MethodConstraintTable : public Details::MethodConstraintTable<DomainTraits> {
     public:
      MethodConstraintTable();
   };

   static MethodApplyTable matMethodApplyTable;
   static MethodApplyToTable mattMethodApplyToTable;
   static MethodConstraintTable mctMethodConstraintTable;

  public:
   class QueryTable : public inherited::QueryTable {
     public:
      QueryTable();
   };
   friend class QueryTable;
   static QueryTable mqtMethodQueryTable;

  protected:
   class ConstantAndIntervalApplyMethod : public Details::MultiApplyMethod<DomainTraits> {
     private:
      typedef bool (Interval::*PointerApplyConstantMethod)
         (const VirtualOperation&, const VirtualElement&, EvaluationEnvironment&);
      typedef bool (Interval::*PointerApplyIntervalMethod)
         (const VirtualOperation&, const Approximate::Details::IntegerInterval&, EvaluationEnvironment&);

      typedef Approximate::Details::MultiApplyMethod<DomainTraits> inherited;
      PointerApplyConstantMethod pacmConstantMethod;
      PointerApplyIntervalMethod paimIntervalMethod;

     public:
      ConstantAndIntervalApplyMethod(PointerApplyConstantMethod constantMethod,
            PointerApplyIntervalMethod intervalMethod)
         :  pacmConstantMethod(constantMethod), paimIntervalMethod(intervalMethod) {}
      ConstantAndIntervalApplyMethod(const ConstantAndIntervalApplyMethod& source) = default;
      ConstantAndIntervalApplyMethod& operator=(const ConstantAndIntervalApplyMethod& source) = default;
      DefineCopy(ConstantAndIntervalApplyMethod)
      DDefineAssign(ConstantAndIntervalApplyMethod)
         
      virtual bool execute(Interval& object, const VirtualOperation& operation, EvaluationEnvironment& env) const override
         {  auto kind = env.getFirstArgument().getApproxKind();
            return (kind.isConstant())
               ? (object.*pacmConstantMethod)((const Operation&) operation, env.getFirstArgument(), env) : (kind.isInterval()
               ? (object.*paimIntervalMethod)((const Operation&) operation,
                     (const Details::IntegerInterval&) env.getFirstArgument(), env)
               : ((const VirtualElement&) env.getFirstArgument()).applyTo((const Operation&) operation, object, env));
         }
   };
   
   class ConstantIntervalAndGenericConstraintCompareMethod : public Details::MultiConstraintCompareMethod<DomainTraits> {
     private:
      typedef bool (Interval::*PointerConstraintCompareConstantMethod)
         (const VirtualOperation&, VirtualElement&, VirtualElement::Argument, ConstraintEnvironment&);
      typedef bool (Interval::*PointerConstraintCompareIntervalMethod)
         (const VirtualOperation&, Approximate::Details::IntegerInterval&, VirtualElement::Argument, ConstraintEnvironment&);
      typedef bool (Interval::*PointerConstraintCompareGenericMethod)
         (const VirtualOperation&, VirtualElement&, VirtualElement::Argument, ConstraintEnvironment&);

      PointerConstraintCompareConstantMethod pcccmConstantMethod;
      PointerConstraintCompareIntervalMethod pccimIntervalMethod;
      PointerConstraintCompareGenericMethod pccgmGenericMethod;
      
     protected:
      friend class MethodConstraintTable;

     public:
      ConstantIntervalAndGenericConstraintCompareMethod(
         PointerConstraintCompareConstantMethod constantMethod,
         PointerConstraintCompareIntervalMethod intervalMethod,
         PointerConstraintCompareGenericMethod genericMethod)
         :  pcccmConstantMethod(constantMethod), pccimIntervalMethod(intervalMethod),
            pccgmGenericMethod(genericMethod) {}

      bool execute(Interval& object, const VirtualOperation& operation, VirtualElement& source, Argument arg, ConstraintEnvironment& env) const
         {  return source.getApproxKind().isConstant()
               ? (object.*pcccmConstantMethod)(operation, source, arg, env) : ((source.getApproxKind().isInterval())
               ? (object.*pccimIntervalMethod)(operation, (Details::IntegerInterval&) source, arg, env)
               : (object.*pccgmGenericMethod)(operation, source, arg, env));
         }
   };

  public:
   Interval(const Init& init) : inherited(init) {}
   Interval(const Interval& source) = default;
   DefineCopy(Interval)
   virtual const FunctionQueryTable& functionQueryTable(int queryIndex) const override
      {  return ((queryIndex == VirtualQueryOperation::TOperation)
               || (queryIndex == VirtualQueryOperation::TDomain) || (queryIndex == VirtualQueryOperation::TExtern))
            ? getMin().functionQueryTable(queryIndex)
            : mqtMethodQueryTable[queryIndex];
      }
   virtual bool isUnsignedTimesOperation(const VirtualOperation& operation) const override
      {  auto type = ((const Operation&) operation).getType();
         AssumeCondition(type == Operation::TTimesSignedAssign || type == Operation::TTimesUnsignedAssign)
         return type == Operation::TTimesUnsignedAssign;
      }

   bool applyCastShiftBit(const VirtualOperation& operation, EvaluationEnvironment& env);
   bool applyCastMultiFloat(const VirtualOperation& operation, EvaluationEnvironment& env);
   bool applyCastMultiFloatPointer(const VirtualOperation& operation, EvaluationEnvironment& env);
   bool constraintCastShiftBit(const VirtualOperation& operation, const VirtualElement& result, ConstraintEnvironment& env);
   bool constraintCastMultiFloat(const VirtualOperation& operation, const VirtualElement& result, ConstraintEnvironment& env);
   bool constraintCastMultiFloatPointer(const VirtualOperation& operation, const VirtualElement& result, ConstraintEnvironment& env);
      
   virtual bool query(const VirtualQueryOperation&, VirtualQueryOperation::Environment& env) const override;
   virtual bool apply(const VirtualOperation& operation, Scalar::EvaluationEnvironment& env) override;
   virtual bool applyTo(const VirtualOperation&, VirtualElement&, EvaluationEnvironment& env) const override;
   virtual bool mergeWith(const Scalar::VirtualElement& source, Scalar::EvaluationEnvironment& env) override;
   virtual bool contain(const Scalar::VirtualElement& source, Scalar::EvaluationEnvironment& env) const override;
   virtual bool mergeWithTo(VirtualElement&, EvaluationEnvironment&) const override;
   virtual bool containTo(const VirtualElement&, EvaluationEnvironment&) const override;
   virtual bool intersectWith(const VirtualElement& source, EvaluationEnvironment& env) override;
   virtual bool intersectWithTo(VirtualElement&, EvaluationEnvironment&) const override;
   virtual bool constraintTo(const VirtualOperation&, VirtualElement& source, const VirtualElement& result,
         ConstraintEnvironment&) override;
   virtual bool constraint(const VirtualOperation&, const VirtualElement& result, ConstraintEnvironment&) override;
};

}} // end of namespace MultiBit::Approximate

}} // end of namespace Analyzer::Scalar

#endif // Analyzer_Scalar_Approximate_IntervalIntegerH
