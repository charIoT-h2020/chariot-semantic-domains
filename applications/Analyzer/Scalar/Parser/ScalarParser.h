/////////////////////////////////
//
// Library   : Analyzer/Scalar
// Unit      : approximate scalar elements
// File      : Parser/ScalarParser.h
// Author    : Franck Vedrine
// Copyright : CEA LIST - 2019-2020 - all rights reserved
//
// Description :
//   Definition of a parser to persist Scalar elements.
//

#pragma once

#include "Analyzer/Scalar/Parser/ScalarLexer.h"
#include "Pointer/DescentParse.h"
#include "Analyzer/Virtual/Scalar/OperationElement.h"
#include "Analyzer/Scalar/Constant/ConstantElement.h"

namespace Analyzer { namespace Scalar { namespace Approximate {

struct BaseIOScalar {
   enum ArgumentType
      {  ATUndefined, ATBit, ATMultiBit, ATMultiFloat };
};

class MapOfIOScalars;
class Parser : public STG::Lexer::Base {
  public:
   typedef Lexer::Error Error;

  public:
   class Arguments : protected BaseIOScalar  {
     private:
      Lexer lLexer;
      unsigned uLine, uColumn;
      COL::TList<Error>* pleErrorMessages;
      int uCountErrors;
      bool fDoesStopOnError;
      int uLocalStackHeight;
      MapOfIOScalars* pmeEnvironment;
      STG::SubString* pssBuffer;
      bool fDebug;
      friend class Parser;

     public:
      typedef PNT::PassPointer<EnhancedObject> PPEnhancedObject;
      typedef Floating::VirtualExact::TMultiFloatElement<Constant::VirtualElement,
            Floating::Implementation::MultiFloatElement> FloatingConstant;

      virtual bool retrieveIdentifier(const STG::SubString&, PPEnhancedObject& result,
            ArgumentType& resultType)
         {  // if (pmeEnvironment) {
            //    MapOfIOScalars::Cursor cursor(*pmeEnvironment);
            //    if (!arguments.environment().locateKey(token.identifier(),
            //          cursor, COL::VirtualCollection::RPExact))
            //       return false;
            //    const IOScalar& element = cursor.elementAt().getValue();
            //    result = createCopyElement(element.atType, element.getElement());
            //    resultType = element.atType;
            //    return true;
            // };
            return false;
         }

