// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_SFDM_P1_Line_hpp
#define CF_SFDM_P1_Line_hpp

#include "SFDM/ShapeFunction.hpp"

namespace CF {
namespace SFDM {
namespace P1 {

class SFDM_API Line : public ShapeFunction {
public:

  typedef boost::shared_ptr<Line>       Ptr;
  typedef boost::shared_ptr<Line const> ConstPtr;


  static const Mesh::GeoShape::Type shape          = Mesh::GeoShape::LINE;
  static const Uint                 nb_nodes       = 2;
  static const Uint                 dimensionality = 1;
  static const Uint                 order          = 1;

public:

  /// Constructor
  Line(const std::string& name = type_name());

  /// Type name
  static std::string type_name() { return "Line"; }

  virtual const ShapeFunction& line() const;
  virtual const ShapeFunction& flux_line() const;

  virtual const RealMatrix& local_coordinates() const;

  virtual void compute_value(const RealVector& local_coordinate, RealRowVector& value) const;
  virtual void compute_gradient(const RealVector& local_coordinate, RealMatrix& gradient) const;

};

} // P1
} // SFDM
} // CF

#endif // CF_SFDM_P1_Line_hpp
