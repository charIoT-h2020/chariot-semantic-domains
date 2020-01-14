/////////////////////////////////
//
// Library   : Analyzer/Scalar
// Unit      : approximate native scalar elements
// File      : ConstantIntegerElement.h
// Author    : Franck Vedrine
// Copyright : CEA LIST - 2019-2020 - all rights reserved
//
// Description :
//   Definition of a class of integer elements that rely on native host implementation.
//   All this stuff is rather defined for source code analysis.
//

#ifndef Analyzer_Scalar_Approximate_ConstantIntegerElementH
#define Analyzer_Scalar_Approximate_ConstantIntegerElementH

#include "Analyzer/Scalar/Approximate/Native/ConstantNativeElement.h"
#include "Analyzer/Scalar/Implementation/Native/IntegerElement.h"

namespace Analyzer { namespace Scalar { namespace Integer { namespace Approximate {

namespace Details {

template <class TypeBase, class TypeDerived> using TBaseElement = Scalar::Approximate::Details::TBaseElement<TypeBase, TypeDerived>;
using BaseConstants = Scalar::Approximate::Details::BaseConstants;
template <class TypeDomain> using TDomainTraits = Scalar::Approximate::Details::TDomainTraits<TypeDomain>;
template <class TypeDomainTraits> using MethodApplyTable = Scalar::Approximate::Details::MethodApplyTable<TypeDomainTraits>;
template <class TypeDomainTraits> using MethodConstraintForConstantsTable = Scalar::Approximate::Details::MethodConstraintForConstantsTable<TypeDomainTraits>;
template <class TypeDomainTraits> using MethodQueryTable = Scalar::Approximate::Details::MethodQueryTable<TypeDomainTraits>;

}

#define DefineDeclareMethod(TypeOperation)                                                        \
bool apply##TypeOperation(const VirtualOperation& operation, EvaluationEnvironment& env);
#define DefineDeclareOBinaryMethod(TypeOperation, TypeArg)                                        \
bool apply##TypeOperation##TypeArg(const VirtualOperation& operation, EvaluationEnvironment& env);

#define DefineDeclareConstraintBinaryMethod(TypeOperation)                                         \
bool constraint##TypeOperation(const VirtualOperation& operation, VirtualElement& source,          \
      const VirtualElement& result, ConstraintEnvironment::Argument arg, ConstraintEnvironment& env);

#define DefineDeclareConstraintBinaryMethodConstant(TypeOperation)                                 \
bool constraint##TypeOperation##WithConstantResult(const VirtualOperation& operation, VirtualElement& source,\
      const VirtualElement& result, ConstraintEnvironment::Argument arg, ConstraintEnvironment& env);

#define DefineDeclareConstraintBinaryMethodConstantDouble(TypeOperation)                           \
bool constraint##TypeOperation##WithConstantResultOnArgResult(const VirtualOperation& operation,   \
      VirtualElement& source, const VirtualElement& result, ConstraintEnvironment& env);           \
bool constraint##TypeOperation##WithConstantResultOnArgSource(const VirtualOperation& operation,   \
      VirtualElement& source, const VirtualElement& result, ConstraintEnvironment& env);

class IntElement;

class BaseElement : public TVirtualElement<Details::IntOperationElement> {
  private:
   typedef TVirtualElement<Details::IntOperationElement> inherited;

#ifndef _MSC_VER
  protected:
   // For the template class instantiation
   static VirtualElement* createPromotionElement(int) { return nullptr; }
   typedef CastIntOperation CastDegradation;
#endif

  public:
   BaseElement(const Init& init) : inherited(init) {}
   BaseElement(const BaseElement& source) = default;
   typedef Integer::Operation Operation;
   typedef Approximate::Interval Interval;

   DefineDeclareConstraintBinaryMethod(PlusAssign)
   DefineDeclareConstraintBinaryMethod(MinusAssign)
   virtual int countAtomic() const { return 1; }
   virtual bool constraintFromResultLeftShiftAssignWithConstantResultOnArgSource(IntElement& thisElement, const VirtualOperation&, VirtualElement& sourceElement, ConstraintEnvironment& env) const { AssumeUncalled return false; }
   virtual bool constraintFromResultRightShiftAssignWithConstantResultOnArgSource(IntElement& thisElement, const VirtualOperation&, VirtualElement& sourceElement, ConstraintEnvironment& env) const { AssumeUncalled return false; }
   virtual bool constraintFromResultRotateAssignWithConstantResultOnArgSource(IntElement& thisElement, const VirtualOperation&, VirtualElement& sourceElement, ConstraintEnvironment& env) const { AssumeUncalled return false; }
};

template <class TypeBase, class TypeDerived>
class TBaseElement : public Details::TBaseElement<TypeBase, TypeDerived> {
  private:
   typedef Details::TBaseElement<TypeBase, TypeDerived> inherited;
   typedef TBaseElement<TypeBase, TypeDerived> thisType;
   
  public:
   typedef IntElement SignedDomain;
   VirtualElement* createSignedElement(Implementation::IntElement::SimulatedType value) const
      {  return inherited::createIntElement(VirtualElement::Init().setInitialValue(
            Implementation::IntElement(value)));
      }

