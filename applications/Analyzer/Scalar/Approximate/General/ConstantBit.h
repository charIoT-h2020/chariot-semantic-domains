/////////////////////////////////
//
// Library   : Analyzer/Scalar
// Unit      : approximate native scalar elements
// File      : ConstantBit.h
// Author    : Franck Vedrine
// Copyright : CEA LIST - 2019-2020 - all rights reserved
//
// Description :
//   Definition of a class of bit elements that relies on independent host analyses.
//

#ifndef Analyzer_Scalar_Approximate_ConstantBitH
#define Analyzer_Scalar_Approximate_ConstantBitH

#include "Analyzer/Scalar/Approximate/VirtualElement.h"
#include "Analyzer/Scalar/Approximate/MethodTable.h"
#include "Analyzer/Scalar/Implementation/Native/IntegerElement.h"

namespace Analyzer { namespace Scalar { namespace Bit { namespace Approximate {

/*******************************************/
/* Definition of the class ConstantElement */
/*******************************************/

namespace DConstantElement {

template <class TypeBase>
class TMethodApplyTable : public TypeBase {
  public:
   TMethodApplyTable();
};

} // end of namespace DConstantElement

class ConstantElement : public TVirtualElement<Details::BitElement> {
  private:
   typedef ConstantElement thisType;
   typedef TVirtualElement<Details::BitElement> inherited;

  protected:
   bool fValue;

  public:
   typedef Bit::Operation Operation;
   typedef Scalar::Approximate::Details::TDomainTraits<ConstantElement> DomainTraits;

   class Constants : public Scalar::Approximate::Details::BaseConstants {
     public:
      static const int NBApplyMethods = Operation::EndOfType;
      static int convertApplyOperationToIndex(Operation::Type type) { return type; }
      
      static const int NBConstraintCompareMethods
         = Operation::EndOfCompare - Operation::StartOfCompare;
      static int convertCompareConstraintOperationToIndex(Operation::Type type)
         {  return type - Operation::StartOfCompare; }
      
      static const int NBBinaryConstraintMethods
         = Operation::EndOfType - Operation::EndOfUnary - NBConstraintCompareMethods;
      static int convertBinaryConstraintOperationToIndex(Operation::Type type)
         {  return (type < Operation::EndOfBinary) ? (type - Operation::EndOfUnary)
               : (type - Operation::EndOfCompare
                     + (Operation::EndOfBinary - Operation::EndOfUnary));
         }

      static const int NBUnaryConstraintMethods = Operation::EndOfUnary + 1;
      static int convertUnaryConstraintToOperationToIndex(Operation::Type type)
         {  return (type < Operation::EndOfUnary) ? type
               : ((type == Operation::TNegateAssign) ? (1+Operation::EndOfUnary) : -1);
         }
   };

  protected:
   template <class TypeDomainTraits>
   class MethodApplyTable : public DConstantElement::TMethodApplyTable<Scalar::Approximate::Details::MethodApplyTable<TypeDomainTraits> > {
     public:
      MethodApplyTable() {}
   };
   class MethodConstraintTable : public Scalar::Approximate::Details::MethodConstraintForConstantsTable<DomainTraits> {
     public:
      MethodConstraintTable();
   };
   friend class Scalar::Approximate::Details::MethodAccess<DomainTraits>;

   static MethodConstraintTable mctMethodConstraintTable;
   virtual int _getSizeInBits() const override { return 1; }

  public:
   class QueryTable : public inherited::QueryTable {
     public:
      QueryTable();
   };

  private:
   static VirtualElement& getFirstArgument(EvaluationEnvironment& env)
      {  return (VirtualElement&) env.getFirstArgument(); }
   
  protected:
   virtual void _write(OSBase& out, const STG::IOObject::FormatParameters& params) const override
      {  out << "Bit::Constant " << (fValue ? "true" : "false"); }
   virtual ComparisonResult _compare(const EnhancedObject& asource) const override
      {  ComparisonResult result = inherited::_compare(asource);
         if (result == CREqual) {
            AssumeCondition(dynamic_cast<const ConstantElement*>(&asource))
            const ConstantElement& source = (const ConstantElement&) asource;
            if (fValue)
               result = source.fValue ? CREqual : CRGreater;
            else
               result = !source.fValue ? CREqual : CRLess;
         };
         return result;
      }

