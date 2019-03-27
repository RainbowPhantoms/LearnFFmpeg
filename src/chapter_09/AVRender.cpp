//
// Created by MirsFang on 2019-03-25.
//

#include "AVRender.h"


AVRender::AVRender() {
    //初始化SDL2
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_EVENTS)) {
        std::cout << "[error] SDL Init error !" << std::endl;
        return;
    }

    //创建window
    window = SDL_CreateWindow("LearnFFmpeg", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, WINDOW_WIDTH,
                              WINDOW_HEIGHT, SDL_WINDOW_OPENGL);
    if (!window) {
        std::cout << "[error] SDL Create window error!" << std::endl;
        return;
    }

    //创建Render
    render = SDL_CreateRenderer(window, -1, 0);
    //创建Texture
    texture = SDL_CreateTexture(render, SDL_PIXELFORMAT_IYUV, SDL_TEXTUREACCESS_STREAMING, WINDOW_WIDTH, WINDOW_HEIGHT);

    //初始化Rect
    rect.x = 0;
    rect.y = 0;
    rect.w = WINDOW_WIDTH;
    rect.h = WINDOW_HEIGHT;
}

AVRender::~AVRender() {
    SDL_CloseAudio();
    SDL_Quit();
    if(render)SDL_DestroyRenderer(render);
    if(texture)SDL_DestroyTexture(texture);
    if(window)SDL_DestroyWindow(window);
}

void AVRender::loopEvent() {
    SDL_Event event;
    for (;;) {
        SDL_PollEvent(&event);
        switch (event.type) {
            case SDL_KEYDOWN:
                switch (event.key.keysym.sym) {

                }
                break;
            case SDL_QUIT:
                return;
            default:
                break;
        }
    }
}


void AVRender::renderVideo(AVFrame *frame, Uint32 duration) {
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
    SDL_Delay(duration);
}

void AVRender::openAudio(int sample_rate, Uint8 channel, Uint16 samples, void *userdata,
                         void (*fill_audio)(void *, Uint8 *, int)) {

    //初始化SDL中自己想设置的参数
    wantSpec.freq = sample_rate;
    wantSpec.format = AUDIO_S16SYS;
    wantSpec.channels = channel;
    wantSpec.silence = 0;
    wantSpec.samples = samples;
    wantSpec.callback = fill_audio;
    wantSpec.userdata = userdata;

    //打开音频之后wantSpec的值可能会有改动，返回实际设备的参数值
    if (SDL_OpenAudio(&wantSpec, NULL) < 0) {
        std::cout << "[error] open audio error" << std::endl;
        return;
    }

    SDL_PauseAudio(0);
}
