/**************************************************************************/
/*                                                                        */
/*  Copyright (C) 2005-2020                                               */
/*    CEA (Commissariat a l'Energie Atomique et aux Energies              */
/*         Alternatives)                                                  */
/*                                                                        */
/*  you can redistribute it and/or modify it under the terms of the GNU   */
/*  Lesser General Public License as published by the Free Software       */
/*  Foundation, version 2.1.                                              */
/*                                                                        */
/*  It is distributed in the hope that it will be useful,                 */
/*  but WITHOUT ANY WARRANTY; without even the implied warranty of        */
/*  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         */
/*  GNU Lesser General Public License for more details.                   */
/*                                                                        */
/*  See the GNU Lesser General Public License version 2.1                 */
/*  for more details (enclosed in the file LICENSE).                      */
/*                                                                        */
/**************************************************************************/

/////////////////////////////////
//
// Library   : ForwardNumerics
// Unit      : Floating
// File      : BaseFloatingGeneric.h
//
// Description :
//   Definition of a class of floating points with unbound size.
//

#ifndef Numerics_BaseFloatingGenericH
#define Numerics_BaseFloatingGenericH

#include "ForwardNumerics/BaseIntegerGeneric.h"
#include "ForwardNumerics/BaseFloatingParameters.h"

namespace Numerics {

#define DefineGeneric
#include "ForwardNumerics/BaseFloating.inch"
#undef DefineGeneric

} // end of namespace Numerics

#endif // Numerics_BaseFloatingGenericH

