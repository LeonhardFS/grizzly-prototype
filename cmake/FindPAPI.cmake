# Try to find PAPI headers and libraries.
#
# Usage of this module as follows:
#
#     find_package(PAPI)
#
# Variables used by this module, they can change the default behaviour and need
# to be set before calling find_package:
#
#  PAPI_PREFIX         Set this variable to the root installation of
#                      libpapi if the module has problems finding the
#                      proper installation path.
#
# Variables defined by this module:
#
#  PAPI_FOUND              System has PAPI libraries and headers
#  PAPI_LIBRARIES          The PAPI library
#  PAPI_INCLUDE_DIRS       The location of PAPI headers

## find the papi include directory
find_path(PAPI_INCLUDE_DIRS papi.h
        PATH_SUFFIXES include
        PATHS
        $ENV{CPLUS_INCLUDE_PATH}
        $ENV{C_INCLUDE_PATH}
        /usr/local/include/
        /usr/local
        /usr
        /usr/include/
        /opt
        /opt/local
        ${PAPI_PREFIX}/include/
        ${PAPI_PREFIX})

# find the papi library
find_library(PAPI_LIBRARIES
        NAMES libpapi.so libpapi.a papi
        PATH_SUFFIXES lib64 lib
        PATHS ~/Library/Frameworks
        $ENV{LD_LIBRARY_PATH}
        /Library/Frameworks
        /usr/local
        /usr
        /sw
        /opt/local
        /opt
        ${PAPI_PREFIX}/lib
        ${PAPI_PREFIX})

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(PAPI DEFAULT_MSG
        PAPI_LIBRARIES
        PAPI_INCLUDE_DIRS
        )

mark_as_advanced(
        PAPI_PREFIX_DIRS
        PAPI_LIBRARIES
        PAPI_INCLUDE_DIRS
)
