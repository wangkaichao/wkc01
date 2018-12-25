#
# wangkaichao2@163.com 2018/09/16
# logcat cmake file
#

# include top cmakefile.txt
INCLUDE(../top.cmake)

set(CMAKE_CXX_STANDARD 11)
# minimum cmake version
CMAKE_MINIMUM_REQUIRED(VERSION 3.5.1)

# project name
PROJECT(logcat CXX)

# include all cxx files
AUX_SOURCE_DIRECTORY(. CXXSRC)

# target static library
ADD_EXECUTABLE(logcat ${CXXSRC})
#TARGET_LINK_LIBRARIES(logcat common)
