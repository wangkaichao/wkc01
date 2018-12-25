#
# wangkaichao2@163.com 2018/09/16
# demo cmake file
#

# include top cmakefile.txt
INCLUDE(../top.cmake)

set(CMAKE_CXX_STANDARD 11)
# minimum cmake version
CMAKE_MINIMUM_REQUIRED(VERSION 3.5.1)

# project name
PROJECT(demo C)
#PROJECT(demo CXX)

# include all cxx files
AUX_SOURCE_DIRECTORY(. CSRC)
#AUX_SOURCE_DIRECTORY(. CSRCXX)

# target static library
ADD_EXECUTABLE(demo ${CSRC})
#ADD_EXECUTABLE(demo ${CSRCXX})

