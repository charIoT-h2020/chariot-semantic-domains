/////////////////////////////////
//
// Library   : Analyzer/Scalar
// Unit      : approximate scalar elements
// File      : Parser/ScalarToken.h
// Author    : Franck Vedrine
// Copyright : CEA LIST - 2019-2020 - all rights reserved
//
// Description :
//   Definition of tokens to lex the input file.
//

#pragma once

#include "TString/String.hpp"

namespace Analyzer { namespace Scalar { namespace Approximate {

class AbstractToken : public EnhancedObject, public ExtendedParameters {
  public:
   enum Type
      {  TUndefined, TIdentifier, TNumber, TKeyword, TOperatorPunctuator, TEnd };
  protected:
   DefineExtendedParameters(4, ExtendedParameters)
   DefineSubExtendedParameters(Type, 3, INHERITED)
   DefineSubExtendedParameters(Full, 1, Type)

   void setType(Type type) { AssumeCondition(!hasTypeField()); mergeTypeField(type); }
   void setFull() { mergeFullField(1); }
  
  public:
   class FullType {};
   AbstractToken() {}
   AbstractToken(unsigned type, FullType) { params() = type; mergeFullField(1); }

  public:
   AbstractToken(Type type) { mergeTypeField(type); }
   AbstractToken(const AbstractToken& source) = default;
   DefineCopy(AbstractToken)

   virtual bool isValid() const override
      {  return hasTypeField() && (queryTypeField() < TEnd); }
   Type getType() const { return (Type) queryTypeField(); }
   bool isFull() const { return hasFullField(); }
   unsigned queryId() const { return queryParams(); }
   void clearFull() { clearFullField(); }
};

class IdentifierToken : public AbstractToken {
  private:
   STG::SubString ssIdentifier;

  public:
   IdentifierToken(const STG::SubString& identifier) : ssIdentifier(identifier)
      {  setType(TIdentifier); }
   IdentifierToken(const IdentifierToken& source) = default;
   DefineCopy(IdentifierToken)
   DDefineAssign(IdentifierToken)

   const STG::SubString& identifier() const { return ssIdentifier; }
};

class NumberToken : public AbstractToken {
  private:
   STG::SubString ssContent;
   int uSize;
   int uAdditionalSize;

   enum IntegerExtension { IENoExtension, IEBit };
   enum Coding { CDecimal, CBit, COctal, CHexaDecimal };

  protected:
   DefineExtendedParameters(3, AbstractToken)
   DefineSubExtendedParameters(BasicType, 1, INHERITED)
   DefineSubExtendedParameters(IntegerExtension, 1, BasicType)
   DefineSubExtendedParameters(Coding, 2, IntegerExtension)

  public:
   NumberToken(const STG::SubString& content, bool isFloat)
      :  ssContent(content), uSize(0), uAdditionalSize(0)
      {  setType(TNumber); 
         mergeBasicTypeField(isFloat);
      }
   NumberToken(const NumberToken& source) = default;
   DefineCopy(NumberToken)
   DDefineAssign(NumberToken)

   const STG::SubString& getContent() const { return ssContent; }

   bool isInteger() const { return !hasBasicTypeField() && !hasIntegerExtensionField(); }
   bool isFloatingPoint() const { return hasBasicTypeField(); }
   bool isBit() const { return hasIntegerExtensionField(); }
   bool isIntegerOrBit() const { return !hasBasicTypeField(); }

   void setIntegerType() { clearBasicTypeField(); }
   void setFloatingPointType() { mergeBasicTypeField(1); }
   int getSize() const { return uSize; }
   void setSize(int size) { uSize = size; }
   void setDoubleSize(int firstSize, int secondSize) { uSize = firstSize; uAdditionalSize = secondSize; }
   int getFirstSize() const { return uSize; }
   int getSecondSize() const { return uAdditionalSize; }

   void addBitExtension() { mergeIntegerExtensionField(IEBit); }
   bool hasBitExtension() const { return queryIntegerExtensionField() == IEBit; }
   bool hasExtension() const { return hasIntegerExtensionField(); }

   void setBitCoding() { AssumeCondition(!hasCodingField()) mergeCodingField(CBit); }
   void setOctalCoding() { AssumeCondition(!hasCodingField()) mergeCodingField(COctal); }
   void setHexaDecimalCoding() { AssumeCondition(!hasCodingField()) mergeCodingField(CHexaDecimal); }
   bool hasSpecialCoding() const { return hasCodingField(); }
   bool hasBitCoding() const { return queryCodingField() == CBit; }
   bool hasOctalCoding() const { return queryCodingField() == COctal; }
   bool hasHexaDecimalCoding() const { return queryCodingField() == CHexaDecimal; }
};

class KeywordToken : public AbstractToken {
  private:
   typedef AbstractToken inherited;

