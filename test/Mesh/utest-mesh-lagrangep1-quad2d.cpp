// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE "Test module for the ETYPE shapefunction"

#include <boost/assign/list_of.hpp>
#include <boost/test/unit_test.hpp>

#include "Common/Log.hpp"
#include "Common/CRoot.hpp"

#include "Mesh/CTable.hpp"
#include "Mesh/Geometry.hpp"
#include "Mesh/Integrators/Gauss.hpp"
#include "Mesh/LagrangeP1/Quad2D.hpp"
#include "Mesh/CElements.hpp"

#include "Tools/Testing/Difference.hpp"

using namespace boost::assign;
using namespace CF;
using namespace CF::Common;
using namespace CF::Mesh;
using namespace CF::Mesh::Integrators;
using namespace CF::Mesh::LagrangeP1;
using namespace CF::Tools::Testing;

//////////////////////////////////////////////////////////////////////////////

typedef Quad2D ETYPE;

struct LagrangeP1Quad2DFixture
{
  typedef ETYPE::NodesT NodesT;
  /// common setup for each test case
  LagrangeP1Quad2DFixture() : mapped_coords(0.1, 0.8), nodes((NodesT() << 0.5,  0.3,
                                                                                    1.1,  1.2,
                                                                                    1.35, 1.9,
                                                                                    0.8,  2.1).finished())
  {
  }

  /// common tear-down for each test case
  ~LagrangeP1Quad2DFixture()
  {
  }
  /// common values accessed by all tests goes here

  const ETYPE::MappedCoordsT mapped_coords;
  const NodesT nodes;

  struct ConstFunctor
  {
    ConstFunctor(const NodesT& node_list) : m_nodes(node_list) {}

    Real operator()() const
    {
      return ETYPE::jacobian_determinant(mapped_coords, m_nodes);
    }
    ETYPE::MappedCoordsT mapped_coords;
  private:
    const NodesT& m_nodes;
  };
};

//////////////////////////////////////////////////////////////////////////////

BOOST_FIXTURE_TEST_SUITE( ETYPESuite, LagrangeP1Quad2DFixture )

//////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE( Volume )
{
  ETYPE::NodesT nodes_quad2D;
  nodes_quad2D <<
    0.0, 0.0,
    1.0, 0.0,
    1.0, 1.0,
    0.0, 1.0;
  BOOST_CHECK_EQUAL(ETYPE::volume(nodes_quad2D), 1.0);
}

BOOST_AUTO_TEST_CASE( Element )
{
  // Create a CElements component
  CElements::Ptr comp = allocate_component<CElements>("comp");
  Geometry::Ptr nodes = allocate_component<Geometry>("nodes");
  comp->initialize("CF.Mesh.LagrangeP1.Quad2D",*nodes);
  BOOST_CHECK_EQUAL(comp->element_type().shape(), GeoShape::QUAD);
  BOOST_CHECK_EQUAL(comp->element_type().nb_faces(), (Uint) 4);

  // Check volume calculation
  ETYPE::NodesT coord;
  coord <<
    15, 15,
    40, 25,
    25, 30,
    30, 40;

  BOOST_CHECK_EQUAL(ETYPE::volume(coord), 150.);
}

BOOST_AUTO_TEST_CASE( computeShapeFunction )
{
  const ETYPE::SF::ValueT reference_result(0.045, 0.055, 0.495, 0.405);
  ETYPE::SF::ValueT result;
  ETYPE::SF::compute_value(mapped_coords, result);
  Accumulator accumulator;
  vector_test(result, reference_result, accumulator);
  BOOST_CHECK_LT(boost::accumulators::max(accumulator.ulps), 10); // Maximal difference can't be greater than 10 times the least representable unit
}

