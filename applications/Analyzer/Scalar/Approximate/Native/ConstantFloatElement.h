/////////////////////////////////
//
// Library   : Analyzer/Scalar
// Unit      : approximate native scalar elements
// File      : ConstantFloatElement.h
// Author    : Franck Vedrine
// Copyright : CEA LIST - 2019-2020 - all rights reserved
//
// Description :
//   Definition of a class of floating-point elements that rely on native host implementation.
//   All this stuff is rather defined for source code analysis.
//

#ifndef Analyzer_Scalar_Approximate_ConstantFloatElementH
#define Analyzer_Scalar_Approximate_ConstantFloatElementH

#include "Analyzer/Scalar/Approximate/Native/ConstantIntegerElement.h"
#include "Analyzer/Scalar/Implementation/Native/FloatElement.h"
#include "Analyzer/Scalar/Constant/ConstantElement.h"

namespace Numerics {

class LongDoubleTraits;
class DoubleTraits;
class FloatTraits;
   
} // end of Numerics

namespace Analyzer { namespace Scalar { namespace Floating { namespace Approximate {

namespace Details {

template <class TypeBase, class TypeDerived> using TBaseElement = Scalar::Approximate::Details::TBaseElement<TypeBase, TypeDerived>;
using BaseConstants = Scalar::Approximate::Details::BaseConstants;
using RealOperationElement = Scalar::Approximate::Details::RealOperationElement;
template <class TypeDomain> using TDomainTraits = Scalar::Approximate::Details::TDomainTraits<TypeDomain>;
template <class TypeDomainTraits> using MethodApplyTable = Scalar::Approximate::Details::MethodApplyTable<TypeDomainTraits>;
template <class TypeDomainTraits> using MethodConstraintForConstantsTable = Scalar::Approximate::Details::MethodConstraintForConstantsTable<TypeDomainTraits>;
template <class TypeDomainTraits> using MethodQueryTable = Scalar::Approximate::Details::MethodQueryTable<TypeDomainTraits>;

}

class BaseElement : public TVirtualElement<Details::RealOperationElement> {
  private:
   typedef TVirtualElement<Details::RealOperationElement> inherited;

  public:
   typedef InitFloat Init;
   BaseElement(const InitFloat& init) : inherited(init) {}
   BaseElement(const BaseElement& source) = default;
   typedef Floating::Operation Operation;
   typedef Approximate::Interval Interval;
   
   class Constants : public Details::BaseConstants {
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
   
/*    static const int NBQueryOperationMethods = 1;
      static const int NBQueryValueMethods = 1;
      static const int NBQueryExternMethods = 1; */
   };
   class ConstantsForDouble : public Constants {
     public:
      static const int NBApplyMethods = Operation::EndOfType;
      static const int NBUnaryConstraintMethods = Operation::EndOfCast
            + Operation::EndOfType - Operation::EndOfSecondBinary;
   };

   bool querySimplification(const QueryOperation&, QueryOperation::Environment& env) const;
};

template <class TypeBase, class TypeDerived>
class TBaseElement : public Details::TBaseElement<TypeBase, TypeDerived> {
  private:
   typedef Details::TBaseElement<TypeBase, TypeDerived> inherited;
   typedef TBaseElement<TypeBase, TypeDerived> thisType;
   
  public:
   TBaseElement(const typename TypeBase::Init& init) : inherited(init) {}
   TBaseElement(const thisType& source) = default;
  
#define DefineDeclareMethod(TypeOperation)                                                        \
bool apply##TypeOperation(const VirtualOperation& operation, EvaluationEnvironment& env);

   DefineDeclareMethod(PlusAssign)
   DefineDeclareMethod(MinusAssign)
   DefineDeclareMethod(TimesAssign)
   DefineDeclareMethod(DivideAssign)
   DefineDeclareMethod(OppositeAssign)
   DefineDeclareMethod(AbsAssign)

   DefineDeclareMethod(Acos)
   DefineDeclareMethod(Asin)
   DefineDeclareMethod(Atan)
   DefineDeclareMethod(Atan2)
   DefineDeclareMethod(Ceil)
   DefineDeclareMethod(Cos)
   DefineDeclareMethod(Cosh)
   DefineDeclareMethod(Exp)
   DefineDeclareMethod(Fabs)
   DefineDeclareMethod(Floor)
   DefineDeclareMethod(Fmod)
   DefineDeclareMethod(Frexp)
   DefineDeclareMethod(Ldexp)
   DefineDeclareMethod(Log)
   DefineDeclareMethod(Log10)
   DefineDeclareMethod(Modf)
   DefineDeclareMethod(Pow)
   DefineDeclareMethod(Sin)
   DefineDeclareMethod(Sinh)
   DefineDeclareMethod(Sqrt)
   DefineDeclareMethod(Tan)
   DefineDeclareMethod(Tanh)

#undef DefineDeclareMethod

