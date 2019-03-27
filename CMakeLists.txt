cmake_minimum_required(VERSION 3.13)
project(LearnFFmpeg)

set(CMAKE_CXX_STANDARD 11)

include_directories(${PROJECT_SOURCE_DIR}/include/)
link_directories(${PROJECT_SOURCE_DIR}/librarys/)

## AVCodec
#add_library(libavcodec SHARED IMPORTED)
#set_target_properties(libavcodec
#        PROPERTIES
#        IMPORTED_LOCATION ${PROJECT_SOURCE_DIR}/librarys/libavcodec.58.dylib
#        )
#
##AVdevice
#add_library(libavdevice SHARED IMPORTED)
#set_target_properties(libavdevice
#        PROPERTIES
#        IMPORTED_LOCATION ${PROJECT_SOURCE_DIR}/librarys/libavdevice.58.dylib
#        )
#
##AVFilter
#add_library(libavfilter SHARED IMPORTED)
#set_target_properties(libavfilter
#        PROPERTIES
#        IMPORTED_LOCATION ${PROJECT_SOURCE_DIR}/librarys/libavfilter.7.dylib
#        )
#
##AVformat
#add_library(libavformat SHARED IMPORTED)
#set_target_properties(libavformat
#        PROPERTIES
#        IMPORTED_LOCATION ${PROJECT_SOURCE_DIR}/librarys/libavformat.58.dylib
#        )
#
##AVUtil
#add_library(libavutil SHARED IMPORTED)
#set_target_properties(
#        libavutil
#        PROPERTIES
#        IMPORTED_LOCATION ${PROJECT_SOURCE_DIR}/librarys/libavutil.56.dylib
#)
#
##Swresample
#add_library(libswresample SHARED IMPORTED)
#set_target_properties(libswresample
#        PROPERTIES
#        IMPORTED_LOCATION ${PROJECT_SOURCE_DIR}/librarys/libswresample.3.dylib)
#
##Swscale
#add_library(libswscale SHARED IMPORTED)
#set_target_properties(libswscale
#        PROPERTIES
#        IMPORTED_LOCATION ${PROJECT_SOURCE_DIR}/librarys/libswscale.5.dylib)
##Postproc
#add_library(libpostproc SHARED IMPORTED)
#set_target_properties(libpostproc
#        PROPERTIES
#        IMPORTED_LOCATION ${PROJECT_SOURCE_DIR}/librarys/libpostproc.55.dylib
#        )
#
##SDL2
#add_library(SDL2 SHARED IMPORTED)
#set_target_properties(SDL2
#        PROPERTIES
#        IMPORTED_LOCATION ${PROJECT_SOURCE_DIR}/librarys/libSDL2-2.0.0.dylib
#        )


set(SOURCE_FILES
        main.cpp
        src/chapter_05/avformatuse.h
        src/chapter_06/sdl_video.h
        src/chapter_07/sdl_audio.h
        src/chapter_08/AVPacketMemoryModel.cpp
        src/chapter_08/AVPacketMemoryModel.h src/chapter_09/VideoThread.cpp src/chapter_09/VideoThread.h src/chapter_09/AudioThread.cpp src/chapter_09/AudioThread.h src/chapter_09/AVRender.cpp src/chapter_09/AVRender.h)


add_executable(LearnFFmpeg

        ${SOURCE_FILES}
        )

target_link_libraries(
        LearnFFmpeg

        avcodec.58
        avdevice.58
        avfilter.7
        avformat.58
        avutil.56
        postproc.55
        swresample.3
        swscale.5
        SDL2-2.0.0
)