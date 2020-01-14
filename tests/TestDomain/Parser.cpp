/////////////////////////////////
//
// Library   : unit test of Analyzer/Scalar
// Unit      : approximate scalar elements
// File      : TestGeneral/Parser.cpp
// Author    : Franck Vedrine
// Copyright : CEA LIST - 2019-2020 - all rights reserved
//
// Description :
//   Implementation of a parser to parse the input file.
//

#include "Parser.h"

namespace STG {}

#include <iostream>
#include "Collection/Collection.template"

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
Parser::parse(STG::IOObject::ISBase& in) {
   STG::SString buffer;
   buffer.bookPlace(5000);
   bool isEndStream = false;
   aArguments.uLine = 1;
   aArguments.uColumn = 1;
   
   int readChars = in.readsome(buffer.asPersistent(), buffer.getPlace());
   bool doesContinue = !((isEndStream = !in.good()) != false && (readChars == 0));
   aArguments.setBuffer(buffer);

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
            if (parseResult == RRFinished)
               return;
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

