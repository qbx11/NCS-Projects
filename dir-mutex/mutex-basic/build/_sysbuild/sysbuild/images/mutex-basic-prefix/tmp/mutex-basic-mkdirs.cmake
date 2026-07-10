# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file LICENSE.rst or https://cmake.org/licensing for details.

cmake_minimum_required(VERSION ${CMAKE_VERSION}) # this file comes with cmake

# If CMAKE_DISABLE_SOURCE_CHANGES is set to true and the source directory is an
# existing directory in our source tree, calling file(MAKE_DIRECTORY) on it
# would cause a fatal error, even though it would be a no-op.
if(NOT EXISTS "/home/goodbyte/Projects/NCS-Projects/mutex-basic")
  file(MAKE_DIRECTORY "/home/goodbyte/Projects/NCS-Projects/mutex-basic")
endif()
file(MAKE_DIRECTORY
  "/home/goodbyte/Projects/NCS-Projects/mutex-basic/build/mutex-basic"
  "/home/goodbyte/Projects/NCS-Projects/mutex-basic/build/_sysbuild/sysbuild/images/mutex-basic-prefix"
  "/home/goodbyte/Projects/NCS-Projects/mutex-basic/build/_sysbuild/sysbuild/images/mutex-basic-prefix/tmp"
  "/home/goodbyte/Projects/NCS-Projects/mutex-basic/build/_sysbuild/sysbuild/images/mutex-basic-prefix/src/mutex-basic-stamp"
  "/home/goodbyte/Projects/NCS-Projects/mutex-basic/build/_sysbuild/sysbuild/images/mutex-basic-prefix/src"
  "/home/goodbyte/Projects/NCS-Projects/mutex-basic/build/_sysbuild/sysbuild/images/mutex-basic-prefix/src/mutex-basic-stamp"
)

set(configSubDirs )
foreach(subDir IN LISTS configSubDirs)
    file(MAKE_DIRECTORY "/home/goodbyte/Projects/NCS-Projects/mutex-basic/build/_sysbuild/sysbuild/images/mutex-basic-prefix/src/mutex-basic-stamp/${subDir}")
endforeach()
if(cfgdir)
  file(MAKE_DIRECTORY "/home/goodbyte/Projects/NCS-Projects/mutex-basic/build/_sysbuild/sysbuild/images/mutex-basic-prefix/src/mutex-basic-stamp${cfgdir}") # cfgdir has leading slash
endif()
