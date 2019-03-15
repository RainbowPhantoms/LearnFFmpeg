cmake_minimum_required(VERSION 3.13)
project(LearnFFmpeg)

set(CMAKE_CXX_STANDARD 11)

set(SRC_DIR ./)

include_directories(include/)
link_directories(librarys/ffmpeg/)

set(SOURCE_FILES

        main.cpp
        src/chapter_05/avformatuse.h)


add_executable(LearnFFmpeg

        ${SOURCE_FILES})

target_link_libraries(
        LearnFFmpeg

        avformat.58
        avcodec.58
        avdevice.58
        avfilter.7
        avutil.56
        postproc.55
        swresample.3
        swscale.5
)