# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file LICENSE.rst or https://cmake.org/licensing for details.

cmake_minimum_required(VERSION ${CMAKE_VERSION}) # this file comes with cmake

# If CMAKE_DISABLE_SOURCE_CHANGES is set to true and the source directory is an
# existing directory in our source tree, calling file(MAKE_DIRECTORY) on it
# would cause a fatal error, even though it would be a no-op.
if(NOT EXISTS "/home/goodbyte/Projects/DevAcademy/ncs-fund/l5/l5_e1")
  file(MAKE_DIRECTORY "/home/goodbyte/Projects/DevAcademy/ncs-fund/l5/l5_e1")
endif()
file(MAKE_DIRECTORY
  "/home/goodbyte/Projects/DevAcademy/ncs-fund/l5/l5_e1/build/l5_e1"
  "/home/goodbyte/Projects/DevAcademy/ncs-fund/l5/l5_e1/build/_sysbuild/sysbuild/images/l5_e1-prefix"
  "/home/goodbyte/Projects/DevAcademy/ncs-fund/l5/l5_e1/build/_sysbuild/sysbuild/images/l5_e1-prefix/tmp"
  "/home/goodbyte/Projects/DevAcademy/ncs-fund/l5/l5_e1/build/_sysbuild/sysbuild/images/l5_e1-prefix/src/l5_e1-stamp"
  "/home/goodbyte/Projects/DevAcademy/ncs-fund/l5/l5_e1/build/_sysbuild/sysbuild/images/l5_e1-prefix/src"
  "/home/goodbyte/Projects/DevAcademy/ncs-fund/l5/l5_e1/build/_sysbuild/sysbuild/images/l5_e1-prefix/src/l5_e1-stamp"
)

set(configSubDirs )
foreach(subDir IN LISTS configSubDirs)
    file(MAKE_DIRECTORY "/home/goodbyte/Projects/DevAcademy/ncs-fund/l5/l5_e1/build/_sysbuild/sysbuild/images/l5_e1-prefix/src/l5_e1-stamp/${subDir}")
endforeach()
if(cfgdir)
  file(MAKE_DIRECTORY "/home/goodbyte/Projects/DevAcademy/ncs-fund/l5/l5_e1/build/_sysbuild/sysbuild/images/l5_e1-prefix/src/l5_e1-stamp${cfgdir}") # cfgdir has leading slash
endif()