  public:
   ConstantElement(const Init& init) : inherited(const_cast<Init&>(init).set1()), fValue(false)
      {  if (init.hasInitialValue()) {
            AssumeCondition(dynamic_cast<const Scalar::Integer::Implementation::IntElement*>(&init.getInitialValue()))
            fValue = ((const Scalar::Integer::Implementation::IntElement&) init.getInitialValue()).getElement();
         };
         setConstant();
      }
   ConstantElement(const thisType& source) = default;
   ConstantElement& operator=(const thisType& source) = default;
   DefineCopy(ConstantElement)
   DDefineAssign(ConstantElement)

   virtual int countAtomic() const override { return 1; }
   virtual bool isBoolean() const override { return true; }
   static int getSize() { return 1; }
   static bool applyToTop(const VirtualElement& topElement, VirtualElement& thisElement, const VirtualOperation& operation, EvaluationEnvironment& env);
   static bool applyTop(VirtualElement& topElement, const VirtualOperation& operation, EvaluationEnvironment& env);
   static bool constraintTop(VirtualElement& topElement, const VirtualOperation& operation, const VirtualElement& result, ConstraintEnvironment& env);

   virtual bool applyTo(const VirtualOperation&, Approximate::VirtualElement&, EvaluationEnvironment&) const override { AssumeUncalled return false; }
   virtual bool mergeWithTo(Approximate::VirtualElement&, EvaluationEnvironment&) const override { AssumeUncalled return false; }
   virtual bool intersectWithTo(Approximate::VirtualElement&, EvaluationEnvironment&) const override;
   virtual bool constraintTo(const VirtualOperation&, Approximate::VirtualElement& source, const Approximate::VirtualElement&, ConstraintEnvironment&) override;
   virtual ZeroResult queryZeroResult() const override { return ZeroResult().setBool(fValue); }

   virtual bool mergeWith(const Scalar::VirtualElement& source, Scalar::EvaluationEnvironment& env) override;
   virtual bool contain(const Scalar::VirtualElement& source, Scalar::EvaluationEnvironment& env) const override;
   virtual bool intersectWith(const Approximate::VirtualElement& source, EvaluationEnvironment& env) override;
   virtual bool constraint(const VirtualOperation&, const Approximate::VirtualElement&, ConstraintEnvironment& env) override;
   virtual bool guard(PPVirtualElement thenElement, PPVirtualElement elseElement, EvaluationEnvironment& env) override
      {  return guardConstant(fValue, thenElement, elseElement, env); }

   static const thisType& getSameArgument(const EvaluationEnvironment& env)
      {  AssumeCondition(dynamic_cast<const thisType*>(&env.getFirstArgument()))
         return (const thisType&) env.getFirstArgument();
      }

   // Definition of the apply methods
#define DefineDeclareMethod(TypeOperation)                                                         \
bool apply##TypeOperation(const VirtualOperation& operation, EvaluationEnvironment& env);

   DefineDeclareMethod(CastMultiBit)
   DefineDeclareMethod(PrevAssign)
   DefineDeclareMethod(NextAssign)
   DefineDeclareMethod(PlusAssign)
   DefineDeclareMethod(MinusAssign)
   DefineDeclareMethod(CompareLess)
   DefineDeclareMethod(CompareLessOrEqual)
   DefineDeclareMethod(CompareEqual)
   DefineDeclareMethod(CompareDifferent)
   DefineDeclareMethod(CompareGreater)
   DefineDeclareMethod(CompareGreaterOrEqual)
   DefineDeclareMethod(MinAssign)
   DefineDeclareMethod(MaxAssign)
   DefineDeclareMethod(OrAssign)
   DefineDeclareMethod(AndAssign)
   DefineDeclareMethod(ExclusiveOrAssign)
   DefineDeclareMethod(NegateAssign)

#undef DefineDeclareMethod

