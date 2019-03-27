//
// Created by MirsFang on 2019-03-25.
//

#ifndef LEARNFFMPEG_AUDIOTHREAD_H
#define LEARNFFMPEG_AUDIOTHREAD_H

#include <pthread.h>
#include <iostream>

extern "C" {
#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
#include <libswresample/swresample.h>
};

#include "AVRender.h"


/**
 * 音频线程
 */
class AudioThread {
public:
    AudioThread();

    ~AudioThread();

    void setUrl(const char *url);

    /** 开启线程 **/
    void start();

    /** 设置渲染器 **/
    void setRender(AVRender *render);

private:
    /** 重采样上下文 **/
    SwrContext *convert_context;
    AVFormatContext *format_context;
    AVCodecContext *codec_context;
    AVCodec *codec;
    AVPacket *packet;
    AVFrame *frame;
    int audioIndex = -1;

    uint64_t out_chn_layout = AV_CH_LAYOUT_STEREO;  //输出的通道布局 双声道
    enum AVSampleFormat out_sample_fmt = AV_SAMPLE_FMT_S16; //输出的声音格式
    int out_sample_rate = 44100;   //输出的采样率
    int out_nb_samples = -1;        //输出的音频采样
    int out_channels = -1;        //输出的通道数
    int out_buffer_size = -1;   //输出buff大小
    unsigned char *outBuff = NULL;//输出的Buffer数据
    uint64_t in_chn_layout = -1;  //输入的通道布局

    pthread_t pid;
    pthread_mutex_t mutex;
    AVRender *av_render;

    const char *url;

    static void *start_thread(void *arg);

    void run();

    /** 初始化解码器 **/
    void prepare_codec();
};


#endif //LEARNFFMPEG_AUDIOTHREAD_H
