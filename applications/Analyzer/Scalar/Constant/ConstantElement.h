/////////////////////////////////
//
// Library   : Analyzer/Scalar
// Unit      : scalar elements common to compilation constants and symbolic execution
// File      : ConstantElement.h
// Author    : Franck Vedrine
// Copyright : CEA LIST - 2019-2020 - all rights reserved
//
// Description :
//   Definition of the compilation constants.
//   All this stuff is rather defined for source code analysis.
//

#ifndef Analyzer_Scalar_Constant_ConstantElementH
#define Analyzer_Scalar_Constant_ConstantElementH

#include "Analyzer/Scalar/VirtualExact/NativeVirtualExact.hpp"
#include "Analyzer/Scalar/VirtualExact/GeneralVirtualExact.hpp"

namespace Analyzer { namespace Scalar { namespace Constant {

/****************************************************/
/* Definition of the class Constant::VirtualElement */
/****************************************************/

class VirtualElement : public Scalar::VirtualElement {
  public:
   typedef TEvaluationEnvironment<VirtualElement, EvaluationEnvironment> ApplyEnvironment;
   typedef PNT::TPassPointer<VirtualElement, PPAbstractElement> PPVirtualElement;
   enum Type { TUndefined, TScalar, TPointer, TTop };
   class InitFloat : public Init {
     private:
      typedef Init inherited;
      int uSizeMantissa, uSizeExponent;
      
     public:
      InitFloat() {}
      InitFloat(const InitFloat& source) = default;
      InitFloat& operator=(const InitFloat& source) = default;
      DefineCopy(InitFloat)
      DDefineAssign(InitFloat)
      
      InitFloat& setSizeMantissa(int sizeMantissa) { uSizeMantissa = sizeMantissa; return *this; }
      InitFloat& setSizeExponent(int sizeExponent) { uSizeExponent = sizeExponent; return *this; }
      const int& sizeMantissa() const { return uSizeMantissa; }
      const int& sizeExponent() const { return uSizeExponent; }
   };

  private:
   typedef Scalar::VirtualElement inherited;

  protected:
   DefineExtendedParameters(2, inherited);
   
   Type type() const { return (Type) queryOwnField(); }
   void setType(Type tType) { AssumeCondition(!hasOwnField()) mergeOwnField(tType); }

  protected:
   VirtualElement(const Init& init) : inherited(init) {}
   VirtualElement(const VirtualElement& source) = default;
   
   void setScalar() { AssumeCondition(!hasOwnField()) mergeOwnField(TScalar); }
   void setPointer() { AssumeCondition(!hasOwnField()) mergeOwnField(TPointer); }
   void setTop() { AssumeCondition(!hasOwnField()) mergeOwnField(TTop); }
   
   virtual VirtualElement* _createMultiBitElement(const Init&) const { AssumeUncalled return nullptr; }
   virtual VirtualElement* _createMultiFloatElement(const InitFloat&) const { AssumeUncalled return nullptr; }

  public:
   bool isScalar() const { return queryOwnField() == TScalar; }
   bool isPointer() const { return queryOwnField() == TPointer; }
   bool isTop() const { return queryOwnField() == TTop; }
   
  public:
   DefineCopy(VirtualElement)
   virtual bool isValid() const override { return inherited::isValid() && hasOwnField(); }

   int getDependanceLevel() const { AssumeCondition(hasOwnField()) return queryOwnField(); }

   virtual bool apply(const VirtualOperation& operation, EvaluationEnvironment& aenv) override
      {  bool result = false;
         if ((result = AbstractElement::apply(operation, aenv)) == false) {
            ApplyEnvironment& env = (ApplyEnvironment&) aenv;
            int args = operation.getArgumentsNumber();
            if ((args > 0) && (env.getFirstArgument().type() > type())) {
               result = env.getFirstArgument().applyTo(operation, *this, env);
               AssumeCondition(result)
            };
         };
         return result;
      }
   virtual bool applyTo(const VirtualOperation& operation, VirtualElement& source, ApplyEnvironment& env) const
      {  return false; }

