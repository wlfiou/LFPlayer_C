#include "lfcodec.h"
#include <iostream>
using namespace std;
extern "C"{
#include <libavcodec/avcodec.h>
#include <libavutil/opt.h>
}

LFCodec::LFCodec()
{

}
void PrintErr(int err){
    char buf[1024] = {0};
    av_strerror(err,buf,sizeof (buf)-1);
    cerr << buf << endl;
}
///////////
/// \brief Create
/// \param codec_id   编码器id
/// \param isencode
/// \return
///创建编解码上下文  失败返回nullptr
AVCodecContext * LFCodec::Create(int codec_id, bool isencode){
    AVCodec *codec = nullptr;
    if(isencode){
        codec = avcodec_find_encoder((AVCodecID)codec_id);
    }else{
        codec = avcodec_find_decoder((AVCodecID)codec_id);
    }

    if(!codec){
        cerr << "avcodec_find_encoder fail" << endl;
        return nullptr;
    }
    //创建上下文
    auto c = avcodec_alloc_context3(codec);
    if(!c){
        cerr << "avcodec_alloc_context3 faild" << endl;
        return nullptr;
    }

    //设置参数默认值
    c->time_base = {1,25};
    c->pix_fmt = AV_PIX_FMT_YUV420P;
    c->thread_count = 16;
    return c;
}

void LFCodec::set_c(AVCodecContext *c){
    unique_lock<mutex> lock(mux_);
    if(con_){
        avcodec_free_context(&con_);
    }
    con_ = c;
}
bool LFCodec::SetOpt(const char *key, const char *val){
    unique_lock<mutex>lock(mux_);
        if (!con_)return false;
        auto re = av_opt_set(con_->priv_data, key, val, 0);
        if (re != 0)
        {
            cerr << "av_opt_set failed!";
            PrintErr(re);
        }
        return true;
}
bool LFCodec::SetOpt(const char *key, const int val){
    unique_lock<mutex>lock(mux_);
        if (!con_)return false;
        auto re = av_opt_set_int(con_->priv_data, key, val, 0);
        if (re != 0)
        {
            cerr << "av_opt_set failed!";
            PrintErr(re);
        }
        return true;
}
//////////////
/// \brief LFCodec::Open
/// \return
///打开编码器 线程安全
bool LFCodec::Open(){
    unique_lock<mutex>lock(mux_);
    if(!con_){
        return false;
    }
    auto re = avcodec_open2(con_,NULL,NULL);

    if(re != 0){
        PrintErr(re);
        return false;
    }
    return true;
}
////////
/// \brief LFCodec::CreateFrame
/// \return
///根据context  创建一个avframe 需要调用者释放
AVFrame * LFCodec::CreateFrame(){
    unique_lock<mutex> lock(mux_);
    if(!con_){
        return nullptr;
    }
    auto frame = av_frame_alloc();
    frame->width = con_->width;
    frame->height = con_->height;
    frame->format = con_->pix_fmt;
    auto re = av_frame_get_buffer(frame,0);
    if(re != 0){
        av_frame_free(&frame);
        PrintErr(re);
        return nullptr;
    }
    return frame;
}




