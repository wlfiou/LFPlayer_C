#ifndef LFFORMAT_H
#define LFFORMAT_H
#include "lftool.h"
#include <mutex>
struct AVFormatContext;
struct AVCodecParameters;
struct AVPacket;
struct AVFrame;
struct AVCodecContext;
struct LFRational
{
    int num;
    int den;
};

class LFFormat
{
public:
    LFFormat();

    ///////////
    /// \brief CopyPara
    /// \param stream_index
    /// \param dst
    /// \return
    ///复制参数 线程安全
    bool CopyPara(int stream_index,AVCodecParameters *dst);
    bool  CopyPara(int stream_index,AVCodecContext *dst);


    ///
    /// \brief CopyVideoPara
    /// \return
    ///返回智能指针  复制视频参数；
    std::shared_ptr<LFPara> CopyVideoPara();
    std::shared_ptr<LFPara> CopyAudioPara();

    int video_index(){return video_index_;}
    int audio_index(){return audio_index_;}

    LFRational video_time_base(){return video_time_base_;}
    LFRational audio_time_base(){return audio_time_base_;}

    /////////
    /// \brief set_c
    /// \param c
    ///设置解封装上下文 并且清理上次设置的值，如果传入null 相当于关闭上下文
    void set_c(AVFormatContext *c);
    //根据timebase换算时间
    bool RescaleTime(AVPacket *pkt,long long offset_pts,LFRational time_base);
    bool RescaleTime(AVPacket* pkt, long long offset_pts, AVRational* time_base);

    ///
    /// \brief RescaleToMs
    /// \param pts
    /// \param index
    /// \return
    ///把pts dts duration 值转为毫秒
    long long RescaleToMs(long long pts,int index);

    int video_codec_id(){return video_codec_id_; }

    //判断是否超时
    bool IsTimeout(){
        if(NowMs() - last_time_ > time_out_ms_){ //超时
            last_time_ = NowMs();
            is_connected_ = false;
            return true;
        }
        return false;
    }
//设定超时时间
    void set_time_out_ms(int ms);

    bool is_connect(){return is_connected_;}


protected:
    AVFormatContext *con_ = nullptr;//解封装封装上下文
    std::mutex mux_;
    int video_index_ = 0;
    int audio_index_ = 1;

    LFRational video_time_base_ = {1,25};
    LFRational audio_time_base_ = {1,9000};



    int video_codec_id_ = 0; //编码器ID


    int time_out_ms_ = 0;//超时时间 毫秒
    long long last_time_ = 0;
    bool is_connected_ = false; //是否连接成功

};

#endif // LFFORMAT_H
