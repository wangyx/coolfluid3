// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_Mesh_LagrangeP1_Hexa_hpp
#define CF_Mesh_LagrangeP1_Hexa_hpp

#include "Mesh/ShapeFunctionBase.hpp"
#include "Mesh/LagrangeP1/API.hpp"

namespace CF {
namespace Mesh {
namespace LagrangeP1 {

////////////////////////////////////////////////////////////////////////////////

struct Mesh_LagrangeP1_API Hexa_traits
{
  enum { nb_nodes       = 8              };
  enum { dimensionality = 3              };
  enum { order          = 1              };
  enum { shape          = GeoShape::HEXA };
};

////////////////////////////////////////////////////////////////////////////////

/// @class Hexa
/// @verbatim
/// Local connectivity:
///                3-----------2
///               /|          /|
///              / |         / |
///             7-----------6  |
///             |  |        |  |
///             |  |        |  |
///             |  0--------|--1
///             | /         | /
///     ETA     |/          |/
///      |      4-----------5
///      |
///      o--- KSI
///     /
///    /
///  ZTA
/// Reference domain: <-1,1> x <-1,1> x <-1,1>
/// @endverbatim
/// @see ShapeFunction for documentation on undocumented static functions
struct Mesh_LagrangeP1_API Hexa : public ShapeFunctionBase<Hexa,Hexa_traits>
{
  enum FaceNumbering { ZTA_NEG, ZTA_POS, ETA_NEG, KSI_POS, ETA_POS, KSI_NEG};

  static const RealMatrix& local_coordinates();
  static void compute_value(const MappedCoordsT& mapped_coord, ValueT& result);
  static void compute_gradient(const MappedCoordsT& mapped_coord, GradientT& result);
};

////////////////////////////////////////////////////////////////////////////////

} // LagrangeP1
} // Mesh
} // CF

#endif // CF_Mesh_LagrangeP1_Hexa_hpp
