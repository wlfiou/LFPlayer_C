#ifndef LFPLAYER_H
#define LFPLAYER_H
#include "LFCode/lfdemux_task.h"
#include "LFCode/lfdecodetask.h"
#include "LFCode/lfvideoview.h"


class LFPlayer : public LFTread
{
public:
    LFPlayer();
 //回调接受 音视频的包
    void Do(AVPacket *pkt) override;
//打开音视频   初始化 和渲染
    bool Open(const char*url,void *winid);
//主线程处理同步
    void Main() override;
//开启级封装 因视频解码  和处理 同步的线程
    void start() override;
//渲染视频  播放音频
    void update();


protected:
    LFDemux_task demux_;//解封装
    LFDecodeTask audio_decode_;//音频解码
    LFDecodeTask video_decode_;//视频解码
    LFVideoView *view_ = nullptr;//

};

#endif // LFPLAYER_H
