//
// Created by MirsFang on 2019-03-25.
//

#include "VideoThread.h"

VideoThread::VideoThread() {

}

VideoThread::~VideoThread() {
    if (format_context != nullptr) avformat_close_input(&format_context);
    if (codec_context != nullptr) avcodec_free_context(&codec_context);
    if (packet != nullptr) av_packet_free(&packet);
    if (frame != nullptr) av_frame_free(&frame);
}

void VideoThread::start() {
    prepare_codec();
    if (pthread_create(&pid, NULL, start_thread, (void *) this) != 0) {
        std::cout << "初始化视频线程失败!" << std::endl;
        return;
    }
}

void *VideoThread::start_thread(void *arg) {
    VideoThread *audioThread = (VideoThread *) arg;
    audioThread->run();
    return nullptr;
}

void VideoThread::run() {
    std::cout << "视频线程运行中..." << std::endl;
    decodec_frame();
}

void VideoThread::setRender(AVRender *render) {
    this->avRender = render;
}

void VideoThread::setUrl(const char *url) {
    this->url = url;
}

void VideoThread::prepare_codec() {
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

    //寻找到视频流的下标
    video_index = av_find_best_stream(format_context, AVMEDIA_TYPE_VIDEO, -1, -1, NULL, 0);
    //将视频流的的编解码信息拷贝到codecContext中
    retcode = avcodec_parameters_to_context(codec_context, format_context->streams[video_index]->codecpar);
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
}

void VideoThread::decodec_frame() {
    int sendcode = 0;

    //计算帧率
    double frameRate = av_q2d(format_context->streams[video_index]->avg_frame_rate);
    //计算显示的时间
    Uint32 display_time_ms = 0;

    if (!is_interval_sync) {
        display_time_ms = sync_frame_rate(frameRate);
    }

    //记录帧间延迟
    clock_t start = 0, finish = 0;
    //读取包
    while (av_read_frame(format_context, packet) == 0) {
        if (packet->stream_index != video_index)continue;
        //接受解码后的帧数据
        while (avcodec_receive_frame(codec_context, frame) == 0) {
            /**
             * 如果开启帧间隔同步模式,那么是根据
             *
             *  显示时长 = 当前帧 - 上一帧 - 单帧解码耗时
             *
             *  可得出当前帧真正要显示的时间
             *
             * **/
            if (is_interval_sync) {
                //计算上一帧与当前帧的延时
                display_time_ms = (Uint32) (
                        sync_frame_interval(format_context->streams[video_index]->time_base, frame->pts) * 1000);
                //帧解码结束时间
                finish = clock();
                double diff_time = (finish - start) / 1000;

                //减去帧间解码时差 帧解码开始时间 - 帧解码结束时间
                if (display_time_ms > diff_time)display_time_ms = display_time_ms - (Uint32) diff_time;

            }
            //绘制图像
            if (avRender)avRender->renderVideo(frame, display_time_ms);

            av_frame_unref(frame);
            //帧解码开始时间
            start = clock();
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

Uint32 VideoThread::sync_frame_rate(double frame_rate) {
    return 1 * 1000 / frame_rate;
}

double VideoThread::sync_frame_interval(AVRational timebase, int pts) {
    double display = (pts - last_pts) * av_q2d(timebase);
    last_pts = pts;
//    std::cout << "pts : " << pts * av_q2d(timebase) << "   --  display :" << display << std::endl;
    return display;
}


