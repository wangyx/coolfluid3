// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include <iostream>
#include <cstdio>        // for printf()
#include <cstdlib>       // for free() and abort()
#include <csignal>       // POSIX signal(), SIGFPE and SIGSEGV
#include <fenv.h>        // floating environment access
#include <sstream>       // streamstring
#include <execinfo.h>    // for backtrace() from glibc
#include <sys/types.h>   // for getting the PID of the process


#include <mach/mach_types.h>
#include <mach/mach_init.h>
#include <mach/task.h>
#include <mach/mach_host.h>
#include <mach/vm_map.h>
#include <mach/shared_memory_server.h>

#include "Common/BasicExceptions.hpp"
#include "Common/CommonAPI.hpp"
#include "Common/MacOSX/OSystemLayer.hpp"


#ifdef CF_HAVE_CXXABI_H
#include <cxxabi.h>
#include <boost/regex.hpp>
#endif

////////////////////////////////////////////////////////////////////////////////

using namespace std;

namespace CF {
  namespace Common {
    namespace MacOSX {

////////////////////////////////////////////////////////////////////////////////

OSystemLayer::OSystemLayer()
{
}

////////////////////////////////////////////////////////////////////////////////

OSystemLayer::~OSystemLayer()
{
}

////////////////////////////////////////////////////////////////////////////////

std::string OSystemLayer::back_trace () const
{
  return dump_back_trace ();
}

////////////////////////////////////////////////////////////////////////////////

std::string OSystemLayer::dump_back_trace ()
{
  #define BUFFER_SIZE 500

  static int i = 0;
  ++i;


  std::ostringstream oss;
  int j, nptrs;
  void *buffer[BUFFER_SIZE];
  char **strings;

	//printf ("dumping %d backtrace ...\n", i);
	oss << "dumping " << i << " backtrace ...\n";
	nptrs = backtrace(buffer, BUFFER_SIZE);
	oss << "\nbacktrace() returned " << nptrs << " addresses\n";

  strings = backtrace_symbols(buffer, nptrs);
  if (strings == NULL)
    oss << "\nno backtrace_symbols found\n";

#ifdef CF_HAVE_CXXABI_H

	boost::regex e("([0-9]+)[[:space:]]+(.+)[[:space:]]+(.+)[[:space:]]+(.+)[[:space:]]+\\+[[:space:]]+(.+)");
	boost::match_results<std::string::const_iterator> what;

	// iterate over the returned symbol lines. skip the first, it is the
	// address of this function.
	for (j = 1; j < nptrs; j++)
	{
		std::string trace(strings[j]);

		if (boost::regex_search(trace,what,e))
		{
			trace = std::string(what[4].first,what[4].second);
			size_t maxName = 1024;
			int demangleStatus;

			char* demangledName = (char*) malloc(maxName);
			if ((demangledName = abi::__cxa_demangle(trace.c_str(), demangledName, &maxName,
																							 &demangleStatus)) && demangleStatus == 0)
			{
				trace = demangledName; // the demangled name is now in our trace string
			}
			free(demangledName);
		}
		oss << trace << "\n";
	}
#else
	for (j = 0; j < nptrs; j++)
		oss << strings[j] << "\n";
#endif

  free(strings);

  #undef BUFFER_SIZE

  oss << "\nexit dumping backtrace ...";

  return oss.str();
}

////////////////////////////////////////////////////////////////////////////////

Uint OSystemLayer::process_id() const
{
  pid_t pid = getpid();
  return static_cast<Uint> ( pid );
}

////////////////////////////////////////////////////////////////////////////////

double OSystemLayer::memory_usage() const
{

  // Copyright (c) 2002 Aram Greenman. All rights reserved.
  //
  // Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:
  //
  // 1. Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
  // 2. Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
  // 3. The name of the author may not be used to endorse or promote products derived from this software without specific prior written permission.
  //
  // THIS SOFTWARE IS PROVIDED BY THE AUTHOR "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
  // http://alt.textdrive.com/svn/altdev/ZOE/Applications/ZOEMenu/AGProcess.m

  struct task_basic_info         t_info;
  struct host_basic_info         h_info;
  struct vm_region_basic_info_64 vm_info;


  mach_msg_type_number_t t_info_count = TASK_BASIC_INFO_COUNT;
  mach_msg_type_number_t h_info_count = HOST_BASIC_INFO_COUNT;
  mach_msg_type_number_t vm_info_count = VM_REGION_BASIC_INFO_COUNT_64;


  vm_address_t address = GLOBAL_SHARED_TEXT_SEGMENT;

  vm_size_t size;
  mach_port_t object_name;


  if (KERN_SUCCESS != task_info(mach_task_self(),
                                TASK_BASIC_INFO, (task_info_t)&t_info,
                                &t_info_count))
  {
    return -1;
  }

  if (KERN_SUCCESS != host_info(mach_host_self(),
                                HOST_BASIC_INFO, (host_info_t)&h_info,
                                &h_info_count))
  {
    return -1;
  }

  if (KERN_SUCCESS != vm_region_64(mach_task_self(), &address, &size,
                                   VM_REGION_BASIC_INFO, (vm_region_info_t)&vm_info,
                                   &vm_info_count, &object_name))
  {
    return -1;
  }

  if (vm_info.reserved && size == SHARED_TEXT_REGION_SIZE && t_info.virtual_size > (SHARED_TEXT_REGION_SIZE + SHARED_DATA_REGION_SIZE))
    t_info.virtual_size -= (SHARED_TEXT_REGION_SIZE + SHARED_DATA_REGION_SIZE);



  // virtual size
  // return static_cast<double>(t_info.virtual_size);

  // resident size
  return static_cast<double>(t_info.resident_size);

  // percent
  // return static_cast<double>(t_info.resident_size) / h_info.memory_size;
}

////////////////////////////////////////////////////////////////////////////////

/// Following functions are required since they are not available for Mac OSX
/// This only works for intel architecture
/// http://www-personal.umich.edu/~williams/archive/computation/fe-handling-example.c

#if 0 // not used for the moment
static int
fegetexcept (void)
{
  static fenv_t fenv;

  return fegetenv (&fenv) ? -1 : (fenv.__control & FE_ALL_EXCEPT);
}
#endif

static int
feenableexcept (unsigned int excepts)
{
  static fenv_t fenv;
  unsigned int new_excepts = excepts & FE_ALL_EXCEPT,
               old_excepts;  // previous masks

  if ( fegetenv (&fenv) ) return -1;
  old_excepts = fenv.__control & FE_ALL_EXCEPT;

  // unmask
  fenv.__control &= ~new_excepts;
  fenv.__mxcsr   &= ~(new_excepts << 7);

  return ( fesetenv (&fenv) ? -1 : (int)old_excepts );
}

#if 0 // not used for the moment
static int
fedisableexcept (unsigned int excepts)
{
  static fenv_t fenv;
  unsigned int new_excepts = excepts & FE_ALL_EXCEPT,
               old_excepts;  // all previous masks

  if ( fegetenv (&fenv) ) return -1;
  old_excepts = fenv.__control & FE_ALL_EXCEPT;

  // mask
  fenv.__control |= new_excepts;
  fenv.__mxcsr   |= new_excepts << 7;

  return ( fesetenv (&fenv) ? -1 : old_excepts );
}
#endif

////////////////////////////////////////////////////////////////////////////////

void OSystemLayer::regist_os_signal_handlers()
{
  // register handler functions for the signals
  signal(SIGFPE,    (sighandler_t) MacOSX::OSystemLayer::handleSIGFPE);
  signal(SIGSEGV,   (sighandler_t) MacOSX::OSystemLayer::handleSIGSEGV);
  signal(SIGABRT,   (sighandler_t) MacOSX::OSystemLayer::handleSIGABRT);

  // enable the exceptions that will raise the SIGFPE signal
  feenableexcept ( FE_DIVBYZERO );
  //feenableexcept ( FE_OVERFLOW  );
  //feenableexcept ( FE_UNDERFLOW );
  // feenableexcept ( FE_INVALID   );
  // feenableexcept ( FE_INEXACT   );
}

////////////////////////////////////////////////////////////////////////////////

int OSystemLayer::handleSIGFPE (int signal)
{
  throw Common::FloatingPointError (FromHere(), "Some floating point operation has given an invalid result");
  return SIGFPE;
}

////////////////////////////////////////////////////////////////////////////////

int OSystemLayer::handleSIGSEGV(int signal)
{
  printf("\nreceived signal SIGSEGV [%d] - 'Segmentation violation'\n",signal);
  static std::string dump = MacOSX::OSystemLayer::dump_back_trace();
  printf( "%s\n", dump.c_str() );
  return SIGSEGV;
}

////////////////////////////////////////////////////////////////////////////////

int OSystemLayer::handleSIGABRT(int signal)
{
  printf("\nreceived signal SIGABRT [%d] - 'abort'\n",signal);
  //static std::string dump = MacOSX::OSystemLayer::dump_back_trace();
  //printf( "%s\n", dump.c_str() );
  return SIGABRT;
}

////////////////////////////////////////////////////////////////////////////////

    } // MacOSX
  } // Common
} // CF

