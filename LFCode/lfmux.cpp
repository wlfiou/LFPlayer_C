#include "lfmux.h"
#include <iostream>
#include<thread>
using namespace std;

extern "C"{
 #include <libavformat/avformat.h>
}

void PrintErr(int err);
#define CERR(err)if(err!=0){PrintErr(err);return 0;}
LFMux::LFMux()
{

}

void LFMux::set_src_video_time_base(AVRational *tb){
    if(!tb)return;
    unique_lock<mutex>lock(mux_);
    if(!src_video_time_base_){
        src_video_time_base_ = new AVRational();
    }
    *src_video_time_base_ = *tb;//这里是需要复制一份 不是单纯的指向。
}
void LFMux::set_src_audio_time_base(AVRational *tb){
    if(!tb)return;
    unique_lock<mutex>lock(mux_);
    if(!src_audio_time_base_){
        src_audio_time_base_ = new AVRational();
    }
    *src_audio_time_base_ = *tb;//

}

AVFormatContext *LFMux::Open(const char *url,
                             AVCodecParameters *video_para,
                             AVCodecParameters *audio_para){
    AVFormatContext *c = nullptr;
    auto re = avformat_alloc_output_context2(&c,NULL,NULL,url);
    CERR(re);
    //添加视频流
    auto vs = avformat_new_stream(c,NULL);
    vs->codecpar->codec_type = AVMEDIA_TYPE_VIDEO;
    auto as = avformat_new_stream(c,NULL);
    as->codecpar->codec_type = AVMEDIA_TYPE_AUDIO;
    //代开IO;
    re = avio_open(&c->pb,url,AVIO_FLAG_WRITE);
    CERR(re);
    return c;
}

bool LFMux::Write(AVPacket *pkt){
    if(!pkt){
        return false;
    }
    unique_lock<mutex>lock(mux_);
    if(!con_)return false;

    if(pkt->pts == AV_NOPTS_VALUE){
        pkt->pts = 0;
        pkt->dts = 0;
    }
    if(pkt->stream_index == video_index_){
        if(begin_video_pts_ < 0){
            begin_video_pts_ = pkt->pts;
        }

        lock.unlock();
        RescaleTime(pkt,begin_video_pts_,src_video_time_base_);
    }

    auto re = av_interleaved_write_frame(con_,pkt);

    CERR(re);
    return true;
    //
}

bool LFMux::WriteHead(){
    unique_lock<mutex>lock(mux_);
    if(!con_){
        return false;
    }
    auto re = avformat_write_header(con_,nullptr);
    CERR(re);
    av_dump_format(con_,0,con_->url,1);
}

bool LFMux::WriteEnd(){
    unique_lock<mutex>lock(mux_);
    if(!con_) return false;
    int re= 0;

    re = av_write_trailer(con_);
    CERR(re);
    return true;
}
















