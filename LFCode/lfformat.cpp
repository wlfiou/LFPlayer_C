#include "lfformat.h"
#include <iostream>
#include <thread>
using namespace std;

extern "C"{
    #include <libavformat/avformat.h>
}

LFFormat::LFFormat()
{

}
//如果有阻塞  则会调用如下函数，如果返回1  则会跳出阻塞函数
static int TimeoutCallback(void *para){
    auto lfp = (LFFormat *)para;
    if(lfp->IsTimeout()){
        return 1;//超时退出Read
    }
    return 0;//正常阻塞
}
void LFFormat::set_c(AVFormatContext *c){
    unique_lock<mutex> lock(mux_);
    if(con_){
        if(con_->oformat){//输出上下文
            //解封装
            if(con_->pb){
                 avio_closep(&con_->pb);
            }
           avformat_free_context(con_);
        }else if(con_->iformat){
            //封装
            avformat_close_input(&con_);
        }else{
            avformat_free_context(con_);
        }

    }
    con_ = c;
    if(!con_){
        is_connected_ = false;
        return;
    }
    is_connected_ = true;
    last_time_ = NowMs();


    if(time_out_ms_>0){
        AVIOInterruptCB cb = {TimeoutCallback,this};
        con_->interrupt_callback = cb;
    }




    audio_index_ = -1;
    video_index_ = -1;

    for(int i = 0;i< c->nb_streams;i++){
        //音频
        if( c->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_AUDIO){

            audio_index_ = i;
            //也可以直接复制  因为内存完全一样 audio_time_base =  streams[i]->time_base；
            audio_time_base_.den =  c->streams[i]->time_base.den;
            audio_time_base_.num =  c->streams[i]->time_base.num;
        }else if( c->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO){
            video_index_ = i;
            video_time_base_.den =  c->streams[i]->time_base.den;
            video_time_base_.num =  c->streams[i]->time_base.num;
            video_codec_id_ =  c->streams[i]->codecpar->codec_id;
        }
    }
}

bool LFFormat::CopyPara(int stream_index, AVCodecParameters *dst){
    unique_lock<mutex>lock(mux_);
    if(!con_){
        return false;
    }
    if(stream_index <0 || stream_index>con_->nb_streams){
        return false;
    }
    auto re = avcodec_parameters_copy(dst,con_->streams[stream_index]->codecpar);
    if(re != 0){
        return false;
    }
    return true;
}
bool LFFormat::CopyPara(int stream_index, AVCodecContext *dst){
    if(!con_){
        return false;
    }
    if(stream_index <0 || stream_index>con_->nb_streams){
        return false;
    }
    auto re = avcodec_parameters_to_context(dst,con_->streams[stream_index]->codecpar);
    if(re != 0){
        return false;
    }
    return true;
}

//返回智能指针 引用计数为0时 自动释放
std::shared_ptr<LFPara>LFFormat::CopyVideoPara(){
    int index = video_index();
    shared_ptr<LFPara> re;
    unique_lock<mutex> lock(mux_);
    if(index < 0 || !con_){
        return re;
    }
    re.reset(LFPara::Create());//re 拥有LFPara::Create() 指针
    *re->time_base = con_->streams[index]->time_base;
    avcodec_parameters_copy(re->para,con_->streams[index]->codecpar);
    return re;
}
std::shared_ptr<LFPara>LFFormat::CopyAudioPara(){
    int index = audio_index();
    shared_ptr<LFPara> re;
    unique_lock<mutex> lock(mux_);
    if(index < 0 || !con_){
        return re;
    }
    re.reset(LFPara::Create());//re 拥有LFPara::Create() 指针
    *re->time_base = con_->streams[index]->time_base;
    avcodec_parameters_copy(re->para,con_->streams[index]->codecpar);
    return re;
}

bool LFFormat::RescaleTime(AVPacket *pkt, long long offset_pts, AVRational* time_base){
    if(!pkt || !time_base){
        return false;
    }
    unique_lock<mutex>lock(mux_);

    if(!con_)return false;
//将以 out_stream->time_base 为基准的时间基  转换成以*time_base,为基准的时间基
    auto out_stream = con_->streams[pkt->stream_index];
    pkt->pts = av_rescale_q_rnd(pkt->pts - offset_pts,*time_base,
                                out_stream->time_base,(AVRounding)(AV_ROUND_NEAR_INF | AV_ROUND_PASS_MINMAX));
    pkt->pts = av_rescale_q_rnd(pkt->dts - offset_pts,*time_base,
                                out_stream->time_base,(AVRounding)(AV_ROUND_NEAR_INF | AV_ROUND_PASS_MINMAX));
    pkt->duration = av_rescale_q(pkt->duration,*time_base,out_stream->time_base);
    pkt->pos = -1;
    return true;
}

bool LFFormat::RescaleTime(AVPacket *pkt, long long offset_pts, LFRational time_base){
    unique_lock<mutex>lock(mux_);
//    if(!con_){
//        return false;
//    }
//        auto out_stream = con_->streams[pkt->stream_index];
        AVRational in_time_base;
        in_time_base.num = time_base.num;
        in_time_base.den = time_base.den;

        return RescaleTime(pkt,offset_pts,&in_time_base);
}

long long LFFormat::RescaleToMs(long long pts, int index){
    unique_lock<mutex>lock(mux_);
    if(!con_ || index < 0 || index > con_->nb_streams){
        return 0;
    }
    auto in_timebase = con_->streams[index]->time_base;
    AVRational out_timebase = {1,1000};//输出timebase毫秒

    return av_rescale_q(pts,in_timebase,out_timebase);
}

void LFFormat::set_time_out_ms(int ms){
    unique_lock<mutex>lock(mux_);
    this->time_out_ms_ = ms;
    //设置回调函数  处理超时退出
    if(con_){
        AVIOInterruptCB cb = {TimeoutCallback,this};
        con_->interrupt_callback = cb;
    }
}










































