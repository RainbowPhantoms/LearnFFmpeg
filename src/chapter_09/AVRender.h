//
// Created by MirsFang on 2019-03-25.
//

#ifndef LEARNFFMPEG_AVRENDER_H
#define LEARNFFMPEG_AVRENDER_H

#define WINDOW_WIDTH 1080
#define WINDOW_HEIGHT 720

#include <iostream>

extern "C" {
#include <SDL2/SDL.h>
#include <libavcodec/avcodec.h>
}

/** 音视频渲染器 **/
class AVRender {
public:
    AVRender();

    ~AVRender();

    /**
     * 打开音频
     *
     * @param sample_rate 采样率
     * @param channel   通道数
     * @param samples   采样大小(一帧的音频数据大小)
     * @param userdata  用户数据
     * @param fillaudio 回调函数
     */
    void openAudio(int sample_rate, Uint8 channel, Uint16 samples, void *userdata,
                   void (*fill_audio)(void *codecContext, Uint8 *stream, int len));

    /** 循环获取事件 **/
    void loopEvent();

    /** 渲染视频
     *
     * @param frame 视频帧
     * @param duration 帧持续的时间
     */
    void renderVideo(AVFrame *frame,Uint32 duration);

private:
    /** SDL窗口 **/
    SDL_Window *window;
    /** SDL渲染者 **/
    SDL_Renderer *render;
    /** SDL纹理 **/
    SDL_Texture *texture;
    /** 显示区域 **/
    SDL_Rect rect;

    /** 自己想要的输出的音频格式 **/
    SDL_AudioSpec wantSpec;

};


#endif //LEARNFFMPEG_AVRENDER_H
