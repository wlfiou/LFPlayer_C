#include "lfencode.h"
#include <iostream>
using namespace std;
extern "C"{
#include <libavcodec/avcodec.h>
#include<libavutil/opt.h>
}
LFEncode::LFEncode()
{

}
AVPacket *LFEncode::Encode(const AVFrame *frame){
    if(!frame){
        return nullptr;
    }
    unique_lock<mutex>lock(mux_);
    if(!con_){
        return nullptr;
    }
    auto re = avcodec_send_frame(con_,frame);
    if(re != 0)return  nullptr;
    auto pkt = av_packet_alloc();

    re = avcodec_receive_packet(con_,pkt);
    if(re == 0){
        return pkt;
    }
    av_packet_free(&pkt);
    if(re == AVERROR(EAGAIN) || re == AVERROR_EOF){
        return nullptr;
    }
    if(re < 0){
        PrintErr(re);
    }
    return nullptr;
}

std::vector <AVPacket *>LFEncode::End(){
    std::vector<AVPacket *>res;
    unique_lock<mutex>lock(mux_);
    if(!con_){
        return res;
    }
    auto re = avcodec_send_frame(con_,NULL);
    if (re != 0)return res;
    while (re >= 0) {
        auto pkt= av_packet_alloc();
        re = avcodec_receive_packet(con_,pkt);
        if(re != 0){
            av_packet_free(&pkt);
            break;
        }
        res.push_back(pkt);
    }
    return res;
}






