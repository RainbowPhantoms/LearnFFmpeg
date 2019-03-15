//
// Created by MirsFang on 2019-03-12.
//
#include <iostream>

extern "C"{
#include <libavformat/avformat.h>
}
#include "src/chapter_05/avformatuse.h"

using namespace std;
int main(){

    int version =avformat_version();
    cout<<"version:"<<version<<endl;


    const char* httpUrl = "http://po79db9wc.bkt.clouddn.com/test_video.mp4";
    const char* url = "../video/test_video.mp4";


    //分离H264
    chapter05_h264(url);

    chapter05_h264_01(httpUrl);

    return 0;
}