  public:
   TBaseElement(const VirtualElement::Init& init) : inherited(init) {}
   TBaseElement(const thisType& source) = default;
  
   typedef typename inherited::Operation Operation;
   
   DefineDeclareMethod(PrevAssign)
   DefineDeclareMethod(NextAssign)
   DefineDeclareMethod(PlusAssign)
   DefineDeclareMethod(MinusAssign)
   DefineDeclareMethod(MinusSigned)
   DefineDeclareMethod(OppositeAssign)
   DefineDeclareMethod(TimesAssign)
   DefineDeclareMethod(DivideAssign)
   DefineDeclareMethod(ModuloAssign)
   DefineDeclareMethod(BitOrAssign)
   DefineDeclareMethod(BitAndAssign)
   DefineDeclareMethod(BitExclusiveOrAssign)
   DefineDeclareMethod(BitNegateAssign)
   DefineDeclareMethod(LeftShiftAssign)
   DefineDeclareMethod(RightShiftAssign)
   DefineDeclareMethod(LeftRotateAssign)
   DefineDeclareMethod(RightRotateAssign)
   DefineDeclareOBinaryMethod(LeftShiftAssign, IntElement)
   DefineDeclareOBinaryMethod(RightShiftAssign, IntElement)
   DefineDeclareOBinaryMethod(LeftRotateAssign, IntElement)
   DefineDeclareOBinaryMethod(RightRotateAssign, IntElement)
   DefineDeclareMethod(Promoted)
   DefineDeclareMethod(PromotedAssign)
   
   DefineDeclareConstraintBinaryMethodConstant(TimesAssign)
   DefineDeclareConstraintBinaryMethodConstantDouble(DivideAssign)
   DefineDeclareConstraintBinaryMethodConstantDouble(ModuloAssign)
   DefineDeclareConstraintBinaryMethodConstant(BitOrAssign)
   DefineDeclareConstraintBinaryMethodConstant(BitAndAssign)
   DefineDeclareConstraintBinaryMethodConstant(BitExclusiveOrAssign)
   bool constraintShiftOrRotateAssignWithConstantResultOnArgResult(const VirtualOperation&, VirtualElement& source, const VirtualElement& result, ConstraintEnvironment& env);
   bool constraintShiftOrRotateAssignWithConstantResult(const VirtualOperation& operation, VirtualElement& source, const VirtualElement& result, ConstraintEnvironment::Argument arg, ConstraintEnvironment& env);
   virtual bool constraintFromResultLeftShiftAssignWithConstantResultOnArgSource(IntElement& thisElement, const VirtualOperation&, VirtualElement& sourceElement, ConstraintEnvironment& env) const;
   virtual bool constraintFromResultRightShiftAssignWithConstantResultOnArgSource(IntElement& thisElement, const VirtualOperation&, VirtualElement& sourceElement, ConstraintEnvironment& env) const;
   virtual bool constraintFromResultRotateAssignWithConstantResultOnArgSource(IntElement& thisElement, const VirtualOperation&, VirtualElement& sourceElement, ConstraintEnvironment& env) const;

   PPVirtualElement newIntegerFromBase2(long int code) const;
   
   bool queryBitDomain(const typename TypeBase::QueryOperation&, typename TypeBase::QueryOperation::Environment& env) const;
   bool queryCompareSpecial(const typename TypeBase::QueryOperation&, typename TypeBase::QueryOperation::Environment& env) const;
   bool queryGuardAll(const typename TypeBase::QueryOperation&, typename TypeBase::QueryOperation::Environment& env) const;

   static bool queryInverseCastOperation(const VirtualElement&, const VirtualQueryOperation&, VirtualQueryOperation::Environment& env);
   static bool queryIsSigned(const VirtualElement& source, const VirtualQueryOperation&, VirtualQueryOperation::Environment& env);
};

/**************************************/
/* Definition of the class IntElement */
/**************************************/

class IntElement : public TBaseElement<BaseElement, IntElement>, protected Implementation::IntElement {
  public:
   typedef Approximate::VirtualElement VirtualElement;
  
  private:
   typedef TBaseElement<BaseElement, IntElement> inherited;

  public:
   typedef Implementation::IntElement inheritedImplementation;
   typedef inheritedImplementation::SimulatedType Implementation;
   friend class TBaseElement<BaseElement, IntElement>;
   friend class BitFieldMask<IntElement>;
   
  public:
   class Constants : public BaseElement::Constants, public Details::BaseConstants {
     public:
      static const int NBApplyMethods = Operation::EndOfType;
      static int convertApplyOperationToIndex(Operation::Type type) { return type; }
      
      static const int NBConstraintCompareMethods
         = Operation::EndOfCompare - Operation::TCompareLess;
      static int convertCompareConstraintOperationToIndex(Operation::Type type)
         {  return type - Operation::TCompareLess; }
      
      static const int NBBinaryConstraintMethods
         = Operation::EndOfType - Operation::EndOfUnary - NBConstraintCompareMethods;
      static int convertBinaryConstraintOperationToIndex(TypeOperation type)
         {  return (type < Operation::EndOfBinary) ? (type - Operation::EndOfUnary)
               : (type - Operation::EndOfCompare + (Operation::EndOfBinary - Operation::EndOfUnary));
         }
   
