/////////////////////////////////
//
// Library   : unit test of Analyzer/Scalar
// Unit      : approximate scalar elements
// File      : TestDomain.cpp
// Author    : Franck Vedrine
// Copyright : CEA LIST - 2019-2020 - all rights reserved
//
// Description :
//   test of general multibit scalar elements.
//

#include "Numerics/Integer.h"
#include "Numerics/HostFloating.h"
#include "TestDomain.h"
#include <iostream>

#include "Numerics/HostFloating.template"

STG::SubString
ConcreteElement::Operator::queryOperatorName() const {
   switch (queryTypeField()) {
      case TPlus: return STG::SString("+");
      case TMinus: return STG::SString("-");
      case TTimes: return STG::SString("*");
      case TDivide: return STG::SString("/");
      case TModulo: return STG::SString("%");
      case TLeftShift: return STG::SString("<<");
      case TRightShift: return STG::SString(">>");
      case TEqual: return STG::SString("==");
      case TDifferent: return STG::SString("!=");
      case TLessOrEqual: return STG::SString("<=");
      case TGreaterOrEqual: return STG::SString(">=");
      case TLess: return STG::SString("<");
      case TGreater: return STG::SString(">");
      case TLogicalAnd: return STG::SString("&&");
      case TLogicalOr: return STG::SString("||");
      case TBitAnd: return STG::SString("&");
      case TBitOr: return STG::SString("|");
      case TBitXOr: return STG::SString("^");
      case TUnaryPlus: return STG::SString("+");
      case TUnaryMinus: return STG::SString("-");
      case TNot: return STG::SString("!");
      case TComplement: return STG::SString("~");
      case TCast:
         switch (queryTypeCastField()) {
             case TCBit: return STG::SString("(bit)");
             case TCMultiBit:
                {  STG::SString result("(multibit");
                   if (uSize > 0)
                       result.cat('_').catInteger(uSize);
                   result.cat(')');
                   return result;
                };
             case TCMultiFloat:
                {  STG::SString result("(multif");
                   if (uSizeExponent > 0 && uSizeMantissa > 0)
                       result.cat('_').catInteger(uSizeExponent)
                             .cat('_').catInteger(uSizeMantissa);
                   result.cat(')');
                   return result;
                };
             case TCMultiFloatPtr:
                {  STG::SString result("(multifptr");
                   if (uSizeExponent > 0 && uSizeMantissa > 0)
                       result.cat('_').catInteger(uSizeExponent)
                             .cat('_').catInteger(uSizeMantissa);
                   result.cat(')');
                   return result;
                };
             default:
                  return STG::SString();
         };
      case TCall:
         switch (queryTypeCallFunctionField()) {
             case TCFExtendZero: return STG::SString("extend_zero_").catInteger(uSize);
             case TCFExtendSign: return STG::SString("extend_sign_").catInteger(uSize);
             case TCFConcat: return STG::SString("concat");
             case TCFReduce: return STG::SString("reduce_").catInteger(uLowBit).cat('_').catInteger(uHighBit);
             case TCFBitSet: return STG::SString("bitset_").catInteger(uLowBit).cat('_').catInteger(uHighBit);
             case TCFShiftBit: return STG::SString("shiftbit_").catInteger(uLowBit);
             case TCFInc: return STG::SString("inc");
             case TCFDec: return STG::SString("dec");
             case TCFMinSigned: return STG::SString("minS");
             case TCFMinUnsigned: return STG::SString("minU");
             case TCFMaxSigned: return STG::SString("maxS");
             case TCFMaxUnsigned: return STG::SString("maxU");
             default:
                  return STG::SString();
         };
      case TConditional: return STG::SString("?:");
      case TMerge: return STG::SString("vv");
      case TIntersect: return STG::SString("^^");
      default:
         break;
   };
   return STG::SString();
}

ConcreteElement::Operator&
ConcreteElement::Operator::setType(Type type, bool isPrefix) {
   AssumeCondition(!hasTypeField() && !hasPrecedenceField() && !hasRightAssociativeField())
   mergeTypeField(type);
   bool isRightAssociative = false;
   Precedence precedence = PUndefined;
   switch (type) {
      case TPlus:
      case TMinus:
         precedence = PAddition;
         break;
      case TTimes:
      case TDivide:
      case TModulo:
         precedence = PMultiplication;
         break;
      case TLeftShift:
      case TRightShift:
         precedence = PShift;
         break;
      case TEqual:
      case TDifferent:
         precedence = PEquality;
         break;
      case TLessOrEqual:
      case TGreaterOrEqual:
      case TLess:
      case TGreater:
         precedence = PComparison;
         break;
      case TLogicalAnd:
         precedence = PLogicalAnd;
         break;
      case TLogicalOr:
         precedence = PLogicalOr;
         break;
      case TBitAnd:
         precedence = PBitAnd;
         break;
      case TBitOr:
         precedence = PBitOr;
         break;
      case TBitXOr:
         precedence = PBitExclusiveOr;
         break;
      case TUnaryPlus:
      case TUnaryMinus:
         isRightAssociative = true;
         precedence = isPrefix ? PUnary : PPostFix;
         break;
      case TNot:
      case TComplement:
         isRightAssociative = true;
         precedence = PUnary;
         break;
      case TCast:
         isRightAssociative = true;
         precedence = PCast;
         break;
      case TCall:
         precedence = PPostFix;
         break;
      case TConditional:
         isRightAssociative = true;
         precedence = PConditional;
         break;
      case TMerge:
         precedence = PMerge;
         break;
      case TIntersect:
         precedence = PIntersect;
         break;
      case TInterval:
         precedence = PPostFix;
         break;
      default:
         break;
   };
   AssumeCondition(precedence);
   mergePrecedenceField(precedence);
   mergeRightAssociativeField(isRightAssociative);
   return *this;
}

Domain
ConcreteElement::OperatorStack::applyInterval(Operator& operation, ArgumentType& argumentType,
      Parser::Arguments& context) {
   AssumeCondition(operation.getType() == Operator::TInterval
         && operation.dFirst.isValid() && operation.dSecond.isValid())
   ArgumentType secondArgumentType = ATUndefined;
   Domain secondArgument = operation.extractSecondArgument(secondArgumentType);
   ArgumentType firstArgumentType = ATUndefined;
   Domain firstArgument = operation.extractFirstArgument(firstArgumentType);
   if (secondArgumentType != firstArgumentType) {
      argumentType = firstArgumentType;
      context.addErrorMessage(STG::SString(
         "interval has incompatible types for elements"));
      return firstArgument;
   };

   argumentType = firstArgumentType;
   if (argumentType == ATMultiBit) {
      Domain result{domain_multibit_create_interval_and_absorb(
            &firstArgument.svalue(), &secondArgument.svalue(), operation.hasSignedTag(),
            operation.isSymbolic())};
      firstArgument.svalue().content = secondArgument.svalue().content = nullptr;
      return result;
   }
   else {
      Domain result{domain_multifloat_create_interval_and_absorb(
            &firstArgument.svalue(), &secondArgument.svalue(),
            operation.isSymbolic())};
      firstArgument.svalue().content = secondArgument.svalue().content = nullptr;
      return result;
   };
}

Domain
ConcreteElement::OperatorStack::applyCallFunction(Operator& operation, ArgumentType& argumentType,
      Parser::Arguments& context) {
   AssumeCondition(operation.getType() == Operator::TCall && operation.dFirst.isValid())
   ArgumentType secondArgumentType = ATUndefined;
   Domain secondArgument;
   if (operation.dSecond.isValid())
      secondArgument = operation.extractSecondArgument(secondArgumentType);
   ArgumentType firstArgumentType = ATUndefined;
   Domain firstArgument = operation.extractFirstArgument(firstArgumentType);

   argumentType = firstArgumentType;
   switch (operation.getTypeCall()) {
      case Operator::TCFExtendZero:
      case Operator::TCFExtendSign:
         if (secondArgument.isValid()) {
            context.addErrorMessage(STG::SString("too more arguments for unary call"));
            return firstArgument;
         };
         if (firstArgumentType != ATMultiBit) {
            context.addErrorMessage(STG::SString(
               "incompatible type for extension: multibit is expected"));
            return firstArgument;
         };
         {  // int originalSize = firstArgument->getSizeInBits();
            if (operation.getExtension() <= 0) {
               context.addErrorMessage(STG::SString(
                  "final size should be greater than original size in extension"));
               return firstArgument;
            };
            context.clearEvaluationEnvironment();
            DomainMultiBitExtendOperation applyOperation;
            applyOperation.new_size = operation.getExtension();
            if (operation.getTypeCall() == Operator::TCFExtendZero)
               applyOperation.type = DMBEOExtendWithZero;
            else
               applyOperation.type = DMBEOExtendWithSign;
            domain_multibit_extend_apply_assign(&firstArgument.svalue(),
                  applyOperation, &context.evaluationEnvironment());
            return firstArgument;
         };
      case Operator::TCFConcat:
         if (!secondArgument.isValid()) {
            context.addErrorMessage(STG::SString("not enough arguments for binary call"));
            return firstArgument;
         };
         if (firstArgumentType != ATMultiBit || secondArgumentType != ATMultiBit) {
            context.addErrorMessage(STG::SString(
               "incompatible type for concatenation: multibit is expected"));
            return firstArgument;
         };
         domain_multibit_binary_apply_assign(&firstArgument.svalue(),
               DMBBOConcat, secondArgument.svalue(), &context.evaluationEnvironment());
         return firstArgument;
      case Operator::TCFReduce:
         if (secondArgument.isValid()) {
            context.addErrorMessage(STG::SString("too more arguments for unary call"));
            return firstArgument;
         };
         if (firstArgumentType != ATMultiBit) {
            context.addErrorMessage(STG::SString(
               "incompatible type for reduction: multibit is expected"));
            return firstArgument;
         };
         {  int originalSize = domain_get_size_in_bits(firstArgument.svalue());
            if (operation.uHighBit > originalSize || operation.uLowBit < 0
                  || operation.uLowBit > operation.uHighBit) {
               context.addErrorMessage(STG::SString(
                  "bad low bit/high bit for reduction"));
               return firstArgument;
            };
            context.clearEvaluationEnvironment();
            DomainMultiBitReduceOperation applyOperation;
            applyOperation.first_bit = operation.uLowBit;
            applyOperation.last_bit = operation.uHighBit;
            domain_multibit_reduce_apply_assign(&firstArgument.svalue(),
                  applyOperation, &context.evaluationEnvironment());
            return firstArgument;
         };
      case Operator::TCFBitSet:
         if (!secondArgument.isValid()) {
            context.addErrorMessage(STG::SString("not enough arguments for binary call"));
            return firstArgument;
         };
         if (firstArgumentType != ATMultiBit || secondArgumentType != ATMultiBit) {
            context.addErrorMessage(STG::SString(
               "incompatible type for concatenation: multibit is expected"));
            return firstArgument;
         };
         {  int originalSize = domain_get_size_in_bits(firstArgument.svalue());
            int argumentSize = domain_get_size_in_bits(secondArgument.svalue());
            if (operation.uHighBit > originalSize || operation.uLowBit < 0
                  || operation.uLowBit > operation.uHighBit) {
               context.addErrorMessage(STG::SString(
                  "bad low bit/high bit for bitset"));
               return firstArgument;
            };
            if (operation.uHighBit - operation.uLowBit + 1 != argumentSize) {
               context.addErrorMessage(STG::SString(
                  "bitset operation should have the size of its argument"));
               return firstArgument;
            }
            context.clearEvaluationEnvironment();
            DomainMultiBitSetOperation applyOperation;
            applyOperation.first_bit = operation.uLowBit;
            applyOperation.last_bit = operation.uHighBit;
            domain_multibit_bitset_apply_assign(&firstArgument.svalue(),
                  applyOperation, secondArgument.svalue(), &context.evaluationEnvironment());
            return firstArgument;
         };
      case Operator::TCFShiftBit:
         argumentType = ATBit;
         if (secondArgument.isValid()) {
            context.addErrorMessage(STG::SString("too more arguments for unary call"));
            return firstArgument;
         };
         if (firstArgumentType != ATMultiBit) {
            context.addErrorMessage(STG::SString(
               "incompatible type for cast shift bit: multibit is expected"));
            return firstArgument;
         };
         {  int originalSize = domain_get_size_in_bits(firstArgument.svalue());
            if (operation.uLowBit < 0 || operation.uLowBit >= originalSize) {
               context.addErrorMessage(STG::SString(
                  "the chozen bit is outside the multibit in the shiftbit operation"));
               return firstArgument;
            };
            context.clearEvaluationEnvironment();
            DomainElement result = domain_multibit_create_cast_shift_bit(firstArgument.svalue(),
                  operation.uLowBit, &context.evaluationEnvironment());
            AssumeCondition(result.content)
            return Domain(std::move(result));
         };
      case Operator::TCFInc:
         if (secondArgument.isValid()) {
            context.addErrorMessage(STG::SString("too more arguments for unary call"));
            return firstArgument;
         };
         if (firstArgumentType != ATMultiBit) {
            context.addErrorMessage(STG::SString(
               "incompatible type for increment: multibit is expected"));
            return firstArgument;
         };
         context.clearEvaluationEnvironment();
         {  bool result = true;
            if (operation.hasUnsignedTag())
               result = domain_multibit_unary_apply_assign(&firstArgument.svalue(),
                     DMBUONextUnsigned, &context.evaluationEnvironment());
            else if (operation.hasSignedTag())
               result = domain_multibit_unary_apply_assign(&firstArgument.svalue(),
                     DMBUONextSigned, &context.evaluationEnvironment());
            else
               context.addErrorMessage(STG::SString(
                  "increment operation should be signed or unsigned"));
            AssumeCondition(result)
         }
         return firstArgument;
      case Operator::TCFDec:
         if (secondArgument.isValid()) {
            context.addErrorMessage(STG::SString("too more arguments for unary call"));
            return firstArgument;
         };
         if (firstArgumentType != ATMultiBit) {
            context.addErrorMessage(STG::SString(
               "incompatible type for increment: multibit is expected"));
            return firstArgument;
         };
         context.clearEvaluationEnvironment();
         {  bool result = true;
            if (operation.hasUnsignedTag())
               result = domain_multibit_unary_apply_assign(&firstArgument.svalue(),
                     DMBUOPrevUnsigned, &context.evaluationEnvironment());
            else if (operation.hasSignedTag())
               result = domain_multibit_unary_apply_assign(&firstArgument.svalue(),
                     DMBUOPrevSigned, &context.evaluationEnvironment());
            else
               context.addErrorMessage(STG::SString(
                  "decrement operation should be signed or unsigned"));
            AssumeCondition(result)
         }
         return firstArgument;
      case Operator::TCFMinSigned:
      case Operator::TCFMinUnsigned:
      case Operator::TCFMaxSigned:
      case Operator::TCFMaxUnsigned:
         if (!secondArgument.isValid()) {
            context.addErrorMessage(STG::SString("not enough arguments for binary call"));
            return firstArgument;
         };
         if (firstArgumentType != secondArgumentType) {
            context.addErrorMessage(STG::SString(
               "incompatible types for concatenation for min/max operations"));
            return firstArgument;
         };
         if (argumentType == ATMultiBit) {
            DomainMultiBitBinaryOperation applyOperation;
            auto type = operation.getTypeCall();
            if (type == Operator::TCFMinSigned)
               applyOperation = DMBBOMinSigned;
            else if (type == Operator::TCFMinUnsigned)
               applyOperation = DMBBOMinUnsigned;
            else if (type == Operator::TCFMaxSigned)
               applyOperation = DMBBOMaxSigned;
            else // type == Operator::TCFMaxUnsigned
               applyOperation = DMBBOMaxUnsigned;
            domain_multibit_binary_apply_assign(&firstArgument.svalue(),
                  applyOperation, secondArgument.svalue(), &context.evaluationEnvironment());
            return firstArgument;
         }
         else if (argumentType == ATBit) {
            DomainBitBinaryOperation applyOperation;
            auto type = operation.getTypeCall();
            if (type == Operator::TCFMinSigned || type == Operator::TCFMinUnsigned)
               applyOperation = DBBOMin;
            else // (type == Operator::TCFMaxUnsigned) || (type == Operator::TCFMaxUnsigned)
               applyOperation = DBBOMax;
            domain_bit_binary_apply_assign(&firstArgument.svalue(),
                  applyOperation, secondArgument.svalue(), &context.evaluationEnvironment());
            return firstArgument;
         }
         else {
            AssumeCondition(argumentType == ATMultiFloat)
            DomainMultiFloatBinaryOperation applyOperation;
            auto type = operation.getTypeCall();
            if (type == Operator::TCFMinSigned || type == Operator::TCFMinUnsigned)
               applyOperation = DMFBOMin;
            else // (type == Operator::TCFMaxUnsigned) || (type == Operator::TCFMaxUnsigned)
               applyOperation = DMFBOMax;
            domain_multifloat_binary_apply_assign(&firstArgument.svalue(),
                  applyOperation, secondArgument.svalue(), &context.evaluationEnvironment());
            return firstArgument;
         }
      default:
         break;
   };
   return Domain();
}