      // virtual PPEnhancedObject createCopyElement(ArgumentType type, EnhancedObject& source)
      //    {  return PPEnhancedObject(); }
      virtual PPEnhancedObject createConstantBit(bool value)
         {  return PPEnhancedObject(); }
      virtual PPEnhancedObject createConstantMultiBit(unsigned* value, int sizeInBits)
         {  return PPEnhancedObject(); }
      virtual PPEnhancedObject createConstantFloating(const FloatingConstant& value)
         {  return PPEnhancedObject(); }
      virtual PPEnhancedObject createTopBit(bool isSymbolic)
         {  return PPEnhancedObject(); }
      virtual PPEnhancedObject createTopMultiBit(int sizeInBits, bool isSymbolic)
         {  return PPEnhancedObject(); }
      virtual PPEnhancedObject createTopFloating(int sizeExponent, int sizeMantissa,
            bool isSymbolic)
         {  return PPEnhancedObject(); }
      virtual int getSizeInBits(const EnhancedObject& source) { return 0; }
      virtual PPEnhancedObject newInterval(ArgumentType type, PPEnhancedObject first,
            PPEnhancedObject second, bool isSigned, bool isSymbolic)
         {  return PPEnhancedObject(); }
      virtual PPEnhancedObject newDisjunction(ArgumentType type, PPEnhancedObject first,
            bool isSymbolic)
         {  return PPEnhancedObject(); }
      virtual void addElementInDisjunction(ArgumentType type, PPEnhancedObject first,
            EnhancedObject& adisjunction) {}
      virtual PPEnhancedObject newFloatInterval(ArgumentType type, PPEnhancedObject first,
            PPEnhancedObject second, bool isSymbolic)
         {  return PPEnhancedObject(); }
      virtual PPEnhancedObject applyAssign(ArgumentType firstType, PPEnhancedObject first,
            const VirtualOperation& operation, bool isSymbolic)
         {  return PPEnhancedObject(); }
      virtual PPEnhancedObject applyCastAssign(ArgumentType firstType, PPEnhancedObject first,
            ArgumentType resultType, const VirtualOperation& operation, bool isSymbolic)
         {  return PPEnhancedObject(); }
      virtual PPEnhancedObject applyAssign(ArgumentType firstType, PPEnhancedObject first,
            const VirtualOperation& operation, ArgumentType secondType, EnhancedObject& second,
            bool isSymbolic)
         {  return PPEnhancedObject(); }
      virtual PPEnhancedObject applyCastAssign(ArgumentType firstType, PPEnhancedObject first,
            ArgumentType resultType, const VirtualOperation& operation, ArgumentType secondType,
            EnhancedObject& second, bool isSymbolic)
         {  return PPEnhancedObject(); }
      virtual PPEnhancedObject applyGuardAssign(ArgumentType conditionType,
            PPEnhancedObject condition, ArgumentType firstType, PPEnhancedObject first,
            ArgumentType secondType, PPEnhancedObject second, bool isSymbolic)
         {  return PPEnhancedObject(); }
      virtual PPEnhancedObject merge(ArgumentType firstType, PPEnhancedObject first,
            ArgumentType secondType, PPEnhancedObject second, bool isSymbolic)
         {  return PPEnhancedObject(); }
      virtual PPEnhancedObject intersect(ArgumentType firstType, PPEnhancedObject first,
            ArgumentType secondType, PPEnhancedObject second, bool isSymbolic)
         {  return PPEnhancedObject(); }
      virtual void specializeShare(ArgumentType firstType, EnhancedObject& first) {}
      virtual void specializeKeep(ArgumentType firstType, EnhancedObject& first) {}
      virtual PPEnhancedObject intersectConstraint(ArgumentType firstType, PPEnhancedObject first,
            ArgumentType secondType, PPEnhancedObject second, bool isSymbolic)
         {  return PPEnhancedObject(); }
      virtual PPEnhancedObject constraint(ArgumentType resultType, EnhancedObject& result,
            ArgumentType firstType, PPEnhancedObject first, const VirtualOperation& operation,
            ArgumentType secondType, PPEnhancedObject& second, bool isSymbolic)
         {  return PPEnhancedObject(); }
      virtual PPEnhancedObject constraint(ArgumentType resultType, EnhancedObject& result,
            ArgumentType firstType, PPEnhancedObject first, const VirtualOperation& operation,
            bool isSymbolic)
         {  return PPEnhancedObject(); }
      virtual void setAssignment(const STG::SubString& name, ArgumentType valueType,
            const PPEnhancedObject& value) {}

     public:
      Arguments(MapOfIOScalars* environment = nullptr)
         :  uLine(0), uColumn(0), pleErrorMessages(&lLexer.errorList()),
            uCountErrors(0), fDoesStopOnError(false), uLocalStackHeight(0),
            pmeEnvironment(environment), pssBuffer(nullptr),
            fDebug(false)
         {}

      bool addErrorMessage(const STG::SubString& message)
         {  ++uCountErrors;
            if (pleErrorMessages && (pleErrorMessages->isEmpty()
                  || (pleErrorMessages->getLast().message != message)))
               pleErrorMessages->insertNewAtEnd(new Error(
                  uLine, uColumn, message));
           return !doesStopAfterTooManyErrors();
         }
      bool doesStopAfterTooManyErrors() const
         {  return (fDoesStopOnError || !pleErrorMessages || uCountErrors >= 10); }
      bool setToNextToken(ReadResult& result)
         {  bool booleanResult = false;
            AssumeCondition(pssBuffer)
            if (isValidRange()) {
               if (lLexer.doesNeedClear())
                  lLexer.clearToken();
               do {
                  result = lLexer.readToken(*pssBuffer, uLine, uColumn);
               } while (result == RRContinue);
               booleanResult = (result == RRHasToken);
            }
            else
               result = RRNeedChars;
            if (!booleanResult)
               clearRange();
            return booleanResult;
         }

