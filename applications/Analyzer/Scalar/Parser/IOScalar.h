/////////////////////////////////
//
// Library   : Analyzer/Scalar
// Unit      : approximate scalar elements
// File      : Parser/IOScalar.h
// Author    : Franck Vedrine
// Copyright : CEA LIST - 2019-2020 - all rights reserved
//
// Description :
//   Definition of persistent scalar elements.
//

#pragma once

#include "Analyzer/Scalar/Parser/ScalarParser.h"
#include "Analyzer/Scalar/ConcreteOperation.h"
#include "Collection/Collection.hpp"

namespace Analyzer { namespace Scalar { namespace Approximate {

/************************************/
/* Definition of the class IOScalar */
/************************************/

class IOScalar : public STG::IOObject, public BaseIOScalar, public STG::Lexer::Base {
  private:
   typedef PNT::PassPointer<EnhancedObject> PPEnhancedObject;

  public:
   class OperatorStack;
   class Operator : public COL::List::Node, public ExtendedParameters {
     private:
      /* operator precedence:
         ->[1] conditional expression ? :
         ->[...] assignment expression =, +=, ...
         ->[2] 'vv',
         ->[3] '^^',
         ->[4] '||',
         ->[5] '&&',
         ->[6] '|'
         ->[7] '^'
         ->[8] '&'
         ->[9] '==' | '!='
         ->[10] '<' | '>' | "<=" | '>='
         ->[11] '<<' | '>>'
         ->[12] '+' | '-'
         ->[13] '*' | '/' | '%'
         ->[14] casts
         ->[15] unary '+' | '-' | '~' | '!'
         ->[16] postfix call '()'
         ->[17] constant | identifier | '('element')' | '[min, max]'
      */

      enum Precedence
         {  PUndefined, PConditional, PMerge, PIntersect, PLogicalOr, PLogicalAnd,
            PBitOr, PBitExclusiveOr, PBitAnd, PEquality, PComparison, PShift,
            PAddition, PMultiplication, PCast, PUnary, PPostFix
         };
      enum Type
         {  TUndefined, TPlus, TMinus, TTimes, TDivide, TModulo, TLeftShift,
            TRightShift, TEqual, TDifferent, TLessOrEqual, TGreaterOrEqual, TLess,
            TGreater, TLogicalAnd, TLogicalOr, TBitAnd, TBitOr, TBitXOr,
            TUnaryPlus, TUnaryMinus, TNot, TComplement, TCast, TCall, TConditional,
            TMerge, TIntersect, TInterval
         };
      enum TypeCast
         {  TCUndefined, TCBit, TCMultiBit, TCMultiFloat, TCMultiFloatPtr
         };
      enum TypeCallFunction
         {  TCFUndefined, TCFExtendZero, TCFExtendSign, TCFConcat, TCFReduce,
            TCFBitSet, TCFShiftBit, TCFInc, TCFDec, TCFMinSigned, TCFMinUnsigned,
            TCFMaxSigned, TCFMaxUnsigned
         };
      enum AdditionalTypeInformation
         {  ATIUndefined, ATIUnsigned, ATISigned, ATIUnsignedWithSigned, ATIFloat };

     private:
      PPEnhancedObject ppveFirst;
      PPEnhancedObject ppveSecond;
      PPEnhancedObject ppveThird;
      int uSize;
      int uLowBit, uHighBit;
      int uSizeExponent;
      int uSizeMantissa;
      // unsigned uStartLine;
      // unsigned uStartColumn;
      friend class OperatorStack;

     protected:
      DefineExtendedParameters(26, ExtendedParameters)
      DefineSubExtendedParameters(Precedence, 4, INHERITED)
      DefineSubExtendedParameters(Type, 5, Precedence)
      DefineSubExtendedParameters(AdditionalType, 3, Type)
      DefineSubExtendedParameters(Symbolic, 1, AdditionalType)
      DefineSubExtendedParameters(TypeCast, 3, Symbolic)
      DefineSubExtendedParameters(TypeCallFunction, 4, Symbolic)
      DefineSubExtendedParameters(LeftSubExpressions, 2, TypeCallFunction)
      DefineSubExtendedParameters(RightAssociative, 1, LeftSubExpressions)
      DefineSubExtendedParameters(FirstArgumentType, 2, RightAssociative)
      DefineSubExtendedParameters(SecondArgumentType, 2, FirstArgumentType)
      DefineSubExtendedParameters(ThirdArgumentType, 2, SecondArgumentType)