Domain
ConcreteElement::OperatorStack::apply(Operator& operation, ArgumentType& argumentType,
      Parser::Arguments& context) {
   GuardLatticeCreation guard(context.evaluationEnvironment());
   if (operation.hasSymbolicField())
      context.evaluationEnvironment().defaultDomainType = DISFormal;
   Operator::Type type = operation.getType();
   if (type == Operator::TCast) {
      Domain argument = operation.extractLastArgument(argumentType);
      auto typeCast = operation.getTypeCast();
      Domain result;
      context.clearEvaluationEnvironment();
      if (argumentType == ATMultiBit) {
         if (typeCast == Operator::TCBit) {
            result = Domain(domain_multibit_create_cast_bit(argument.svalue(), &context.evaluationEnvironment()));
            argumentType = ATBit;
         }
         else if (typeCast == Operator::TCMultiBit) {
            if (!operation.hasSize()) {
               context.addErrorMessage(STG::SString(
                  "multibit cast should define a target size when applied on multibits"));
               return argument;
            };
            int oldSize = domain_get_size_in_bits(argument.svalue());
            bool boolResult;
            if (operation.getSize() < oldSize)
               boolResult = domain_multibit_reduce_apply_assign(&argument.svalue(),
                     DomainMultiBitReduceOperation{0, operation.getSize()-1},
                     &context.evaluationEnvironment());
            else
               boolResult = domain_multibit_extend_apply_assign(&argument.svalue(),
                     DomainMultiBitExtendOperation{DMBEOExtendWithZero, operation.getSize()-oldSize},
                     &context.evaluationEnvironment());
            AssumeCondition(boolResult)
            result = std::move(argument);
         }
         else if (typeCast == Operator::TCMultiFloat) {
            if (!operation.hasSizeExponent() || !operation.hasSizeMantissa()) {
               context.addErrorMessage(STG::SString(
                  "multifloat cast should define an exponent and a mantissa size when applied on multibits"));
               return argument;
            };
            result = Domain(domain_multibit_create_cast_multifloat(argument.svalue(),
                     operation.getSizeExponent()+operation.getSizeMantissa()+1, true,
                     &context.evaluationEnvironment()));
            argumentType = ATMultiFloat;
         }
         else if (typeCast == Operator::TCMultiFloatPtr) {
            if (!operation.hasSizeExponent() || !operation.hasSizeMantissa()) {
               context.addErrorMessage(STG::SString(
                  "multifloatptr cast should define an exponent and a mantissa size when applied on multibits"));
               return argument;
            };
            if (domain_get_size_in_bits(argument.svalue()) != operation.getSizeExponent() + operation.getSizeMantissa() + 1) {
               context.addErrorMessage(STG::SString(
                  "multifloatptr cast should keep the original size of its argument"));
               return argument;
            };
            result = Domain(domain_multibit_create_cast_multifloat_ptr(argument.svalue(),
                     operation.getSizeExponent()+operation.getSizeMantissa()+1, true,
                     &context.evaluationEnvironment()));
            argumentType = ATMultiFloat;
         }
         else
            {  AssumeUncalled }
      }
      else if (argumentType == ATBit) {
         if (typeCast == Operator::TCBit)
            result = std::move(argument);
         else if (typeCast == Operator::TCMultiBit) {
            int sizeInBits = operation.hasSize() ? operation.getSize() : 1;
            result = Domain(domain_bit_create_cast_multibit(argument.svalue(),
                     sizeInBits, &context.evaluationEnvironment()));
         }
         else {
            context.addErrorMessage(STG::SString(
               "unsupported cast operation on bit"));
            return argument;
         };
      }
      else {
         AssumeCondition(argumentType == ATMultiFloat)
         if (typeCast == Operator::TCMultiBit) {
            int sizeInBits = operation.hasSize() ? operation.getSize() : domain_get_size_in_bits(argument.svalue());
            result = Domain(domain_multifloat_create_cast_multibit(argument.svalue(),
                     sizeInBits, &context.evaluationEnvironment()));
            argumentType = ATMultiBit;
         }
         else if (typeCast == Operator::TCMultiFloat) {
            if (!operation.hasSizeExponent() || !operation.hasSizeMantissa()) {
               context.addErrorMessage(STG::SString(
                  "multifloat cast should define an exponent and a mantissa size when applied on multifloats"));
               return argument;
            };
            domain_multifloat_cast_multifloat_assign(&argument.svalue(),
                     operation.getSizeExponent()+operation.getSizeMantissa()+1,
                     &context.evaluationEnvironment());
            return argument;
         }
         else {
            context.addErrorMessage(STG::SString(
               "unsupported cast operation on multifloat"));
            return argument;
         };
      }
      return result;
   };
   
   int argumentsNumber = operation.queryArgumentsNumber();
   if (argumentsNumber == 1) {
      Domain argument = operation.extractLastArgument(argumentType);
      switch (type) {
         case Operator::TUnaryPlus:
            return argument;
         case Operator::TUnaryMinus:
            context.clearEvaluationEnvironment();
            {  bool boolResult = true;
               if (argumentType == ATMultiBit)
                  boolResult = domain_multibit_unary_apply_assign(&argument.svalue(),
                        DMBUOOppositeSigned, &context.evaluationEnvironment());
               else if (argumentType == ATMultiFloat)
                  boolResult = domain_multifloat_unary_apply_assign(&argument.svalue(),
                        DMFUOOpposite, &context.evaluationEnvironment());
               else
                  context.addErrorMessage(STG::SString(
                     "unsupported type for the unary - operation"));
               AssumeCondition(boolResult)
            }
            return argument;
         case Operator::TNot:
            if (argumentType != ATBit) {
               context.addErrorMessage(STG::SString(
                  "types others than bits do not support unary ! operation"));
               return argument;
            };
            context.clearEvaluationEnvironment();
            domain_bit_unary_apply_assign(&argument.svalue(),
                  DBUONegate, &context.evaluationEnvironment());
            return argument;
         case Operator::TComplement:
            if (argumentType != ATMultiBit) {
               context.addErrorMessage(STG::SString(
                  "types other than multibits do not support unary ~ operation"));
               return argument;
            };
            context.clearEvaluationEnvironment();
            domain_multibit_unary_apply_assign(&argument.svalue(),
                  DMBUOBitNegate, &context.evaluationEnvironment());
            return argument;
         default:
            context.addErrorMessage(STG::SString("unknown unary operation type"));
            return argument;
      };
   }
   else if (argumentsNumber == 2) {
      ArgumentType secondArgumentType = ATUndefined;
      Domain secondArgument = operation.extractSecondArgument(secondArgumentType);
      ArgumentType firstArgumentType = ATUndefined;
      Domain firstArgument = operation.extractFirstArgument(firstArgumentType);
      if (firstArgumentType != secondArgumentType) {
         argumentType = firstArgumentType;
         context.addErrorMessage(STG::SString("arguments have incompatible types"));
         return firstArgument;
      };

      if (type >= Operator::TEqual && type <= Operator::TGreater) {
         auto thisType = firstArgumentType;
         argumentType = ATBit;
         if (thisType == ATMultiBit) {
            DomainMultiBitCompareOperation compareType = DMBCOUndefined;
            switch (type) {
               case Operator::TEqual: compareType = DMBCOCompareEqual; break;
               case Operator::TDifferent: compareType = DMBCOCompareDifferent; break;
               case Operator::TLessOrEqual:
                  if (operation.hasUnsignedTag())
                     compareType = DMBCOCompareLessOrEqualUnsigned;
                  else if (operation.hasSignedTag())
                     compareType = DMBCOCompareLessOrEqualSigned;
                  else if (operation.hasFloatTag())
                     compareType = DMBCOCompareLessOrEqualFloat;
                  else {
                     context.addErrorMessage(STG::SString("comparison on multibits should be signed or unsigned"));
                     return firstArgument;
                  };
                  break;
               case Operator::TGreaterOrEqual:
                  if (operation.hasUnsignedTag())
                     compareType = DMBCOCompareGreaterOrEqualUnsigned;
                  else if (operation.hasSignedTag())
                     compareType = DMBCOCompareGreaterOrEqualSigned;
                  else if (operation.hasFloatTag())
                     compareType = DMBCOCompareGreaterOrEqualFloat;
                  else {
                     context.addErrorMessage(STG::SString("comparison on multibits should be signed or unsigned"));
                     return firstArgument;
                  };
                  break;
               case Operator::TLess:
                  if (operation.hasUnsignedTag())
                     compareType = DMBCOCompareLessUnsigned;
                  else if (operation.hasSignedTag())
                     compareType = DMBCOCompareLessSigned;
                  else if (operation.hasFloatTag())
                     compareType = DMBCOCompareLessFloat;
                  else {
                     context.addErrorMessage(STG::SString("comparison on multibits should be signed or unsigned"));
                     return firstArgument;
                  };
                  break;
               case Operator::TGreater:
                  if (operation.hasUnsignedTag())
                     compareType = DMBCOCompareGreaterUnsigned;
                  else if (operation.hasSignedTag())
                     compareType = DMBCOCompareGreaterSigned;
                  else if (operation.hasFloatTag())
                     compareType = DMBCOCompareGreaterFloat;
                  else {
                     context.addErrorMessage(STG::SString("comparison on multibits should be signed or unsigned"));
                     return firstArgument;
                  };
                  break;
               default:
                  AssumeUncalled
                  compareType = DMBCOCompareEqual; break;
            };
            context.clearEvaluationEnvironment();
            return Domain(domain_multibit_binary_compare_domain(firstArgument.svalue(),
                  compareType, secondArgument.svalue(), &context.evaluationEnvironment()));
         }
         else if (thisType == ATBit) {
            DomainBitCompareOperation compareType = DBCOUndefined;
            switch (type) {
               case Operator::TEqual: compareType = DBCOCompareEqual; break;
               case Operator::TDifferent: compareType = DBCOCompareDifferent; break;
               case Operator::TLessOrEqual: compareType = DBCOCompareLessOrEqual; break;
               case Operator::TGreaterOrEqual: compareType = DBCOCompareGreaterOrEqual; break;
               case Operator::TLess: compareType = DBCOCompareLess; break;
               case Operator::TGreater: compareType = DBCOCompareGreater; break;
               default:
                  AssumeUncalled
                  compareType = DBCOCompareEqual; break;
            };
            context.clearEvaluationEnvironment();
            return Domain(domain_bit_binary_compare_domain(firstArgument.svalue(),
                  compareType, secondArgument.svalue(), &context.evaluationEnvironment()));
         }
         else {
            AssumeCondition(thisType == ATMultiFloat)
            DomainMultiFloatCompareOperation compareType = DMFCOUndefined;
            switch (type) {
               case Operator::TEqual: compareType = DMFCOCompareEqual; break;
               case Operator::TDifferent: compareType = DMFCOCompareDifferent; break;
               case Operator::TLessOrEqual: compareType = DMFCOCompareLessOrEqual; break;
               case Operator::TGreaterOrEqual: compareType = DMFCOCompareGreaterOrEqual; break;
               case Operator::TLess: compareType = DMFCOCompareLess; break;
               case Operator::TGreater: compareType = DMFCOCompareGreater; break;
               default:
                  AssumeUncalled
                  compareType = DMFCOCompareEqual; break;
            };
            context.clearEvaluationEnvironment();
            return Domain(domain_multifloat_binary_compare_domain(firstArgument.svalue(),
                  compareType, secondArgument.svalue(), &context.evaluationEnvironment()));
         };
      }

      argumentType = firstArgumentType;
      if (argumentType == ATMultiBit) {
         DomainMultiBitBinaryOperation applyType = DMBBOUndefined;
         switch (type) {
            case Operator::TPlus:
               if (operation.hasUnsignedTag())
                  applyType = DMBBOPlusUnsigned;
               else if (operation.hasSignedTag())
                  applyType = DMBBOPlusSigned;
               else if (operation.hasUnsignedWithSignedTag())
                  applyType = DMBBOPlusUnsignedWithSigned;
               else if (operation.hasFloatTag())
                  applyType = DMBBOPlusFloat;
               else
                  applyType = DMBBOPlusUnsigned;
               break;
            case Operator::TMinus:
               if (operation.hasUnsignedTag())
                  applyType = DMBBOMinusUnsigned;
               else if (operation.hasSignedTag())
                  applyType = DMBBOMinusSigned;
               else if (operation.hasFloatTag())
                  applyType = DMBBOMinusFloat;
               else
                  applyType = DMBBOMinusUnsigned;
               break;
            case Operator::TTimes:
               if (operation.hasUnsignedTag())
                  applyType = DMBBOTimesUnsigned;
               else if (operation.hasSignedTag())
                  applyType = DMBBOTimesSigned;
               else if (operation.hasFloatTag())
                  applyType = DMBBOTimesFloat;
               else
                  applyType = DMBBOTimesUnsigned;
               break;
            case Operator::TDivide:
               if (operation.hasUnsignedTag())
                  applyType = DMBBODivideUnsigned;
               else if (operation.hasSignedTag())
                  applyType = DMBBODivideSigned;
               else if (operation.hasFloatTag())
                  applyType = DMBBODivideFloat;
               else
                  applyType = DMBBODivideUnsigned;
               break;
            case Operator::TModulo:
               if (operation.hasUnsignedTag())
                  applyType = DMBBOModuloUnsigned;
               else if (operation.hasSignedTag())
                  applyType = DMBBOModuloSigned;
               else
                  applyType = DMBBOModuloUnsigned;
               break;
            case Operator::TLeftShift:
               applyType = DMBBOLeftShift;
               break;
            case Operator::TRightShift:
               if (operation.hasUnsignedTag())
                  applyType = DMBBOLogicalRightShift;
               else if (operation.hasSignedTag())
                  applyType = DMBBOArithmeticRightShift;
               else
                  applyType = DMBBOLogicalRightShift;
               break;
            case Operator::TLogicalAnd:
               context.addErrorMessage(STG::SString("unsupported operator && on multibits"));
               return firstArgument;
            case Operator::TLogicalOr:
               context.addErrorMessage(STG::SString("unsupported operator || on multibits"));
               return firstArgument;
            case Operator::TBitAnd: applyType = DMBBOBitAnd; break;
            case Operator::TBitOr: applyType = DMBBOBitOr; break;
            case Operator::TBitXOr: applyType = DMBBOBitExclusiveOr; break;
            case Operator::TMerge:
               context.clearEvaluationEnvironment();
               domain_merge(&firstArgument.svalue(),
                     secondArgument.svalue(), &context.evaluationEnvironment());
               return firstArgument;
            case Operator::TIntersect:
               context.clearEvaluationEnvironment();
               domain_intersect(&firstArgument.svalue(),
                     secondArgument.svalue(), &context.evaluationEnvironment());
               return firstArgument;
            default:
               context.addErrorMessage(STG::SString("unknown binary operation type for multibit"));
               return firstArgument;
         };
         context.clearEvaluationEnvironment();
         domain_multibit_binary_apply_assign(&firstArgument.svalue(),
               applyType, secondArgument.svalue(), &context.evaluationEnvironment());
         return firstArgument;
      }
      else if (argumentType == ATBit) {
         DomainBitBinaryOperation applyType = DBBOUndefined;
         switch (type) {
            case Operator::TPlus: applyType = DBBOPlus; break;
            case Operator::TMinus: applyType = DBBOMinus; break;
            case Operator::TLogicalAnd: applyType = DBBOAnd; break;
            case Operator::TLogicalOr: applyType = DBBOOr; break;
            case Operator::TBitAnd: applyType = DBBOAnd; break;
            case Operator::TBitOr: applyType = DBBOOr; break;
            case Operator::TBitXOr: applyType = DBBOExclusiveOr; break;
            case Operator::TMerge:
               context.clearEvaluationEnvironment();
               domain_merge(&firstArgument.svalue(),
                     secondArgument.svalue(), &context.evaluationEnvironment());
               return firstArgument;
            case Operator::TIntersect:
               context.clearEvaluationEnvironment();
               domain_intersect(&firstArgument.svalue(),
                     secondArgument.svalue(), &context.evaluationEnvironment());
               return firstArgument;
            default:
               context.addErrorMessage(STG::SString("unknown binary operation type for bit"));
               return firstArgument;
         };
         context.clearEvaluationEnvironment();
         domain_bit_binary_apply_assign(&firstArgument.svalue(),
               applyType, secondArgument.svalue(), &context.evaluationEnvironment());
         return firstArgument;
      }
      else {
         AssumeCondition(argumentType == ATMultiFloat)
         DomainMultiFloatBinaryOperation applyType = DMFBOUndefined;
         switch (type) {
            case Operator::TPlus: applyType = DMFBOPlus; break;
            case Operator::TMinus: applyType = DMFBOMinus; break;
            case Operator::TTimes: applyType = DMFBOTimes; break;
            case Operator::TDivide: applyType = DMFBODivide; break;
            case Operator::TMerge:
               context.clearEvaluationEnvironment();
               domain_merge(&firstArgument.svalue(),
                     secondArgument.svalue(), &context.evaluationEnvironment());
               return firstArgument;
            case Operator::TIntersect:
               context.clearEvaluationEnvironment();
               domain_intersect(&firstArgument.svalue(),
                     secondArgument.svalue(), &context.evaluationEnvironment());
               return firstArgument;
            default:
               context.addErrorMessage(STG::SString("unsupported binary operation for floating point type"));
               return firstArgument;
         };
         context.clearEvaluationEnvironment();
         domain_multifloat_binary_apply_assign(&firstArgument.svalue(),
               applyType, secondArgument.svalue(), &context.evaluationEnvironment());
         return firstArgument;
      }
   }
   else {
      AssumeCondition(type == Operator::TConditional)
      ArgumentType thirdArgumentType = ATUndefined;
      Domain thirdArgument = operation.extractThirdArgument(thirdArgumentType);
      ArgumentType secondArgumentType = ATUndefined;
      Domain secondArgument = operation.extractSecondArgument(secondArgumentType);
      ArgumentType firstArgumentType = ATUndefined;
      Domain firstArgument = operation.extractFirstArgument(firstArgumentType);
      if (secondArgumentType != thirdArgumentType) {
         argumentType = secondArgumentType;
         context.addErrorMessage(STG::SString("arguments of ternary operator ?: have incompatible types"));
         return secondArgument;
      };
      if (firstArgumentType != ATBit) {
         argumentType = secondArgumentType;
         context.addErrorMessage(STG::SString("condition of ternary operator ?: should have bit type"));
         return secondArgument;
      };
      argumentType = secondArgumentType;
      DomainElement result;
      if (secondArgumentType == ATBit)
         result = domain_bit_guard_assign(&firstArgument.svalue(),
               &secondArgument.svalue(), &thirdArgument.svalue(), &context.evaluationEnvironment());
      else if (secondArgumentType == ATMultiBit)
         result = domain_multibit_guard_assign(&firstArgument.svalue(),
               &secondArgument.svalue(), &thirdArgument.svalue(), &context.evaluationEnvironment());
      else // secondArgumentType == ATMultiFloat
         result = domain_multifloat_guard_assign(&firstArgument.svalue(),
               &secondArgument.svalue(), &thirdArgument.svalue(), &context.evaluationEnvironment());
      return Domain(std::move(result));
   };
   return Domain();
}

ConcreteElement::Operator&
ConcreteElement::OperatorStack::pushOperator(Operator::Type typeOperator,
      int leftSubExpressions, Parser::Arguments& context, bool& hasFailed) {
   Operator* result = &getSLast();
   if (!result->isValid())
      result->setType(typeOperator).setLeftSubExpressions(leftSubExpressions);
   else {
      PNT::AutoPointer<Operator> expressionOperator(new Operator(), PNT::Pointer::Init());
      expressionOperator->setType(typeOperator)
         .setLeftSubExpressions(leftSubExpressions);
      Operator* lastOperator = nullptr;
      bool isLeftAssociative = expressionOperator->isLeftAssociative();
      Domain resultElement;
      ArgumentType typeResult = ATUndefined;
      while (!isEmpty() && (isLeftAssociative
            ? ((lastOperator = &getSLast())->getPrecedence()
               >= expressionOperator->getPrecedence())
            : ((lastOperator = &getSLast())->getPrecedence()
               > expressionOperator->getPrecedence()))) {
         if (resultElement.isValid())
            lastOperator->absorbLastArgument(std::move(resultElement), typeResult);
         if (!lastOperator->isFinished())
            break;
         resultElement = apply(*lastOperator, typeResult, context); 
         if (!resultElement.isValid()) {
            hasFailed = true;
            return *lastOperator;
         };
         lastOperator = nullptr;
         freeLast();
      };
      if (!resultElement.isValid()) {
         AssumeCondition(lastOperator != nullptr);
         if (!lastOperator->isBinaryOperator() ||
               lastOperator->hasSecondArgument())
            resultElement = lastOperator->extractLastArgument(typeResult);
         else
            lastOperator->incLeftSubExpressions();
      };
      expressionOperator->absorbFirstArgument(std::move(resultElement), typeResult);
      insertNewAtEnd(result = expressionOperator.extractElement());
   };
   return *result;
}


Domain
ConcreteElement::OperatorStack::clear(ArgumentType& argumentType, Parser::Arguments& context) {
   Domain result;
   if (count() == 1) {
      Operator* operation = &getSLast();
      if (!operation->isValid()) {
         result = operation->extractLastArgument(argumentType);
         freeAll();
         return result;
      };
   };

   ArgumentType resultType = ATUndefined;
   while (!isEmpty()) {
      PNT::AutoPointer<Operator> operation(&extractLast(), PNT::Pointer::Init());
      if (result.isValid())
         operation->absorbLastArgument(std::move(result), resultType);

      if (!operation->isFinished()) {
         context.addErrorMessage(STG::SString("unexpected end of expression"));
         return Domain();
      };
      result = apply(*operation, resultType, context);
      if (!result.isValid())
         return result;
   };
   argumentType = resultType;
   return result;
}

