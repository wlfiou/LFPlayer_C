#include "lfaudioplay.h"

#include <iostream>
#include<SDL2/SDL.h>

extern "C"{
    #include <libavformat/avformat.h>
}

using namespace std;

class LFAudioPlayer:public LFAudioPlay{


public:
    //引用传递 可以避免临时对像
    bool Open(LFAudioSpec& spec){
        this->spec_ = spec;
        //退出上一次音频
        SDL_QuitSubSystem(SDL_INIT_AUDIO);

        SDL_AudioSpec sdl_spec;
        sdl_spec.freq = spec.freq;
        sdl_spec.format = spec.format;
        sdl_spec.channels = spec.channels;
        sdl_spec.samples = spec.samples;
        sdl_spec.silence = 0;//静音;
        sdl_spec.callback = AudioCallBack;
        sdl_spec.userdata = this;

        if(SDL_OpenAudio(&sdl_spec,nullptr) < 0){
            cerr << SDL_GetError()<<endl;
            return false;
        }
        //开始播放
         SDL_PauseAudio(0);
        return true;
    }


    void Close(){
        SDL_QuitSubSystem(SDL_INIT_AUDIO);
        unique_lock<mutex> lock(mux_);
        audio_lists_.clear();
    }
    void Callback(unsigned char *stream, int len){
        //静音
        SDL_memset(stream,0,len);
        unique_lock<mutex>lock(mux_);
        if(audio_lists_.empty())return;
        auto buf = audio_lists_.front();
        //1  buf  大于 stream 缓冲区  offset 记录位置
        //2 buf 小于stream缓冲区 拼接
        int mixed_size = 0;// 已经处理的字节数
        int need_size  = len;// 需要处理的字节数

        cur_pts_ = buf.pts;     //当前播放的pts
        last_ms_ = NowMs();     //计时开始播放

        while(mixed_size < len){
            if(audio_lists_.empty()){
                break;
            }
            buf = audio_lists_.front();
            int size = buf.data.size() - buf.offset;
            if(size > need_size){
                size = need_size;//取小值
            }
            SDL_MixAudio(stream + mixed_size,
                         buf.data.data()+buf.offset
                         ,size,volume_);
            need_size-=size;
            mixed_size+=size;
            buf.offset += size;
            if(buf.offset >= buf.data.size()){
                audio_lists_.pop_front();
            }
        }
    }

    long long cur_pts() override{
        double ms = 0;
        if(last_ms_>0){
            ms = NowMs()-last_ms_;//距离上次写入缓冲的播放时间毫秒
        }
    //pts  毫秒换算成pts 的时间基数
        if(time_base_>0){
            //
            ms = ms/(double)1000/(double)time_base_;

        }
        return cur_pts_+ ms;

    }

private:
    long long cur_pts_ = 0;//当前播放的位置
    long long last_ms_ = 0;//上次播放的时间戳


};




LFAudioPlay::LFAudioPlay()
{
    //初始化sdl 音频
    SDL_Init(SDL_INIT_AUDIO);
}
LFAudioPlay *LFAudioPlay::Instance(){
    static LFAudioPlayer x;
    return &x;
}
void LFAudioPlay::Push(AVFrame *frame){
    if(!frame || !frame->data[0]){
        return;
    }
    vector <unsigned char> buf;

    int  sample_size = 4;//32 位深度  一个声道4字节
    int channals = frame->channels;
    unsigned char *L = frame->data[0];
    unsigned char *R = frame->data[1];
    unsigned char *data = nullptr;
    //将planer 格式转成交叉格式

    switch (frame->format) {
        case AV_SAMPLE_FMT_S32:
        case AV_SAMPLE_FMT_S32P:
//        // *
//        * For audio, only linesize[0] may be set. For planar audio, each channel
//        * plane must be the same size. linesize[0] 为总大小

                // LLLL RRRR  32个字节 8个字节为 一个样本=sample_size*changnel
                // LR LR LR LR 4个样本
//        */
            buf.resize(frame->linesize[0]);
            data = buf.data();//获取vectior 的指针
            for(int i = 0;i< frame->nb_samples;i++){
                memcpy(data+i*sample_size*channals,L+i*sample_size,sample_size);
                memcpy(data+i*sample_size*channals+sample_size,R+i*sample_size,sample_size);
            }
            Push(data,frame->linesize[0]);
        return;
            break;
        default:
        break;
    }
        Push(frame->data[0],frame->linesize[0]);
}
bool LFAudioPlay::Open(LFPara &para){
    if(para.time_base->num >0){
        time_base_ =(double)para.time_base->den / (double)para.time_base->num;
        cout<<"time_base_:"<<time_base_ << endl;
    }
    cout << "LFPara &para"<<endl;
    return Open(para.para);
}
bool LFAudioPlay::Open(AVCodecParameters *para){
    LFAudioSpec spec;
    spec.channels = para->channels;
    spec.freq = para->sample_rate;
    cout << "AVCodecParameters *para"<<endl;
    switch (para->format) {
        case AV_SAMPLE_FMT_S16:
        case AV_SAMPLE_FMT_S16P:
            spec.format = AUDIO_S16;
            break;
        case AV_SAMPLE_FMT_S32:
        case AV_SAMPLE_FMT_S32P:
            spec.format = AUDIO_S32;
            break;
        case AV_SAMPLE_FMT_FLT:
        case AV_SAMPLE_FMT_FLTP:
            spec.format = AUDIO_F32;
            break;
    default:
        break;
    }
    return Open(spec);



}