BOOST_AUTO_TEST_CASE( computeMappedCoordinates )
{
  Accumulator accumulator;
  ETYPE::CoordsT test_coords(0.9375, 1.375); // center of the element
  ETYPE::MappedCoordsT result;
  ETYPE::compute_mapped_coordinate(test_coords, nodes, result);
  BOOST_CHECK_LT(std::abs(result[0]), 1e-12);
  BOOST_CHECK_LT(std::abs(result[1]), 1e-12);// sqrt from the expression gives too many ULPS in difference for Accumulator

  test_coords = nodes.row(0);
  ETYPE::compute_mapped_coordinate(test_coords, nodes, result);
  vector_test(result,ETYPE::MappedCoordsT(-1.0, -1.0),accumulator);
  BOOST_CHECK_LT(boost::accumulators::max(accumulator.ulps), 600);
  CFinfo << "result[0] = " << result[0] << CFendl;
  CFinfo << "result[1] = " << result[1] << CFendl << CFendl;

  test_coords = nodes.row(1);
  ETYPE::compute_mapped_coordinate(test_coords, nodes, result);
  vector_test(result,ETYPE::MappedCoordsT(1.0, -1.0),accumulator);
  BOOST_CHECK_LT(boost::accumulators::max(accumulator.ulps), 600);
  CFinfo << "result[0] = " << result[0] << CFendl;
  CFinfo << "result[1] = " << result[1] << CFendl << CFendl;

  test_coords = nodes.row(2);
  ETYPE::compute_mapped_coordinate(test_coords, nodes, result);
  vector_test(result,ETYPE::MappedCoordsT(1.0, 1.0),accumulator);
  BOOST_CHECK_LT(boost::accumulators::max(accumulator.ulps), 600);
  CFinfo << "result[0] = " << result[0] << CFendl;
  CFinfo << "result[1] = " << result[1] << CFendl << CFendl;

  test_coords = nodes.row(3);
  ETYPE::compute_mapped_coordinate(test_coords, nodes, result);
  vector_test(result,ETYPE::MappedCoordsT(-1.0, 1.0),accumulator);
  BOOST_CHECK_LT(boost::accumulators::max(accumulator.ulps), 600);
  CFinfo << "result[0] = " << result[0] << CFendl;
  CFinfo << "result[1] = " << result[1] << CFendl << CFendl;

  // Try another element
  ETYPE::NodesT nodes_2;
  nodes_2 <<
    1.0, 1.0,
    2.0, 1.0,
    2.0, 2.0,
    1.0, 2.0;

  test_coords = nodes_2.row(0);
  ETYPE::compute_mapped_coordinate(test_coords, nodes_2, result);
  CFinfo << "result[0] = " << result[0] << CFendl;
  CFinfo << "result[1] = " << result[1] << CFendl << CFendl;
  vector_test(result,ETYPE::MappedCoordsT(-1.0, -1.0),accumulator);
  BOOST_CHECK_LT(boost::accumulators::max(accumulator.ulps), 600);

  test_coords = nodes_2.row(1);
  ETYPE::compute_mapped_coordinate(test_coords, nodes_2, result);
  vector_test(result,ETYPE::MappedCoordsT(1.0, -1.0),accumulator);
  BOOST_CHECK_LT(boost::accumulators::max(accumulator.ulps), 600);
  CFinfo << "result[0] = " << result[0] << CFendl;
  CFinfo << "result[1] = " << result[1] << CFendl << CFendl;

  test_coords = nodes_2.row(2);
  ETYPE::compute_mapped_coordinate(test_coords, nodes_2, result);
  vector_test(result,ETYPE::MappedCoordsT(1.0, 1.0),accumulator);
  BOOST_CHECK_LT(boost::accumulators::max(accumulator.ulps), 600);
  CFinfo << "result[0] = " << result[0] << CFendl;
  CFinfo << "result[1] = " << result[1] << CFendl << CFendl;

  test_coords = nodes_2.row(3);
  ETYPE::compute_mapped_coordinate(test_coords, nodes_2, result);
  vector_test(result,ETYPE::MappedCoordsT(-1.0, 1.0),accumulator);
  BOOST_CHECK_LT(boost::accumulators::max(accumulator.ulps), 600);
  CFinfo << "result[0] = " << result[0] << CFendl;
  CFinfo << "result[1] = " << result[1] << CFendl << CFendl;
}

BOOST_AUTO_TEST_CASE( computeMappedGradient )
{
  ETYPE::SF::GradientT expected;
  const CF::Real ksi  = mapped_coords[0];
  const CF::Real eta = mapped_coords[1];
  expected(0,0) = 0.25 * (-1 + eta);
  expected(1,0) = 0.25 * (-1 + ksi);
  expected(0,1) = 0.25 * ( 1 - eta);
  expected(1,1) = 0.25 * (-1 - ksi);
  expected(0,2) = 0.25 * ( 1 + eta);
  expected(1,2) = 0.25 * ( 1 + ksi);
  expected(0,3) = 0.25 * (-1 - eta);
  expected(1,3) = 0.25 * ( 1 - ksi);
  ETYPE::SF::GradientT result;
  ETYPE::SF::compute_gradient(mapped_coords, result);
  Accumulator accumulator;
  vector_test(result, expected, accumulator);
  BOOST_CHECK_LT(boost::accumulators::max(accumulator.ulps), 2);
}

