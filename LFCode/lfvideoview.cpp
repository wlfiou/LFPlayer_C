#include "lfvideoview.h"
#include "lfsdl.h"
#include <thread>
#include <QDebug>
#include <iostream>
using namespace std;
extern "C"
{
#include <libavcodec/avcodec.h>
#include <libavutil/avutil.h>
}
LFVideoView::LFVideoView()
{

}
//void MSleep(unsigned int ms){
//    auto beg = clock();
//    for(int i = 0;i<ms;i++){
//        this_thread::sleep_for(1ms);
//        if((clock()-beg )/(CLOCKS_PER_SEC / 1000) >= ms){
//            break;
//        }
//    }
//}
//long long NowMs()
//{
//    return clock() / (CLOCKS_PER_SEC / 1000);
//}

bool LFVideoView::Init(AVCodecParameters *para){
    if(!para){
        return false;
    }
    auto fmt = (Format)para->format;
    switch (para->format) {

    case AV_PIX_FMT_YUV420P:
    case AV_PIX_FMT_YUVJ420P:
        fmt = YUV420P;
        break;
    default:
        break;
    }

    return Init(para->width,para->height,fmt);

}







LFVideoView * LFVideoView::create(RenderType type){

    switch (type) {
    case LFVideoView::SDL:
        return new LFSDL();
        break;
    default:
        break;
    }
    return nullptr;
}
AVFrame  * LFVideoView::Read(){

    if(_width <= 0 || _height <= 0||!ifs_) return NULL;
    if(frame_){
        if(frame_->width != _width || frame_->height != _height||frame_->format != _fmt){
            av_frame_free(&frame_);
        }
    }


    if(!frame_){
        frame_ = av_frame_alloc();
        frame_->width = _width;
        frame_->height = _height;
        frame_->format = _fmt;
        frame_->linesize[0] = _width*4;
        if(frame_->format == AV_PIX_FMT_YUV420P){
            frame_->linesize[0] = _width;//Y
            frame_->linesize[1] = _width/2;//U
            frame_->linesize[2] = _width/2;//V
        }
        cout << "3111" << endl;
        //生成avframe、空间
        auto re = av_frame_get_buffer(frame_,0);
        if(re != 0){
            char buf[1024] = {0};
            av_strerror(re,buf,sizeof (buf) - 1);
            cout << buf << endl;
            return NULL;
        }

    }

    if(!frame_){
        return NULL;
    }
    cout << "44444" << endl;
    if(frame_->format == AV_PIX_FMT_YUV420P){
        ifs_.read((char *)frame_->data[0],frame_->linesize[0]*_height);
        ifs_.read((char *)frame_->data[1],frame_->linesize[1]*_height/2);
        ifs_.read((char *)frame_->data[2],frame_->linesize[2]*_height/2);
    }else{
        ifs_.read((char *)frame_->data[0],frame_->linesize[0]*_height);
    }
    if(ifs_.gcount() == 0){
        return NULL;
    }
    return frame_;

}
bool LFVideoView::Open(std::string filepath){
    if(ifs_.is_open()){
        ifs_.close();
    }
    ifs_.open(filepath,ios::binary);
    return ifs_.is_open();
}
bool LFVideoView::DrawFrame(AVFrame *frame){
    if(!frame || !frame->data[0]){
        return false;
    }

    //1000 毫秒
    count_ ++ ;
    if(beg_ms_ <= 0){
        beg_ms_ = clock();
        qDebug() << "beg_ms_:" << beg_ms_;
    }else if(((clock() - beg_ms_)/(CLOCKS_PER_SEC/1000)) >= 1000){

        render_fps_ = count_;
        count_ = 0;
        beg_ms_ = clock();
    }


    switch (frame->format) {
    case AV_PIX_FMT_YUV420P:
        return Draw(frame->data[0],frame->linesize[0],
                frame->data[1],frame->linesize[1],
                frame->data[2],frame->linesize[2]);
        break;
    case AV_PIX_FMT_RGBA:
    case AV_PIX_FMT_BGRA:
    case AV_PIX_FMT_ARGB:
        return Draw(frame->data[0],frame->linesize[0]);
        break;
    default:
        break;
    }
    return false;
}
