// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_SFDM_PrepareMesh_hpp
#define CF_SFDM_PrepareMesh_hpp

#include "Solver/ActionDirector.hpp"

#include "SFDM/LibSFDM.hpp"

namespace CF {
namespace SFDM {

class CellTerm;

/////////////////////////////////////////////////////////////////////////////////////

class SFDM_API PrepareMesh : public CF::Solver::ActionDirector {

public: // typedefs

  typedef boost::shared_ptr<PrepareMesh> Ptr;
  typedef boost::shared_ptr<PrepareMesh const> ConstPtr;

public: // functions
  /// Contructor
  /// @param name of the component
  PrepareMesh ( const std::string& name );

  /// Virtual destructor
  virtual ~PrepareMesh() {}

  /// Get the class name
  static std::string type_name () { return "PrepareMesh"; }

  /// execute the action
  virtual void execute ();

};

/////////////////////////////////////////////////////////////////////////////////////


} // SFDM
} // CF

#endif // CF_SFDM_PrepareMesh_hpp