  public:
   enum Type
      {  TUndefined, TVar, TMulti, TBit, TMultiFloat, TMultiFloatPointer,
         TExtendZero, TExtendSign, TConcat, TReduce, TBitSet, TShiftBit,
         TIncSigned, TIncUnsigned, TDecSigned, TDecUnsigned, TMinSigned, TMinUnsigned,
         TMaxSigned, TMaxUnsigned, TMerge, TTop, TTopBit, TSigned, TUnsigned,
         TUnsignedWithSigned, TSymbolic, TMultiSymbolic, TBitSymbolic, TMultiFloatSymbolic,
         TMultiFloatPointerSymbolic, TExtendZeroSymbolic, TExtendSignSymbolic, TConcatSymbolic,
         TReduceSymbolic, TBitSetSymbolic, TShiftBitSymbolic, TIncSignedSymbolic,
         TIncUnsignedSymbolic, TDecSignedSymbolic, TDecUnsignedSymbolic, TMinSignedSymbolic,
         TMinUnsignedSymbolic, TMaxSignedSymbolic, TMaxUnsignedSymbolic,
         TMergeSymbolic, TTopSymbolic, TTopBitSymbolic, TSignedSymbolic, TUnsignedSymbolic, 
         TUnsignedWithSignedSymbolic, TAssert
      };

  protected:
   DefineExtendedParameters(8, inherited)
   DefineSubExtendedParameters(Type, 6, INHERITED)
   DefineSubExtendedParameters(Size, 2, Type)

  public:
   KeywordToken(Type type)
      {  mergeTypeField(type);
         inherited::setType(AbstractToken::TKeyword);
         inherited::setFull();
      }
   KeywordToken(const KeywordToken& source) = default;
   DefineCopy(KeywordToken)
   Type getType() const { return (Type) queryTypeField(); }
   bool isSized() const { return hasSizeField(); }
   bool isSingleSized() const { return querySizeField() == 1; }
   bool isDoubleSized() const { return querySizeField() == 2; }
};

class SizedKeywordToken : public KeywordToken {
  private:
   int uSize;

  public:
   SizedKeywordToken(Type type, int size) : KeywordToken(type), uSize(size)
      {  KeywordToken::clearFull(); mergeSizeField(1); }
   SizedKeywordToken(const SizedKeywordToken& source) = default;
   DefineCopy(SizedKeywordToken)

   int getSize() const { return uSize; }
};

class FloatSizedKeywordToken : public KeywordToken {
  private:
   int uSizeExponent, uSizeMantissa;

  public:
   FloatSizedKeywordToken(Type type, int sizeExponent, int sizeMantissa)
      :  KeywordToken(type), uSizeExponent(sizeExponent), uSizeMantissa(sizeMantissa)
      {  KeywordToken::clearFull(); mergeSizeField(2); }
   FloatSizedKeywordToken(const FloatSizedKeywordToken& source) = default;
   DefineCopy(FloatSizedKeywordToken)

   int getSizeExponent() const { return uSizeExponent; }
   int getSizeMantissa() const { return uSizeMantissa; }
};

class OperatorPunctuatorToken : public AbstractToken {
  private:
   typedef AbstractToken inherited;

  public:
   enum Type
      {  TUndefined, TAssign, TOpenParen, TCloseParen, TOpenBracket,
         TCloseBracket, TOpenBrace, TCloseBrace, TComma, TMinus, TPlus,
         TLess, TLessOrEqual, TEqual, TDifferent, TGreaterOrEqual, TGreater,
         TTimes, TDivide, TModulo, TBitOr, TBitAnd, TBitXor, TIntersect,
         TBitNegate, TLeftShift, TRightShift, TLogicalOr, TLogicalAnd,
         TLogicalNegate, TConditional, TColon, TSemiColon
      };

  protected:
   DefineExtendedParameters(6, inherited)

  public:
   OperatorPunctuatorToken(Type type)
      {  mergeOwnField(type);
         inherited::setType(AbstractToken::TOperatorPunctuator);
         inherited::setFull();
      }
   OperatorPunctuatorToken(const OperatorPunctuatorToken& source) = default;
   DefineCopy(OperatorPunctuatorToken)
   Type getType() const { return (Type) queryOwnField(); }
};

class Token {
  private:
   unsigned uType;
   PNT::PassPointer<AbstractToken> aptContent;

  public:
   Token() : uType(0) {}
   Token(unsigned type, AbstractToken* token) : uType(type), aptContent(token, PNT::Pointer::Init()) {}
   Token(const AbstractToken& fullToken) : uType(fullToken.queryId()) {}
   Token(AbstractToken* token)
      {  AssumeAllocation(token)
         aptContent.absorbElement(token);
         uType = token->queryId();
      }
   Token(const Token& source)
      :  uType(source.uType), aptContent(source.aptContent)
      {  const_cast<Token&>(source).uType = AbstractToken::TUndefined; }
   Token& operator=(const Token& source)
      {  uType = source.uType;
         aptContent = source.aptContent;
         const_cast<Token&>(source).uType = AbstractToken::TUndefined;
         return *this;
      }
   AbstractToken getFullToken() const
      {  AssumeCondition(uType != 0);
         return AbstractToken(uType, AbstractToken::FullType());
      }

   unsigned getType() const { return uType; }
   void assumeContent();
   AbstractToken* extractContent() { return aptContent.extractElement(); }
   const AbstractToken& getContent() const { return *aptContent; }
   bool hasContent() const { return aptContent.isValid(); }
};

}}} // end of namespace Analyzer::Scalar::Approximate