      friend class IOScalar;

     public:
      Operator() : uSize(0), uLowBit(0), uHighBit(0), uSizeExponent(0), uSizeMantissa(0) {}
      Operator(const Operator& source) = default;
      Operator& operator=(const Operator& source) = default;
      DefineCopy(Operator)
      DDefineAssign(Operator)

      bool hasFirstArgument() const { return ppveFirst.isValid(); }
      bool hasSecondArgument() const { return ppveSecond.isValid(); }
      bool hasThirdArgument() const { return ppveThird.isValid(); }
      PPEnhancedObject extractLastArgument(ArgumentType& argumentType);
      PPEnhancedObject extractThirdArgument(ArgumentType& argumentType);
      PPEnhancedObject extractSecondArgument(ArgumentType& argumentType);
      PPEnhancedObject extractFirstArgument(ArgumentType& argumentType);
      EnhancedObject& getFirstArgument(ArgumentType& argumentType) const;
      EnhancedObject& getSecondArgument(ArgumentType& argumentType) const;
      EnhancedObject& getThirdArgument(ArgumentType& argumentType) const;
      EnhancedObject& getLastArgument(ArgumentType& argumentType) const;
      Precedence getPrecedence() const { return (Precedence) queryPrecedenceField(); }

      Operator& setType(Type type, bool isPrefix=false);
      Operator& setUnsignedTag() { AssumeCondition(!hasAdditionalTypeField()) mergeAdditionalTypeField(ATIUnsigned); return *this; }
      Operator& setSignedTag() { AssumeCondition(!hasAdditionalTypeField()) mergeAdditionalTypeField(ATISigned); return *this; }
      Operator& setUnsignedWithSignedTag() { AssumeCondition(!hasAdditionalTypeField()) mergeAdditionalTypeField(ATIUnsignedWithSigned); return *this; }
      Operator& setFloatTag() { AssumeCondition(!hasAdditionalTypeField()) mergeAdditionalTypeField(ATIFloat); return *this; }
      Operator& setSymbolic() { mergeSymbolicField(1); return *this; }
      bool hasUnsignedTag() const { return queryAdditionalTypeField() == ATIUnsigned; }
      bool hasSignedTag() const { return queryAdditionalTypeField() == ATISigned; }
      bool hasUnsignedWithSignedTag() const { return queryAdditionalTypeField() == ATIUnsignedWithSigned; }
      bool hasFloatTag() const { return queryAdditionalTypeField() == ATIFloat; }
      bool hasTag() const { return hasAdditionalTypeField(); }
      bool isSymbolic() const { return hasSymbolicField(); }
      Type getType() const { return (Type) queryTypeField(); }
      bool isBinaryOperator() const
         {  Type type = (Type) queryTypeField();
            return type >= TPlus && type <= TBitXOr;
         }
      int getSize() const { AssumeCondition(uSize > 0) return uSize; }
      bool hasSize() const { return uSize > 0; }
      int getSizeMantissa() const { AssumeCondition(uSizeMantissa > 0) return uSizeMantissa; }
      bool hasSizeMantissa() const { return uSizeMantissa > 0; }
      void setSizeMantissa(int sizeMantissa) { uSizeMantissa = sizeMantissa; }
      int getSizeExponent() const { AssumeCondition(uSizeExponent > 0) return uSizeExponent; }
      bool hasSizeExponent() const { return uSizeExponent > 0; }
      void setSizeExponent(int sizeExponent) { uSizeExponent = sizeExponent; }
      void setBitSize(int size) { AssumeCondition(uSize == 0) uSize = size; }
      void setFloatSize(int sizeExponent, int sizeMantissa)
         {  AssumeCondition(uSizeExponent == 0 && uSizeMantissa == 0)
            uSizeExponent = sizeExponent; uSizeMantissa = sizeMantissa;
         }
      STG::SubString queryOperatorName() const;
      Operator& setTypeCast(TypeCast typeCast)
         {  AssumeCondition(!hasTypeCastField());
            mergeTypeCastField(typeCast);
            return *this;
         }
      Operator& changeTypeCast(TypeCast typeCast)
         {  setTypeCastField(typeCast);
            return *this;
         }
      bool isCast() const { return queryTypeField() == TCast; }
      TypeCast getTypeCast() const { return (TypeCast) queryTypeCastField(); }
      Operator& setTypeCall(TypeCallFunction typeCall)
         {  AssumeCondition(!hasTypeCallFunctionField());
            mergeTypeCallFunctionField(typeCall);
            return *this;
         }
      TypeCallFunction getTypeCall() const { return (TypeCallFunction) queryTypeCallFunctionField(); }
      void setLowBit(int lowBit) { uLowBit = lowBit; }
      void setHighBit(int highBit) { uHighBit = highBit; }
      void setSize(int size) { uSize = size; }
      void setExtension(int extension) { uSize = extension; }
      int getExtension() const { return uSize; }
      
