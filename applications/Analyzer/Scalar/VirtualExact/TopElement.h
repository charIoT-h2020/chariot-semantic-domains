/////////////////////////////////
//
// Library   : Analyzer/Scalar
// Unit      : scalar elements common to compilation constants and symbolic execution
// File      : TopElement.h
// Author    : Franck Vedrine
// Copyright : CEA LIST - 2019-2020 - all rights reserved
//
// Description :
//   Definition of undefined compilation constants
//     or undefined elements used in simulation/symbolic execution.
//

#ifndef Analyzer_Scalar_VirtualExact_TopElementH
#define Analyzer_Scalar_VirtualExact_TopElementH

#include "Analyzer/Virtual/Scalar/AtomicElement.h"
#include "Analyzer/Scalar/ConcreteOperation.h"
#include "Pointer/TreeMethods.h"

namespace Analyzer { namespace Scalar { namespace VirtualExact {

/****************************************/
/* Definition of the class UndefElement */
/****************************************/

template <class TypeBase>
class TUndefElement : public TypeBase {
  private:
   typedef TUndefElement<TypeBase> thisType;
   typedef TypeBase inherited;
   int uSize;

  protected:
   virtual ComparisonResult _compare(const EnhancedObject& asource) const override
      {  return inherited::_compare(asource); }

  public:
   TUndefElement(const typename TypeBase::Init& iInit) : inherited(iInit), uSize(iInit.getBitSize()) {}
   TUndefElement(const thisType& source) = default;
   thisType& operator=(const thisType& source) = default;
   TemplateDefineCopy(TUndefElement, TypeBase)
   DTemplateDefineAssign(TUndefElement, TypeBase)

   virtual typename inherited::ZeroResult queryZeroResult() const override
      {  return typename inherited::ZeroResult(); }
   virtual int _getSizeInBits() const override { return uSize; }
   virtual bool apply(const VirtualOperation& operation, EvaluationEnvironment& env) override
      {  if (operation.isConst())
            env.storeResult(createSCopy());
         return true;
      }
   virtual bool applyTo(const VirtualOperation& operation, TypeBase& source,
         typename TypeBase::ApplyEnvironment& env) const override
      {  env.storeResult(createSCopy());
         return true;
      }
};

/****************************************************/
/* Definition of the template class TBadCastElement */
/****************************************************/

template <class TypeBase>
class TBadCastElement : public TypeBase {
  private:
   PPAbstractElement ppaeOriginal;

   typedef TBadCastElement<TypeBase> thisType;
   typedef TypeBase inherited;

  protected:
   virtual ComparisonResult _compare(const EnhancedObject& asource) const
      {  return inherited::_compare(asource); }

   TypeBase& subElement() const { return *ppaeOriginal; }
   static TypeBase& subElement(const TBadCastElement<TypeBase>& source)
      {  return source.subElement(); }

  public:
   TBadCastElement(const typename TypeBase::Init& init, AbstractElement* original)
      :  inherited(init), ppaeOriginal(original, PNT::Pointer::Init()) {}
   TBadCastElement(const thisType& source)
      :  inherited(source), ppaeOriginal(source.ppaeOriginal, PNT::Pointer::Duplicate()) {}
   thisType& operator=(const thisType& source)
      {  inherited::operator=(source);
         ppaeOriginal.fullAssign(source.ppaeOriginal);
         return *this;
      }
   TemplateDefineCopy(TBadCastElement, TypeBase)
   DTemplateDefineAssign(TBadCastElement, TypeBase)

   // Cast...
   bool applyCast(const VirtualOperation& operation, EvaluationEnvironment& env)
      {  return applyUndefined(operation, env); }
   virtual typename inherited::ZeroResult queryZeroResult() const { return ppaeOriginal->queryZeroResult(); }
      
   // is likely to be defined at the TypeBase level
   bool applyUndefined(const VirtualOperation& operation, EvaluationEnvironment& env);
   bool applyToUndefined(const VirtualOperation& operation, TypeBase& source,
         typename TypeBase::ApplyEnvironment& env) const;
};

}}} // end of namespace Analyzer::Scalar::VirtualExact

#endif // Analyzer_Scalar_VirtualExact_TopElementH

