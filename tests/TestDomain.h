/////////////////////////////////
//
// Library   : unit test of Analyzer/Scalar
// Unit      : approximate scalar elements
// File      : TestDomain.h
// Author    : Franck Vedrine
// Copyright : CEA LIST - 2019-2020 - all rights reserved
//
// Description :
//   Definition of the components for the test of general multibit scalar elements.
//

#pragma once

#include "Analyzer/Scalar/AbstractDomain.h"
#include "Collection/Collection.hpp"
#include "TestDomain/Parser.h"

/*******************************************/
/* Definition of the class ConcreteElement */
/*******************************************/

class GuardLatticeCreation {
  private:
   DomainEvaluationEnvironment oldEnv;
   DomainEvaluationEnvironment& newEnv;

  public:
   GuardLatticeCreation(DomainEvaluationEnvironment& env) : oldEnv(env), newEnv(env) {}
   ~GuardLatticeCreation() { newEnv = oldEnv; }
};

class Domain : public STG::IOObject {
  private:
   DomainElement deValue;

  public:
   Domain() : deValue{} {}
   Domain(Domain&& source) : STG::IOObject(source), deValue{}
      {  std::swap(deValue.content, source.deValue.content); }
   Domain(DomainElement&& value) : deValue(value) { value.content = nullptr; } 
   Domain(const Domain& source) : STG::IOObject(source), deValue(domain_clone(source.deValue)) {}
   Domain& operator=(const Domain& source)
      {  if (deValue.content != source.deValue.content) {
            domain_free(&deValue);
            deValue = domain_clone(source.deValue);
         }
         return *this;
      }
   Domain& operator=(Domain&& source)
      {  if (deValue.content != source.deValue.content) {
            domain_free(&deValue);
            std::swap(deValue.content, source.deValue.content);
         }
         return *this;
      }
   ~Domain() { domain_free(&deValue); }

   const DomainElement& value() const { return deValue; }
   DomainElement& svalue() { return deValue; }
   void clear() { domain_free(&deValue); }
   bool isValid() const override { return STG::IOObject::isValid() && deValue.content; }
};

class BaseConcreteElement {
  public:
   enum ArgumentType
      {  ATUndefined, ATBit, ATMultiBit, ATMultiFloat };
};

class ConcreteElement : public STG::IOObject, public STG::Lexer::Base, public BaseConcreteElement {
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
      Domain dFirst;
      Domain dSecond;
      Domain dThird;
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

      friend class ConcreteElement;

     public:
      Operator() : uSize(0), uLowBit(0), uHighBit(0), uSizeExponent(0), uSizeMantissa(0) {}
      Operator(const Operator& source) = default;
      Operator& operator=(const Operator& source) = default;
      DefineCopy(Operator)
      DDefineAssign(Operator)

