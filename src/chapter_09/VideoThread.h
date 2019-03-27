//
// Created by MirsFang on 2019-03-25.
//

#ifndef LEARNFFMPEG_VIDEOTHREAD_H
#define LEARNFFMPEG_VIDEOTHREAD_H

#include <pthread.h>
#include <iostream>
#include "AVRender.h"

extern "C" {
#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
};

/** 视频线程 **/
class VideoThread {
public:
    VideoThread();

    ~VideoThread();

    /** 设置视频路径 **/
    void setUrl(const char *url);

    /** 设置渲染器 **/
    void setRender(AVRender *render);

    /** 开始运行线程 **/
    void start();


private:
    AVFormatContext *format_context;
    AVCodecContext *codec_context;
    AVCodec *codec;
    AVPacket *packet;
    AVFrame *frame;

    const char *url;
    int video_index;

    pthread_t pid;
    pthread_mutex_t mutex;

    AVRender *avRender;

    double last_pts = 0;
    /** 帧间距同步 **/
    bool is_interval_sync = true;

    static void *start_thread(void *arg);

    void run();

    /** 初始化解码器 **/
    void prepare_codec();

    /** 解码数据帧 **/
    void decodec_frame();

    /**
     * 根据帧率获取显示时间
     * @param frame_rate 帧率
     * @return 需要显示的时长
     */
    Uint32 sync_frame_rate(double frame_rate);

    /**
     * 根据帧间隔获取一帧显示的时长
     * @param timebase
     * @param pts 秒
     * @return
     */
    double sync_frame_interval(AVRational timebase, int pts);
};


#endif //LEARNFFMPEG_VIDEOTHREAD_H
