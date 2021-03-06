// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "Common/Log.hpp"
#include "Common/CBuilder.hpp"
#include "Common/FindComponents.hpp"
#include "RiemannSolvers/Roe.hpp"
#include "Common/OptionComponent.hpp"

namespace CF {
namespace RiemannSolvers {

using namespace Common;
using namespace Physics;

Common::ComponentBuilder < Roe, RiemannSolver, LibRiemannSolvers > Roe_Builder;

////////////////////////////////////////////////////////////////////////////////

Roe::Roe ( const std::string& name ) : RiemannSolver(name)
{
  options().add_option( OptionComponent<Physics::Variables>::create("roe_vars",&m_roe_vars) )
      ->description("The component describing the Roe variables")
      ->pretty_name("Roe Variables");

  option("physical_model").attach_trigger( boost::bind( &Roe::trigger_physical_model, this) );
}

////////////////////////////////////////////////////////////////////////////////

Roe::~Roe()
{
}

////////////////////////////////////////////////////////////////////////////////

void Roe::trigger_physical_model()
{
  coord.resize(physical_model().ndim());
  grads.resize(physical_model().neqs(),physical_model().ndim());
  p_left  = physical_model().create_properties();
  p_right = physical_model().create_properties();
  p_avg   = physical_model().create_properties();

  roe_left.resize(physical_model().neqs());
  roe_right.resize(physical_model().neqs());
  roe_avg.resize(physical_model().neqs());

  f_left.resize(physical_model().neqs(),physical_model().ndim());
  f_right.resize(physical_model().neqs(),physical_model().ndim());

  eigenvalues.resize(physical_model().neqs());
  right_eigenvectors.resize(physical_model().neqs(),physical_model().neqs());
  left_eigenvectors.resize(physical_model().neqs(),physical_model().neqs());
  abs_jacobian.resize(physical_model().neqs(),physical_model().neqs());


  // Try to configure solution_vars automatically
  if (m_solution_vars.expired())
  {
    if (Component::Ptr found_solution_vars = find_component_ptr_recursively_with_name(physical_model(),"solution_vars"))
    {
      configure_option("solution_vars",found_solution_vars->uri());
    }
    else
    {
      CFwarn << "Roe RiemannSolver " << uri().string() << " could not auto-config \"solution_vars\".\nConfigure manually\n    ( Reason: component with name \"solution_vars\" not found in ["<<physical_model().uri().string() << "] )" << CFendl;
    }
  }

  // Try to configure roe_vars automatically
  if (m_roe_vars.expired())
  {
    if (Component::Ptr found_roe_vars = find_component_ptr_recursively_with_name(physical_model(),"roe_vars"))
    {
      configure_option("roe_vars",found_roe_vars->uri());
    }
    else
    {
      CFwarn << "Roe RiemannSolver " << uri().string() << " could not auto-config \"roe_vars\".\nConfigure manually\n    ( Reason: component with name \"roe_vars\" not found in ["<<physical_model().uri().string() << "] )" << CFendl;
    }
  }
}

////////////////////////////////////////////////////////////////////////////////

void Roe::compute_interface_flux(const RealVector& left, const RealVector& right, const RealVector& normal,
                                 RealVector& flux)
{
  // Compute left and right properties
  solution_vars().compute_properties(coord,left,grads,*p_left);
  solution_vars().compute_properties(coord,right,grads,*p_right);

  // Compute the Roe averaged properties
  // Roe-average = standard average of the Roe-parameter vectors
  roe_vars().compute_variables(*p_left,  roe_left );
  roe_vars().compute_variables(*p_right, roe_right);
  roe_avg = 0.5*(roe_left+roe_right);                // Roe-average is result
  roe_vars().compute_properties(coord, roe_avg, grads, *p_avg);

  // Compute absolute jacobian using Roe averaged properties
  solution_vars().flux_jacobian_eigen_structure(*p_avg,normal,right_eigenvectors,left_eigenvectors,eigenvalues);
  abs_jacobian = right_eigenvectors * eigenvalues.cwiseAbs().asDiagonal() * left_eigenvectors;

  // Compute left and right fluxes
  solution_vars().flux(*p_left , f_left);
  solution_vars().flux(*p_right, f_right);

  // Compute flux at interface composed of central part and upwind part
  flux = 0.5*(f_left*normal+f_right*normal) - 0.5 * abs_jacobian*(right-left);
}

////////////////////////////////////////////////////////////////////////////////

void Roe::compute_interface_flux_and_wavespeeds(const RealVector& left, const RealVector& right, const RealVector& normal,
                                                RealVector& flux, RealVector& wave_speeds)
{
  compute_interface_flux(left,right,normal,flux);
  wave_speeds = eigenvalues;
}

////////////////////////////////////////////////////////////////////////////////

} // RiemannSolvers
} // CF