      static const int NBUnaryConstraintMethods = Operation::EndOfUnary + 3;
      static int convertUnaryConstraintToOperationToIndex(Operation::Type type)
         {  return (type < Operation::EndOfUnary) ? type
               : ((type == Operation::TOppositeAssign) ? (1+Operation::EndOfUnary)
               : ((type == Operation::TBitNegateAssign) ? (2+Operation::EndOfUnary)
               : ((type == Operation::TLogicalNegateAssign) ? (3+Operation::EndOfUnary) : -1)));
         }
   };
   typedef Details::TDomainTraits<IntElement> DomainTraits;
   static inheritedImplementation::SimulatedType getMax() { return inheritedImplementation::getMax(); }
   static inheritedImplementation::SimulatedType getMin() { return inheritedImplementation::getMin(); }

  private:
   template <class TypeOperation>
   static bool setCastToThis(TypeOperation& operation, QueryOperation::InverseCastOperationEnvironment&)
      {  operation.setCastInt(); return true; }
   
  protected:
   template <class TypeDomainTraits>
   class MethodApplyTable : public Details::MethodApplyTable<TypeDomainTraits> {
     private:
      typedef Details::MethodApplyTable<TypeDomainTraits> inherited;
      
     public:
      MethodApplyTable();
   };

   template <class TypeDomainTraits>
   class MethodConstraintTable : public Details::MethodConstraintForConstantsTable<TypeDomainTraits> {
     private:
      typedef Details::MethodConstraintForConstantsTable<TypeDomainTraits> inherited;
     public:
      MethodConstraintTable();
   };
   static void setToNegative(inheritedImplementation::SimulatedType& value) { value = -value; }
   static bool isNegative(inheritedImplementation::SimulatedType value) { return value < 0; }
   virtual void _write(OSBase& out, const STG::IOObject::FormatParameters& params) const override
      {  out << "Int::Constant ";
         out.write(inheritedImplementation::getElement(), false);
      }

  public:
   template <class TypeDomainTraits>
   class MethodQueryTable : public Scalar::Approximate::Details::MethodQueryTable<TypeDomainTraits> {
     private:
      typedef Approximate::Details::MethodQueryTable<TypeDomainTraits> inherited;
     public:
      MethodQueryTable();
   };
   
  private:
   static const IntElement& getSameArgument(const EvaluationEnvironment& env)
      {  AssumeCondition(dynamic_cast<const IntElement*>(&env.getFirstArgument()))
         return (const IntElement&) env.getFirstArgument();
      }
   static VirtualElement& getFirstArgument(EvaluationEnvironment& env)
      { return (VirtualElement&) env.getFirstArgument(); }

  protected:
   virtual int _getSizeInBits() const override { return getSize()*8; }
   virtual ComparisonResult _compare(const EnhancedObject& asource) const override
      {  ComparisonResult result = inherited::_compare(asource);
         return (result == CREqual)
            ? inheritedImplementation::_compare((const inheritedImplementation&) castFromCopyHandler(asource))
            : result;
      }

  public:
   IntElement(const Init& init) : inherited(init), inheritedImplementation(0)
      {  if (init.hasInitialValue()) {
            AssumeCondition(dynamic_cast<const Scalar::Implementation::ImplementationElement*>(&init.getInitialValue()))
            const auto& implementation = (const Scalar::Implementation::ImplementationElement&)
               init.getInitialValue();
            if (implementation.isConstantSigned()) {
               AssumeCondition(dynamic_cast<const inheritedImplementation*>(&implementation))
               inheritedImplementation::_assign((const inheritedImplementation&) implementation);
            }
            else if (implementation.isMultiBit()) {
               AssumeCondition(dynamic_cast<const Scalar::MultiBit::Implementation::MultiBitElement*>(&implementation))
               inheritedImplementation::setElement((int) ((const Scalar::MultiBit::Implementation::MultiBitElement&)
                  implementation)[0]);
            }
            else
               AssumeUnimplemented
         };
         setConstant();
      }
   IntElement(const IntElement& source) = default;
   IntElement& operator=(const IntElement& source) = default;
   DefineCopy(IntElement)
   DDefineAssign(IntElement)
   DCompare(IntElement)
   StaticInheritConversions(IntElement, inherited)

   template <class TypeRingTraits> static VirtualElement* createConstantElement(TypeRingTraits, const VirtualElement::Init& init)
      {  return TypeRingTraits::createIntElement(init); }
   int getInt() const { return inheritedImplementation::getElement(); }
   const inheritedImplementation& getImplementation() const { return (const inheritedImplementation&) *this; }
   inheritedImplementation& getSImplementation() { return (inheritedImplementation&) *this; }

   static int getSize() { return inheritedImplementation::getSize(); }
   virtual bool apply(const VirtualOperation& operation, Scalar::EvaluationEnvironment& env) override
      {  return inherited::apply(operation, env); }