      Operator& setLeftSubExpressions(int leftSubExpressions)
         {  AssumeCondition(leftSubExpressions < 4 && !hasLeftSubExpressionsField());
            mergeLeftSubExpressionsField(leftSubExpressions);
            return *this;
         }
      void incLeftSubExpressions()
         {  int leftSubExpressions = queryLeftSubExpressionsField();
            AssumeCondition(leftSubExpressions < 3);
            setLeftSubExpressionsField(leftSubExpressions+1);
         }
      void decLeftSubExpressions()
         {  int leftSubExpressions = queryLeftSubExpressionsField();
            AssumeCondition(leftSubExpressions > 0);
            setLeftSubExpressionsField(leftSubExpressions-1);
         }
      bool isFinished() const { return !hasLeftSubExpressionsField(); }
      int getLeftSubExpressions() const { return queryLeftSubExpressionsField(); }
      void advance()
         {  unsigned leftSubExpressions = queryLeftSubExpressionsField();
            AssumeCondition(leftSubExpressions > 0)
            setLeftSubExpressionsField(leftSubExpressions-1);
         }

      bool isLeftAssociative() { return !hasRightAssociativeField(); }
      bool isRightAssociative() { return hasRightAssociativeField(); }
      bool isValid() const override { return hasTypeField(); }
      Operator& setFirstArgument(PPEnhancedObject, ArgumentType argumentType);
      Operator& setSecondArgument(PPEnhancedObject, ArgumentType argumentType);
      Operator& setThirdArgument(PPEnhancedObject, ArgumentType argumentType);
      Operator& setLastArgument(PPEnhancedObject, ArgumentType argumentType);
      int queryArgumentsNumber() const;
   };

   class OperatorStack : public COL::TCopyCollection<COL::TList<Operator> > {
     private:
      static PPEnhancedObject apply(Operator& operation, ArgumentType& argumentType,
         Parser::Arguments& context);
      static PPEnhancedObject applyInterval(Operator& operation, ArgumentType& argumentType,
         Parser::Arguments& context);
      static PPEnhancedObject applyCallFunction(Operator& operation, ArgumentType& argumentType,
         Parser::Arguments& context);
      friend class IOScalar;

      PNT::AutoPointer<IOScalar> apceSubElement;

     public:
      OperatorStack() {}
      OperatorStack(const OperatorStack& source) = default;
      OperatorStack(OperatorStack&& source)
         {  COL::TList<Operator>::swap(source);
            apceSubElement.swap(source.apceSubElement);
         }

      void pushLastArgument(PPEnhancedObject, ArgumentType argumentType);
      EnhancedObject& getLastArgument(ArgumentType& argumentType) const;

