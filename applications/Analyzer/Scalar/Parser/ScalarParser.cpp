/////////////////////////////////
//
// Library   : Analyzer/Scalar
// Unit      : approximate scalar elements
// File      : Parser/ScalarParser.cpp
// Author    : Franck Vedrine
// Copyright : CEA LIST - 2019-2020 - all rights reserved
//
// Description :
//   Implementation of a parser to parse the input file.
//

#include "Analyzer/Scalar/Parser/ScalarParser.h"

namespace STG {}

#include <iostream>
#include "Collection/Collection.template"

namespace Analyzer { namespace Scalar { namespace Approximate {

void
Parser::Arguments::fail() const {
   auto& msgs = errors();
   for (const auto& msg : msgs)
     std::cerr << msg.linePos << ':'
               << msg.columnPos << ": " 
               << msg.message.getChunk().string << std::endl;
   msgs.freeAll();
}

void
Parser::parse(STG::IOObject::ISBase& in, bool doesEarlyEnd, int earlyPosition) {
   STG::SString buffer;
   buffer.bookPlace(5000);
   bool isEndStream = false;
   aArguments.uLine = 1;
   aArguments.uColumn = 1;
   
   int readChars = in.readsome(buffer.asPersistent(), buffer.getPlace());
   bool doesContinue = !((isEndStream = !in.good()) != false && (readChars == 0));
   aArguments.setBuffer(buffer);
   if (doesEarlyEnd)
      aArguments.setDebug();

   try {
   if (doesContinue) {
      ReadResult parseResult = aArguments.lLexer.readToken(buffer,
            aArguments.uLine, aArguments.uColumn);
      while (parseResult != RRFinished) {
         while (parseResult == RRNeedChars) {
            do {
               parseResult = aArguments.lLexer.readToken(buffer,
                  aArguments.uLine, aArguments.uColumn, isEndStream);
            } while (parseResult == RRContinue);
            if (parseResult == RRFinished) {
               if (!sState.isEmpty()) { // clear the parser state
                  parseResult = sState.parse(aArguments);
                  if (!aArguments.pleErrorMessages->isEmpty()
                        && sState.getLevel() == 0 && sState.point() == 0)
                     aArguments.fail();
               }
               return;
            }
            if (parseResult == RRNeedChars) {
               readChars = 0;
               int result;
               if (buffer.length() > 0) {
                  STG::SubString text(buffer);
                  buffer.seek(0,0);
                  buffer.copy(text).setToEnd();
                  result = in.readsome(buffer.asPersistent(), buffer.getPlace());
                  buffer.seek(0,0);
               }
               else {
                  buffer.seek(0,0);
                  buffer.setLength(0);
                  result = in.readsome(buffer.asPersistent(), buffer.getPlace());
               };
               doesContinue = !((isEndStream = !in.good()) != false && (result == 0));
               if (!doesContinue)
                  return;
            };
         };
    
         if (!aArguments.pleErrorMessages->isEmpty()
               && sState.getLevel() == 0 && sState.point() == 0)
            aArguments.fail();

         if (parseResult == RRHasToken) {
            do {
               parseResult = sState.parse(aArguments);
               if (!aArguments.pleErrorMessages->isEmpty()
                     && sState.getLevel() == 0 && sState.point() == 0)
                  aArguments.fail();
            } while (parseResult == RRContinue || parseResult == RRHasToken);
            if (parseResult == RRNeedChars && aArguments.lLexer.doesNeedClear())
               aArguments.lLexer.clearToken();
            if (doesEarlyEnd && parseResult == RRNeedChars) {
               if (sState.getLevel() == 1 && sState.point() == earlyPosition) {
                  if (!aArguments.pleErrorMessages->isEmpty())
                     aArguments.fail();
                  aArguments.clearBuffer();
                  return;
               }
            }
         };
      };
   }
   } 
   catch (...) {
      if (!aArguments.pleErrorMessages->isEmpty())
         aArguments.fail();
      aArguments.clearBuffer();
      throw;
   }
   if (!aArguments.pleErrorMessages->isEmpty())
      aArguments.fail();
   aArguments.clearBuffer();
}

}}} // end of namespace Analyzer::Scalar::Approximate

