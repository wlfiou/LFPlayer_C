#include "lfplayer.h"

#include "LFCode/lfaudioplay.h"
#include <iostream>
using namespace std;
LFPlayer::LFPlayer()
{

}

bool LFPlayer::Open(const char * url,void* winid){
    //解封装

    if(!demux_.Open(url)){
        return false;
    }
    //视频解码
    auto vp = demux_.CopyVideoPara();
    if(vp){
        if(!video_decode_.Open(vp->para)){
            return false;
        }
        //用于过滤音频包
        video_decode_.set_stream_index(demux_.video_index());
        //缓冲
        video_decode_.set_block_size(100);
        //视频渲染
        if(!view_)
        {
               view_ = LFVideoView::create();
               view_->set_win_id(winid);
        }
        if(!view_->Init(vp->para)){
            return false;
        }
    }

    //音频解码

    auto ap = demux_.CopyAudioPara();
    if(ap){
        if(!audio_decode_.Open(ap->para)){
            return false;
        }
        //缓冲
        audio_decode_.set_block_size(100);

        //用来过滤视频
        audio_decode_.set_stream_index(demux_.audio_index());

        audio_decode_.set_frame_cache(true);
        LFAudioPlay::Instance()->Open(*ap);

        //设置时间基数
       // double time_base = 0;
    }else{
        demux_.set_syn_type(LFSYN_VIDEO);//根据视频同步
    }
    //解封装数据传到当前类
    demux_.set_next(this);
    return true;
}

void LFPlayer::Do(AVPacket *pkt){
    if(audio_decode_.is_open()){
        audio_decode_.Do(pkt);
    }
    if(video_decode_.is_open()){
        video_decode_.Do(pkt);
    }
}

void LFPlayer::start(){
    demux_.start();
    if(audio_decode_.is_open()){
        audio_decode_.start();
    }
    if(video_decode_.is_open()){
        video_decode_.start();
    }
    LFTread::start();
}
//渲染视频 播放音频
void LFPlayer::update(){
    //渲染视频
    if(view_){
        auto f = video_decode_.GetFrame();
        if(f){

            view_->DrawFrame(f);
            LFreeFrame(&f);
        }
    }
    //音频播放
    auto au = LFAudioPlay::Instance();
    auto f = audio_decode_.GetFrame();
    if(!f)return;
    au->Push(f);
    LFreeFrame(&f);
}

void LFPlayer::Main(){
    long long syn = 0;
    auto au = LFAudioPlay::Instance();
    auto ap = demux_.CopyAudioPara();
    auto vp = demux_.CopyVideoPara();
    while (!is_exit_) {
        syn = LFRescale(au->cur_pts(),ap->time_base,
                        vp->time_base);
        cout << "syn:" <<syn<<endl;
        audio_decode_.set_sys_pts(au->cur_pts()+10000);
        video_decode_.set_sys_pts(syn);
        MSleep(1);
    }
}