STG::Lexer::Base::ReadResult
ConcreteElement::readToken(Parser::State& state, Parser::Arguments& arguments) {
   enum Delimiters
      {  DBegin, DBeginStack, DAfterPrimary, DAfterPrimaryStack, DAfterParen, DAfterParenStack,
         DAfterTypeCast, DParen, DParenStack, DEndParen, DInterval, DAfterInterval, DEndBracket,
         DSet, DSetStack, DFirstSetElement, DFirstSetElementStack, DSetElement, DSetElementStack,
         DEndSet, DEndSetStack, DAfterSet, DAfterSetStack, DAfterExtendZero, DAfterExtendZeroStack,
         DAfterExtendSign, DAfterExtendSignStack, DAfterConcat, DAfterConcatStack, DAfterReduce,
         DAfterReduceStack, DAfterBitSet, DAfterBitSetStack, DAfterShiftBit, DAfterShiftBitStack,
         DAfterIncSigned, DAfterIncSignedStack, DAfterIncUnsigned, DAfterIncUnsignedStack,
         DAfterDecSigned, DAfterDecSignedStack, DAfterDecUnsigned, DAfterDecUnsignedStack,
         DAfterMinSigned, DAfterMinSignedStack, DAfterMinUnsigned, DAfterMinUnsignedStack,
         DAfterMaxSigned, DAfterMaxSignedStack, DAfterMaxUnsigned, DAfterMaxUnsignedStack,
         DAfterExtendZeroSymbolic, DAfterExtendZeroSymbolicStack, DAfterExtendSignSymbolic,
         DAfterExtendSignSymbolicStack, DAfterConcatSymbolic, DAfterConcatSymbolicStack,
         DAfterReduceSymbolic, DAfterReduceSymbolicStack, DAfterBitSetSymbolic,
         DAfterBitSetSymbolicStack, DAfterShiftBitSymbolic, DAfterShiftBitSymbolicStack,
         DAfterIncSignedSymbolic, DAfterIncSignedSymbolicStack, DAfterIncUnsignedSymbolic,
         DAfterIncUnsignedSymbolicStack, DAfterDecSignedSymbolic, DAfterDecSignedSymbolicStack,
         DAfterDecUnsignedSymbolic, DAfterDecUnsignedSymbolicStack, DAfterMinSignedSymbolic,
         DAfterMinSignedSymbolicStack, DAfterMinUnsignedSymbolic, DAfterMinUnsignedSymbolicStack,
         DAfterMaxSignedSymbolic, DAfterMaxSignedSymbolicStack, DAfterMaxUnsignedSymbolic,
         DAfterMaxUnsignedSymbolicStack, DInFunction, DInExtensionFunction, DInShiftBitFunction,
         DInExtensionFunctionSize, DInShiftBitFunctionBitSelection, DInBitSetFunction,
         DInBitSetFunctionLowBit, DInBitSetFunctionAfterLowBit, DInBitSetFunctionHighBit,
         DInReductionFunctionLowBit, DInReductionFunctionAfterLowBit, DInReductionFunctionHighBit,
         DInReductionFunction, DEndCallFunction, DBeforeParenStack
      };

   ReadResult result = RRNeedChars;
   switch (state.point()) {
      DefineLightParseCase(Begin)
      DefineParseCase(BeginStack)
         AssumeCondition(!dElement.isValid())
         {  AbstractToken token = arguments.lexer().queryToken();
            switch (token.getType()) {
               case AbstractToken::TIdentifier:
                  {  const IdentifierToken& token = (const IdentifierToken&)
                        arguments.lexer().getContentToken();
                     MapOfElements::Cursor cursor(arguments.environment());
                     if (!arguments.environment().locateKey(token.identifier(),
                           cursor, COL::VirtualCollection::RPExact)) {
                        DefineAddError(STG::SString("unknown identifier \"")
                           .cat(token.identifier()).cat("\" in expression"));
                        DefineReduceAndParse
                     };
                     const ConcreteElement& element = cursor.elementAt().getValue();
                     if (state.point() == DBegin) {
                        if (element.isValid()) {
                           dElement = element.dElement;
                           atType = element.atType;
                        }
                        else {
                           DefineAddError(STG::SString("unassigned identifier \"")
                              .cat(token.identifier()).cat("\" in expression"));
                           DefineReduceAndParse
                        };
                     }
                     else
                        state.getUnionResult((OperatorStack*) nullptr).pushLastArgument(
                           Domain(element.dElement), element.atType);
                     DefineGotoCaseWithIncrement(DAfterPrimary - DBegin, LAfterPrimary)
                  };
               case AbstractToken::TNumber:
                  {  const NumberToken& token = (const NumberToken&)
                        arguments.lexer().getContentToken();
                     Domain element;
                     ArgumentType resultType = ATUndefined;
                     if (token.isIntegerOrBit()) {
                        Numerics::BigInteger val;
                        STG::DIOObject::ISSubString in(token.getContent());
                        Numerics::BigInteger::FormatParameters format;
                        format.setText();
                        if (token.hasSpecialCoding()) {
                           if (token.hasHexaDecimalCoding())
                              format.setText().setHexaDecimal();
                           else if (token.hasOctalCoding()) {
                              DefineAddError(STG::SString("unsupported octal coding for number in expression"));
                              DefineReduceAndParse
                              // format.setText().setOctal();
                           }
                           else if (token.hasBitCoding()) {
                              DefineAddError(STG::SString("unsupported bit coding for number in expression"));
                              DefineReduceAndParse
                              // format.setText().setBitCoding();
                           };
                        };
                        val.read(in, format);
                        if (token.hasBitExtension()) {
                           element = Domain(domain_bit_create_constant(!val.isZero()));
                           resultType = ATBit;
                        }
                        else {
                           val.adjustBitSize(token.getSize());
                           element = Domain(domain_multibit_create_constant(
                              DomainIntegerConstant{ token.getSize(), false, val[0]}));
                           resultType = ATMultiBit;
                        };
                     }
                     else {
                        AssumeCondition(token.isFloatingPoint())
                        int sizeExponent = token.getFirstSize();
                        int sizeMantissa = token.getSecondSize();
                        if (sizeExponent == 0 || sizeMantissa == 0) {
                           DefineAddError(STG::SString("exponent and mantissa should have a positive size in floating point number"));
                           DefineReduceAndParse
                        };
                        Numerics::DDouble::Access::ReadParameters format;
                        format.setText();
                        if (token.hasSpecialCoding()) {
                           DefineAddError(STG::SString("unsupported special coding for floating number in expression"));
                           DefineReduceAndParse
                           // if (token.hasHexaDecimalCoding())
                           //    format.setHexaDecimal();
                           // else if (token.hasOctalCoding())
                           //    format.setOctal();
                           // else if (token.hasBitCoding())
                           //    format.setBit();
                        };
                        STG::DIOObject::ISSubString in(token.getContent());
                        Numerics::TDoubleElement<Numerics::TFloatingBase<Numerics::DoubleTraits> > val;
                        val.read(in, format);
                        element = Domain(domain_multifloat_create_constant(
                                 DomainFloatingPointConstant{ sizeExponent+sizeMantissa+1, val.implementation() }));
                        resultType = ATMultiFloat;
                     }
                     if (state.point() == DBegin) {
                        dElement = std::move(element);
                        atType = resultType;
                     }
                     else
                        state.getUnionResult((OperatorStack*) nullptr)
                           .pushLastArgument(std::move(element), resultType);
                  }
                  DefineGotoCaseWithIncrement(DAfterPrimary - DBegin, LAfterPrimary)
               case AbstractToken::TKeyword:
                  switch (((const KeywordToken&) token).getType()) {
                     case KeywordToken::TExtendZero:
                        DefineGotoCaseWithIncrement(DAfterExtendZero - DBegin, LAfterExtendZero)
                     case KeywordToken::TExtendSign:
                        DefineGotoCaseWithIncrement(DAfterExtendSign - DBegin, LAfterExtendSign)
                     case KeywordToken::TConcat:
                        DefineGotoCaseWithIncrement(DAfterConcat - DBegin, LAfterConcat)
                     case KeywordToken::TReduce:
                        DefineGotoCaseWithIncrement(DAfterReduce - DBegin, LAfterReduce)
                     case KeywordToken::TBitSet:
                        DefineGotoCaseWithIncrement(DAfterBitSet - DBegin, LAfterBitSet)
                     case KeywordToken::TShiftBit:
                        DefineGotoCaseWithIncrement(DAfterShiftBit - DBegin, LAfterShiftBit)
                     case KeywordToken::TIncSigned:
                        DefineGotoCaseWithIncrement(DAfterIncSigned - DBegin, LAfterIncSigned)
                     case KeywordToken::TIncUnsigned:
                        DefineGotoCaseWithIncrement(DAfterIncUnsigned - DBegin, LAfterIncUnsigned)
                     case KeywordToken::TDecSigned:
                        DefineGotoCaseWithIncrement(DAfterDecSigned - DBegin, LAfterDecSigned)
                     case KeywordToken::TDecUnsigned:
                        DefineGotoCaseWithIncrement(DAfterDecUnsigned - DBegin, LAfterDecUnsigned)
                     case KeywordToken::TMinSigned:
                        DefineGotoCaseWithIncrement(DAfterMinSigned - DBegin, LAfterMinSigned)
                     case KeywordToken::TMinUnsigned:
                        DefineGotoCaseWithIncrement(DAfterMinUnsigned - DBegin, LAfterMinUnsigned)
                     case KeywordToken::TMaxSigned:
                        DefineGotoCaseWithIncrement(DAfterMaxSigned - DBegin, LAfterMaxSigned)
                     case KeywordToken::TMaxUnsigned:
                        DefineGotoCaseWithIncrement(DAfterMaxUnsigned - DBegin, LAfterMaxUnsigned)
                     case KeywordToken::TTop:
                        {  if (!((const KeywordToken&) token).isSized()) {
                              DefineAddError(STG::SString("the top keyword should have a size"));
                              DefineReduceAndParse
                           };
                           if (((const KeywordToken&) token).isSingleSized()) {
                              AssumeCondition(dynamic_cast<const SizedKeywordToken*>(&arguments.lexer().getContentToken()))
                              Domain element = Domain(domain_multibit_create_top(
                                    ((const SizedKeywordToken&) arguments.lexer().getContentToken()).getSize(),
                                    false /* isSymbolic */));
                              if (state.point() == DBegin) {
                                 dElement = std::move(element);
                                 atType = ATMultiBit;
                              }
                              else
                                 state.getUnionResult((OperatorStack*) nullptr)
                                       .pushLastArgument(std::move(element), ATMultiBit);
                           }
                           else {
                              AssumeCondition(dynamic_cast<const FloatSizedKeywordToken*>(&arguments.lexer().getContentToken()))
                              const auto& token = (const FloatSizedKeywordToken&) arguments.lexer().getContentToken();
                              Domain element = Domain(domain_multifloat_create_top(
                                    token.getSizeExponent()+token.getSizeMantissa()+1,
                                    false /* isSymbolic */));
                              if (state.point() == DBegin) {
                                 dElement = std::move(element);
                                 atType = ATMultiFloat;
                              }
                              else
                                 state.getUnionResult((OperatorStack*) nullptr)
                                       .pushLastArgument(std::move(element), ATMultiFloat);
                           };
                        };
                        DefineGotoCaseWithIncrement(DAfterPrimary - DBegin, LAfterPrimary)
                     case KeywordToken::TTopBit:
                        {  Domain element = Domain(domain_bit_create_top(false /* isSymbolic */));
                           if (state.point() == DBegin) {
                              dElement = std::move(element);
                              atType = ATBit;
                           }
                           else
                              state.getUnionResult((OperatorStack*) nullptr)
                                    .pushLastArgument(std::move(element), ATBit);
                        };
                        DefineGotoCaseWithIncrement(DAfterPrimary - DBegin, LAfterPrimary)
                     case KeywordToken::TUnsigned:
                     case KeywordToken::TSigned:
                        if (state.point() == DBegin) {
                           DefineAddError(STG::SString("keyword 'U/S' encountered"
                              " where an expression was expected"));
                           DefineReduceAndParse
                        }
                        else {
                           OperatorStack* operatorStack = &state.getUnionResult((OperatorStack*) nullptr);
                           if (((const KeywordToken&) token).getType() == KeywordToken::TUnsigned)
                              operatorStack->getSLast().setUnsignedTag();
                           else
                              operatorStack->getSLast().setSignedTag();
                           DefineGotoCase(BeginStack)
                        }
                     case KeywordToken::TUnsignedWithSigned:
                        if (state.point() == DBegin) {
                           DefineAddError(STG::SString("keyword 'US' encountered"
                              " where an expression was expected"));
                           DefineReduceAndParse
                        }
                        else {
                           OperatorStack* operatorStack = &state.getUnionResult((OperatorStack*) nullptr);
                           operatorStack->getSLast().setUnsignedWithSignedTag();
                           DefineGotoCase(BeginStack)
                        }
                     case KeywordToken::TSymbolic:
                        if (state.point() == DBegin) {
                           DefineAddError(STG::SString("keyword '_s' encountered"
                              " where an expression was expected"));
                           DefineReduceAndParse
                        }
                        else {
                           OperatorStack* operatorStack = &state.getUnionResult((OperatorStack*) nullptr);
                           operatorStack->getSLast().setSymbolic();
                           DefineGotoCase(BeginStack)
                        }
                     case KeywordToken::TExtendZeroSymbolic:
                        DefineGotoCaseWithIncrement(DAfterExtendZeroSymbolic - DBegin, LAfterExtendZeroSymbolic)
                     case KeywordToken::TExtendSignSymbolic:
                        DefineGotoCaseWithIncrement(DAfterExtendSignSymbolic - DBegin, LAfterExtendSignSymbolic)
                     case KeywordToken::TConcatSymbolic:
                        DefineGotoCaseWithIncrement(DAfterConcatSymbolic - DBegin, LAfterConcatSymbolic)
                     case KeywordToken::TReduceSymbolic:
                        DefineGotoCaseWithIncrement(DAfterReduceSymbolic - DBegin, LAfterReduceSymbolic)
                     case KeywordToken::TBitSetSymbolic:
                        DefineGotoCaseWithIncrement(DAfterBitSetSymbolic - DBegin, LAfterBitSetSymbolic)
                     case KeywordToken::TShiftBitSymbolic:
                        DefineGotoCaseWithIncrement(DAfterShiftBitSymbolic - DBegin, LAfterShiftBitSymbolic)
                     case KeywordToken::TIncSignedSymbolic:
                        DefineGotoCaseWithIncrement(DAfterIncSignedSymbolic - DBegin, LAfterIncSignedSymbolic)
                     case KeywordToken::TIncUnsignedSymbolic:
                        DefineGotoCaseWithIncrement(DAfterIncUnsignedSymbolic - DBegin, LAfterIncUnsignedSymbolic)
                     case KeywordToken::TDecSignedSymbolic:
                        DefineGotoCaseWithIncrement(DAfterDecSignedSymbolic - DBegin, LAfterDecSignedSymbolic)
                     case KeywordToken::TDecUnsignedSymbolic:
                        DefineGotoCaseWithIncrement(DAfterDecUnsignedSymbolic - DBegin, LAfterDecUnsignedSymbolic)
                     case KeywordToken::TMinSignedSymbolic:
                        DefineGotoCaseWithIncrement(DAfterMinSignedSymbolic - DBegin, LAfterMinSignedSymbolic)
                     case KeywordToken::TMinUnsignedSymbolic:
                        DefineGotoCaseWithIncrement(DAfterMinUnsignedSymbolic - DBegin, LAfterMinUnsignedSymbolic)
                     case KeywordToken::TMaxSignedSymbolic:
                        DefineGotoCaseWithIncrement(DAfterMaxSignedSymbolic - DBegin, LAfterMaxSignedSymbolic)
                     case KeywordToken::TMaxUnsignedSymbolic:
                        DefineGotoCaseWithIncrement(DAfterMaxUnsignedSymbolic - DBegin, LAfterMaxUnsignedSymbolic)
                     case KeywordToken::TTopSymbolic:
                        {  if (!((const KeywordToken&) token).isSized()) {
                              DefineAddError(STG::SString("the top keyword should have a size"));
                              DefineReduceAndParse
                           };
                           if (((const KeywordToken&) token).isSingleSized()) {
                              AssumeCondition(dynamic_cast<const SizedKeywordToken*>(&arguments.lexer().getContentToken()))
                              Domain element = Domain(domain_multibit_create_top(
                                    ((const SizedKeywordToken&) arguments.lexer().getContentToken()).getSize(),
                                    true /* isSymbolic */));
                              if (state.point() == DBegin) {
                                 dElement = std::move(element);
                                 atType = ATMultiBit;
                              }
                              else
                                 state.getUnionResult((OperatorStack*) nullptr)
                                       .pushLastArgument(std::move(element), ATMultiBit);
                           }
                           else {
                              AssumeCondition(dynamic_cast<const FloatSizedKeywordToken*>(&arguments.lexer().getContentToken()))
                              const auto& token = (const FloatSizedKeywordToken&) arguments.lexer().getContentToken();
                              Domain element = Domain(domain_multifloat_create_top(
                                    token.getSizeExponent()+token.getSizeMantissa()+1,
                                    true /* isSymbolic */));
                              if (state.point() == DBegin) {
                                 dElement = std::move(element);
                                 atType = ATMultiFloat;
                              }
                              else
                                 state.getUnionResult((OperatorStack*) nullptr)
                                       .pushLastArgument(std::move(element), ATMultiFloat);
                           };
                        };
                        DefineGotoCaseWithIncrement(DAfterPrimary - DBegin, LAfterPrimary)
                     case KeywordToken::TTopBitSymbolic:
                        {  Domain element = Domain(domain_bit_create_top(true /* isSymbolic */));
                           if (state.point() == DBegin) {
                              dElement = std::move(element);
                              atType = ATBit;
                           }
                           else
                              state.getUnionResult((OperatorStack*) nullptr)
                                    .pushLastArgument(std::move(element), ATBit);
                        };
                        DefineGotoCaseWithIncrement(DAfterPrimary - DBegin, LAfterPrimary)
                     case KeywordToken::TSignedSymbolic:
                     case KeywordToken::TUnsignedSymbolic:
                        if (state.point() == DBegin) {
                           DefineAddError(STG::SString("keyword 'S_s' encountered"
                              " where an expression was expected"));
                           DefineReduceAndParse
                        }
                        else {
                           OperatorStack* operatorStack = &state.getUnionResult((OperatorStack*) nullptr);
                           operatorStack->getSLast().setSymbolic();
                           if (((const KeywordToken&) token).getType() == KeywordToken::TUnsignedSymbolic)
                              operatorStack->getSLast().setUnsignedTag();
                           else
                              operatorStack->getSLast().setSignedTag();
                           DefineGotoCase(BeginStack)
                        }
                     case KeywordToken::TUnsignedWithSignedSymbolic:
                        if (state.point() == DBegin) {
                           DefineAddError(STG::SString("keyword 'US_s' encountered"
                              " where an expression was expected"));
                           DefineReduceAndParse
                        }
                        else {
                           OperatorStack* operatorStack = &state.getUnionResult((OperatorStack*) nullptr);
                           operatorStack->getSLast().setSymbolic();
                           operatorStack->getSLast().setUnsignedWithSignedTag();
                           DefineGotoCase(BeginStack)
                        }
                     default:
                        DefineAddError(STG::SString("keyword encountered"
                           " where an expression was expected"));
                        DefineReduceAndParse
                  };
               case AbstractToken::TOperatorPunctuator:
                  switch (((const OperatorPunctuatorToken&) token).getType()) {
                     case OperatorPunctuatorToken::TOpenParen:
                        DefineGotoCaseWithIncrement(DAfterParen - DBegin, LAfterParen)
                     case OperatorPunctuatorToken::TOpenBracket:
                        {  if (state.point() == DBegin)
                              state.absorbUnionResult(OperatorStack());
                           OperatorStack* operatorStack = &state.getUnionResult((OperatorStack*) nullptr);
                           ConcreteElement* subElement = new ConcreteElement;
                           operatorStack->absorbSubElement(subElement);
                           operatorStack->pushPrefixBinaryOperator(Operator::TInterval);
                           DefineShift(Interval, *subElement, &ConcreteElement::readToken,
                                 (Parser::State::UnionResult<ConcreteElement, OperatorStack>*) nullptr);
                        };
                     case OperatorPunctuatorToken::TOpenBrace:
                        DefineGotoCaseWithIncrement(DSet - DBegin, LSet)
                     case OperatorPunctuatorToken::TMinus:
                        {  if (state.point() == DBegin)
                              state.absorbUnionResult(OperatorStack());
                           OperatorStack* operatorStack = &state.getUnionResult((OperatorStack*) nullptr);
                           operatorStack->pushPrefixUnaryOperator(Operator::TUnaryMinus);
                        };
                        DefineGotoCase(BeginStack)
                     case OperatorPunctuatorToken::TPlus:
                        {  if (state.point() == DBegin)
                              state.absorbUnionResult(OperatorStack());
                           OperatorStack* operatorStack = &state.getUnionResult((OperatorStack*) nullptr);
                           operatorStack->pushPrefixUnaryOperator(Operator::TUnaryPlus);
                        };
                        DefineGotoCase(BeginStack)
                     case OperatorPunctuatorToken::TBitNegate:
                        {  if (state.point() == DBegin)
                              state.absorbUnionResult(OperatorStack());
                           OperatorStack* operatorStack = &state.getUnionResult((OperatorStack*) nullptr);
                           operatorStack->pushPrefixUnaryOperator(Operator::TComplement);
                        };
                        DefineGotoCase(BeginStack)
                     case OperatorPunctuatorToken::TLogicalNegate:
                        {  if (state.point() == DBegin)
                              state.absorbUnionResult(OperatorStack());
                           OperatorStack* operatorStack = &state.getUnionResult((OperatorStack*) nullptr);
                           operatorStack->pushPrefixUnaryOperator(Operator::TNot);
                        };
                        DefineGotoCase(BeginStack)
                     default:
                        DefineAddError(STG::SString("operator/punctuator encountered"
                           " where an expression was expected"));
                        DefineReduceAndParse
                  };
               default:
                  AssumeUncalled
            };
         }
         break;
      DefineParseCase(AfterPrimary)
      DefineParseCase(AfterPrimaryStack)
         AssumeCondition(state.point() == DAfterPrimary ? dElement.isValid() : !dElement.isValid())
         {  AbstractToken token = arguments.lexer().queryToken();
            if (token.getType() == AbstractToken::TOperatorPunctuator) {
               auto type = ((const OperatorPunctuatorToken&) token).getType();
               if (type >= OperatorPunctuatorToken::TMinus
                        && type <= OperatorPunctuatorToken::TLogicalAnd
                        && type != OperatorPunctuatorToken::TBitNegate) {
                  Operator::Type operatorType = Operator::TUndefined;
                  switch (type) {
                     case OperatorPunctuatorToken::TPlus:   operatorType = Operator::TPlus; break;
                     case OperatorPunctuatorToken::TMinus:  operatorType = Operator::TMinus; break;
                     case OperatorPunctuatorToken::TTimes:  operatorType = Operator::TTimes; break;
                     case OperatorPunctuatorToken::TDivide: operatorType = Operator::TDivide; break;
                     case OperatorPunctuatorToken::TModulo: operatorType = Operator::TModulo; break;
                     case OperatorPunctuatorToken::TBitXor: operatorType = Operator::TBitXOr; break;
                     case OperatorPunctuatorToken::TBitAnd: operatorType = Operator::TBitAnd; break;
                     case OperatorPunctuatorToken::TBitOr:  operatorType = Operator::TBitOr; break;
                     case OperatorPunctuatorToken::TLess:   operatorType = Operator::TLess; break;
                     case OperatorPunctuatorToken::TGreater:operatorType = Operator::TGreater; break;
                     case OperatorPunctuatorToken::TLeftShift: operatorType = Operator::TLeftShift; break;
                     case OperatorPunctuatorToken::TRightShift: operatorType = Operator::TRightShift; break;
                     case OperatorPunctuatorToken::TEqual: operatorType = Operator::TEqual; break;
                     case OperatorPunctuatorToken::TDifferent: operatorType = Operator::TDifferent; break;
                     case OperatorPunctuatorToken::TLessOrEqual: operatorType = Operator::TLessOrEqual; break;
                     case OperatorPunctuatorToken::TGreaterOrEqual: operatorType = Operator::TGreaterOrEqual; break;
                     case OperatorPunctuatorToken::TLogicalAnd: operatorType = Operator::TLogicalAnd; break;
                     case OperatorPunctuatorToken::TLogicalOr: operatorType = Operator::TLogicalOr; break;
                     case OperatorPunctuatorToken::TIntersect: operatorType = Operator::TIntersect; break;
                     default: break;
                  };
                  if (operatorType != 0) {
                     if (state.point() == DAfterPrimary) {
                        state.absorbUnionResult(OperatorStack());
                        OperatorStack* operatorStack = &state.getUnionResult((OperatorStack*) nullptr);
                        operatorStack->pushBinaryOperator(operatorType)
                           .absorbFirstArgument(std::move(dElement), atType)
                           .decLeftSubExpressions();
                        atType = ATUndefined;
                     }
                     else {
                        OperatorStack* operatorStack = &state.getUnionResult((OperatorStack*) nullptr);
                        bool hasFailed = false;
                        operatorStack->pushBinaryOperator(operatorType, arguments, hasFailed);
                        if (hasFailed)
                           DefineReduce
                     }
                     DefineGotoCase(BeginStack)
                  }

                  if (state.point() == DAfterPrimaryStack) {
                     OperatorStack* operatorStack = &state.getUnionResult((OperatorStack*) nullptr);
                     ArgumentType typeResult = ATUndefined;
                     Domain element = operatorStack->clear(typeResult, arguments);
                     if (!element.isValid() && arguments.doesStopAfterTooManyErrors())
                        return RRFinished;
                  };
                  DefineReduceAndParse
               }
               else {
                  if (type == OperatorPunctuatorToken::TConditional) {
                     OperatorStack* operatorStack;
                     if (state.point() == DAfterPrimary) {
                        state.absorbUnionResult(OperatorStack());
                        operatorStack = &state.getUnionResult((OperatorStack*) nullptr);
                        operatorStack->pushLastArgument(std::move(dElement), atType);
                        atType = ATUndefined;
                     }
                     else
                        operatorStack = &state.getUnionResult((OperatorStack*) nullptr);
                     bool hasFailed = false;
                     operatorStack->pushConditionalOperator(arguments, hasFailed);
                     if (hasFailed) {
                        if (arguments.doesStopAfterTooManyErrors())
                           return RRFinished;
                        DefineReduce
                     };
                     DefineGotoCase(BeginStack)
                  }
                  else if (type == OperatorPunctuatorToken::TColon
                        && state.point() == DAfterPrimaryStack) {
                     OperatorStack* operatorStack = &state.getUnionResult((OperatorStack*) nullptr);
                     Operator* lastOperator = &operatorStack->getSLast();
                     if (lastOperator->isValid()
                           && lastOperator->getLeftSubExpressions() == 0) {
                        bool isStackEmpty = false;
                        while (!isStackEmpty && lastOperator->getPrecedence()
                                 >= Operator::PConditional) {
                           ArgumentType resultType = ATUndefined;
                           Domain resultElement
                              = operatorStack->apply(*lastOperator, resultType, arguments); 
                           if (!resultElement.isValid()) {
                              if (arguments.doesStopAfterTooManyErrors())
                                 return RRFinished;
                              DefineReduce
                           };
                           operatorStack->freeLast();
                           isStackEmpty = operatorStack->isEmpty();
                           if (isStackEmpty) {
                              dElement = std::move(resultElement);
                              atType = resultType;
                              DefineReduceAndParse
                           };
                           lastOperator = &operatorStack->getSLast();
                           lastOperator->absorbLastArgument(std::move(resultElement), resultType);
                           if (!lastOperator->isValid()
                                 || lastOperator->getLeftSubExpressions() != 0)
                              break;
                        };
                     };
                     if (lastOperator && lastOperator->isValid()
                           && lastOperator->getType() == Operator::TConditional
                           && lastOperator->getLeftSubExpressions() == 1)
                        DefineGotoCase(BeginStack)
                  }
               };
            }
            else if (token.getType() == AbstractToken::TKeyword) {
               auto type = ((const KeywordToken&) token).getType();
               if (type == KeywordToken::TMerge || type == KeywordToken::TMergeSymbolic) {
                  if (state.point() == DAfterPrimary) {
                     state.absorbUnionResult(OperatorStack());
                     OperatorStack* operatorStack = &state.getUnionResult((OperatorStack*) nullptr);
                     operatorStack->pushBinaryOperator(Operator::TMerge)
                        .absorbFirstArgument(std::move(dElement), atType)
                        .decLeftSubExpressions();
                     atType = ATUndefined;
                     if (type == KeywordToken::TMergeSymbolic)
                        operatorStack->getSLast().setSymbolic();
                  }
                  else {
                     OperatorStack* operatorStack = &state.getUnionResult((OperatorStack*) nullptr);
                     bool hasFailed = false;
                     operatorStack->pushBinaryOperator(Operator::TMerge, arguments, hasFailed);
                     if (type == KeywordToken::TMergeSymbolic)
                        operatorStack->getSLast().setSymbolic();
                     if (hasFailed)
                        DefineReduce
                  }
                  DefineGotoCase(BeginStack)
               }
            }
         };
         if (!dElement.isValid()) {
            AssumeCondition(state.point() == DAfterPrimaryStack)
            OperatorStack operatorStack = std::move(state.getUnionResult((OperatorStack*) nullptr));
            state.freeUnionResult((OperatorStack*) nullptr);
            dElement = operatorStack.clear(atType, arguments);
            if (!dElement.isValid() && arguments.doesStopAfterTooManyErrors())
               result = RRFinished;
         };
         DefineReduceAndParse
      DefineParseCase(AfterParen)
      DefineLightParseCase(AfterParenStack)
         {  AbstractToken token = arguments.lexer().queryToken();
            if (token.getType() == AbstractToken::TKeyword) {
               switch (((const KeywordToken&) token).getType()) {
                  case KeywordToken::TMulti:
                  case KeywordToken::TBit:
                  case KeywordToken::TMultiFloat:
                  case KeywordToken::TMultiFloatPointer:
                  case KeywordToken::TMultiSymbolic:
                  case KeywordToken::TBitSymbolic:
                  case KeywordToken::TMultiFloatSymbolic:
                  case KeywordToken::TMultiFloatPointerSymbolic:
                     {  if (state.point() == DAfterParen)
                           state.absorbUnionResult(OperatorStack());
                        OperatorStack* operatorStack = &state.getUnionResult((OperatorStack*) nullptr);
                        auto tokenType = ((const KeywordToken&) token).getType();
                        bool isSymbolic = false;
                        if (tokenType >= KeywordToken::TMultiSymbolic) {
                           tokenType = (KeywordToken::Type) (tokenType - KeywordToken::TMultiSymbolic + KeywordToken::TMulti);
                           isSymbolic = true;
                        };
                        Operator::TypeCast typeCast = (tokenType == KeywordToken::TMulti)
                           ? Operator::TCMultiBit : ((tokenType == KeywordToken::TBit)
                           ? Operator::TCBit : ((tokenType == KeywordToken::TMultiFloat)
                           ? Operator::TCMultiFloat : Operator::TCMultiFloatPtr));
                        auto& operation = operatorStack->pushPrefixUnaryOperator(Operator::TCast)
                           .setTypeCast(typeCast);
                        if (isSymbolic)
                           operation.setSymbolic();
                        if (((const KeywordToken&) token).isSized()) {
                           if (((const KeywordToken&) token).isSingleSized()) {
                              AssumeCondition(dynamic_cast<const SizedKeywordToken*>(&arguments.lexer().getContentToken()))
                              operation.setBitSize(((const SizedKeywordToken&) arguments.lexer().getContentToken()).getSize());
                           }
                           else {
                              AssumeCondition(dynamic_cast<const FloatSizedKeywordToken*>(&arguments.lexer().getContentToken()))
                              const auto& token = (const FloatSizedKeywordToken&) arguments.lexer().getContentToken();
                              operation.setSizeExponent(token.getSizeExponent());
                              operation.setSizeMantissa(token.getSizeMantissa());
                           };
                        };
                     };
                     DefineGotoCase(AfterTypeCast)
                  default:
                     break;
               };
            }
         };
         {  ConcreteElement* subElement;
            if (state.point() == DBegin) {
               state.absorbUnionResult(ConcreteElement());
               subElement = &state.getUnionResult((ConcreteElement*) nullptr);
            }
            else
               state.getUnionResult((OperatorStack*) nullptr).absorbSubElement(subElement = new ConcreteElement());
            DefineShiftAndParseWithIncrement(DParen - DAfterParen, LParen, *subElement,
                  &ConcreteElement::readToken, (Parser::State::UnionResult<ConcreteElement, OperatorStack>*) nullptr);
         };
      DefineParseCase(AfterTypeCast)
         {  AbstractToken token = arguments.lexer().queryToken();
            if (token.getType() == AbstractToken::TOperatorPunctuator) {
               if (((const OperatorPunctuatorToken&) token).getType()
                     == OperatorPunctuatorToken::TCloseParen)
                  DefineGotoCase(BeginStack)
            }
         };
         DefineAddError(STG::SString("unknow cast operator"))
         DefineGotoCaseAndParse(AfterTypeCast)
      DefineParseCase(Paren)
      DefineLightParseCase(ParenStack)
         {  PNT::AutoPointer<ConcreteElement> subElement;
            if (state.point() == DParen) {
               subElement.absorbElement(new ConcreteElement(std::move(
                           state.getUnionResult((ConcreteElement*) nullptr))));
               state.freeUnionResult((ConcreteElement*) nullptr);
            }
            else
               subElement.absorbElement(state.getUnionResult((OperatorStack*) nullptr)
                     .extractSubElement());
            AbstractToken token = arguments.lexer().queryToken();
            if (subElement->dElement.isValid()) {
               if (state.point() == DParen) {
                  dElement = std::move(subElement->dElement);
                  atType = subElement->atType;
               }
               else
                  state.getUnionResult((OperatorStack*) nullptr).pushLastArgument(
                     std::move(subElement->dElement), subElement->atType);

               if (token.getType() == AbstractToken::TOperatorPunctuator
                     && ((const OperatorPunctuatorToken&) token).getType()
                        == OperatorPunctuatorToken::TCloseParen)
                 DefineGotoCaseWithIncrement(DAfterPrimary - DParen, LAfterPrimary)
            }
            else {
               if (token.getType() == AbstractToken::TOperatorPunctuator
                     && ((const OperatorPunctuatorToken&) token).getType()
                        == OperatorPunctuatorToken::TCloseParen)
                  DefineReduce
            };
            DefineAddError(STG::SString("')' was expected"
               " during the parsing of a subexpression"));
            DefineGotoCase(EndParen)
         };
         DefineReduce
      DefineParseCase(EndParen)
         {  AbstractToken token = arguments.lexer().queryToken();
            if (token.getType() == AbstractToken::TOperatorPunctuator
                  && ((const OperatorPunctuatorToken&) token).getType()
                     == OperatorPunctuatorToken::TCloseParen)
               DefineReduce
         };
         DefineGotoCase(EndParen)
      DefineParseCase(Interval)
         {  OperatorStack* operatorStack = &state.getUnionResult((OperatorStack*) nullptr);
            PNT::AutoPointer<ConcreteElement> subElement(operatorStack
                  ->extractSubElement(), PNT::Pointer::Init());
            Operator* interval = &operatorStack->getSLast();
            AssumeCondition(interval->getType() == Operator::TInterval)
            AbstractToken token = arguments.lexer().queryToken();
            if (subElement->dElement.isValid()) {
               operatorStack->pushLastArgument(
                  std::move(subElement->dElement), subElement->atType);
               if (interval->getLeftSubExpressions() == 1) {
                  if (token.getType() == AbstractToken::TOperatorPunctuator
                        && ((const OperatorPunctuatorToken&) token).getType()
                           == OperatorPunctuatorToken::TComma) {
                     operatorStack->absorbSubElement(subElement.extractElement());
                     DefineShift(Interval, operatorStack->getSubElement(),
                           &ConcreteElement::readToken, (Parser::State::UnionResult<ConcreteElement, OperatorStack>*) nullptr);
                  };
               }
               else {
                  AssumeCondition(interval->getLeftSubExpressions() == 0)
                  if (token.getType() == AbstractToken::TOperatorPunctuator
                        && ((const OperatorPunctuatorToken&) token).getType()
                           == OperatorPunctuatorToken::TCloseBracket)
                     DefineGotoCase(AfterInterval)
               };
            };
            operatorStack->freeLast();
            if (token.getType() == AbstractToken::TOperatorPunctuator
                  && ((const OperatorPunctuatorToken&) token).getType()
                     == OperatorPunctuatorToken::TCloseBracket)
               DefineReduce
            DefineAddError(STG::SString("']' was expected"
               " during the parsing of an interval"));
            DefineGotoCase(EndBracket)
         };
         DefineReduce
      DefineParseCase(AfterInterval)
         {  OperatorStack* operatorStack = &state.getUnionResult((OperatorStack*) nullptr);
            Operator* interval = &operatorStack->getSLast();
            AssumeCondition(interval->getType() == Operator::TInterval)
            AbstractToken token = arguments.lexer().queryToken();
            AssumeCondition(interval->getLeftSubExpressions() == 0)
            bool hasReadToken = false;
            if (token.getType() == AbstractToken::TKeyword) {
               auto type = ((const KeywordToken&) token).getType();
               if (type == KeywordToken::TSymbolic) {
                  interval->setSymbolic();
                  hasReadToken = true;
               }
               else if (type == KeywordToken::TSigned) {
                  interval->setSignedTag();
                  hasReadToken = true;
               }
               else if (type == KeywordToken::TUnsigned) {
                  interval->setUnsignedTag();
                  hasReadToken = true;
               }
               else if (type == KeywordToken::TSignedSymbolic) {
                  interval->setSignedTag();
                  interval->setSymbolic();
                  hasReadToken = true;
               }
               else if (type == KeywordToken::TUnsignedSymbolic) {
                  interval->setUnsignedTag();
                  interval->setSymbolic();
                  hasReadToken = true;
               }
            };
            ArgumentType resultType = ATUndefined;
            Domain resultElement = operatorStack
               ->applyInterval(*interval, resultType, arguments);
            operatorStack->freeLast();
            interval = nullptr;
            if (operatorStack->isEmpty()) {
               state.freeUnionResult((OperatorStack*) nullptr);
               operatorStack = nullptr;
               dElement = std::move(resultElement);
               atType = resultType;
               if (!hasReadToken)
                  DefineGotoCaseAndParse(AfterPrimary)
               else
                  DefineGotoCase(AfterPrimary)
            }
            else {
               operatorStack->pushLastArgument(std::move(resultElement), resultType);
               if (!hasReadToken)
                  DefineGotoCaseAndParse(AfterPrimaryStack)
               else
                  DefineGotoCase(AfterPrimaryStack)
            };
         };
      DefineParseCase(EndBracket)
         {  AbstractToken token = arguments.lexer().queryToken();
            if (token.getType() == AbstractToken::TOperatorPunctuator
                  && ((const OperatorPunctuatorToken&) token).getType()
                     == OperatorPunctuatorToken::TCloseBracket)
               DefineGotoCase(AfterPrimaryStack)
         };
         DefineGotoCase(EndBracket)
      DefineParseCase(Set)
      DefineLightParseCase(SetStack)
         {  AbstractToken token = arguments.lexer().queryToken();
            if (token.getType() == AbstractToken::TOperatorPunctuator) {
               auto type = ((const OperatorPunctuatorToken&) token).getType();
               if (type == OperatorPunctuatorToken::TCloseBrace) {
                  DefineAddError(STG::SString("empty set is not allowed"))
                  DefineGotoCaseWithIncrement(DAfterPrimary - DSet, LAfterPrimary)
               };
            }
            ConcreteElement* subElement;
            if (state.point() == DSet) {
               state.absorbUnionResult(ConcreteElement());
               subElement = &state.getUnionResult((ConcreteElement*) nullptr);
            }
            else
               state.getUnionResult((OperatorStack*) nullptr)
                  .absorbSubElement(subElement = new ConcreteElement);
            DefineShiftAndParseWithIncrement(DFirstSetElement - DSet, LFirstSetElement,
                  *subElement, &ConcreteElement::readToken, (Parser::State::UnionResult<ConcreteElement, OperatorStack>*) nullptr)
         };
      DefineParseCase(FirstSetElement)
      DefineLightParseCase(FirstSetElementStack)
         {  PNT::AutoPointer<ConcreteElement> subElement;
            if (state.point() == DFirstSetElement) {
               subElement.absorbElement(new ConcreteElement(std::move(
                           state.getUnionResult((ConcreteElement*) nullptr))));
               state.freeUnionResult((ConcreteElement*) nullptr);
            }
            else
               subElement.absorbElement(state.getUnionResult(
                        (OperatorStack*) nullptr).extractSubElement());
            AbstractToken token = arguments.lexer().queryToken();
            if (subElement->dElement.isValid()) {
               Domain elementAsDisjunction(domain_create_disjunction_and_absorb(
                     &subElement->dElement.svalue()));
               ArgumentType subResultType = subElement->atType;
               if (state.point() == DFirstSetElement) {
                  dElement = std::move(elementAsDisjunction);
                  atType = subResultType;
               }
               else
                  state.getUnionResult((OperatorStack*) nullptr).pushLastArgument(
                     std::move(elementAsDisjunction), subResultType);

               if (token.getType() == AbstractToken::TOperatorPunctuator) {
                  auto type = ((const OperatorPunctuatorToken&) token).getType();
                  if (type == OperatorPunctuatorToken::TCloseBrace)
                     DefineGotoCaseWithIncrement(DAfterSet - DFirstSetElement, LAfterSet)
                  if (type == OperatorPunctuatorToken::TComma) {
                     ConcreteElement* element;
                     if (state.point() == DFirstSetElement) {
                        state.absorbUnionResult(std::move(*subElement));
                        element = &state.getUnionResult((ConcreteElement*) nullptr);
                     }
                     else
                        state.getUnionResult((OperatorStack*) nullptr)
                           .absorbSubElement(element = subElement.extractElement());
                     DefineShiftWithIncrement(DSetElement - DFirstSetElement,
                        LSetElement, *element, &ConcreteElement::readToken, (Parser::State::UnionResult<ConcreteElement, OperatorStack>*) nullptr)
                  }
               }
            };
            DefineAddError(STG::SString("'}' was expected"
               " during the parsing of a set"));
            DefineGotoCaseWithIncrement(DEndSet - DFirstSetElement, LEndSet)
         };
      DefineParseCase(SetElement)
      DefineLightParseCase(SetElementStack)
         {  PNT::AutoPointer<ConcreteElement> subElement;
            Domain* disjunction;
            ArgumentType resultType;
            if (state.point() == DSetElement) {
               subElement.absorbElement(new ConcreteElement(std::move(
                           state.getUnionResult((ConcreteElement*) nullptr))));
               state.freeUnionResult((ConcreteElement*) nullptr);
               disjunction = &dElement;
               resultType = atType;
            }
            else {
               OperatorStack* operatorStack = &state.getUnionResult((OperatorStack*) nullptr);
               subElement.absorbElement(operatorStack->extractSubElement());
               disjunction = &const_cast<Domain&>(operatorStack->getLastArgument(resultType));
            }
            AbstractToken token = arguments.lexer().queryToken();
            if (subElement->dElement.isValid()) {
               ArgumentType localSubType = subElement->atType;
               if (localSubType != resultType)
                  DefineAddError(STG::SString("set has incompatible types for elements"))
               else
                  domain_disjunction_absorb(&disjunction->svalue(), &subElement->dElement.svalue());
               if (token.getType() == AbstractToken::TOperatorPunctuator) {
                  auto type = ((const OperatorPunctuatorToken&) token).getType();
                  if (type == OperatorPunctuatorToken::TCloseBrace)
                     DefineGotoCaseWithIncrement(DAfterSet - DSetElement, LAfterSet)
                  if (type == OperatorPunctuatorToken::TComma) {
                     ConcreteElement* element;
                     if (state.point() == DSetElement) {
                        state.absorbUnionResult(std::move(*subElement));
                        element = &state.getUnionResult((ConcreteElement*) nullptr);
                     }
                     else
                        state.getUnionResult((OperatorStack*) nullptr)
                           .absorbSubElement(element = subElement.extractElement());
                     DefineShiftWithIncrement(DSetElement - DSetElement,
                        LSetElement, *element, &ConcreteElement::readToken, (Parser::State::UnionResult<ConcreteElement, OperatorStack>*) nullptr)
                  }
               }
            };
            DefineAddError(STG::SString("'}' was expected"
               " during the parsing of a set"));
            DefineGotoCaseWithIncrement(DEndSet - DSetElement, LEndSet)
         };
      DefineParseCase(EndSet)
      DefineLightParseCase(EndSetStack)
         {  AbstractToken token = arguments.lexer().queryToken();
            if (token.getType() == AbstractToken::TOperatorPunctuator
                  && ((const OperatorPunctuatorToken&) token).getType()
                     == OperatorPunctuatorToken::TCloseBrace)
               DefineGotoCaseWithIncrement(DAfterSet - DEndSet, LAfterSet)
         };
         DefineGotoCaseWithIncrement(DEndSet - DEndSet, LEndSet)
      DefineParseCase(AfterSet)
      DefineLightParseCase(AfterSetStack)
         {  AbstractToken token = arguments.lexer().queryToken();
            if (token.getType() == AbstractToken::TKeyword
                  && ((const KeywordToken&) token).getType()
                     == KeywordToken::TSymbolic)
               DefineGotoCaseWithIncrement(DAfterPrimary - DAfterSet, LAfterPrimary)
         };
         DefineGotoCaseAndParseWithIncrement(DAfterPrimary - DAfterSet, LAfterPrimary)
      DefineParseCase(AfterExtendZero)
      DefineLightParseCase(AfterExtendZeroStack)
      DefineParseCase(AfterExtendZeroSymbolic)
      DefineLightParseCase(AfterExtendZeroSymbolicStack)
      DefineParseCase(AfterExtendSign)
      DefineLightParseCase(AfterExtendSignStack)
      DefineParseCase(AfterExtendSignSymbolic)
      DefineLightParseCase(AfterExtendSignSymbolicStack)
         {  AbstractToken token = arguments.lexer().queryToken();
            if (token.getType() == AbstractToken::TOperatorPunctuator
                  && ((const OperatorPunctuatorToken&) token).getType()
                     == OperatorPunctuatorToken::TOpenParen) {
               if (((state.point() - DAfterExtendZero) & 0x1) == 0)
                  state.absorbUnionResult(OperatorStack());
               OperatorStack* operatorStack = &state.getUnionResult((OperatorStack*) nullptr);
               ConcreteElement* subElement = new ConcreteElement;
               operatorStack->absorbSubElement(subElement);
               int point = state.point();
               Operator::TypeCallFunction typeCall =
                  ((point >= DAfterExtendZero && point <= DAfterExtendZeroStack)
                     || (point >= DAfterExtendZeroSymbolic && point <= DAfterExtendZeroSymbolicStack))
                  ? Operator::TCFExtendZero : Operator::TCFExtendSign;
               auto& operation = operatorStack->pushCallFunction(typeCall, 1);
               if (point >= DAfterExtendZeroSymbolic)
                  operation.setSymbolic();
               DefineShift(InExtensionFunction, *subElement, &ConcreteElement::readToken,
                     (Parser::State::UnionResult<ConcreteElement, OperatorStack>*) nullptr);
            }
         };
         if (!arguments.addErrorMessage(STG::SString("extension construction expects an '('")))
            return RRFinished;
         DefineReduceAndParse
      DefineParseCase(AfterConcat)
      DefineLightParseCase(AfterConcatStack)
      DefineParseCase(AfterConcatSymbolic)
      DefineLightParseCase(AfterConcatSymbolicStack)
      DefineParseCase(AfterMinSigned)
      DefineLightParseCase(AfterMinSignedStack)
      DefineParseCase(AfterMinSignedSymbolic)
      DefineLightParseCase(AfterMinSignedSymbolicStack)
      DefineParseCase(AfterMinUnsigned)
      DefineLightParseCase(AfterMinUnsignedStack)
      DefineParseCase(AfterMinUnsignedSymbolic)
      DefineLightParseCase(AfterMinUnsignedSymbolicStack)
      DefineParseCase(AfterMaxSigned)
      DefineLightParseCase(AfterMaxSignedStack)
      DefineParseCase(AfterMaxSignedSymbolic)
      DefineLightParseCase(AfterMaxSignedSymbolicStack)
      DefineParseCase(AfterMaxUnsigned)
      DefineLightParseCase(AfterMaxUnsignedStack)
      DefineParseCase(AfterMaxUnsignedSymbolic)
      DefineLightParseCase(AfterMaxUnsignedSymbolicStack)
         {  AbstractToken token = arguments.lexer().queryToken();
            if (token.getType() == AbstractToken::TOperatorPunctuator
                  && ((const OperatorPunctuatorToken&) token).getType()
                     == OperatorPunctuatorToken::TOpenParen) {
               if (((state.point() - DAfterConcat) & 0x1) == 0)
                  state.absorbUnionResult(OperatorStack());
               OperatorStack* operatorStack = &state.getUnionResult((OperatorStack*) nullptr);
               ConcreteElement* subElement = new ConcreteElement;
               operatorStack->absorbSubElement(subElement);
               int point = state.point();
               Operator::TypeCallFunction typeCall = (Operator::TypeCallFunction)
                  (((point >= DAfterConcat && point <= DAfterConcatStack)
                     || (point >= DAfterConcatSymbolic && point <= DAfterConcatSymbolicStack))
                  ? Operator::TCFConcat : ((point >= DAfterMinSigned && point <= DAfterMaxUnsignedStack)
                  ? ((point - DAfterMinSigned)/2 + Operator::TCFMinSigned)
                  : ((point - DAfterMinSignedSymbolic)/2 + Operator::TCFMinSigned)));
               auto& operation = operatorStack->pushCallFunction(typeCall, 2);
               if (state.point() >= DAfterConcatSymbolic)
                  operation.setSymbolic();
               if (point >= DAfterMinSigned) {
                  if ((point - DAfterMinSigned) % 4 < 2)
                     operation.setUnsignedTag();
                  else
                     operation.setSignedTag();
               };
               DefineShift(InFunction, *subElement, &ConcreteElement::readToken,
                     (Parser::State::UnionResult<ConcreteElement, OperatorStack>*) nullptr);
            }
         };
         if (!arguments.addErrorMessage(STG::SString("binary function construction expects an '('")))
            return RRFinished;
         DefineReduceAndParse
      DefineParseCase(AfterReduce)
      DefineLightParseCase(AfterReduceStack)
      DefineParseCase(AfterReduceSymbolic)
      DefineLightParseCase(AfterReduceSymbolicStack)
         {  AbstractToken token = arguments.lexer().queryToken();
            if (token.getType() == AbstractToken::TOperatorPunctuator
                  && ((const OperatorPunctuatorToken&) token).getType()
                     == OperatorPunctuatorToken::TOpenParen) {
               if (state.point() == DAfterReduce || state.point() == DAfterReduceSymbolic)
                  state.absorbUnionResult(OperatorStack());
               OperatorStack* operatorStack = &state.getUnionResult((OperatorStack*) nullptr);
               ConcreteElement* subElement = new ConcreteElement;
               operatorStack->absorbSubElement(subElement);
               auto& operation = operatorStack->pushCallFunction(Operator::TCFReduce, 1);
               if (state.point() >= DAfterReduceSymbolic)
                  operation.setSymbolic();
               DefineShift(InReductionFunction, *subElement, &ConcreteElement::readToken,
                     (Parser::State::UnionResult<ConcreteElement, OperatorStack>*) nullptr);
            }
         };
         if (!arguments.addErrorMessage(STG::SString("reduce construction expects an '('")))
            return RRFinished;
         DefineReduceAndParse
      DefineParseCase(AfterBitSet)
      DefineLightParseCase(AfterBitSetStack)
      DefineParseCase(AfterBitSetSymbolic)
      DefineLightParseCase(AfterBitSetSymbolicStack)
         {  AbstractToken token = arguments.lexer().queryToken();
            if (token.getType() == AbstractToken::TOperatorPunctuator
                  && ((const OperatorPunctuatorToken&) token).getType()
                     == OperatorPunctuatorToken::TOpenParen) {
               if (state.point() == DAfterBitSet || state.point() == DAfterBitSetSymbolic)
                  state.absorbUnionResult(OperatorStack());
               OperatorStack* operatorStack = &state.getUnionResult((OperatorStack*) nullptr);
               ConcreteElement* subElement = new ConcreteElement;
               operatorStack->absorbSubElement(subElement);
               auto& operation = operatorStack->pushCallFunction(Operator::TCFBitSet, 2);
               if (state.point() >= DAfterBitSetSymbolic)
                  operation.setSymbolic();
               DefineShift(InBitSetFunction, *subElement, &ConcreteElement::readToken,
                     (Parser::State::UnionResult<ConcreteElement, OperatorStack>*) nullptr);
            }
         };
         if (!arguments.addErrorMessage(STG::SString("bitset construction expects an '('")))
            return RRFinished;
         DefineReduceAndParse
      DefineParseCase(AfterShiftBit)
      DefineLightParseCase(AfterShiftBitStack)
      DefineParseCase(AfterShiftBitSymbolic)
      DefineLightParseCase(AfterShiftBitSymbolicStack)
         {  AbstractToken token = arguments.lexer().queryToken();
            if (token.getType() == AbstractToken::TOperatorPunctuator
                  && ((const OperatorPunctuatorToken&) token).getType()
                     == OperatorPunctuatorToken::TOpenParen) {
               if (state.point() == DAfterShiftBit || state.point() == DAfterShiftBitSymbolic)
                  state.absorbUnionResult(OperatorStack());
               OperatorStack* operatorStack = &state.getUnionResult((OperatorStack*) nullptr);
               ConcreteElement* subElement = new ConcreteElement;
               operatorStack->absorbSubElement(subElement);
               auto& operation = operatorStack->pushCallFunction(Operator::TCFShiftBit, 1);
               if (state.point() >= DAfterShiftBitSymbolic)
                  operation.setSymbolic();
               DefineShift(InShiftBitFunction, *subElement, &ConcreteElement::readToken,
                     (Parser::State::UnionResult<ConcreteElement, OperatorStack>*) nullptr);
            }
         };
         if (!arguments.addErrorMessage(STG::SString("extension construction expects an '('")))
            return RRFinished;
         DefineReduceAndParse
      DefineParseCase(AfterIncSigned)
      DefineLightParseCase(AfterIncSignedStack)
      DefineParseCase(AfterIncSignedSymbolic)
      DefineLightParseCase(AfterIncSignedSymbolicStack)
      DefineParseCase(AfterIncUnsigned)
      DefineLightParseCase(AfterIncUnsignedStack)
      DefineParseCase(AfterIncUnsignedSymbolic)
      DefineLightParseCase(AfterIncUnsignedSymbolicStack)
      DefineParseCase(AfterDecSigned)
      DefineLightParseCase(AfterDecSignedStack)
      DefineParseCase(AfterDecSignedSymbolic)
      DefineLightParseCase(AfterDecSignedSymbolicStack)
      DefineParseCase(AfterDecUnsigned)
      DefineLightParseCase(AfterDecUnsignedStack)
      DefineParseCase(AfterDecUnsignedSymbolic)
      DefineLightParseCase(AfterDecUnsignedSymbolicStack)
         {  AbstractToken token = arguments.lexer().queryToken();
            if (token.getType() == AbstractToken::TOperatorPunctuator
                  && ((const OperatorPunctuatorToken&) token).getType()
                     == OperatorPunctuatorToken::TOpenParen) {
               if (((state.point() - DAfterIncSigned) & 0x1) == 0)
                  state.absorbUnionResult(OperatorStack());
               OperatorStack* operatorStack = &state.getUnionResult((OperatorStack*) nullptr);
               ConcreteElement* subElement = new ConcreteElement;
               operatorStack->absorbSubElement(subElement);
               int point = state.point();
               Operator::TypeCallFunction typeCall
                  = (point >= DAfterIncSignedSymbolic)
                     ? (Operator::TypeCallFunction) ((point - DAfterIncSignedSymbolic) / 4 + Operator::TCFInc)
                     : (Operator::TypeCallFunction) ((point - DAfterIncSigned) / 4 + Operator::TCFInc);
               auto& operation = operatorStack->pushCallFunction(typeCall, 1);
               if ((point - DAfterIncSigned) % 4 < 2)
                  operation.setUnsignedTag();
               else
                  operation.setSignedTag();
               if (point >= DAfterIncSignedSymbolic)
                  operation.setSymbolic();
               DefineShift(InFunction, *subElement, &ConcreteElement::readToken,
                     (Parser::State::UnionResult<ConcreteElement, OperatorStack>*) nullptr);
            }
         };
         if (!arguments.addErrorMessage(STG::SString("dec construction expects an '('")))
            return RRFinished;
         DefineReduceAndParse
      DefineParseCase(InFunction)
         {  OperatorStack* operatorStack = &state.getUnionResult((OperatorStack*) nullptr);
            PNT::AutoPointer<ConcreteElement> subElement(operatorStack
                  ->extractSubElement(), PNT::Pointer::Init());
            Operator* call = &operatorStack->getSLast();
            AssumeCondition(call->getType() == Operator::TCall)
            AbstractToken token = arguments.lexer().queryToken();
            if (subElement->dElement.isValid()) {
               operatorStack->pushLastArgument(
                  std::move(subElement->dElement), subElement->atType);
               if (call->getLeftSubExpressions() > 0) {
                  if (token.getType() == AbstractToken::TOperatorPunctuator
                        && ((const OperatorPunctuatorToken&) token).getType()
                           == OperatorPunctuatorToken::TComma) {
                     operatorStack->absorbSubElement(subElement.extractElement());
                     DefineShift(InFunction, operatorStack->getSubElement(),
                           &ConcreteElement::readToken, (Parser::State::UnionResult<ConcreteElement, OperatorStack>*) nullptr);
                  };
               }
               else {
                  if (token.getType() == AbstractToken::TOperatorPunctuator
                        && ((const OperatorPunctuatorToken&) token).getType()
                           == OperatorPunctuatorToken::TCloseParen) {
                     ArgumentType resultType = ATUndefined;
                     Domain resultElement = operatorStack
                        ->applyCallFunction(*call, resultType, arguments);
                     operatorStack->freeLast();
                     call = nullptr;
                     if (operatorStack->isEmpty()) {
                        state.freeUnionResult((OperatorStack*) nullptr);
                        operatorStack = nullptr;
                        dElement = std::move(resultElement);
                        atType = resultType;
                        DefineGotoCase(AfterPrimary)
                     }
                     else {
                        operatorStack->pushLastArgument(std::move(resultElement), resultType);
                        DefineGotoCase(AfterPrimaryStack)
                     };
                  };
               };
            };
            operatorStack->freeLast();
            if (token.getType() == AbstractToken::TOperatorPunctuator
                  && ((const OperatorPunctuatorToken&) token).getType()
                     == OperatorPunctuatorToken::TCloseParen)
               DefineReduce
            DefineAddError(STG::SString("')' was expected"
               " during the parsing of a call"));
            DefineGotoCase(EndParen)
         };
         DefineReduce
      DefineParseCase(InExtensionFunction)
      DefineParseCase(InShiftBitFunction)
         {  OperatorStack* operatorStack = &state.getUnionResult((OperatorStack*) nullptr);
            PNT::AutoPointer<ConcreteElement> subElement(operatorStack
                  ->extractSubElement(), PNT::Pointer::Init());
            Operator* call = &operatorStack->getSLast();
            AssumeCondition(call->getType() == Operator::TCall)
            AbstractToken token = arguments.lexer().queryToken();
            if (subElement->dElement.isValid()) {
               operatorStack->pushLastArgument(
                  std::move(subElement->dElement), subElement->atType);
               if (token.getType() == AbstractToken::TOperatorPunctuator
                     && ((const OperatorPunctuatorToken&) token).getType()
                        == OperatorPunctuatorToken::TComma) {
                  if (state.point() == DInExtensionFunction)
                     DefineGotoCase(InExtensionFunctionSize)
                  else
                     DefineGotoCase(InShiftBitFunctionBitSelection)
               };
            };
            operatorStack->freeLast();
            DefineAddError(STG::SString("',' was expected"
               " during the parsing of a shift_bit"));
            DefineGotoCaseAndParse(EndParen)
         };
         DefineReduce
      DefineParseCase(InExtensionFunctionSize)
         {  OperatorStack* operatorStack = &state.getUnionResult((OperatorStack*) nullptr);
            Operator* call = &operatorStack->getSLast();
            AssumeCondition(call->getType() == Operator::TCall)
            AbstractToken token = arguments.lexer().queryToken();
            if (token.getType() == AbstractToken::TNumber) {
               AssumeCondition(dynamic_cast<const NumberToken*>(&arguments.lexer().getContentToken()))
               const auto& token = (const NumberToken&) arguments.lexer().getContentToken();
               if (token.hasSpecialCoding() || token.getSize() != 0) {
                  operatorStack->freeLast();
                  DefineAddError(STG::SString("bit number should be standard integer in shift_bit"));
                  DefineGotoCase(EndParen)
               }
               call->setExtension(token.getContent().queryInteger());
               DefineGotoCase(EndCallFunction);
            };
            operatorStack->freeLast();
            DefineAddError(STG::SString("bit selection was expected during the parsing of a shift_bit"));
            DefineGotoCaseAndParse(EndParen)
         };
         DefineReduce
      DefineParseCase(InShiftBitFunctionBitSelection)
         {  OperatorStack* operatorStack = &state.getUnionResult((OperatorStack*) nullptr);
            Operator* call = &operatorStack->getSLast();
            AssumeCondition(call->getType() == Operator::TCall
                  && call->getTypeCall() == Operator::TCFShiftBit)
            AbstractToken token = arguments.lexer().queryToken();
            if (token.getType() == AbstractToken::TNumber) {
               AssumeCondition(dynamic_cast<const NumberToken*>(&arguments.lexer().getContentToken()))
               const auto& token = (const NumberToken&) arguments.lexer().getContentToken();
               if (token.hasSpecialCoding() || token.getSize() != 0) {
                  operatorStack->freeLast();
                  DefineAddError(STG::SString("bit number should be standard integer in shift_bit"));
                  DefineGotoCase(EndParen)
               }
               call->setLowBit(token.getContent().queryInteger());
               DefineGotoCase(EndCallFunction);
            };
            operatorStack->freeLast();
            DefineAddError(STG::SString("bit selection was expected during the parsing of a shift_bit"));
            DefineGotoCaseAndParse(EndParen)
         };
         DefineReduce
      DefineParseCase(InBitSetFunction)
         {  OperatorStack* operatorStack = &state.getUnionResult((OperatorStack*) nullptr);
            PNT::AutoPointer<ConcreteElement> subElement(operatorStack
                  ->extractSubElement(), PNT::Pointer::Init());
            Operator* call = &operatorStack->getSLast();
            AssumeCondition(call->getType() == Operator::TCall
                  && call->getTypeCall() == Operator::TCFBitSet)
            AbstractToken token = arguments.lexer().queryToken();
            if (subElement->dElement.isValid()) {
               operatorStack->pushLastArgument(
                  std::move(subElement->dElement), subElement->atType);
               if (call->getLeftSubExpressions() > 0) {
                  if (token.getType() == AbstractToken::TOperatorPunctuator
                        && ((const OperatorPunctuatorToken&) token).getType()
                           == OperatorPunctuatorToken::TComma) {
                     operatorStack->absorbSubElement(subElement.extractElement());
                     DefineShift(InBitSetFunction, operatorStack->getSubElement(),
                           &ConcreteElement::readToken, (Parser::State::UnionResult<ConcreteElement, OperatorStack>*) nullptr);
                  };
               }
               else {
                  if (token.getType() == AbstractToken::TOperatorPunctuator
                        && ((const OperatorPunctuatorToken&) token).getType()
                           == OperatorPunctuatorToken::TComma)
                     DefineGotoCase(InBitSetFunctionLowBit)
               };
            };
            operatorStack->freeLast();
            if (token.getType() == AbstractToken::TOperatorPunctuator
                  && ((const OperatorPunctuatorToken&) token).getType()
                     == OperatorPunctuatorToken::TCloseParen)
               DefineReduce
            DefineAddError(STG::SString("')' was expected"
               " during the parsing of a call"));
            DefineGotoCase(EndParen)
         };
         DefineReduce
      DefineParseCase(InBitSetFunctionLowBit)
      DefineParseCase(InBitSetFunctionHighBit)
      DefineParseCase(InReductionFunctionLowBit)
      DefineLightParseCase(InReductionFunctionHighBit)
         {  OperatorStack* operatorStack = &state.getUnionResult((OperatorStack*) nullptr);
            Operator* call = &operatorStack->getSLast();
            AssumeCondition(call->getType() == Operator::TCall)
            AbstractToken token = arguments.lexer().queryToken();
            if (token.getType() == AbstractToken::TNumber) {
               AssumeCondition(dynamic_cast<const NumberToken*>(&arguments.lexer().getContentToken()))
               const auto& token = (const NumberToken&) arguments.lexer().getContentToken();
               if (token.hasSpecialCoding() || token.getSize() != 0) {
                  operatorStack->freeLast();
                  DefineAddError(STG::SString("bit number should be standard integer in bitset/reduction"));
                  DefineGotoCase(EndParen)
               }
               int point = state.point();
               if (point == DInBitSetFunctionLowBit || point == DInReductionFunctionLowBit) {
                  call->setLowBit(token.getContent().queryInteger());
                  DefineGotoCaseWithIncrement(DInBitSetFunctionAfterLowBit-DInBitSetFunctionLowBit, LInBitSetFunctionAfterLowBit);
               }
               else {
                  call->setHighBit(token.getContent().queryInteger());
                  DefineGotoCase(EndCallFunction);
               };
            };
            operatorStack->freeLast();
            DefineAddError(STG::SString("bit selection was expected during the parsing of a bitset/reduction"));
            DefineGotoCaseAndParse(EndParen)
         };
         DefineReduce
      DefineParseCase(InBitSetFunctionAfterLowBit)
      DefineLightParseCase(InReductionFunctionAfterLowBit)
         {  AbstractToken token = arguments.lexer().queryToken();
            if (token.getType() == AbstractToken::TOperatorPunctuator
                  && ((const OperatorPunctuatorToken&) token).getType()
                     == OperatorPunctuatorToken::TComma)
               DefineGotoCaseWithIncrement(DInBitSetFunctionHighBit-DInBitSetFunctionAfterLowBit, LInBitSetFunctionHighBit)
            OperatorStack* operatorStack = &state.getUnionResult((OperatorStack*) nullptr);
            operatorStack->freeLast();
            DefineAddError(STG::SString("',' was expected"
               " during the parsing of a bitset/reduction"));
            DefineGotoCase(EndParen)
         };
         DefineReduce

      DefineParseCase(InReductionFunction)
         {  OperatorStack* operatorStack = &state.getUnionResult((OperatorStack*) nullptr);
            PNT::AutoPointer<ConcreteElement> subElement(operatorStack
                  ->extractSubElement(), PNT::Pointer::Init());
            Operator* call = &operatorStack->getSLast();
            AssumeCondition(call->getType() == Operator::TCall
                  && call->getTypeCall() == Operator::TCFReduce)
            AbstractToken token = arguments.lexer().queryToken();
            if (subElement->dElement.isValid()) {
               operatorStack->pushLastArgument(
                  std::move(subElement->dElement), subElement->atType);
               if (token.getType() == AbstractToken::TOperatorPunctuator
                     && ((const OperatorPunctuatorToken&) token).getType()
                        == OperatorPunctuatorToken::TComma)
                  DefineGotoCase(InReductionFunctionLowBit)
            };
            operatorStack->freeLast();
            DefineAddError(STG::SString("',' was expected"
               " during the parsing of a shift_bit"));
            DefineGotoCaseAndParse(EndParen)
         };
         DefineReduce
      DefineParseCase(EndCallFunction)
         {  OperatorStack* operatorStack = &state.getUnionResult((OperatorStack*) nullptr);
            Operator* call = &operatorStack->getSLast();
            AssumeCondition(call->getType() == Operator::TCall)
            AbstractToken token = arguments.lexer().queryToken();
            if (token.getType() == AbstractToken::TOperatorPunctuator
                  && ((const OperatorPunctuatorToken&) token).getType()
                     == OperatorPunctuatorToken::TCloseParen) {
               ArgumentType resultType = ATUndefined;
               Domain resultElement = operatorStack
                  ->applyCallFunction(*call, resultType, arguments);
               operatorStack->freeLast();
               call = nullptr;
               if (operatorStack->isEmpty()) {
                  state.freeUnionResult((OperatorStack*) nullptr);
                  operatorStack = nullptr;
                  dElement = std::move(resultElement);
                  atType = resultType;
                  DefineGotoCase(AfterPrimary)
               }
               else {
                  operatorStack->pushLastArgument(std::move(resultElement), resultType);
                  DefineGotoCase(AfterPrimaryStack)
               };
            };
            operatorStack->freeLast();
            if (token.getType() == AbstractToken::TOperatorPunctuator
                  && ((const OperatorPunctuatorToken&) token).getType()
                     == OperatorPunctuatorToken::TCloseParen)
               DefineReduce
            DefineAddError(STG::SString("')' was expected"
               " during the parsing of a call"));
            DefineGotoCase(EndParen)
         };
         DefineReduce
      default:
         break;
   };
   AssumeUncalled
   return result;
}

