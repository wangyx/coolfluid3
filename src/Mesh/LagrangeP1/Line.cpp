// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "Common/CBuilder.hpp"

#include "Mesh/ShapeFunctionT.hpp"
#include "Mesh/LagrangeP1/LibLagrangeP1.hpp"
#include "Mesh/LagrangeP1/Line.hpp"

namespace CF {
namespace Mesh {
namespace LagrangeP1 {

////////////////////////////////////////////////////////////////////////////////

Common::ComponentBuilder < ShapeFunctionT<Line>, ShapeFunction, LibLagrangeP1 >
   Line_Builder(LibLagrangeP1::library_namespace()+"."+Line::type_name());

////////////////////////////////////////////////////////////////////////////////

void Line::compute_value(const MappedCoordsT& mapped_coord, ValueT& result)
{
  result[0] = 0.5 * (1.0 - mapped_coord[KSI]);
  result[1] = 0.5 * (1.0 + mapped_coord[KSI]);
}

////////////////////////////////////////////////////////////////////////////////

void Line::compute_gradient(const MappedCoordsT& mapped_coord, GradientT& result)
{
  result(KSI, 0) = -0.5;
  result(KSI, 1) =  0.5;
}

////////////////////////////////////////////////////////////////////////////////

const RealMatrix& Line::local_coordinates()
{
  static const RealMatrix loc_coord =
      (RealMatrix(nb_nodes, dimensionality) <<

       -1.,
        1.

       ).finished();
  return loc_coord;
}

////////////////////////////////////////////////////////////////////////////////

} // LagrangeP1
} // Mesh
} // CF
