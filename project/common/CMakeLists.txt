#
# wangkaichao2@163.com 2018/09/16
# common cmake file
#

# include top cmakefile.txt
INCLUDE(../top.cmake)

set(CMAKE_CXX_STANDARD 11)
#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
# minimum cmake version
CMAKE_MINIMUM_REQUIRED(VERSION 3.5.1)

# project name
PROJECT(common C)
PROJECT(common CXX)

# include all cxx files
AUX_SOURCE_DIRECTORY(. CSRC)
AUX_SOURCE_DIRECTORY(. CXXSRC)

# target static library
ADD_LIBRARY(common STATIC ${CSRC} ${CXXSRC})