bool
ConstraintElement::apply(Parser::Arguments& arguments) {
   if (!ceResult.isValid() && neSecond.hasName()) {
      bool isFinished = false;;
      for (auto& operation : apvcfOperation) {
         if (operation.isValid()) {
            if (operation->isBinaryMinus()) {
               operation.absorbElement(operation->createUnaryOpposite());
               ceResult = neSecond.getValue();
               neSecond.clear();
            }
            else if (operation->isBinaryPlus()) {
               ceResult = neSecond.getValue();
               neSecond.clear();
               arguments.clearEvaluationEnvironment();
               domain_intersect(&neFirst.getSValue().selement().svalue(),
                     ceResult.getElement().value(), &arguments.evaluationEnvironment());
               if (neFirst.getSValue().selement().isValid())
                  domain_specialize(&neFirst.getSValue().selement().svalue());
               isFinished = true;
            };
         }
      }
      if (isFinished)
         return true;
   }
   if (!ceResult.isValid())
      return false;
   if (!neFirst.hasName())
      return false;
   GuardLatticeCreation guard(arguments.constraintEnvironment());
   if (fIsSymbolic)
      arguments.constraintEnvironment().defaultDomainType = DISFormal;
   int index = neFirst.getValue().getArgumentType()-1;
   if (neSecond.hasName()) {
      if (index < 0 || index >= 3
            || neSecond.getValue().getArgumentType() != neFirst.getValue().getArgumentType()) {
         arguments.addErrorMessage(STG::SString("arguments have different numerical types"));
         return false;
      };
      auto& aoperation = apvcfOperation[index];
      if (!aoperation.isValid() || aoperation->getType() != neFirst.getValue().getArgumentType()
            || aoperation->getArgumentsNumber() != 2) {
         if (!aoperation.isValid() || aoperation->getType() != neFirst.getValue().getArgumentType())
            arguments.addErrorMessage(STG::SString("arguments have numerical types not supported by the aoperation"));
         else
            arguments.addErrorMessage(STG::SString("2 arguments provided which is not supported by the aoperation"));
         return false;
      };
      arguments.clearConstraintEnvironment();
      domain_specialize(&ceResult.selement().svalue());
      AssumeCondition(dynamic_cast<const BinaryConstraintFunction*>(aoperation.key()))
      const auto& operation = static_cast<const BinaryConstraintFunction&>(*aoperation);
      operation(ceResult.getElement(),
            neFirst.getSValue().selement(), neSecond.getSValue().selement(),
            arguments.constraintEnvironment());
      if (neFirst.getSValue().selement().isValid())
         domain_specialize(&neFirst.getSValue().selement().svalue());
      if (neSecond.getSValue().selement().isValid())
         domain_specialize(&neSecond.getSValue().selement().svalue());
   }
   else {
      if (index < 0 || index >= 3) {
         arguments.addErrorMessage(STG::SString("argument has wrong numerical type"));
         return false;
      };
      auto& aoperation = apvcfOperation[index];
      if (!aoperation.isValid() || aoperation->getType() != neFirst.getValue().getArgumentType()
            || aoperation->getArgumentsNumber() != 1) {
         if (!aoperation.isValid() || aoperation->getType() != neFirst.getValue().getArgumentType())
            arguments.addErrorMessage(STG::SString("argument has numerical types not supported by the aoperation"));
         else
            arguments.addErrorMessage(STG::SString("1 argument provided which is not supported by the aoperation"));
         return false;
      };
      arguments.clearConstraintEnvironment();
      domain_specialize(&ceResult.selement().svalue());
      AssumeCondition(dynamic_cast<const UnaryConstraintFunction*>(aoperation.key()))
      const auto& operation = static_cast<const UnaryConstraintFunction&>(*aoperation);
      operation(ceResult.getElement(),
            neFirst.getSValue().selement(), arguments.constraintEnvironment());
      if (neFirst.getSValue().selement().isValid())
         domain_specialize(&neFirst.getSValue().selement().svalue());
   };
   return true;
}

