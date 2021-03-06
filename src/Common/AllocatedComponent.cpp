// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include <boost/accumulators/accumulators.hpp>
#include <boost/accumulators/statistics/max.hpp>
#include <boost/accumulators/statistics/mean.hpp>
#include <boost/accumulators/statistics/min.hpp>
#include <boost/accumulators/statistics/stats.hpp>
#include <boost/accumulators/statistics/variance.hpp>

#include "Common/AllocatedComponent.hpp"
#include "Common/CAction.hpp"
#include "Common/Timer.hpp"

namespace CF {
namespace Common {

////////////////////////////////////////////////////////////////////////////////////////////

#ifdef CF_ENABLE_COMPONENT_TIMING

struct TimedActionImpl::Implementation
{
  Implementation(CAction& timed_action) : m_timed_component(timed_action)
  {
    m_timed_component.properties().add_property("timer_count", Uint(0));
    m_timed_component.properties().add_property("timer_minimum", Real(0.));
    m_timed_component.properties().add_property("timer_mean", Real(0.));
    m_timed_component.properties().add_property("timer_maximum", Real(0.));
    m_timed_component.properties().add_property("timer_variance", Real(0.));
  }
  
  Timer m_timer;
  
  boost::accumulators::accumulator_set
  <
    Real,
    boost::accumulators::stats
    <
      boost::accumulators::tag::count,
      boost::accumulators::tag::min,
      boost::accumulators::tag::mean,
      boost::accumulators::tag::max,
      boost::accumulators::tag::lazy_variance
    >
  > m_timing_stats;
  
  CAction& m_timed_component;
};
  
////////////////////////////////////////////////////////////////////////////////////////////

TimedActionImpl::TimedActionImpl(IAction& action) : m_implementation(new Implementation(dynamic_cast<CAction&>(action)))
{
}

TimedActionImpl::~TimedActionImpl()
{
}


void TimedActionImpl::start_timing()
{
  m_implementation->m_timer.restart();
}

void TimedActionImpl::stop_timing()
{
  m_implementation->m_timing_stats(m_implementation->m_timer.elapsed());
}

void TimedActionImpl::store_timings()
{
  m_implementation->m_timed_component.properties().configure_property("timer_count", boost::accumulators::count(m_implementation->m_timing_stats));
  m_implementation->m_timed_component.properties().configure_property("timer_minimum", boost::accumulators::min(m_implementation->m_timing_stats));
  m_implementation->m_timed_component.properties().configure_property("timer_mean", boost::accumulators::mean(m_implementation->m_timing_stats));
  m_implementation->m_timed_component.properties().configure_property("timer_maximum", boost::accumulators::max(m_implementation->m_timing_stats));
  m_implementation->m_timed_component.properties().configure_property("timer_variance", boost::accumulators::lazy_variance(m_implementation->m_timing_stats));
}

#endif

////////////////////////////////////////////////////////////////////////////////////////////

} // Actions
} // CF