   DefineDeclareMethod(LogicalAndAssign)
   DefineDeclareMethod(LogicalOrAssign)
   DefineDeclareMethod(LogicalNegateAssign)
   bool constraintLeftShiftAssignWithConstantResultOnArgSource(const VirtualOperation& operation, VirtualElement& source, const VirtualElement& result, ConstraintEnvironment& env);
   bool constraintRightShiftAssignWithConstantResultOnArgSource(const VirtualOperation& operation, VirtualElement& source, const VirtualElement& result, ConstraintEnvironment& env);
   bool constraintRotateAssignWithConstantResultOnArgSource(const VirtualOperation& operation, VirtualElement& source, const VirtualElement& result, ConstraintEnvironment& env);
};

/**********************************************/
/* Definition of the class UnsignedIntElement */
/**********************************************/

class UnsignedIntElement : public TBaseElement<BaseElement, UnsignedIntElement>, protected Implementation::UnsignedIntElement {
  public:
   typedef Approximate::VirtualElement VirtualElement;
   typedef TBaseElement<BaseElement, UnsignedIntElement> inherited;
  
  public:
   typedef Implementation::UnsignedIntElement inheritedImplementation;
   typedef inheritedImplementation::SimulatedType Implementation;
   
  public:
   class Constants : public BaseElement::Constants, public Approximate::Details::BaseConstants {
     public:
      static const int NBApplyMethods = Operation::EndOfType;
      static int convertApplyOperationToIndex(TypeOperation type) { return type; }
      
      static const int NBConstraintCompareMethods
         = Operation::EndOfCompare - Operation::TCompareLess;
      static int convertCompareConstraintOperationToIndex(TypeOperation type)
         {  return type - Operation::TCompareLess; }
      
      static const int NBBinaryConstraintMethods
         = Operation::EndOfType - Operation::EndOfUnary - NBConstraintCompareMethods;
      static int convertBinaryConstraintOperationToIndex(TypeOperation type)
         {  return (type < Operation::EndOfBinary) ? (type - Operation::EndOfUnary)
               : (type - Operation::EndOfCompare + (Operation::EndOfBinary - Operation::EndOfUnary));
         }
   
      static const int NBUnaryConstraintMethods = Operation::EndOfUnary + 2;
      static int convertUnaryConstraintToOperationToIndex(TypeOperation type)
         {  return (type < Operation::EndOfUnary) ? type
               : ((type == Operation::TBitNegateAssign) ? (1+Operation::EndOfUnary)
               : ((type == Operation::TLogicalNegateAssign) ? (2+Operation::EndOfUnary) : -1));
         }
   };
   typedef Approximate::Details::TDomainTraits<UnsignedIntElement> DomainTraits;

  protected:
   template <class TypeOperation>
   static bool setCastToThis(TypeOperation& operation, QueryOperation::InverseCastOperationEnvironment&)
      {  operation.setCastUnsignedInt(); return true; }
   friend class TBaseElement<BaseElement, UnsignedIntElement>;
   friend class BitFieldMask<UnsignedIntElement>;
   
   template <class TypeDomainTraits>
   class MethodApplyTable : public Approximate::Details::MethodApplyTable<TypeDomainTraits> {
     private:
      typedef Approximate::Details::MethodApplyTable<TypeDomainTraits> inherited;
     public:
      MethodApplyTable();
   };
   template <class TypeDomainTraits>
   class MethodConstraintTable : public Approximate::Details::MethodConstraintForConstantsTable<TypeDomainTraits> {
     private:
      typedef Approximate::Details::MethodConstraintForConstantsTable<TypeDomainTraits> inherited;
     public:
      MethodConstraintTable();
   };

   virtual void _write(OSBase& out, const STG::IOObject::FormatParameters& params) const override
      {  out << "UnsignedInt::Constant ";
         out.write(inheritedImplementation::getElement(), false);
      }

  public:
   template <class TypeDomainTraits>
   class MethodQueryTable : public Details::MethodQueryTable<TypeDomainTraits> {
     private:
      typedef Details::MethodQueryTable<TypeDomainTraits> inherited;
     public:
      MethodQueryTable();
   };

  private:
   static const UnsignedIntElement& getSameArgument(const EvaluationEnvironment& env)
      {  AssumeCondition(dynamic_cast<const UnsignedIntElement*>(&env.getFirstArgument()))
         return (const UnsignedIntElement&) env.getFirstArgument();
      }
   static VirtualElement& getFirstArgument(EvaluationEnvironment& env)
      { return (VirtualElement&) env.getFirstArgument(); }

  protected:
   virtual int _getSizeInBits() const override { return getSize()*8; }
   virtual ComparisonResult _compare(const EnhancedObject& asource) const override
      {  ComparisonResult result = inherited::_compare(asource);
         return (result == CREqual)
            ? inheritedImplementation::_compare((const inheritedImplementation&) castFromCopyHandler(asource))
            : result;
      }
   static void setToNegative(inheritedImplementation::SimulatedType& value) { AssumeUncalled }
   static bool isNegative(inheritedImplementation::SimulatedType value) { return false; }

