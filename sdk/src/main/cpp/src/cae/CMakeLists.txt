#cmake_minimum_required(VERSION 3.14)
message(${CMAKE_SOURCE_DIR})

include_directories(.)
include_directories(..)
include_directories(../recorder)
include_directories(../recorder/alsa)
include_directories(../recorder/include)
include_directories(../cae)
include_directories(../cae/include)
include_directories(../util/android)
include_directories(../util/log)
include_directories(../util/queue)

file (GLOB SRC_LIST
        "*.cpp"
        "../*.cpp"
        "../recorder/alsa/*.cpp"
        "../util/android/*.cpp"
        "../util/jvm/*.cpp"
        "../util/log/*.cpp"
        "../util/queue/*.c"
        )
link_directories(
        ../../lib
        ../../lib-alsa
)

add_library(alsacae SHARED ${SRC_LIST})
set(distribution_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../../../../..)
set_target_properties(
        alsacae
        PROPERTIES
        LIBRARY_OUTPUT_DIRECTORY
        ${distribution_DIR}/output/${ANDROID_ABI})
target_link_libraries(
        alsacae
        log
        hlw
        tinyalsa)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")