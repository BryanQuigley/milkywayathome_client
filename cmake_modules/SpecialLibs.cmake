# Copyright 2010 Matthew Arsenault, Travis Desell, Dave Przybylo,
# Nathan Cole, Boleslaw Szymanski, Heidi Newberg, Carlos Varela, Malik
# Magdon-Ismail and Rensselaer Polytechnic Institute.

# This file is part of Milkway@Home.

# Milkyway@Home is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.

# Milkyway@Home is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.

# You should have received a copy of the GNU General Public License
# along with Milkyway@Home.  If not, see <http://www.gnu.org/licenses/>.
#

# For releases, we want to be able to statically link as much as
# possible. This requires special handling on OS X since Apple
# doesn't like you trying to statically link the standard libraries.
# We also have to link as C++ when we do this because of BOINC.

macro(set_os_specific_libs cl_required)
  if(APPLE)
    # OpenCL is 10.6+ feature
    if(${cl_required} MATCHES "ON")
      set(CMAKE_OSX_DEPLOYMENT_TARGET 10.6)
      set(CMAKE_OSX_SYSROOT "/Developer/SDKs/MacOSX10.6.sdk")
    else()
      # Try to avoid the dyld: unknown required load command 0x80000022
      # runtime error on Leopard for binaries built on 10.6
      set(CMAKE_OSX_DEPLOYMENT_TARGET 10.5)
      set(CMAKE_OSX_SYSROOT "/Developer/SDKs/MacOSX10.5.sdk")
    endif()

    find_library(COREFOUNDATION_LIBRARY CoreFoundation )
    list(APPEND OS_SPECIFIC_LIBS ${COREFOUNDATION_LIBRARY})
  endif()

  if(WIN32)
    set(OS_SPECIFIC_LIBS msvcrt)
  endif()
endmacro()
