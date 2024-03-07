#
# Copyright 2024
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#    http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#
# Tries to find GLIB Client headers and libraries.
#
# Usage of this module as follows:
#
#  find_package(GLIB)
#
# Variables used by this module, they can change the default behaviour and need
# to be set before calling find_package:
#
#  GLIB_ROOT_DIR  Set this variable to the root installation of
#                  	 GLIB Library if the module has problems finding
#                    the proper installation path.
#
# Variables defined by this module:
#
#  GLIB_FOUND              System has GLIB libs/headers
#  GLIB_LIBRARIES          The GLIB libraries
#  GLIB_INCLUDE_DIRS       The location of GLIB headers

message ("\nLooking for GLIB headers and libraries")

if (GLIB_ROOT_DIR) 
    message (STATUS "Root dir: ${GLIB_ROOT_DIR}")
endif ()

find_package(PkgConfig)
IF (PkgConfig_FOUND)
    message("using pkgconfig")
    pkg_check_modules(PC_GLIB glib)
ENDIF(PkgConfig_FOUND)

set(GLIB_DEFINITIONS ${PC_GLIB_CFLAGS_OTHER})

find_path(GLIB_INCLUDE_DIR_1
	NAMES
		glib.h
    PATHS 
		${GLIB_ROOT_DIR}/include
        ${PC_GLIB_INCLUDEDIR} 
        ${PC_GLIB_INCLUDE_DIRS}
    PATH_SUFFIXES 
		glib-2.0
)

find_path(GLIB_INCLUDE_DIR_2
	NAMES
		glibconfig.h
    PATHS 
		${GLIB_ROOT_DIR}/lib/x86_64-linux-gnu/glib-2.0/include
        ${PC_GLIB_INCLUDEDIR} 
        ${PC_GLIB_INCLUDE_DIRS}
)


find_library(GLIB_LIBRARY
    NAMES 
		glib-2.0
    PATHS 
		${GLIB_ROOT_DIR}/lib/x86_64-linux-gnu/glib-2.0/
        ${PC_GLIB_LIBDIR} 
        ${PC_GLIB_LIBRARY_DIRS}
)

include(FindPackageHandleStandardArgs)

# handle the QUIETLY and REQUIRED arguments and set GLIB_FOUND to TRUE
# if all listed variables are TRUE
find_package_handle_standard_args(GLIB 
	DEFAULT_MSG
    GLIB_LIBRARY 
	GLIB_INCLUDE_DIR_1
  GLIB_INCLUDE_DIR_2
)

mark_as_advanced(GLIB_INCLUDE_DIR GLIB_LIBRARY)

if (GLIB_FOUND)
	set(GLIB_INCLUDE_DIRS ${GLIB_INCLUDE_DIR_1} ${GLIB_INCLUDE_DIR_2})
	set(GLIB_LIBRARIES ${GLIB_LIBRARY})
	
    get_filename_component(GLIB_LIBRARY_DIR ${GLIB_LIBRARY} PATH)
    get_filename_component(GLIB_LIBRARY_NAME ${GLIB_LIBRARY} NAME_WE)
    
    mark_as_advanced(GLIB_LIBRARY_DIR GLIB_LIBRARY_NAME)

	message (STATUS "Include directories: ${GLIB_INCLUDE_DIRS}") 
	message (STATUS "Libraries: ${GLIB_LIBRARIES}") 
endif ()