STG::Lexer::Base::ReadResult
MapOfElements::readToken(Parser::State& state, Parser::Arguments& arguments) {
   enum Delimiters
      {  DBegin, DAfterVar, DAfterVarIdent, DAfterIdent, DAfterAssert, DOperationAssert,
         DOperationExtensionAssert, DBeforeOperationSizeAssert, DOperationSizeAssert,
         DBeforeOperationLowBitAssert, DOperationLowBitAssert, DOperationNextLowBitAssert,
         DOperationHighBitAssert, DTypeAssert, DAfterTypeAssert, DAfterOperationAssert,
         DAssertFirstArgument, DInAssert, DAssertArgument, DEndNamedDeclaration, DEndDeclaration,
         DDoubleEndParen, DEndParen, DBeforeSemiColon
      };
   ReadResult result = RRNeedChars;
   switch (state.point()) {
      DefineParseCase(Begin)
         {  AbstractToken token = arguments.lexer().queryToken();
            auto basicType = token.getType();
            if (basicType == AbstractToken::TIdentifier) {
               const IdentifierToken& token = (const IdentifierToken&)
                  arguments.lexer().getContentToken();
               state.absorbUnionResult(NamedElement(token.identifier()));
               DefineGotoCase(AfterIdent)
            }
            else if (basicType == AbstractToken::TKeyword) {
               auto type = ((const KeywordToken&) token).getType();
               if (type == KeywordToken::TVar)
                  DefineGotoCase(AfterVar)
               if (type == KeywordToken::TAssert)
                  DefineGotoCase(AfterAssert)
            }
            else if (basicType == AbstractToken::TOperatorPunctuator) {
               auto type = ((const OperatorPunctuatorToken&) token).getType();
               if (type == OperatorPunctuatorToken::TSemiColon)
                  DefineGotoCase(Begin)
            };
         }
         DefineAddError(STG::SString("unknown token for the beginning of an instruction"))
         DefineGotoCaseAndParse(BeforeSemiColon)
      DefineParseCase(AfterVar)
         if (arguments.lexer().queryToken().getType() == AbstractToken::TIdentifier) {
            const IdentifierToken& token = (const IdentifierToken&)
               arguments.lexer().getContentToken();
            state.absorbUnionResult(NamedElement(token.identifier()));
            DefineGotoCase(AfterVarIdent)
         }
         DefineAddError(STG::SString("expected identifier after var declaration"))
         DefineGotoCaseAndParse(BeforeSemiColon)
      DefineParseCase(AfterVarIdent)
         {  AbstractToken token = arguments.lexer().queryToken();
            if (token.getType() == AbstractToken::TOperatorPunctuator) {
               auto type = ((const OperatorPunctuatorToken&) token).getType();
               if (type == OperatorPunctuatorToken::TSemiColon) {
                  Cursor locateCursor(*this);
                  PNT::AutoPointer<NamedElement> namedElement(
                        new NamedElement(std::move(state.getUnionResult((NamedElement*) nullptr))), PNT::Pointer::Init());
                  state.freeUnionResult((NamedElement*) nullptr);
                  auto locationResult = locate(*namedElement, locateCursor, COL::VirtualCollection::RPUndefined);
                  if (locationResult)
                     DefineAddError(STG::SString("identifier ").cat(namedElement->getName())
                        .cat("has soon been declared"))
                  else
                     add(namedElement.extractElement(),
                        locationResult.queryInsertionParameters().setFreeOnError(),
                        &locateCursor);
                  DefineGotoCase(Begin)
               }
               if (type == OperatorPunctuatorToken::TAssign) {
                  Cursor locateCursor(*this);
                  PNT::AutoPointer<NamedElement> namedElement(
                        new NamedElement(std::move(state.getUnionResult((NamedElement*) nullptr))), PNT::Pointer::Init());
                  state.freeUnionResult((NamedElement*) nullptr);
                  auto locationResult = locate(*namedElement, locateCursor, COL::VirtualCollection::RPUndefined);
                  if (locationResult) {
                     DefineAddError(STG::SString("identifier ").cat(namedElement->getName())
                        .cat("has soon been declared"))
                     DefineGotoCase(BeforeSemiColon)
                  }
                  ConcreteElement* element = &namedElement->getSValue();
                  add(namedElement.extractElement(),
                        locationResult.queryInsertionParameters().setFreeOnError(),
                        &locateCursor);
                  if (arguments.hasNotification())
                     state.absorbUnionResult(Cursor(locateCursor));
                  DefineShift(EndNamedDeclaration, *element, &ConcreteElement::readToken,
                        (Parser::State::UnionResult<ConcreteElement, ConcreteElement::OperatorStack>*) nullptr);
               }
            };
         }
         DefineAddError(STG::SString("unknown token after var \"")
            .cat(state.getUnionResult((NamedElement*) nullptr).getName()).cat('"'))
         DefineGotoCaseAndParse(BeforeSemiColon)
      DefineParseCase(AfterIdent)
         {  AbstractToken token = arguments.lexer().queryToken();
            if (token.getType() == AbstractToken::TOperatorPunctuator) {
               auto type = ((const OperatorPunctuatorToken&) token).getType();
               if (type == OperatorPunctuatorToken::TAssign) {
                  Cursor locateCursor(*this);
                  NamedElement namedElement(std::move(state.getUnionResult((NamedElement*) nullptr)));
                  state.freeUnionResult((NamedElement*) nullptr);
                  if (locate(namedElement, locateCursor, COL::VirtualCollection::RPExact)) {
                     ConcreteElement* element = &locateCursor.elementSAt().getSValue();
                     element->clear();
                     if (arguments.hasNotification())
                        state.absorbUnionResult(std::move(locateCursor));
                     DefineShift(EndNamedDeclaration, *element, &ConcreteElement::readToken,
                           (Parser::State::UnionResult<ConcreteElement, ConcreteElement::OperatorStack>*) nullptr);
                  }
                  else
                     DefineAddError(STG::SString("identifier ").cat(namedElement.getName())
                        .cat(" has not been declared"))
                  DefineGotoCase(EndDeclaration)
               }
            };
         }
         DefineAddError(STG::SString("expected '=' after identifier\"")
            .cat(state.getUnionResult((NamedElement*) nullptr).getName()).cat('"'))
         DefineGotoCaseAndParse(BeforeSemiColon)
      DefineParseCase(AfterAssert)
         {  AbstractToken token = arguments.lexer().queryToken();
            if (token.getType() == AbstractToken::TOperatorPunctuator
                  && ((const OperatorPunctuatorToken&) token).getType()
                     == OperatorPunctuatorToken::TOpenParen) {
               state.absorbUnionResult(ConstraintElement());
               DefineGotoCase(OperationAssert)
            };
         };
         DefineAddError(STG::SString("expected '(' after the assert keyword"))
         DefineGotoCase(BeforeSemiColon)
      DefineParseCase(OperationAssert)
         {  using BinaryOperationConstraintFunction = ConstraintElement::BinaryOperationConstraintFunction;
            using BinaryCompareConstraintFunction = ConstraintElement::BinaryCompareConstraintFunction;
            using UnaryOperationConstraintFunction = ConstraintElement::UnaryOperationConstraintFunction;
            AbstractToken token = arguments.lexer().queryToken();
            if (token.getType() == AbstractToken::TOperatorPunctuator) {
               switch (((const OperatorPunctuatorToken&) token).getType()) {
                  case OperatorPunctuatorToken::TOpenParen:
                     DefineGotoCase(TypeAssert)
                  case OperatorPunctuatorToken::TPlus:
                     {  auto& constraint = state.getUnionResult((ConstraintElement*) nullptr);
                        BinaryOperationConstraintFunction* binaryOperation;
                        constraint.apvcfOperation[ATBit-1].absorbElement(binaryOperation = new BinaryOperationConstraintFunction());
                        binaryOperation->setBit(DBBOPlus);
                        constraint.apvcfOperation[ATMultiFloat-1].absorbElement(binaryOperation = new BinaryOperationConstraintFunction());
                        binaryOperation->setMultiFloat(DMFBOPlus);
                     }
                     DefineGotoCase(OperationExtensionAssert)
                  case OperatorPunctuatorToken::TMinus:
                     {  auto& constraint = state.getUnionResult((ConstraintElement*) nullptr);
                        auto* unaryOperation = new UnaryOperationConstraintFunction();
                        constraint.apvcfOperation[ATMultiBit-1].absorbElement(unaryOperation);
                        unaryOperation->setMultiBit(DMBUOOppositeSigned);
                        BinaryOperationConstraintFunction* binaryOperation;
                        constraint.apvcfOperation[ATBit-1].absorbElement(binaryOperation = new BinaryOperationConstraintFunction());
                        binaryOperation->setBit(DBBOMinus);
                        constraint.apvcfOperation[ATMultiFloat-1].absorbElement(binaryOperation = new BinaryOperationConstraintFunction());
                        binaryOperation->setMultiFloat(DMFBOMinus);
                     }
                     DefineGotoCase(OperationExtensionAssert)
                  case OperatorPunctuatorToken::TLess:
                     {  auto& constraint = state.getUnionResult((ConstraintElement*) nullptr);
                        BinaryCompareConstraintFunction* compareOperation;
                        constraint.apvcfOperation[ATBit-1].absorbElement(compareOperation = new BinaryCompareConstraintFunction());
                        compareOperation->setBit(DBCOCompareLess);
                        constraint.apvcfOperation[ATMultiFloat-1].absorbElement(compareOperation = new BinaryCompareConstraintFunction());
                        compareOperation->setMultiFloat(DMFCOCompareLess);
                     }
                     DefineGotoCase(OperationExtensionAssert)
                  case OperatorPunctuatorToken::TLessOrEqual:
                     {  auto& constraint = state.getUnionResult((ConstraintElement*) nullptr);
                        BinaryCompareConstraintFunction* compareOperation;
                        constraint.apvcfOperation[ATBit-1].absorbElement(compareOperation = new BinaryCompareConstraintFunction());
                        compareOperation->setBit(DBCOCompareLessOrEqual);
                        constraint.apvcfOperation[ATMultiFloat-1].absorbElement(compareOperation = new BinaryCompareConstraintFunction());
                        compareOperation->setMultiFloat(DMFCOCompareLessOrEqual);
                     }
                     DefineGotoCase(OperationExtensionAssert)
                  case OperatorPunctuatorToken::TEqual:
                     {  auto& constraint = state.getUnionResult((ConstraintElement*) nullptr);
                        BinaryCompareConstraintFunction* compareOperation;
                        constraint.apvcfOperation[ATBit-1].absorbElement(compareOperation = new BinaryCompareConstraintFunction());
                        compareOperation->setBit(DBCOCompareEqual);
                        constraint.apvcfOperation[ATMultiBit-1].absorbElement(compareOperation = new BinaryCompareConstraintFunction());
                        compareOperation->setMultiBit(DMBCOCompareEqual);
                        constraint.apvcfOperation[ATMultiFloat-1].absorbElement(compareOperation = new BinaryCompareConstraintFunction());
                        compareOperation->setMultiFloat(DMFCOCompareEqual);
                     }
                     DefineGotoCase(AfterOperationAssert)
                  case OperatorPunctuatorToken::TDifferent:
                     {  auto& constraint = state.getUnionResult((ConstraintElement*) nullptr);
                        BinaryCompareConstraintFunction* compareOperation;
                        constraint.apvcfOperation[ATBit-1].absorbElement(compareOperation = new BinaryCompareConstraintFunction());
                        compareOperation->setBit(DBCOCompareDifferent);
                        constraint.apvcfOperation[ATMultiBit-1].absorbElement(compareOperation = new BinaryCompareConstraintFunction());
                        compareOperation->setMultiBit(DMBCOCompareDifferent);
                        constraint.apvcfOperation[ATMultiFloat-1].absorbElement(compareOperation = new BinaryCompareConstraintFunction());
                        compareOperation->setMultiFloat(DMFCOCompareDifferent);
                     }
                     DefineGotoCase(AfterOperationAssert)
                  case OperatorPunctuatorToken::TGreaterOrEqual:
                     {  auto& constraint = state.getUnionResult((ConstraintElement*) nullptr);
                        BinaryCompareConstraintFunction* compareOperation;
                        constraint.apvcfOperation[ATBit-1].absorbElement(compareOperation = new BinaryCompareConstraintFunction());
                        compareOperation->setBit(DBCOCompareGreaterOrEqual);
                        constraint.apvcfOperation[ATMultiFloat-1].absorbElement(compareOperation = new BinaryCompareConstraintFunction());
                        compareOperation->setMultiFloat(DMFCOCompareGreaterOrEqual);
                     }
                     DefineGotoCase(OperationExtensionAssert)
                  case OperatorPunctuatorToken::TGreater:
                     {  auto& constraint = state.getUnionResult((ConstraintElement*) nullptr);
                        BinaryCompareConstraintFunction* compareOperation;
                        constraint.apvcfOperation[ATBit-1].absorbElement(compareOperation = new BinaryCompareConstraintFunction());
                        compareOperation->setBit(DBCOCompareGreater);
                        constraint.apvcfOperation[ATMultiFloat-1].absorbElement(compareOperation = new BinaryCompareConstraintFunction());
                        compareOperation->setMultiFloat(DMFCOCompareGreater);
                     }
                     DefineGotoCase(OperationExtensionAssert)
                  case OperatorPunctuatorToken::TTimes:
                     {  auto& constraint = state.getUnionResult((ConstraintElement*) nullptr);
                        BinaryOperationConstraintFunction* binaryOperation = new BinaryOperationConstraintFunction();
                        constraint.apvcfOperation[ATMultiFloat-1].absorbElement(binaryOperation);
                        binaryOperation->setMultiFloat(DMFBOTimes);
                     }
                     DefineGotoCase(OperationExtensionAssert)
                  case OperatorPunctuatorToken::TDivide:
                     {  auto& constraint = state.getUnionResult((ConstraintElement*) nullptr);
                        BinaryOperationConstraintFunction* binaryOperation = new BinaryOperationConstraintFunction();
                        constraint.apvcfOperation[ATMultiFloat-1].absorbElement(binaryOperation);
                        binaryOperation->setMultiFloat(DMFBODivide);
                     }
                     DefineGotoCase(OperationExtensionAssert)
                  case OperatorPunctuatorToken::TModulo:
                     {  auto& constraint = state.getUnionResult((ConstraintElement*) nullptr);
                        BinaryOperationConstraintFunction* binaryOperation = new BinaryOperationConstraintFunction();
                        constraint.apvcfOperation[ATMultiBit-1].absorbElement(binaryOperation);
                        binaryOperation->setMultiBit(DMBBOModuloUnsigned);
                     }
                     DefineGotoCase(OperationExtensionAssert)
                  case OperatorPunctuatorToken::TBitOr:
                     {  auto& constraint = state.getUnionResult((ConstraintElement*) nullptr);
                        BinaryOperationConstraintFunction* binaryOperation;
                        constraint.apvcfOperation[ATBit-1].absorbElement(binaryOperation = new BinaryOperationConstraintFunction());
                        binaryOperation->setBit(DBBOOr);
                        constraint.apvcfOperation[ATMultiBit-1].absorbElement(binaryOperation = new BinaryOperationConstraintFunction());
                        binaryOperation->setMultiBit(DMBBOBitOr);
                     }
                     DefineGotoCase(AfterOperationAssert)
                  case OperatorPunctuatorToken::TBitAnd:
                     {  auto& constraint = state.getUnionResult((ConstraintElement*) nullptr);
                        BinaryOperationConstraintFunction* binaryOperation;
                        constraint.apvcfOperation[ATBit-1].absorbElement(binaryOperation = new BinaryOperationConstraintFunction());
                        binaryOperation->setBit(DBBOAnd);
                        constraint.apvcfOperation[ATMultiBit-1].absorbElement(binaryOperation = new BinaryOperationConstraintFunction());
                        binaryOperation->setMultiBit(DMBBOBitAnd);
                     }
                     DefineGotoCase(AfterOperationAssert)
                  case OperatorPunctuatorToken::TBitXor:
                     {  auto& constraint = state.getUnionResult((ConstraintElement*) nullptr);
                        BinaryOperationConstraintFunction* binaryOperation;
                        constraint.apvcfOperation[ATBit-1].absorbElement(binaryOperation = new BinaryOperationConstraintFunction());
                        binaryOperation->setBit(DBBOExclusiveOr);
                        constraint.apvcfOperation[ATMultiBit-1].absorbElement(binaryOperation = new BinaryOperationConstraintFunction());
                        binaryOperation->setMultiBit(DMBBOBitExclusiveOr);
                     }
                     DefineGotoCase(AfterOperationAssert)
                  case OperatorPunctuatorToken::TBitNegate:
                     {  auto& constraint = state.getUnionResult((ConstraintElement*) nullptr);
                        UnaryOperationConstraintFunction* unaryOperation;
                        constraint.apvcfOperation[ATBit-1].absorbElement(unaryOperation = new UnaryOperationConstraintFunction());
                        unaryOperation->setBit(DBUONegate);
                        constraint.apvcfOperation[ATMultiBit-1].absorbElement(unaryOperation = new UnaryOperationConstraintFunction());
                        unaryOperation->setMultiBit(DMBUOBitNegate);
                     }
                     DefineGotoCase(AfterOperationAssert)
                  case OperatorPunctuatorToken::TLeftShift:
                     {  auto& constraint = state.getUnionResult((ConstraintElement*) nullptr);
                        BinaryOperationConstraintFunction* binaryOperation = new BinaryOperationConstraintFunction();
                        constraint.apvcfOperation[ATMultiBit-1].absorbElement(binaryOperation);
                        binaryOperation->setMultiBit(DMBBOLeftShift);
                     }
                     DefineGotoCase(AfterOperationAssert)
                  case OperatorPunctuatorToken::TRightShift:
                     {  auto& constraint = state.getUnionResult((ConstraintElement*) nullptr);
                        BinaryOperationConstraintFunction* binaryOperation = new BinaryOperationConstraintFunction();
                        constraint.apvcfOperation[ATMultiBit-1].absorbElement(binaryOperation);
                        binaryOperation->setMultiBit(DMBBOLogicalRightShift);
                     }
                     DefineGotoCase(OperationExtensionAssert)
                  case OperatorPunctuatorToken::TLogicalAnd:
                     {  auto& constraint = state.getUnionResult((ConstraintElement*) nullptr);
                        BinaryOperationConstraintFunction* binaryOperation = new BinaryOperationConstraintFunction();
                        constraint.apvcfOperation[ATBit-1].absorbElement(binaryOperation);
                        binaryOperation->setBit(DBBOAnd);
                     }
                     DefineGotoCase(AfterOperationAssert)
                  case OperatorPunctuatorToken::TLogicalOr:
                     {  auto& constraint = state.getUnionResult((ConstraintElement*) nullptr);
                        BinaryOperationConstraintFunction* binaryOperation = new BinaryOperationConstraintFunction();
                        constraint.apvcfOperation[ATBit-1].absorbElement(binaryOperation);
                        binaryOperation->setBit(DBBOOr);
                     }
                     DefineGotoCase(AfterOperationAssert)
                  case OperatorPunctuatorToken::TLogicalNegate:
                     {  auto& constraint = state.getUnionResult((ConstraintElement*) nullptr);
                        UnaryOperationConstraintFunction* unaryOperation = new UnaryOperationConstraintFunction();
                        constraint.apvcfOperation[ATBit-1].absorbElement(unaryOperation);
                        unaryOperation->setBit(DBUONegate);
                     }
                     DefineGotoCase(AfterOperationAssert)
                  default:
                     state.freeUnionResult((ConstraintElement*) nullptr);
                     break;
               };
            }
            else if (token.getType() == AbstractToken::TKeyword) {
               switch (((const KeywordToken&) token).getType()) {
                  case KeywordToken::TExtendZero: case KeywordToken::TExtendZeroSymbolic:
                     {  auto& constraint = state.getUnionResult((ConstraintElement*) nullptr);
                        auto* operation = new ConstraintElement::ExtendConstraintFunction();
                        constraint.apvcfOperation[ATMultiBit-1].absorbElement(operation);
                        operation->setType(DMBEOExtendWithZero);
                        constraint.fIsSymbolic = ((const KeywordToken&) token).getType() == KeywordToken::TExtendZeroSymbolic;
                     }
                     DefineGotoCase(BeforeOperationSizeAssert)
                  case KeywordToken::TExtendSign: case KeywordToken::TExtendSignSymbolic:
                     {  auto& constraint = state.getUnionResult((ConstraintElement*) nullptr);
                        auto* operation = new ConstraintElement::ExtendConstraintFunction();
                        constraint.apvcfOperation[ATMultiBit-1].absorbElement(operation);
                        operation->setType(DMBEOExtendWithSign);
                        constraint.fIsSymbolic = ((const KeywordToken&) token).getType() == KeywordToken::TExtendSignSymbolic;
                     }
                     DefineGotoCase(BeforeOperationSizeAssert)
                  case KeywordToken::TConcat: case KeywordToken::TConcatSymbolic:
                     {  auto& constraint = state.getUnionResult((ConstraintElement*) nullptr);
                        auto* operation = new ConstraintElement::BinaryOperationConstraintFunction();
                        constraint.apvcfOperation[ATMultiBit-1].absorbElement(operation);
                        operation->setMultiBit(DMBBOConcat);
                        constraint.fIsSymbolic = ((const KeywordToken&) token).getType() == KeywordToken::TConcatSymbolic;
                     }
                     DefineGotoCase(AfterOperationAssert)
                  case KeywordToken::TReduce: case KeywordToken::TReduceSymbolic:
                     {  auto& constraint = state.getUnionResult((ConstraintElement*) nullptr);
                        auto* operation = new ConstraintElement::ReduceConstraintFunction();
                        constraint.apvcfOperation[ATMultiBit-1].absorbElement(operation);
                        constraint.fIsSymbolic = ((const KeywordToken&) token).getType() == KeywordToken::TReduceSymbolic;
                     }
                     DefineGotoCase(BeforeOperationLowBitAssert)
                  case KeywordToken::TBitSet: case KeywordToken::TBitSetSymbolic:
                     {  auto& constraint = state.getUnionResult((ConstraintElement*) nullptr);
                        auto* operation = new ConstraintElement::BitSetConstraintFunction();
                        constraint.apvcfOperation[ATMultiBit-1].absorbElement(operation);
                        constraint.fIsSymbolic = ((const KeywordToken&) token).getType() == KeywordToken::TBitSetSymbolic;
                     }
                     DefineGotoCase(BeforeOperationLowBitAssert)
                  case KeywordToken::TShiftBit: case KeywordToken::TShiftBitSymbolic:
                     {  auto& constraint = state.getUnionResult((ConstraintElement*) nullptr);
                        auto* operation = new ConstraintElement::CastShiftBitConstraintFunction();
                        constraint.apvcfOperation[ATMultiBit-1].absorbElement(operation);
                        constraint.fIsSymbolic = ((const KeywordToken&) token).getType() == KeywordToken::TShiftBitSymbolic;
                     }
                     DefineGotoCase(BeforeOperationLowBitAssert)
                  case KeywordToken::TIncSigned: case KeywordToken::TIncSignedSymbolic:
                     {  auto& constraint = state.getUnionResult((ConstraintElement*) nullptr);
                        auto* operation = new ConstraintElement::UnaryOperationConstraintFunction();
                        constraint.apvcfOperation[ATMultiBit-1].absorbElement(operation);
                        operation->setMultiBit(DMBUONextSigned);
                        constraint.fIsSymbolic = ((const KeywordToken&) token).getType() == KeywordToken::TIncSignedSymbolic;
                     }
                     DefineGotoCase(AfterOperationAssert)
                  case KeywordToken::TIncUnsigned: case KeywordToken::TIncUnsignedSymbolic:
                     {  auto& constraint = state.getUnionResult((ConstraintElement*) nullptr);
                        auto* operation = new ConstraintElement::UnaryOperationConstraintFunction();
                        constraint.apvcfOperation[ATMultiBit-1].absorbElement(operation);
                        operation->setMultiBit(DMBUONextUnsigned);
                        constraint.fIsSymbolic = ((const KeywordToken&) token).getType() == KeywordToken::TIncUnsignedSymbolic;
                     }
                     DefineGotoCase(AfterOperationAssert)
                  case KeywordToken::TDecSigned: case KeywordToken::TDecSignedSymbolic:
                     {  auto& constraint = state.getUnionResult((ConstraintElement*) nullptr);
                        auto* operation = new ConstraintElement::UnaryOperationConstraintFunction();
                        constraint.apvcfOperation[ATMultiBit-1].absorbElement(operation);
                        operation->setMultiBit(DMBUOPrevSigned);
                        constraint.fIsSymbolic = ((const KeywordToken&) token).getType() == KeywordToken::TDecSignedSymbolic;
                     }
                     DefineGotoCase(AfterOperationAssert)
                  case KeywordToken::TDecUnsigned: case KeywordToken::TDecUnsignedSymbolic:
                     {  auto& constraint = state.getUnionResult((ConstraintElement*) nullptr);
                        auto* operation = new ConstraintElement::UnaryOperationConstraintFunction();
                        constraint.apvcfOperation[ATMultiBit-1].absorbElement(operation);
                        operation->setMultiBit(DMBUOPrevUnsigned);
                        constraint.fIsSymbolic = ((const KeywordToken&) token).getType() == KeywordToken::TDecUnsignedSymbolic;
                     }
                     DefineGotoCase(AfterOperationAssert)
                  case KeywordToken::TMinSigned: case KeywordToken::TMinSignedSymbolic:
                     {  auto& constraint = state.getUnionResult((ConstraintElement*) nullptr);
                        auto* operation = new ConstraintElement::BinaryOperationConstraintFunction();
                        constraint.apvcfOperation[ATMultiBit-1].absorbElement(operation);
                        operation->setMultiBit(DMBBOMinSigned);
                        constraint.fIsSymbolic = ((const KeywordToken&) token).getType() == KeywordToken::TMinSignedSymbolic;
                     }
                     DefineGotoCase(AfterOperationAssert)
                  case KeywordToken::TMinUnsigned: case KeywordToken::TMinUnsignedSymbolic:
                     {  auto& constraint = state.getUnionResult((ConstraintElement*) nullptr);
                        auto* operation = new ConstraintElement::BinaryOperationConstraintFunction();
                        constraint.apvcfOperation[ATMultiBit-1].absorbElement(operation);
                        operation->setMultiBit(DMBBOMinUnsigned);
                        constraint.fIsSymbolic = ((const KeywordToken&) token).getType() == KeywordToken::TMinUnsignedSymbolic;
                     }
                     DefineGotoCase(AfterOperationAssert)
                  case KeywordToken::TMaxSigned: case KeywordToken::TMaxSignedSymbolic:
                     {  auto& constraint = state.getUnionResult((ConstraintElement*) nullptr);
                        auto* operation = new ConstraintElement::BinaryOperationConstraintFunction();
                        constraint.apvcfOperation[ATMultiBit-1].absorbElement(operation);
                        operation->setMultiBit(DMBBOMaxSigned);
                        constraint.fIsSymbolic = ((const KeywordToken&) token).getType() == KeywordToken::TMaxSignedSymbolic;
                     }
                     DefineGotoCase(AfterOperationAssert)
                  case KeywordToken::TMaxUnsigned: case KeywordToken::TMaxUnsignedSymbolic:
                     {  auto& constraint = state.getUnionResult((ConstraintElement*) nullptr);
                        auto* operation = new ConstraintElement::BinaryOperationConstraintFunction();
                        constraint.apvcfOperation[ATMultiBit-1].absorbElement(operation);
                        operation->setMultiBit(DMBBOMaxUnsigned);
                        constraint.fIsSymbolic = ((const KeywordToken&) token).getType() == KeywordToken::TMaxUnsignedSymbolic;
                     }
                     DefineGotoCase(AfterOperationAssert)
                  default:
                     state.freeUnionResult((ConstraintElement*) nullptr);
                     break;
               };
            }
         };
         DefineAddError(STG::SString("expected operator as first argument of assert"))
         state.freeUnionResult((ConstraintElement*) nullptr);
         DefineGotoCase(EndParen)
      DefineParseCase(OperationExtensionAssert)
         {  AbstractToken token = arguments.lexer().queryToken();
            if (token.getType() == AbstractToken::TKeyword) {
               auto type = ((const KeywordToken&) token).getType();
               if (type == KeywordToken::TUnsigned || type == KeywordToken::TSigned
                     || type == KeywordToken::TUnsignedWithSigned
                     || type == KeywordToken::TUnsignedSymbolic || type == KeywordToken::TSignedSymbolic
                     || type == KeywordToken::TUnsignedWithSignedSymbolic) {
                  auto& constraint = state.getUnionResult((ConstraintElement*) nullptr);
                  if ((type == KeywordToken::TUnsignedWithSigned || type == KeywordToken::TUnsignedWithSignedSymbolic)
                        && (!constraint.apvcfOperation[ATMultiFloat-1].isValid()
                              || !constraint.apvcfOperation[ATMultiFloat-1]->isBinaryPlus())) {
                     DefineAddError(STG::SString("the operator/function do not accept unsigned with signed qualification"))
                     state.freeUnionResult((ConstraintElement*) nullptr);
                     DefineGotoCase(EndParen)
                  }
                  ConstraintElement::VirtualConstraintFunction* areferenceOperation = nullptr;
                  if (constraint.apvcfOperation[ATMultiFloat-1].isValid()) {
                     areferenceOperation = &*constraint.apvcfOperation[ATMultiFloat-1];
                     if (areferenceOperation->getArgumentsNumber() == 2) {
                        using BinaryConstraintFunction = ConstraintElement::BinaryConstraintFunction;
                        using BinaryOperationConstraintFunction = ConstraintElement::BinaryOperationConstraintFunction;
                        AssumeCondition(dynamic_cast<const BinaryConstraintFunction*>(areferenceOperation))
                        if (((const BinaryConstraintFunction*) areferenceOperation)->isOperation()) {
                           AssumeCondition(dynamic_cast<const BinaryOperationConstraintFunction*>(areferenceOperation))
                           auto& referenceOperation = *(const BinaryOperationConstraintFunction*) areferenceOperation;
                           switch (referenceOperation.getMultiFloatOperation()) {
                              case DMFBOPlus:
                                 if (type == KeywordToken::TUnsigned || type == KeywordToken::TUnsignedSymbolic) {
                                    auto* operation = new ConstraintElement::BinaryOperationConstraintFunction();
                                    constraint.apvcfOperation[ATMultiBit-1].absorbElement(operation);
                                    operation->setMultiBit(DMBBOPlusUnsigned);
                                    if (type == KeywordToken::TUnsignedSymbolic)
                                       constraint.fIsSymbolic = true;
                                 }
                                 else if (type == KeywordToken::TSigned || type == KeywordToken::TSignedSymbolic) {
                                    auto* operation = new ConstraintElement::BinaryOperationConstraintFunction();
                                    constraint.apvcfOperation[ATMultiBit-1].absorbElement(operation);
                                    operation->setMultiBit(DMBBOPlusSigned);
                                    if (type == KeywordToken::TSignedSymbolic)
                                       constraint.fIsSymbolic = true;
                                 }
                                 else {
                                    auto* operation = new ConstraintElement::BinaryOperationConstraintFunction();
                                    constraint.apvcfOperation[ATMultiBit-1].absorbElement(operation);
                                    operation->setMultiBit(DMBBOPlusUnsignedWithSigned);
                                    if (type == KeywordToken::TUnsignedWithSignedSymbolic)
                                       constraint.fIsSymbolic = true;
                                 }
                                 DefineGotoCase(AfterOperationAssert)
                              case DMFBOMinus:
                                 if (type == KeywordToken::TUnsigned || type == KeywordToken::TUnsignedSymbolic) {
                                    auto* operation = new ConstraintElement::BinaryOperationConstraintFunction();
                                    constraint.apvcfOperation[ATMultiBit-1].absorbElement(operation);
                                    operation->setMultiBit(DMBBOMinusUnsigned);
                                    if (type == KeywordToken::TUnsignedSymbolic)
                                       constraint.fIsSymbolic = true;
                                 }
                                 else {
                                    auto* operation = new ConstraintElement::BinaryOperationConstraintFunction();
                                    constraint.apvcfOperation[ATMultiBit-1].absorbElement(operation);
                                    operation->setMultiBit(DMBBOMinusSigned);
                                    if (type == KeywordToken::TSignedSymbolic)
                                       constraint.fIsSymbolic = true;
                                 }
                                 DefineGotoCase(AfterOperationAssert)
                              case DMFBOTimes:
                                 if (type == KeywordToken::TUnsigned  || type == KeywordToken::TUnsignedSymbolic) {
                                    auto* operation = new ConstraintElement::BinaryOperationConstraintFunction();
                                    constraint.apvcfOperation[ATMultiBit-1].absorbElement(operation);
                                    operation->setMultiBit(DMBBOTimesUnsigned);
                                    if (type == KeywordToken::TUnsignedSymbolic)
                                       constraint.fIsSymbolic = true;
                                 }
                                 else {
                                    auto* operation = new ConstraintElement::BinaryOperationConstraintFunction();
                                    constraint.apvcfOperation[ATMultiBit-1].absorbElement(operation);
                                    operation->setMultiBit(DMBBOTimesSigned);
                                    if (type == KeywordToken::TSignedSymbolic)
                                       constraint.fIsSymbolic = true;
                                 }
                                 DefineGotoCase(AfterOperationAssert)
                              case DMFBODivide:
                                 if (type == KeywordToken::TUnsigned  || type == KeywordToken::TUnsignedSymbolic) {
                                    auto* operation = new ConstraintElement::BinaryOperationConstraintFunction();
                                    constraint.apvcfOperation[ATMultiBit-1].absorbElement(operation);
                                    operation->setMultiBit(DMBBODivideUnsigned);
                                    if (type == KeywordToken::TUnsignedSymbolic)
                                       constraint.fIsSymbolic = true;
                                 }
                                 else {
                                    auto* operation = new ConstraintElement::BinaryOperationConstraintFunction();
                                    constraint.apvcfOperation[ATMultiBit-1].absorbElement(operation);
                                    operation->setMultiBit(DMBBODivideSigned);
                                    if (type == KeywordToken::TSignedSymbolic)
                                       constraint.fIsSymbolic = true;
                                 }
                                 DefineGotoCase(AfterOperationAssert)
                              default:
                                 break;
                           };
                        }
                        else if (((const BinaryConstraintFunction*) areferenceOperation)->isCompare()) {
                           using BinaryCompareConstraintFunction = ConstraintElement::BinaryCompareConstraintFunction;
                           AssumeCondition(dynamic_cast<const BinaryCompareConstraintFunction*>(areferenceOperation))
                           auto& referenceOperation = *(const BinaryCompareConstraintFunction*) areferenceOperation;
                           switch (referenceOperation.getMultiFloatOperation()) {
                              case DMFCOCompareLess:
                                 if (type == KeywordToken::TUnsigned || type == KeywordToken::TUnsignedSymbolic) {
                                    auto* operation = new ConstraintElement::BinaryCompareConstraintFunction();
                                    constraint.apvcfOperation[ATMultiBit-1].absorbElement(operation);
                                    operation->setMultiBit(DMBCOCompareLessUnsigned);
                                    if (type == KeywordToken::TUnsignedSymbolic)
                                       constraint.fIsSymbolic = true;
                                 }
                                 else {
                                    auto* operation = new ConstraintElement::BinaryCompareConstraintFunction();
                                    constraint.apvcfOperation[ATMultiBit-1].absorbElement(operation);
                                    operation->setMultiBit(DMBCOCompareLessSigned);
                                    if (type == KeywordToken::TSignedSymbolic)
                                       constraint.fIsSymbolic = true;
                                 }
                                 DefineGotoCase(AfterOperationAssert)
                              case DMFCOCompareLessOrEqual:
                                 if (type == KeywordToken::TUnsigned || type == KeywordToken::TUnsignedSymbolic) {
                                    auto* operation = new ConstraintElement::BinaryCompareConstraintFunction();
                                    constraint.apvcfOperation[ATMultiBit-1].absorbElement(operation);
                                    operation->setMultiBit(DMBCOCompareLessOrEqualUnsigned);
                                    if (type == KeywordToken::TUnsignedSymbolic)
                                       constraint.fIsSymbolic = true;
                                 }
                                 else {
                                    auto* operation = new ConstraintElement::BinaryCompareConstraintFunction();
                                    constraint.apvcfOperation[ATMultiBit-1].absorbElement(operation);
                                    operation->setMultiBit(DMBCOCompareLessOrEqualSigned);
                                    if (type == KeywordToken::TSignedSymbolic)
                                       constraint.fIsSymbolic = true;
                                 }
                                 DefineGotoCase(AfterOperationAssert)
                              case DMFCOCompareGreaterOrEqual:
                                 if (type == KeywordToken::TUnsigned  || type == KeywordToken::TUnsignedSymbolic) {
                                    auto* operation = new ConstraintElement::BinaryCompareConstraintFunction();
                                    constraint.apvcfOperation[ATMultiBit-1].absorbElement(operation);
                                    operation->setMultiBit(DMBCOCompareGreaterOrEqualUnsigned);
                                    if (type == KeywordToken::TUnsignedSymbolic)
                                       constraint.fIsSymbolic = true;
                                 }
                                 else {
                                    auto* operation = new ConstraintElement::BinaryCompareConstraintFunction();
                                    constraint.apvcfOperation[ATMultiBit-1].absorbElement(operation);
                                    operation->setMultiBit(DMBCOCompareGreaterOrEqualSigned);
                                    if (type == KeywordToken::TSignedSymbolic)
                                       constraint.fIsSymbolic = true;
                                 }
                                 DefineGotoCase(AfterOperationAssert)
                              case DMFCOCompareGreater:
                                 if (type == KeywordToken::TUnsigned  || type == KeywordToken::TUnsignedSymbolic) {
                                    auto* operation = new ConstraintElement::BinaryCompareConstraintFunction();
                                    constraint.apvcfOperation[ATMultiBit-1].absorbElement(operation);
                                    operation->setMultiBit(DMBCOCompareGreaterUnsigned);
                                    if (type == KeywordToken::TUnsignedSymbolic)
                                       constraint.fIsSymbolic = true;
                                 }
                                 else {
                                    auto* operation = new ConstraintElement::BinaryCompareConstraintFunction();
                                    constraint.apvcfOperation[ATMultiBit-1].absorbElement(operation);
                                    operation->setMultiBit(DMBCOCompareGreaterSigned);
                                    if (type == KeywordToken::TSignedSymbolic)
                                       constraint.fIsSymbolic = true;
                                 }
                                 DefineGotoCase(AfterOperationAssert)
                              default:
                                 break;
                           }
                        }
                     }
                  }
                  else if (constraint.apvcfOperation[ATMultiBit-1].isValid()) { // !constraint.apvcfOperation[ATMultiFloat-1].isValid()
                     areferenceOperation = &*constraint.apvcfOperation[ATMultiBit-1];
                     if (areferenceOperation->getArgumentsNumber() == 2) {
                        using BinaryConstraintFunction = ConstraintElement::BinaryConstraintFunction;
                        AssumeCondition(dynamic_cast<const BinaryConstraintFunction*>(areferenceOperation))
                        if (((const BinaryConstraintFunction*) areferenceOperation)->isOperation()) {
                           using BinaryOperationConstraintFunction = ConstraintElement::BinaryOperationConstraintFunction;
                           AssumeCondition(dynamic_cast<const BinaryOperationConstraintFunction*>(areferenceOperation))
                           auto& referenceOperation = *(BinaryOperationConstraintFunction*) areferenceOperation;
                           switch (referenceOperation.getMultiBitOperation()) {
                              case DMBBOModuloUnsigned:
                                 if (type == KeywordToken::TUnsigned  || type == KeywordToken::TUnsignedSymbolic) {
                                    if (type == KeywordToken::TUnsignedSymbolic)
                                       constraint.fIsSymbolic = true;
                                 }
                                 else {
                                    referenceOperation.setMultiBit(DMBBOModuloSigned);
                                    if (type == KeywordToken::TSignedSymbolic)
                                       constraint.fIsSymbolic = true;
                                 }
                                 DefineGotoCase(AfterOperationAssert)
                              case DMBBOLogicalRightShift:
                                 if (type == KeywordToken::TUnsigned  || type == KeywordToken::TUnsignedSymbolic) {
                                    if (type == KeywordToken::TUnsignedSymbolic)
                                       constraint.fIsSymbolic = true;
                                 }
                                 else {
                                    referenceOperation.setMultiBit(DMBBOArithmeticRightShift);
                                    if (type == KeywordToken::TSignedSymbolic)
                                       constraint.fIsSymbolic = true;
                                 }
                                 DefineGotoCase(AfterOperationAssert)
                              default:
                                 break;
                           };
                        }
                     }
                  }
                  DefineAddError(STG::SString("the operator/function do not accept signed/unsigned qualification"))
                  state.freeUnionResult((ConstraintElement*) nullptr);
                  DefineGotoCase(EndParen)
               }
            };
         }
         DefineGotoCaseAndParse(AfterOperationAssert)
      DefineParseCase(BeforeOperationSizeAssert)
         {  AbstractToken token = arguments.lexer().queryToken();
            if (token.getType() == AbstractToken::TOperatorPunctuator
                  && ((const OperatorPunctuatorToken&) token).getType()
                     == OperatorPunctuatorToken::TComma)
               DefineGotoCase(OperationSizeAssert)
         };
         DefineAddError(STG::SString("',' was expected between low bit and high bit in assert reduce/bitset"))
         state.freeUnionResult((ConstraintElement*) nullptr);
         DefineGotoCase(EndParen)
      DefineParseCase(OperationSizeAssert)
         {  AbstractToken token = arguments.lexer().queryToken();
            if (token.getType() == AbstractToken::TNumber) {
               AssumeCondition(dynamic_cast<const NumberToken*>(&arguments.lexer().getContentToken()))
               const auto& token = (const NumberToken&) arguments.lexer().getContentToken();
               if (token.hasSpecialCoding() || token.getSize() != 0) {
                  DefineAddError(STG::SString("size should be standard integer in bitset"));
                  state.freeUnionResult((ConstraintElement*) nullptr);
                  DefineGotoCase(EndParen)
               }
               auto& constraint = state.getUnionResult((ConstraintElement*) nullptr);
               auto& aoperation = *constraint.apvcfOperation[ATMultiBit-1];
               AssumeCondition(dynamic_cast<const ConstraintElement::ExtendConstraintFunction*>(&aoperation))
               auto& operation = static_cast<ConstraintElement::ExtendConstraintFunction&>(aoperation);
               operation.setNewSize(token.getContent().queryInteger());
               DefineGotoCase(AfterOperationAssert)
            }
         }
         DefineAddError(STG::SString("size was expected as first arguments of assert extension"))
         state.freeUnionResult((ConstraintElement*) nullptr);
         DefineGotoCase(EndParen)
      DefineParseCase(BeforeOperationLowBitAssert)
         {  AbstractToken token = arguments.lexer().queryToken();
            if (token.getType() == AbstractToken::TOperatorPunctuator
                  && ((const OperatorPunctuatorToken&) token).getType()
                     == OperatorPunctuatorToken::TComma)
               DefineGotoCase(OperationLowBitAssert)
         };
         DefineAddError(STG::SString("',' was expected before low bit in assert reduce/bitset"))
         state.freeUnionResult((ConstraintElement*) nullptr);
         DefineGotoCase(EndParen)
      DefineParseCase(OperationLowBitAssert)
         {  AbstractToken token = arguments.lexer().queryToken();
            if (token.getType() == AbstractToken::TNumber) {
               AssumeCondition(dynamic_cast<const NumberToken*>(&arguments.lexer().getContentToken()))
               const auto& token = (const NumberToken&) arguments.lexer().getContentToken();
               if (token.hasSpecialCoding() || token.getSize() != 0) {
                  DefineAddError(STG::SString("low bit should be standard integer"));
                  state.freeUnionResult((ConstraintElement*) nullptr);
                  DefineGotoCase(EndParen)
               }
               auto& constraint = state.getUnionResult((ConstraintElement*) nullptr);
               auto& aoperation = *constraint.apvcfOperation[ATMultiBit-1];
               if (aoperation.isMultiBitReduction()) {
                  AssumeCondition(dynamic_cast<const ConstraintElement::ReduceConstraintFunction*>(&aoperation))
                  auto& operation = static_cast<ConstraintElement::ReduceConstraintFunction&>(aoperation);
                  operation.setFirstBit(token.getContent().queryInteger());
                  DefineGotoCase(OperationNextLowBitAssert)
               }
               else if (aoperation.isMultiBitSet()) {
                  AssumeCondition(dynamic_cast<const ConstraintElement::BitSetConstraintFunction*>(&aoperation))
                  auto& operation = static_cast<ConstraintElement::BitSetConstraintFunction&>(aoperation);
                  operation.setFirstBit(token.getContent().queryInteger());
                  DefineGotoCase(OperationNextLowBitAssert)
               }
               else if (aoperation.isMultiBitCastShiftBit()) {
                  AssumeCondition(dynamic_cast<const ConstraintElement::CastShiftBitConstraintFunction*>(&aoperation))
                  auto& operation = static_cast<ConstraintElement::CastShiftBitConstraintFunction&>(aoperation);
                  operation.setShift(token.getContent().queryInteger());
                  DefineGotoCase(AfterOperationAssert)

               }
               else
                  {  AssumeUncalled }
            }
         }
         DefineAddError(STG::SString("low bit was expected as first argument of assert reduce/bitset/shiftbit"))
         state.freeUnionResult((ConstraintElement*) nullptr);
         DefineGotoCase(EndParen)
      DefineParseCase(OperationNextLowBitAssert)
         {  AbstractToken token = arguments.lexer().queryToken();
            if (token.getType() == AbstractToken::TOperatorPunctuator
                  && ((const OperatorPunctuatorToken&) token).getType()
                     == OperatorPunctuatorToken::TComma)
               DefineGotoCase(OperationHighBitAssert)
         };
         DefineAddError(STG::SString("',' was expected between low bit and high bit in assert reduce/bitset"))
         state.freeUnionResult((ConstraintElement*) nullptr);
         DefineGotoCase(EndParen)
      DefineParseCase(OperationHighBitAssert)
         {  AbstractToken token = arguments.lexer().queryToken();
            if (token.getType() == AbstractToken::TNumber) {
               AssumeCondition(dynamic_cast<const NumberToken*>(&arguments.lexer().getContentToken()))
               const auto& token = (const NumberToken&) arguments.lexer().getContentToken();
               if (token.hasSpecialCoding() || token.getSize() != 0) {
                  DefineAddError(STG::SString("high bit should be standard integer"));
                  state.freeUnionResult((ConstraintElement*) nullptr);
                  DefineGotoCase(EndParen)
               }
               auto& constraint = state.getUnionResult((ConstraintElement*) nullptr);
               auto& aoperation = *constraint.apvcfOperation[ATMultiBit-1];
               if (aoperation.isMultiBitReduction()) {
                  AssumeCondition(dynamic_cast<const ConstraintElement::ReduceConstraintFunction*>(&aoperation))
                  auto& operation = static_cast<ConstraintElement::ReduceConstraintFunction&>(aoperation);
                  operation.setLastBit(token.getContent().queryInteger());
               }
               else if (aoperation.isMultiBitSet()) {
                  AssumeCondition(dynamic_cast<const ConstraintElement::BitSetConstraintFunction*>(&aoperation))
                  auto& operation = static_cast<ConstraintElement::BitSetConstraintFunction&>(aoperation);
                  operation.setLastBit(token.getContent().queryInteger());
               }
               else
                  {  AssumeUncalled }
               DefineGotoCase(AfterOperationAssert)
            }
         }
         DefineAddError(STG::SString("high bit was expected as second argument of assert reduce/bitset/shiftbit"))
         state.freeUnionResult((ConstraintElement*) nullptr);
         DefineGotoCase(EndParen)
      DefineParseCase(TypeAssert)
         {  AbstractToken token = arguments.lexer().queryToken();
            if (token.getType() == AbstractToken::TKeyword) {
               switch (((const KeywordToken&) token).getType()) {
                  case KeywordToken::TMulti: case KeywordToken::TMultiSymbolic:
                     {  auto& constraint = state.getUnionResult((ConstraintElement*) nullptr);
                        int size = 0;
                        if (((const KeywordToken&) token).isSingleSized()) {
                           AssumeCondition(dynamic_cast<const SizedKeywordToken*>(&arguments.lexer().getContentToken()))
                           size = ((const SizedKeywordToken&) arguments.lexer().getContentToken()).getSize();
                        }
                        for (ArgumentType type = ATBit; type <= ATMultiFloat; type = (ArgumentType) (type+1)) {
                           auto* operation = new ConstraintElement::CastMultiBitConstraintFunction();
                           constraint.apvcfOperation[type-1].absorbElement(operation);
                           operation->setType(type).setSizeInBits(size);
                        }
                        constraint.fIsSymbolic = ((const KeywordToken&) token).getType() == KeywordToken::TMultiSymbolic;
                     }
                     DefineGotoCase(AfterTypeAssert)
                  case KeywordToken::TBit: case KeywordToken::TBitSymbolic:
                     {  auto& constraint = state.getUnionResult((ConstraintElement*) nullptr);
                        auto* operation = new ConstraintElement::UnaryOperationConstraintFunction();
                        constraint.apvcfOperation[ATMultiBit-1].absorbElement(operation);
                        operation->setCastBit();
                        constraint.fIsSymbolic = ((const KeywordToken&) token).getType() == KeywordToken::TBitSymbolic;
                     }
                     DefineGotoCase(AfterTypeAssert)
                  case KeywordToken::TMultiFloat: case KeywordToken::TMultiFloatSymbolic:
                     {  auto& constraint = state.getUnionResult((ConstraintElement*) nullptr);
                        int sizeExponent = 0, sizeMantissa = 0;
                        if (((const KeywordToken&) token).isDoubleSized()) {
                           AssumeCondition(dynamic_cast<const FloatSizedKeywordToken*>(&arguments.lexer().getContentToken()))
                           const auto& token = (const FloatSizedKeywordToken&) arguments.lexer().getContentToken();
                           sizeExponent = token.getSizeExponent();
                           sizeMantissa = token.getSizeMantissa();
                        }
                        auto* operation = new ConstraintElement::CastMultiFloatConstraintFunction();
                        constraint.apvcfOperation[ATMultiBit-1].absorbElement(operation);
                        operation->setMultiBit().setSigned().setSizeInBits(sizeExponent+sizeMantissa+1);
                        operation = new ConstraintElement::CastMultiFloatConstraintFunction();
                        constraint.apvcfOperation[ATMultiFloat-1].absorbElement(operation);
                        operation->setMultiFloat().setSigned().setSizeInBits(sizeExponent+sizeMantissa+1);
                        constraint.fIsSymbolic = ((const KeywordToken&) token).getType() == KeywordToken::TMultiFloatSymbolic;
                     }
                     DefineGotoCase(AfterTypeAssert)
                  case KeywordToken::TMultiFloatPointer: case KeywordToken::TMultiFloatPointerSymbolic:
                     {  auto& constraint = state.getUnionResult((ConstraintElement*) nullptr);
                        int sizeExponent = 0, sizeMantissa = 0;
                        if (((const KeywordToken&) token).isDoubleSized()) {
                           AssumeCondition(dynamic_cast<const FloatSizedKeywordToken*>(&arguments.lexer().getContentToken()))
                           const auto& token = (const FloatSizedKeywordToken&) arguments.lexer().getContentToken();
                           sizeExponent = token.getSizeExponent();
                           sizeMantissa = token.getSizeMantissa();
                        }
                        auto* operation = new ConstraintElement::CastMultiFloatConstraintFunction();
                        constraint.apvcfOperation[ATMultiBit-1].absorbElement(operation);
                        operation->setPtr().setMultiBit().setSigned().setSizeInBits(sizeExponent+sizeMantissa+1);
                        operation = new ConstraintElement::CastMultiFloatConstraintFunction();
                        constraint.apvcfOperation[ATMultiFloat-1].absorbElement(operation);
                        operation->setPtr().setMultiFloat().setSigned().setSizeInBits(sizeExponent+sizeMantissa+1);
                        constraint.fIsSymbolic = ((const KeywordToken&) token).getType() == KeywordToken::TMultiFloatSymbolic;
                     }
                     DefineGotoCase(AfterTypeAssert)
                  default:
                     break;
               };
            }
         };
         DefineAddError(STG::SString("unknow cast operator"))
         state.freeUnionResult((ConstraintElement*) nullptr);
         DefineGotoCaseAndParse(DoubleEndParen)
      DefineParseCase(AfterTypeAssert)
         {  AbstractToken token = arguments.lexer().queryToken();
            if (token.getType() == AbstractToken::TOperatorPunctuator) {
               if (((const OperatorPunctuatorToken&) token).getType()
                     == OperatorPunctuatorToken::TCloseParen)
                  DefineGotoCase(AfterOperationAssert)
            }
         };
         DefineAddError(STG::SString("unknow cast operator"))
         state.freeUnionResult((ConstraintElement*) nullptr);
         DefineGotoCaseAndParse(DoubleEndParen)
      DefineParseCase(AfterOperationAssert)
         {  AbstractToken token = arguments.lexer().queryToken();
            if (token.getType() == AbstractToken::TOperatorPunctuator
                  && ((const OperatorPunctuatorToken&) token).getType()
                     == OperatorPunctuatorToken::TComma)
               DefineGotoCase(AssertFirstArgument)
            if (token.getType() == AbstractToken::TKeyword
                  && ((const KeywordToken&) token).getType() == KeywordToken::TSymbolic) {
               auto& constraint = state.getUnionResult((ConstraintElement*) nullptr);
               if (!constraint.fIsSymbolic) {
                  constraint.fIsSymbolic = true;
                  DefineGotoCase(AfterOperationAssert)
               }
            };
         };
         DefineAddError(STG::SString("expected ',' after assert(operation ',' ...)"))
         state.freeUnionResult((ConstraintElement*) nullptr);
         DefineGotoCase(EndParen)
      DefineParseCase(AssertFirstArgument)
         if (arguments.lexer().queryToken().getType() == AbstractToken::TIdentifier) {
            const IdentifierToken& token = (const IdentifierToken&)
               arguments.lexer().getContentToken();
            auto& constraint = state.getUnionResult((ConstraintElement*) nullptr);
            constraint.neFirst.setName(token.identifier());
            Cursor cursor(*this);
            if (!locateKey(token.identifier(), cursor, COL::VirtualCollection::RPExact)) {
               DefineAddError(STG::SString("identifier \"").cat(token.identifier())
                  .cat("\" is not known in assert"))
               state.freeUnionResult((ConstraintElement*) nullptr);
               DefineGotoCase(EndParen)
            }
            constraint.neFirst.setValue(cursor.elementAt().getValue());
            if (constraint.neFirst.getSValue().selement().isValid())
               domain_specialize(&constraint.neFirst.getSValue().selement().svalue());
            DefineGotoCase(InAssert)
         }
         DefineAddError(STG::SString("expected known identifier in assert "
               "to constraint its value"))
         state.freeUnionResult((ConstraintElement*) nullptr);
         DefineGotoCase(EndParen)
      DefineParseCase(InAssert)
         {  AbstractToken token = arguments.lexer().queryToken();
            if (token.getType() == AbstractToken::TOperatorPunctuator) {
               auto type = ((const OperatorPunctuatorToken&) token).getType();
               if (type == OperatorPunctuatorToken::TComma) {
                  auto& constraint = state.getUnionResult((ConstraintElement*) nullptr);
                  if (!constraint.neFirst.hasName()) {
                     state.freeUnionResult((ConstraintElement*) nullptr);
                     DefineGotoCase(EndParen)
                  }
                  if (!constraint.neSecond.hasName())
                     DefineGotoCase(AssertArgument)
                  if (constraint.ceResult.isValid()) {
                     DefineAddError(STG::SString("too more arguments for the operation"))
                     state.freeUnionResult((ConstraintElement*) nullptr);
                     DefineGotoCase(EndParen)
                  }
                  DefineShift(InAssert, constraint.ceResult, &ConcreteElement::readToken,
                        (Parser::State::UnionResult<ConcreteElement, ConcreteElement::OperatorStack>*) nullptr)
               }
               else if (type == OperatorPunctuatorToken::TCloseParen) {
                  auto constraint = std::move(state.getUnionResult((ConstraintElement*) nullptr));
                  state.freeUnionResult((ConstraintElement*) nullptr);
                  if (constraint.apply(arguments)) {
                     Cursor cursor(*this);
                     locateKey(constraint.neFirst.getName(), cursor, COL::VirtualCollection::RPExact);
                     if (arguments.hasNotification()) {
                        if (constraint.neFirst.getValue().isValid())
                           arguments.getNotification().notifyAssignment(constraint.neFirst.getName(),
                              constraint.neFirst.getValue().getElement().value(), arguments.getFormatParameters());
                        else
                           arguments.getNotification().notifyEmptyAssignment(constraint.neFirst.getName(),
                              arguments.getFormatParameters());
                     };
                     cursor.elementSAt().setValue(constraint.neFirst.getValue());
                     if (cursor.elementSAt().getSValue().selement().isValid())
                        domain_specialize(&cursor.elementSAt().getSValue().selement().svalue());
                     if (constraint.neSecond.hasName()) {
                        locateKey(constraint.neSecond.getName(), cursor, COL::VirtualCollection::RPExact);
                        if (arguments.hasNotification()) {
                           if (constraint.neSecond.getValue().isValid())
                              arguments.getNotification().notifyAssignment(constraint.neSecond.getName(),
                                 constraint.neSecond.getValue().getElement().value(), arguments.getFormatParameters());
                           else
                              arguments.getNotification().notifyEmptyAssignment(constraint.neSecond.getName(),
                                 arguments.getFormatParameters());
                        };
                        cursor.elementSAt().setValue(constraint.neSecond.getValue());
                        if (cursor.elementSAt().getSValue().selement().isValid())
                           domain_specialize(&cursor.elementSAt().getSValue().selement().svalue());
                     };
                     DefineGotoCase(EndDeclaration)
                  }
                  else
                     DefineGotoCase(BeforeSemiColon)
               }
            }
            if (token.getType() == AbstractToken::TKeyword
                  && ((const KeywordToken&) token).getType() == KeywordToken::TSymbolic) {
               auto& constraint = state.getUnionResult((ConstraintElement*) nullptr);
               if (!constraint.fIsSymbolic) {
                  constraint.fIsSymbolic = true;
                  DefineGotoCase(AfterOperationAssert)
               }
            };
         };
         DefineAddError(STG::SString("expected ',' after assert(operation ',' ...)"))
         state.freeUnionResult((ConstraintElement*) nullptr);
         DefineGotoCase(EndParen)
      DefineParseCase(AssertArgument)
         if (arguments.lexer().queryToken().getType() == AbstractToken::TIdentifier) {
            const IdentifierToken& token = (const IdentifierToken&)
               arguments.lexer().getContentToken();
            auto& constraint = state.getUnionResult((ConstraintElement*) nullptr);
            if (!constraint.neSecond.hasName()) {
               constraint.neSecond.setName(token.identifier());
               Cursor cursor(*this);
               if (!locateKey(token.identifier(), cursor, COL::VirtualCollection::RPExact)) {
                  DefineAddError(STG::SString("identifier \"").cat(token.identifier())
                     .cat("\" is not known in assert"))
                  state.freeUnionResult((ConstraintElement*) nullptr);
                  DefineGotoCase(EndParen)
               }
               constraint.neSecond.setValue(cursor.elementAt().getValue());
               domain_specialize(&constraint.neSecond.getSValue().selement().svalue());
               DefineGotoCase(InAssert)
            }
            else
               DefineShiftAndParse(InAssert, constraint.ceResult, &ConcreteElement::readToken,
                     (Parser::State::UnionResult<ConcreteElement, ConcreteElement::OperatorStack>*) nullptr)
         }
         else
            DefineShiftAndParse(InAssert, state.getUnionResult((ConstraintElement*) nullptr)
               .ceResult, &ConcreteElement::readToken, (Parser::State::UnionResult<ConcreteElement, ConcreteElement::OperatorStack>*) nullptr)
      DefineParseCase(EndNamedDeclaration)
         {  AbstractToken token = arguments.lexer().queryToken();
            if (token.getType() == AbstractToken::TOperatorPunctuator
                  && ((const OperatorPunctuatorToken&) token).getType()
                     == OperatorPunctuatorToken::TSemiColon) {
               if (arguments.hasNotification()) {
                  Cursor cursor = std::move(state.getUnionResult((Cursor*) nullptr));
                  state.freeUnionResult((Cursor*) nullptr);
                  NamedElement& namedElement = cursor.elementSAt();
                  if (namedElement.getValue().isValid()) {
                     arguments.getNotification().notifyAssignment(namedElement.getName(),
                        namedElement.getValue().getElement().value(), arguments.getFormatParameters());
                     domain_specialize(&namedElement.getSValue().selement().svalue());
                  }
                  else
                     arguments.getNotification().notifyEmptyAssignment(namedElement.getName(),
                        arguments.getFormatParameters());
               };
               DefineGotoCase(Begin)
            }
         };
         state.freeUnionResult((Cursor*) nullptr);
         DefineAddError(STG::SString("expected ';' at the end of a declaration/instruction"))
         DefineGotoCase(BeforeSemiColon)
      DefineParseCase(EndDeclaration)
         {  AbstractToken token = arguments.lexer().queryToken();
            if (token.getType() == AbstractToken::TOperatorPunctuator
                  && ((const OperatorPunctuatorToken&) token).getType()
                     == OperatorPunctuatorToken::TSemiColon)
               DefineGotoCase(Begin)
         };
         DefineAddError(STG::SString("expected ';' at the end of a declaration/instruction"))
         DefineGotoCase(BeforeSemiColon)
      DefineParseCase(DoubleEndParen)
         {  AbstractToken token = arguments.lexer().queryToken();
            if (token.getType() == AbstractToken::TOperatorPunctuator
                  && ((const OperatorPunctuatorToken&) token).getType()
                     == OperatorPunctuatorToken::TCloseParen)
               DefineGotoCase(EndParen)
         };
         DefineGotoCase(DoubleEndParen)
      DefineParseCase(EndParen)
         {  AbstractToken token = arguments.lexer().queryToken();
            if (token.getType() == AbstractToken::TOperatorPunctuator
                  && ((const OperatorPunctuatorToken&) token).getType()
                     == OperatorPunctuatorToken::TCloseParen)
               DefineGotoCase(BeforeSemiColon)
         };
         DefineGotoCase(EndParen)
      DefineParseCase(BeforeSemiColon)
         {  AbstractToken token = arguments.lexer().queryToken();
            if (token.getType() == AbstractToken::TOperatorPunctuator
                  && ((const OperatorPunctuatorToken&) token).getType()
                     == OperatorPunctuatorToken::TSemiColon)
               DefineGotoCase(Begin)
         };
         DefineGotoCase(BeforeSemiColon)
      default:
         break;
   };
   AssumeUncalled
   return result;
}

