cmake_minimum_required(VERSION 3.4.1)

add_library(native_lib
        SHARED
        src/main/cpp/native_lib.cpp)

find_library(log-lib
        log)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/src/main/cpp/include)
set(DIR ${CMAKE_CURRENT_SOURCE_DIR}/jniLibs)

message("current abi:${DIR}/${ANDROID_ABI}")
add_library(avcodec
        SHARED
        IMPORTED)
set_target_properties(avcodec
        PROPERTIES IMPORTED_LOCATION
        ${DIR}/${ANDROID_ABI}/libavcodec.so)
#if(avcodec)
#    message("avcodec found success!")
#else()
#    message(FATAL_ERROR "avcodec found failed!")
#endif()

add_library(avdevice
        SHARED
        IMPORTED)
set_target_properties(avdevice
        PROPERTIES IMPORTED_LOCATION
        ${DIR}/${ANDROID_ABI}/libavdevice.so)
add_library(avformat
        SHARED
        IMPORTED)
set_target_properties(avformat
        PROPERTIES IMPORTED_LOCATION
        ${DIR}/${ANDROID_ABI}/libavformat.so)
add_library(avutil
        SHARED
        IMPORTED)
set_target_properties(avutil
        PROPERTIES IMPORTED_LOCATION
        ${DIR}/${ANDROID_ABI}/libavutil.so)
add_library(swresample
        SHARED
        IMPORTED)
set_target_properties(swresample
        PROPERTIES IMPORTED_LOCATION
        ${DIR}/${ANDROID_ABI}/libswresample.so)
add_library(swscale
        SHARED
        IMPORTED)
set_target_properties(swscale
        PROPERTIES IMPORTED_LOCATION
        ${DIR}/${ANDROID_ABI}/libswscale.so)
add_library(avfilter
        SHARED
        IMPORTED)
set_target_properties(avfilter
        PROPERTIES IMPORTED_LOCATION
        ${DIR}/${ANDROID_ABI}/libavfilter.so)
target_link_libraries(native_lib
        avutil
        swresample
        avcodec
        avformat
        swscale
        avfilter
        avdevice
        ${log-lib}
        android)