/////////////////////////////////
//
// Library   : Analyzer/Scalar
// Unit      : approximate scalar elements
// File      : Parser/ScalarLexer.h
// Author    : Franck Vedrine
// Copyright : CEA LIST - 2019-2020 - all rights reserved
//
// Description :
//   Definition of a lexer to lex the input file.
//

#pragma once

#include "Analyzer/Scalar/Parser/ScalarToken.h"
#include "TString/TernaryTreeInterface.h"

namespace Analyzer { namespace Scalar { namespace Approximate {

class Lexer : public STG::Lexer::Base {
  public:
   class Error : public COL::List::Node {
     public:
      unsigned linePos;
      unsigned columnPos;
      STG::SubString message;

      Error(unsigned line, unsigned column, const STG::SubString& msg)
         :  linePos(line), columnPos(column), message(msg) {}
      Error(const Error& source) = default;

      DefineCopy(Error)
      DDefineAssign(Error)
   };

  private:
   typedef ReadResult (Lexer::*ReadPointerMethod)(STG::SubString& in,
         unsigned& line, unsigned& column, bool doesForce);
   typedef STG::Lexer::TTextBuffer<STG::SubString> TextBuffer;

   TextBuffer tbCurrentToken;
   ReadPointerMethod rpmMainState, rpmSecondaryState; 
   char chContext;
   char chExtension;
   Token tToken;
   COL::TCopyCollection<COL::TList<Error> > leErrorList;
   COL::TCopyCollection<COL::TTernaryTree<STG::SubString, KeywordToken> > ttKeywords;

  protected:
   ReadResult readMain(STG::SubString& in, unsigned& line,
      unsigned& column, bool doesForce);
   ReadResult readEndComment(STG::SubString& in, unsigned& line,
      unsigned& column, bool doesForce);
   ReadResult readIdentifierToken(STG::SubString& in, unsigned& line,
      unsigned& column, bool doesForce);
   ReadResult readNumberToken(STG::SubString& in, unsigned& line,
      unsigned& column, bool doesForce);

  public:
   Lexer();

   ReadResult readToken(STG::SubString& in, unsigned& line, unsigned& column,
         bool doesForce=false)
      {  return rpmSecondaryState
            ? (this->*rpmSecondaryState)(in, line, column, doesForce)
            : (this->*rpmMainState)(in, line, column, doesForce);
      }
   void setToken(const Token& token)
      {  AssumeCondition(tToken.getType() == AbstractToken::TUndefined);
         tToken = token;
      }
   AbstractToken queryToken() { return tToken.getFullToken(); }
   const AbstractToken& getContentToken() const { return tToken.getContent(); }
   void assumeContentToken() { tToken.assumeContent(); }
   bool hasErrors() const { return !leErrorList.isEmpty(); }
   COL::TList<Error>& errorList() { return leErrorList; }
   Token& extractToken() { return tToken; }
   bool hasToken() const { return tToken.getType() > AbstractToken::TUndefined; }
   bool doesNeedClear() const { return tToken.getType(); }

   void clearToken()
      {  tbCurrentToken.clear();
         rpmMainState = &Lexer::readMain;
         rpmSecondaryState = nullptr;
         chContext = '\0';
         chExtension = '\0';
         tToken = Token();
      }
};

}}} // end of namespace Analyzer::Scalar::Approximate

