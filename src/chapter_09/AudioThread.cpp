//
// Created by MirsFang on 2019-03-25.
//

#include "AudioThread.h"


#define MAX_AUDIO_FRAME_SIZE 192000 // 1 second of 48khz 32bit audio   48000 * (32/8)

//一帧PCM的数据长度
unsigned int audioLen = 0;
unsigned char *audioChunk = nullptr;
//当前读取的位置
unsigned char *audioPos = nullptr;

/** 被SDL2调用的回调函数 当需要获取数据喂入硬件播放的时候调用 **/
void fill_audio(void *codecContext, Uint8 *stream, int len) {
    //SDL2中必须首先使用SDL_memset()将stream中的数据设置为0
    SDL_memset(stream, 0, len);
    if (audioLen == 0)
        return;

    len = (len > audioLen ? audioLen : len);
    //将数据合并到 stream 里
    SDL_MixAudio(stream, audioPos, len, SDL_MIX_MAXVOLUME);

    //一帧的数据控制
    audioPos += len;
    audioLen -= len;
}

AudioThread::AudioThread() {

}


AudioThread::~AudioThread() {
    if (format_context != nullptr) avformat_close_input(&format_context);
    if (codec_context != nullptr) avcodec_free_context(&codec_context);
    if (packet != nullptr) av_packet_free(&packet);
    if (frame != nullptr) av_frame_free(&frame);
    if (convert_context != nullptr) swr_free(&convert_context);
}


void AudioThread::start() {
    prepare_codec();
    if (pthread_create(&pid, NULL, start_thread, (void *) this) != 0) {
        std::cout << "初始化音频线程失败!" << std::endl;
        return;
    }
}

void *AudioThread::start_thread(void *arg) {
    AudioThread *audioThread = (AudioThread *) arg;
    audioThread->run();
    return nullptr;
}

void AudioThread::run() {
    std::cout << "音频线程已启动" << std::endl;

    //循环读取packet并且解码
    int sendcode = 0;
    while (av_read_frame(format_context, packet) >= 0) {
        if (packet->stream_index != audioIndex)continue;
        //接受解码后的音频数据
        while (avcodec_receive_frame(codec_context, frame) == 0) {
            swr_convert(convert_context, &outBuff, MAX_AUDIO_FRAME_SIZE, (const uint8_t **) frame->data,
                        frame->nb_samples);
            std::cout<<"audio pts : "<<(frame->pts * av_q2d(format_context->streams[audioIndex]->time_base))<<std::endl;
            //如果没有播放完就等待1ms
            while (audioLen > 0)
                SDL_Delay(1);
            //同步数据
            audioChunk = (unsigned char *) outBuff;
            audioPos = audioChunk;
            audioLen = out_buffer_size;
            av_frame_unref(frame);
        }
        //发送解码前的包数据
        sendcode = avcodec_send_packet(codec_context, packet);
        //根据发送的返回值判断状态
        if (sendcode == 0) {
//            std::cout << "[debug] " << "SUCCESS" << std::endl;
        } else if (sendcode == AVERROR_EOF) {
            std::cout << "[debug] " << "EOF" << std::endl;
        } else if (sendcode == AVERROR(EAGAIN)) {
            std::cout << "[debug] " << "EAGAIN" << std::endl;
        } else {
            std::cout << "[debug] " << av_err2str(AVERROR(sendcode)) << std::endl;
        }

        av_packet_unref(packet);
    }

}

void AudioThread::setRender(AVRender *render) {
    this->av_render = render;
}

void AudioThread::prepare_codec() {
    int retcode;
    //初始化FormatContext
    format_context = avformat_alloc_context();
    if (!format_context) {
        std::cout << "[error] alloc format context error!" << std::endl;
        return;
    }

    //打开输入流
    retcode = avformat_open_input(&format_context, url, nullptr, nullptr);
    if (retcode != 0) {
        std::cout << "[error] open input error!" << std::endl;
        return;
    }

    //读取媒体文件信息
    retcode = avformat_find_stream_info(format_context, NULL);
    if (retcode != 0) {
        std::cout << "[error] find stream error!" << std::endl;
        return;
    }

    //分配codecContext
    codec_context = avcodec_alloc_context3(NULL);
    if (!codec_context) {
        std::cout << "[error] alloc codec context error!" << std::endl;
        return;
    }

    //寻找到音频流的下标
    audioIndex = av_find_best_stream(format_context, AVMEDIA_TYPE_AUDIO, -1, -1, NULL, 0);
    //将视频流的的编解码信息拷贝到codecContext中
    retcode = avcodec_parameters_to_context(codec_context, format_context->streams[audioIndex]->codecpar);
    if (retcode != 0) {
        std::cout << "[error] parameters to context error!" << std::endl;
        return;
    }

    //查找解码器
    codec = avcodec_find_decoder(codec_context->codec_id);
    if (codec == nullptr) {
        std::cout << "[error] find decoder error!" << std::endl;
        return;
    }

    //打开解码器
    retcode = avcodec_open2(codec_context, codec, nullptr);
    if (retcode != 0) {
        std::cout << "[error] open decodec error!" << std::endl;
        return;
    }

    //初始化一个packet
    packet = av_packet_alloc();
    //初始化一个Frame
    frame = av_frame_alloc();


    /** ########## 获取实际音频的参数 ##########**/
    //单个通道中的采样数
    out_nb_samples = codec_context->frame_size;
    //输出的声道数
    out_channels = av_get_channel_layout_nb_channels(out_chn_layout);
    //输出音频的布局
    in_chn_layout = av_get_default_channel_layout(codec_context->channels);

    /** 计算重采样后的实际数据大小,并分配空间 **/
    //计算输出的buffer的大小
    out_buffer_size = av_samples_get_buffer_size(NULL, out_channels, out_nb_samples, out_sample_fmt, 1);
    //分配输出buffer的空间
    outBuff = (unsigned char *) av_malloc(MAX_AUDIO_FRAME_SIZE * 2); //双声道

    //初始化SDL中自己想设置的参数
    if (av_render)av_render->openAudio(out_sample_rate, out_channels, out_nb_samples, codec_context, fill_audio);

    convert_context = swr_alloc_set_opts(NULL, out_chn_layout, out_sample_fmt, out_sample_rate,
                                          in_chn_layout, codec_context->sample_fmt, codec_context->sample_rate, 0,
                                          NULL);
    //初始化SwResample的Context
    swr_init(convert_context);

}

void AudioThread::setUrl(const char *url) {
    this->url = url;
}







