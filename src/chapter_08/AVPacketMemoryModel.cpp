//
// Created by MirsFang on 2019-03-25.
//

#include "AVPacketMemoryModel.h"

#include <iostream>
#include <string>

extern "C" {
#include "libavcodec/avcodec.h"
}


void AVPacketMemoryModel::testAVPacketAlloc() {

    AVPacket *packet = av_packet_alloc();
    std::string log = (packet->buf == nullptr) ? "null" : "not null";
    std::cout << log << std::endl;

    av_new_packet(packet, 20 * 1024 * 1024);
    memccpy(packet->data, this, 1, 20 * 1024 * 1024);

    if (packet->buf) {
        int ret = av_buffer_get_ref_count(packet->buf);
        std::cout<<"当前引用值 :"<<ret<<std::endl;
    }

    AVPacket* packet1 = av_packet_alloc();
    av_packet_ref(packet,packet);

    if (packet->buf) {
        int ret = av_buffer_get_ref_count(packet->buf);
        std::cout<<"当前引用值 1 :"<<ret<<std::endl;
    }

    av_packet_free(&packet);
    av_packet_free(&packet1);

}
