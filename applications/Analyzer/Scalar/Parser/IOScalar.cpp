/////////////////////////////////
//
// Library   : Analyzer/Scalar
// Unit      : approximate scalar elements
// File      : IOScalar.cpp
// Author    : Franck Vedrine
// Copyright : CEA LIST - 2019-2020 - all rights reserved
//
// Description :
//   Implementation of persistent scalar elements.
//

#include "Analyzer/Scalar/Parser/IOScalar.h"

namespace Analyzer {}

#include <iostream>
#include "Analyzer/Scalar/VirtualExact/General/MultiFloatElement.template"

namespace Analyzer { namespace Scalar { namespace Approximate {

STG::SubString
IOScalar::Operator::queryOperatorName() const {
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

IOScalar::Operator&
IOScalar::Operator::setType(Type type, bool isPrefix) {
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

IOScalar::PPEnhancedObject
IOScalar::OperatorStack::applyInterval(Operator& operation, ArgumentType& argumentType,
      Parser::Arguments& context) {
   AssumeCondition(operation.getType() == Operator::TInterval
         && operation.ppveFirst.isValid() && operation.ppveSecond.isValid())
   ArgumentType secondArgumentType = ATUndefined;
   PPEnhancedObject secondArgument = operation.extractSecondArgument(secondArgumentType);
   ArgumentType firstArgumentType = ATUndefined;
   PPEnhancedObject firstArgument = operation.extractFirstArgument(firstArgumentType);
   if (secondArgumentType != firstArgumentType) {
      argumentType = firstArgumentType;
      context.addErrorMessage(STG::SString(
         "interval has incompatible types for elements"));
      return firstArgument;
   };

   argumentType = firstArgumentType;
   if (argumentType == ATMultiBit)
      return context.newInterval(argumentType, firstArgument, secondArgument,
            operation.hasSignedTag(), operation.isSymbolic());
   else {
      AssumeCondition(argumentType == ATMultiFloat)
      return context.newFloatInterval(argumentType, firstArgument, secondArgument,
            operation.isSymbolic());
   };
}

IOScalar::PPEnhancedObject
IOScalar::OperatorStack::applyCallFunction(Operator& operation, ArgumentType& argumentType,
      Parser::Arguments& context) {
   AssumeCondition(operation.getType() == Operator::TCall
         && operation.ppveFirst.isValid())
   ArgumentType secondArgumentType = ATUndefined;
   PPEnhancedObject secondArgument;
   if (operation.ppveSecond.isValid())
      secondArgument = operation.extractSecondArgument(secondArgumentType);
   ArgumentType firstArgumentType = ATUndefined;
   PPEnhancedObject firstArgument = operation.extractFirstArgument(firstArgumentType);

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
            MultiBit::ExtendOperation applyOperation;
            if (operation.getTypeCall() == Operator::TCFExtendZero)
               applyOperation.setExtendWithZero(operation.getExtension());
            else
               applyOperation.setExtendWithSign(operation.getExtension());
            return context.applyAssign(firstArgumentType, firstArgument, applyOperation,
                  operation.hasSymbolicField());
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
         return context.applyAssign(firstArgumentType, firstArgument,
               MultiBit::Operation().setConcat(), secondArgumentType, *secondArgument,
               operation.hasSymbolicField());
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
         {  int originalSize = context.getSizeInBits(*firstArgument);
            if (operation.uHighBit > originalSize || operation.uLowBit < 0
                  || operation.uLowBit > operation.uHighBit) {
               context.addErrorMessage(STG::SString(
                  "bad low bit/high bit for reduction"));
               return firstArgument;
            };
            MultiBit::ReduceOperation applyOperation;
            applyOperation.setLowBit(operation.uLowBit).setHighBit(operation.uHighBit);
            return context.applyAssign(firstArgumentType, firstArgument, applyOperation,
                  operation.hasSymbolicField());
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
         {  int originalSize = context.getSizeInBits(*firstArgument);
            int argumentSize = context.getSizeInBits(*secondArgument);
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
            MultiBit::BitSetOperation applyOperation;
            applyOperation.setLowBit(operation.uLowBit).setHighBit(operation.uHighBit);
            return context.applyAssign(firstArgumentType, firstArgument, applyOperation,
                  secondArgumentType, *secondArgument, operation.hasSymbolicField());
         };
      case Operator::TCFShiftBit:
         if (secondArgument.isValid()) {
            context.addErrorMessage(STG::SString("too more arguments for unary call"));
            return firstArgument;
         };
         if (firstArgumentType != ATMultiBit) {
            context.addErrorMessage(STG::SString(
               "incompatible type for cast shift bit: multibit is expected"));
            return firstArgument;
         };
         {  int originalSize = context.getSizeInBits(*firstArgument);
            if (operation.uLowBit < 0 || operation.uLowBit >= originalSize) {
               context.addErrorMessage(STG::SString(
                  "the chosen bit is outside the multibit in the shiftbit operation"));
               return firstArgument;
            };
            auto oldArgumentType = argumentType;
            argumentType = ATBit;
            return context.applyCastAssign(oldArgumentType, firstArgument, argumentType,
                  MultiBit::CastShiftBitOperation(operation.uLowBit), operation.hasSymbolicField());
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
         {  MultiBit::Operation applyOperation;
            if (operation.hasUnsignedTag())
               applyOperation.setNextUnsignedAssign();
            else if (operation.hasSignedTag())
               applyOperation.setNextSignedAssign();
            else {
               context.addErrorMessage(STG::SString(
                  "increment operation should be signed or unsigned"));
               return firstArgument;
            };
            return context.applyAssign(firstArgumentType, firstArgument, applyOperation,
                  operation.hasSymbolicField());
         }
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
         {  MultiBit::Operation applyOperation;
            if (operation.hasUnsignedTag())
               applyOperation.setPrevUnsignedAssign();
            else if (operation.hasSignedTag())
               applyOperation.setPrevSignedAssign();
            else {
               context.addErrorMessage(STG::SString(
                  "decrement operation should be signed or unsigned"));
               return firstArgument;
            };
            return context.applyAssign(firstArgumentType, firstArgument, applyOperation,
                  operation.hasSymbolicField());
         }
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
            MultiBit::Operation applyOperation;
            auto type = operation.getTypeCall();
            if (type == Operator::TCFMinSigned)
               applyOperation.setMinSignedAssign();
            else if (type == Operator::TCFMinUnsigned)
               applyOperation.setMinUnsignedAssign();
            else if (type == Operator::TCFMaxSigned)
               applyOperation.setMaxSignedAssign();
            else // type == Operator::TCFMaxUnsigned
               applyOperation.setMaxUnsignedAssign();
            return context.applyAssign(argumentType, firstArgument, applyOperation,
                  argumentType, *secondArgument, operation.hasSymbolicField());
         }
         else if (argumentType == ATBit) {
            Bit::Operation applyOperation;
            auto type = operation.getTypeCall();
            if (type == Operator::TCFMinSigned || type == Operator::TCFMinUnsigned)
               applyOperation.setMinAssign();
            else // (type == Operator::TCFMaxUnsigned) || (type == Operator::TCFMaxUnsigned)
               applyOperation.setMaxAssign();
            return context.applyAssign(argumentType, firstArgument, applyOperation,
                  argumentType, *secondArgument, operation.hasSymbolicField());
         }
         else {
            AssumeCondition(argumentType == ATMultiFloat)
            Floating::Operation applyOperation;
            auto type = operation.getTypeCall();
            if (type == Operator::TCFMinSigned || type == Operator::TCFMinUnsigned)
               applyOperation.setMinAssign();
            else // (type == Operator::TCFMaxUnsigned) || (type == Operator::TCFMaxUnsigned)
               applyOperation.setMaxAssign();
            return context.applyAssign(argumentType, firstArgument, applyOperation,
                  argumentType, *secondArgument, operation.hasSymbolicField());
         }
      default:
         break;
   };
   return PPEnhancedObject();
}

IOScalar::PPEnhancedObject
IOScalar::OperatorStack::apply(Operator& operation, ArgumentType& argumentType,
      Parser::Arguments& context) {
   Operator::Type type = operation.getType();
   if (type == Operator::TCast) {
      PPEnhancedObject argument = operation.extractLastArgument(argumentType);
      auto typeCast = operation.getTypeCast();
      PPEnhancedObject result;
      if (argumentType == ATMultiBit) {
         if (typeCast == Operator::TCBit) {
            result = context.applyCastAssign(argumentType, argument, ATBit, 
               MultiBit::Operation().setCastBit(), operation.hasSymbolicField());
            argumentType = ATBit;
         }
         else if (typeCast == Operator::TCMultiBit) {
            if (!operation.hasSize()) {
               context.addErrorMessage(STG::SString(
                  "multibit cast should define a target size when applied on multibits"));
               return argument;
            };
            result = context.applyCastAssign(argumentType, argument, ATMultiBit, 
               MultiBit::CastMultiBitOperation().setSize(operation.getSize()),
               operation.hasSymbolicField());
         }
         else if (typeCast == Operator::TCMultiFloat) {
            if (!operation.hasSizeExponent() || !operation.hasSizeMantissa()) {
               context.addErrorMessage(STG::SString(
                  "multifloat cast should define an exponent and a mantissa size when applied on multibits"));
               return argument;
            };
            result = context.applyCastAssign(argumentType, argument, ATMultiFloat,
               MultiBit::CastMultiFloatOperation().setSigned()
                  .setSizeExponent(operation.getSizeExponent())
                  .setSizeMantissa(operation.getSizeMantissa()),
               operation.hasSymbolicField());
            argumentType = ATMultiFloat;
         }
         else if (typeCast == Operator::TCMultiFloatPtr) {
            if (!operation.hasSizeExponent() || !operation.hasSizeMantissa()) {
               context.addErrorMessage(STG::SString(
                  "multifloatptr cast should define an exponent and a mantissa size when applied on multibits"));
               return argument;
            };
            if (context.getSizeInBits(*argument) != operation.getSizeExponent()
                  + operation.getSizeMantissa() + 1) {
               context.addErrorMessage(STG::SString(
                  "multifloatptr cast should keep the original size of its argument"));
               return argument;
            };
            result = context.applyCastAssign(argumentType, argument, ATMultiFloat,
               MultiBit::CastMultiFloatPointerOperation().setSigned()
                  .setSizeExponent(operation.getSizeExponent())
                  .setSizeMantissa(operation.getSizeMantissa()),
               operation.hasSymbolicField());
            argumentType = ATMultiFloat;
         }
         else
            {  AssumeUncalled }
      }
      else if (argumentType == ATBit) {
         if (typeCast == Operator::TCBit)
            result = argument;
         else if (typeCast == Operator::TCMultiBit) {
            result = context.applyCastAssign(argumentType, argument, ATMultiBit, Bit
               ::CastMultiBitOperation().setSize(!operation.hasSize() ? 1 : operation.getSize()),
               operation.hasSymbolicField());
            argumentType = ATMultiBit;
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
            result = context.applyCastAssign(argumentType, argument, ATMultiBit,
               Floating::CastMultiBitOperation().setSize(!operation.hasSize()
                  ? context.getSizeInBits(*argument) : operation.getSize()),
               operation.hasSymbolicField());
            argumentType = ATMultiBit;
         }
         else if (typeCast == Operator::TCMultiFloat) {
            if (!operation.hasSizeExponent() || !operation.hasSizeMantissa()) {
               context.addErrorMessage(STG::SString(
                  "multifloat cast should define an exponent and a mantissa size when applied on multifloats"));
               return argument;
            };
            result = context.applyCastAssign(argumentType, argument, ATMultiFloat,
               Floating::CastMultiFloatOperation().setSigned()
                  .setSizeExponent(operation.getSizeExponent())
                  .setSizeMantissa(operation.getSizeMantissa()),
               operation.hasSymbolicField());
            argumentType = ATMultiFloat;
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
      PPEnhancedObject argument = operation.extractLastArgument(argumentType);
      switch (type) {
         case Operator::TUnaryPlus:
            return argument;
         case Operator::TUnaryMinus:
            if (argumentType == ATMultiBit)
               return context.applyAssign(argumentType, argument,
                  MultiBit::Operation().setOppositeSignedAssign(),
                  operation.hasSymbolicField());
            else if (argumentType == ATMultiFloat)
               return context.applyAssign(argumentType, argument,
                  Floating::Operation().setOppositeAssign(),
                  operation.hasSymbolicField());
            else {
               context.addErrorMessage(STG::SString(
                  "unsupported type for the unary - operation"));
               return argument;
            };
         case Operator::TNot:
            if (argumentType != ATBit) {
               context.addErrorMessage(STG::SString(
                  "types others than bits do not support unary ! operation"));
               return argument;
            };
            return context.applyAssign(argumentType, argument,
               Bit::Operation().setNegateAssign(), operation.hasSymbolicField());
         case Operator::TComplement:
            if (argumentType != ATMultiBit) {
               context.addErrorMessage(STG::SString(
                  "types other than multibits do not support unary ~ operation"));
               return argument;
            };
            return context.applyAssign(argumentType, argument,
               MultiBit::Operation().setBitNegateAssign(),
               operation.hasSymbolicField());
         default:
            context.addErrorMessage(STG::SString("unknown unary operation type"));
            return argument;
      };
   }
   else if (argumentsNumber == 2) {
      ArgumentType secondArgumentType = ATUndefined;
      PPEnhancedObject secondArgument = operation.extractSecondArgument(secondArgumentType);
      ArgumentType firstArgumentType = ATUndefined;
      PPEnhancedObject firstArgument = operation.extractFirstArgument(firstArgumentType);
      if (firstArgumentType != secondArgumentType) {
         argumentType = firstArgumentType;
         context.addErrorMessage(STG::SString("arguments have incompatible types"));
         return firstArgument;
      };

      if (type >= Operator::TEqual && type <= Operator::TGreater) {
         auto thisType = firstArgumentType;
         argumentType = ATBit;
         if (thisType == ATMultiBit) {
            MultiBit::Operation::Type applyType;
            switch (type) {
               case Operator::TEqual: applyType = MultiBit::Operation::TCompareEqual; break;
               case Operator::TDifferent: applyType = MultiBit::Operation::TCompareDifferent; break;
               case Operator::TLessOrEqual:
                  if (operation.hasUnsignedTag())
                     applyType = MultiBit::Operation::TCompareLessOrEqualUnsigned;
                  else if (operation.hasSignedTag())
                     applyType = MultiBit::Operation::TCompareLessOrEqualSigned;
                  else if (operation.hasFloatTag())
                     applyType = MultiBit::Operation::TCompareLessOrEqualFloat;
                  else {
                     context.addErrorMessage(STG::SString("comparison on multibits should be signed or unsigned"));
                     return firstArgument;
                  };
                  break;
               case Operator::TGreaterOrEqual:
                  if (operation.hasUnsignedTag())
                     applyType = MultiBit::Operation::TCompareGreaterOrEqualUnsigned;
                  else if (operation.hasSignedTag())
                     applyType = MultiBit::Operation::TCompareGreaterOrEqualSigned;
                  else if (operation.hasFloatTag())
                     applyType = MultiBit::Operation::TCompareGreaterOrEqualFloat;
                  else {
                     context.addErrorMessage(STG::SString("comparison on multibits should be signed or unsigned"));
                     return firstArgument;
                  };
                  break;
               case Operator::TLess:
                  if (operation.hasUnsignedTag())
                     applyType = MultiBit::Operation::TCompareLessUnsigned;
                  else if (operation.hasSignedTag())
                     applyType = MultiBit::Operation::TCompareLessSigned;
                  else if (operation.hasFloatTag())
                     applyType = MultiBit::Operation::TCompareLessFloat;
                  else {
                     context.addErrorMessage(STG::SString("comparison on multibits should be signed or unsigned"));
                     return firstArgument;
                  };
                  break;
               case Operator::TGreater:
                  if (operation.hasUnsignedTag())
                     applyType = MultiBit::Operation::TCompareGreaterUnsigned;
                  else if (operation.hasSignedTag())
                     applyType = MultiBit::Operation::TCompareGreaterSigned;
                  else if (operation.hasFloatTag())
                     applyType = MultiBit::Operation::TCompareGreaterFloat;
                  else {
                     context.addErrorMessage(STG::SString("comparison on multibits should be signed or unsigned"));
                     return firstArgument;
                  };
                  break;
               default:
                  AssumeUncalled
                  applyType = MultiBit::Operation::TCompareEqual; break;
            };
            return context.applyCastAssign(firstArgumentType, firstArgument, ATBit,
               MultiBit::Operation().setType(applyType).setConst().setArgumentsNumber(1),
               secondArgumentType, *secondArgument, operation.hasSymbolicField());
         }
         else if (thisType == ATBit) {
            Bit::Operation::Type applyType;
            switch (type) {
               case Operator::TEqual: applyType = Bit::Operation::TCompareEqual; break;
               case Operator::TDifferent: applyType = Bit::Operation::TCompareDifferent; break;
               case Operator::TLessOrEqual: applyType = Bit::Operation::TCompareLessOrEqual; break;
               case Operator::TGreaterOrEqual: applyType = Bit::Operation::TCompareGreaterOrEqual; break;
               case Operator::TLess: applyType = Bit::Operation::TCompareLess; break;
               case Operator::TGreater: applyType = Bit::Operation::TCompareGreater; break;
               default:
                  AssumeUncalled
                  applyType = Bit::Operation::TCompareEqual; break;
            };
            return context.applyCastAssign(firstArgumentType, firstArgument, ATBit,
               Bit::Operation().setType(applyType).setConst().setArgumentsNumber(1),
               secondArgumentType, *secondArgument, operation.hasSymbolicField());
         }
         else {
            AssumeCondition(thisType == ATMultiFloat)
            Floating::Operation::Type applyType;
            switch (type) {
               case Operator::TEqual: applyType = Floating::Operation::TCompareEqual; break;
               case Operator::TDifferent: applyType = Floating::Operation::TCompareDifferent; break;
               case Operator::TLessOrEqual: applyType = Floating::Operation::TCompareLessOrEqual; break;
               case Operator::TGreaterOrEqual: applyType = Floating::Operation::TCompareGreaterOrEqual; break;
               case Operator::TLess: applyType = Floating::Operation::TCompareLess; break;
               case Operator::TGreater: applyType = Floating::Operation::TCompareGreater; break;
               default:
                  AssumeUncalled
                  applyType = Floating::Operation::TCompareEqual; break;
            };
            return context.applyCastAssign(firstArgumentType, firstArgument, ATBit,
               Floating::Operation().setType(applyType).setConst().setArgumentsNumber(1),
               secondArgumentType, *secondArgument, operation.hasSymbolicField());
         };
      }

      argumentType = firstArgumentType;
      if (argumentType == ATMultiBit) {
         MultiBit::Operation::Type applyType;
         switch (type) {
            case Operator::TPlus:
               if (operation.hasUnsignedTag())
                  applyType = MultiBit::Operation::TPlusUnsignedAssign;
               else if (operation.hasSignedTag())
                  applyType = MultiBit::Operation::TPlusSignedAssign;
               else if (operation.hasUnsignedWithSignedTag())
                  applyType = MultiBit::Operation::TPlusUnsignedWithSignedAssign;
               else if (operation.hasFloatTag())
                  applyType = MultiBit::Operation::TPlusFloatAssign;
               else
                  applyType = MultiBit::Operation::TPlusUnsignedAssign;
               break;
            case Operator::TMinus:
               if (operation.hasUnsignedTag())
                  applyType = MultiBit::Operation::TMinusUnsignedAssign;
               else if (operation.hasSignedTag())
                  applyType = MultiBit::Operation::TMinusSignedAssign;
               else if (operation.hasFloatTag())
                  applyType = MultiBit::Operation::TMinusFloatAssign;
               else
                  applyType = MultiBit::Operation::TMinusUnsignedAssign;
               break;
            case Operator::TTimes:
               if (operation.hasUnsignedTag())
                  applyType = MultiBit::Operation::TTimesUnsignedAssign;
               else if (operation.hasSignedTag())
                  applyType = MultiBit::Operation::TTimesSignedAssign;
               else if (operation.hasFloatTag())
                  applyType = MultiBit::Operation::TTimesFloatAssign;
               else
                  applyType = MultiBit::Operation::TTimesUnsignedAssign;
               break;
            case Operator::TDivide:
               if (operation.hasUnsignedTag())
                  applyType = MultiBit::Operation::TDivideUnsignedAssign;
               else if (operation.hasSignedTag())
                  applyType = MultiBit::Operation::TDivideSignedAssign;
               else if (operation.hasFloatTag())
                  applyType = MultiBit::Operation::TDivideFloatAssign;
               else
                  applyType = MultiBit::Operation::TDivideUnsignedAssign;
               break;
            case Operator::TModulo:
               if (operation.hasUnsignedTag())
                  applyType = MultiBit::Operation::TModuloUnsignedAssign;
               else if (operation.hasSignedTag())
                  applyType = MultiBit::Operation::TModuloSignedAssign;
               else
                  applyType = MultiBit::Operation::TModuloUnsignedAssign;
               break;
            case Operator::TLeftShift:
               applyType = MultiBit::Operation::TLeftShiftAssign;
               break;
            case Operator::TRightShift:
               if (operation.hasUnsignedTag())
                  applyType = MultiBit::Operation::TLogicalRightShiftAssign;
               else if (operation.hasSignedTag())
                  applyType = MultiBit::Operation::TArithmeticRightShiftAssign;
               else
                  applyType = MultiBit::Operation::TLogicalRightShiftAssign;
               break;
            case Operator::TLogicalAnd:
               context.addErrorMessage(STG::SString("unsupported operator && on multibits"));
               return firstArgument;
            case Operator::TLogicalOr:
               context.addErrorMessage(STG::SString("unsupported operator || on multibits"));
               return firstArgument;
            case Operator::TBitAnd: applyType = MultiBit::Operation::TBitAndAssign; break;
            case Operator::TBitOr: applyType = MultiBit::Operation::TBitOrAssign; break;
            case Operator::TBitXOr: applyType = MultiBit::Operation::TBitExclusiveOrAssign; break;
            case Operator::TMerge:
               return context.merge(firstArgumentType, firstArgument,
                     secondArgumentType, secondArgument, operation.hasSymbolicField());
            case Operator::TIntersect:
               return context.intersect(firstArgumentType, firstArgument,
                     secondArgumentType, secondArgument, operation.hasSymbolicField());
            default:
               context.addErrorMessage(STG::SString("unknown binary operation type for multibit"));
               return firstArgument;
         };
         return context.applyAssign(firstArgumentType, firstArgument,
            MultiBit::Operation().setType(applyType).setArgumentsNumber(1),
            secondArgumentType, *secondArgument, operation.hasSymbolicField());
      }
      else if (argumentType == ATBit) {
         Bit::Operation::Type applyType;
         switch (type) {
            case Operator::TPlus: applyType = Bit::Operation::TPlusAssign; break;
            case Operator::TMinus: applyType = Bit::Operation::TMinusAssign; break;
            case Operator::TLogicalAnd: applyType = Bit::Operation::TAndAssign; break;
            case Operator::TLogicalOr: applyType = Bit::Operation::TOrAssign; break;
            case Operator::TBitAnd: applyType = Bit::Operation::TAndAssign; break;
            case Operator::TBitOr: applyType = Bit::Operation::TOrAssign; break;
            case Operator::TBitXOr: applyType = Bit::Operation::TExclusiveOrAssign; break;
            case Operator::TMerge:
               return context.merge(firstArgumentType, firstArgument,
                     secondArgumentType, secondArgument, operation.hasSymbolicField());
            case Operator::TIntersect:
               return context.intersect(firstArgumentType, firstArgument,
                     secondArgumentType, secondArgument, operation.hasSymbolicField());
            default:
               context.addErrorMessage(STG::SString("unknown binary operation type for bit"));
               return firstArgument;
         };
         return context.applyAssign(firstArgumentType, firstArgument,
            Bit::Operation().setType(applyType).setArgumentsNumber(1),
            secondArgumentType, *secondArgument, operation.hasSymbolicField());
      }
      else {
         AssumeCondition(argumentType == ATMultiFloat)
         Floating::Operation::Type applyType;
         switch (type) {
            case Operator::TPlus: applyType = Floating::Operation::TPlusAssign; break;
            case Operator::TMinus: applyType = Floating::Operation::TMinusAssign; break;
            case Operator::TTimes: applyType = Floating::Operation::TTimesAssign; break;
            case Operator::TDivide: applyType = Floating::Operation::TDivideAssign; break;
            case Operator::TMerge:
               return context.merge(firstArgumentType, firstArgument,
                     secondArgumentType, secondArgument, operation.hasSymbolicField());
            case Operator::TIntersect:
               return context.intersect(firstArgumentType, firstArgument,
                     secondArgumentType, secondArgument, operation.hasSymbolicField());
            default:
               context.addErrorMessage(STG::SString("unsupported binary operation for floating point type"));
               return firstArgument;
         };
         return context.applyAssign(firstArgumentType, firstArgument,
            Floating::Operation().setType(applyType).setArgumentsNumber(1),
            secondArgumentType, *secondArgument, operation.hasSymbolicField());
      }
   }
   else {
      AssumeCondition(type == Operator::TConditional)
      ArgumentType thirdArgumentType = ATUndefined;
      PPEnhancedObject thirdArgument = operation.extractThirdArgument(thirdArgumentType);
      ArgumentType secondArgumentType = ATUndefined;
      PPEnhancedObject secondArgument = operation.extractSecondArgument(secondArgumentType);
      ArgumentType firstArgumentType = ATUndefined;
      PPEnhancedObject firstArgument = operation.extractFirstArgument(firstArgumentType);
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
      return context.applyGuardAssign(ATBit, firstArgument, secondArgumentType,
            secondArgument, thirdArgumentType, thirdArgument, operation.hasSymbolicField());
   };
   return PPEnhancedObject();
}

IOScalar::Operator&
IOScalar::OperatorStack::pushOperator(Operator::Type typeOperator,
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
      PPEnhancedObject resultElement;
      ArgumentType typeResult = ATUndefined;
      while (!isEmpty() && (isLeftAssociative
            ? ((lastOperator = &getSLast())->getPrecedence()
               >= expressionOperator->getPrecedence())
            : ((lastOperator = &getSLast())->getPrecedence()
               > expressionOperator->getPrecedence()))) {
         if (resultElement.isValid())
            lastOperator->setLastArgument(resultElement, typeResult);
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
      expressionOperator->setFirstArgument(resultElement, typeResult);
      insertNewAtEnd(result = expressionOperator.extractElement());
   };
   return *result;
}


IOScalar::PPEnhancedObject
IOScalar::OperatorStack::clear(ArgumentType& argumentType, Parser::Arguments& context) {
   PPEnhancedObject result;
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
         operation->setLastArgument(result, resultType);

      if (!operation->isFinished()) {
         context.addErrorMessage(STG::SString("unexpected end of expression"));
         return PPEnhancedObject();
      };
      result = apply(*operation, resultType, context);
      if (!result.isValid())
         return result;
   };
   argumentType = resultType;
   return result;
}

STG::Lexer::Base::ReadResult
IOScalar::readToken(Parser::State& state, Parser::Arguments& arguments) {
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
         AssumeCondition(!ppveElement.isValid())
         {  AbstractToken token = arguments.lexer().queryToken();
            switch (token.getType()) {
               case AbstractToken::TIdentifier:
                  {  const IdentifierToken& token = (const IdentifierToken&)
                        arguments.lexer().getContentToken();
                     PPEnhancedObject element;
                     ArgumentType elementType = ATUndefined;
                     if (!arguments.retrieveIdentifier(token.identifier(), element, elementType)) {
                        DefineAddError(STG::SString("unknown identifier \"")
                           .cat(token.identifier()).cat("\" in expression"));
                        DefineReduceAndParse
                     };
                     if (state.point() == DBegin) {
                        if (element.isValid()) {
                           ppveElement = element;
                           atType = elementType;
                        }
                        else {
                           DefineAddError(STG::SString("unassigned identifier \"")
                              .cat(token.identifier()).cat("\" in expression"));
                           DefineReduceAndParse
                        };
                     }
                     else
                        state.getUnionResult((OperatorStack*) nullptr).pushLastArgument(
                           element, elementType);
                     DefineGotoCaseWithIncrement(DAfterPrimary - DBegin, LAfterPrimary)
                  };
               case AbstractToken::TNumber:
                  {  const NumberToken& token = (const NumberToken&)
                        arguments.lexer().getContentToken();
                     PPEnhancedObject element;
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
                           element = arguments.createConstantBit(!val.isZero());
                           resultType = ATBit;
                        }
                        else {
                           val.adjustBitSize(token.getSize());
                           element = arguments.createConstantMultiBit(&val[0], token.getSize());
                           resultType = ATMultiBit;
                        };
                     }
                     else {
                        AssumeCondition(token.isFloatingPoint())
                        typedef Floating::VirtualExact::TMultiFloatElement<
                           Constant::VirtualElement,
                           Floating::Implementation::MultiFloatElement> ConstantResult;
                        int sizeExponent = token.getFirstSize();
                        int sizeMantissa = token.getSecondSize();
                        if (sizeExponent == 0 || sizeMantissa == 0) {
                           DefineAddError(STG::SString("exponent and mantissa should have a positive size in floating point number"));
                           DefineReduceAndParse
                        };
                        ConstantResult val(ConstantResult::InitFloat()
                              .setSizeExponent(token.getFirstSize())
                              .setSizeMantissa(token.getSecondSize()));
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
                        val.simplementation().read(in, format);
                        element = arguments.createConstantFloating(val);
                        resultType = ATMultiFloat;
                     }
                     if (state.point() == DBegin) {
                        ppveElement = element;
                        atType = resultType;
                     }
                     else
                        state.getUnionResult((OperatorStack*) nullptr)
                           .pushLastArgument(element, resultType);
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
                              PPEnhancedObject element = arguments.createTopMultiBit(((const SizedKeywordToken&)
                                    arguments.lexer().getContentToken()).getSize(), false);
                              if (state.point() == DBegin) {
                                 ppveElement = element;
                                 atType = ATMultiBit;
                              }
                              else
                                 state.getUnionResult((OperatorStack*) nullptr)
                                       .pushLastArgument(element, ATMultiBit);
                           }
                           else {
                              AssumeCondition(dynamic_cast<const FloatSizedKeywordToken*>(&arguments.lexer().getContentToken()))
                              const auto& token = (const FloatSizedKeywordToken&) arguments.lexer().getContentToken();
                              PPEnhancedObject element = arguments.createTopFloating(
                                    token.getSizeExponent(), token.getSizeMantissa(), false);
                              if (state.point() == DBegin) {
                                 ppveElement = element;
                                 atType = ATMultiFloat;
                              }
                              else
                                 state.getUnionResult((OperatorStack*) nullptr)
                                       .pushLastArgument(element, ATMultiFloat);
                           };
                        };
                        DefineGotoCaseWithIncrement(DAfterPrimary - DBegin, LAfterPrimary)
                     case KeywordToken::TTopBit:
                        {  PPEnhancedObject element = arguments.createTopBit(false);
                           if (state.point() == DBegin) {
                              ppveElement = element;
                              atType = ATBit;
                           }
                           else
                              state.getUnionResult((OperatorStack*) nullptr)
                                    .pushLastArgument(element, ATBit);
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
                              PPEnhancedObject element = arguments.createTopMultiBit(((const SizedKeywordToken&)
                                    arguments.lexer().getContentToken()).getSize(), true);
                              if (state.point() == DBegin) {
                                 ppveElement = element;
                                 atType = ATMultiBit;
                              }
                              else
                                 state.getUnionResult((OperatorStack*) nullptr)
                                       .pushLastArgument(element, ATMultiBit);
                           }
                           else {
                              AssumeCondition(dynamic_cast<const FloatSizedKeywordToken*>(&arguments.lexer().getContentToken()))
                              const auto& token = (const FloatSizedKeywordToken&) arguments.lexer().getContentToken();
                              PPEnhancedObject element = arguments.createTopFloating(
                                    token.getSizeExponent(), token.getSizeMantissa(), true);
                              if (state.point() == DBegin) {
                                 ppveElement = element;
                                 atType = ATMultiFloat;
                              }
                              else
                                 state.getUnionResult((OperatorStack*) nullptr)
                                       .pushLastArgument(element, ATMultiFloat);
                           };
                        };
                        DefineGotoCaseWithIncrement(DAfterPrimary - DBegin, LAfterPrimary)
                     case KeywordToken::TTopBitSymbolic:
                        {  PPEnhancedObject element = arguments.createTopBit(true);
                           if (state.point() == DBegin) {
                              ppveElement = element;
                              atType = ATBit;
                           }
                           else
                              state.getUnionResult((OperatorStack*) nullptr)
                                    .pushLastArgument(element, ATBit);
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
                           IOScalar* subElement = new IOScalar;
                           operatorStack->absorbSubElement(subElement);
                           operatorStack->pushPrefixBinaryOperator(Operator::TInterval);
                           DefineShift(Interval, *subElement, &IOScalar::readToken,
                                 (Parser::State::UnionResult<IOScalar, OperatorStack>*) nullptr);
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
         AssumeCondition(state.point() == DAfterPrimary ? ppveElement.isValid() : !ppveElement.isValid())
         if (arguments.lexer().hasToken()) {
            AbstractToken token = arguments.lexer().queryToken();
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
                           .setFirstArgument(ppveElement, atType)
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
                     PPEnhancedObject element = operatorStack->clear(typeResult, arguments);
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
                        operatorStack->pushLastArgument(ppveElement, atType);
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
                           PPEnhancedObject resultElement
                              = operatorStack->apply(*lastOperator, resultType, arguments); 
                           if (!resultElement.isValid()) {
                              if (arguments.doesStopAfterTooManyErrors())
                                 return RRFinished;
                              DefineReduce
                           };
                           operatorStack->freeLast();
                           isStackEmpty = operatorStack->isEmpty();
                           if (isStackEmpty) {
                              ppveElement = resultElement;
                              atType = resultType;
                              DefineReduceAndParse
                           };
                           lastOperator = &operatorStack->getSLast();
                           lastOperator->setLastArgument(resultElement, resultType);
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
                        .setFirstArgument(ppveElement, atType)
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
         if (!ppveElement.isValid()) {
            AssumeCondition(state.point() == DAfterPrimaryStack)
            OperatorStack operatorStack = std::move(state.getUnionResult((OperatorStack*) nullptr));
            state.freeUnionResult((OperatorStack*) nullptr);
            ppveElement = operatorStack.clear(atType, arguments);
            if (!ppveElement.isValid() && arguments.doesStopAfterTooManyErrors())
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
         {  IOScalar* subElement;
            if (state.point() == DBegin) {
               state.absorbUnionResult(IOScalar());
               subElement = &state.getUnionResult((IOScalar*) nullptr);
            }
            else
               state.getUnionResult((OperatorStack*) nullptr).absorbSubElement(
                     subElement = new IOScalar());
            DefineShiftAndParseWithIncrement(DParen - DAfterParen, LParen, *subElement,
                  &IOScalar::readToken, (Parser::State::UnionResult<IOScalar, OperatorStack>*) nullptr);
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
         {  PNT::AutoPointer<IOScalar> subElement;
            if (state.point() == DParen) {
               subElement.absorbElement(new IOScalar(std::move(
                           state.getUnionResult((IOScalar*) nullptr))));
               state.freeUnionResult((IOScalar*) nullptr);
            }
            else
               subElement.absorbElement(state.getUnionResult((OperatorStack*) nullptr)
                     .extractSubElement());
            AbstractToken token = arguments.lexer().queryToken();
            if (subElement->ppveElement.isValid()) {
               if (state.point() == DParen) {
                  ppveElement = subElement->ppveElement;
                  atType = subElement->atType;
               }
               else
                  state.getUnionResult((OperatorStack*) nullptr).pushLastArgument(
                     subElement->ppveElement, subElement->atType);

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
            PNT::AutoPointer<IOScalar> subElement(operatorStack
                  ->extractSubElement(), PNT::Pointer::Init());
            Operator* interval = &operatorStack->getSLast();
            AssumeCondition(interval->getType() == Operator::TInterval)
            AbstractToken token = arguments.lexer().queryToken();
            if (subElement->ppveElement.isValid()) {
               operatorStack->pushLastArgument(
                  subElement->ppveElement, subElement->atType);
               if (interval->getLeftSubExpressions() == 1) {
                  if (token.getType() == AbstractToken::TOperatorPunctuator
                        && ((const OperatorPunctuatorToken&) token).getType()
                           == OperatorPunctuatorToken::TComma) {
                     operatorStack->absorbSubElement(subElement.extractElement());
                     DefineShift(Interval, operatorStack->getSubElement(),
                           &IOScalar::readToken, (Parser::State::UnionResult<IOScalar, OperatorStack>*) nullptr);
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
            PPEnhancedObject resultElement = operatorStack
               ->applyInterval(*interval, resultType, arguments);
            operatorStack->freeLast();
            interval = nullptr;
            if (operatorStack->isEmpty()) {
               state.freeUnionResult((OperatorStack*) nullptr);
               operatorStack = nullptr;
               ppveElement = resultElement;
               atType = resultType;
               if (!hasReadToken)
                  DefineGotoCaseAndParse(AfterPrimary)
               else
                  DefineGotoCase(AfterPrimary)
            }
            else {
               operatorStack->pushLastArgument(resultElement, resultType);
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
            IOScalar* subElement;
            if (state.point() == DSet) {
               state.absorbUnionResult(IOScalar());
               subElement = &state.getUnionResult((IOScalar*) nullptr);
            }
            else
               state.getUnionResult((OperatorStack*) nullptr)
                  .absorbSubElement(subElement = new IOScalar());
            DefineShiftAndParseWithIncrement(DFirstSetElement - DSet, LFirstSetElement,
                  *subElement, &IOScalar::readToken, (Parser::State::UnionResult<IOScalar, OperatorStack>*) nullptr)
         };
      DefineParseCase(FirstSetElement)
      DefineLightParseCase(FirstSetElementStack)
         {  PNT::AutoPointer<IOScalar> subElement;
            if (state.point() == DFirstSetElement) {
               subElement.absorbElement(new IOScalar(std::move(
                           state.getUnionResult((IOScalar*) nullptr))));
               state.freeUnionResult((IOScalar*) nullptr);
            }
            else
               subElement.absorbElement(state.getUnionResult(
                        (OperatorStack*) nullptr).extractSubElement());
            AbstractToken token = arguments.lexer().queryToken();
            if (subElement->ppveElement.isValid()) {
               ArgumentType subResultType = subElement->atType;
               PPEnhancedObject element = arguments.newDisjunction(subResultType,
                     subElement->ppveElement, false);
               if (state.point() == DFirstSetElement) {
                  ppveElement = element;
                  atType = subResultType;
               }
               else
                  state.getUnionResult((OperatorStack*) nullptr).pushLastArgument(
                     element, subResultType);

               if (token.getType() == AbstractToken::TOperatorPunctuator) {
                  auto type = ((const OperatorPunctuatorToken&) token).getType();
                  if (type == OperatorPunctuatorToken::TCloseBrace)
                     DefineGotoCaseWithIncrement(DAfterSet - DFirstSetElement, LAfterSet)
                  if (type == OperatorPunctuatorToken::TComma) {
                     IOScalar* element;
                     if (state.point() == DFirstSetElement) {
                        state.absorbUnionResult(std::move(*subElement));
                        element = &state.getUnionResult((IOScalar*) nullptr);
                     }
                     else
                        state.getUnionResult((OperatorStack*) nullptr)
                           .absorbSubElement(element = subElement.extractElement());
                     DefineShiftWithIncrement(DSetElement - DFirstSetElement,
                        LSetElement, *element, &IOScalar::readToken, (Parser::State::UnionResult<IOScalar, OperatorStack>*) nullptr)
                  }
               }
            };
            DefineAddError(STG::SString("'}' was expected"
               " during the parsing of a set"));
            DefineGotoCaseWithIncrement(DEndSet - DFirstSetElement, LEndSet)
         };
      DefineParseCase(SetElement)
      DefineLightParseCase(SetElementStack)
         {  PNT::AutoPointer<IOScalar> subElement;
            EnhancedObject* disjunction;
            ArgumentType resultType;
            if (state.point() == DSetElement) {
               subElement.absorbElement(new IOScalar(std::move(
                           state.getUnionResult((IOScalar*) nullptr))));
               state.freeUnionResult((IOScalar*) nullptr);
               disjunction = &*ppveElement;
               resultType = atType;
            }
            else {
               OperatorStack* operatorStack = &state.getUnionResult((OperatorStack*) nullptr);
               subElement.absorbElement(operatorStack->extractSubElement());
               disjunction = &operatorStack->getLastArgument(resultType);
            }
            AbstractToken token = arguments.lexer().queryToken();
            if (subElement->ppveElement.isValid()) {
               ArgumentType localSubType = subElement->atType;
               if (localSubType != resultType)
                  DefineAddError(STG::SString("set has incompatible types for elements"))
               else
                  arguments.addElementInDisjunction(subElement->atType, subElement->ppveElement,
                        *disjunction);
               if (token.getType() == AbstractToken::TOperatorPunctuator) {
                  auto type = ((const OperatorPunctuatorToken&) token).getType();
                  if (type == OperatorPunctuatorToken::TCloseBrace)
                     DefineGotoCaseWithIncrement(DAfterSet - DSetElement, LAfterSet)
                  if (type == OperatorPunctuatorToken::TComma) {
                     IOScalar* element;
                     if (state.point() == DSetElement) {
                        state.absorbUnionResult(std::move(*subElement));
                        element = &state.getUnionResult((IOScalar*) nullptr);
                     }
                     else
                        state.getUnionResult((OperatorStack*) nullptr)
                           .absorbSubElement(element = subElement.extractElement());
                     DefineShiftWithIncrement(DSetElement - DSetElement,
                        LSetElement, *element, &IOScalar::readToken, (Parser::State::UnionResult<IOScalar, OperatorStack>*) nullptr)
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
         if (arguments.lexer().hasToken()) {
            AbstractToken token = arguments.lexer().queryToken();
            if (token.getType() == AbstractToken::TOperatorPunctuator
                  && ((const OperatorPunctuatorToken&) token).getType()
                     == OperatorPunctuatorToken::TOpenParen) {
               if (((state.point() - DAfterExtendZero) & 0x1) == 0)
                  state.absorbUnionResult(OperatorStack());
               OperatorStack* operatorStack = &state.getUnionResult((OperatorStack*) nullptr);
               IOScalar* subElement = new IOScalar();
               operatorStack->absorbSubElement(subElement);
               int point = state.point();
               Operator::TypeCallFunction typeCall =
                  ((point >= DAfterExtendZero && point <= DAfterExtendZeroStack)
                     || (point >= DAfterExtendZeroSymbolic && point <= DAfterExtendZeroSymbolicStack))
                  ? Operator::TCFExtendZero : Operator::TCFExtendSign;
               auto& operation = operatorStack->pushCallFunction(typeCall, 1);
               if (point >= DAfterExtendZeroSymbolic)
                  operation.setSymbolic();
               DefineShift(InExtensionFunction, *subElement, &IOScalar::readToken,
                     (Parser::State::UnionResult<IOScalar, OperatorStack>*) nullptr);
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
         if (arguments.lexer().hasToken()) {
            AbstractToken token = arguments.lexer().queryToken();
            if (token.getType() == AbstractToken::TOperatorPunctuator
                  && ((const OperatorPunctuatorToken&) token).getType()
                     == OperatorPunctuatorToken::TOpenParen) {
               if (((state.point() - DAfterConcat) & 0x1) == 0)
                  state.absorbUnionResult(OperatorStack());
               OperatorStack* operatorStack = &state.getUnionResult((OperatorStack*) nullptr);
               IOScalar* subElement = new IOScalar();
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
               DefineShift(InFunction, *subElement, &IOScalar::readToken,
                     (Parser::State::UnionResult<IOScalar, OperatorStack>*) nullptr);
            }
         };
         if (!arguments.addErrorMessage(STG::SString("binary function construction expects an '('")))
            return RRFinished;
         DefineReduceAndParse
      DefineParseCase(AfterReduce)
      DefineLightParseCase(AfterReduceStack)
      DefineParseCase(AfterReduceSymbolic)
      DefineLightParseCase(AfterReduceSymbolicStack)
         if (arguments.lexer().hasToken()) {
            AbstractToken token = arguments.lexer().queryToken();
            if (token.getType() == AbstractToken::TOperatorPunctuator
                  && ((const OperatorPunctuatorToken&) token).getType()
                     == OperatorPunctuatorToken::TOpenParen) {
               if (state.point() == DAfterReduce || state.point() == DAfterReduceSymbolic)
                  state.absorbUnionResult(OperatorStack());
               OperatorStack* operatorStack = &state.getUnionResult((OperatorStack*) nullptr);
               IOScalar* subElement = new IOScalar();
               operatorStack->absorbSubElement(subElement);
               auto& operation = operatorStack->pushCallFunction(Operator::TCFReduce, 1);
               if (state.point() >= DAfterReduceSymbolic)
                  operation.setSymbolic();
               DefineShift(InReductionFunction, *subElement, &IOScalar::readToken,
                     (Parser::State::UnionResult<IOScalar, OperatorStack>*) nullptr);
            }
         };
         if (!arguments.addErrorMessage(STG::SString("reduce construction expects an '('")))
            return RRFinished;
         DefineReduceAndParse
      DefineParseCase(AfterBitSet)
      DefineLightParseCase(AfterBitSetStack)
      DefineParseCase(AfterBitSetSymbolic)
      DefineLightParseCase(AfterBitSetSymbolicStack)
         if (arguments.lexer().hasToken()) {
            AbstractToken token = arguments.lexer().queryToken();
            if (token.getType() == AbstractToken::TOperatorPunctuator
                  && ((const OperatorPunctuatorToken&) token).getType()
                     == OperatorPunctuatorToken::TOpenParen) {
               if (state.point() == DAfterBitSet || state.point() == DAfterBitSetSymbolic)
                  state.absorbUnionResult(OperatorStack());
               OperatorStack* operatorStack = &state.getUnionResult((OperatorStack*) nullptr);
               IOScalar* subElement = new IOScalar();
               operatorStack->absorbSubElement(subElement);
               auto& operation = operatorStack->pushCallFunction(Operator::TCFBitSet, 2);
               if (state.point() >= DAfterBitSetSymbolic)
                  operation.setSymbolic();
               DefineShift(InBitSetFunction, *subElement, &IOScalar::readToken,
                     (Parser::State::UnionResult<IOScalar, OperatorStack>*) nullptr);
            }
         };
         if (!arguments.addErrorMessage(STG::SString("bitset construction expects an '('")))
            return RRFinished;
         DefineReduceAndParse
      DefineParseCase(AfterShiftBit)
      DefineLightParseCase(AfterShiftBitStack)
      DefineParseCase(AfterShiftBitSymbolic)
      DefineLightParseCase(AfterShiftBitSymbolicStack)
         if (arguments.lexer().hasToken()) {
            AbstractToken token = arguments.lexer().queryToken();
            if (token.getType() == AbstractToken::TOperatorPunctuator
                  && ((const OperatorPunctuatorToken&) token).getType()
                     == OperatorPunctuatorToken::TOpenParen) {
               if (state.point() == DAfterShiftBit || state.point() == DAfterShiftBitSymbolic)
                  state.absorbUnionResult(OperatorStack());
               OperatorStack* operatorStack = &state.getUnionResult((OperatorStack*) nullptr);
               IOScalar* subElement = new IOScalar();
               operatorStack->absorbSubElement(subElement);
               auto& operation = operatorStack->pushCallFunction(Operator::TCFShiftBit, 1);
               if (state.point() >= DAfterShiftBitSymbolic)
                  operation.setSymbolic();
               DefineShift(InShiftBitFunction, *subElement, &IOScalar::readToken,
                     (Parser::State::UnionResult<IOScalar, OperatorStack>*) nullptr);
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
         if (arguments.lexer().hasToken()) {
            AbstractToken token = arguments.lexer().queryToken();
            if (token.getType() == AbstractToken::TOperatorPunctuator
                  && ((const OperatorPunctuatorToken&) token).getType()
                     == OperatorPunctuatorToken::TOpenParen) {
               if (((state.point() - DAfterIncSigned) & 0x1) == 0)
                  state.absorbUnionResult(OperatorStack());
               OperatorStack* operatorStack = &state.getUnionResult((OperatorStack*) nullptr);
               IOScalar* subElement = new IOScalar();
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
               DefineShift(InFunction, *subElement, &IOScalar::readToken,
                     (Parser::State::UnionResult<IOScalar, OperatorStack>*) nullptr);
            }
         };
         if (!arguments.addErrorMessage(STG::SString("dec construction expects an '('")))
            return RRFinished;
         DefineReduceAndParse
      DefineParseCase(InFunction)
         {  OperatorStack* operatorStack = &state.getUnionResult((OperatorStack*) nullptr);
            PNT::AutoPointer<IOScalar> subElement(operatorStack
                  ->extractSubElement(), PNT::Pointer::Init());
            Operator* call = &operatorStack->getSLast();
            AssumeCondition(call->getType() == Operator::TCall)
            AbstractToken token = arguments.lexer().queryToken();
            if (subElement->ppveElement.isValid()) {
               operatorStack->pushLastArgument(
                  subElement->ppveElement, subElement->atType);
               if (call->getLeftSubExpressions() > 0) {
                  if (token.getType() == AbstractToken::TOperatorPunctuator
                        && ((const OperatorPunctuatorToken&) token).getType()
                           == OperatorPunctuatorToken::TComma) {
                     operatorStack->absorbSubElement(subElement.extractElement());
                     DefineShift(InFunction, operatorStack->getSubElement(),
                           &IOScalar::readToken, (Parser::State::UnionResult<IOScalar, OperatorStack>*) nullptr);
                  };
               }
               else {
                  if (token.getType() == AbstractToken::TOperatorPunctuator
                        && ((const OperatorPunctuatorToken&) token).getType()
                           == OperatorPunctuatorToken::TCloseParen) {
                     ArgumentType resultType = ATUndefined;
                     PPEnhancedObject resultElement = operatorStack
                        ->applyCallFunction(*call, resultType, arguments);
                     operatorStack->freeLast();
                     call = nullptr;
                     if (operatorStack->isEmpty()) {
                        state.freeUnionResult((OperatorStack*) nullptr);
                        operatorStack = nullptr;
                        ppveElement = resultElement;
                        atType = resultType;
                        DefineGotoCase(AfterPrimary)
                     }
                     else {
                        operatorStack->pushLastArgument(resultElement, resultType);
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
            PNT::AutoPointer<IOScalar> subElement(operatorStack
                  ->extractSubElement(), PNT::Pointer::Init());
            Operator* call = &operatorStack->getSLast();
            AssumeCondition(call->getType() == Operator::TCall)
            AbstractToken token = arguments.lexer().queryToken();
            if (subElement->ppveElement.isValid()) {
               operatorStack->pushLastArgument(
                  subElement->ppveElement, subElement->atType);
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
            PNT::AutoPointer<IOScalar> subElement(operatorStack
                  ->extractSubElement(), PNT::Pointer::Init());
            Operator* call = &operatorStack->getSLast();
            AssumeCondition(call->getType() == Operator::TCall
                  && call->getTypeCall() == Operator::TCFBitSet)
            AbstractToken token = arguments.lexer().queryToken();
            if (subElement->ppveElement.isValid()) {
               operatorStack->pushLastArgument(
                  subElement->ppveElement, subElement->atType);
               if (call->getLeftSubExpressions() > 0) {
                  if (token.getType() == AbstractToken::TOperatorPunctuator
                        && ((const OperatorPunctuatorToken&) token).getType()
                           == OperatorPunctuatorToken::TComma) {
                     operatorStack->absorbSubElement(subElement.extractElement());
                     DefineShift(InBitSetFunction, operatorStack->getSubElement(),
                           &IOScalar::readToken, (Parser::State::UnionResult<IOScalar, OperatorStack>*) nullptr);
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
            PNT::AutoPointer<IOScalar> subElement(operatorStack
                  ->extractSubElement(), PNT::Pointer::Init());
            Operator* call = &operatorStack->getSLast();
            AssumeCondition(call->getType() == Operator::TCall
                  && call->getTypeCall() == Operator::TCFReduce)
            AbstractToken token = arguments.lexer().queryToken();
            if (subElement->ppveElement.isValid()) {
               operatorStack->pushLastArgument(
                  subElement->ppveElement, subElement->atType);
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
               PPEnhancedObject resultElement = operatorStack
                  ->applyCallFunction(*call, resultType, arguments);
               operatorStack->freeLast();
               call = nullptr;
               if (operatorStack->isEmpty()) {
                  state.freeUnionResult((OperatorStack*) nullptr);
                  operatorStack = nullptr;
                  ppveElement = resultElement;
                  atType = resultType;
                  DefineGotoCase(AfterPrimary)
               }
               else {
                  operatorStack->pushLastArgument(resultElement, resultType);
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
ConstraintIOScalar::apply(Parser::Arguments& arguments) {
   if (!ceResult.isValid() && neSecond.hasName()) {
      if ((tIntApplyType == MultiBit::Operation::TMinusSignedAssign)
            || (tIntApplyType == MultiBit::Operation::TMinusUnsignedAssign)) {
         tIntApplyType = MultiBit::Operation::TOppositeSignedAssign;
         ceResult = neSecond.getValue();
         neSecond.clear();
      }
      else if ((tIntApplyType == MultiBit::Operation::TPlusSignedAssign)
            || (tIntApplyType == MultiBit::Operation::TPlusUnsignedAssign)) {
         ceResult = neSecond.getValue();
         neSecond.clear();
         neFirst.getSValue().selement() = arguments.intersectConstraint(
               neFirst.getValue().getArgumentType(), neFirst.getSValue().selement(),
               ceResult.getArgumentType(), ceResult.getElement(), false);
         return true;
      };
      if (tFloatApplyType == Floating::Operation::TMinusAssign) {
         tFloatApplyType = Floating::Operation::TOppositeAssign;
         ceResult = neSecond.getValue();
         neSecond.clear();
      }
      else if (tFloatApplyType == Floating::Operation::TPlusAssign) {
         ceResult = neSecond.getValue();
         neSecond.clear();
         neFirst.getSValue().selement() = arguments.intersectConstraint(
               neFirst.getValue().getArgumentType(), neFirst.getSValue().selement(),
               ceResult.getArgumentType(), ceResult.getElement(), false);
         return true;
      };
   }
   if (!ceResult.isValid())
      return false;
   if (!neFirst.hasName())
      return false;
   if (neSecond.hasName()) {
      if (neSecond.getValue().getArgumentType() != neFirst.getValue().getArgumentType()) {
         arguments.addErrorMessage(STG::SString("arguments have different numerical types"));
         return false;
      };
      if (neFirst.getValue().getArgumentType() == IOScalar::ATMultiBit) {
         if (tIntApplyType == MultiBit::Operation::TUndefined) {
            arguments.addErrorMessage(STG::SString("arguments have numerical types not supported by the operation"));
            return false;
         };
         neFirst.getSValue().selement() = arguments.constraint(ceResult.getArgumentType(),
               ceResult.getElement(), neFirst.getValue().getArgumentType(),
               neFirst.getSValue().selement(),
               apoIntApplyType.isValid() ? *apoIntApplyType
                  : MultiBit::Operation().setType(tIntApplyType).setArgumentsNumber(1),
               neSecond.getValue().getArgumentType(), neSecond.getSValue().selement(),
               fIsSymbolic);
      }
      else if (neFirst.getValue().getArgumentType() == IOScalar::ATBit) {
         if (tBitApplyType == Bit::Operation::TUndefined) {
            arguments.addErrorMessage(STG::SString("arguments have bit types not supported by the operation"));
            return false;
         };
         neFirst.getSValue().selement() = arguments.constraint(ceResult.getArgumentType(),
               ceResult.getElement(), neFirst.getValue().getArgumentType(),
               neFirst.getSValue().selement(),
               apoBitApplyType.isValid() ? *apoBitApplyType
                  : Bit::Operation().setType(tBitApplyType).setArgumentsNumber(1),
               neSecond.getValue().getArgumentType(), neSecond.getSValue().selement(),
               fIsSymbolic);
      }
      else if (neFirst.getValue().getArgumentType() == IOScalar::ATMultiFloat) {
         if (tFloatApplyType == Floating::Operation::TUndefined) {
            arguments.addErrorMessage(STG::SString("arguments have numerical types not supported by the operation"));
            return false;
         };
         neFirst.getSValue().selement() = arguments.constraint(ceResult.getArgumentType(),
               ceResult.getElement(), neFirst.getValue().getArgumentType(),
               neFirst.getSValue().selement(),
               apoFloatApplyType.isValid() ? *apoFloatApplyType
                  : Floating::Operation().setType(tFloatApplyType).setArgumentsNumber(1),
               neSecond.getValue().getArgumentType(), neSecond.getSValue().selement(),
               fIsSymbolic);
      }
      else
         {  AssumeUncalled }
   }
   else {
      if (neFirst.getValue().getArgumentType() == IOScalar::ATMultiBit) {
         if (tIntApplyType == MultiBit::Operation::TUndefined) {
            arguments.addErrorMessage(STG::SString("argument has a numerical type not supported by the operation"));
            return false;
         };
         neFirst.getSValue().selement() = arguments.constraint(ceResult.getArgumentType(),
               ceResult.getElement(), neFirst.getValue().getArgumentType(),
               neFirst.getSValue().selement(),
               apoIntApplyType.isValid() ? *apoIntApplyType
                  : MultiBit::Operation().setType(tIntApplyType).setArgumentsNumber(0),
               fIsSymbolic);
      }
      else if (neFirst.getValue().getArgumentType() == IOScalar::ATBit) {
         if (tBitApplyType == Bit::Operation::TUndefined) {
            arguments.addErrorMessage(STG::SString("argument has a bit type not supported by the operation"));
            return false;
         };
         neFirst.getSValue().selement() = arguments.constraint(ceResult.getArgumentType(),
               ceResult.getElement(), neFirst.getValue().getArgumentType(),
               neFirst.getSValue().selement(),
               apoBitApplyType.isValid() ? *apoBitApplyType
                  : Bit::Operation().setType(tBitApplyType).setArgumentsNumber(0),
               neSecond.getValue().getArgumentType(), neSecond.getSValue().selement(),
               fIsSymbolic);
      }
      else if (neFirst.getValue().getArgumentType() == IOScalar::ATMultiFloat) {
         if (tFloatApplyType == Floating::Operation::TUndefined) {
            arguments.addErrorMessage(STG::SString("argument has a numerical type not supported by the operation"));
            return false;
         };
         neFirst.getSValue().selement() = arguments.constraint(ceResult.getArgumentType(),
               ceResult.getElement(), neFirst.getValue().getArgumentType(),
               neFirst.getSValue().selement(),
               apoFloatApplyType.isValid() ? *apoFloatApplyType
                  : Floating::Operation().setType(tFloatApplyType).setArgumentsNumber(1),
               neSecond.getValue().getArgumentType(), neSecond.getSValue().selement(),
               fIsSymbolic);
      }
      else
         {  AssumeUncalled }
   };
   return true;
}

STG::Lexer::Base::ReadResult
MapOfIOScalars::readToken(Parser::State& state, Parser::Arguments& arguments) {
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
               state.absorbUnionResult(NamedIOScalar(token.identifier()));
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
            state.absorbUnionResult(NamedIOScalar(token.identifier()));
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
                  PNT::AutoPointer<NamedIOScalar> namedElement(
                        new NamedIOScalar(std::move(state.getUnionResult((NamedIOScalar*) nullptr))), PNT::Pointer::Init());
                  state.freeUnionResult((NamedIOScalar*) nullptr);
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
                  PNT::AutoPointer<NamedIOScalar> namedElement(
                        new NamedIOScalar(std::move(state.getUnionResult((NamedIOScalar*) nullptr))), PNT::Pointer::Init());
                  state.freeUnionResult((NamedIOScalar*) nullptr);
                  auto locationResult = locate(*namedElement, locateCursor, COL::VirtualCollection::RPUndefined);
                  if (locationResult) {
                     DefineAddError(STG::SString("identifier ").cat(namedElement->getName())
                        .cat("has soon been declared"))
                     DefineGotoCase(BeforeSemiColon)
                  }
                  IOScalar* element = &namedElement->getSValue();
                  add(namedElement.extractElement(),
                        locationResult.queryInsertionParameters().setFreeOnError(),
                        &locateCursor);
                  state.absorbUnionResult(Cursor(locateCursor));
                  DefineShift(EndNamedDeclaration, *element, &IOScalar::readToken,
                        (Parser::State::UnionResult<IOScalar, IOScalar::OperatorStack>*) nullptr);
               }
            };
         }
         DefineAddError(STG::SString("unknown token after var \"")
            .cat(state.getUnionResult((NamedIOScalar*) nullptr).getName()).cat('"'))
         DefineGotoCaseAndParse(BeforeSemiColon)
      DefineParseCase(AfterIdent)
         {  AbstractToken token = arguments.lexer().queryToken();
            if (token.getType() == AbstractToken::TOperatorPunctuator) {
               auto type = ((const OperatorPunctuatorToken&) token).getType();
               if (type == OperatorPunctuatorToken::TAssign) {
                  Cursor locateCursor(*this);
                  NamedIOScalar namedElement(std::move(state.getUnionResult((NamedIOScalar*) nullptr)));
                  state.freeUnionResult((NamedIOScalar*) nullptr);
                  if (locate(namedElement, locateCursor, COL::VirtualCollection::RPExact)) {
                     IOScalar* element = &locateCursor.elementSAt().getSValue();
                     element->clear();
                     state.absorbUnionResult(std::move(locateCursor));
                     DefineShift(EndNamedDeclaration, *element, &IOScalar::readToken,
                           (Parser::State::UnionResult<IOScalar, IOScalar::OperatorStack>*) nullptr);
                  }
                  else
                     DefineAddError(STG::SString("identifier ").cat(namedElement.getName())
                        .cat(" has not been declared"))
                  DefineGotoCase(EndDeclaration)
               }
            };
         }
         DefineAddError(STG::SString("expected '=' after identifier\"")
            .cat(state.getUnionResult((NamedIOScalar*) nullptr).getName()).cat('"'))
         DefineGotoCaseAndParse(BeforeSemiColon)
      DefineParseCase(AfterAssert)
         {  AbstractToken token = arguments.lexer().queryToken();
            if (token.getType() == AbstractToken::TOperatorPunctuator
                  && ((const OperatorPunctuatorToken&) token).getType()
                     == OperatorPunctuatorToken::TOpenParen) {
               state.absorbUnionResult(ConstraintIOScalar());
               DefineGotoCase(OperationAssert)
            };
         };
         DefineAddError(STG::SString("expected '(' after the assert keyword"))
         DefineGotoCase(BeforeSemiColon)
      DefineParseCase(OperationAssert)
         {  AbstractToken token = arguments.lexer().queryToken();
            if (token.getType() == AbstractToken::TOperatorPunctuator) {
               switch (((const OperatorPunctuatorToken&) token).getType()) {
                  case OperatorPunctuatorToken::TOpenParen:
                     DefineGotoCase(TypeAssert)
                  case OperatorPunctuatorToken::TPlus:
                     {  auto& constraint = state.getUnionResult((ConstraintIOScalar*) nullptr);
                        constraint.tFloatApplyType = Floating::Operation::TPlusAssign;
                        constraint.tBitApplyType = Bit::Operation::TPlusAssign;
                     }
                     DefineGotoCase(OperationExtensionAssert)
                  case OperatorPunctuatorToken::TMinus:
                     {  auto& constraint = state.getUnionResult((ConstraintIOScalar*) nullptr);
                        constraint.tIntApplyType = MultiBit::Operation::TOppositeSignedAssign;
                        constraint.tFloatApplyType = Floating::Operation::TMinusAssign;
                        constraint.tBitApplyType = Bit::Operation::TMinusAssign;
                     }
                     DefineGotoCase(OperationExtensionAssert)
                  case OperatorPunctuatorToken::TLess:
                     {  auto& constraint = state.getUnionResult((ConstraintIOScalar*) nullptr);
                        constraint.tFloatApplyType = Floating::Operation::TCompareLess;
                        constraint.tBitApplyType = Bit::Operation::TCompareLess;
                     }
                     DefineGotoCase(OperationExtensionAssert)
                  case OperatorPunctuatorToken::TLessOrEqual:
                     {  auto& constraint = state.getUnionResult((ConstraintIOScalar*) nullptr);
                        constraint.tFloatApplyType = Floating::Operation::TCompareLessOrEqual;
                        constraint.tBitApplyType = Bit::Operation::TCompareLessOrEqual;
                     }
                     DefineGotoCase(OperationExtensionAssert)
                  case OperatorPunctuatorToken::TEqual:
                     {  auto& constraint = state.getUnionResult((ConstraintIOScalar*) nullptr);
                        constraint.tIntApplyType = MultiBit::Operation::TCompareEqual;
                        constraint.tFloatApplyType = Floating::Operation::TCompareEqual;
                        constraint.tBitApplyType = Bit::Operation::TCompareEqual;
                     }
                     DefineGotoCase(AfterOperationAssert)
                  case OperatorPunctuatorToken::TDifferent:
                     {  auto& constraint = state.getUnionResult((ConstraintIOScalar*) nullptr);
                        constraint.tIntApplyType = MultiBit::Operation::TCompareDifferent;
                        constraint.tFloatApplyType = Floating::Operation::TCompareDifferent;
                        constraint.tBitApplyType = Bit::Operation::TCompareDifferent;
                     }
                     DefineGotoCase(AfterOperationAssert)
                  case OperatorPunctuatorToken::TGreaterOrEqual:
                     {  auto& constraint = state.getUnionResult((ConstraintIOScalar*) nullptr);
                        constraint.tFloatApplyType = Floating::Operation::TCompareGreaterOrEqual;
                        constraint.tBitApplyType = Bit::Operation::TCompareGreaterOrEqual;
                     }
                     DefineGotoCase(OperationExtensionAssert)
                  case OperatorPunctuatorToken::TGreater:
                     {  auto& constraint = state.getUnionResult((ConstraintIOScalar*) nullptr);
                        constraint.tFloatApplyType = Floating::Operation::TCompareGreater;
                        constraint.tBitApplyType = Bit::Operation::TCompareGreater;
                     }
                     DefineGotoCase(OperationExtensionAssert)
                  case OperatorPunctuatorToken::TTimes:
                     {  auto& constraint = state.getUnionResult((ConstraintIOScalar*) nullptr);
                        constraint.tFloatApplyType = Floating::Operation::TTimesAssign;
                     }
                     DefineGotoCase(OperationExtensionAssert)
                  case OperatorPunctuatorToken::TDivide:
                     {  auto& constraint = state.getUnionResult((ConstraintIOScalar*) nullptr);
                        constraint.tFloatApplyType = Floating::Operation::TDivideAssign;
                     }
                     DefineGotoCase(OperationExtensionAssert)
                  case OperatorPunctuatorToken::TModulo:
                     {  auto& constraint = state.getUnionResult((ConstraintIOScalar*) nullptr);
                        constraint.tIntApplyType = MultiBit::Operation::TModuloUnsignedAssign;
                     }
                     DefineGotoCase(OperationExtensionAssert)
                  case OperatorPunctuatorToken::TBitOr:
                     {  auto& constraint = state.getUnionResult((ConstraintIOScalar*) nullptr);
                        constraint.tIntApplyType = MultiBit::Operation::TBitOrAssign;
                        constraint.tBitApplyType = Bit::Operation::TOrAssign;
                     }
                     DefineGotoCase(AfterOperationAssert)
                  case OperatorPunctuatorToken::TBitAnd:
                     {  auto& constraint = state.getUnionResult((ConstraintIOScalar*) nullptr);
                        constraint.tIntApplyType = MultiBit::Operation::TBitAndAssign;
                        constraint.tBitApplyType = Bit::Operation::TAndAssign;
                     }
                     DefineGotoCase(AfterOperationAssert)
                  case OperatorPunctuatorToken::TBitXor:
                     {  auto& constraint = state.getUnionResult((ConstraintIOScalar*) nullptr);
                        constraint.tIntApplyType = MultiBit::Operation::TBitExclusiveOrAssign;
                        constraint.tBitApplyType = Bit::Operation::TExclusiveOrAssign;
                     }
                     DefineGotoCase(AfterOperationAssert)
                  case OperatorPunctuatorToken::TBitNegate:
                     {  auto& constraint = state.getUnionResult((ConstraintIOScalar*) nullptr);
                        constraint.tIntApplyType = MultiBit::Operation::TBitNegateAssign;
                        constraint.tBitApplyType = Bit::Operation::TNegateAssign;
                     }
                     DefineGotoCase(AfterOperationAssert)
                  case OperatorPunctuatorToken::TLeftShift:
                     {  auto& constraint = state.getUnionResult((ConstraintIOScalar*) nullptr);
                        constraint.tIntApplyType = MultiBit::Operation::TLeftShiftAssign;
                     }
                     DefineGotoCase(AfterOperationAssert)
                  case OperatorPunctuatorToken::TRightShift:
                     {  auto& constraint = state.getUnionResult((ConstraintIOScalar*) nullptr);
                        constraint.tIntApplyType = MultiBit::Operation::TLogicalRightShiftAssign;
                     }
                     DefineGotoCase(OperationExtensionAssert)
                  case OperatorPunctuatorToken::TLogicalAnd:
                     {  auto& constraint = state.getUnionResult((ConstraintIOScalar*) nullptr);
                        constraint.tBitApplyType = Bit::Operation::TAndAssign;
                     }
                     DefineGotoCase(AfterOperationAssert)
                  case OperatorPunctuatorToken::TLogicalOr:
                     {  auto& constraint = state.getUnionResult((ConstraintIOScalar*) nullptr);
                        constraint.tBitApplyType = Bit::Operation::TOrAssign;
                     }
                     DefineGotoCase(AfterOperationAssert)
                  case OperatorPunctuatorToken::TLogicalNegate:
                     {  auto& constraint = state.getUnionResult((ConstraintIOScalar*) nullptr);
                        constraint.tBitApplyType = Bit::Operation::TNegateAssign;
                     }
                     DefineGotoCase(AfterOperationAssert)
                  default:
                     state.freeUnionResult((ConstraintIOScalar*) nullptr);
                     break;
               };
            }
            else if (token.getType() == AbstractToken::TKeyword) {
               switch (((const KeywordToken&) token).getType()) {
                  case KeywordToken::TExtendZero: case KeywordToken::TExtendZeroSymbolic:
                     {  auto& constraint = state.getUnionResult((ConstraintIOScalar*) nullptr);
                        constraint.tIntApplyType = MultiBit::Operation::TExtendWithZero;
                        constraint.fIsSymbolic = ((const KeywordToken&) token).getType() == KeywordToken::TExtendZeroSymbolic;
                     }
                     DefineGotoCase(BeforeOperationSizeAssert)
                  case KeywordToken::TExtendSign: case KeywordToken::TExtendSignSymbolic:
                     {  auto& constraint = state.getUnionResult((ConstraintIOScalar*) nullptr);
                        constraint.tIntApplyType = MultiBit::Operation::TExtendWithSign;
                        constraint.fIsSymbolic = ((const KeywordToken&) token).getType() == KeywordToken::TExtendSignSymbolic;
                     }
                     DefineGotoCase(BeforeOperationSizeAssert)
                  case KeywordToken::TConcat: case KeywordToken::TConcatSymbolic:
                     {  auto& constraint = state.getUnionResult((ConstraintIOScalar*) nullptr);
                        constraint.tIntApplyType = MultiBit::Operation::TConcat;
                        constraint.fIsSymbolic = ((const KeywordToken&) token).getType() == KeywordToken::TConcatSymbolic;
                     }
                     DefineGotoCase(AfterOperationAssert)
                  case KeywordToken::TReduce: case KeywordToken::TReduceSymbolic:
                     {  auto& constraint = state.getUnionResult((ConstraintIOScalar*) nullptr);
                        constraint.tIntApplyType = MultiBit::Operation::TReduce;
                        constraint.fIsSymbolic = ((const KeywordToken&) token).getType() == KeywordToken::TReduceSymbolic;
                     }
                     DefineGotoCase(BeforeOperationLowBitAssert)
                  case KeywordToken::TBitSet: case KeywordToken::TBitSetSymbolic:
                     {  auto& constraint = state.getUnionResult((ConstraintIOScalar*) nullptr);
                        constraint.tIntApplyType = MultiBit::Operation::TBitSet;
                        constraint.fIsSymbolic = ((const KeywordToken&) token).getType() == KeywordToken::TBitSetSymbolic;
                     }
                     DefineGotoCase(BeforeOperationLowBitAssert)
                  case KeywordToken::TShiftBit: case KeywordToken::TShiftBitSymbolic:
                     {  auto& constraint = state.getUnionResult((ConstraintIOScalar*) nullptr);
                        constraint.tIntApplyType = MultiBit::Operation::TCastShiftBit;
                        constraint.fIsSymbolic = ((const KeywordToken&) token).getType() == KeywordToken::TShiftBitSymbolic;
                     }
                     DefineGotoCase(BeforeOperationLowBitAssert)
                  case KeywordToken::TIncSigned: case KeywordToken::TIncSignedSymbolic:
                     {  auto& constraint = state.getUnionResult((ConstraintIOScalar*) nullptr);
                        constraint.tIntApplyType = MultiBit::Operation::TNextSignedAssign;
                        constraint.fIsSymbolic = ((const KeywordToken&) token).getType() == KeywordToken::TIncSignedSymbolic;
                     }
                     DefineGotoCase(AfterOperationAssert)
                  case KeywordToken::TIncUnsigned: case KeywordToken::TIncUnsignedSymbolic:
                     {  auto& constraint = state.getUnionResult((ConstraintIOScalar*) nullptr);
                        constraint.tIntApplyType = MultiBit::Operation::TNextUnsignedAssign;
                        constraint.fIsSymbolic = ((const KeywordToken&) token).getType() == KeywordToken::TIncUnsignedSymbolic;
                     }
                     DefineGotoCase(AfterOperationAssert)
                  case KeywordToken::TDecSigned: case KeywordToken::TDecSignedSymbolic:
                     {  auto& constraint = state.getUnionResult((ConstraintIOScalar*) nullptr);
                        constraint.tIntApplyType = MultiBit::Operation::TPrevSignedAssign;
                        constraint.fIsSymbolic = ((const KeywordToken&) token).getType() == KeywordToken::TDecSignedSymbolic;
                     }
                     DefineGotoCase(AfterOperationAssert)
                  case KeywordToken::TDecUnsigned: case KeywordToken::TDecUnsignedSymbolic:
                     {  auto& constraint = state.getUnionResult((ConstraintIOScalar*) nullptr);
                        constraint.tIntApplyType = MultiBit::Operation::TPrevUnsignedAssign;
                        constraint.fIsSymbolic = ((const KeywordToken&) token).getType() == KeywordToken::TDecUnsignedSymbolic;
                     }
                     DefineGotoCase(AfterOperationAssert)
                  case KeywordToken::TMinSigned: case KeywordToken::TMinSignedSymbolic:
                     {  auto& constraint = state.getUnionResult((ConstraintIOScalar*) nullptr);
                        constraint.tIntApplyType = MultiBit::Operation::TMinSignedAssign;
                        constraint.fIsSymbolic = ((const KeywordToken&) token).getType() == KeywordToken::TMinSignedSymbolic;
                     }
                     DefineGotoCase(AfterOperationAssert)
                  case KeywordToken::TMinUnsigned: case KeywordToken::TMinUnsignedSymbolic:
                     {  auto& constraint = state.getUnionResult((ConstraintIOScalar*) nullptr);
                        constraint.tIntApplyType = MultiBit::Operation::TMinUnsignedAssign;
                        constraint.fIsSymbolic = ((const KeywordToken&) token).getType() == KeywordToken::TMinUnsignedSymbolic;
                     }
                     DefineGotoCase(AfterOperationAssert)
                  case KeywordToken::TMaxSigned: case KeywordToken::TMaxSignedSymbolic:
                     {  auto& constraint = state.getUnionResult((ConstraintIOScalar*) nullptr);
                        constraint.tIntApplyType = MultiBit::Operation::TMaxSignedAssign;
                        constraint.fIsSymbolic = ((const KeywordToken&) token).getType() == KeywordToken::TMaxSignedSymbolic;
                     }
                     DefineGotoCase(AfterOperationAssert)
                  case KeywordToken::TMaxUnsigned: case KeywordToken::TMaxUnsignedSymbolic:
                     {  auto& constraint = state.getUnionResult((ConstraintIOScalar*) nullptr);
                        constraint.tIntApplyType = MultiBit::Operation::TMaxUnsignedAssign;
                        constraint.fIsSymbolic = ((const KeywordToken&) token).getType() == KeywordToken::TMaxUnsignedSymbolic;
                     }
                     DefineGotoCase(AfterOperationAssert)
                  default:
                     state.freeUnionResult((ConstraintIOScalar*) nullptr);
                     break;
               };
            }
         };
         DefineAddError(STG::SString("expected operator as first argument of assert"))
         state.freeUnionResult((ConstraintIOScalar*) nullptr);
         DefineGotoCase(EndParen)
      DefineParseCase(OperationExtensionAssert)
         {  AbstractToken token = arguments.lexer().queryToken();
            if (token.getType() == AbstractToken::TKeyword) {
               auto type = ((const KeywordToken&) token).getType();
               if (type == KeywordToken::TUnsigned || type == KeywordToken::TSigned
                     || type == KeywordToken::TUnsignedWithSigned
                     || type == KeywordToken::TUnsignedSymbolic || type == KeywordToken::TSignedSymbolic
                     || type == KeywordToken::TUnsignedWithSignedSymbolic) {
                  auto& constraint = state.getUnionResult((ConstraintIOScalar*) nullptr);
                  if ((type == KeywordToken::TUnsignedWithSigned || type == KeywordToken::TUnsignedWithSignedSymbolic)
                        && constraint.tFloatApplyType != Floating::Operation::TPlusAssign) {
                     DefineAddError(STG::SString("the operator/function do not accept unsigned with signed qualification"))
                     state.freeUnionResult((ConstraintIOScalar*) nullptr);
                     DefineGotoCase(EndParen)
                  } 
                  switch (constraint.tFloatApplyType) {
                     case Floating::Operation::TPlusAssign:
                        if (type == KeywordToken::TUnsigned || type == KeywordToken::TUnsignedSymbolic) {
                           constraint.tIntApplyType = MultiBit::Operation::TPlusUnsignedAssign;
                           if (type == KeywordToken::TUnsignedSymbolic)
                              constraint.fIsSymbolic = true;
                        }
                        else if (type == KeywordToken::TSigned || type == KeywordToken::TSignedSymbolic) {
                           constraint.tIntApplyType = MultiBit::Operation::TPlusSignedAssign;
                           if (type == KeywordToken::TSignedSymbolic)
                              constraint.fIsSymbolic = true;
                        }
                        else {
                           constraint.tIntApplyType = MultiBit::Operation::TPlusUnsignedWithSignedAssign;
                           if (type == KeywordToken::TUnsignedWithSignedSymbolic)
                              constraint.fIsSymbolic = true;
                        }
                        DefineGotoCase(AfterOperationAssert)
                     case Floating::Operation::TMinusAssign:
                        if (type == KeywordToken::TUnsigned || type == KeywordToken::TUnsignedSymbolic) {
                           constraint.tIntApplyType = MultiBit::Operation::TMinusUnsignedAssign;
                           if (type == KeywordToken::TUnsignedSymbolic)
                              constraint.fIsSymbolic = true;
                        }
                        else {
                           constraint.tIntApplyType = MultiBit::Operation::TMinusSignedAssign;
                           if (type == KeywordToken::TSignedSymbolic)
                              constraint.fIsSymbolic = true;
                        }
                        DefineGotoCase(AfterOperationAssert)
                     case Floating::Operation::TCompareLess:
                        if (type == KeywordToken::TUnsigned || type == KeywordToken::TUnsignedSymbolic) {
                           constraint.tIntApplyType = MultiBit::Operation::TCompareLessUnsigned;
                           if (type == KeywordToken::TUnsignedSymbolic)
                              constraint.fIsSymbolic = true;
                        }
                        else {
                           constraint.tIntApplyType = MultiBit::Operation::TCompareLessSigned;
                           if (type == KeywordToken::TSignedSymbolic)
                              constraint.fIsSymbolic = true;
                        }
                        DefineGotoCase(AfterOperationAssert)
                     case Floating::Operation::TCompareLessOrEqual:
                        if (type == KeywordToken::TUnsigned || type == KeywordToken::TUnsignedSymbolic) {
                           constraint.tIntApplyType = MultiBit::Operation::TCompareLessOrEqualUnsigned;
                           if (type == KeywordToken::TUnsignedSymbolic)
                              constraint.fIsSymbolic = true;
                        }
                        else {
                           constraint.tIntApplyType = MultiBit::Operation::TCompareLessOrEqualSigned;
                           if (type == KeywordToken::TSignedSymbolic)
                              constraint.fIsSymbolic = true;
                        }
                        DefineGotoCase(AfterOperationAssert)
                     case Floating::Operation::TCompareGreaterOrEqual:
                        if (type == KeywordToken::TUnsigned  || type == KeywordToken::TUnsignedSymbolic) {
                           constraint.tIntApplyType = MultiBit::Operation::TCompareGreaterOrEqualUnsigned;
                           if (type == KeywordToken::TUnsignedSymbolic)
                              constraint.fIsSymbolic = true;
                        }
                        else {
                           constraint.tIntApplyType = MultiBit::Operation::TCompareGreaterOrEqualSigned;
                           if (type == KeywordToken::TSignedSymbolic)
                              constraint.fIsSymbolic = true;
                        }
                        DefineGotoCase(AfterOperationAssert)
                     case Floating::Operation::TCompareGreater:
                        if (type == KeywordToken::TUnsigned  || type == KeywordToken::TUnsignedSymbolic) {
                           constraint.tIntApplyType = MultiBit::Operation::TCompareGreaterUnsigned;
                           if (type == KeywordToken::TUnsignedSymbolic)
                              constraint.fIsSymbolic = true;
                        }
                        else {
                           constraint.tIntApplyType = MultiBit::Operation::TCompareGreaterSigned;
                           if (type == KeywordToken::TSignedSymbolic)
                              constraint.fIsSymbolic = true;
                        }
                        DefineGotoCase(AfterOperationAssert)
                     case Floating::Operation::TTimesAssign:
                        if (type == KeywordToken::TUnsigned  || type == KeywordToken::TUnsignedSymbolic) {
                           constraint.tIntApplyType = MultiBit::Operation::TTimesUnsignedAssign;
                           if (type == KeywordToken::TUnsignedSymbolic)
                              constraint.fIsSymbolic = true;
                        }
                        else {
                           constraint.tIntApplyType = MultiBit::Operation::TTimesSignedAssign;
                           if (type == KeywordToken::TSignedSymbolic)
                              constraint.fIsSymbolic = true;
                        }
                        DefineGotoCase(AfterOperationAssert)
                     case Floating::Operation::TDivideAssign:
                        if (type == KeywordToken::TUnsigned  || type == KeywordToken::TUnsignedSymbolic) {
                           constraint.tIntApplyType = MultiBit::Operation::TDivideUnsignedAssign;
                           if (type == KeywordToken::TUnsignedSymbolic)
                              constraint.fIsSymbolic = true;
                        }
                        else {
                           constraint.tIntApplyType = MultiBit::Operation::TDivideSignedAssign;
                           if (type == KeywordToken::TSignedSymbolic)
                              constraint.fIsSymbolic = true;
                        }
                        DefineGotoCase(AfterOperationAssert)
                     case Floating::Operation::TUndefined:
                        switch (constraint.tIntApplyType) {
                           case MultiBit::Operation::TModuloUnsignedAssign:
                              if (type == KeywordToken::TUnsigned  || type == KeywordToken::TUnsignedSymbolic) {
                                 constraint.tIntApplyType = MultiBit::Operation::TModuloUnsignedAssign;
                                 if (type == KeywordToken::TUnsignedSymbolic)
                                    constraint.fIsSymbolic = true;
                              }
                              else {
                                 constraint.tIntApplyType = MultiBit::Operation::TModuloSignedAssign;
                                 if (type == KeywordToken::TSignedSymbolic)
                                    constraint.fIsSymbolic = true;
                              }
                              DefineGotoCase(AfterOperationAssert)
                           case MultiBit::Operation::TLogicalRightShiftAssign:
                              if (type == KeywordToken::TUnsigned  || type == KeywordToken::TUnsignedSymbolic) {
                                 constraint.tIntApplyType = MultiBit::Operation::TLogicalRightShiftAssign;
                                 if (type == KeywordToken::TUnsignedSymbolic)
                                    constraint.fIsSymbolic = true;
                              }
                              else {
                                 constraint.tIntApplyType = MultiBit::Operation::TArithmeticRightShiftAssign;
                                 if (type == KeywordToken::TSignedSymbolic)
                                    constraint.fIsSymbolic = true;
                              }
                              DefineGotoCase(AfterOperationAssert)
                           default:
                              break;
                        };
                        break;
                     default:
                        break;
                  };
                  DefineAddError(STG::SString("the operator/function do not accept signed/unsigned qualification"))
                  state.freeUnionResult((ConstraintIOScalar*) nullptr);
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
         state.freeUnionResult((ConstraintIOScalar*) nullptr);
         DefineGotoCase(EndParen)
      DefineParseCase(OperationSizeAssert)
         {  AbstractToken token = arguments.lexer().queryToken();
            if (token.getType() == AbstractToken::TNumber) {
               AssumeCondition(dynamic_cast<const NumberToken*>(&arguments.lexer().getContentToken()))
               const auto& token = (const NumberToken&) arguments.lexer().getContentToken();
               if (token.hasSpecialCoding() || token.getSize() != 0) {
                  DefineAddError(STG::SString("size should be standard integer in bitset"));
                  state.freeUnionResult((ConstraintIOScalar*) nullptr);
                  DefineGotoCase(EndParen)
               }
               auto& constraint = state.getUnionResult((ConstraintIOScalar*) nullptr);
               auto* operation = new MultiBit::ExtendOperation;
               constraint.apoIntApplyType.absorbElement(operation);
               if (constraint.tIntApplyType == MultiBit::Operation::TExtendWithZero)
                  operation->setExtendWithZero(token.getContent().queryInteger());
               else if (constraint.tIntApplyType == MultiBit::Operation::TExtendWithSign)
                  operation->setExtendWithSign(token.getContent().queryInteger());
               else
                  {  AssumeUncalled }
               DefineGotoCase(AfterOperationAssert)
            }
         }
         DefineAddError(STG::SString("size was expected as first arguments of assert extension"))
         state.freeUnionResult((ConstraintIOScalar*) nullptr);
         DefineGotoCase(EndParen)
      DefineParseCase(BeforeOperationLowBitAssert)
         {  AbstractToken token = arguments.lexer().queryToken();
            if (token.getType() == AbstractToken::TOperatorPunctuator
                  && ((const OperatorPunctuatorToken&) token).getType()
                     == OperatorPunctuatorToken::TComma)
               DefineGotoCase(OperationLowBitAssert)
         };
         DefineAddError(STG::SString("',' was expected before low bit in assert reduce/bitset"))
         state.freeUnionResult((ConstraintIOScalar*) nullptr);
         DefineGotoCase(EndParen)
      DefineParseCase(OperationLowBitAssert)
         {  AbstractToken token = arguments.lexer().queryToken();
            if (token.getType() == AbstractToken::TNumber) {
               AssumeCondition(dynamic_cast<const NumberToken*>(&arguments.lexer().getContentToken()))
               const auto& token = (const NumberToken&) arguments.lexer().getContentToken();
               if (token.hasSpecialCoding() || token.getSize() != 0) {
                  DefineAddError(STG::SString("low bit should be standard integer"));
                  state.freeUnionResult((ConstraintIOScalar*) nullptr);
                  DefineGotoCase(EndParen)
               }
               auto& constraint = state.getUnionResult((ConstraintIOScalar*) nullptr);
               if (constraint.tIntApplyType == MultiBit::Operation::TReduce) {
                  auto* operation = new MultiBit::ReduceOperation;
                  constraint.apoIntApplyType.absorbElement(operation);
                  operation->setLowBit(token.getContent().queryInteger());
                  DefineGotoCase(OperationNextLowBitAssert)
               }
               else if (constraint.tIntApplyType == MultiBit::Operation::TBitSet) {
                  auto* operation = new MultiBit::BitSetOperation;
                  constraint.apoIntApplyType.absorbElement(operation);
                  operation->setLowBit(token.getContent().queryInteger());
                  DefineGotoCase(OperationNextLowBitAssert)
               }
               else if (constraint.tIntApplyType == MultiBit::Operation::TCastShiftBit) {
                  auto* operation = new MultiBit::CastShiftBitOperation(token.getContent().queryInteger());
                  constraint.apoIntApplyType.absorbElement(operation);
                  DefineGotoCase(AfterOperationAssert)
               }
               else
                  {  AssumeUncalled }
            }
         }
         DefineAddError(STG::SString("low bit was expected as first argument of assert reduce/bitset/shiftbit"))
         state.freeUnionResult((ConstraintIOScalar*) nullptr);
         DefineGotoCase(EndParen)
      DefineParseCase(OperationNextLowBitAssert)
         {  AbstractToken token = arguments.lexer().queryToken();
            if (token.getType() == AbstractToken::TOperatorPunctuator
                  && ((const OperatorPunctuatorToken&) token).getType()
                     == OperatorPunctuatorToken::TComma)
               DefineGotoCase(OperationHighBitAssert)
         };
         DefineAddError(STG::SString("',' was expected between low bit and high bit in assert reduce/bitset"))
         state.freeUnionResult((ConstraintIOScalar*) nullptr);
         DefineGotoCase(EndParen)
      DefineParseCase(OperationHighBitAssert)
         {  AbstractToken token = arguments.lexer().queryToken();
            if (token.getType() == AbstractToken::TNumber) {
               AssumeCondition(dynamic_cast<const NumberToken*>(&arguments.lexer().getContentToken()))
               const auto& token = (const NumberToken&) arguments.lexer().getContentToken();
               if (token.hasSpecialCoding() || token.getSize() != 0) {
                  DefineAddError(STG::SString("high bit should be standard integer"));
                  state.freeUnionResult((ConstraintIOScalar*) nullptr);
                  DefineGotoCase(EndParen)
               }
               auto& constraint = state.getUnionResult((ConstraintIOScalar*) nullptr);
               if (constraint.tIntApplyType == MultiBit::Operation::TReduce) {
                  AssumeCondition(dynamic_cast<const MultiBit::ReduceOperation*>(constraint.apoIntApplyType.key()))
                  auto& operation = (MultiBit::ReduceOperation&) *constraint.apoIntApplyType;
                  operation.setHighBit(token.getContent().queryInteger());
               }
               else if (constraint.tIntApplyType == MultiBit::Operation::TBitSet) {
                  AssumeCondition(dynamic_cast<const MultiBit::BitSetOperation*>(constraint.apoIntApplyType.key()))
                  auto& operation = (MultiBit::BitSetOperation&) *constraint.apoIntApplyType;
                  operation.setHighBit(token.getContent().queryInteger());
               }
               else
                  {  AssumeUncalled }
               DefineGotoCase(AfterOperationAssert)
            }
         }
         DefineAddError(STG::SString("high bit was expected as second argument of assert reduce/bitset/shiftbit"))
         state.freeUnionResult((ConstraintIOScalar*) nullptr);
         DefineGotoCase(EndParen)
      DefineParseCase(TypeAssert)
         {  AbstractToken token = arguments.lexer().queryToken();
            if (token.getType() == AbstractToken::TKeyword) {
               switch (((const KeywordToken&) token).getType()) {
                  case KeywordToken::TMulti: case KeywordToken::TMultiSymbolic:
                     {  auto& constraint = state.getUnionResult((ConstraintIOScalar*) nullptr);
                        int size = 0;
                        if (((const KeywordToken&) token).isSingleSized()) {
                           AssumeCondition(dynamic_cast<const SizedKeywordToken*>(&arguments.lexer().getContentToken()))
                           size = ((const SizedKeywordToken&) arguments.lexer().getContentToken()).getSize();
                        }
                        constraint.tIntApplyType = MultiBit::Operation::TCastMultiBit;
                        {  auto* operation = new MultiBit::CastMultiBitOperation;
                           operation->setSize(size);
                           constraint.apoIntApplyType.absorbElement(operation);
                        };
                        constraint.tBitApplyType = Bit::Operation::TCastMultiBit;
                        {  auto* operation = new Bit::CastMultiBitOperation;
                           operation->setSize(size);
                           constraint.apoBitApplyType.absorbElement(operation);
                        };
                        constraint.tFloatApplyType = Floating::Operation::TCastMultiBit;
                        {  auto* operation = new Floating::CastMultiBitOperation;
                           operation->setSize(size);
                           constraint.apoFloatApplyType.absorbElement(operation);
                        };
                        constraint.fIsSymbolic = ((const KeywordToken&) token).getType() == KeywordToken::TMultiSymbolic;
                     }
                     DefineGotoCase(AfterTypeAssert)
                  case KeywordToken::TBit: case KeywordToken::TBitSymbolic:
                     {  auto& constraint = state.getUnionResult((ConstraintIOScalar*) nullptr);
                        constraint.tIntApplyType = MultiBit::Operation::TCastBit;
                        constraint.fIsSymbolic = ((const KeywordToken&) token).getType() == KeywordToken::TBitSymbolic;
                     }
                     DefineGotoCase(AfterTypeAssert)
                  case KeywordToken::TMultiFloat: case KeywordToken::TMultiFloatSymbolic:
                     {  auto& constraint = state.getUnionResult((ConstraintIOScalar*) nullptr);
                        int sizeExponent = 0, sizeMantissa = 0;
                        if (((const KeywordToken&) token).isDoubleSized()) {
                           AssumeCondition(dynamic_cast<const FloatSizedKeywordToken*>(&arguments.lexer().getContentToken()))
                           const auto& token = (const FloatSizedKeywordToken&) arguments.lexer().getContentToken();
                           sizeExponent = token.getSizeExponent();
                           sizeMantissa = token.getSizeMantissa();
                        }
                        constraint.tIntApplyType = MultiBit::Operation::TCastMultiFloat;
                        {  auto* operation = new MultiBit::CastMultiFloatOperation;
                           operation->setSigned().setSizeExponent(sizeExponent).setSizeMantissa(sizeMantissa);
                           constraint.apoIntApplyType.absorbElement(operation);
                        };
                        constraint.tFloatApplyType = Floating::Operation::TCastMultiFloat;
                        {  auto* operation = new Floating::CastMultiFloatOperation;
                           operation->setSigned().setSizeExponent(sizeExponent).setSizeMantissa(sizeMantissa);
                           constraint.apoFloatApplyType.absorbElement(operation);
                        };
                        constraint.fIsSymbolic = ((const KeywordToken&) token).getType() == KeywordToken::TMultiFloatSymbolic;
                     }
                     DefineGotoCase(AfterTypeAssert)
                  case KeywordToken::TMultiFloatPointer: case KeywordToken::TMultiFloatPointerSymbolic:
                     {  auto& constraint = state.getUnionResult((ConstraintIOScalar*) nullptr);
                        int sizeExponent = 0, sizeMantissa = 0;
                        if (((const KeywordToken&) token).isDoubleSized()) {
                           AssumeCondition(dynamic_cast<const FloatSizedKeywordToken*>(&arguments.lexer().getContentToken()))
                           const auto& token = (const FloatSizedKeywordToken&) arguments.lexer().getContentToken();
                           sizeExponent = token.getSizeExponent();
                           sizeMantissa = token.getSizeMantissa();
                        }
                        constraint.tIntApplyType = MultiBit::Operation::TCastMultiFloatPointer;
                        {  auto* operation = new MultiBit::CastMultiFloatPointerOperation;
                           operation->setSigned().setSizeExponent(sizeExponent).setSizeMantissa(sizeMantissa);
                           constraint.apoIntApplyType.absorbElement(operation);
                        };
                        constraint.fIsSymbolic = ((const KeywordToken&) token).getType() == KeywordToken::TMultiFloatSymbolic;
                     }
                     DefineGotoCase(AfterTypeAssert)
                  default:
                     break;
               };
            }
         };
         DefineAddError(STG::SString("unknow cast operator"))
         state.freeUnionResult((ConstraintIOScalar*) nullptr);
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
         state.freeUnionResult((ConstraintIOScalar*) nullptr);
         DefineGotoCaseAndParse(DoubleEndParen)
      DefineParseCase(AfterOperationAssert)
         {  AbstractToken token = arguments.lexer().queryToken();
            if (token.getType() == AbstractToken::TOperatorPunctuator
                  && ((const OperatorPunctuatorToken&) token).getType()
                     == OperatorPunctuatorToken::TComma)
               DefineGotoCase(AssertFirstArgument)
            if (token.getType() == AbstractToken::TKeyword
                  && ((const KeywordToken&) token).getType() == KeywordToken::TSymbolic) {
               auto& constraint = state.getUnionResult((ConstraintIOScalar*) nullptr);
               if (!constraint.fIsSymbolic) {
                  constraint.fIsSymbolic = true;
                  DefineGotoCase(AfterOperationAssert)
               }
            };
         };
         DefineAddError(STG::SString("expected ',' after assert(operation ',' ...)"))
         state.freeUnionResult((ConstraintIOScalar*) nullptr);
         DefineGotoCase(EndParen)
      DefineParseCase(AssertFirstArgument)
         if (arguments.lexer().queryToken().getType() == AbstractToken::TIdentifier) {
            const IdentifierToken& token = (const IdentifierToken&)
               arguments.lexer().getContentToken();
            auto& constraint = state.getUnionResult((ConstraintIOScalar*) nullptr);
            constraint.neFirst.setName(token.identifier());
            Cursor cursor(*this);
            if (!locateKey(token.identifier(), cursor, COL::VirtualCollection::RPExact)) {
               DefineAddError(STG::SString("identifier \"").cat(token.identifier())
                  .cat("\" is not known in assert"))
               state.freeUnionResult((ConstraintIOScalar*) nullptr);
               DefineGotoCase(EndParen)
            }
            constraint.neFirst.setValue(cursor.elementAt().getValue());
            if (constraint.neFirst.getSValue().selement().isValid()) {
               arguments.specializeShare(constraint.neFirst.getValue().getArgumentType(),
                     constraint.neFirst.getValue().getElement());
            };
            DefineGotoCase(InAssert)
         }
         DefineAddError(STG::SString("expected known identifier in assert "
               "to constraint its value"))
         state.freeUnionResult((ConstraintIOScalar*) nullptr);
         DefineGotoCase(EndParen)
      DefineParseCase(InAssert)
         {  AbstractToken token = arguments.lexer().queryToken();
            if (token.getType() == AbstractToken::TOperatorPunctuator) {
               auto type = ((const OperatorPunctuatorToken&) token).getType();
               if (type == OperatorPunctuatorToken::TComma) {
                  auto& constraint = state.getUnionResult((ConstraintIOScalar*) nullptr);
                  if (!constraint.neFirst.hasName()) {
                     state.freeUnionResult((ConstraintIOScalar*) nullptr);
                     DefineGotoCase(EndParen)
                  }
                  if (!constraint.neSecond.hasName())
                     DefineGotoCase(AssertArgument)
                  if (constraint.ceResult.isValid()) {
                     DefineAddError(STG::SString("too more arguments for the operation"))
                     state.freeUnionResult((ConstraintIOScalar*) nullptr);
                     DefineGotoCase(EndParen)
                  }
                  DefineShift(InAssert, constraint.ceResult, &IOScalar::readToken,
                        (Parser::State::UnionResult<IOScalar, IOScalar::OperatorStack>*) nullptr)
               }
               else if (type == OperatorPunctuatorToken::TCloseParen) {
                  auto constraint = std::move(state.getUnionResult((ConstraintIOScalar*) nullptr));
                  state.freeUnionResult((ConstraintIOScalar*) nullptr);
                  if (constraint.apply(arguments)) {
                     Cursor cursor(*this);
                     locateKey(constraint.neFirst.getName(), cursor, COL::VirtualCollection::RPExact);
                     arguments.setAssignment(constraint.neFirst.getName(),
                           constraint.neFirst.getValue().getArgumentType(),
                           constraint.neFirst.getSValue().selement());
                     cursor.elementSAt().setValue(constraint.neFirst.getValue());
                     if (cursor.elementSAt().getSValue().selement().isValid())
                        arguments.specializeKeep(cursor.elementAt().getValue().getArgumentType(),
                              cursor.elementAt().getValue().getElement());
                     if (constraint.neSecond.hasName()) {
                        locateKey(constraint.neSecond.getName(), cursor, COL::VirtualCollection::RPExact);
                        arguments.setAssignment(constraint.neSecond.getName(),
                              constraint.neSecond.getValue().getArgumentType(),
                              constraint.neSecond.getSValue().selement());
                        cursor.elementSAt().setValue(constraint.neSecond.getValue());
                        if (cursor.elementSAt().getSValue().selement().isValid())
                           arguments.specializeKeep(cursor.elementAt().getValue().getArgumentType(),
                                 cursor.elementAt().getValue().getElement());
                     };
                     DefineGotoCase(EndDeclaration)
                  }
                  else
                     DefineGotoCase(BeforeSemiColon)
               }
            }
            if (token.getType() == AbstractToken::TKeyword
                  && ((const KeywordToken&) token).getType() == KeywordToken::TSymbolic) {
               auto& constraint = state.getUnionResult((ConstraintIOScalar*) nullptr);
               if (!constraint.fIsSymbolic) {
                  constraint.fIsSymbolic = true;
                  DefineGotoCase(AfterOperationAssert)
               }
            };
         };
         DefineAddError(STG::SString("expected ',' after assert(operation ',' ...)"))
         state.freeUnionResult((ConstraintIOScalar*) nullptr);
         DefineGotoCase(EndParen)
      DefineParseCase(AssertArgument)
         if (arguments.lexer().queryToken().getType() == AbstractToken::TIdentifier) {
            const IdentifierToken& token = (const IdentifierToken&)
               arguments.lexer().getContentToken();
            auto& constraint = state.getUnionResult((ConstraintIOScalar*) nullptr);
            if (!constraint.neSecond.hasName()) {
               constraint.neSecond.setName(token.identifier());
               Cursor cursor(*this);
               if (!locateKey(token.identifier(), cursor, COL::VirtualCollection::RPExact)) {
                  DefineAddError(STG::SString("identifier \"").cat(token.identifier())
                     .cat("\" is not known in assert"))
                  state.freeUnionResult((ConstraintIOScalar*) nullptr);
                  DefineGotoCase(EndParen)
               }
               constraint.neSecond.setValue(cursor.elementAt().getValue());
               arguments.specializeShare(constraint.neSecond.getValue().getArgumentType(),
                     constraint.neSecond.getValue().getElement());
               DefineGotoCase(InAssert)
            }
            else
               DefineShiftAndParse(InAssert, constraint.ceResult, &IOScalar::readToken,
                     (Parser::State::UnionResult<IOScalar, IOScalar::OperatorStack>*) nullptr)
         }
         else
            DefineShiftAndParse(InAssert, state.getUnionResult((ConstraintIOScalar*) nullptr)
               .ceResult, &IOScalar::readToken, (Parser::State::UnionResult<IOScalar, IOScalar::OperatorStack>*) nullptr)
      DefineParseCase(EndNamedDeclaration)
         {  AbstractToken token = arguments.lexer().queryToken();
            if (token.getType() == AbstractToken::TOperatorPunctuator
                  && ((const OperatorPunctuatorToken&) token).getType()
                     == OperatorPunctuatorToken::TSemiColon) {
               Cursor cursor = std::move(state.getUnionResult((Cursor*) nullptr));
               state.freeUnionResult((Cursor*) nullptr);
               NamedIOScalar& namedElement = cursor.elementSAt();
               arguments.setAssignment(namedElement.getName(),
                     namedElement.getValue().getArgumentType(),
                     namedElement.getSValue().selement());
               if (namedElement.getSValue().selement().isValid())
                  arguments.specializeKeep(namedElement.getValue().getArgumentType(),
                        namedElement.getValue().getElement());
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

// void
// MapOfIOScalars::_read(ISBase& in, const STG::IOObject::FormatParameters& params) {
//    const auto& parameters = (const FormatParameters&) params;
//    ApproximateAccessEnvironment scalarFactory;
//    Parser parser;
//    auto* arguments = new Parser::Argument(*this, scalarFactory);
//    parser.absorbArgument(arguments);
//    parser.state().shift(*this, &MapOfIOScalars::readToken,
//          (Parser::State::UnionResult<NamedIOScalar, Cursor, ConstraintIOScalar>*) nullptr);
//    if (parameters.isDebug())
//       arguments->setDebug();
//    if (parameters.hasNotification())
//       arguments->setNotification(parameters.getNotification(), params);
//    try {
//       parser.parse(in);
//    }
//    catch (ESPreconditionError& error) {
//       std::cerr << error << std::endl;
//    };
// }

}}} // end of namespace Analyzer::Scalar::Approximate
