#ifndef LFMUXTASK_H
#define LFMUXTASK_H

#include "lftool.h"
#include "lfmux.h"
class LFMuxTask : public LFTread
{
public:
    LFMuxTask();

    void Main() override;

    ///
    /// \brief Open
    /// \param url 输出地址
    /// \param video_para  视频参数
    /// \param video_time_base 视频时间基
    /// \param audio_para  音频参数
    /// \param audio_time_base 音频时间基
    ///
    bool Open(const char *url,
             AVCodecParameters *video_para = nullptr,
             AVRational *video_time_base = nullptr,
              AVCodecParameters *audio_para = nullptr,
              AVRational *audio_time_base = nullptr
              );
    ///
    /// \brief Do
    /// \param pkt
    ///接收数据
    void Do(AVPacket *pkt);
private:
    LFMux lmue_;
    LFAVPacketList pkts_;
    std::mutex mux_;

};

#endif // LFMUXTASK_H