  public:
   UnsignedIntElement(const Init& init) : inherited(init), inheritedImplementation(0)
      {  if (init.hasInitialValue()) {
            AssumeCondition(dynamic_cast<const Scalar::Implementation::ImplementationElement*>(&init.getInitialValue()))
            const auto& implementation = (const Scalar::Implementation::ImplementationElement&)
               init.getInitialValue();
            if (implementation.isConstantUnsigned()) {
               AssumeCondition(dynamic_cast<const inheritedImplementation*>(&implementation))
               inheritedImplementation::_assign((const inheritedImplementation&) implementation);
            }
            else if (implementation.isMultiBit()) {
               AssumeCondition(dynamic_cast<const Scalar::MultiBit::Implementation::MultiBitElement*>(&implementation))
               inheritedImplementation::setElement(((const Scalar::MultiBit::Implementation::MultiBitElement&)
                  implementation)[0]);
            }
            else
               AssumeUnimplemented
         };
         setConstant();
      }
   UnsignedIntElement(const UnsignedIntElement& source) = default;
   UnsignedIntElement& operator=(const UnsignedIntElement& source) = default;
   DefineCopy(UnsignedIntElement)
   DDefineAssign(UnsignedIntElement)
   DCompare(UnsignedIntElement)
   StaticInheritConversions(UnsignedIntElement, inherited)

   typedef IntElement SignedDomain;
   VirtualElement* createSignedElement(SignedDomain::Implementation value) const
      {  return createIntElement(VirtualElement::Init().setInitialValue(
            Integer::Implementation::IntElement(value)));
      }

   template <class TypeRingTraits> static VirtualElement* createConstantElement(TypeRingTraits, const VirtualElement::Init& init)
      {  return TypeRingTraits::createUnsignedIntElement(init); }
   const inheritedImplementation& getImplementation() const { return (const inheritedImplementation&) *this; }
   inheritedImplementation& getSImplementation() { return (inheritedImplementation&) *this; }
   static int getSize() { return inheritedImplementation::getSize(); }
   virtual bool apply(const VirtualOperation& operation, Scalar::EvaluationEnvironment& env) override
      {  return inherited::apply(operation, env); }
};

/***************************************/
/* Definition of the class CharElement */
/***************************************/

namespace Details {

class SubElement : public BaseElement {
  private:
   typedef BaseElement inherited;

  public:
   typedef Integer::Operation Operation;
   class Constants : public BaseElement::Constants, public Approximate::Details::BaseConstants {
     public:
      static const int NBApplyMethods = Operation::EndOfType;
      static int convertApplyOperationToIndex(TypeOperation type) { return type; }
   
      static const int NBConstraintCompareMethods
         = Operation::EndOfCompare - Operation::TCompareLess;
      static int convertCompareConstraintOperationToIndex(TypeOperation type)
         {  return type - Operation::TCompareLess; }
   
      static const int NBBinaryConstraintMethods
         = Operation::EndOfType - Operation::EndOfUnary - NBConstraintCompareMethods;
      static int convertBinaryConstraintOperationToIndex(TypeOperation type)
         {  return (type < Operation::EndOfBinary) ? (type - Operation::EndOfUnary)
               : (type - Operation::EndOfCompare + (Operation::EndOfBinary - Operation::EndOfUnary));
         }
   
      static const int NBUnaryConstraintMethods = Operation::EndOfUnary + 3;
      static int convertUnaryConstraintToOperationToIndex(TypeOperation type)
         {  return (type < Operation::EndOfUnary) ? type
               : ((type == Operation::TOppositeAssign) ? (1+Operation::EndOfUnary)
               : ((type == Operation::TBitNegateAssign) ? (2+Operation::EndOfUnary)
               : ((type == Operation::TLogicalNegateAssign) ? (3+Operation::EndOfUnary) : -1)));
         }
   };
   
  protected:
   SubElement(const Init& init) : inherited(init) {}
   
  public:
   SubElement(const SubElement& source) = default;
};

} // end of namespace Details

#define DefineSubElement(TypeCast, TypePromotion, CodeSetToNegative, CodeIsNegative) \
namespace Details { namespace D##TypeCast##Element {                                             \
                                                                                                 \
typedef Approximate::Details::IntOperationElement::QueryOperation QueryOperation;                \
                                                                                                 \
template <class TypeOperation>                                                                   \
inline bool setCastToThis(TypeOperation& operation, QueryOperation::InverseCastOperationEnvironment&)\
   {  operation.setCast##TypeCast(); return true; }                                              \
                                                                                                 \
template <>                                                                                      \
inline bool setCastToThis<Scalar::MultiBit::Operation>(Scalar::MultiBit::Operation& operation, QueryOperation::InverseCastOperationEnvironment& env)\
   {  return false; }                                                                            \
                                                                                                 \
DefineCastBitToThis                                                                              \
}}                                                                                               \
                                                                                                 \