void
MapOfElements::_read(ISBase& in, const STG::IOObject::FormatParameters& params) {
   const auto& parameters = (const FormatParameters&) params;
   Parser parser(*this, *this);
   parser.state().shift(*this, &MapOfElements::readToken,
         (Parser::State::UnionResult<NamedElement, Cursor, ConstraintElement>*) nullptr);
   if (parameters.isDebug())
      parser.setDebug();
   if (parameters.hasNotification())
      parser.setNotification(parameters.getNotification(), params);
   try {
      parser.parse(in);
   }
   catch (ESPreconditionError& error) {
      std::cerr << error << std::endl;
   };
}

class WriteBuffer {
  private:
   char szInitBuffer[1024];
   char* szCurrent;
   int uSize;

  public:
   WriteBuffer() : szCurrent(szInitBuffer), uSize(1024) {}
   ~WriteBuffer()
      {  if (szCurrent != szInitBuffer) {
            delete [] szCurrent;
            szCurrent = szInitBuffer;
            uSize = 1024;
         }
      }

   char* getBuffer() const { return szCurrent; }
   int getBufferSize() const { return uSize; }

   static char* increase_buffer_size(char* buffer, int old_length, int new_length, void* writer)
      {  auto* thisObject = reinterpret_cast<WriteBuffer*>(writer);
         AssumeCondition(thisObject->szCurrent == buffer && thisObject->uSize == old_length)
         if (new_length > old_length) {
            char* result = new char[new_length];
            memcpy(result, buffer, old_length);
            if (thisObject->szInitBuffer != buffer)
               delete [] buffer;
            thisObject->szCurrent = result;
            thisObject->uSize = new_length;
            buffer = result;
         }
         return buffer;
      }
};