      bool hasFirstArgument() const { return dFirst.isValid(); }
      bool hasSecondArgument() const { return dSecond.isValid(); }
      bool hasThirdArgument() const { return dThird.isValid(); }
      Domain extractLastArgument(ArgumentType& argumentType);
      Domain extractThirdArgument(ArgumentType& argumentType);
      Domain extractSecondArgument(ArgumentType& argumentType);
      Domain extractFirstArgument(ArgumentType& argumentType);
      const Domain& getFirstArgument(ArgumentType& argumentType) const;
      const Domain& getSecondArgument(ArgumentType& argumentType) const;
      const Domain& getThirdArgument(ArgumentType& argumentType) const;
      const Domain& getLastArgument(ArgumentType& argumentType) const;
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
      Operator& absorbFirstArgument(Domain&& domain, ArgumentType argumentType);
      Operator& absorbSecondArgument(Domain&& domain, ArgumentType argumentType);
      Operator& absorbThirdArgument(Domain&& domain, ArgumentType argumentType);
      Operator& absorbLastArgument(Domain&& domain, ArgumentType argumentType);
      int queryArgumentsNumber() const;
   };

   class OperatorStack : public COL::TCopyCollection<COL::TList<Operator> > {
     private:
      static Domain apply(Operator& operation, ArgumentType& argumentType,
         Parser::Arguments& context);
      static Domain applyInterval(Operator& operation, ArgumentType& argumentType,
         Parser::Arguments& context);
      static Domain applyCallFunction(Operator& operation, ArgumentType& argumentType,
         Parser::Arguments& context);
      friend class ConcreteElement;

      PNT::AutoPointer<ConcreteElement> apceSubElement;

     public:
      OperatorStack() {}
      OperatorStack(const OperatorStack& source) = default;
      OperatorStack(OperatorStack&& source)
         {  COL::TList<Operator>::swap(source);
            apceSubElement.swap(source.apceSubElement);
         }

      void pushLastArgument(Domain&& domain, ArgumentType argumentType);
      const Domain& getLastArgument(ArgumentType& argumentType) const;

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
      Domain clear(ArgumentType& argumentType, Parser::Arguments& context);
      void absorbSubElement(ConcreteElement* subElement) { apceSubElement.absorbElement(subElement); }
      ConcreteElement* extractSubElement() { return apceSubElement.extractElement(); }
      ConcreteElement& getSubElement() const { return *apceSubElement; }
   };

  private:
   Domain dElement;
   ArgumentType atType;

  protected:
   virtual void _write(OSBase& out, const FormatParameters& params) const override
      {  AssumeUnimplemented }

  public:
   ConcreteElement() : atType(ATUndefined) {}
   ConcreteElement(Domain&& element, ArgumentType type)
      :  dElement(element), atType(type) {}
   ConcreteElement(const ConcreteElement& source) = default;
   ConcreteElement(ConcreteElement&& source) = default;
   ConcreteElement& operator=(const ConcreteElement& source) = default;
   ConcreteElement& operator=(ConcreteElement&& source) = default;
   DefineCopy(ConcreteElement)
   DDefineAssign(ConcreteElement)

   void clear() { dElement.clear(); atType = ATUndefined; }
   bool isValid() const override { return dElement.isValid(); }
   ReadResult readToken(Parser::State& state, Parser::Arguments& arguments);
   const Domain& getElement() const { return dElement; }
   Domain& selement() { return dElement; }
   ArgumentType getArgumentType() const { return atType; }
};

inline Domain
ConcreteElement::Operator::extractLastArgument(ArgumentType& argumentType) {
   incLeftSubExpressions();
   Domain result;
   if (dThird.isValid()) {
      argumentType = (ArgumentType) queryThirdArgumentTypeField();
      clearThirdArgumentTypeField();
      result = dThird;
   }
   else if (dSecond.isValid()) {
      argumentType = (ArgumentType) querySecondArgumentTypeField();
      clearSecondArgumentTypeField();
      result = dSecond;
   }
   else {
      argumentType = (ArgumentType) queryFirstArgumentTypeField();
      clearFirstArgumentTypeField();
      result = dFirst;
   };
   return result;
}

inline Domain
ConcreteElement::Operator::extractThirdArgument(ArgumentType& argumentType) {
   incLeftSubExpressions();
   argumentType = (ArgumentType) queryThirdArgumentTypeField();
   clearThirdArgumentTypeField();
   return std::move(dThird);
}

inline Domain
ConcreteElement::Operator::extractSecondArgument(ArgumentType& argumentType) {
   incLeftSubExpressions();
   argumentType = (ArgumentType) querySecondArgumentTypeField();
   clearSecondArgumentTypeField();
   return std::move(dSecond);
}

inline Domain
ConcreteElement::Operator::extractFirstArgument(ArgumentType& argumentType) {
   incLeftSubExpressions();
   argumentType = (ArgumentType) queryFirstArgumentTypeField();
   clearFirstArgumentTypeField();
   return std::move(dFirst);
}

inline const Domain&
ConcreteElement::Operator::getFirstArgument(ArgumentType& argumentType) const {
   argumentType = (ArgumentType) queryFirstArgumentTypeField();
   return dFirst;
}

inline const Domain&
ConcreteElement::Operator::getSecondArgument(ArgumentType& argumentType) const {
   argumentType = (ArgumentType) querySecondArgumentTypeField();
   return dSecond;
}

inline const Domain&
ConcreteElement::Operator::getThirdArgument(ArgumentType& argumentType) const {
   argumentType = (ArgumentType) queryThirdArgumentTypeField();
   return dThird;
}

