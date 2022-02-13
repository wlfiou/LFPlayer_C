#ifndef LFAUDIOPLAY_H
#define LFAUDIOPLAY_H



#define AUDIO_U8        0x0008  /**< Unsigned 8-bit samples */
#define AUDIO_S8        0x8008  /**< Signed 8-bit samples */
#define AUDIO_U16LSB    0x0010  /**< Unsigned 16-bit samples */
#define AUDIO_S16LSB    0x8010  /**< Signed 16-bit samples */
#define AUDIO_U16MSB    0x1010  /**< As above, but big-endian byte order */
#define AUDIO_S16MSB    0x9010  /**< As above, but big-endian byte order */
#define AUDIO_U16       AUDIO_U16LSB
#define AUDIO_S16       AUDIO_S16LSB
/* @} */

/**
 *  \name int32 support
 */
 /* @{ */
#define AUDIO_S32LSB    0x8020  /**< 32-bit integer samples */
#define AUDIO_S32MSB    0x9020  /**< As above, but big-endian byte order */
#define AUDIO_S32       AUDIO_S32LSB
/* @} */

/**
 *  \name float32 support
 */
 /* @{ */
#define AUDIO_F32LSB    0x8120  /**< 32-bit floating point samples */
#define AUDIO_F32MSB    0x9120  /**< As above, but big-endian byte order */
#define AUDIO_F32       AUDIO_F32LSB
/* @} */

/**
 *  \name Native audio byte ordering
 */
 /* @{ */
#if SDL_BYTEORDER == SDL_LIL_ENDIAN
#define AUDIO_U16SYS    AUDIO_U16LSB
#define AUDIO_S16SYS    AUDIO_S16LSB
#define AUDIO_S32SYS    AUDIO_S32LSB
#define AUDIO_F32SYS    AUDIO_F32LSB
#else
#define AUDIO_U16SYS    AUDIO_U16MSB
#define AUDIO_S16SYS    AUDIO_S16MSB
#define AUDIO_S32SYS    AUDIO_S32MSB
#define AUDIO_F32SYS    AUDIO_F32MSB
#endif

#include <vector>
#include <list>
#include<mutex>
#include "LFCode/lftool.h"
//  上面是为了 不让头文件 与SDL  有多余的相关性 所以直接拷贝宏过来，不影响后面
struct LFAudioSpec{


    int freq = 44100;//音频采样率   每秒44100个样本
    unsigned short format = AUDIO_S16SYS;
    unsigned char channels = 2;//声道数
    unsigned short samples = 1024;//音频缓冲区 每次读取 样本数量  大小(字节数) 即为 samples*channels*16/4

};

struct LFData{
    std::vector<unsigned char> data; //每次从文件读取的数据 大小为文件读取缓冲区大小
    int offset = 0;//偏移位置
    long long pts;
};

class LFAudioPlay
{
public:

    static LFAudioPlay *Instance();
    ///
    /// \brief Open
    /// \param spec
    /// \return
    ///打开音频 开始播放  调动回调函数
    virtual bool Open(LFAudioSpec &spec) = 0;
    ///
    /// \brief Open
    /// \param frame
    /// \return
    /// 如果是解封装解码来的数据
    virtual bool Open(AVCodecParameters *para);
    virtual bool Open(LFPara &para);
    virtual void Close() = 0;


    //获取当前播放位置
    virtual long long cur_pts() = 0;

    ///
    /// \brief Push
    /// \param frame
    ///如果是解封装解码来的数据
    virtual void Push(AVFrame *frame);

    ///
    /// \brief Push
    /// \param data  文件读取的数据
    /// \param size  读取的大小
    ///
    void Push(const unsigned char *data,int size){
        std::unique_lock<std::mutex> lock(mux_);
        //这里直接push 而不是新建  ldata =  LFData();ldata.data.assign(data, data + size)
        //是因为 push 本身就是一个复制操作 我们希望直接操作item本身
        audio_lists_.push_back(LFData());
        //assign 从data 到 data+size 处 全部复制到 audio_lists_.back().data
        audio_lists_.back().data.assign(data,data+size);

    }

    void set_speed(float s){
        auto spec = spec_;
        auto old_freq = spec.freq;
        spec.freq *=s;
        Open(spec);
        spec_.freq = old_freq;
    }

    void set_volume(int v){
        volume_ = v;
    }

protected:

    LFAudioPlay();
    double time_base_ = 0;

    virtual void Callback(unsigned char *stream,int len) = 0;
    static void AudioCallBack(void *userdata,unsigned char *stream,int len){
        auto ap = (LFAudioPlay *)userdata;
        LOGINFO("AudioCallBack");
        ap->Callback(stream,len);
    }

    std::list<LFData> audio_lists_;
    std::mutex mux_;
    unsigned char volume_ = 128;//音量
     LFAudioSpec spec_;//传进来的数据


};

#endif // LFAUDIOPLAY_H
