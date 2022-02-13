#include "lfdecodetask.h"
#include <iostream>
#include <typeinfo>
extern "C" {
    #include <libavcodec/avcodec.h>
    #include <libavformat/avformat.h>
}
using namespace std;

LFDecodeTask::LFDecodeTask()
{

}

void LFDecodeTask::stop(){
    LFTread::stop();
    pkt_list_.Clear();
    unique_lock<mutex> lock(mux_);
    decode_.set_c(nullptr);
    is_open_ = false;
    while (!frames_.empty()) {
        av_frame_free(&frames_.front());
        frames_.pop_front();
    }
}

bool LFDecodeTask::Open(AVCodecParameters *para){

    if(!para){
        LOGERROR("para is null");
        return false;
    }
    unique_lock<mutex>lock(mux_);
     is_open_ = false;
    auto c = decode_.Create(para->codec_id,false);

    if(!c){
        LOGERROR("decode_.Create faild");
        return false;
    }
    //复制视频参数
    avcodec_parameters_to_context(c,para);
    cout << "para:" << para->format<< para->width<<endl;
    decode_.set_c(c);
    if(!decode_.Open()){
        LOGERROR("decode_.Open() failed");
        return false;
    }
    LOGINFO("Open decode suc");
    is_open_ = true;
    return true;

}
//责任链处理函数

void LFDecodeTask::Do(AVPacket *pkt){

    if(!pkt || pkt->stream_index!=stream_index_){//判断是否是音视频
        return;
    }
    pkt_list_.Push(pkt);
    cout << "pkt_list_.Size():"<<pkt_list_.Size()<<endl;
    while (!is_exit_) {
        if(pkt_list_.Size()>block_size_){
            MSleep(1);
            continue;
        }
        break;
    }
}

AVFrame * LFDecodeTask::GetFrame(){
    unique_lock<mutex>lock(mux_);

    if(frame_cache_){
        if(frames_.empty()){
            return nullptr;
        }
        auto f = frames_.front();
        frames_.pop_front();
        return f;
    }

    if(!need_view_ || !frame_||!frame_->buf[0]){
        return nullptr;
    }

    auto f = av_frame_alloc();
    auto re = av_frame_ref(f,frame_);
    cout << "video_decode_.GetFrame" <<f->width<< endl;
    if(re != 0){
        av_frame_free(&f);
        PrintErr(re);
        return nullptr;
    }
    need_view_ = false;
    return f;
}

void  LFDecodeTask::Main(){
    {
        unique_lock<mutex>lock(mux_);
        if(!frame_){
            frame_ = av_frame_alloc();
        }
    }
    long long cur_pts = -1;//当前解码到的pts（以解码数据为准）
    while (!is_exit_) {

        //同步
               while (!is_exit_)
               {
                   if (syn_pts_ >= 0 && cur_pts > syn_pts_)
                   {
                       MSleep(1);
                       continue;
                   }
                   break;
               }

        auto pkt = pkt_list_.Pop();
        if(!pkt){
            //等待一号秒后继续取值
            this_thread::sleep_for(1ms);
            continue;
        }
        //发送到解码线程
        bool re = decode_.Send(pkt);
        av_packet_free(&pkt);
        if(!re){
            this_thread::sleep_for(1ms);
            continue;
        }
        {
            unique_lock<mutex>lock(mux_);
            if(decode_.Recv(frame_)){
                need_view_ = true;
                cur_pts = frame_->pts;
                cout <<"frame_->width"<<frame_->width<<flush;
            }
            if(frame_cache_){
                auto f = av_frame_alloc();
                av_frame_ref(f,frame_);
                frames_.push_back(f);
            }

        }
        this_thread::sleep_for(1ms);
    }
    {
        unique_lock<mutex>lock(mux_);
        if(frame_){
            av_frame_free(&frame_);
        }
    }
}

