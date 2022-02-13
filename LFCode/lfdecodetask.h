#ifndef LFDECODETASK_H
#define LFDECODETASK_H

#include"lftool.h"
#include"lfdecode.h"
class LFDecodeTask : public LFTread
{
public:
    LFDecodeTask();
    ///
    /// \brief Open
    /// \param para
    /// \return
    ///打开编码器
    bool Open(AVCodecParameters *para);
    //责任链处理函数
    void Do(AVPacket *pkt) override;
    //线程主函数
    void Main() override;
    ///
    /// \brief GetFrame
    /// \return
    ///线程安全 返回当前需要渲染的AVFrame,如果没有返回nullptr
    /// need_view_控制渲染
    /// 返回结果需要用freeframe 释放
    AVFrame *GetFrame();


    void  set_stream_index(int i){stream_index_ = i;}

    void set_frame_cache(bool is){frame_cache_ = is;}


    bool is_open(){return is_open_;}
    //设置同步时间
    void set_sys_pts(long long p){syn_pts_ = p;}

    void set_block_size(int s){block_size_ = s;}

    void stop();

private:
    std::mutex mux_;
    LFDecode decode_;
    AVFrame *frame_ = nullptr;//解码后存储
    LFAVPacketList pkt_list_;
    bool need_view_ = false;//是否需要渲染，每帧只渲染一次通过GetFrame
    int stream_index_ = 0;//区分音视频
    std::list<AVFrame *>frames_;//解码后存放帧
    bool frame_cache_ = false;//是否需要缓存帧  只有音频需要，
    //因为音频解码速度快，可能大于播放速度，会造成丢帧

    int block_size_ = 0;//阻塞大小
    long  long syn_pts_= -1;//同步时间
    bool is_open_ = false;


};

#endif // LFDECODETASK_H
