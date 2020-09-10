/////////////////////////////////
//
// Library   : Analyzer/Scalar
// Unit      : precision lattice
// File      : PrecisionLattice.h
// Author    : Franck Vedrine
// Copyright : CEA LIST - 2019-2020 - all rights reserved
//
// Description :
//   Definition of a lattice for the precision requirements.
//   It is exclusively used in backward analysis.
//

#ifndef Analyzer_Scalar_Approximate_PrecisionLatticeH
#define Analyzer_Scalar_Approximate_PrecisionLatticeH

#include "Analyzer/Scalar/Approximate/VirtualElement.h"

namespace Analyzer { namespace Scalar { namespace Approximate {

class PrecisionLattice : public VirtualElement, public BaseRequirement {
  private:
   typedef VirtualElement inherited;
   PPVirtualElement ppveDomain;

  protected:
   DefineExtendedParameters(10, VirtualElement)
   DefineSubExtendedParameters(Lattice, 7, INHERITED)
  public:
   DefineSubExtendedParameters(Required, 6, INHERITED)
   static Required queryRequired(const VirtualElement& concrete, TypeOperation typeOperation, bool* isComplete); 

  protected:
   DefineSubExtendedParameters(Complete, 1, Required)
   DefineSubExtendedParameters(TypeOperation, 3, Complete)

   void _setFromDomain(const VirtualElement& concrete);
   void setDomain(PPVirtualElement domain) { ppveDomain = domain; }
   PPVirtualElement extractDomain() { return ppveDomain; }

  public:
   Required getRequired() const { return (Required) queryRequiredField(); }
   void intersectRequired(Required intersect) { intersectRequiredField(intersect); }
   void setRequired(Required required) { setRequiredField(required); }
   TypeOperation getTypeOperation() const { return (TypeOperation) queryTypeOperationField(); }
   void setTypeOperation(TypeOperation operation) { setTypeOperationField(operation); }
   
  public:
   PrecisionLattice(const Init& init) : inherited(init) { setUnknown(); }
   PrecisionLattice(const InitFloat& init) : inherited(init) { setUnknown(); }
   PrecisionLattice(const PrecisionLattice& source)
      :  inherited(source), ppveDomain(source.ppveDomain, PNT::Pointer::Duplicate()) {}
   PrecisionLattice& operator=(const PrecisionLattice& source)
      {  inherited::operator=(source);
         ppveDomain.fullAssign(source.ppveDomain);
         setOwnField(source.queryOwnField());
         return *this;
      }

   DefineCopy(PrecisionLattice)
   DDefineAssign(PrecisionLattice)
   virtual int _getSizeInBits() const override { return ppveDomain->getSizeInBits(); }
   const VirtualElement& domain() const { return *ppveDomain; }
   bool hasDomain() const { return ppveDomain.isValid(); }

   // change the information type required on the environment
   virtual bool applyTo(const VirtualOperation& operation, VirtualElement&, EvaluationEnvironment& env) const override { AssumeUncalled return false; }
   virtual bool mergeWithTo(VirtualElement& source, EvaluationEnvironment& env) const override { AssumeUncalled return false; }
   virtual bool containTo(const VirtualElement& source, EvaluationEnvironment& env) const override { AssumeUncalled return false; }
   virtual bool intersectWithTo(VirtualElement& source, EvaluationEnvironment& env) const override { AssumeUncalled return false; }
   virtual bool constraintTo(const VirtualOperation&, VirtualElement& source, const VirtualElement&, ConstraintEnvironment&) override { AssumeUncalled return false; }
   virtual ZeroResult queryZeroResult() const override { return ZeroResult(); }
   virtual bool apply(const VirtualOperation& operation, Scalar::EvaluationEnvironment& env) override;
   virtual bool mergeWith(const Scalar::VirtualElement& source, Scalar::EvaluationEnvironment& env) override;
   virtual bool contain(const Scalar::VirtualElement& source, Scalar::EvaluationEnvironment& env) const override
      {  AssumeCondition(dynamic_cast<const PrecisionLattice*>(&source))
         int thisLattice = queryRequiredField(), sourceLattice = ((const PrecisionLattice&) source).queryRequiredField();
         if ((thisLattice & sourceLattice) != sourceLattice)
            env.setNoneApplied();
         return true;
      }
   virtual bool intersectWith(const VirtualElement& source, EvaluationEnvironment& env) override
      {  AssumeCondition(dynamic_cast<const PrecisionLattice*>(&source))
         int latticeField = queryLatticeField();
         intersectLatticeField(((const PrecisionLattice&) source).queryLatticeField());
         if (latticeField != queryLatticeField())
            env.setUnstable();
         return true;
      }
   virtual bool constraint(const VirtualOperation&, const VirtualElement&, ConstraintEnvironment&) override;
   virtual bool query(const VirtualQueryOperation& operation, VirtualQueryOperation::Environment& env) const override
      {  return ppveDomain->query(operation, env); }
   virtual bool guard(PPVirtualElement thenElement, PPVirtualElement elseElement, EvaluationEnvironment& env) override
      {  return guardApproximate(thenElement, elseElement, env); }

   void setFromDomain(PPVirtualElement concrete)
      {  _setFromDomain(*concrete);
         ppveDomain = concrete;
      }
   void clearFromDomain()
      {  ppveDomain.release();
         clearOwnField();
      }
   void changeDomain(PPVirtualElement domain) { ppveDomain = domain; }
};

}}} // end of namespace Analyzer::Scalar::Approximate

#endif // Analyzer_Scalar_Approximate_PrecisionLatticeH