   bool queryCompareSpecial(const typename TypeBase::QueryOperation&, typename TypeBase::QueryOperation::Environment&) const;
   bool queryGuardAll(const typename TypeBase::QueryOperation&, typename TypeBase::QueryOperation::Environment&) const;
   static bool queryInverseCastOperation(const VirtualElement&, const VirtualQueryOperation&, VirtualQueryOperation::Environment& env);
   bool querySizes(const typename TypeBase::QueryOperation&, typename TypeBase::QueryOperation::Environment& env) const;

#define DefineDeclareConstraintBinaryMethod(TypeOperation)                                        \
bool constraint##TypeOperation(const VirtualOperation&, VirtualElement& source, const VirtualElement& result,\
      ConstraintEnvironment::Argument arg, ConstraintEnvironment& env);

   DefineDeclareConstraintBinaryMethod(ArithmeticAssignWithConstantResult)

   // DefineDeclareConstraintBinaryMethod(Atan2)
   // DefineDeclareConstraintBinaryMethod(Fmod)
   // DefineDeclareConstraintBinaryMethod(Ldexp)
   // DefineDeclareConstraintBinaryMethod(Pow)

#undef DefineDeclareConstraintBinaryMethod
};


/****************************************/
/* Definition of the class FloatElement */
/****************************************/

#define DefineFloatElement(TypeCast)                                                             \
namespace Details { namespace D##TypeCast##Element {                                             \
                                                                                                 \
typedef Details::RealOperationElement::QueryOperation QueryOperation;                            \
                                                                                                 \
template <class TypeOperation>                                                                   \
inline bool setCastToThis(TypeOperation& operation, QueryOperation::InverseCastOperationEnvironment&)\
   {  operation.setCast##TypeCast(); return true; }                                              \
                                                                                                 \
template <>                                                                                      \
inline bool setCastToThis<Scalar::MultiBit::Operation>(Scalar::MultiBit::Operation& operation, QueryOperation::InverseCastOperationEnvironment& env)\
   {  return false; }                                                                            \
                                                                                                 \
template <>                                                                                      \
inline bool setCastToThis<Scalar::Bit::Operation>(Scalar::Bit::Operation& operation, QueryOperation::InverseCastOperationEnvironment& env)\
   {  return false; }                                                                            \
                                                                                                 \
}}                                                                                               \
                                                                                                 \
class TypeCast##Element : public TBaseElement<BaseElement, TypeCast##Element>, protected Implementation::TypeCast##Element {\
  public:                                                                                        \
   typedef Approximate::VirtualElement VirtualElement;                                           \
   typedef Numerics::TypeCast##Traits ImplementationTraits;                                      \
   typedef Implementation::TypeCast##Element inheritedImplementation;                            \
                                                                                                 \
  private:                                                                                       \
   typedef TBaseElement<BaseElement, TypeCast##Element> inherited;                               \
                                                                                                 \
  public:                                                                                        \
   static Constant::VirtualElement* createEpsilon(inheritedImplementation::SimulatedType epsilon)\
      {  return new Constant::TypeCast##Element(Constant::VirtualElement::InitFloat()            \
            .setInitialValue(inheritedImplementation(epsilon)));                                 \
      }                                                                                          \
  public:                                                                                        \
   typedef inheritedImplementation::SimulatedType Implementation;                                \
   typedef Approximate::Details::TDomainTraits<TypeCast##Element> DomainTraits;                  \
                                                                                                 \
  protected:                                                                                     \
   template <class TypeOperation>                                                                \
   static bool setCastToThis(TypeOperation& operation, QueryOperation::InverseCastOperationEnvironment& env)\
      {  return Details::D##TypeCast##Element::setCastToThis(operation, env); }                  \
   friend class TBaseElement<BaseElement, TypeCast##Element>;                                    \
                                                                                                 \
   template <class TypeDomainTraits>                                                             \
   class MethodApplyTable : public Details::MethodApplyTable<TypeDomainTraits> {                 \
     private:                                                                                    \
      typedef Details::MethodApplyTable<TypeDomainTraits> inherited;                             \
     public:                                                                                     \
      MethodApplyTable();                                                                        \
   };                                                                                            \
   template <class TypeDomainTraits>                                                             \
   class MethodConstraintTable : public Details::MethodConstraintForConstantsTable<TypeDomainTraits> {\
     private:                                                                                    \
      typedef Details::MethodConstraintForConstantsTable<TypeDomainTraits> inherited;            \
     public:                                                                                     \
      MethodConstraintTable();                                                                   \
   };                                                                                            \
                                                                                                 \
  public:                                                                                        \
   template <class TypeDomainTraits>                                                             \
   class MethodQueryTable : public Details::MethodQueryTable<TypeDomainTraits> {                 \
     private:                                                                                    \
      typedef Details::MethodQueryTable<TypeDomainTraits> inherited;                             \
     public:                                                                                     \
      MethodQueryTable();                                                                        \
   };                                                                                            \
                                                                                                 \
   virtual void _write(OSBase& out, const STG::IOObject::FormatParameters& params) const override\
      {  out << #TypeCast "Constant ";                                                           \
         out.write((double) inheritedImplementation::getElement(), false);                       \
      }                                                                                          \
                                                                                                 \
  protected:                                                                                     \
   virtual int _getSizeInBits() const override { return sizeof(SimulatedType)*8; }               \
   virtual ComparisonResult _compare(const EnhancedObject& asource) const override               \
      {  ComparisonResult result = VirtualElement::_compare(asource);                            \
         return (result == CREqual)                                                              \
            ? inheritedImplementation::_compare((const inheritedImplementation&) castFromCopyHandler(asource))\
            : result;                                                                            \
      }                                                                                          \
                                                                                                 \
  public:                                                                                        \
   TypeCast##Element(const InitFloat& init) : inherited(init), inheritedImplementation(0.0)      \
      {  if (init.hasInitialValue()) {                                                           \
            AssumeCondition(dynamic_cast<const Scalar::Implementation::ImplementationElement*>(&init.getInitialValue()))\
            const auto& implementation = (const Scalar::Implementation::ImplementationElement&)  \
               init.getInitialValue();                                                           \
            if (implementation.isConstantDouble()) {                                             \
               AssumeCondition(dynamic_cast<const inheritedImplementation*>(&implementation))    \
               inheritedImplementation::_assign((const inheritedImplementation&) implementation);\
            }                                                                                    \
            else if (implementation.isMultiFloat()) {                                            \
               AssumeCondition(dynamic_cast<const Scalar::Floating::Implementation::MultiFloatElement*>(&implementation))\
               AssumeUnimplemented                                                               \
            }                                                                                    \
            else                                                                                 \
               AssumeUnimplemented                                                               \
         };                                                                                      \
         setConstant();                                                                          \
      }                                                                                          \
   TypeCast##Element(const TypeCast##Element& source) = default;                                 \
   TypeCast##Element& operator=(const TypeCast##Element& source) = default;                      \
   DefineCopy(TypeCast##Element)                                                                 \
   DDefineAssign(TypeCast##Element)                                                              \
   DCompare(TypeCast##Element)                                                                   \
   StaticInheritConversions(TypeCast##Element, inherited)                                        \
                                                                                                 \
   virtual int countAtomic() const override { return 1; }                                        \
   virtual bool apply(const VirtualOperation& operation, Scalar::EvaluationEnvironment& env) override\
      {  return inherited::apply(operation, env); }                                              \
                                                                                                 \
   const inheritedImplementation& getImplementation() const { return (const inheritedImplementation&) *this; }\
   inheritedImplementation& getSImplementation() { return (inheritedImplementation&) *this; }    \
   static int getSize() { return sizeof(inheritedImplementation::SimulatedType); }               \
                                                                                                 \
   template <class TypeRingTraits> static VirtualElement* createConstantElement(TypeRingTraits, const VirtualElement::InitFloat& iInit)\
      {  return TypeRingTraits::create##TypeCast##Element(iInit); }                              \
};

DefineFloatElement(Float)
DefineFloatElement(Double)
DefineFloatElement(LongDouble)

#undef DefineFloatElement

}}}} // end of namespace Analyzer::Scalar::Floating::Approximate

#endif // Analyzer_Scalar_Approximate_ConstantFloatElementH
