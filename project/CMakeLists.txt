cmake_minimum_required(VERSION 2.6)

#set(CMAKE_SYSTEM_NAME Linux)
set(CMAKE_C_COMPILER "gcc")
set(CMAKE_CXX_COMPILER "g++")

set(PROJECT_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR})
#set(CMAKE_FIND_ROOT_PATH ${PROJECT_SOURCE_DIR} /opt/hisi-linux/x86-arm/arm-hisiv300-linux)
# search for programs in the build host directories (not necessary NEVER ONLY BOTH)
#set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
# for libraries and headers in the target directories
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)

#execute_process(COMMAND sh ${PATH}/XXXX.sh)

#set head file path
include_directories(${PROJECT_SOURCE_DIR}/3rd/md5-cc/include)
include_directories(${PROJECT_SOURCE_DIR}/3rd/sqlite/sqlite-amalgamation-3260000)
include_directories(${PROJECT_SOURCE_DIR}/include)
include_directories(${PROJECT_SOURCE_DIR}/log)
include_directories(${PROJECT_SOURCE_DIR}/utils)

#set library path
link_directories(${PROJECT_SOURCE_DIR}/3rd/md5-cc/lib)
link_directories(${PROJECT_SOURCE_DIR}/3rd/sqlite/sqlite-amalgamation-3260000)
link_directories(${PROJECT_SOURCE_DIR}/lib)

# set compile flags
#set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -mcpu=cortex-a7 -mfloat-abi=softfp -mfpu=neon-vfpv4 -ffunction-sections")
#set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -mno-unaligned-access -fno-aggressive-loop-optimizations")
#set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -DARCH=arm -DHIARCH=hi3516a -DHICHIP=0x3516A100 ")

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -g3")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -g3")

message("========================================")
option(Mac "Mac" ON)
if(Mac)
    message("Mac")
    set(CMAKE_MACOSX_RPATH 1)
    link_libraries("-lpthread -ldl -lm -lc -lstdc++")
else()
    message("Linux")
    link_libraries("-Wl,-Bdynamic")
    link_libraries("-lpthread -lrt -ldl -lm -lc -lstdc++")
endif()

option(USE_SHARED "SHARED" ON)
if(USE_SHARED)
    message("output shared library")
else()
    message("output static library")
endif()
message("========================================")

set(LIBRARY_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/lib)
set(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin)

######################### lib ######################################
aux_source_directory(${PROJECT_SOURCE_DIR}/log LOG_SRC)
if(USE_SHARED)
    add_library(log_share SHARED ${LOG_SRC})
    set_target_properties(log_share PROPERTIES OUTPUT_NAME "log")
    set_target_properties(log_share PROPERTIES CLEAN_DIRECT_OUTPUT 1)
    #target_link_libraries(log_share )
else()
    add_library(log_static STATIC ${LOG_SRC})
    set_target_properties(log_static PROPERTIES OUTPUT_NAME "log")
    set_target_properties(log_static PROPERTIES CLEAN_DIRECT_OUTPUT 1)
    #target_link_libraries(log_static )
endif()

aux_source_directory(${PROJECT_SOURCE_DIR}/utils UTILS_SRC)
if(USE_SHARED)
    add_library(utils_share SHARED ${UTILS_SRC})
    set_target_properties(utils_share PROPERTIES OUTPUT_NAME "utils")
    set_target_properties(utils_share PROPERTIES CLEAN_DIRECT_OUTPUT 1)
    target_link_libraries(utils_share md5cc)
else()
    add_library(utils_static STATIC ${UTILS_SRC})
    set_target_properties(utils_static PROPERTIES OUTPUT_NAME "utils")
    set_target_properties(utils_static PROPERTIES CLEAN_DIRECT_OUTPUT 1)
    target_link_libraries(utils_static md5cc)
endif()

######################### bin ######################################
set(DEMOLOG_SRC ${PROJECT_SOURCE_DIR}/demo/demolog.c)
# target execute
add_executable(demolog_exe ${DEMOLOG_SRC})
if(USE_SHARED)
    add_dependencies(demolog_exe log_share)
else()
    add_dependencies(demolog_exe log_static)
endif()
set_target_properties(demolog_exe PROPERTIES OUTPUT_NAME "demolog")
set_target_properties(demolog_exe PROPERTIES CLEAN_DIRECT_OUTPUT 1)
target_link_libraries(demolog_exe log)

aux_source_directory(${PROJECT_SOURCE_DIR}/logcat LOGCAT_SRC)
# target execute
add_executable(logcat_exe ${LOGCAT_SRC})
if(USE_SHARED)
    add_dependencies(logcat_exe log_share utils_share)
else()
    add_dependencies(logcat_exe log_static utils_share)
endif()
set_target_properties(logcat_exe PROPERTIES OUTPUT_NAME "logcat")
set_target_properties(logcat_exe PROPERTIES CLEAN_DIRECT_OUTPUT 1)
target_link_libraries(logcat_exe log utils)