   // Definition of the constraint methods

#define DefineDeclareConstraintCompareMethod(TypeOperation)                                        \
bool constraint##TypeOperation(const VirtualOperation&, VirtualElement& source, Argument arg, ConstraintEnvironment& env);

#define DefineDeclareConstraintBinaryMethod(TypeOperation)                                         \
bool constraint##TypeOperation(const VirtualOperation&, VirtualElement& source,                    \
      const VirtualElement& result, Argument arg, ConstraintEnvironment& env);

   DefineDeclareConstraintBinaryMethod(PlusAssign)
   DefineDeclareConstraintBinaryMethod(MinusAssign)
   DefineDeclareConstraintCompareMethod(CompareLessOrGreater)
   DefineDeclareConstraintCompareMethod(CompareLessOrEqualOrGreater)
   DefineDeclareConstraintCompareMethod(CompareEqual)
   DefineDeclareConstraintCompareMethod(CompareDifferent)
   DefineDeclareConstraintBinaryMethod(OrAssign)
   DefineDeclareConstraintBinaryMethod(AndAssign)
   DefineDeclareConstraintBinaryMethod(ExclusiveOrAssign)

#undef DefineDeclareConstraintBinaryMethod
#undef DefineDeclareConstraintCompareMethod

   static bool queryCompareSpecial(const VirtualElement&, const VirtualQueryOperation&, VirtualQueryOperation::Environment& env);
   static bool querySimplification(const VirtualElement&, const VirtualQueryOperation&, VirtualQueryOperation::Environment& env);
};

template <class TypeRingTraits>
class TCloseConstantElement : public ConstantElement {
  private:
   typedef ConstantElement inherited;
   typedef TCloseConstantElement<TypeRingTraits> thisType;

  protected:
   bool& value() { return fValue; }
   
  protected:
   typedef Scalar::Approximate::Details::TDomainTraits<thisType> DomainTraits;
   typedef typename inherited::MethodApplyTable<DomainTraits> InheritedMethodApplyTable;
   typedef inherited::QueryTable InheritedQueryTable;
   class MethodApplyTable : public InheritedMethodApplyTable {
     private:
      typedef InheritedMethodApplyTable inherited;
     public:
      MethodApplyTable();
   };
   friend class MethodApplyTable;
   class QueryTable : public InheritedQueryTable {
     private:
      typedef InheritedQueryTable inherited;
     public:
      QueryTable();
   };
   friend class QueryTable;
  public:
   static MethodApplyTable matMethodApplyTable;
   static QueryTable mqtMethodQueryTable;
   friend class Scalar::Approximate::Details::MethodAccess<DomainTraits>;
   
  public:
   TCloseConstantElement(const Init& init) : inherited(init) {}
   TCloseConstantElement(const thisType& source) : inherited(source) {}
   TemplateDefineCopy(TCloseConstantElement, TypeRingTraits)

#define DefineDeclareMethod(TypeOperation)                                                         \
bool apply##TypeOperation(const VirtualOperation& operation, EvaluationEnvironment& env);

   DefineDeclareMethod(CastChar)
   DefineDeclareMethod(CastInt)
   DefineDeclareMethod(CastUnsignedInt)

#undef DefineDeclareMethod

   virtual const FunctionQueryTable& functionQueryTable(int queryIndex) const override { return mqtMethodQueryTable[queryIndex]; }

   static bool applyTop(VirtualElement& topElement, const VirtualOperation& operation, EvaluationEnvironment& env);
   virtual bool query(const VirtualQueryOperation&, VirtualQueryOperation::Environment& env) const override;
   virtual bool apply(const VirtualOperation& operation, Scalar::EvaluationEnvironment& env) override;

   static bool queryNewMultiBitConstant(const VirtualElement&, const VirtualQueryOperation&, VirtualQueryOperation::Environment& env);
   static bool queryNewTop(const VirtualElement&, const VirtualQueryOperation&, VirtualQueryOperation::Environment& env);
   static bool queryNewFalse(const VirtualElement&, const VirtualQueryOperation&, VirtualQueryOperation::Environment& env);
   static bool queryNewTrue(const VirtualElement&, const VirtualQueryOperation&, VirtualQueryOperation::Environment& env);
};

}}}} // end of namespace Analyzer::Scalar::Bit::Approximate

#endif // Analyzer_Scalar_Approximate_ConstantBitH

