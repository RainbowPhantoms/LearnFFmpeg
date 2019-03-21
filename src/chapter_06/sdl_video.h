//
// Created by MirsFang on 2019-03-15.
//

namespace sdl_video {
#include <iostream>

extern "C" {

#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
#include <libavutil/avutil.h>
#include <SDL2/SDL.h>

}

using namespace std;


#define WINDOW_WIDTH 1080
#define WINDOW_HEIGHT 720

/** ########## SDL2 相关 ############# **/
SDL_Window *window;
SDL_Renderer *render;
SDL_Texture *texture;
SDL_Rect rect;

/** ########### FFmpeg 相关 ############# **/
AVFormatContext *formatContext;
AVCodecContext *codecContext;
AVCodec *codec;
AVPacket *packet;
AVFrame *frame;
int videoIndex = -1;
double displayTimeUs = 0;


/** 初始化SDL2 **/
void initSDL2();

/** 初始化FFmpeg  **/
void preparDecodec(const char *url);

/** 解码播放 **/
void decodecFrame();

/** 释放资源 **/
void freeContext();

/** 绘制一帧数据 **/
void drawFrame(AVFrame *frame);

/** 播放视频 **/
void playVideo(const char *url);

/**
 * 初始化SDL2
 */
void initSDL2() {
    //初始化SDL2
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS | SDL_INIT_TIMER)) {
        cout << "[error] SDL Init error!" << endl;
        return;
    }

    //创建Window
    window = SDL_CreateWindow("LearnFFmpeg", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, WINDOW_WIDTH,
                              WINDOW_HEIGHT, SDL_WINDOW_OPENGL);
    if (!window) {
        cout << "[error] SDL CreateWindow error!" << endl;
        return;
    }

    //创建Render
    render = SDL_CreateRenderer(window, -1, 0);
    //创建Texture
    texture = SDL_CreateTexture(render, SDL_PIXELFORMAT_IYUV, SDL_TEXTUREACCESS_STREAMING, WINDOW_WIDTH,
                                WINDOW_HEIGHT);

    rect.x = 0;
    rect.y = 0;
    rect.w = WINDOW_WIDTH;
    rect.h = WINDOW_HEIGHT;
}

/** 初始化FFmpeg  **/
void preparDecodec(const char *url) {
    int retcode;
    //初始化FormatContext
    formatContext = avformat_alloc_context();
    if (!formatContext) {
        cout << "[error] alloc format context error!" << endl;
        return;
    }

    //打开输入流
    retcode = avformat_open_input(&formatContext, url, nullptr, nullptr);
    if (retcode != 0) {
        cout << "[error] open input error!" << endl;
        return;
    }

    //读取媒体文件信息
    retcode = avformat_find_stream_info(formatContext, NULL);
    if (retcode != 0) {
        cout << "[error] find stream error!" << endl;
        return;
    }

    //分配codecContext
    codecContext = avcodec_alloc_context3(NULL);
    if (!codecContext) {
        cout << "[error] alloc codec context error!" << endl;
        return;
    }

    //寻找到视频流的下标
    videoIndex = av_find_best_stream(formatContext, AVMEDIA_TYPE_VIDEO, -1, -1, NULL, 0);
    //将视频流的的编解码信息拷贝到codecContext中
    retcode = avcodec_parameters_to_context(codecContext, formatContext->streams[videoIndex]->codecpar);
    if (retcode != 0) {
        cout << "[error] parameters to context error!" << endl;
        return;
    }

    //查找解码器
    codec = avcodec_find_decoder(codecContext->codec_id);
    if (codec == nullptr) {
        cout << "[error] find decoder error!" << endl;
        return;
    }

    //打开解码器
    retcode = avcodec_open2(codecContext, codec, nullptr);
    if (retcode != 0) {
        cout << "[error] open decodec error!" << endl;
        return;
    }

    //初始化一个packet
    packet = av_packet_alloc();
    //初始化一个Frame
    frame = av_frame_alloc();
}

/** 解码数据 **/
void decodecFrame() {
    int sendcode = 0;

    //计算帧率
    double frameRate = av_q2d(formatContext->streams[videoIndex]->avg_frame_rate);
    //计算显示的时间
    displayTimeUs = 1 * 1000 / frameRate;

    //读取包
    while (av_read_frame(formatContext, packet) == 0) {
        if (packet->stream_index != videoIndex)continue;
        //接受解码后的帧数据
        while (avcodec_receive_frame(codecContext, frame) == 0) {
            //绘制图像
            drawFrame(frame);
        }
        //发送解码前的包数据
        sendcode = avcodec_send_packet(codecContext, packet);
        //根据发送的返回值判断状态
        if (sendcode == 0) {
            cout << "[debug] " << "SUCCESS" << endl;
        } else if (sendcode == AVERROR_EOF) {
            cout << "[debug] " << "EOF" << endl;
        } else if (sendcode == AVERROR(EAGAIN)) {
            cout << "[debug] " << "EAGAIN" << endl;
        } else {
            cout << "[debug] " << av_err2str(AVERROR(sendcode)) << endl;
        }
        av_packet_unref(packet);
    }

}

/** 释放资源 **/
void freeContext() {
    if (formatContext != nullptr) avformat_close_input(&formatContext);
    if (codecContext != nullptr) avcodec_free_context(&codecContext);
    if (packet != nullptr) av_packet_free(&packet);
    if (frame != nullptr) av_frame_free(&frame);
}


/** 绘制一帧数据 **/
void drawFrame(AVFrame *frame) {
    if (frame == nullptr)return;
    //上传YUV到Texture
    SDL_UpdateYUVTexture(texture, &rect,
                         frame->data[0], frame->linesize[0],
                         frame->data[1], frame->linesize[1],
                         frame->data[2], frame->linesize[2]
    );

    SDL_RenderClear(render);
    SDL_RenderCopy(render, texture, NULL, &rect);
    SDL_RenderPresent(render);
    SDL_Delay(displayTimeUs);
}

/** 播放视频 **/
void playVideo(const char *url) {
    initSDL2();
    preparDecodec(url);
    decodecFrame();
    freeContext();
}
}