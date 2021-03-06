// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "Common/RegistLibrary.hpp"

#include "Mesh/LagrangeP2/LibLagrangeP2.hpp"

namespace CF {
namespace Mesh {
namespace LagrangeP2 {

CF::Common::RegistLibrary<LibLagrangeP2> LibLagrangeP2;

////////////////////////////////////////////////////////////////////////////////

void LibLagrangeP2::initiate_impl()
{
}

////////////////////////////////////////////////////////////////////////////////

void LibLagrangeP2::terminate_impl()
{
}

////////////////////////////////////////////////////////////////////////////////

} // LagrangeP2
} // Mesh
} // CF
