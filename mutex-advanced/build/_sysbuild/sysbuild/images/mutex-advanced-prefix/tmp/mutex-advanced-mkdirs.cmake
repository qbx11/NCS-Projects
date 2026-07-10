# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file LICENSE.rst or https://cmake.org/licensing for details.

cmake_minimum_required(VERSION ${CMAKE_VERSION}) # this file comes with cmake

# If CMAKE_DISABLE_SOURCE_CHANGES is set to true and the source directory is an
# existing directory in our source tree, calling file(MAKE_DIRECTORY) on it
# would cause a fatal error, even though it would be a no-op.
if(NOT EXISTS "/home/goodbyte/Projects/NCS-Projects/mutex-advanced")
  file(MAKE_DIRECTORY "/home/goodbyte/Projects/NCS-Projects/mutex-advanced")
endif()
file(MAKE_DIRECTORY
  "/home/goodbyte/Projects/NCS-Projects/mutex-advanced/build/mutex-advanced"
  "/home/goodbyte/Projects/NCS-Projects/mutex-advanced/build/_sysbuild/sysbuild/images/mutex-advanced-prefix"
  "/home/goodbyte/Projects/NCS-Projects/mutex-advanced/build/_sysbuild/sysbuild/images/mutex-advanced-prefix/tmp"
  "/home/goodbyte/Projects/NCS-Projects/mutex-advanced/build/_sysbuild/sysbuild/images/mutex-advanced-prefix/src/mutex-advanced-stamp"
  "/home/goodbyte/Projects/NCS-Projects/mutex-advanced/build/_sysbuild/sysbuild/images/mutex-advanced-prefix/src"
  "/home/goodbyte/Projects/NCS-Projects/mutex-advanced/build/_sysbuild/sysbuild/images/mutex-advanced-prefix/src/mutex-advanced-stamp"
)

set(configSubDirs )
foreach(subDir IN LISTS configSubDirs)
    file(MAKE_DIRECTORY "/home/goodbyte/Projects/NCS-Projects/mutex-advanced/build/_sysbuild/sysbuild/images/mutex-advanced-prefix/src/mutex-advanced-stamp/${subDir}")
endforeach()
if(cfgdir)
  file(MAKE_DIRECTORY "/home/goodbyte/Projects/NCS-Projects/mutex-advanced/build/_sysbuild/sysbuild/images/mutex-advanced-prefix/src/mutex-advanced-stamp${cfgdir}") # cfgdir has leading slash
endif()
