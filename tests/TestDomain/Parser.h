/////////////////////////////////
//
// Library   : unit test of Analyzer/Scalar
// Unit      : approximate scalar elements
// File      : TestGeneral/Parser.h
// Author    : Franck Vedrine
// Copyright : CEA LIST - 2019-2020 - all rights reserved
//
// Description :
//   Definition of a parser to parse the input file.
//

#ifndef TestGeneralParserH
#define TestGeneralParserH

#include "Lexer.h"
#include "Pointer/DescentParse.h"
#include "Analyzer/Scalar/AbstractDomain.h"

class MapOfElements;

class VirtualReadEventNotification {
  public:
   virtual ~VirtualReadEventNotification() {}
   virtual void notifyText(const STG::SubString& text, STG::IOObject::FormatParameters& parameters) {}
   virtual void notifyEmptyAssignment(const STG::SubString& variable,
         const STG::IOObject::FormatParameters& parameters) {}
   virtual void notifyAssignment(const STG::SubString& variable,
         const DomainElement& element, const STG::IOObject::FormatParameters& parameters) {}
};

class Parser : public STG::Lexer::Base {
  public:
   typedef Lexer::Error Error;

  public:
   class Arguments {
     private:
      Lexer lLexer;
      unsigned uLine, uColumn;
      COL::TList<Error>* pleErrorMessages;
      int uCountErrors;
      bool fDoesStopOnError;
      int uLocalStackHeight;
      MapOfElements& meEnvironment;
      DomainEvaluationEnvironment deeEvaluationEnvironment;
      DomainEvaluationEnvironment deeConstraintEnvironment;
      VirtualReadEventNotification* prenNotification;
      const STG::IOObject::FormatParameters* pfpFormatParameters;
      STG::SubString* pssBuffer;
      bool fDebug;
      friend class Parser;

     public:
      Arguments(MapOfElements& environment)
         :  uLine(0), uColumn(0), pleErrorMessages(&lLexer.errorList()),
            uCountErrors(0), fDoesStopOnError(false), uLocalStackHeight(0),
            meEnvironment(environment), deeEvaluationEnvironment{}, deeConstraintEnvironment{},
            prenNotification(nullptr), pfpFormatParameters(nullptr), pssBuffer(nullptr),
            fDebug(false)
         {  deeEvaluationEnvironment.defaultDomainType = DISUndefined; // DISShareTop;
            deeConstraintEnvironment.defaultDomainType = DISUndefined; // DISShareTop;
         }

      DomainEvaluationEnvironment& evaluationEnvironment() { return deeEvaluationEnvironment; }
      DomainEvaluationEnvironment& constraintEnvironment() { return deeConstraintEnvironment; }
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
      MapOfElements& environment() { return meEnvironment; }
      Lexer& lexer() { return lLexer; }

      void setNotification(VirtualReadEventNotification& notification,
            const STG::IOObject::FormatParameters& formatParameters)
         {  prenNotification = &notification; pfpFormatParameters = &formatParameters; }
      bool hasNotification() const { return prenNotification; }
      VirtualReadEventNotification& getNotification() const
         {  AssumeCondition(prenNotification) return *prenNotification; }
      const STG::IOObject::FormatParameters& getFormatParameters() const
         {  AssumeCondition(pfpFormatParameters) return *pfpFormatParameters; }
      void setBuffer(STG::SubString& buffer) { pssBuffer = &buffer; }
      void clearBuffer() { pssBuffer = nullptr; }
      STG::SubString& getBuffer() const { AssumeCondition(pssBuffer) return *pssBuffer; }
      typedef STG::Lexer::Base::ReadResult ResultAction;

      void clearEvaluationEnvironment()
         {  memset(&deeEvaluationEnvironment, 0, sizeof(deeEvaluationEnvironment));
            deeEvaluationEnvironment.defaultDomainType = DISUndefined; // DISShareTop;
         }
      void clearConstraintEnvironment()
         {  memset(&deeConstraintEnvironment, 0, sizeof(deeConstraintEnvironment));
            deeConstraintEnvironment.defaultDomainType = DISUndefined; // DISShareTop;
         }
   };

   typedef STG::Parser::TStateStack<Arguments> State;

  private:
   State sState;
   Arguments aArguments;

  public:
   template <class TypeObject> Parser(TypeObject& object,
         MapOfElements& environment)
      :  aArguments(environment) {}
   // {  sState.shift(object, &TypeObject::readToken); }

   void setNotification(VirtualReadEventNotification& notification,
            const STG::IOObject::FormatParameters& formatParameters)
      {  aArguments.setNotification(notification, formatParameters); }
   void setDebug() { aArguments.setDebug(); }
   State& state() { return sState; }
   const State& state() const { return sState; }
   const COL::TList<Error>& errors() const { return aArguments.errors(); }
   bool hasError() const { return !aArguments.errors().isEmpty(); }
   void parse(STG::IOObject::ISBase& in);
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

#endif // TestGeneralParserH
