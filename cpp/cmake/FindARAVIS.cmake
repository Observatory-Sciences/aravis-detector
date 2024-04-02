# Copyright 2024 

# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#
# Tries to find ARAVIS Client headers and libraries.
#
# Usage of this module as follows:
#
#  find_package(ARAVIS)
#
# Variables used by this module, they can change the default behaviour and need
# to be set before calling find_package:
#
#  ARAVIS_ROOT_DIR  Set this variable to the root installation of
#                  	 ARAVIS Library if the module has problems finding
#                    the proper installation path.
#
# Variables defined by this module:
#
#  ARAVIS_FOUND              System has ARAVIS libs/headers
#  ARAVIS_LIBRARIES          The ARAVIS libraries
#  ARAVIS_INCLUDE_DIRS       The location of ARAVIS headers

# You might need to change this depending on what version you want to use
# I'll try to add something to do it for you in the future
set(ARAVIS_VERSIONED "aravis-0.8")

message ("\nLooking for ARAVIS headers and libraries")



if (ARAVIS_ROOT_DIR) 
    message (STATUS "Root dir: ${ARAVIS_ROOT_DIR}")
endif ()

find_package(PkgConfig)
IF (PkgConfig_FOUND)
    message("using pkgconfig")
    pkg_check_modules(PC_ARAVIS ${ARAVIS_VERSIONED})
ENDIF(PkgConfig_FOUND)

set(ARAVIS_DEFINITIONS ${PC_ARAVIS_CFLAGS_OTHER})

find_path(ARAVIS_INCLUDE_DIR 
	NAMES
	arv.h
    PATHS 
		${ARAVIS_ROOT_DIR}/include/${ARAVIS_VERSIONED}/
		${PC_ARAVIS_INCLUDEDIR} 
        ${PC_ARAVIS_INCLUDE_DIRS}
	PATH_SUFFIXES
)

find_library(ARAVIS_LIBRARY
    NAMES 
		${ARAVIS_VERSIONED}
    PATHS 
		${ARAVIS_ROOT_DIR}/lib/x86_64-linux-gnu
        ${PC_ARAVIS_LIBDIR} 
        ${PC_ARAVIS_LIBRARY_DIRS}
)

include(FindPackageHandleStandardArgs)

# handle the QUIETLY and REQUIRED arguments and set ARAVIS_FOUND to TRUE
# if all listed variables are TRUE
find_package_handle_standard_args(ARAVIS 
	DEFAULT_MSG
    ARAVIS_LIBRARY 
	ARAVIS_INCLUDE_DIR
)

mark_as_advanced(ARAVIS_INCLUDE_DIR ARAVIS_LIBRARY)

if (ARAVIS_FOUND)
	set(ARAVIS_INCLUDE_DIRS ${ARAVIS_INCLUDE_DIR})
	set(ARAVIS_LIBRARIES ${ARAVIS_LIBRARY})
	
    get_filename_component(ARAVIS_LIBRARY_DIR ${ARAVIS_LIBRARY} PATH)
    get_filename_component(ARAVIS_LIBRARY_NAME ${ARAVIS_LIBRARY} NAME_WE)
    
    mark_as_advanced(ARAVIS_LIBRARY_DIR ARAVIS_LIBRARY_NAME)

	message (STATUS "Include directories: ${ARAVIS_INCLUDE_DIRS}") 
	message (STATUS "Libraries: ${ARAVIS_LIBRARIES}") 
endif ()
