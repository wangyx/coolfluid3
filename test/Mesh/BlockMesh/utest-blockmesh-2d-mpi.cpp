// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE "Test module for CF::Mesh::BlockMesh::BlockMeshMPI"

#include <boost/assign.hpp>
#include <boost/test/unit_test.hpp>

#include "Common/Core.hpp"
#include "Common/Log.hpp"
#include "Common/CRoot.hpp"
#include "Common/PE/Comm.hpp"

#include "Mesh/BlockMesh/BlockData.hpp"
#include "Mesh/CDomain.hpp"
#include "Mesh/CElements.hpp"
#include "Mesh/CMesh.hpp"
#include "Mesh/CMeshWriter.hpp"
#include "Mesh/CRegion.hpp"
#include "Mesh/CSpace.hpp"
#include "Mesh/Field.hpp"
#include "Mesh/FieldGroup.hpp"

using namespace CF;
using namespace CF::Common;
using namespace CF::Mesh;

using namespace boost::assign;

//////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_SUITE( BlockMesh2D )

//////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE( Grid2D )
{
  PE::Comm::instance().init(boost::unit_test::framework::master_test_suite().argc, boost::unit_test::framework::master_test_suite().argv);

  const Uint nb_procs = PE::Comm::instance().size();
  const Uint rank = PE::Comm::instance().rank();

  CMeshWriter::Ptr writer =  build_component_abstract_type<CMeshWriter>("CF.Mesh.VTKXML.CWriter", "writer");
  
  CDomain& domain = Core::instance().root().create_component<CDomain>("domain");
  domain.add_component(writer);

  const Real length = 1.;
  const Real half_height = 1.;
  const Real ratio = 0.2;
  const Uint x_segs = 12;
  const Uint y_segs = 10;

  BlockMesh::BlockData& blocks = domain.create_component<BlockMesh::BlockData>("blocks");

  blocks.dimension = 2;
  blocks.scaling_factor = 1.;

  blocks.points += list_of(0.    )(-half_height)
                 , list_of(length)(-half_height)
                 , list_of(0.    )( 0.         )
                 , list_of(length)( 0.         )
                 , list_of(0.    )( half_height)
                 , list_of(length)( half_height);

  blocks.block_points += list_of(0)(1)(3)(2),
                         list_of(2)(3)(5)(4);
  blocks.block_subdivisions += list_of(x_segs)(y_segs),
                               list_of(x_segs)(y_segs);
  blocks.block_gradings += list_of(1.)(1.)(1./ratio)(1./ratio),
                           list_of(1.)(1.)(ratio)(ratio);

  blocks.patch_names += "left", "right", "top",  "bottom";
  blocks.patch_types += "wall", "wall",  "wall", "wall";
  blocks.patch_points += list_of(2)(0)(4)(2),
                         list_of(1)(3)(3)(5),
                         list_of(5)(4),
                         list_of(0)(1);

  blocks.block_distribution += 0, 2;

  // Test block partitioning
  BlockMesh::BlockData& parallel_blocks = domain.create_component<BlockMesh::BlockData>("parallel_blocks");
  BlockMesh::partition_blocks(blocks, nb_procs, XX, parallel_blocks);

  // Build the mesh
  CMesh& mesh = domain.create_component<CMesh>("mesh");
  BlockMesh::build_mesh(parallel_blocks, mesh);

  // Store element ranks
  FieldGroup& elems_P0 = mesh.create_space_and_field_group("elems_P0",FieldGroup::Basis::ELEMENT_BASED,"CF.Mesh.LagrangeP0");
  Field& elem_rank = elems_P0.create_field("elem_rank");

  boost_foreach(CElements& elements , elems_P0.elements_range())
  {
    CSpace& space = elems_P0.space(elements);
    for (Uint elem=0; elem<elements.size(); ++elem)
    {
      Uint field_idx = space.indexes_for_element(elem)[0];
      elem_rank[field_idx][0] = elements.rank()[elem];
    }
  }

  // Write to disk
  std::vector<Field::Ptr> fields;
  fields.push_back(mesh.geometry().coordinates().as_ptr<Field>());
  fields.push_back(elem_rank.as_ptr<Field>());
  writer->set_fields(fields);
  writer->write_from_to(mesh, URI("utest-blockmesh-2d-mpi_output.pvtu"));
}

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_SUITE_END()

////////////////////////////////////////////////////////////////////////////////