class TypeCast##Element : public TBaseElement<Details::SubElement, TypeCast##Element>, protected Implementation::TypeCast##Element {\
  public:                                                                                        \
   typedef Approximate::VirtualElement VirtualElement;                                           \
   typedef Implementation::TypeCast##Element inheritedImplementation;                            \
   typedef Approximate::Details::TDomainTraits<TypeCast##Element> DomainTraits;                  \
   typedef inheritedImplementation::SimulatedType Implementation;                                \
                                                                                                 \
  private:                                                                                       \
   typedef TBaseElement<Details::SubElement, TypeCast##Element> inherited;                       \
   typedef Cast##TypeCast##Operation CastDegradation;                                            \
                                                                                                 \
   template <class TypeOperation>                                                                \
   static bool setCastToThis(TypeOperation& operation, QueryOperation::InverseCastOperationEnvironment& env)\
      {  return Details::D##TypeCast##Element::setCastToThis(operation, env); }                  \
   friend class TBaseElement<Details::SubElement, TypeCast##Element>;                            \
   friend class BitFieldMask<TypeCast##Element>;                                                 \
   static const TypeCast##Element& getSameArgument(const EvaluationEnvironment& env)             \
      {  AssumeCondition(dynamic_cast<const TypeCast##Element*>(&env.getFirstArgument()))        \
         return (const TypeCast##Element&) env.getFirstArgument();                               \
      }                                                                                          \
                                                                                                 \
  protected:                                                                                     \
   template <class TypeDomainTraits>                                                             \
   class MethodApplyTable : public Approximate::Details::MethodApplyTable<TypeDomainTraits> {    \
     private:                                                                                    \
      typedef Approximate::Details::MethodApplyTable<TypeDomainTraits> inherited;                \
     public:                                                                                     \
      MethodApplyTable();                                                                        \
   };                                                                                            \
   template <class TypeDomainTraits>                                                             \
   class MethodConstraintTable : public Approximate::Details::MethodConstraintForConstantsTable<TypeDomainTraits> {\
     private:                                                                                    \
      typedef Approximate::Details::MethodConstraintForConstantsTable<TypeDomainTraits> inherited;\
     public:                                                                                     \
      MethodConstraintTable();                                                                   \
   };                                                                                            \
                                                                                                 \
  public:                                                                                        \
   template <class TypeDomainTraits>                                                             \
   class MethodQueryTable : public Approximate::Details::MethodQueryTable<TypeDomainTraits> {    \
     private:                                                                                    \
      typedef Approximate::Details::MethodQueryTable<TypeDomainTraits> inherited;                \
     public:                                                                                     \
      MethodQueryTable();                                                                        \
   };                                                                                            \
                                                                                                 \
   virtual void _write(OSBase& out, const STG::IOObject::FormatParameters& params) const override\
      {  out << #TypeCast "Constant ";                                                           \
         out.write((int) inheritedImplementation::getElement(), false);                          \
      }                                                                                          \
                                                                                                 \
  protected:                                                                                     \
   virtual int _getSizeInBits() const override { return getSize()*8; }                           \
   virtual ComparisonResult _compare(const EnhancedObject& asource) const override               \
      {  ComparisonResult result = inherited::_compare(asource);                                 \
         return (result == CREqual)                                                              \
            ? inheritedImplementation::_compare((const inheritedImplementation&) castFromCopyHandler(asource))\
            : result;                                                                            \
      }                                                                                          \
   static void setToNegative(inheritedImplementation::SimulatedType& value) { CodeSetToNegative; }\
   static bool isNegative(inheritedImplementation::SimulatedType value) { return CodeIsNegative; }\
                                                                                                 \
  public:                                                                                        \
   TypeCast##Element(const Init& init)                                                           \
      : inherited(init), inheritedImplementation(0)                                              \
      {  if (init.hasInitialValue()) {                                                           \
            AssumeCondition(dynamic_cast<const Scalar::Implementation::ImplementationElement*>(&init.getInitialValue()))\
            const auto& implementation = (const Scalar::Implementation::ImplementationElement&)  \
               init.getInitialValue();                                                           \
            if (implementation.isConstantUnsigned()) {                                           \
               AssumeCondition(dynamic_cast<const inheritedImplementation*>(&implementation))    \
               inheritedImplementation::_assign((const inheritedImplementation&) implementation);\
            }                                                                                    \
            else if (implementation.isMultiBit()) {                                              \
               AssumeCondition(dynamic_cast<const Scalar::MultiBit::Implementation::MultiBitElement*>(&implementation))\
               inheritedImplementation::setElement(((const Scalar::MultiBit::Implementation::MultiBitElement&)\
                  implementation)[0]);                                                           \
            }                                                                                    \
            else                                                                                 \
               AssumeUnimplemented                                                               \
         };                                                                                      \
         setConstant();                                                                          \
      }                                                                                          \
   TypeCast##Element(const TypeCast##Element& source) = default;                                 \
   DefineCopy(TypeCast##Element)                                                                 \
   DDefineAssign(TypeCast##Element)                                                              \
   DCompare(TypeCast##Element)                                                                   \
   StaticInheritConversions(TypeCast##Element, inherited)                                        \
                                                                                                 \
   static inheritedImplementation::SimulatedType getMax() { return inheritedImplementation::getMax(); }\
   static inheritedImplementation::SimulatedType getMin() { return inheritedImplementation::getMin(); }\
                                                                                                 \
   virtual bool apply(const VirtualOperation& operation, Scalar::EvaluationEnvironment& env) override\
      {  return inherited::apply(operation, env); }                                              \
                                                                                                 \
   const inheritedImplementation& getImplementation() const { return (const inheritedImplementation&) *this; }\
   inheritedImplementation& getSImplementation() { return (inheritedImplementation&) *this; }    \
   static int getSize() { return inheritedImplementation::getSize(); }                           \
                                                                                                 \
   template <class TypeRingTraits> static VirtualElement* createConstantElement(TypeRingTraits, const VirtualElement::Init& init)\
      {  return TypeRingTraits::create##TypeCast##Element(init); }                               \
   VirtualElement* createPromotionElement(inheritedImplementation::SimulatedType value) const    \
      {  return create##TypePromotion##Element(VirtualElement::Init().setInitialValue(           \
            Integer::Implementation::TypePromotion##Element(value)));                                     \
      }                                                                                          \
   DefineSignedCast                                                                              \
};

#define DefineLongElement(TypeCast, CodeSetToNegative, CodeIsNegative)                           \
namespace Details { namespace D##TypeCast##Element {                                             \
                                                                                                 \
typedef Approximate::Details::IntOperationElement::QueryOperation QueryOperation;                \
                                                                                                 \
template <class TypeOperation>                                                                   \
inline bool setCastToThis(TypeOperation& operation, QueryOperation::InverseCastOperationEnvironment&)\
   {  operation.setCast##TypeCast(); return true; }                                              \
                                                                                                 \
template <>                                                                                      \
inline bool setCastToThis<Scalar::MultiBit::Operation>(Scalar::MultiBit::Operation& operation, QueryOperation::InverseCastOperationEnvironment& env)\
   {  return false; }                                                                            \
                                                                                                 \
DefineCastBitToThis                                                                              \
}}                                                                                               \
                                                                                                 \
class TypeCast##Element : public TBaseElement<Details::SubElement, TypeCast##Element>, protected Implementation::TypeCast##Element {\
  public:                                                                                        \
   typedef Approximate::VirtualElement VirtualElement;                                           \
   typedef Implementation::TypeCast##Element inheritedImplementation;                            \
   typedef Approximate::Details::TDomainTraits<TypeCast##Element> DomainTraits;                  \
   typedef inheritedImplementation::SimulatedType Implementation;                                \
                                                                                                 \
  private:                                                                                       \
   typedef TBaseElement<Details::SubElement, TypeCast##Element> inherited;                       \
   typedef Cast##TypeCast##Operation CastDegradation;                                            \
                                                                                                 \
   template <class TypeOperation>                                                                \
   static bool setCastToThis(TypeOperation& operation, QueryOperation::InverseCastOperationEnvironment& env)\
      {  return Details::D##TypeCast##Element::setCastToThis(operation, env); }                  \
   friend class TBaseElement<Details::SubElement, TypeCast##Element>;                            \
   friend class BitFieldMask<TypeCast##Element>;                                                 \
   static const TypeCast##Element& getSameArgument(const EvaluationEnvironment& env)             \
      {  AssumeCondition(dynamic_cast<const TypeCast##Element*>(&env.getFirstArgument()))        \
         return (const TypeCast##Element&) env.getFirstArgument();                               \
      }                                                                                          \
                                                                                                 \
  protected:                                                                                     \
   template <class TypeDomainTraits>                                                             \
   class MethodApplyTable : public Approximate::Details::MethodApplyTable<TypeDomainTraits> {    \
     private:                                                                                    \
      typedef Approximate::Details::MethodApplyTable<TypeDomainTraits> inherited;                \
     public:                                                                                     \
      MethodApplyTable();                                                                        \
   };                                                                                            \
   template <class TypeDomainTraits>                                                             \
   class MethodConstraintTable : public Approximate::Details::MethodConstraintForConstantsTable<TypeDomainTraits> {\
     private:                                                                                    \
      typedef Approximate::Details::MethodConstraintForConstantsTable<TypeDomainTraits> inherited;\
     public:                                                                                     \
      MethodConstraintTable();                                                                   \
   };                                                                                            \
                                                                                                 \
  public:                                                                                        \
   template <class TypeDomainTraits>                                                             \
   class MethodQueryTable : public Approximate::Details::MethodQueryTable<TypeDomainTraits> {    \
     private:                                                                                    \
      typedef Approximate::Details::MethodQueryTable<TypeDomainTraits> inherited;                \
     public:                                                                                     \
      MethodQueryTable();                                                                        \
   };                                                                                            \
                                                                                                 \
  protected:                                                                                     \
   virtual int _getSizeInBits() const override { return getSize()*8; }                           \
   virtual ComparisonResult _compare(const EnhancedObject& asource) const override               \
      {  ComparisonResult result = inherited::_compare(asource);                                 \
         return (result == CREqual)                                                              \
            ? inheritedImplementation::_compare((const inheritedImplementation&) castFromCopyHandler(asource))\
            : result;                                                                            \
      }                                                                                          \
   static void setToNegative(inheritedImplementation::SimulatedType& value) { CodeSetToNegative; }\
   static bool isNegative(inheritedImplementation::SimulatedType value) { return CodeIsNegative; }\
                                                                                                 \
   virtual void _write(OSBase& out, const STG::IOObject::FormatParameters& params) const override\
      {  out << #TypeCast "Constant ";                                                           \
         out.write((int) inheritedImplementation::getElement(), false);                          \
      }                                                                                          \
                                                                                                 \
  public:                                                                                        \
   TypeCast##Element(const Init& init)                                                           \
      : inherited(init), inheritedImplementation(0)                                              \
      {  if (init.hasInitialValue()) {                                                           \
            AssumeCondition(dynamic_cast<const Scalar::Implementation::ImplementationElement*>(&init.getInitialValue()))\
            const auto& implementation = (const Scalar::Implementation::ImplementationElement&)  \
               init.getInitialValue();                                                           \
            if (implementation.isConstantUnsigned()) {                                           \
               AssumeCondition(dynamic_cast<const inheritedImplementation*>(&implementation))    \
               inheritedImplementation::_assign((const inheritedImplementation&) implementation);\
            }                                                                                    \
            else if (implementation.isMultiBit()) {                                              \
               AssumeCondition(dynamic_cast<const Scalar::MultiBit::Implementation::MultiBitElement*>(&implementation))\
               const auto& simplemementation = (const Scalar::MultiBit::Implementation::MultiBitElement&)\
                  implementation;                                                                \
               unsigned long int result = 0UL;                                                   \
               if (simplemementation.getSize() >= 2)                                             \
                  result = ((unsigned long int) simplemementation[1]) << (8*sizeof(unsigned));   \
               result |= simplemementation[0];                                                   \
               inheritedImplementation::setElement(result);                                      \
            }                                                                                    \
            else                                                                                 \
               AssumeUnimplemented                                                               \
         };                                                                                      \
         setConstant();                                                                          \
      }                                                                                          \
   TypeCast##Element(const TypeCast##Element& source) = default;                                 \
   DefineCopy(TypeCast##Element)                                                                 \
   DDefineAssign(TypeCast##Element)                                                              \
   DCompare(TypeCast##Element)                                                                   \
   StaticInheritConversions(TypeCast##Element, inherited)                                        \
                                                                                                 \
   static inheritedImplementation::SimulatedType getMax() { return inheritedImplementation::getMax(); }\
   static inheritedImplementation::SimulatedType getMin() { return inheritedImplementation::getMin(); }\
                                                                                                 \
   virtual bool apply(const VirtualOperation& operation, Scalar::EvaluationEnvironment& env) override\
      {  return inherited::apply(operation, env); }                                              \
                                                                                                 \
   const inheritedImplementation& getImplementation() const { return (const inheritedImplementation&) *this; }\
   inheritedImplementation& getSImplementation() { return (inheritedImplementation&) *this; }    \
   static int getSize() { return inheritedImplementation::getSize(); }                           \
   DefineSignedCast                                                                              \
                                                                                                 \
   template <class TypeRingTraits> static VirtualElement* createConstantElement(TypeRingTraits, const VirtualElement::Init& init)\
      {  return TypeRingTraits::create##TypeCast##Element(init); }                               \
};

#define DefineCastBitToThis 
#define DefineSignedCast 
DefineSubElement(Char, Int, value = -value, value < 0)
#undef DefineSignedCast
#undef DefineCastBitToThis

#define DefineCastBitToThis                                                                      \
template <>                                                                                      \
inline bool setCastToThis<Scalar::Bit::Operation>(Scalar::Bit::Operation& operation, QueryOperation::InverseCastOperationEnvironment& env)\
   {  return false; }                                                                            \
   
#define DefineSignedCast 
DefineSubElement(SignedChar, Int, value = -value, value < 0)
#undef DefineSignedCast

#define DefineSignedCast                                                                         \
   typedef CharElement SignedDomain;                                                             \
   VirtualElement* createSignedElement(SignedDomain::Implementation value) const                 \
      {  return createCharElement(VirtualElement::Init().setInitialValue(                        \
            Integer::Implementation::CharElement(value)));                                       \
      }
   
DefineSubElement(UnsignedChar, UnsignedInt, AssumeUncalled, false)
#undef DefineSignedCast

#define DefineSignedCast 
DefineSubElement(Short, Int, value = -value, value < 0)
#undef DefineSignedCast
   
#define DefineSignedCast                                                                         \
   typedef ShortElement SignedDomain;                                                            \
   VirtualElement* createSignedElement(SignedDomain::Implementation value) const                 \
      {  return createShortElement(VirtualElement::Init().setInitialValue(                       \
            Integer::Implementation::ShortElement(value)));                                      \
      }
   
DefineSubElement(UnsignedShort, UnsignedInt, AssumeUncalled, false)
#undef DefineSignedCast

#define DefineSignedCast 
DefineLongElement(Long, value = -value, value < 0)
#undef DefineSignedCast
   
#define DefineSignedCast                                                                         \
   typedef LongElement SignedDomain;                                                             \
   VirtualElement* createSignedElement(SignedDomain::Implementation value) const                 \
      {  return createLongElement(VirtualElement::Init().setInitialValue(                        \
            Integer::Implementation::LongElement(value)));                                       \
      }
   
DefineLongElement(UnsignedLong, AssumeUncalled, false)
#undef DefineSignedCast

#undef DefineCastBitToThis
#undef DefineSubElement
#undef DefineLongElement

#undef DefineDeclareMethod
#undef DefineDeclareOBinaryMethod

#undef DefineDeclareConstraintBinaryMethodConstantDouble
#undef DefineDeclareConstraintBinaryMethodConstant
#undef DefineDeclareConstraintBinaryMethod

}}}} // end of namespace Analyzer::Scalar::Integer::Approximate

#endif // Analyzer_Scalar_Approximate_ConstantIntegerElementH

