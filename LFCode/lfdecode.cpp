#include "lfdecode.h"
#include <iostream>
using namespace std;

extern "C"{
#include <libavcodec/avcodec.h>
#include<libavutil/opt.h>
}

LFDecode::LFDecode()
{

}

bool LFDecode::Send(const AVPacket *pkt){//发送解码
    unique_lock<mutex>lock(mux_);
    if(!con_)return false;
    auto re = avcodec_send_packet(con_,pkt);
    if(re != 0) return false;
    return true;
}


bool LFDecode :: Recv(AVFrame *frame){
    unique_lock<mutex>lock(mux_);
    if(!con_)return false;
    auto f = frame;
    if(con_->hw_device_ctx){
        f = av_frame_alloc();
    }
    auto re = avcodec_receive_frame(con_,f);
    if(re == 0){
        if(con_->hw_device_ctx){
            re = av_hwframe_transfer_data(frame,f,0);
            av_frame_free(&f);
            if(re != 0){
                PrintErr(re);
                return false;
            }
        }
        return true;
    }
    if(con_->hw_device_ctx){
        av_frame_free(&f);
    }
    return false;
}

bool LFDecode::InitHW(int type){
    unique_lock<mutex> lock(mux_);
    if(!con_)return false;
    AVBufferRef *ctx = nullptr;//应将加速上下文
    auto re = av_hwdevice_ctx_create(&ctx,(AVHWDeviceType)type,NULL,NULL,0);
    if(re != 0){
        PrintErr(re);
        return false;
    }
    con_->hw_device_ctx = ctx;
    cout << "硬件加速" << type << endl;
    return true;
}
std::vector<AVFrame *>LFDecode::End(){
    std::vector<AVFrame *>res;
    unique_lock<mutex> lock(mux_);
    if(!con_){
        return res;
    }
    int ret = avcodec_send_packet(con_,NULL);
    while (ret >= 0) {
        auto frame = av_frame_alloc();
        ret = avcodec_receive_frame(con_,frame);
        if(ret < 0){
            av_frame_free(&frame);
            break;
        }
        res.push_back(frame);
    }
    return res;
}