   // Methods for the intervace with PPVirtualElement
   static VirtualElement* createBitElement(bool value);
   static VirtualElement* createIntElement(int value);
   static VirtualElement* createIntElement(const Init& init);
   static VirtualElement* createBoolElement(bool value);
   static VirtualElement* createUndefined(const Init& init);

   static VirtualElement* createCharElement(const Init& init);
   static VirtualElement* createSignedCharElement(const Init& init);
   static VirtualElement* createUnsignedCharElement(const Init& init);
   static VirtualElement* createShortElement(const Init& init);
   static VirtualElement* createUnsignedShortElement(const Init& init);
   static VirtualElement* createUnsignedIntElement(const Init& init);
   static VirtualElement* createLongElement(const Init& init);
   VirtualElement* createMultiBitElement(const Init& init) const { return _createMultiBitElement(init); }
   static VirtualElement* createUnsignedLongElement(const Init& init);
   static VirtualElement* createFloatElement(const Init& init);
   static VirtualElement* createDoubleElement(const Init& init);
   static VirtualElement* createLongDoubleElement(const Init& init);
   VirtualElement* createMultiFloatElement(const InitFloat& init) const { return _createMultiFloatElement(init); }

   // Definition of the comparison methods
   bool applyComparison(const VirtualOperation& operation, const VirtualElement& source) const
      {  EvaluationEnvironment env = EvaluationEnvironment(EvaluationEnvironment::Init());
         env.setFirstArgument(source);
         const_cast<VirtualElement&>(*this).apply(operation, env);
         return ((const VirtualElement&) env.getResult()).queryZeroResult().isTrue();
      }
};

class PPVirtualElement : public VirtualElement::PPVirtualElement {
  private:
   typedef VirtualElement::PPVirtualElement inherited;
   PPVirtualElement(const PPAbstractElement& source) : inherited(source) {}

  public:
   PPVirtualElement() {}
   PPVirtualElement(int value) : inherited(VirtualElement::createIntElement(value), Init()) {}
   PPVirtualElement(const VirtualElement& element) : inherited(element) {}
   PPVirtualElement(VirtualElement* element, inherited::Init init) : inherited(element, init) {}
   PPVirtualElement(const PPVirtualElement& source) : inherited(source) {}
   PPVirtualElement(const PPVirtualElement& source, inherited::Duplicate duplicate) : inherited(source, duplicate) {}
   DefineCopy(PPVirtualElement)

   void fullAssign(const PPVirtualElement& source) { inherited::fullAssign(source); }

   void applyAssign(const VirtualOperation& assignOperation, const VirtualElement& source);
};

/* inline methods of the class PPVirtualElement */

inline void
PPVirtualElement::applyAssign(const VirtualOperation& assignOperation, const VirtualElement& source) {
   EvaluationEnvironment env = EvaluationEnvironment(EvaluationEnvironment::Init());
   env.setFirstArgument(source);
   getElement().apply(assignOperation, env);
   if (env.hasResult())
      assign(env.presult());
}

/************************************/
/* Definition of the concrete class */
/************************************/

class BitElement;
class CharElement;
class SignedCharElement;
class UnsignedCharElement;
class ShortElement;
class UnsignedShortElement;
class IntElement;
class UnsignedIntElement;
class LongElement;
class UnsignedLongElement;
class FloatElement;
class DoubleElement;
class LongDoubleElement;

#define DefineScalarClass(TypeDomain, TypeKind, TypeImplementation)                                  \
class TypeKind##Element : public TypeDomain::VirtualExact::T##TypeKind##Element<Constant::VirtualElement> {\
  private:                                                                                         \
   typedef TypeDomain::VirtualExact::T##TypeKind##Element<Constant::VirtualElement> inherited;     \
                                                                                                   \
  protected:                                                                                       \
   virtual void _write(OSBase& out, const STG::IOObject::FormatParameters& params) const override  \
      {  out.write(getElement(), params.isRaw()); }                                                \
  public:                                                                                          \
   TypeKind##Element(const Init& init) : inherited(init) { setScalar(); }                          \
   TypeKind##Element(const TypeKind##Element& source) = default;                                   \
   DefineCopy(TypeKind##Element)                                                                   \
                                                                                                   \
   TypeImplementation getElement() const { return implementation().getElement(); }                 \
};

DefineScalarClass(Bit, Bit, bool)
DefineScalarClass(Integer, Char, char)
DefineScalarClass(Integer, SignedChar, signed char)
DefineScalarClass(Integer, UnsignedChar, unsigned char)
DefineScalarClass(Integer, Short, short int)
DefineScalarClass(Integer, UnsignedShort, unsigned short int)
DefineScalarClass(Integer, Int, int)
DefineScalarClass(Integer, UnsignedInt, unsigned int)
DefineScalarClass(Floating, Float, float)
DefineScalarClass(Floating, Double, double)

#undef DefineScalarClass

class UnsignedLongElement : public Integer::VirtualExact::TUnsignedLongElement<Constant::VirtualElement> {
  private:
   typedef Integer::VirtualExact::TUnsignedLongElement<Constant::VirtualElement> inherited;

