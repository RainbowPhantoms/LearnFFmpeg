//
// Created by MirsFang on 2019-03-13.
//

#include <iostream>

extern "C" {
#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
}

using namespace std;

/**
 * avformat 的简单使用
 *
 * 分离视频
 *
 * @param url 视频的Url(本地/网络)
 */
void chapter05_h264(const char *url) {
    //打开文件流
    FILE *output = fopen("./output.h264", "wb+");

    //返回状态码
    int ret_code;
    //寻找到指定的流下标
    int media_index = -1;
    //分配一个存储解封装后的Packet
    AVPacket *packet = av_packet_alloc();

    //初始化网络
    avformat_network_init();

    //分配AVFormatContext
    AVFormatContext *avFormatContext = avformat_alloc_context();
    if (avFormatContext == nullptr) {
        cout << "[error] AVFormat alloc error" << endl;
        goto failed;
    }

    //打开输入流
    ret_code = avformat_open_input(&avFormatContext, url, nullptr, nullptr);
    if (ret_code < 0) {
        cout << "[error] open input failed " << av_err2str(AVERROR(ret_code)) << endl;
        goto failed;
    }

    //读取媒体文件信息
    ret_code = avformat_find_stream_info(avFormatContext, nullptr);
    if (ret_code < 0) {
        cout << "[error] find stream info failed " << av_err2str(AVERROR(ret_code)) << endl;
        goto failed;
    }

    //寻找到指定的视频流
    media_index = av_find_best_stream(avFormatContext, AVMEDIA_TYPE_VIDEO, -1, -1, NULL, 0);
    if (media_index < 0) {
        cout << "[error] find stream index error" << endl;
        goto failed;
    }

    //读取pakcet
    while (av_read_frame(avFormatContext, packet) == 0) {
        //判断是不是指定流的packet
        if (packet->stream_index == media_index) {
            //写入到文件
            fwrite(packet->data, 1, packet->size, output);
        }
    }


    failed:
    av_packet_free(&packet);
    avformat_close_input(&avFormatContext);

}

/**
 * avformat 的简单使用
 *
 * 分离视频
 * @param url 视频的Url(本地/网络)
 */
void chapter05_h264_01(const char *url) {
    //打开文件流
    FILE *output = fopen("./output_01.h264", "wb+");

    //返回状态码
    int ret_code;
    //寻找到的指定的流下标
    int media_index = -1;
    //分配一个存储读取出来的数据的 packet
    AVPacket *packet = av_packet_alloc();

    //初始化网络
    avformat_network_init();

    //创建H264_mp4的filter
    const AVBitStreamFilter *bsf = av_bsf_get_by_name("h264_mp4toannexb");
    AVBSFContext *ctx = NULL;

    //分配AVFormatContext
    AVFormatContext *avFormatContext = avformat_alloc_context();
    if (avFormatContext == nullptr) {
        cout << "[error] AVFormat alloc error" << endl;
        goto failed;
    }

    //打开输入流
    ret_code = avformat_open_input(&avFormatContext, url, nullptr, nullptr);
    if (ret_code < 0) {
        cout << "[error] open input failed " << av_err2str(AVERROR(ret_code)) << endl;
        goto failed;
    }

    //读取媒体文件信息
    ret_code = avformat_find_stream_info(avFormatContext, nullptr);
    if (ret_code < 0) {
        cout << "[error] find stream info failed " << av_err2str(AVERROR(ret_code)) << endl;
        goto failed;
    }

    //寻找到指定的视频流
    media_index = av_find_best_stream(avFormatContext, AVMEDIA_TYPE_VIDEO, -1, -1, NULL, 0);
    if (media_index < 0) {
        cout << "[error] find stream index error" << endl;
        goto failed;
    }

    //alloc bsf
    ret_code = av_bsf_alloc(bsf, &ctx);
    if (ret_code < 0) {
        cout << "[error] BSF alloc failed " << av_err2str(AVERROR(ret_code)) << endl;
        goto failed;
    }

    //复制解码器参数到BSFContext
    ret_code = avcodec_parameters_copy(ctx->par_in, avFormatContext->streams[media_index]->codecpar);
    if (ret_code < 0) {
        cout << "[error] BSF copy parameter failed " << av_err2str(AVERROR(ret_code)) << endl;
        goto failed;
    }

    //同步time_base
    ctx->time_base_in = avFormatContext->streams[media_index]->time_base;

    //初始化bsf
    ret_code = av_bsf_init(ctx);
    if (ret_code < 0) {
        cout << "[error] BSF init failed " << av_err2str(AVERROR(ret_code)) << endl;
        goto failed;
    }

    while (av_read_frame(avFormatContext, packet) == 0) {
        if (packet->stream_index != media_index)continue;
        //发送packet到BitStreamFilter
        ret_code = av_bsf_send_packet(ctx, packet);
        if (ret_code < 0) {
            cout << "[error] BSF send packet failed " << av_err2str(AVERROR(ret_code)) << endl;
            goto failed;
        }

        //接受添加sps pps头的packet
        while ((ret_code = av_bsf_receive_packet(ctx, packet)) == 0) {
            //写入到文件
            fwrite(packet->data, 1, packet->size, output);
            av_packet_unref(packet);
        }

        //需要输入数据
        if (ret_code == AVERROR(EAGAIN)) {
            cout << "[debug] BSF EAGAIN " << endl;
            av_packet_unref(packet);
            continue;
        }

        //已经读取到结尾
        if (ret_code == AVERROR_EOF) {
            cout << "[debug] BSF EOF " << endl;
            break;
        }

        if (ret_code < 0) {
            cout << "[error] BSF receive packet failed " << av_err2str(AVERROR(ret_code)) << endl;
            goto failed;
        }
    }

    //Flush
    ret_code = av_bsf_send_packet(ctx, NULL);
    if (ret_code < 0) {
        cout << "[error] BSF flush send packet failed " << av_err2str(AVERROR(ret_code)) << endl;
        goto failed;
    }

    while ((ret_code = av_bsf_receive_packet(ctx, packet)) == 0) {
        fwrite(packet->data, 1, packet->size, output);
    }

    if (ret_code != AVERROR_EOF) {
        cout << "[debug] BSF flush EOF " << endl;
        goto failed;
    }


    failed:
    //释放packet
    av_packet_free(&packet);
    //释放AVFormatContext
    avformat_close_input(&avFormatContext);
    //关闭网络流
    avformat_network_deinit();
    //释放BSFContext
    av_bsf_free(&ctx);
    //关闭文件流
    fclose(output);
}