      Operator& pushOperator(Operator::Type typeOperator, int leftSubExpressions,
            Parser::Arguments& context, bool& hasFailed);
      Operator& pushPrefixUnaryOperator(Operator::Type typeOperator)
         {  Operator* result;
            insertNewAtEnd(result = new Operator());
            result->setType(typeOperator, true).setLeftSubExpressions(1);
            return *result;
         }
      Operator& pushPrefixBinaryOperator(Operator::Type typeOperator)
         {  Operator* result;
            insertNewAtEnd(result = new Operator());
            result->setType(typeOperator, true).setLeftSubExpressions(2);
            return *result;
         }
      Operator& pushBinaryOperator(Operator::Type typeOperator)
         {  Operator* result;
            insertNewAtEnd(result = new Operator());
            result->setType(typeOperator, true).setLeftSubExpressions(2);
            return *result;
         }
      Operator& pushBinaryOperator(Operator::Type typeOperator,
            Parser::Arguments& context, bool& hasFailed)
         {  return pushOperator(typeOperator, 1, context, hasFailed); }
      Operator& pushConditionalOperator(Parser::Arguments& context, bool& hasFailed)
         {  return pushOperator(Operator::TConditional, 2, context, hasFailed); }
      Operator& pushCallFunction(Operator::TypeCallFunction typeCall, int leftSubExpressions)
         {  Operator* result;
            insertNewAtEnd(result = new Operator());
            result->setType(Operator::TCall)
               .setTypeCall(typeCall)
               .setLeftSubExpressions(leftSubExpressions);
            return *result;
         }
      Operator& pushPostfixUnaryOperator(Operator::Type typeOperator,
            Parser::Arguments& context, bool& hasFailed)
         {  return pushOperator(typeOperator, 0, context, hasFailed); }
      Operator& pushCastOperator(Operator::TypeCast typeCast)
         {  Operator* result;
            insertNewAtEnd(result = new Operator());
            result->setType(Operator::TCast).setTypeCast(typeCast).setLeftSubExpressions(1);
            return *result;
         }
      PPEnhancedObject clear(ArgumentType& argumentType, Parser::Arguments& context);
      void absorbSubElement(IOScalar* subElement) { apceSubElement.absorbElement(subElement); }
      IOScalar* extractSubElement() { return apceSubElement.extractElement(); }
      IOScalar& getSubElement() const { return *apceSubElement; }
   };

  private:
   PPEnhancedObject ppveElement;
   ArgumentType atType;

  protected:
   virtual void _write(OSBase& out, const FormatParameters& params) const override
      {  AssumeUnimplemented }

  public:
   IOScalar() : atType(ATUndefined) {}
   IOScalar(PPEnhancedObject element, ArgumentType type)
      :  ppveElement(element), atType(type) {}
   IOScalar(const IOScalar& source) = default;
   IOScalar(const IOScalar& source, PNT::Pointer::Duplicate duplicate)
      :  STG::IOObject(source), ppveElement(source.ppveElement, duplicate),
         atType(source.atType) {}
   IOScalar& operator=(const IOScalar& source) = default;
   DefineCopy(IOScalar)
   DDefineAssign(IOScalar)

