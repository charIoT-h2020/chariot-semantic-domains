/////////////////////////////////
//
// Library   : Analyzer/Scalar
// Unit      : approximate scalar elements
// File      : Parser/ScalarLexer.cpp
// Author    : Franck Vedrine
// Copyright : CEA LIST - 2019-2020 - all rights reserved
//
// Description :
//   Implementation of a lexer to lex the input file.
//

#include "Analyzer/Scalar/Parser/ScalarLexer.h"

namespace STG {}

#include "TString/TernaryTreeInterface.template"

namespace Analyzer { namespace Scalar { namespace Approximate {

Lexer::Lexer()
   :  rpmMainState(&Lexer::readMain), rpmSecondaryState(nullptr),
      chContext('\0'), chExtension('\0'),
      ttKeywords(COL::TTernaryTree<STG::SubString, KeywordToken>::InitialNewValues()
         << STG::SString("var") << KeywordToken(KeywordToken::TVar)
         << STG::SString("multi") << KeywordToken(KeywordToken::TMulti)
         << STG::SString("bit") << KeywordToken(KeywordToken::TBit)
         << STG::SString("multif") << KeywordToken(KeywordToken::TMultiFloat)
         << STG::SString("multifptr") << KeywordToken(KeywordToken::TMultiFloatPointer)
         << STG::SString("extend_zero") << KeywordToken(KeywordToken::TExtendZero)
         << STG::SString("extend_sign") << KeywordToken(KeywordToken::TExtendSign)
         << STG::SString("concat") << KeywordToken(KeywordToken::TConcat)
         << STG::SString("reduce") << KeywordToken(KeywordToken::TReduce)
         << STG::SString("bitset") << KeywordToken(KeywordToken::TBitSet)
         << STG::SString("shift_bit") << KeywordToken(KeywordToken::TShiftBit)
         << STG::SString("incS") << KeywordToken(KeywordToken::TIncSigned)
         << STG::SString("incU") << KeywordToken(KeywordToken::TIncUnsigned)
         << STG::SString("decS") << KeywordToken(KeywordToken::TDecSigned)
         << STG::SString("decU") << KeywordToken(KeywordToken::TDecUnsigned)
         << STG::SString("minS") << KeywordToken(KeywordToken::TMinSigned)
         << STG::SString("minU") << KeywordToken(KeywordToken::TMinUnsigned)
         << STG::SString("maxS") << KeywordToken(KeywordToken::TMaxSigned)
         << STG::SString("maxU") << KeywordToken(KeywordToken::TMaxUnsigned)
         << STG::SString("vv") << KeywordToken(KeywordToken::TMerge)
         << STG::SString("T") << KeywordToken(KeywordToken::TTop)
         << STG::SString("Tb") << KeywordToken(KeywordToken::TTopBit)
         << STG::SString("S") << KeywordToken(KeywordToken::TSigned)
         << STG::SString("U") << KeywordToken(KeywordToken::TUnsigned)
         << STG::SString("US") << KeywordToken(KeywordToken::TUnsignedWithSigned)
         << STG::SString("_s") << KeywordToken(KeywordToken::TSymbolic)
         << STG::SString("multi_s") << KeywordToken(KeywordToken::TMultiSymbolic)
         << STG::SString("bit_s") << KeywordToken(KeywordToken::TBitSymbolic)
         << STG::SString("multif_s") << KeywordToken(KeywordToken::TMultiFloatSymbolic)
         << STG::SString("multifptr_s") << KeywordToken(KeywordToken::TMultiFloatPointerSymbolic)
         << STG::SString("extend_zero_s") << KeywordToken(KeywordToken::TExtendZeroSymbolic)
         << STG::SString("extend_sign_s") << KeywordToken(KeywordToken::TExtendSignSymbolic)
         << STG::SString("concat_s") << KeywordToken(KeywordToken::TConcatSymbolic)
         << STG::SString("reduce_s") << KeywordToken(KeywordToken::TReduceSymbolic)
         << STG::SString("bitset_s") << KeywordToken(KeywordToken::TBitSetSymbolic)
         << STG::SString("shift_bit_s") << KeywordToken(KeywordToken::TShiftBitSymbolic)
         << STG::SString("incS_s") << KeywordToken(KeywordToken::TIncSignedSymbolic)
         << STG::SString("incU_s") << KeywordToken(KeywordToken::TIncUnsignedSymbolic)
         << STG::SString("decS_s") << KeywordToken(KeywordToken::TDecSignedSymbolic)
         << STG::SString("decU_s") << KeywordToken(KeywordToken::TDecUnsignedSymbolic)
         << STG::SString("minS_s") << KeywordToken(KeywordToken::TMinSignedSymbolic)
         << STG::SString("minU_s") << KeywordToken(KeywordToken::TMinUnsignedSymbolic)
         << STG::SString("maxS_s") << KeywordToken(KeywordToken::TMaxSignedSymbolic)
         << STG::SString("maxU_s") << KeywordToken(KeywordToken::TMaxUnsignedSymbolic)
         << STG::SString("vv_s") << KeywordToken(KeywordToken::TMergeSymbolic)
         << STG::SString("T_s") << KeywordToken(KeywordToken::TTopSymbolic)
         << STG::SString("Tb_s") << KeywordToken(KeywordToken::TTopBitSymbolic)
         << STG::SString("S_s") << KeywordToken(KeywordToken::TSignedSymbolic)
         << STG::SString("U_s") << KeywordToken(KeywordToken::TUnsignedSymbolic)
         << STG::SString("US_s") << KeywordToken(KeywordToken::TUnsignedWithSignedSymbolic)
         << STG::SString("assert") << KeywordToken(KeywordToken::TAssert))
   {}

Lexer::ReadResult
Lexer::readMain(STG::SubString& in, unsigned& line, unsigned& column,
      bool doesForce) {
   auto chunk = in.getChunk();
   if (chunk.length == 0)
      return doesForce ? Lexer::RRFinished : Lexer::RRNeedChars;
   unsigned pos = 0;
   while (chunk.length > pos
         && STG::SubString::Traits::isspace(chunk.string[pos])) {
      if (chunk.string[pos] == '\n') {
         ++line;
         column = 1;
      }
      else
         ++column;
      ++pos;
   };
   in.advance(pos);
   if (chunk.length == pos)
      return doesForce ? Lexer::RRFinished : Lexer::RRNeedChars;

   switch (chunk.string[pos]) {
      case '<':
         if (!doesForce && chunk.length - pos < 2)
            return Lexer::RRNeedChars;
         if (chunk.string[pos+1] == '=') {
            pos += 2; column += 2;
            in.advance(2);
            tToken = OperatorPunctuatorToken(OperatorPunctuatorToken::TLessOrEqual);
            return RRHasToken;
         }
         else if (chunk.string[pos+1] == '<') {
            pos += 2; column += 2;
            in.advance(2);
            tToken = OperatorPunctuatorToken(OperatorPunctuatorToken::TLeftShift);
            return RRHasToken;
         }
         pos += 1; column += 1;
         in.advance();
         tToken = OperatorPunctuatorToken(OperatorPunctuatorToken::TLess);
         return RRHasToken;
      case '>':
         if (!doesForce && chunk.length - pos < 2)
            return Lexer::RRNeedChars;
         if (chunk.string[pos+1] == '=') {
            pos += 2; column += 2;
            in.advance(2);
            tToken = OperatorPunctuatorToken(OperatorPunctuatorToken::TGreaterOrEqual);
            return RRHasToken;
         }
         else if (chunk.string[pos+1] == '>') {
            pos += 2; column += 2;
            in.advance(2);
            tToken = OperatorPunctuatorToken(OperatorPunctuatorToken::TRightShift);
            return RRHasToken;
         }
         pos += 1; column += 1;
         in.advance();
         tToken = OperatorPunctuatorToken(OperatorPunctuatorToken::TGreater);
         return RRHasToken;
      case '!':
         if (!doesForce && chunk.length - pos < 2)
            return Lexer::RRNeedChars;
         if (chunk.string[pos+1] == '=') {
            pos += 2; column += 2;
            in.advance(2);
            tToken = OperatorPunctuatorToken(OperatorPunctuatorToken::TDifferent);
            return RRHasToken;
         }
         pos += 1; column += 1;
         in.advance();
         tToken = OperatorPunctuatorToken(OperatorPunctuatorToken::TLogicalNegate);
         return RRHasToken;
      case '=':
         if (!doesForce && chunk.length - pos < 2)
            return Lexer::RRNeedChars;
         if (chunk.string[pos+1] == '=') {
            pos += 2; column += 2;
            in.advance(2);
            tToken = OperatorPunctuatorToken(OperatorPunctuatorToken::TEqual);
            return RRHasToken;
         };
         pos += 1; column += 1;
         in.advance();
         tToken = OperatorPunctuatorToken(OperatorPunctuatorToken::TAssign);
         return RRHasToken;
      case '&':
         if (!doesForce && chunk.length - pos < 2)
            return Lexer::RRNeedChars;
         if (chunk.string[pos+1] == '&') {
            pos += 2; column += 2;
            in.advance(2);
            tToken = OperatorPunctuatorToken(OperatorPunctuatorToken::TLogicalAnd);
            return RRHasToken;
         }
         pos += 1; column += 1;
         in.advance();
         tToken = OperatorPunctuatorToken(OperatorPunctuatorToken::TBitAnd);
         return RRHasToken;
      case '|':
         if (!doesForce && chunk.length - pos < 2)
            return Lexer::RRNeedChars;
         if (chunk.string[pos+1] == '|') {
            pos += 2; column += 2;
            in.advance(2);
            tToken = OperatorPunctuatorToken(OperatorPunctuatorToken::TLogicalOr);
            return RRHasToken;
         }
         pos += 1; column += 1;
         in.advance();
         tToken = OperatorPunctuatorToken(OperatorPunctuatorToken::TBitOr);
         return RRHasToken;
      case '^':
         if (!doesForce && chunk.length - pos < 2)
            return Lexer::RRNeedChars;
         if (chunk.string[pos+1] == '^') {
            pos += 2; column += 2;
            in.advance(2);
            tToken = OperatorPunctuatorToken(OperatorPunctuatorToken::TIntersect);
            return RRHasToken;
         };
         pos += 1; column += 1;
         in.advance();
         tToken = OperatorPunctuatorToken(OperatorPunctuatorToken::TBitXor);
         return RRHasToken;
      case '-':
         pos += 1; column += 1;
         in.advance();
         tToken = OperatorPunctuatorToken(OperatorPunctuatorToken::TMinus);
         return RRHasToken;
      case '+':
         pos += 1; column += 1;
         in.advance();
         tToken = OperatorPunctuatorToken(OperatorPunctuatorToken::TPlus);
         return RRHasToken;
      case '{':
         pos += 1; column += 1;
         in.advance();
         tToken = OperatorPunctuatorToken(OperatorPunctuatorToken::TOpenBrace);
         return RRHasToken;
      case '}':
         pos += 1; column += 1;
         in.advance();
         tToken = OperatorPunctuatorToken(OperatorPunctuatorToken::TCloseBrace);
         return RRHasToken;
      case '[':
         pos += 1; column += 1;
         in.advance();
         tToken = OperatorPunctuatorToken(OperatorPunctuatorToken::TOpenBracket);
         return RRHasToken;
      case ']':
         pos += 1; column += 1;
         in.advance();
         tToken = OperatorPunctuatorToken(OperatorPunctuatorToken::TCloseBracket);
         return RRHasToken;
      case '(':
         pos += 1; column += 1;
         in.advance();
         tToken = OperatorPunctuatorToken(OperatorPunctuatorToken::TOpenParen);
         return RRHasToken;
      case ')':
         pos += 1; column += 1;
         in.advance();
         tToken = OperatorPunctuatorToken(OperatorPunctuatorToken::TCloseParen);
         return RRHasToken;
      case ';':
         pos += 1; column += 1;
         in.advance();
         tToken = OperatorPunctuatorToken(OperatorPunctuatorToken::TSemiColon);
         return RRHasToken;
      case ':':
         pos += 1; column += 1;
         in.advance();
         tToken = OperatorPunctuatorToken(OperatorPunctuatorToken::TColon);
         return RRHasToken;
      case '.':
         if (!doesForce && chunk.length - pos < 1)
            return Lexer::RRNeedChars;
         if (STG::SubString::Traits::isdigit(chunk.string[pos+1])) {
            rpmSecondaryState = &Lexer::readNumberToken; 
            chContext = (char) 0;
            return (this->*rpmSecondaryState)(in, line, column, doesForce);
         }
         // pos += 1; column += 1;
         // in.advance();
         // tToken = OperatorPunctuatorToken(OperatorPunctuatorToken::TPunct);
         // return RRHasToken;
         break;
      case '?':
         pos += 1; column += 1;
         in.advance();
         tToken = OperatorPunctuatorToken(OperatorPunctuatorToken::TConditional);
         return RRHasToken;
      case '*':
         pos += 1; column += 1;
         in.advance();
         tToken = OperatorPunctuatorToken(OperatorPunctuatorToken::TTimes);
         return RRHasToken;
      case '/':
         if (!doesForce && chunk.length - pos < 2)
            return Lexer::RRNeedChars;
         if (chunk.string[pos+1] == '/') {
            pos += 2; column += 2;
            in.advance(2);
            rpmSecondaryState = &Lexer::readEndComment; 
            return (this->*rpmSecondaryState)(in, line, column, doesForce);
         }
         else if (chunk.string[pos+1] == '*') {
            pos += 2; column += 2;
            in.advance(2);
            chContext = '*';
            rpmSecondaryState = &Lexer::readEndComment; 
            return (this->*rpmSecondaryState)(in, line, column, doesForce);
         };
         pos += 1; column += 1;
         in.advance();
         tToken = OperatorPunctuatorToken(OperatorPunctuatorToken::TDivide);
         return RRHasToken;
      case '%':
         pos += 1; column += 1;
         in.advance();
         tToken = OperatorPunctuatorToken(OperatorPunctuatorToken::TModulo);
         return RRHasToken;
      case '~':
         pos += 1; column += 1;
         in.advance();
         tToken = OperatorPunctuatorToken(OperatorPunctuatorToken::TBitNegate);
         return RRHasToken;
      case ',':
         pos += 1; column += 1;
         in.advance();
         tToken = OperatorPunctuatorToken(OperatorPunctuatorToken::TComma);
         return RRHasToken;
   };
   char ch = chunk.string[pos];
   if (ch == '0') {
      if (!doesForce && chunk.length - pos < 2)
         return Lexer::RRNeedChars;
      char chs = chunk.string[pos+1];
      if ((chs == 'x') || (chs == 'X')) {
         rpmSecondaryState = &Lexer::readNumberToken; 
         chContext = (char) 3;
         in.advance(2);
         pos += 2; column += 2;
         return (this->*rpmSecondaryState)(in, line, column, doesForce);
      }
      else if ((chs == 'b') || (chs == 'B')) {
         rpmSecondaryState = &Lexer::readNumberToken; 
         chContext = (char) 1;
         in.advance(2);
         pos += 2; column += 2;
         return (this->*rpmSecondaryState)(in, line, column, doesForce);
      }
      else if (chs >= '0' && chs <= '7') {
         rpmSecondaryState = &Lexer::readNumberToken; 
         chContext = (char) 2;
         in.advance(1);
         pos += 1; column += 1;
         return (this->*rpmSecondaryState)(in, line, column, doesForce);
      };
   };
   if (STG::SubString::Traits::isdigit(ch)) {
      rpmSecondaryState = &Lexer::readNumberToken; 
      chContext = (char) 0;
      return (this->*rpmSecondaryState)(in, line, column, doesForce);
   };
   if (STG::SubString::Traits::isalnum(ch) || ch == '_') {
      rpmSecondaryState = &Lexer::readIdentifierToken; 
      return (this->*rpmSecondaryState)(in, line, column, doesForce);
   };
   leErrorList.insertNewAtEnd(new Error(line, column, STG::SString("unknown character.")));
   return RRContinue;
}

Lexer::ReadResult
Lexer::readEndComment(STG::SubString& in, unsigned& line, unsigned& column, bool doesForce) {
   int endPos;
   unsigned lineEnd = line;
   unsigned columnEnd = column;
   if (chContext == '*') {
      endPos = in.scanPos("*/");
      int localPos = in.scanPos('\n');
      if (localPos >= 0) {
         ++lineEnd;
         columnEnd = 0;
         STG::SubString local(in);
         if (endPos >= 0)
            local.setUpperClosed().setLength(endPos);
         do {
            local.advance(localPos+1);
            localPos = local.scanPos('\n');
            if (localPos >= 0)
               ++lineEnd;
         } while (localPos >= 0);
         columnEnd = local.length();
         if (endPos < 0)
            columnEnd--;
      }
      else if (endPos >= 0)
         columnEnd += endPos + 2;
      else
         columnEnd += in.length() - 1;
   }
   else {
      endPos = in.scanPos('\n');
      if (endPos >= 0)
         columnEnd += endPos + 1;
      else
         columnEnd += in.length() - 1;
   };

   if (endPos >= 0) {
      clearToken();
      // tbCurrentToken << STG::SubString(in).setUpperClosed().setLength(endPos);
      // CommentToken* result = new CommentToken;
      // if (chContext == '\n')
      //    result->setEndLine();
      // tToken = Token(result);
      // tbCurrentToken >> result->content();
      // rpmSecondaryState = nullptr;
      line = lineEnd;
      column = columnEnd;
      return RRHasToken;
   };

   if (!doesForce) {
      if (chContext == '*')
         tbCurrentToken << STG::SubString(in).setUpperClosed().setLength(in.length()-1);
      else
         tbCurrentToken << in;
      line = lineEnd;
      column = columnEnd;
      return RRNeedChars;
   };
   
   tbCurrentToken << in;
   if (chContext == '*') {
      ++columnEnd;
      leErrorList.insertNewAtEnd(new Error(line, column,
         STG::SString("comment not terminated")));
      // line = lineEnd;
      // column = columnEnd;
      return RRFinished;
   };
   return RRHasToken;
}

Lexer::ReadResult
Lexer::readIdentifierToken(STG::SubString& in, unsigned& line, unsigned& column, bool doesForce) {
   auto chunk = in.getChunk();
   if (chunk.length == 0) {
      if (doesForce) {
         STG::SString identifier;
         tbCurrentToken >> identifier;
         IdentifierToken* result = new IdentifierToken(identifier);
         tToken = Token(result);
         rpmSecondaryState = nullptr;
         return Lexer::RRHasToken;
      }
      return Lexer::RRNeedChars;
   }
   unsigned pos = 0;
   while (chunk.length > pos
         && (STG::SubString::Traits::isalnum(chunk.string[pos])
            || (chunk.string[pos] == '_')))
      ++pos;
   if (pos > 0)
      tbCurrentToken << STG::SubString(in).setUpperClosed().setLength(pos);

   column += pos;
   in.advance(pos);
   if (0 < in.length() || doesForce) {
      STG::SString identifier;
      tbCurrentToken >> identifier;
      STG::SubString extension(identifier);
      extension.setUpperClosed().advance(extension.length()-1);
      if (STG::SubString::Traits::isdigit(extension.getSChar())) {
         while (extension.cstart().getPosition() > 0) {
            extension.advance(-1);
            if (!STG::SubString::Traits::isdigit(extension.getSChar()))
               break;
         };
         if (extension.getSChar() != '_')
            extension.clear();
         else {
            extension.advance(-1);
            if (STG::SubString::Traits::isdigit(extension.getSChar())) {
               STG::SubString firstExtension(extension);
               while (extension.cstart().getPosition() > 0) {
                  extension.advance(-1);
                  if (!STG::SubString::Traits::isdigit(extension.getSChar()))
                     break;
               };
               if (extension.getSChar() != '_')
                  extension = firstExtension;
            }
            else
               extension.advance();
         };
      }
      else
         extension.clear();
      {  STG::SubString content = identifier;
         if (extension.length() > 0)
            content.setUpperClosed().setLength(identifier.getSubStringPos(extension));
         COL::TTernaryTree<STG::SubString, KeywordToken>::Cursor
            keyCusor(ttKeywords);
         if (ttKeywords.locateKey(content, keyCusor, COL::VirtualCollection::RPExact)) {
            tToken = keyCusor.elementAt();
            rpmSecondaryState = nullptr;
            if (extension.length() > 0) {
               AssumeCondition(extension.getSChar() == '_')
               extension.advance();
               int first = extension.readInteger();
               auto fullToken = tToken.getFullToken();
               if (extension.length() > 0) {
                  AssumeCondition(extension.getSChar() == '_')
                  extension.advance();
                  int second = extension.readInteger();
                  FloatSizedKeywordToken* result = new FloatSizedKeywordToken(
                     ((const KeywordToken&) fullToken).getType(), first, second);
                  tToken = Token(result);
               }
               else {
                  SizedKeywordToken* result = new SizedKeywordToken(
                     ((const KeywordToken&) fullToken).getType(), first);
                  tToken = Token(result);
               };
            };
            return RRHasToken;
         };
      };
      IdentifierToken* result = new IdentifierToken(identifier);
      tToken = Token(result);
      rpmSecondaryState = nullptr;
      return RRHasToken;
   };
   return RRNeedChars;
}

inline bool isCharDigit(char ch, int context) {
   return (context == 0)
      ? STG::SubString::Traits::isdigit(ch) : ((context == 1)
      ? (ch == '0' || ch == '1') : ((context == 2)
      ? STG::SubString::Traits::isoctaldigit(ch) : ((context == 3)
      ? STG::SubString::Traits::ishexadigit(ch) : false)));
}

Lexer::ReadResult
Lexer::readNumberToken(STG::SubString& in, unsigned& line, unsigned& column, bool doesForce) {
   auto chunk = in.getChunk();
   int pos = 0;
   char ch = '\0';
   if (chunk.length == 0) {
      if (doesForce)
         goto LEnd;
      return Lexer::RRNeedChars;
   };
   while (isCharDigit(chunk.string[pos], chContext & 0x3) && (chContext >> 2) < 16) {
      if ((chContext >> 2) == 2)
         chContext = (3 << 2) | (chContext & 0x3);
      ++pos;
   };
   if (chunk.string[pos] == '.' && (chContext >> 2) == 0) {
      ++pos;
      chContext = (1 << 2) | (chContext & 0x3);
      while (isCharDigit(chunk.string[pos], chContext & 0x3))
         ++pos;
   };
   if ((chunk.string[pos] == 'e' || chunk.string[pos] == 'E') && (chContext >> 2 == 1)) {
      ++pos;
      chContext = (2 << 2) | (chContext & 0x3);
   };
   if ((chunk.string[pos] == '+' || chunk.string[pos] == '-') && (chContext >> 2 == 2)) {
      ++pos;
      chContext = (3 << 2) | (chContext & 0x3);
      while (isCharDigit(chunk.string[pos], chContext & 0x3))
         ++pos;
   };
   if (chunk.string[pos] == '_' && (chContext >> 2 < 4)) {
      ++pos;
      chContext |= (4 << 2);
      while (STG::SubString::Traits::isdigit(chunk.string[pos]))
         ++pos;
   };
   if (chunk.string[pos] == '_' && (chContext >> 2 < 8)) {
      ++pos;
      chContext |= (8 << 2);
      while (STG::SubString::Traits::isdigit(chunk.string[pos]))
         ++pos;
   };

   enum Shift { SBit = 1, SUnsigned = 2 };

   ch = chunk.string[pos];
   while ((ch == 'b') || (ch == 'B')) {
      unsigned shift = 0;
      if (ch == 'b' || ch == 'B')
         shift = SBit;
      if ((chContext >> 2) < 16) {
         chExtension = 0;
         chContext |= (16 << 2);
      };
      chExtension |= (1 << shift);
      ++pos;
      ch = chunk.string[pos];
   };
   if (pos > 0)
      tbCurrentToken << STG::SubString(in).setUpperClosed().setLength(pos);

LEnd:
   if (pos < in.length() || doesForce) {
      STG::SString extendedNumber;
      tbCurrentToken >> extendedNumber;
      STG::SubString number(extendedNumber);
      number.setUpperClosed();
      int firstSize= 0, secondSize = 0;
      if ((chContext >> 2) & 4) {
         int pos = number.scanPos('_');
         AssumeCondition(pos >= 0)
         STG::SubString first(number, pos+1);
         firstSize = first.readInteger();
         if ((chContext >> 2) & 8) {
            int pos2 = first.scanPos('_');
            AssumeCondition(pos2 >= 0)
            STG::SubString second(first, pos2+1);
            secondSize = second.readInteger();
         };
         number.setLength(pos);
      };
      bool isInteger = (((chContext >> 2) & 0x3) == 0);
      NumberToken* result = new NumberToken(number, !isInteger /* isFloat */);
      switch (chContext & 0x3) {
         case 1: result->setBitCoding(); break;
         case 2: result->setOctalCoding(); break;
         case 3: result->setHexaDecimalCoding(); break;
         default: break;
      }

      tToken = Token(result);
      if ((chContext >> 2) & 4) {
         if ((chContext >> 2) & 8)
            result->setDoubleSize(firstSize, secondSize);
         else
            result->setSize(firstSize);
      };
         
      bool hasExtensionIssue = false;
      if (isInteger) {
         result->setIntegerType();
         if (chExtension & (1 << SBit))
            result->addBitExtension();
      }
      else
         result->setFloatingPointType();
      rpmSecondaryState = nullptr;
      if (hasExtensionIssue)
         leErrorList.insertNewAtEnd(new Error(line, column,
            STG::SString("bad extension for number")));
      in.advance(pos);
      column += pos;
      return RRHasToken;
   };
   in.advance(pos);
   column += pos;
   return RRNeedChars;
}

}}} // end of namespace Analyzer::Scalar::Approximate