class ReadEventNotification : public VirtualReadEventNotification {
  public:
   virtual void notifyText(const STG::SubString& text, STG::IOObject::FormatParameters& parameters) override
      {  text.write(std::cout);
         std::cout << '\n';
      }
   virtual void notifyEmptyAssignment(const STG::SubString& variable,
         const STG::IOObject::FormatParameters& parameters) override
      {  STG::IOObject::OSStream out(std::cout);
         out.writeall(variable.asPersistent());
         out << " <- empty\n";
         out.flush();
      }
   virtual void notifyAssignment(const STG::SubString& variable,
         const DomainElement& element, const STG::IOObject::FormatParameters& parameters) override
      {  STG::IOObject::OSStream out(std::cout);
         out.writeall(variable.asPersistent());
         out << " <- ";
         WriteBuffer writeBuffer;
         int length = 0;
         domain_write(element, writeBuffer.getBuffer(), writeBuffer.getBufferSize(),
               &length, &writeBuffer, &WriteBuffer::increase_buffer_size);
         STG::TChunk<char> chunk;
         chunk.string = writeBuffer.getBuffer();
         chunk.length = length;
         out.writechunk(&chunk);
         out.put('\n').flush();
      }
};

STG::DIOObject::OSSubString* possOut = new STG::DIOObject::OSSubString;

