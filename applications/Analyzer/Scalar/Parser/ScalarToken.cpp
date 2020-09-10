/////////////////////////////////
//
// Library   : Analyzer/Scalar
// Unit      : approximate scalar elements
// File      : Parser/ScalarToken.cpp
// Author    : Franck Vedrine
// Copyright : CEA LIST - 2019-2020 - all rights reserved
//
// Description :
//   Implementation of tokens to lex the input file.
//

#include "Analyzer/Scalar/Parser/ScalarToken.h"

namespace Analyzer { namespace Scalar { namespace Approximate {

void
Token::assumeContent()
{  AssumeCondition(uType != 0);
   if (!aptContent.isValid()) {
      AbstractToken basic(uType, AbstractToken::FullType());
      basic.clearFull();
      switch (basic.getType()) {
         case AbstractToken::TKeyword:
            aptContent.absorbElement(new KeywordToken((const KeywordToken&) basic));
            break;
         case AbstractToken::TOperatorPunctuator:
            aptContent.absorbElement(new OperatorPunctuatorToken((const OperatorPunctuatorToken&) basic));
            break;
         default:
            aptContent.absorbElement(new AbstractToken(basic));
      };
   };
}

}}} // end of namespace Analyzer::Scalar::Approximate