      bool isValidRange() const
         {  return !fDebug && (uLocalStackHeight >= 0 && uLocalStackHeight <= 50)
               && (!pleErrorMessages || pleErrorMessages->isEmpty());
         }
      void shift() { ++uLocalStackHeight; }
      void reduce() { --uLocalStackHeight; }
      void clearRange() { uLocalStackHeight = 0; }
      bool hasErrors() const
         {  return pleErrorMessages && !pleErrorMessages->isEmpty(); }
      COL::TList<Error>& errors() const
         {  AssumeCondition(pleErrorMessages); return *pleErrorMessages; }
      void setDebug() { fDebug = true; }

      void fail() const;
      Lexer& lexer() { return lLexer; }
      MapOfIOScalars* senvironment() const { return pmeEnvironment; }

      void setBuffer(STG::SubString& buffer) { pssBuffer = &buffer; }
      void clearBuffer() { pssBuffer = nullptr; }
      STG::SubString& getBuffer() const { AssumeCondition(pssBuffer) return *pssBuffer; }
      typedef STG::Lexer::Base::ReadResult ResultAction;
      unsigned getLine() const { return uLine; }
      unsigned getColumn() const { return uColumn; }
   };

   typedef STG::Parser::TStateStack<Arguments> State;

  private:
   State sState;
   Arguments& aArguments;

  public:
   Parser(Arguments& arguments) : aArguments(arguments) {}