inline const Domain&
ConcreteElement::Operator::getLastArgument(ArgumentType& argumentType) const {
   const Domain* result = nullptr;
   if (dThird.isValid()) {
      argumentType = (ArgumentType) queryThirdArgumentTypeField();
      result = &dThird;
   }
   else if (dSecond.isValid()) {
      argumentType = (ArgumentType) querySecondArgumentTypeField();
      result = &dSecond;
   }
   else {
      argumentType = (ArgumentType) queryFirstArgumentTypeField();
      result = &dFirst;
   };
   return *result;
}

inline int
ConcreteElement::Operator::queryArgumentsNumber() const {
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

inline ConcreteElement::Operator&
ConcreteElement::Operator::absorbFirstArgument(Domain&& expression, ArgumentType argumentType) {
   AssumeCondition(!hasFirstArgumentTypeField())
   dFirst = std::move(expression);
   mergeFirstArgumentTypeField(argumentType);
   return *this;
}

inline ConcreteElement::Operator&
ConcreteElement::Operator::absorbSecondArgument(Domain&& expression, ArgumentType argumentType) {
   AssumeCondition(!hasSecondArgumentTypeField())
   dSecond = std::move(expression);
   mergeSecondArgumentTypeField(argumentType);
   return *this;
}

inline ConcreteElement::Operator&
ConcreteElement::Operator::absorbThirdArgument(Domain&& expression, ArgumentType argumentType) {
   AssumeCondition(!hasThirdArgumentTypeField())
   dThird = std::move(expression);
   mergeThirdArgumentTypeField(argumentType);
   return *this;
}

inline ConcreteElement::Operator&
ConcreteElement::Operator::absorbLastArgument(Domain&& element, ArgumentType argumentType) {
   if (!dFirst.isValid()) {
      AssumeCondition(!hasFirstArgumentTypeField())
      dFirst = std::move(element);
      mergeFirstArgumentTypeField(argumentType);
   }
   else if (!dSecond.isValid()) {
      AssumeCondition(!hasSecondArgumentTypeField())
      dSecond = std::move(element);
      mergeSecondArgumentTypeField(argumentType);
   }
   else if (!dThird.isValid()) {
      AssumeCondition(!hasThirdArgumentTypeField())
      dThird = std::move(element);
      mergeThirdArgumentTypeField(argumentType);
   }
   if (isValid())
      decLeftSubExpressions();
   return *this;
}

inline void
ConcreteElement::OperatorStack::pushLastArgument(Domain&& element, ArgumentType argumentType) {
   Operator* lastOperator;
   if (isEmpty())
      insertNewAtEnd(lastOperator = new Operator);
   else
      lastOperator = &getSLast();
   lastOperator->absorbLastArgument(std::move(element), argumentType);
}

inline const Domain&
ConcreteElement::OperatorStack::getLastArgument(ArgumentType& argumentType) const {
   return getLast().getLastArgument(argumentType);
}

/****************************************/
/* Definition of the class NamedElement */
/****************************************/

class NamedElement : public EnhancedObject {
  public:
   typedef ConcreteElement::ArgumentType ArgumentType;

  private:
   STG::SubString ssName;
   ConcreteElement ceValue;

  // protected:
  //  virtual void _write(OSBase& out, const FormatParameters& params) const;
  //  virtual void _read(ISBase& in, const FormatParameters& params);

  public:
   NamedElement() : ssName(STG::SString()) {}
   NamedElement(const STG::SubString& name) : ssName(name) {}
   NamedElement(const STG::SubString& name, Domain&& element, ArgumentType argumentType)
      :  ssName(name), ceValue(std::move(element), argumentType) {}
   NamedElement(const NamedElement& source) = default;
   DefineCopy(NamedElement)
   DDefineAssign(NamedElement)

   virtual bool isValid() const override { return ceValue.isValid(); }
   const STG::SubString& getName() const { return ssName; }
   void setName(const STG::SubString& name) { ssName = name; }
   bool hasName() const { return ssName.length() > 0; }
   ConcreteElement& getSValue() { return ceValue; }
   const ConcreteElement& getValue() const { return ceValue; }
   void setValue(const ConcreteElement& value) { ceValue = value; }
   void clear() { ssName = STG::SString(); ceValue.clear(); }

   class Key {
     public:
      typedef const STG::SubString& KeyType;
      typedef STG::SubString TypeOfKey;
      // typedef STG::SubString ControlKeyType;
      static ComparisonResult compare(KeyType fstKey, KeyType sndKey) { return fstKey.compare(sndKey); }
      static const STG::SubString& key(const NamedElement& namedElement)
         {  return namedElement.getName(); }
   };
};

class MapOfElements : public COL::TCopyCollection<COL::SortedArray<NamedElement, NamedElement::Key> >,
                      public STG::IOObject, public STG::Lexer::Base, public BaseConcreteElement {
  private:
   typedef COL::TCopyCollection<COL::SortedArray<NamedElement, NamedElement::Key> > inherited;

  public:
   class FormatParameters;

  protected:
   virtual void _write(OSBase& out, const STG::IOObject::FormatParameters& params) const override
      {  AssumeUnimplemented }
   virtual void _read(ISBase& in, const STG::IOObject::FormatParameters& params) override;

  public:
   MapOfElements() {}
   MapOfElements(const MapOfElements& source) = default;
   MapOfElements& operator=(const MapOfElements& source) = default;
   StaticInheritConversions(MapOfElements, inherited)
   DefineCopy(MapOfElements)
   DDefineAssign(MapOfElements)

   ReadResult readToken(Parser::State& state, Parser::Arguments& arguments);
};

class ConstraintElement : public EnhancedObject {
  private:
   class VirtualConstraintFunction : public EnhancedObject, public BaseConcreteElement {
     public:
      VirtualConstraintFunction() = default;
      VirtualConstraintFunction(const VirtualConstraintFunction&) = default;
      DefineCopy(VirtualConstraintFunction)

      virtual int getArgumentsNumber() const { return 0; }
      virtual ArgumentType getType() const { return ATUndefined; }
      bool isBit() const { return getType() == ATBit; }
      bool isMultiBit() const { return getType() == ATMultiBit; }
      bool isMultiFloat() const { return getType() == ATMultiFloat; }
      virtual bool isBinaryMinus() const { return false; }
      virtual VirtualConstraintFunction* createUnaryOpposite() const { return nullptr; }
      virtual bool isBinaryPlus() const { return false; }
      virtual bool isMultiBitReduction() const { return false; }
      virtual bool isMultiBitSet() const { return false; }
      virtual bool isMultiBitCastShiftBit() const { return false; }
   };
   class UnaryConstraintFunction : public VirtualConstraintFunction {
     public:
      UnaryConstraintFunction() = default;
      UnaryConstraintFunction(const UnaryConstraintFunction&) = default;
      DefineCopy(UnaryConstraintFunction)

      virtual int getArgumentsNumber() const override { return 1; }
      virtual bool operator()(const Domain& result, Domain& argument, DomainEvaluationEnvironment& env) const
         {  return false; }
   };
   class CastMultiBitConstraintFunction : public UnaryConstraintFunction {
     private:
      ArgumentType tType = ATUndefined;
      int uSizeInBits = 0;

     public:
      CastMultiBitConstraintFunction() = default;
      CastMultiBitConstraintFunction(const CastMultiBitConstraintFunction&) = default;
      DefineCopy(CastMultiBitConstraintFunction)

      virtual ArgumentType getType() const override { return tType; }
      CastMultiBitConstraintFunction& setType(ArgumentType type) { tType = type; return *this; }
      CastMultiBitConstraintFunction& setBit() { tType = ATBit; return *this; }
      CastMultiBitConstraintFunction& setMultiFloat() { tType = ATMultiFloat; return *this; }
      CastMultiBitConstraintFunction& setSizeInBits(int sizeInBits) { uSizeInBits = sizeInBits; return *this; }
      virtual bool operator()(const Domain& result, Domain& argument, DomainEvaluationEnvironment& env) const override
         {  AssumeCondition(tType != ATUndefined && uSizeInBits > 0)
            if (tType == ATBit)
               return domain_bit_cast_multibit_constraint(result.value(), uSizeInBits,
                     &argument.svalue(), &env);
            else // tType == ATMultiFloat
               return domain_multifloat_cast_multibit_constraint(result.value(), uSizeInBits,
                     &argument.svalue(), &env);
         }
   };
   class CastMultiFloatConstraintFunction : public UnaryConstraintFunction {
     private:
      ArgumentType tType = ATUndefined;
      int uSizeInBits = 0;
      bool fIsSigned = false;
      bool fIsPtr = false;

     public:
      CastMultiFloatConstraintFunction() = default;
      CastMultiFloatConstraintFunction(const CastMultiFloatConstraintFunction&) = default;
      DefineCopy(CastMultiFloatConstraintFunction)

      virtual ArgumentType getType() const override { return tType; }
      CastMultiFloatConstraintFunction& setMultiFloat() { tType = ATMultiFloat; return *this; }
      CastMultiFloatConstraintFunction& setMultiBit() { tType = ATMultiBit; return *this; }
      CastMultiFloatConstraintFunction& setSizeInBits(int sizeInBits) { uSizeInBits = sizeInBits; return *this; }
      CastMultiFloatConstraintFunction& setSigned(bool isSigned=true) { fIsSigned = isSigned; return *this; }
      CastMultiFloatConstraintFunction& setPtr(bool isPtr=true) { fIsPtr = isPtr; return *this; }
      virtual bool operator()(const Domain& result, Domain& argument, DomainEvaluationEnvironment& env) const override
         {  AssumeCondition(tType != ATUndefined && uSizeInBits > 0)
            if (tType == ATMultiBit)
               return domain_multibit_cast_multifloat_constraint(result.value(), uSizeInBits, fIsSigned,
                     &argument.svalue(), &env);
            else // tType == ATMultiFloat
               return domain_multifloat_cast_multifloat_constraint(result.value(), uSizeInBits,
                     &argument.svalue(), &env);
         }
   };
   class UnaryOperationConstraintFunction : public UnaryConstraintFunction {
     private:
      ArgumentType tType = ATUndefined;
      int uOperationOpcode = 0;
      bool fCastBit = false;
      bool fMultiFloatQuery = false;

     public:
      UnaryOperationConstraintFunction() = default;
      UnaryOperationConstraintFunction(const UnaryOperationConstraintFunction&) = default;
      DefineCopy(UnaryOperationConstraintFunction)

      virtual ArgumentType getType() const override { return tType; }
      UnaryOperationConstraintFunction& setBit(DomainBitUnaryOperation operation)
         {  tType = ATBit; uOperationOpcode = operation; return *this; }
      UnaryOperationConstraintFunction& setMultiBit(DomainMultiBitUnaryOperation operation)
         {  tType = ATMultiBit; uOperationOpcode = operation; return *this; }
      UnaryOperationConstraintFunction& setCastBit()
         {  tType = ATMultiBit; fCastBit = true; return *this; }
      UnaryOperationConstraintFunction& setMultiFloat(DomainMultiFloatUnaryOperation operation)
         {  tType = ATMultiFloat; uOperationOpcode = operation; return *this; }
      UnaryOperationConstraintFunction& setMultiFloat(DomainMultiFloatToIntOperation operation)
         {  tType = ATMultiFloat; fMultiFloatQuery = true; uOperationOpcode = operation; return *this; }
      virtual bool operator()(const Domain& result, Domain& argument, DomainEvaluationEnvironment& env) const override
         {  AssumeCondition(tType != ATUndefined && (fCastBit == (uOperationOpcode == 0)))
            if (tType == ATBit)
               return domain_bit_unary_constraint(result.value(), (DomainBitUnaryOperation) uOperationOpcode,
                     &argument.svalue(), &env);
            else if (tType == ATMultiBit) {
               if (!fCastBit)
                  return domain_multibit_unary_constraint(result.value(), (DomainMultiBitUnaryOperation) uOperationOpcode,
                        &argument.svalue(), &env);
               else
                  return domain_multibit_cast_bit_constraint(result.value(), &argument.svalue(), &env);
            }
            else if (tType == ATMultiFloat) {
               if (!fMultiFloatQuery)
                  return domain_multifloat_unary_constraint(result.value(), (DomainMultiFloatUnaryOperation) uOperationOpcode,
                        &argument.svalue(), &env);
               else
                  return domain_multifloat_query_to_multibit_constraint(result.value(), (DomainMultiFloatToIntOperation) uOperationOpcode,
                        &argument.svalue(), &env);
            }
            return false;
         }
   };
   class ExtendConstraintFunction : public UnaryConstraintFunction {
     private:
      DomainMultiBitExtendOperation uOperation = { DMBEOUndefined, false };

     public:
      ExtendConstraintFunction() = default;
      ExtendConstraintFunction(const ExtendConstraintFunction&) = default;
      DefineCopy(ExtendConstraintFunction)

      virtual ArgumentType getType() const override { return ATMultiBit; }
      ExtendConstraintFunction& setType(DomainMultiBitExtendType type) { uOperation.type = type; return *this; }
      ExtendConstraintFunction& setNewSize(int newSize) { uOperation.new_size = newSize; return *this; }
      virtual bool operator()(const Domain& result, Domain& argument, DomainEvaluationEnvironment& env) const override
         {  AssumeCondition(uOperation.new_size > 0)
            return domain_multibit_extend_constraint(result.value(), uOperation,
                  &argument.svalue(), &env);
         }
   };
   class CastShiftBitConstraintFunction : public UnaryConstraintFunction {
     private:
      int uShift = 0;

     public:
      CastShiftBitConstraintFunction() = default;
      CastShiftBitConstraintFunction(const CastShiftBitConstraintFunction&) = default;
      DefineCopy(CastShiftBitConstraintFunction)

      virtual ArgumentType getType() const override { return ATMultiBit; }
      CastShiftBitConstraintFunction& setShift(int shift) { uShift = shift; return *this; }
      virtual bool operator()(const Domain& result, Domain& argument, DomainEvaluationEnvironment& env) const override
         {  AssumeCondition(uShift > 0)
            return domain_multibit_cast_shift_bit_constraint(result.value(), uShift,
                  &argument.svalue(), &env);
         }
      virtual bool isMultiBitCastShiftBit() const override { return true; }
   };
   class ReduceConstraintFunction : public UnaryConstraintFunction {
     private:
      DomainMultiBitReduceOperation uOperation = { DMBEOUndefined, false };

     public:
      ReduceConstraintFunction() = default;
      ReduceConstraintFunction(const ReduceConstraintFunction&) = default;
      DefineCopy(ReduceConstraintFunction)

      virtual ArgumentType getType() const override { return ATMultiBit; }
      ReduceConstraintFunction& setFirstBit(int firstBit) { uOperation.first_bit = firstBit; return *this; }
      ReduceConstraintFunction& setLastBit(int lastBit) { uOperation.last_bit = lastBit; return *this; }
      virtual bool operator()(const Domain& result, Domain& argument, DomainEvaluationEnvironment& env) const override
         {  AssumeCondition(uOperation.first_bit >= 0 && uOperation.first_bit <= uOperation.last_bit)
            return domain_multibit_reduce_constraint(result.value(), uOperation,
                  &argument.svalue(), &env);
         }
      virtual bool isMultiBitReduction() const override { return true; }
   };
   class BinaryConstraintFunction : public VirtualConstraintFunction {
     public:
      BinaryConstraintFunction() = default;
      BinaryConstraintFunction(const BinaryConstraintFunction&) = default;
      DefineCopy(BinaryConstraintFunction)

      virtual bool isCompare() const { return false; }
      virtual bool isOperation() const { return false; }
      virtual int getArgumentsNumber() const override { return 2; }
      virtual bool operator()(const Domain& result, Domain& first, Domain& second, DomainEvaluationEnvironment& env) const
         {  return false; }
   };
   class BinaryOperationConstraintFunction : public BinaryConstraintFunction {
     private:
      ArgumentType tType = ATUndefined;
      int uOperationOpcode = 0;

     public:
      BinaryOperationConstraintFunction() = default;
      BinaryOperationConstraintFunction(const BinaryOperationConstraintFunction&) = default;
      DefineCopy(BinaryOperationConstraintFunction)

      virtual ArgumentType getType() const override { return tType; }
      virtual bool isOperation() const override { return true; }
      BinaryOperationConstraintFunction& setBit(DomainBitBinaryOperation operation)
         {  tType = ATBit; uOperationOpcode = operation; return *this; }
      BinaryOperationConstraintFunction& setMultiBit(DomainMultiBitBinaryOperation operation)
         {  tType = ATMultiBit; uOperationOpcode = operation; return *this; }
      BinaryOperationConstraintFunction& setMultiFloat(DomainMultiFloatBinaryOperation operation)
         {  tType = ATMultiFloat; uOperationOpcode = operation; return *this; }
      DomainBitBinaryOperation getBitOperation() const
         { AssumeCondition(tType == ATBit) return (DomainBitBinaryOperation) uOperationOpcode; }
      DomainMultiBitBinaryOperation getMultiBitOperation() const
         { AssumeCondition(tType == ATMultiBit) return (DomainMultiBitBinaryOperation) uOperationOpcode; }
      DomainMultiFloatBinaryOperation getMultiFloatOperation() const
         { AssumeCondition(tType == ATMultiFloat) return (DomainMultiFloatBinaryOperation) uOperationOpcode; }
      virtual bool operator()(const Domain& result, Domain& first, Domain& second, DomainEvaluationEnvironment& env) const override
         {  AssumeCondition(tType != ATUndefined && uOperationOpcode > 0)
            if (tType == ATBit)
               return domain_bit_binary_constraint(result.value(), (DomainBitBinaryOperation) uOperationOpcode,
                     &first.svalue(), &second.svalue(), &env);
            else if (tType == ATMultiBit)
               return domain_multibit_binary_constraint(result.value(), (DomainMultiBitBinaryOperation) uOperationOpcode,
                     &first.svalue(), &second.svalue(), &env);
            else if (tType == ATMultiFloat)
               return domain_multifloat_binary_constraint(result.value(), (DomainMultiFloatBinaryOperation) uOperationOpcode,
                     &first.svalue(), &second.svalue(), &env);
            return false;
         }
      virtual bool isBinaryMinus() const override
         {  if (tType == ATMultiBit)
               return uOperationOpcode == DMBBOMinusSigned || uOperationOpcode == DMBBOMinusUnsigned;
            else if (tType == ATMultiFloat)
               return uOperationOpcode == DMFBOMinus;
            return false;
         }
      virtual VirtualConstraintFunction* createUnaryOpposite() const override
         {  UnaryOperationConstraintFunction* result = nullptr;
            if (tType == ATMultiBit) {
               if (uOperationOpcode == DMBBOMinusSigned || uOperationOpcode == DMBBOMinusUnsigned) {
                  result = new UnaryOperationConstraintFunction();
                  result->setMultiBit(DMBUOOppositeSigned);
               }
            }
            else if (tType == ATMultiFloat) {
               if (uOperationOpcode == DMFBOMinus) {
                  result = new UnaryOperationConstraintFunction();
                  result->setMultiFloat(DMFUOOpposite);
               }
            }
            return result;
         }
      virtual bool isBinaryPlus() const override
         {  if (tType == ATMultiBit)
               return uOperationOpcode == DMBBOPlusSigned || uOperationOpcode == DMBBOPlusUnsigned;
            else if (tType == ATMultiFloat)
               return uOperationOpcode == DMFBOPlus;
            return false;
         }
   };
   class BinaryCompareConstraintFunction : public BinaryConstraintFunction {
     private:
      ArgumentType tType = ATUndefined;
      int uCompareOpcode = 0;

     public:
      BinaryCompareConstraintFunction() = default;
      BinaryCompareConstraintFunction(const BinaryCompareConstraintFunction&) = default;
      DefineCopy(BinaryCompareConstraintFunction)

      virtual ArgumentType getType() const override { return tType; }
      virtual bool isCompare() const override { return true; }
      BinaryCompareConstraintFunction& setBit(DomainBitCompareOperation operation)
         {  tType = ATBit; uCompareOpcode = operation; return *this; }
      BinaryCompareConstraintFunction& setMultiBit(DomainMultiBitCompareOperation operation)
         {  tType = ATMultiBit; uCompareOpcode = operation; return *this; }
      BinaryCompareConstraintFunction& setMultiFloat(DomainMultiFloatCompareOperation operation)
         {  tType = ATMultiFloat; uCompareOpcode = operation; return *this; }
      DomainBitCompareOperation getBitOperation() const
         { AssumeCondition(tType == ATBit) return (DomainBitCompareOperation) uCompareOpcode; }
      DomainMultiBitCompareOperation getMultiBitOperation() const
         { AssumeCondition(tType == ATMultiBit) return (DomainMultiBitCompareOperation) uCompareOpcode; }
      DomainMultiFloatCompareOperation getMultiFloatOperation() const
         { AssumeCondition(tType == ATMultiFloat) return (DomainMultiFloatCompareOperation) uCompareOpcode; }
      virtual bool operator()(const Domain& result, Domain& first, Domain& second, DomainEvaluationEnvironment& env) const override
         {  AssumeCondition(tType != ATUndefined && uCompareOpcode > 0)
            if (tType == ATBit)
               return domain_bit_compare_constraint(result.value(), (DomainBitCompareOperation) uCompareOpcode,
                     &first.svalue(), &second.svalue(), &env);
            else if (tType == ATMultiBit)
               return domain_multibit_compare_constraint(result.value(), (DomainMultiBitCompareOperation) uCompareOpcode,
                     &first.svalue(), &second.svalue(), &env);
            else if (tType == ATMultiFloat)
               return domain_multifloat_compare_constraint(result.value(), (DomainMultiFloatCompareOperation) uCompareOpcode,
                     &first.svalue(), &second.svalue(), &env);
            return false;
         }
   };
   class BitSetConstraintFunction : public BinaryConstraintFunction {
     private:
      DomainMultiBitSetOperation uOperation = { DMBEOUndefined, false };

     public:
      BitSetConstraintFunction() = default;
      BitSetConstraintFunction(const BitSetConstraintFunction&) = default;
      DefineCopy(BitSetConstraintFunction)

      virtual ArgumentType getType() const override { return ATMultiBit; }
      BitSetConstraintFunction& setFirstBit(int firstBit) { uOperation.first_bit = firstBit; return *this; }
      BitSetConstraintFunction& setLastBit(int lastBit) { uOperation.last_bit = lastBit; return *this; }
      virtual bool operator()(const Domain& result, Domain& first, Domain& second, DomainEvaluationEnvironment& env) const override
         {  AssumeCondition(uOperation.first_bit >= 0 && uOperation.first_bit <= uOperation.last_bit)
            return domain_multibit_bitset_constraint(result.value(), uOperation,
                  &first.svalue(), &second.svalue(), &env);
         }
      virtual bool isMultiBitSet() const override { return true; }
   };
   class MultiFloatTernaryConstraintFunction : public VirtualConstraintFunction {
     private:
      DomainMultiFloatTernaryOperation uOperation = DMFTOUndefined;

     public:
      MultiFloatTernaryConstraintFunction() = default;
      MultiFloatTernaryConstraintFunction(const MultiFloatTernaryConstraintFunction&) = default;
      DefineCopy(MultiFloatTernaryConstraintFunction)

      virtual ArgumentType getType() const override { return ATMultiFloat; }
      MultiFloatTernaryConstraintFunction& setOperation(DomainMultiFloatTernaryOperation operation) { uOperation = operation; return *this; }
      virtual int getArgumentsNumber() const override { return 3; }
      virtual bool operator()(const Domain& result, Domain& first, Domain& second, Domain& third, DomainEvaluationEnvironment& env) const
         {  AssumeCondition(uOperation != DMFTOUndefined)
            return domain_multifloat_ternary_constraint(result.value(), uOperation,
                  &first.svalue(), &second.svalue(), &third.svalue(), &env);
         }
   };

   PNT::AutoPointer<VirtualConstraintFunction> apvcfOperation[3];
   bool fIsSymbolic;
   NamedElement neFirst;
   NamedElement neSecond;
   ConcreteElement ceResult;
   friend class MapOfElements;

  public:
   ConstraintElement() : fIsSymbolic(false) {}
   ConstraintElement(const ConstraintElement& source) = default;
   ConstraintElement& operator=(const ConstraintElement& source) = default;
   DefineCopy(ConstraintElement)
   DDefineAssign(ConstraintElement)

   bool apply(Parser::Arguments& arguments);
};

class MapOfElements::FormatParameters : public STG::IOObject::FormatParameters {
  private:
   VirtualReadEventNotification* prenNotification;
   bool fDebug;

  public:
   FormatParameters() : prenNotification(nullptr), fDebug(false) {}
   FormatParameters(const FormatParameters& source) = default;

   FormatParameters& setNotification(VirtualReadEventNotification& notification)
      {  prenNotification = &notification; return *this; }
   FormatParameters& setDebug() { fDebug = true; return *this; }
   bool hasNotification() const { return prenNotification; }
   bool isDebug() const { return fDebug; }
   VirtualReadEventNotification& getNotification() const
      {  AssumeCondition(prenNotification) return *prenNotification; } 
};