  protected:
   virtual void _write(OSBase& out, const STG::IOObject::FormatParameters& params) const override
      {  out.write(getElement(), params.isRaw()); }

  public:
   UnsignedLongElement(const Init& init) : inherited(init) { setScalar(); }
   UnsignedLongElement(const UnsignedLongElement& source) = default;
   DefineCopy(UnsignedLongElement)

   unsigned long getElement() const { return implementation().getElement(); }
};
   
class LongElement : public Integer::VirtualExact::TLongElement<Constant::VirtualElement> {
  private:
   typedef Integer::VirtualExact::TLongElement<Constant::VirtualElement> inherited;

  protected:
   virtual void _write(OSBase& out, const STG::IOObject::FormatParameters& params) const override
      {  out.write(getElement(), params.isRaw()); }

  public:
   LongElement(const Init& init) : inherited(init) { setScalar(); }
   LongElement(const LongElement& source) = default;
   DefineCopy(LongElement)

   unsigned long getElement() const { return implementation().getElement(); }
};
   
class LongDoubleElement : public Floating::VirtualExact::TLongDoubleElement<Constant::VirtualElement> {
  private:
   typedef Floating::VirtualExact::TLongDoubleElement<Constant::VirtualElement> inherited;

  protected:
   virtual void _write(OSBase& out, const STG::IOObject::FormatParameters& params) const override
      {  out.write((double) getElement(), params.isRaw()); }

  public:
   LongDoubleElement(const Init& init) : inherited(init) { setScalar(); }
   LongDoubleElement(const LongDoubleElement& source) = default;
   DefineCopy(LongDoubleElement)

   long double getElement() const { return implementation().getElement(); }
};

typedef VirtualExact::TUndefElement<VirtualElement> UndefElement;

} // end of namespace Constant

#define DefineMethodTableVariable(TypeDomain, TypeKind)                                            \
extern template TypeDomain::VirtualExact::T##TypeKind##Element<Constant::VirtualElement>::MethodApplyTable \
TypeDomain::VirtualExact::T##TypeKind##Element<Constant::VirtualElement>::matMethodTable;

DefineMethodTableVariable(Integer, Char)
DefineMethodTableVariable(Integer, SignedChar)
DefineMethodTableVariable(Integer, UnsignedChar)
DefineMethodTableVariable(Integer, Short)
DefineMethodTableVariable(Integer, UnsignedShort)
DefineMethodTableVariable(Integer, Int)
DefineMethodTableVariable(Integer, UnsignedInt)
DefineMethodTableVariable(Integer, UnsignedLong)
DefineMethodTableVariable(Integer, Long)
DefineMethodTableVariable(Floating, Float)
DefineMethodTableVariable(Floating, Double)
DefineMethodTableVariable(Floating, LongDouble)

#undef DefineMethodTableVariable

}} // end of namespace Analyzer::Scalar

#endif // Analyzer_Scalar_Constant_ConstantElementH