   State& state() { return sState; }
   const State& state() const { return sState; }
   const COL::TList<Error>& errors() const { return aArguments.errors(); }
   bool hasError() const { return !aArguments.errors().isEmpty(); }
   void parse(STG::IOObject::ISBase& in, bool doesEarlyEnd=false, int earlyPosition=0);
};

#define DefineParseCase(Delim)                                                 \
case D##Delim:                                                                 \
L##Delim:

#define DefineLightParseCase(Delim)                                            \
case D##Delim:

#define DefineGotoCase(Delim)                                                  \
{  state.point() = D##Delim;                                                   \
   if (arguments.setToNextToken(result)) goto L##Delim;                        \
   return result;                                                              \
}

#define DefineGotoCaseWithIncrement(Increment, LabelTarget)                    \
{  state.point() += Increment;                                                 \
   if (arguments.setToNextToken(result)) goto LabelTarget;                     \
   return result;                                                              \
}

#define DefineGotoCaseAndParse(Delim)                                          \
{  state.point() = D##Delim;                                                   \
   goto L##Delim;                                                              \
}

#define DefineGotoCaseAndParseWithIncrement(Increment, LabelTarget)            \
{  state.point() += Increment;                                                 \
   goto LabelTarget;                                                           \
}

#define DefineShift(Delim, Object, MethodPointer, ruleResult)                  \
{  arguments.shift();                                                          \
   state.point() = D##Delim;                                                   \
   state.shift(Object, MethodPointer, ruleResult);                             \
   if (arguments.setToNextToken(result)) {                                     \
      result = (Object).readToken(state, arguments);                           \
      if (result == RRContinue) {                                              \
         result = RRHasToken;                                                  \
         state.point() = D##Delim;                                             \
         goto L##Delim;                                                        \
      };                                                                       \
   };                                                                          \
   return result;                                                              \
}

#define DefineShiftFrom(Delim, Point, Object, MethodPointer, ruleResult)       \
{  arguments.shift();                                                          \
   state.point() = D##Delim;                                                   \
   state.shift(Object, MethodPointer, ruleResult);                             \
   state.point() = Point;                                                      \
   if (arguments.setToNextToken(result)) {                                     \
      result = (Object).readToken(state, arguments);                           \
      if (result == RRContinue) {                                              \
         result = RRHasToken;                                                  \
         state.point() = D##Delim;                                             \
         goto L##Delim;                                                        \
      };                                                                       \
   };                                                                          \
   return result;                                                              \
}

#define DefineShiftWithIncrement(Increment, LabelTarget, Object, MethodPointer, ruleResult)\
{  arguments.shift();                                                          \
   state.point() += Increment;                                                 \
   state.shift(Object, MethodPointer, ruleResult);                             \
   if (arguments.setToNextToken(result)) {                                     \
      result = (Object).readToken(state, arguments);                           \
      if (result == RRContinue) {                                              \
         result = RRHasToken;                                                  \
         goto LabelTarget;                                                     \
      };                                                                       \
   };                                                                          \
   return result;                                                              \
}

#define DefineShiftAndParse(Delim, Object, MethodPointer, ruleResult)          \
{  arguments.shift();                                                          \
   state.point() = D##Delim;                                                   \
   state.shift(Object, MethodPointer, ruleResult);                             \
   if (arguments.isValidRange())                                               \
      result = (Object).readToken(state, arguments);                           \
   else {                                                                      \
      result = state.parse(arguments);                                         \
      if (result != RRContinue)                                                \
         arguments.clearRange();                                               \
   };                                                                          \
   if (result == RRContinue) {                                                 \
      result = RRHasToken;                                                     \
      state.point() = D##Delim;                                                \
      goto L##Delim;                                                           \
   };                                                                          \
   return result;                                                              \
}

#define DefineShiftAndParseWithIncrement(Increment, LabelTarget, Object, MethodPointer, ruleResult)\
{  arguments.shift();                                                          \
   state.point() += Increment;                                                 \
   state.shift(Object, MethodPointer, ruleResult);                             \
   if (arguments.isValidRange())                                               \
      result = (Object).readToken(state, arguments);                           \
   else {                                                                      \
      result = state.parse(arguments);                                         \
      if (result != RRContinue)                                                \
         arguments.clearRange();                                               \
   };                                                                          \
   if (result == RRContinue) {                                                 \
      result = RRHasToken;                                                     \
      goto LabelTarget;                                                        \
   };                                                                          \
   return result;                                                              \
}

#define DefineShiftAndParseFrom(Delim, Point, Object, MethodPointer, ruleResult)\
{  state.point() = D##Delim;                                                   \
   state.shift(Object, MethodPointer, ruleResult);                             \
   state.point() = Point;                                                      \
   if (arguments.isValidRange())                                               \
      result = (Object).readToken(state, arguments);                           \
   else {                                                                      \
      result = state.parse(arguments);                                         \
      if (result != RRContinue)                                                \
         arguments.clearRange();                                               \
   };                                                                          \
   if (result == RRContinue) {                                                 \
      result = RRHasToken;                                                     \
      state.point() = D##Delim;                                                \
      goto L##Delim;                                                           \
   };                                                                          \
   return result;                                                              \
}

#define DefineReduce                                                           \
{  arguments.reduce();                                                         \
   state.reduce();                                                             \
   if (arguments.setToNextToken(result) && result == RRHasToken)               \
      result = RRContinue;                                                     \
   return result;                                                              \
}

#define DefineReduceAndParse                                                   \
{  state.reduce();                                                             \
   if (arguments.isValidRange()) {                                             \
      arguments.reduce();                                                      \
      result = RRContinue;                                                     \
   }                                                                           \
   else {                                                                      \
      result = state.parse(arguments);                                         \
      if (result == RRContinue)                                                \
         result = RRHasToken;                                                  \
      arguments.clearRange();                                                  \
   };                                                                          \
   return result;                                                              \
}

#define DefineAddError(Message)                                                \
{  if (!arguments.addErrorMessage(Message))                                    \
      return RRFinished;                                                       \
}

#define DefineAddErrorAndParse(Message)                                        \
{  if (!arguments.addErrorMessage(Message))                                    \
      return RRFinished;                                                       \
   arguments.clearRange();                                                     \
   return RRNeedChars;                                                         \
}

}}} // end of namespace Analyzer::Scalar::Approximate

