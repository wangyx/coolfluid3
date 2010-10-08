// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "Common/ObjectProvider.hpp"

#include "Solver/LibSolver.hpp"

#include "Solver/CMethod.hpp"

namespace CF {
namespace Solver {

using namespace Common;
using namespace Common::String;

Common::ObjectProvider < CMethod, Component, LibSolver, NB_ARGS_1 >
CMethod_Provider ( CMethod::type_name() );

////////////////////////////////////////////////////////////////////////////////

CMethod::CMethod ( const CName& name  ) :
  Component ( name )
{
  BUILD_COMPONENT;
}

////////////////////////////////////////////////////////////////////////////////

CMethod::~CMethod()
{
}

////////////////////////////////////////////////////////////////////////////////

void CMethod::regist_signals ( CMethod* self )
{
  self->regist_signal ( "run_operation" , "run an operation", "Run Operation" )->connect ( boost::bind ( &CMethod::run_operation, self, _1 ) );
}

////////////////////////////////////////////////////////////////////////////////

CMethod& CMethod::operation(const std::string& name)
{
  return *get_child_type<CMethod>(name);
}

////////////////////////////////////////////////////////////////////////////////

} // Solver
} // CF