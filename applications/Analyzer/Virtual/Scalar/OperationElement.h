/////////////////////////////////
//
// Library   : Analyzer/Virtual
// Unit      : Scalar
// File      : OperationElement.h
// Author    : Franck Vedrine
// Copyright : CEA LIST - 2019-2020 - all rights reserved
//
// Description :
//   Definition of the scalar virtual operations required by the memory.
//

#ifndef Analyzer_Scalar_OperationElementH
#define Analyzer_Scalar_OperationElementH

#include "TString/String.hpp"

namespace Analyzer { namespace Scalar {

class OperationSignature : protected ExtendedParameters {
  protected:
   enum Constant { CNotConstant, CConstant=1, CConstantWithAssign=3 };
   DefineExtendedParameters(4, ExtendedParameters)
   DefineSubExtendedParameters(ArgumentsNumber, 2, INHERITED)
   DefineSubExtendedParameters(Constant, 2, ArgumentsNumber)

  public:
   OperationSignature() {}
   OperationSignature(const OperationSignature& source) = default;
   OperationSignature& operator=(const OperationSignature& source) = default;

   OperationSignature& setArgumentsNumber(int args)
      {  AssumeCondition(!hasArgumentsNumberField())
         mergeArgumentsNumberField(args);
         return *this;
      }
   int getArgumentsNumber() const { return queryArgumentsNumberField(); }
   OperationSignature& setConst()
      {  AssumeCondition(!hasConstantField())
         mergeConstantField(CConstant);
         return *this;
      }
   bool isConst() const { return hasConstantField(); }
   bool isAssign() const { return queryConstantField() == CNotConstant; }
   OperationSignature& setConstWithAssign()
      {  AssumeCondition(!hasConstantField())
         mergeConstantField(CConstantWithAssign);
         return *this;
      }
   OperationSignature& clearConstWithAssign()
      {  AssumeCondition(queryConstantField() == CConstantWithAssign)
         clearConstantField();
         return *this;
      }
   bool isConstWithAssign() const { return queryConstantField() == CConstantWithAssign; }
};

/*************************************/
/* Definition of the class Operation */
/*************************************/

class VirtualOperation : public EnhancedObject, public OperationSignature {
  private:
   typedef EnhancedObject inherited;

  protected:
   DefineExtendedParameters(1, OperationSignature)
   void setUndefined() { mergeOwnField(1); }

  public:
   VirtualOperation() {}
   VirtualOperation(const VirtualOperation& source) = default;
   DefineCopy(VirtualOperation)
   DDefineAssign(VirtualOperation)
   virtual bool isValid() const override { return hasOwnField(); }

   VirtualOperation& setArgumentsNumber(int args) { return (VirtualOperation&) OperationSignature::setArgumentsNumber(args); }
   VirtualOperation& setAssign() { clearConstWithAssign(); return *this; }
   VirtualOperation& setConst() { return (VirtualOperation&) OperationSignature::setConst(); }
   VirtualOperation& setConstWithAssign() { return (VirtualOperation&) OperationSignature::setConstWithAssign(); }
   VirtualOperation& clearConstWithAssign() { return (VirtualOperation&) OperationSignature::clearConstWithAssign(); }
   bool isDefined() const { return !hasOwnField(); }
   bool isUndefined() const { return hasOwnField(); }
   void clear() { params() = 0U; }
   virtual bool isSimple() const { return true; }
};

/**********************************************/
/* Definition of the class UndefinedOperation */
/**********************************************/
// useful to define the dependency relations without any knowledge about the computation details.
//

class UndefinedOperation : public VirtualOperation {
  private:
   typedef VirtualOperation inherited;
   
  public:
   UndefinedOperation() { setUndefined(); }
   UndefinedOperation(const UndefinedOperation& source) : inherited(source) {}
   DefineCopy(UndefinedOperation)
   
   UndefinedOperation& setArgumentsNumber(int number) { return (UndefinedOperation&) inherited::setArgumentsNumber(number); }
};

}} // end of namespace Analyzer::Scalar

#endif // Analyzer_Scalar_OperationElementH
