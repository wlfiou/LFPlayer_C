#ifndef LFMUX_H
#define LFMUX_H

#include "lfformat.h"

class LFMux : public LFFormat
{
public:
    LFMux();
    //打开封装
    static AVFormatContext *Open(const char*url,
                                 AVCodecParameters *video_para = nullptr,
                                 AVCodecParameters *audio_para = nullptr
                                 );
    bool WriteHead();
    bool Write(AVPacket *pkt);
    bool WriteEnd();

    //音视频时间基础

    void set_src_video_time_base(AVRational *tb);
    void set_src_audio_time_base(AVRational *tb);

    ~LFMux();
private:
    AVRational *src_video_time_base_ = nullptr;//
    AVRational *src_audio_time_base_ = nullptr;//

    long long begin_video_pts_ = -1;//原视频开始的时间
    long long begin_audio_pts_ = -1;//原音频开始的时间
};

#endif // LFMUX_H
