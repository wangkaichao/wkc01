#
# wangkaichao2@163.com 2018-09-16
# top CMakeList.txt
#

# set top directory path
#exec_program("dirname `pwd`" OUTPUT_VARIABLE TOP)
set(TOP "/work2/wangkaichao/project")

set(CMAKE_SYSTEM_NAME Linux)
# specify the cross compiler
#set(CMAKE_C_COMPILER "arm-hisiv500-linux-gcc")
#set(CMAKE_CXX_COMPILER "arm-hisiv500-linux-g++")
set(CMAKE_C_COMPILER "gcc")
set(CMAKE_CXX_COMPILER "g++")
# where is the target environment 
#SET(CMAKE_FIND_ROOT_PATH ${TOP} /opt/hisi-linux/x86-arm/arm-hisiv500-linux)

# search for programs in the build host directories (not necessary NEVER ONLY BOTH)
#SET(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
# for libraries and headers in the target directories
#SET(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
#SET(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)

#set head file path
include_directories(
#  ${TOP}/3rd/libevent/include
#  ${TOP}/3rd/protobuf/include
#  ${TOP}/3rd/qt/include
  ${TOP}/common
  ${TOP}/3rd/md5-cc/include
#  ${TOP}/3rd/boost/include
#  ${TOP}/3rd/radix_tree/radix_tree-master
)

#set library path
link_directories(
#  ${TOP}/3rd/libevent/lib
#  ${TOP}/3rd/protobuf/lib
#  ${TOP}/3rd/qt/lib
  ${TOP}/common/build/
  ${TOP}/3rd/md5-cc/lib
#  ${TOP}/3rd/boost/lib
#  ${TOP}/3rd/radix_tree/radix_tree-master
)

# set compile flags
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=c99 -g3")
#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -g3 -Wno-error=implicit-function-declaration -Wno-date-time \
#	-mcpu=cortex-a17.cortex-a7 -mfloat-abi=softfp -mfpu=neon-vfpv4 \
#	-mno-unaligned-access -fno-aggressive-loop-optimizations -ffunction-sections -fdata-sections -fsigned-char")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -g3")

# add 3rd libraries here
link_libraries("-Wl,-Bstatic")
link_libraries("-Wl,--start-group")
#link_libraries("-levent_core")
link_libraries("-lcommon")
#link_libraries("-lprotobuf")
link_libraries("-lmd5cc")
#link_libraries("-lboost_serialization")
link_libraries("-Wl,--end-group")
link_libraries("-Wl,-Bdynamic")

# system library
link_libraries("-lpthread -lrt -ldl -lm -lc -lstdc++")
