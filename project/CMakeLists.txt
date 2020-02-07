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

execute_process(COMMAND mkdir -p ${PROJECT_SOURCE_DIR}/bin)
execute_process(COMMAND mkdir -p ${PROJECT_SOURCE_DIR}/lib)

#set head file path
include_directories(${PROJECT_SOURCE_DIR}/3rd/md5-cc/include)
include_directories(${PROJECT_SOURCE_DIR}/3rd/sqlite/sqlite-amalgamation-3260000)
include_directories(${PROJECT_SOURCE_DIR}/include)
include_directories(${PROJECT_SOURCE_DIR}/utils)
include_directories(${PROJECT_SOURCE_DIR}/ipc)
include_directories(${PROJECT_SOURCE_DIR}/basic)

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
#option(Mac "Mac" ON)
if(Mac)
    message("Mac")
    set(CMAKE_MACOSX_RPATH 1)
    link_libraries("-lpthread")
else()
    message("Linux")
    link_libraries("-lpthread -lrt")
endif()

option(USE_SHARED "SHARED" ON)

if(USE_SHARED)
    message("output shared library")
    set(flag SHARED)
else()
    message("output static library")
    set(flag STATIC)
endif()
message("========================================")

set(LIBRARY_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/lib)
set(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin)

######################### lib ######################################
aux_source_directory(${PROJECT_SOURCE_DIR}/utils UTILS_SRC)
set                  (  utils_obj utils_${flag})
add_library          (${utils_obj} ${flag} ${UTILS_SRC})
set_target_properties(${utils_obj} PROPERTIES OUTPUT_NAME "utils")
set_target_properties(${utils_obj} PROPERTIES CLEAN_DIRECT_OUTPUT 1)
target_link_libraries(${utils_obj} stdc++ md5cc)

aux_source_directory(${PROJECT_SOURCE_DIR}/ipc IPC_SRC)
set                  (  ipc_obj ipc_${flag})
add_library          (${ipc_obj} ${flag} ${IPC_SRC})
set_target_properties(${ipc_obj} PROPERTIES OUTPUT_NAME "ipc")
set_target_properties(${ipc_obj} PROPERTIES CLEAN_DIRECT_OUTPUT 1)

aux_source_directory(${PROJECT_SOURCE_DIR}/basic BASIC_SRC)
set                  (  basic_obj basic_${flag})
add_library          (${basic_obj} ${flag} ${BASIC_SRC})
set_target_properties(${basic_obj} PROPERTIES OUTPUT_NAME "basic")
set_target_properties(${basic_obj} PROPERTIES CLEAN_DIRECT_OUTPUT 1)
target_link_libraries(${basic_obj} stdc++)

######################### bin ######################################
set(DEMOLOG_SRC ${PROJECT_SOURCE_DIR}/demo/demolog.c)
# target execute
add_executable       (demolog_exe ${DEMOLOG_SRC})
set_target_properties(demolog_exe PROPERTIES OUTPUT_NAME "demolog")
set_target_properties(demolog_exe PROPERTIES CLEAN_DIRECT_OUTPUT 1)

set(DEMOIPC_SRC ${PROJECT_SOURCE_DIR}/demo/demoipc.c)
# target execute
add_executable       (demoipc_exe ${DEMOIPC_SRC})
add_dependencies     (demoipc_exe ${ipc_obj})
set_target_properties(demoipc_exe PROPERTIES OUTPUT_NAME "demoipc")
set_target_properties(demoipc_exe PROPERTIES CLEAN_DIRECT_OUTPUT 1)
target_link_libraries(demoipc_exe ipc)
#target_link_libraries(demoipc_exe ipc efence)

set(DEMOTHREAD_SRC ${PROJECT_SOURCE_DIR}/demo/DemoThread.cpp)
# target execute
add_executable       (demothread_exe ${DEMOTHREAD_SRC})
add_dependencies     (demothread_exe ${basic_obj})
set_target_properties(demothread_exe PROPERTIES OUTPUT_NAME "demothread")
set_target_properties(demothread_exe PROPERTIES CLEAN_DIRECT_OUTPUT 1)
target_link_libraries(demothread_exe basic stdc++)

set(DEMOMSGQUEUE_SRC ${PROJECT_SOURCE_DIR}/demo/DemoMsgQueue.cpp)
# target execute
add_executable       (demomsgqueue_exe ${DEMOMSGQUEUE_SRC})
add_dependencies     (demomsgqueue_exe ${basic_obj})
set_target_properties(demomsgqueue_exe PROPERTIES OUTPUT_NAME "demomsgqueue")
set_target_properties(demomsgqueue_exe PROPERTIES CLEAN_DIRECT_OUTPUT 1)
target_link_libraries(demomsgqueue_exe basic stdc++)

set(DEMOTHREADWITHMSGQUEUE_SRC ${PROJECT_SOURCE_DIR}/demo/DemoThreadWithMsgQueue.cpp)
# target execute
add_executable       (demothreadwithmsgqueue_exe ${DEMOTHREADWITHMSGQUEUE_SRC})
add_dependencies     (demothreadwithmsgqueue_exe ${basic_obj})
set_target_properties(demothreadwithmsgqueue_exe PROPERTIES OUTPUT_NAME "demothreadwithmsgqueue")
set_target_properties(demothreadwithmsgqueue_exe PROPERTIES CLEAN_DIRECT_OUTPUT 1)
target_link_libraries(demothreadwithmsgqueue_exe basic stdc++)

set(DEMOOBSERVABLE_SRC ${PROJECT_SOURCE_DIR}/demo/DemoObservable.cpp)
# target execute
add_executable       (demoobservable_exe ${DEMOOBSERVABLE_SRC})
add_dependencies     (demoobservable_exe ${basic_obj})
set_target_properties(demoobservable_exe PROPERTIES OUTPUT_NAME "demoobservable")
set_target_properties(demoobservable_exe PROPERTIES CLEAN_DIRECT_OUTPUT 1)
target_link_libraries(demoobservable_exe basic stdc++)

