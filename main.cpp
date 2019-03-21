//
// Created by MirsFang on 2019-03-12.
//
#include <iostream>

extern "C" {
#include <libavformat/avformat.h>
#include <SDL2/SDL.h>
}

#include "src/chapter_05/avformatuse.h"
#include "src/chapter_06/sdl_video.h"
#include "src/chapter_07/sdl_audio.h"

/** 第五章的例子 分离H264 **/
//#define chapter_05
/** 第六章 SDL2播放视频 **/
//#define chapter_06
/** 第七章 SDL2播放音频 **/
#define chapter_07

using namespace std;

int main() {
    //打印版本号
    int version = avformat_version();
    cout << "version:" << version << endl;

    const char *httpUrl = "http://po79db9wc.bkt.clouddn.com/test_video.mp4";
    const char *url = "../video/test_video.mp4";

/** 第五章例子 **/
#ifdef chapter_05
    //分离H264
   extract_video::chapter05_h264(url);
   extract_video::chapter05_h264_01(httpUrl);
#endif

/** 第六章例子 **/
#ifdef chapter_06

    sdl_video::playVideo(url);
#endif

#ifdef chapter_07

   sdl_audio::playAudio(url);

#endif



    return 0;
}