BOOST_AUTO_TEST_CASE( computeJacobianDeterminant )
{
  // Shapefunction determinant at center should be a quarter of the cell volume
  const ETYPE::CoordsT center_coords(0., 0.);
  BOOST_CHECK_LT(boost::accumulators::max(test(4.*ETYPE::jacobian_determinant(center_coords, nodes), ETYPE::volume(nodes)).ulps), 1);
}

BOOST_AUTO_TEST_CASE( computeJacobian )
{
  ETYPE::JacobianT expected;
  expected(0,0) = 0.2775;
  expected(0,1) = -0.045;
  expected(1,0) = 0.13625;
  expected(1,1) = 0.5975;
  ETYPE::JacobianT result;
  ETYPE::compute_jacobian(mapped_coords, nodes, result);
  Accumulator accumulator;
  vector_test(result, expected, accumulator);
  BOOST_CHECK_LT(boost::accumulators::max(accumulator.ulps), 15);


  ETYPE::NodesT nodes;
  nodes << 0. , 0. ,    1. , 0.,   1. , 1. ,   0. , 1.;
  ETYPE::MappedCoordsT d_mapped;  d_mapped << 2. , 2.;
  ETYPE::MappedCoordsT mapped; mapped << 0.5, 0.5;
  ETYPE::JacobianT jacobian;
  ETYPE::compute_jacobian(mapped, nodes, jacobian);

  RealVector2 d_phys = jacobian * d_mapped;
  BOOST_CHECK_EQUAL( d_phys[0], 1. );
  BOOST_CHECK_EQUAL( d_phys[1], 1. );
}

BOOST_AUTO_TEST_CASE( computeJacobianAdjoint )
{
  ETYPE::JacobianT expected;
  expected(0,0) = 0.5975;
  expected(0,1) = 0.045;
  expected(1,0) = -0.13625;
  expected(1,1) = 0.2775;
  ETYPE::JacobianT result(2, 2);
  ETYPE::compute_jacobian_adjoint(mapped_coords, nodes, result);
  Accumulator accumulator;
  vector_test(result, expected, accumulator);
  BOOST_CHECK_LT(boost::accumulators::max(accumulator.ulps), 15);
}

BOOST_AUTO_TEST_CASE( integrateConst )
{
  // Shapefunction determinant should be double the volume for triangles
  ConstFunctor ftor(nodes);
  const Real vol = ETYPE::volume(nodes);

  CF::Real result1 = 0.0;
  CF::Real result2 = 0.0;
  CF::Real result4 = 0.0;
  CF::Real result8 = 0.0;
  CF::Real result16 = 0.0;
  CF::Real result32 = 0.0;

  gauss_integrate<1, GeoShape::QUAD>(ftor, ftor.mapped_coords, result1);
  gauss_integrate<2, GeoShape::QUAD>(ftor, ftor.mapped_coords, result2);
  gauss_integrate<4, GeoShape::QUAD>(ftor, ftor.mapped_coords, result4);
  gauss_integrate<8, GeoShape::QUAD>(ftor, ftor.mapped_coords, result8);
  gauss_integrate<16, GeoShape::QUAD>(ftor, ftor.mapped_coords, result16);
  gauss_integrate<32, GeoShape::QUAD>(ftor, ftor.mapped_coords, result32);

  BOOST_CHECK_LT(boost::accumulators::max(test(result1, vol).ulps), 1);
  BOOST_CHECK_LT(boost::accumulators::max(test(result2, vol).ulps), 15);
  BOOST_CHECK_LT(boost::accumulators::max(test(result4, vol).ulps), 15);
  BOOST_CHECK_LT(boost::accumulators::max(test(result8, vol).ulps), 15);
  BOOST_CHECK_LT(boost::accumulators::max(test(result16, vol).ulps), 15);
  BOOST_CHECK_LT(boost::accumulators::max(test(result32, vol).ulps), 15);
}

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_SUITE_END()

////////////////////////////////////////////////////////////////////////////////

