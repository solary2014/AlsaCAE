#cmake_minimum_required(VERSION 3.14)
message(${CMAKE_SOURCE_DIR})
include_directories(.)
include_directories(../src)
include_directories(../src/alsarecorder)
include_directories(../src/android)
include_directories(../src/cae)
include_directories(../src/log)
include_directories(../src/queue)
include_directories(../src/tinyalsa)
SET(SRC_LIST
        com_iflytek_iflyos_AlsaCAE.cpp
        ../src/log/JniLog.cpp
        ../src/android/android_help.cpp
        ../src/Recorder.cpp
        ../src/queue/AudioQueue.c
        ../src/alsarecorder/alsa_recorder.cpp
        )
link_directories(../libs)
add_library(alsacae SHARED ${SRC_LIST})
target_link_libraries(
        alsacae
        log
        hlw
        tinyalsa)

add_definitions(-std=c++11)

#link_directories(../lib)
#add_executable(alsaexec ${SRC_LIST} main.cpp)
#target_link_libraries(
#        alsaexec
#        log
#        hlw
#        tinyalsa)

#add_subdirectory(exec)

#cmake_minimum_required(VERSION 3.14)
#set(BASE_SRC pipe/DoubleReceiverPipe.cpp
#        pipe/DoubleSendPipe.cpp
#        pipe/SendPipe.cpp
#        pipe/ReceiverPipe.cpp
#        pipe/pipe_help.cpp
#        tlv/TLV.cpp
#        tlv/TLVData.cpp)
##add_subdirectory(pipe)
##add_subdirectory(tlv)
#add_executable(test_pipe_tlv_client
#        test_pipe_tlv_client.cpp
#        ${BASE_SRC})
#add_executable(test_pipe_tlv_server
#        test_pipe_tlv_server.cpp
#        ${BASE_SRC})
#
#target_link_libraries(test_pipe_tlv_client pthread ${CMAKE_DL_LIBS})
#target_link_libraries(test_pipe_tlv_server pthread ${CMAKE_DL_LIBS})