   void clear() { ppveElement.release(); atType = ATUndefined; }
   bool isValid() const override { return ppveElement.isValid(); }
   ReadResult readToken(Parser::State& state, Parser::Arguments& arguments);
   EnhancedObject& getElement() const { return *ppveElement; }
   PPEnhancedObject& selement() { return ppveElement; }
   ArgumentType getArgumentType() const { return atType; }
};

inline IOScalar::PPEnhancedObject
IOScalar::Operator::extractLastArgument(ArgumentType& argumentType) {
   incLeftSubExpressions();
   PPEnhancedObject result;
   if (ppveThird.isValid()) {
      argumentType = (ArgumentType) queryThirdArgumentTypeField();
      clearThirdArgumentTypeField();
      result = ppveThird;
   }
   else if (ppveSecond.isValid()) {
      argumentType = (ArgumentType) querySecondArgumentTypeField();
      clearSecondArgumentTypeField();
      result = ppveSecond;
   }
   else {
      argumentType = (ArgumentType) queryFirstArgumentTypeField();
      clearFirstArgumentTypeField();
      result = ppveFirst;
   };
   return result;
}

inline IOScalar::PPEnhancedObject
IOScalar::Operator::extractThirdArgument(ArgumentType& argumentType) {
   incLeftSubExpressions();
   argumentType = (ArgumentType) queryThirdArgumentTypeField();
   clearThirdArgumentTypeField();
   return ppveThird;
}

inline IOScalar::PPEnhancedObject
IOScalar::Operator::extractSecondArgument(ArgumentType& argumentType) {
   incLeftSubExpressions();
   argumentType = (ArgumentType) querySecondArgumentTypeField();
   clearSecondArgumentTypeField();
   return ppveSecond;
}

inline IOScalar::PPEnhancedObject
IOScalar::Operator::extractFirstArgument(ArgumentType& argumentType) {
   incLeftSubExpressions();
   argumentType = (ArgumentType) queryFirstArgumentTypeField();
   clearFirstArgumentTypeField();
   return ppveFirst;
}

inline IOScalar::EnhancedObject&
IOScalar::Operator::getFirstArgument(ArgumentType& argumentType) const {
   argumentType = (ArgumentType) queryFirstArgumentTypeField();
   return *ppveFirst;
}

inline IOScalar::EnhancedObject&
IOScalar::Operator::getSecondArgument(ArgumentType& argumentType) const {
   argumentType = (ArgumentType) querySecondArgumentTypeField();
   return *ppveSecond;
}

inline IOScalar::EnhancedObject&
IOScalar::Operator::getThirdArgument(ArgumentType& argumentType) const {
   argumentType = (ArgumentType) queryThirdArgumentTypeField();
   return *ppveThird;
}

inline IOScalar::EnhancedObject&
IOScalar::Operator::getLastArgument(ArgumentType& argumentType) const {
   EnhancedObject* result = nullptr;
   if (ppveThird.isValid()) {
      argumentType = (ArgumentType) queryThirdArgumentTypeField();
      result = &*ppveThird;
   }
   else if (ppveSecond.isValid()) {
      argumentType = (ArgumentType) querySecondArgumentTypeField();
      result = &*ppveSecond;
   }
   else {
      argumentType = (ArgumentType) queryFirstArgumentTypeField();
      result = &*ppveFirst;
   };
   return *result;
}

inline int
IOScalar::Operator::queryArgumentsNumber() const {
   int result = 0;
   Type type = (Type) queryTypeField();
   if (type >= TPlus && type <= TBitXOr)
      result = 2;
   else if (type >= TUnaryPlus && type <= TCast)
      result = 1;
   else if (type == TCall) {
      switch (queryTypeCallFunctionField()) {
         case TCFExtendZero:
         case TCFExtendSign:
         case TCFReduce:
         case TCFShiftBit:
         case TCFInc:
         case TCFDec:
            result = 1;
            break;
         case TCFConcat:
         case TCFBitSet:
         case TCFMinSigned:
         case TCFMinUnsigned:
         case TCFMaxSigned:
         case TCFMaxUnsigned:
            result = 2;
            break;
         default:
            result = 1;
      };
   }
   else if (type == TConditional)
      result = 3;
   else if (type >= TMerge && type <= TIntersect)
      result = 2;
   return result;
}

inline IOScalar::Operator&
IOScalar::Operator::setFirstArgument(PPEnhancedObject expression, ArgumentType argumentType) {
   AssumeCondition(!hasFirstArgumentTypeField())
   ppveFirst = expression;
   mergeFirstArgumentTypeField(argumentType);
   return *this;
}

inline IOScalar::Operator&
IOScalar::Operator::setSecondArgument(PPEnhancedObject expression, ArgumentType argumentType) {
   AssumeCondition(!hasSecondArgumentTypeField())
   ppveSecond = expression;
   mergeSecondArgumentTypeField(argumentType);
   return *this;
}

inline IOScalar::Operator&
IOScalar::Operator::setThirdArgument(PPEnhancedObject expression, ArgumentType argumentType) {
   AssumeCondition(!hasThirdArgumentTypeField())
   ppveThird = expression;
   mergeThirdArgumentTypeField(argumentType);
   return *this;
}

inline IOScalar::Operator&
IOScalar::Operator::setLastArgument(PPEnhancedObject element, ArgumentType argumentType) {
   if (!ppveFirst.isValid()) {
      AssumeCondition(!hasFirstArgumentTypeField())
      ppveFirst = element;
      mergeFirstArgumentTypeField(argumentType);
   }
   else if (!ppveSecond.isValid()) {
      AssumeCondition(!hasSecondArgumentTypeField())
      ppveSecond = element;
      mergeSecondArgumentTypeField(argumentType);
   }
   else if (!ppveThird.isValid()) {
      AssumeCondition(!hasThirdArgumentTypeField())
      ppveThird = element;
      mergeThirdArgumentTypeField(argumentType);
   }
   if (isValid())
      decLeftSubExpressions();
   return *this;
}

inline void
IOScalar::OperatorStack::pushLastArgument(
      PPEnhancedObject element, ArgumentType argumentType) {
   Operator* lastOperator;
   if (isEmpty())
      insertNewAtEnd(lastOperator = new Operator);
   else
      lastOperator = &getSLast();
   lastOperator->setLastArgument(element, argumentType);
}

inline EnhancedObject&
IOScalar::OperatorStack::getLastArgument(ArgumentType& argumentType) const {
   return getLast().getLastArgument(argumentType);
}

/****************************************/
/* Definition of the class NamedIOScalar */
/****************************************/

class NamedIOScalar : public EnhancedObject {
  public:
   typedef IOScalar::ArgumentType ArgumentType;

