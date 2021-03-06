// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3.
// See doc/lgpl.txt and doc/gpl.txt for the license text.

/// @file AllocatedComponent.hpp
/// @brief Wrapper around component, ensuring proper allocation and type identification, as well as optional function wrapping

#ifndef CF_Common_AllocatedComponent_hpp
#define CF_Common_AllocatedComponent_hpp

////////////////////////////////////////////////////////////////////////////////////////////

#include <boost/scoped_ptr.hpp>
#include <boost/mpl/and.hpp>
#include <boost/mpl/if.hpp>
#include <boost/type_traits/is_base_of.hpp>

#include "Common/CF.hpp"
#include "Common/TypeInfo.hpp"

namespace CF {
namespace Common {

/// Wraps a component that has been allocated using Common::allocate_component.
/// This class implements derived_type_name, making it transparent to the portable typing system
/// and ensuring at compile time that components must be properly allocated
/// This class also provides some possibilities to wrap certain component functions, allowing transparent
/// addition of features like timing
/// This class should never be used directly, use the component interface instead
template<typename ComponentT>
class AllocatedComponent : public ComponentT
{
public:
  AllocatedComponent(const std::string& name) : ComponentT(name)
  {
  }

  /// Component::derived_type_name implementation
  std::string derived_type_name() const
  {
    return TypeInfo::instance().portable_types[ typeid(ComponentT).name() ];
  }
};

#ifdef CF_ENABLE_COMPONENT_TIMING
}
}

#include "Common/IAction.hpp"
#include "Common/TimedComponent.hpp"

namespace CF {
namespace Common {

////////////////////////////////////////////////////////////////////////////////////////////
// IAction timing
////////////////////////////////////////////////////////////////////////////////////////////

/// For internal use, allows non-template parts of TimedAction to be in the .cpp
struct TimedActionImpl
{
  TimedActionImpl(IAction& action);
  ~TimedActionImpl();
  
  void start_timing();
  void stop_timing();
  void store_timings();
  
  // Avoid dragging in the timer-related headers
  class Implementation;
  boost::scoped_ptr<Implementation> m_implementation;
};

/// Alternative wrapper that adds timing functionality around the execute() function for IAction
template<typename ComponentT>
class TimedAction : public ComponentT, public TimedComponent
{
public:
  TimedAction(const std::string& name) : ComponentT(name), m_impl(*this)
  {
  }

  /// Component::derived_type_name implementation
  std::string derived_type_name() const
  {
    return TypeInfo::instance().portable_types[ typeid(ComponentT).name() ];
  }
  
  inline void execute()
  {
    m_impl.start_timing();
    ComponentT::execute();
    m_impl.stop_timing();
  }
  
  inline void store_timings()
  {
    m_impl.store_timings();
  }
  
  TimedActionImpl m_impl;
};

template<typename T>
struct is_timeable : boost::true_type
{
  typedef boost::true_type type;
};

/// Helper struct to select the correct wrapper for a component
template<typename ComponentT>
struct SelectComponentWrapper
{
  typedef typename boost::mpl::if_
  <
    boost::mpl::and_< boost::is_base_of<IAction, ComponentT>, is_timeable<ComponentT> >,
    TimedAction<ComponentT>,
    AllocatedComponent<ComponentT>
  >::type type;
};

#else

/// Helper struct to select the correct wrapper for a component
template<typename ComponentT>
struct SelectComponentWrapper
{
  typedef AllocatedComponent<ComponentT> type;
};

#endif


} // Common
} // CF

#endif // CF_Common_AllocatedComponent_hpp