const char* debugWrite(const STG::IOObject* object) {
   possOut->buffer().clear();
   object->write((STG::IOObject::OSBase&) *possOut, STG::IOObject::FormatParameters());
   return possOut->buffer().getChunk().string;
}

const char* (*pdebugWrite)(const STG::IOObject* object) = &debugWrite;

int
main(int argc, char** argv) {
   try {

   if (argc > 1) {
      MapOfElements::FormatParameters formatParameters;
      int argFile = 1;
      if (argc > 2) {
         if (strcmp(argv[1], "-d") == 0) {
            formatParameters.setDebug();
            argFile = 2;
         }
         else
            std::cout << "Usage: TestDomain file.txt" << std::endl;
      };
      if (argc > argFile+1)
         std::cout << "Usage: TestDomain file.txt" << std::endl;
      else {
         STG::DIOObject::IFStream in(argv[argFile]);
         MapOfElements mapOfElements;
         ReadEventNotification eventNotification;
         mapOfElements.read(in, formatParameters.setNotification(eventNotification));
      }
   }
   else
      std::cout << "Usage: TestDomain file.txt" << std::endl;

   }
   catch (STG::EReadError&) {
      std::cout << "read error" << std::endl;
   }
   catch (ESPreconditionError& error) {
      std::cerr << error << std::endl;
   };

   delete possOut;
#if DefineDebugLevel > 2
   EnhancedObject::writeMessages(std::cout);
   std::cout << std::endl;
#endif

   return 0;
}