  private:
   typedef PNT::PassPointer<EnhancedObject> PPEnhancedObject;

   STG::SubString ssName;
   IOScalar ceValue;

  public:
   NamedIOScalar() : ssName(STG::SString()) {}
   NamedIOScalar(STG::SubString&& name) : ssName(name) {}
   NamedIOScalar(const STG::SubString& name) : ssName(name) {}
   NamedIOScalar(const STG::SubString& name, PPEnhancedObject element, ArgumentType argumentType)
      :  ssName(name), ceValue(element, argumentType) {}
   NamedIOScalar(const NamedIOScalar& source) = default;
   NamedIOScalar(NamedIOScalar&& source) = default;
   NamedIOScalar& operator=(const NamedIOScalar& source) = default;
   NamedIOScalar& operator=(NamedIOScalar&& source) = default;
   DefineCopy(NamedIOScalar)
   DDefineAssign(NamedIOScalar)

   virtual bool isValid() const override { return ceValue.isValid(); }
   const STG::SubString& getName() const { return ssName; }
   void setName(const STG::SubString& name) { ssName = name; }
   bool hasName() const { return ssName.length() > 0; }
   IOScalar& getSValue() { return ceValue; }
   const IOScalar& getValue() const { return ceValue; }
   void setValue(const IOScalar& value)
      {  ceValue = IOScalar(value, PNT::Pointer::Duplicate()); }
   void clear() { ssName = STG::SString(); ceValue.clear(); }

   class Key {
     public:
      typedef const STG::SubString& KeyType;
      typedef STG::SubString TypeOfKey;
      // typedef STG::SubString ControlKeyType;
      static ComparisonResult compare(KeyType fstKey, KeyType sndKey) { return fstKey.compare(sndKey); }
      static const STG::SubString& key(const NamedIOScalar& namedElement)
         {  return namedElement.getName(); }
   };
};

class MapOfIOScalars : public COL::TCopyCollection<COL::SortedArray<NamedIOScalar, NamedIOScalar::Key> >,
                       public STG::IOObject, public STG::Lexer::Base {
  private:
   typedef COL::TCopyCollection<COL::SortedArray<NamedIOScalar, NamedIOScalar::Key> > inherited;

  public:
   MapOfIOScalars() {}
   MapOfIOScalars(const MapOfIOScalars& source) = default;
   MapOfIOScalars& operator=(const MapOfIOScalars& source) = default;
   StaticInheritConversions(MapOfIOScalars, inherited)
   DefineCopy(MapOfIOScalars)
   DDefineAssign(MapOfIOScalars)

   ReadResult readToken(Parser::State& state, Parser::Arguments& arguments);
};

class ConstraintIOScalar : public EnhancedObject {
  private:
   Bit::Operation::Type tBitApplyType;
   MultiBit::Operation::Type tIntApplyType;
   Floating::Operation::Type tFloatApplyType;
   PNT::AutoPointer<MultiBit::Operation> apoIntApplyType;
   PNT::AutoPointer<Bit::Operation> apoBitApplyType;
   PNT::AutoPointer<Floating::Operation> apoFloatApplyType;
   bool fIsSymbolic;
   NamedIOScalar neFirst;
   NamedIOScalar neSecond;
   IOScalar ceResult;
   friend class MapOfIOScalars;

  public:
   ConstraintIOScalar()
      :  tBitApplyType(Bit::Operation::TUndefined),
         tIntApplyType(MultiBit::Operation::TUndefined),
         tFloatApplyType(Floating::Operation::TUndefined),
         fIsSymbolic(false) {}
   ConstraintIOScalar(const ConstraintIOScalar& source) = default;
   ConstraintIOScalar& operator=(const ConstraintIOScalar& source) = default;
   DefineCopy(ConstraintIOScalar)
   DDefineAssign(ConstraintIOScalar)

   bool apply(Parser::Arguments& arguments);
};

}}} // end of namespace Analyzer::Scalar::Approximate

